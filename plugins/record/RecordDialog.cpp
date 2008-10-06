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

#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QTreeView>
#include <QPixmap>
#include <QProgressBar>
#include <QSlider>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStatusBar>

#include <kcombobox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include <kstatusbar.h>
#include <ktoolinvocation.h>

#include "libkwave/CompressionType.h"
#include "libkwave/KwavePlugin.h" // for some helper functions
#include "libkwave/SampleFormat.h"
#include "libgui/HMSTimeWidget.h"
#include "libgui/KwaveFileDialog.h"

#include "LevelMeter.h"
#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordParams.h"
#include "RecordState.h"

#include "record_stop.xpm"
#include "record_pause.xpm"
#include "krec_record.xpm"

// status bar icons
#include "stop_hand.xpm"
#include "ok.xpm"
#include "ledred.xpm"
#include "ledgreen.xpm"
#include "ledlightgreen.xpm"
#include "ledyellow.xpm"
#include "walk_r1.xpm"
#include "walk_r2.xpm"
#include "walk_r3.xpm"
#include "walk_r4.xpm"
#include "walk_r5.xpm"
#include "walk_r6.xpm"
#include "walk_r7.xpm"
#include "walk_r8.xpm"

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


enum {
    ID_ICON=1,          /**< status bar id: state icon */
    ID_STATE,           /**< status bar id: state text */
    ID_TIME,            /**< status bar id: recorded time */
    ID_SAMPLE_RATE,     /**< status bar id: sample rate */
    ID_BITS_PER_SAMPLE, /**< status bar id: number of tracks */
    ID_TRACKS           /**< status bar id: number of tracks */
};

//***************************************************************************
RecordDialog::RecordDialog(QWidget *parent, QStringList &params,
                           RecordController *controller)
    :QDialog(parent), Ui::RecordDlg(), m_methods_map(),
     m_file_filter(), m_devices_list_map(),
     m_state(REC_EMPTY), m_params(),
     m_supported_resolutions(), m_buffer_progress_count(0),
     m_buffer_progress_total(0), m_buffer_progress_timer(this),
     m_record_enabled(true), m_samples_recorded(0),
     m_enable_setDevice(true), m_state_icon_widget(0)
{
    setupUi(this);

    /* get initial parameters */
    m_params.fromList(params);

    /* set the icons of the record control buttons */
    KIconLoader icon_loader;
    btNew->setIcon(   KIcon(icon_loader.loadIcon(
	              "document-new", KIconLoader::Toolbar)));
    btStop->setIcon(  KIcon(QPixmap(xpm_stop)));
    btPause->setIcon( KIcon(QPixmap(xpm_pause)));
    btRecord->setIcon(KIcon(QPixmap(xpm_krec_record)));

    // fill the combo box with playback methods
    unsigned int index=0;
    for (index=0; index < m_methods_map.count(); index++) {
	cbMethod->addItem(m_methods_map.description(index, true));
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
    displayLevelMeterChecked(m_params.display_level_meter);
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
    connect(listDevices,
            SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
            SLOT(listEntrySelected(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(listDevices, SIGNAL(itemExpanded(QTreeWidgetItem *)),
            SLOT(listItemExpanded(QTreeWidgetItem *)));
    connect(listDevices, SIGNAL(focusLost()),
            SLOT(updateListSelection()));

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

    // status bar
    m_state_icon_widget = new StatusWidget(this);
    Q_ASSERT(m_state_icon_widget);
    if (!m_state_icon_widget) return;

    m_state_icon_widget->setFixedSize(16, 16);
    lbl_state->addWidget(m_state_icon_widget);

    lbl_state->insertItem(" ", ID_STATE, 100);
    lbl_state->setItemAlignment(ID_STATE,
                                Qt::AlignLeft | Qt::AlignVCenter);
    lbl_state->insertItem(" ", ID_TIME);
    lbl_state->setItemAlignment(ID_TIME,
                                Qt::AlignLeft | Qt::AlignVCenter);
    lbl_state->insertItem(" ", ID_SAMPLE_RATE);
    lbl_state->setItemAlignment(ID_SAMPLE_RATE,
                                Qt::AlignRight | Qt::AlignVCenter);
    lbl_state->insertItem(" ", ID_BITS_PER_SAMPLE);
    lbl_state->setItemAlignment(ID_BITS_PER_SAMPLE,
                                Qt::AlignRight | Qt::AlignVCenter);
    lbl_state->insertItem(" ", ID_TRACKS);
    lbl_state->setItemAlignment(ID_TRACKS,
                                Qt::AlignRight | Qt::AlignVCenter);

    m_state_icon_widget->setFixedSize(16, lbl_state->childrenRect().height());

    // set the initial state of the dialog to "Reset/Empty"
    setState(REC_EMPTY);

    // disable the "level" tab, it is not implemented yet
    tabRecord->setCurrentIndex(1);
    QWidget *page = tabRecord->currentWidget();
    tabRecord->setCurrentIndex(0);
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
    cbMethod->setCurrentIndex(m_methods_map.findFromData(
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

    cbDevice->clearEditText();
    cbDevice->clear();
    listDevices->clear();

    if (devices.contains("#EDIT#")) {
	devices.removeAll("#EDIT#");
	cbDevice->setEditable(true);
    } else {
	cbDevice->setEditable(false);
    }

    if (devices.contains("#SELECT#")) {
	devices.removeAll("#SELECT#");
	btSourceSelect->setEnabled(true);
	btSourceSelect->show();
    } else {
	btSourceSelect->setEnabled(false);
	btSourceSelect->hide();
    }

    if (devices.contains("#TREE#")) {
	// treeview mode
	devices.removeAll("#TREE#");
	listDevices->setEnabled(true);
	cbDevice->setEnabled(false);
	cbDevice->hide();
	m_devices_list_map.clear();

	// build a tree with all nodes in the list
	foreach (QString dev_id, devices) {
	    QTreeWidgetItem *parent = 0;

	    QStringList list = dev_id.split("||", QString::KeepEmptyParts);
	    foreach (QString token, list) {
		QTreeWidgetItem *item = 0;

		// split the icon name from the token
		QString icon_name;
		int pos = token.indexOf('|');
		if (pos > 0) {
		    icon_name = token.mid(pos+1);
		    token     = token.left(pos);
		}

		// find the first item with the same text
		// and the same root
		if (parent) {
		    for (int i=0; i < parent->childCount(); i++) {
			QTreeWidgetItem *node = parent->child(i);
			if (node && node->text(0) == token) {
			    item = node;
			    break;
			}
		    }
		} else {
		    QList<QTreeWidgetItem *> matches =
			listDevices->findItems(token, Qt::MatchExactly);
		    if (matches.count())
			item = matches.takeFirst();
		}

		if (item) {
		    // already in the list
		    parent = item;
		} else if (parent) {
		    // new leaf, add to the parent
		    item = new QTreeWidgetItem(parent);
		    Q_ASSERT(item);
		    if (item) {
			item->setText(0, token);
			m_devices_list_map.insert(item, dev_id);
		    }

		    parent->setExpanded(true);
		    parent->setFlags(parent->flags() &
			~(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable));
		    if (m_devices_list_map.contains(parent)) {
			// make the parent not selectable
			m_devices_list_map.remove(parent);
		    }
		} else {
		    // new root node
		    item = new QTreeWidgetItem(listDevices);
		    Q_ASSERT(item);
		    if (item) {
			item->setText(0, token);
			m_devices_list_map.insert(item, dev_id);
		    }
		}

		if (icon_name.length()) {
		    QIcon icon = icon_loader.loadIcon(
			icon_name, KIconLoader::User);
		    item->setIcon(0, icon);
		}

		// use the current item as parent for the next pass
		parent = item;
	    }
	}
    } else {
	// combo box mode
	cbDevice->addItems(devices);
	cbDevice->show();
	listDevices->setEnabled(false);

	if (devices.contains(current_device)) {
	    // current device is in the list
	    cbDevice->setCurrentIndex(cbDevice->findText(current_device));
	} else {
	    if (cbDevice->isEditable() && current_device.length()) {
		// user defined device name
		cbDevice->setEditText(current_device);
	    } else if (devices.count()) {
		// one or more other possibilities -> take the first one
		cbDevice->setCurrentIndex(0);
	    } else {
		// empty list of possibilities
		cbDevice->clearEditText();
		cbDevice->clear();
	    }
	}
	cbDevice->setEnabled(devices.count() > 1);
    }

    // enable changes in the device controls again
    m_enable_setDevice = true;
}

//***************************************************************************
void RecordDialog::listEntrySelected(QTreeWidgetItem *current,
                                       QTreeWidgetItem *previous)
{
    Q_ASSERT(listDevices);
    Q_UNUSED(previous);
    if (!current || !listDevices) return;

    if (m_devices_list_map.contains(current))
	setDevice(m_devices_list_map[current]);
}

//***************************************************************************
void RecordDialog::listItemExpanded(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    updateListSelection();
}

//***************************************************************************
void RecordDialog::updateListSelection()
{
    // set the current device again, otherwise nothing will be selected
    setDevice(m_params.device_name);
}

//***************************************************************************
void RecordDialog::setDevice(const QString &device)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;

    bool device_changed = (device != m_params.device_name);
    m_params.device_name = device;
//     qDebug("RecordDialog::setDevice(%s)", device.local8Bit().data());

    if (listDevices->isEnabled()) {
	// treeview mode
	QTreeWidgetItem *node = m_devices_list_map.key(device, 0);
	if (node) {
	    node->setSelected(true);
	    listDevices->scrollToItem(node);
	    listDevices->setCurrentItem(node);
	}
    } else if (cbDevice->isEditable() && device.length()) {
	// user defined device name
	if (!device.isEmpty() && (cbDevice->currentText() != device)) {
	    cbDevice->setCurrentIndex(cbDevice->findText(device));
	    cbDevice->setEditText(device);
	}
    } else {
	// just take one from the list
	if (cbDevice->findText(device) >= 0) {
	    cbDevice->setCurrentIndex(cbDevice->findText(device));
	} else if (cbDevice->count()) {
	    cbDevice->setCurrentIndex(0);
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
    txtSourceBuffer->setText(i18n("%1 samples", buffer_size));

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
	true, "file:/dev");
    dlg.setKeepLocation(true);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Select Record Device"));
    if (m_params.device_name[0] != '[')
        dlg.setUrl(KUrl("file:"+m_params.device_name));
    else
        dlg.setUrl(KUrl("file:/dev/*"));
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

    if (sbFormatTracks->value() < sbFormatTracks->minimum()) {
	sbFormatTracks->setMaximum(max);
	sbFormatTracks->setMinimum(min);
    } else {
	sbFormatTracks->setMinimum(min);
	sbFormatTracks->setMaximum(max);
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
    QString tracks_str;

    switch (tracks) {
	case 1:
	    tracks_str = i18n("Mono");
	    break;
	case 2:
	    tracks_str = i18n("Stereo");
	    break;
	case 4:
	    tracks_str = i18n("Quadro");
	    break;
	default:
	    tracks_str = "";
    }

    if (tracks_str.length()) {
	lblTracksVerbose->setText("("+tracks_str+")");
	lbl_state->changeItem(tracks_str, ID_TRACKS);
    } else {
	lblTracksVerbose->setText("");
	lbl_state->changeItem(i18n("%1 tracks", tracks), ID_TRACKS);
    }

    sbFormatTracks->setValue(tracks);
}

//***************************************************************************
void RecordDialog::tracksChanged(int tracks)
{
    if (tracks < 1) return; // no device
    if (tracks == static_cast<int>(m_params.tracks)) return;

    m_params.tracks = tracks;
    emit sigTracksChanged(tracks);
}

//***************************************************************************
void RecordDialog::setSupportedSampleRates(const QList<double> &rates)
{
    Q_ASSERT(cbFormatSampleRate);
    if (!cbFormatSampleRate) return;

    cbFormatSampleRate->clearEditText();
    cbFormatSampleRate->setEditable(false);
    cbFormatSampleRate->clear();

    foreach (double r, rates) {
	QString rate = rate2string(r);
	Q_ASSERT(rate.length());
	if (!rate.length()) continue; // string was zero?
	cbFormatSampleRate->addItem(rate);
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
    lbl_state->changeItem(i18n("%1 Hz", rate), ID_SAMPLE_RATE);
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
void RecordDialog::setSupportedCompressions(const QList<int> &comps)
{
    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

    cbFormatCompression->clear();
    CompressionType types;

    if (comps.isEmpty()) {
	// no compressions -> add "none" manually
	cbFormatCompression->addItem(types.name(0));
    } else {
	foreach (int c, comps) {
	    int index = types.findFromData(c);
	    cbFormatCompression->addItem(types.name(index));
	}
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
void RecordDialog::setSupportedBits(const QList<unsigned int> &bits)
{
    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    m_supported_resolutions = bits;
    if (bits.count()) {
	sbFormatResolution->setMinimum(bits.first());
	sbFormatResolution->setMaximum(bits.last());
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
	sbFormatResolution->setEnabled(m_supported_resolutions.count() > 1);
	m_params.bits_per_sample = bits;
    }

    lbl_state->changeItem(i18n("%1 bit", bits), ID_BITS_PER_SAMPLE);
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
	    QListIterator<unsigned int> it(m_supported_resolutions);
	    while (it.hasNext()) {
		bits = it.next();
		if (bits > last) break;
	    }
	    if (bits < last) bits = m_supported_resolutions.last();
	} else {
	    // step down to the next supported value
	    QListIterator<unsigned int> it(m_supported_resolutions);
	    it.toBack();
	    while (it.hasPrevious()) {
		bits = it.previous();
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
    const QList<SampleFormat> &formats)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    cbFormatSampleFormat->clear();
    SampleFormat::Map types;

    foreach (SampleFormat format, formats) {
	int index = types.findFromData(format);
	cbFormatSampleFormat->addItem(types.name(index));
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
    QString state_text = "";
    QVector<QPixmap> pixmaps;
    unsigned int animation_time = 500;

    m_state = state;
    switch (state) {
	case REC_UNINITIALIZED:
	    state_text = i18n("please check the source device settings...");
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = false;
	    enable_settings = true;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(stop_hand_xpm));
	    pixmaps.push_back(QPixmap(ledred_xpm));
	    lbl_state->changeItem("", ID_TIME);
	    break;
	case REC_EMPTY:
	    state_text = i18n("(empty)");
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = m_params.device_name.length();
	    enable_settings = true;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(ledgreen_xpm));
	    lbl_state->changeItem("", ID_TIME);
	    break;
	case REC_BUFFERING:
	    state_text = i18n("buffering...");
	    enable_new      = true; /* throw away current FIFO content */
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(ledgreen_xpm));
	    pixmaps.push_back(QPixmap(ledlightgreen_xpm));
	    break;
	case REC_PRERECORDING:
	    state_text = i18n("prerecording...");
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true;
	    enable_settings = false;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(ledgreen_xpm));
	    pixmaps.push_back(QPixmap(ledlightgreen_xpm));
	    break;
	case REC_WAITING_FOR_TRIGGER:
	    state_text = i18n("waiting for trigger...");
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(ledgreen_xpm));
	    pixmaps.push_back(QPixmap(ledlightgreen_xpm));
	    break;
	case REC_RECORDING:
	    state_text = i18n("recording...");
	    enable_new      = false;
	    enable_pause    = true;
	    enable_stop     = true;
	    enable_record   = false;
	    enable_settings = false;
	    enable_trigger  = false;
	    pixmaps.push_back(QPixmap(walk_r1_xpm));
	    pixmaps.push_back(QPixmap(walk_r2_xpm));
	    pixmaps.push_back(QPixmap(walk_r3_xpm));
	    pixmaps.push_back(QPixmap(walk_r4_xpm));
	    pixmaps.push_back(QPixmap(walk_r5_xpm));
	    pixmaps.push_back(QPixmap(walk_r6_xpm));
	    pixmaps.push_back(QPixmap(walk_r7_xpm));
	    pixmaps.push_back(QPixmap(walk_r8_xpm));
	    animation_time = 100;
	    break;
	case REC_PAUSED:
	    state_text = i18n("paused");
	    enable_new      = true; /* start again */
	    enable_pause    = true; /* used for "continue" */
	    enable_stop     = true;
	    enable_record   = true; /* used for "continue" */
	    enable_settings = false;
	    enable_trigger  = false;
	    pixmaps.push_back(QPixmap(ledgreen_xpm));
	    pixmaps.push_back(QPixmap(ledyellow_xpm));
	    break;
	case REC_DONE:
	    state_text = i18n("done");
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = true;
	    enable_settings = true;
	    enable_trigger  = true;
	    pixmaps.push_back(QPixmap(ok_xpm));
	    break;
    }
    lbl_state->changeItem(state_text, ID_STATE);
    m_state_icon_widget->setPixmaps(pixmaps, animation_time);

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
	m_buffer_progress_count = 0;
	m_buffer_progress_total = 0;
	progress_bar->setTextVisible(false);
	progress_bar->setMinimum(0);
	progress_bar->setMaximum(100);
	progress_bar->setValue(0);
	progress_bar->reset();
    } else {
	m_buffer_progress_count = count;
	m_buffer_progress_total = total;

	if (!m_buffer_progress_timer.isActive())
	    updateBufferProgressBar();
    }

    // update recording time
    QString txt;
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    txt = "";
	    break;
	case REC_RECORDING:
	case REC_PAUSED:
	case REC_DONE: {
	    if (m_samples_recorded > 1) {
		double rate = m_params.sample_rate;
		double ms = (rate) ?
		    ((static_cast<double>(m_samples_recorded) / rate) * 1E3)
		    : 0;
		txt = " " + i18n("Length: %1", KwavePlugin::ms2string(ms)) +
		    " " + i18n("(%1 samples)",
		    KwavePlugin::dottedNumber(m_samples_recorded));
	    } else txt = "";
	    break;
	}
    }
    lbl_state->changeItem(txt, ID_TIME);
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
    emit sigTriggerChanged(enabled);
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
    Q_ASSERT(lbl_level_meter);
    if (!level_meter || !lbl_level_meter) return;
    if (enabled) {
	level_meter->show();
	lbl_level_meter->show();
    } else {
	level_meter->setTracks(0);
	level_meter->reset();
	level_meter->hide();
	lbl_level_meter->hide();
    }
}

//***************************************************************************
void RecordDialog::updateBufferProgressBar()
{
    unsigned int count = m_buffer_progress_count;
    unsigned int total = m_buffer_progress_total;
//     qDebug("RecordDialog::updateBufferProgressBar(): %u/%u",
//            count, total);

    /*
     * @note: QProgressBar has a bug when handling small numbers,
     *        therefore we multiply everything by 100
     */
    progress_bar->setTextVisible(true);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100 * total);
    progress_bar->setValue(100 * count);

    m_buffer_progress_timer.setSingleShot(true);
    m_buffer_progress_timer.setInterval(100);
    m_buffer_progress_timer.start();
}

//***************************************************************************
void RecordDialog::updateEffects(unsigned int track,
                                 Kwave::SampleArray &buffer)
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
    KToolInvocation::invokeHelp("recording");
}

//***************************************************************************
void RecordDialog::message(const QString &message)
{
    if (lbl_state) lbl_state->showMessage(message, 3000);
}

//***************************************************************************
void RecordDialog::showDevicePage()
{
    if (tabRecord) tabRecord->setCurrentIndex(2);
}

//***************************************************************************
#include "RecordDialog.moc"
//***************************************************************************
//***************************************************************************
