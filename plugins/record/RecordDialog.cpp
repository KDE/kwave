/*************************************************************************
       RecordDialog.cpp  -  dialog window for controlling audio recording
                             -------------------
    begin                : Wed Aug 20 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <qbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qslider.h>
#include <qtabwidget.h>

#include <kapplication.h> // for invokeHelp
#include <kcombobox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"
#include "libgui/HMSTimeWidget.h"
#include "libgui/KwaveFileDialog.h"

#include "LevelMeter.h"
#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordParams.h"
#include "RecordState.h"

#include "filenew.xpm"
#include "record_stop.xpm"
#include "record_pause.xpm"
#include "krec_record.xpm"

/* some macros, for laziness ;-) */
#define SETUP(enabled,property,check,value) \
	check->setChecked(m_params.enabled); \
	value->setValue(m_params.property); \

#define STD_SETUP(enabled,property,control) \
	SETUP(enabled,property,chk##control,sb##control); \
	sb##control->setEnabled(chk##control->isEnabled() && \
	                        chk##control->isChecked());

#define STD_SETUP_SLIDER(enabled,property,control) \
	STD_SETUP(enabled,property,control); \
	sl##control->setEnabled(sb##control->isEnabled());

//***************************************************************************
RecordDialog::RecordDialog(QWidget *parent, QStringList &params,
                           RecordController *controller)
    :RecordDlg(parent,0), m_methods_map(),
     m_file_filter(), m_devices_list_map(),
     m_state(REC_EMPTY), m_params(),
     m_supported_resolutions(), m_buffer_progress_count(0),
     m_buffer_progress_total(0), m_buffer_progress_timer(this),
     m_record_enabled(true), m_samples_recorded(0),
     m_enable_setDevice(true)
{
    /* get initial parameters */
    m_params.fromList(params);

    /* set the icons of the record control buttons */
    btNew->setIconSet(   QIconSet(QPixmap(xpm_filenew)));
    btStop->setIconSet(  QIconSet(QPixmap(xpm_stop)));
    btPause->setIconSet( QIconSet(QPixmap(xpm_pause)));
    btRecord->setIconSet(QIconSet(QPixmap(xpm_krec_record)));

    // fill the combo box with playback methods
    unsigned int index=0;
    for (index=0; index < m_methods_map.count(); index++) {
	cbMethod->insertItem(m_methods_map.description(index, true));
    }
    cbMethod->setEnabled(cbMethod->count() > 1);

    /* --- set up all controls with their default/startup values --- */

    // pre-record
    STD_SETUP_SLIDER(pre_record_enabled, pre_record_time, RecordPre);
    connect(chkRecordPre, SIGNAL(toggled(bool)),
            this,         SLOT(preRecordingChecked(bool)));
    connect(sbRecordPre,  SIGNAL(valueChanged(int)),
            this,         SLOT(preRecordingTimeChanged(int)));

    // record time
    STD_SETUP(record_time_limited, record_time, RecordTime);

    // record trigger
    STD_SETUP_SLIDER(record_trigger_enabled, record_trigger, RecordTrigger);

    // amplification
    STD_SETUP_SLIDER(amplification_enabled, amplification, LevelAmplify);

    // AGC
    STD_SETUP_SLIDER(agc_enabled, agc_decay, LevelAGC);

    // fade in
    STD_SETUP(fade_in_enabled, fade_in_time, LevelFadeIn);

    // fade out
    STD_SETUP(fade_out_enabled, fade_out_time, LevelFadeOut);

    // sample rate, bits per sample, track
    // -> will be initialized later, by the plugin

    // number of buffers
    slSourceBufferCount->setValue(m_params.buffer_count);

    // power of buffer size
    slSourceBufferSize->setValue(m_params.buffer_size);
    sourceBufferSizeChanged(m_params.buffer_size);

    // various displays: level meter, oscilloscope, FFT, Overview
    chkDisplayLevelMeter->setChecked(m_params.display_level_meter);
    chkDisplayOscilloscope->setChecked(m_params.display_oscilloscope);
    chkDisplayFFT->setChecked(m_params.display_fft);
    chkDisplayOverview->setChecked(m_params.display_overview);

    // after this point all controls have their initial values

    /* --- connect some missing lowlevel gui functionality --- */

    connect(cbMethod, SIGNAL(activated(int)),
            this, SLOT(methodSelected(int)));

    // record buffer size and count
    slSourceBufferCount->setValue(m_params.buffer_count);
    slSourceBufferSize->setValue(m_params.buffer_size);
    connect(slSourceBufferSize, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferSizeChanged(int)));
    connect(slSourceBufferCount, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferCountChanged(int)));

    // device treeview
    connect(listDevices, SIGNAL(selectionChanged(QListViewItem *)),
            SLOT(listEntrySelected(QListViewItem *)));

    // "select device..." button
    connect(btSourceSelect, SIGNAL(clicked()),
            this, SLOT(selectRecordDevice()));
    connect(cbDevice, SIGNAL(activated(const QString &)),
            this, SLOT(setDevice(const QString &)));

    // visualizations
    connect(chkDisplayLevelMeter, SIGNAL(toggled(bool)),
            this, SLOT(displayLevelMeterChecked(bool)));

    connect(chkRecordTime, SIGNAL(toggled(bool)),
            this, SLOT(recordTimeChecked(bool)));
    connect(sbRecordTime, SIGNAL(valueChanged(int)),
            this, SLOT(recordTimeChanged(int)));

    connect(chkRecordTrigger, SIGNAL(toggled(bool)),
            this, SLOT(triggerChecked(bool)));
    connect(sbRecordTrigger, SIGNAL(valueChanged(int)),
            this, SLOT(triggerChanged(int)));

    connect(cbFormatSampleRate, SIGNAL(textChanged(const QString &)),
            this, SLOT(sampleRateChanged(const QString &)));
    connect(cbFormatSampleRate, SIGNAL(activated(const QString &)),
            this, SLOT(sampleRateChanged(const QString &)));

    connect(sbFormatTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));

    connect(cbFormatCompression, SIGNAL(activated(const QString &)),
            this, SLOT(compressionChanged(const QString&)));

    connect(sbFormatResolution, SIGNAL(valueChanged(int)),
            this, SLOT(bitsPerSampleChanged(int)));

    connect(cbFormatSampleFormat, SIGNAL(activated(const QString &)),
            this, SLOT(sampleFormatChanged(const QString &)));

    // connect the buttons to the record controller
    connect(btNew, SIGNAL(clicked()),
            controller, SLOT(actionReset()));
    connect(btStop, SIGNAL(clicked()),
            controller, SLOT(actionStop()));
    connect(btPause, SIGNAL(clicked()),
            controller, SLOT(actionPause()));
    connect(btRecord, SIGNAL(clicked()),
            controller, SLOT(actionStart()));

    // connect the notifications/commands of the record controller
    connect(controller, SIGNAL(stateChanged(RecordState)),
            this, SLOT(setState(RecordState )));

    // timer for updating the buffer progress bar
    connect(&m_buffer_progress_timer, SIGNAL(timeout()),
            this, SLOT(updateBufferProgressBar()));

    // help button
    connect(btHelp, SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the initial state of the dialog to "Reset/Empty"
    setState(REC_EMPTY);

    // disable the "level" tab, it it not implemented yet
    tabRecord->setCurrentPage(1);
    QWidget *page = tabRecord->currentPage();
    tabRecord->setCurrentPage(0);
    if (page) delete page;

}

//***************************************************************************
RecordDialog::~RecordDialog()
{
    updateBufferState(0,0);
}

//***************************************************************************
RecordParams &RecordDialog::params()
{
    return m_params;
}

//***************************************************************************
void RecordDialog::setMethod(record_method_t method)
{
    m_params.method = method;
    cbMethod->setCurrentItem(m_methods_map.findFromData(
        m_params.method));
}

//***************************************************************************
void RecordDialog::methodSelected(int index)
{
    record_method_t method = m_methods_map.data(index);
//     qDebug("RecordDialog::methodSelected(%d) - %d", index, (int)method);

    Q_ASSERT(method > RECORD_NONE);
    Q_ASSERT(method < RECORD_INVALID);
    if (method <= RECORD_NONE) return;
    if (method >= RECORD_INVALID) return;

    if (method != m_params.method) {
	setMethod(method);
	emit sigMethodChanged(method);
    }
}

//***************************************************************************
void RecordDialog::setSupportedDevices(QStringList devices)
{
//     qDebug("RecordDialog::setSupportedDevices(QStringList devices)");
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;
    QString current_device = m_params.device_name;

    // disable all that noisy stuff that comes from modifying the
    // device controls...
    m_enable_setDevice = false;

    KIconLoader icon_loader;

    cbDevice->clearEdit();
    cbDevice->clear();
    listDevices->clear();

    if (devices.contains("#EDIT#")) {
	devices.remove("#EDIT#");
	cbDevice->setEditable(true);
    } else {
	cbDevice->setEditable(false);
    }

    if (devices.contains("#SELECT#")) {
	devices.remove("#SELECT#");
	btSourceSelect->setEnabled(true);
	btSourceSelect->show();
    } else {
	btSourceSelect->setEnabled(false);
	btSourceSelect->hide();
    }

    if (devices.contains("#TREE#")) {
	// treeview mode
	devices.remove("#TREE#");
	listDevices->setEnabled(true);
	cbDevice->setEnabled(false);
	cbDevice->hide();
	m_devices_list_map.clear();

	// build a tree with all nodes in the list
	for (QStringList::Iterator it=devices.begin();
	     it != devices.end(); ++it)
	{
	    QString dev_id = *it;
	    QListViewItem *parent = 0;

	    QStringList list;
	    list = QStringList::split("||", dev_id, true);
	    for (QStringList::Iterator it=list.begin();
	         it != list.end(); ++it)
	    {
		QString token = *it;
		QListViewItem *item = 0;

		// split the icon name from the token
		QString icon_name;
		int pos = token.find('|');
		if (pos > 0) {
		    icon_name = token.mid(pos+1);
		    token     = token.left(pos);
		}

		// find the first item with the same text
		// and the same root
		for (QListViewItem *node = (parent) ? parent->firstChild() :
		     listDevices->firstChild(); (node);
		     node = node->nextSibling())
		{
		    if (node->text(0) == token) {
			item = node;
			break;
		    }
		}

		if (item) {
		    // already in the list
		    parent = item;
		} else if (parent) {
		    // new leaf, add to the parent
		    item = new QListViewItem(parent, token);
		    Q_ASSERT(item);
		    if (item) m_devices_list_map.insert(item, dev_id);

		    parent->setOpen(true);
		    parent->setSelectable(false);
		    parent->setExpandable(true);
		    if (m_devices_list_map.contains(parent)) {
			// make the parent not selectable
			m_devices_list_map.remove(parent);
		    }
		} else {
		    // new root node
		    item = new QListViewItem(listDevices, token);
		    Q_ASSERT(item);
		    if (item) m_devices_list_map.insert(item, dev_id);
		}

		if (icon_name.length()) {
		    QPixmap icon = icon_loader.loadIcon(icon_name,
		                                        KIcon::User);
		    item->setPixmap(0, icon);
		}

		// use the current item as parent for the next pass
		parent = item;
	    }
	}
    } else {
	// combo box mode
	cbDevice->insertStringList(devices);
	cbDevice->show();
	listDevices->setEnabled(false);

	if (devices.contains(current_device)) {
	    // current device is in the list
	    cbDevice->setCurrentText(current_device);
	} else {
	    if (cbDevice->editable()) {
		// user defined device name
		cbDevice->setEditText(current_device);
	    } else if (devices.count()) {
		// one or more other possibilities -> take the first one
		cbDevice->setCurrentItem(0);
	    } else {
		// empty list of possibilities
		cbDevice->clearEdit();
		cbDevice->clear();
	    }
	}
	cbDevice->setEnabled(devices.count() > 1);
    }

    // enable changes in the device controls again
    m_enable_setDevice = true;
}

//***************************************************************************
void RecordDialog::listEntrySelected(QListViewItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(listDevices);
    if (!item || !listDevices) return;

    if (m_devices_list_map.contains(item))
	setDevice(m_devices_list_map[item]);
}

//***************************************************************************
void RecordDialog::setDevice(const QString &device)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;

    bool device_changed = (device != m_params.device_name);
    if (!device_changed) return;
    m_params.device_name = device;
//     qDebug("RecordDialog::setDevice(%s)", device.local8Bit().data());

    if (listDevices->isEnabled()) {
	// treeview mode
	QListViewItemIterator it(listDevices);
	while (it.current()) {
	    QListViewItem *node = it.current();
	    if (m_devices_list_map.contains(node)) {
		if (m_devices_list_map[node] == device) {
		    listDevices->setSelected(node, true);
		}
	    }
	    ++it;
	}
    } else if (cbDevice->editable() && device.length()) {
	// user defined device name
	if (cbDevice->currentText() != device) {
	    cbDevice->setCurrentText(device);
	    cbDevice->setEditText(device);
	}
    } else {
	// just take one from the list
	if (cbDevice->listBox()->findItem(device)) {
	    cbDevice->setCurrentText(device);
	} else if (cbDevice->count()) {
	    cbDevice->setCurrentItem(0);
	}
    }

    if (device_changed) emit sigDeviceChanged(device);
}

//***************************************************************************
void RecordDialog::sourceBufferCountChanged(int value)
{
    Q_ASSERT(value >=  4);
    Q_ASSERT(value <= 64);
    if (value < 4)  value = 4;
    if (value > 64) value = 64;

    // take the value into our struct
    m_params.buffer_count = value;
    emit sigBuffersChanged();
}

//***************************************************************************
void RecordDialog::sourceBufferSizeChanged(int value)
{
    Q_ASSERT(value >= 10);
    Q_ASSERT(value <= 18);
    if (value < 10) value = 10;
    if (value > 18) value = 18;

    // take the value into our struct
    m_params.buffer_size = value;

    // update the text
    unsigned int buffer_size = (1 << value);
    QString text;
    text = i18n("%1 samples");
    txtSourceBuffer->setText(text.arg(buffer_size));

    emit sigBuffersChanged();
}

//***************************************************************************
void RecordDialog::setFileFilter(const QString &filter)
{
    m_file_filter = filter;
    if (btSourceSelect) btSourceSelect->setEnabled(m_file_filter.length());
}

//***************************************************************************
void RecordDialog::selectRecordDevice()
{
    if (!m_enable_setDevice) return;

    QString filter;
    filter += QString("dsp*|") + i18n("OSS record device (dsp*)");
    filter += QString("\nadsp*|") + i18n("ALSA record device (adsp*)");
    filter += QString("\n*|") + i18n("Any device (*)");

    KwaveFileDialog dlg(":<kwave_record_device>", filter, this,
	"Kwave select record device", true, "file:/dev");
    dlg.setKeepLocation(true);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Select Record Device"));
    if (m_params.device_name[0] != '[')
        dlg.setURL("file:"+m_params.device_name);
    else
        dlg.setURL("file:/dev/*");
    if (dlg.exec() != QDialog::Accepted) return;

    // selected new device
    QString new_device = dlg.selectedFile();
    if (new_device != m_params.device_name) emit sigDeviceChanged(new_device);
}

//***************************************************************************
QString RecordDialog::rate2string(double rate) const
{
    const KLocale *locale = KGlobal::locale();
    Q_ASSERT(locale);

    QString s;
    if (!locale) return s.setNum(rate,'f', 3);

    const QString dot  = locale->decimalSymbol();
    const QString tsep = locale->thousandsSeparator();

    // format number with 3 digits
    s = locale->formatNumber(rate, 3);

    // remove thousands separator (looks ugly)
    s.remove(tsep);

    // remove trailing zeroes
    while (s.endsWith("0")) s.remove(s.length()-1, 1);

    // remove decimal point if necessary
    if (s.endsWith(dot)) s.remove(s.length()-1, 1);

    return s;
}

//***************************************************************************
double RecordDialog::string2rate(const QString &rate) const
{
    const KLocale *locale = KGlobal::locale();
    Q_ASSERT(locale);

    const QString s = rate;
    if (!locale) return s.toDouble();

    double r;
    bool ok;
    r = locale->readNumber(rate, &ok);
    Q_ASSERT(ok);
    if (!ok) return s.toDouble();

    return r;
}

//***************************************************************************
void RecordDialog::setSupportedTracks(unsigned int min, unsigned int max)
{
    Q_ASSERT(sbFormatTracks);
    if (!sbFormatTracks) return;

    if ((min == max) || (!max)) {
	sbFormatTracks->setEnabled(false);
	return;
    } else
	sbFormatTracks->setEnabled(true);

    if (sbFormatTracks->value() < sbFormatTracks->minValue()) {
	sbFormatTracks->setMaxValue(max);
	sbFormatTracks->setMinValue(min);
    } else {
	sbFormatTracks->setMinValue(min);
	sbFormatTracks->setMaxValue(max);
    }

}

//***************************************************************************
void RecordDialog::setTracks(unsigned int tracks)
{
//     qDebug("+++ RecordDialog::setTracks(%u)", tracks);
    Q_ASSERT(sbFormatTracks);
    if (!sbFormatTracks) return;
    if (!tracks) return;

    m_params.tracks = tracks;

    switch (tracks) {
	case 1:
	    lblTracksVerbose->setText(i18n("(Mono)"));
	    break;
	case 2:
	    lblTracksVerbose->setText(i18n("(Stereo)"));
	    break;
	case 4:
	    lblTracksVerbose->setText(i18n("(Quadro)"));
	    break;
	default:
	    lblTracksVerbose->setText("");
    }

    sbFormatTracks->setValue(tracks);
}

//***************************************************************************
void RecordDialog::tracksChanged(int tracks)
{
    if (tracks < 1) return; // no device
    if (tracks == (int)m_params.tracks) return;

    m_params.tracks = tracks;
    emit sigTracksChanged(tracks);
}

//***************************************************************************
void RecordDialog::setSupportedSampleRates(const QValueList<double> &rates)
{
    Q_ASSERT(cbFormatSampleRate);
    if (!cbFormatSampleRate) return;

    cbFormatSampleRate->clearEdit();
    cbFormatSampleRate->setEditable(false);
    cbFormatSampleRate->clear();

    QValueList<double>::ConstIterator it;
    for (it=rates.begin(); it != rates.end(); ++it) {
	QString rate = rate2string(*it);
	Q_ASSERT(rate.length());
	if (!rate.length()) continue; // string was zero?
	cbFormatSampleRate->insertItem(rate);
    }

    bool have_choice = (cbFormatSampleRate->count() > 1);
    cbFormatSampleRate->setEnabled(have_choice);
}

//***************************************************************************
void RecordDialog::setSampleRate(double new_rate)
{
    Q_ASSERT(cbFormatSampleRate);
    if (!cbFormatSampleRate) return;

    if (new_rate <= 0) {
	cbFormatSampleRate->setEnabled(false);
	return;
    } else {
	bool have_choice = (cbFormatSampleRate->count() > 1);
	cbFormatSampleRate->setEnabled(have_choice);
	m_params.sample_rate = new_rate;
    }

    QString rate;
    rate = rate2string(new_rate);
    cbFormatSampleRate->setCurrentItem(rate, true);
}

//***************************************************************************
void RecordDialog::sampleRateChanged(const QString &rate)
{
    if (!rate.length()) return; // no rate selected, combo box clear
    double sample_rate = string2rate(rate);
    if (sample_rate == m_params.sample_rate) return;

    m_params.sample_rate = sample_rate;
    emit sampleRateChanged(sample_rate);
}

//***************************************************************************
void RecordDialog::setSupportedCompressions(const QValueList<int> &comps)
{
    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

    cbFormatCompression->clear();
    CompressionType types;

    QValueList<int>::ConstIterator it;
    if (comps.isEmpty()) {
	// no compressions -> add "none" manually
	cbFormatCompression->insertItem(types.name(0));
    }

    for (it=comps.begin(); it != comps.end(); ++it) {
	int index = types.findFromData(*it);
	cbFormatCompression->insertItem(types.name(index));
    }

    bool have_choice = (cbFormatCompression->count() > 1);
    cbFormatCompression->setEnabled(have_choice);
}

//***************************************************************************
void RecordDialog::setCompression(int compression)
{
    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

    if (compression < 0) {
	cbFormatCompression->setEnabled(false);
	return;
    } else {
	bool have_choice = (cbFormatCompression->count() > 1);
	cbFormatCompression->setEnabled(have_choice);
	m_params.compression = compression;
    }

    CompressionType types;
    int index = types.findFromData(compression);
    cbFormatCompression->setCurrentItem(types.name(index), true);
}

//***************************************************************************
void RecordDialog::compressionChanged(const QString &name)
{
    CompressionType types;
    int index = types.findFromName(name);
    int compression = types.data(index);
    if (compression == m_params.compression) return;

    emit sigCompressionChanged(compression);
}

//***************************************************************************
void RecordDialog::setSupportedBits(const QValueList<unsigned int> &bits)
{
    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    m_supported_resolutions = bits;
    if (bits.count()) {
	sbFormatResolution->setMinValue(bits.first());
	sbFormatResolution->setMaxValue(bits.last());
    }

    // enable only if there is a choice
    sbFormatResolution->setEnabled(bits.count() > 1);
}

//***************************************************************************
void RecordDialog::setBitsPerSample(unsigned int bits)
{
    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    if (!bits ) {
	sbFormatResolution->setEnabled(false);
	return;
    } else {
	sbFormatResolution->setEnabled(m_supported_resolutions.count());
	m_params.bits_per_sample = bits;
    }

    sbFormatResolution->setValue(bits);
}

//***************************************************************************
void RecordDialog::bitsPerSampleChanged(int bits)
{
    if (bits < 1) return; // no device
    int last = m_params.bits_per_sample;
    if (bits == last) return;

    // round up or down to the next supported resolution in bits per sample
    if (!m_supported_resolutions.isEmpty()) {
	if (bits > last) {
	    // step up to the next supported value
	    QValueList<unsigned int>::Iterator it;
	    it=m_supported_resolutions.begin();
	    while (it != m_supported_resolutions.end()) {
		bits = *(it++);
		if (bits > last) break;
	    }
	    if (bits < last) bits = m_supported_resolutions.last();
	} else {
	    // step down to the next supported value
	    QValueList<unsigned int>::Iterator it;
	    it=m_supported_resolutions.end();
	    while (it != m_supported_resolutions.begin()) {
		bits = *(--it);
		if (bits < last) break;
	    }
	    if (bits > last) bits = m_supported_resolutions.first();
	}
    }

    m_params.bits_per_sample = bits;

    if (sbFormatResolution && (bits != sbFormatResolution->value()))
        sbFormatResolution->setValue(bits);

    emit sigBitsPerSampleChanged(bits);
}

//***************************************************************************
void RecordDialog::setSupportedSampleFormats(
    const QValueList<SampleFormat> &formats)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    cbFormatSampleFormat->clear();
    SampleFormat::Map types;

    QValueList<SampleFormat>::ConstIterator it;
    for (it=formats.begin(); it != formats.end(); ++it) {
	int index = types.findFromData(*it);
	cbFormatSampleFormat->insertItem(types.name(index));
    }

    bool have_choice = (cbFormatSampleFormat->count() > 1);
    cbFormatSampleFormat->setEnabled(have_choice);
}

//***************************************************************************
void RecordDialog::setSampleFormat(SampleFormat sample_format)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    if (sample_format == SampleFormat::Unknown) {
	cbFormatSampleFormat->setEnabled(false);
	return;
    } else {
	bool have_choice = (cbFormatSampleFormat->count() > 1);
	cbFormatSampleFormat->setEnabled(have_choice);
	m_params.sample_format = sample_format;
    }

    SampleFormat::Map types;
    int index = types.findFromData(sample_format);
    cbFormatSampleFormat->setCurrentItem(
        (sample_format != -1) ? types.name(index) : "", true);
}

//***************************************************************************
void RecordDialog::sampleFormatChanged(const QString &name)
{
    SampleFormat::Map types;
    int index = types.findFromName(name);
    SampleFormat format = types.data(index);
    if (format == m_params.sample_format) return;

    emit sigSampleFormatChanged(format);
}

//***************************************************************************
void RecordDialog::setState(RecordState state)
{
    bool enable_new = false;
    bool enable_pause = false;
    bool enable_stop = false;
    bool enable_record = false;
    bool enable_settings = false;
    bool enable_trigger = false;

    m_state = state;
    switch (state) {
	case REC_EMPTY:
	    lbl_state->setText(i18n("(empty)"));
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = m_params.device_name.length();
	    enable_settings = true;
	    enable_trigger  = true;
	    break;
	case REC_BUFFERING:
	    lbl_state->setText(i18n("buffering..."));
	    enable_new      = true; /* throw away current FIFO content */
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    enable_trigger  = true;
	    break;
	case REC_PRERECORDING:
	    lbl_state->setText(i18n("prerecording..."));
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true;
	    enable_settings = false;
	    enable_trigger  = true;
	    break;
	case REC_WAITING_FOR_TRIGGER:
	    lbl_state->setText(i18n("waiting for trigger..."));
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    enable_trigger  = true;
	    break;
	case REC_RECORDING:
	    lbl_state->setText(i18n("recording..."));
	    enable_new      = false;
	    enable_pause    = true;
	    enable_stop     = true;
	    enable_record   = false;
	    enable_settings = false;
	    enable_trigger  = false;
	    break;
	case REC_PAUSED:
	    lbl_state->setText(i18n("paused"));
	    enable_new      = true; /* start again */
	    enable_pause    = true; /* used for "continue" */
	    enable_stop     = true;
	    enable_record   = true; /* used for "continue" */
	    enable_settings = false;
	    enable_trigger  = false;
	    break;
	case REC_DONE:
	    lbl_state->setText(i18n("done."));
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = true;
	    enable_settings = true;
	    enable_trigger  = true;
	    break;
    }

    // enable/disable the record control buttons
    btNew->setEnabled(enable_new);
    btPause->setEnabled(enable_pause);
    btStop->setEnabled(enable_stop);
    m_record_enabled = enable_record;
    updateRecordButton();

    // enable disable all controls (groups) for setup
    chkRecordPre->setEnabled(enable_settings);
    sbRecordPre->setEnabled(enable_settings &&
                            chkRecordPre->isChecked());
    slRecordPre->setEnabled(enable_settings &&
                            chkRecordPre->isChecked());

    chkRecordTime->setEnabled(enable_settings);
    sbRecordTime->setEnabled(enable_settings &&
                             chkRecordTime->isChecked());
    chkRecordTrigger->setEnabled(enable_settings);

    // it is not really necessary to disable these ;-)
    sbRecordTrigger->setEnabled(enable_trigger &&
                                chkRecordTrigger->isChecked());
    slRecordTrigger->setEnabled(enable_trigger &&
                                chkRecordTrigger->isChecked());

    grpFormat->setEnabled(enable_settings);
    grpSource->setEnabled(enable_settings);

}

//***************************************************************************
void RecordDialog::updateBufferState(unsigned int count, unsigned int total)
{
    Q_ASSERT(progress_bar);
    if (!progress_bar) return;

    if (total == 0) {
	// we are done: stop update timer and reset buffer percentage
	m_buffer_progress_timer.stop();
	progress_bar->setPercentageVisible(false);
	progress_bar->setTotalSteps(1);
	progress_bar->setProgress(0);
    } else {
	if (!m_buffer_progress_timer.isActive()) {
	    m_buffer_progress_count = count;
	    m_buffer_progress_total = total;
	    m_buffer_progress_timer.start(300, true);
	} else {
	    m_buffer_progress_count += count;
	    m_buffer_progress_total += total;
	}
    }

}

//***************************************************************************
void RecordDialog::preRecordingChecked(bool enabled)
{
    m_params.pre_record_enabled = enabled;
    emit sigPreRecordingChanged(enabled);
}

//***************************************************************************
void RecordDialog::preRecordingTimeChanged(int time)
{
    m_params.pre_record_time = time;
}

//***************************************************************************
void RecordDialog::recordTimeChecked(bool limited)
{
    m_params.record_time_limited = limited;
    emit sigRecordTimeChanged(limited ? sbRecordTime->value() : -1);
}

//***************************************************************************
void RecordDialog::recordTimeChanged(int limit)
{
    m_params.record_time = limit;
    emit sigRecordTimeChanged(chkRecordTime->isChecked() ?
                              limit : -1);
    updateRecordButton();
}

//***************************************************************************
void RecordDialog::triggerChecked(bool enabled)
{
    m_params.record_trigger_enabled = enabled;
}

//***************************************************************************
void RecordDialog::triggerChanged(int trigger)
{
    m_params.record_trigger = trigger;
}

//***************************************************************************
void RecordDialog::displayLevelMeterChecked(bool enabled)
{
    m_params.display_level_meter = enabled;

    Q_ASSERT(level_meter);
    if (!level_meter) return;
    if (!enabled) {
	level_meter->setTracks(0);
	level_meter->reset();
    }
    level_meter->setEnabled(enabled);
}

//***************************************************************************
void RecordDialog::updateBufferProgressBar()
{
//     qDebug("RecordDialog::updateBufferProgressBar(): %u/%u",
//            m_buffer_progress_count, m_buffer_progress_total);

    progress_bar->setPercentageVisible(true);
    progress_bar->setTotalSteps(m_buffer_progress_total);
    progress_bar->setProgress(m_buffer_progress_count);
}

//***************************************************************************
void RecordDialog::updateEffects(unsigned int track,
                                 QMemArray<sample_t> &buffer)
{
    if (!buffer.size()) return;

    if (m_params.display_level_meter && level_meter) {
	level_meter->setTracks(m_params.tracks);
	level_meter->setSampleRate(m_params.sample_rate);
	level_meter->updateTrack(track, buffer);
    }

}

//***************************************************************************
void RecordDialog::setRecordedSamples(unsigned int samples_recorded)
{
    // if (!m_params.record_time_limited) return; // not of interest
    m_samples_recorded = samples_recorded;
    updateRecordButton();
}

//***************************************************************************
void RecordDialog::updateRecordButton()
{
    bool old_enable = btRecord->isEnabled();
    bool new_enable;

    // enabled if not disabled by status and also not limited or
    // less than the limit has been recorded
    new_enable = m_record_enabled && (!m_params.record_time_limited ||
        (m_samples_recorded < m_params.record_time * m_params.sample_rate));

    if (new_enable != old_enable) btRecord->setEnabled(new_enable);
}

//***************************************************************************
void RecordDialog::invokeHelp()
{
    kapp->invokeHelp("recording");
}

//***************************************************************************
//***************************************************************************
