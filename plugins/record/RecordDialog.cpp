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
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qslider.h>

#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include "libkwave/CompressionType.h"
#include "libkwave/SampleFormat.h"
#include "libgui/HMSTimeWidget.h"
#include "libgui/KwaveFileDialog.h"

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

/** list pf well-known audio devices */
static const char *well_known_devices[] = {
#ifdef ARTS_CAN_RECORD
    "[aRts sound daemon]",
#endif
    "/dev/dsp",
    "/dev/dsp0",
    "/dev/dsp1",
    "/dev/dsp2",
    "/dev/audio",
    "/dev/adsp",
    "/dev/adsp0",
    "/dev/adsp1",
    "/dev/adsp2",
    "/dev/dio",
    0
};

//***************************************************************************
RecordDialog::RecordDialog(QWidget *parent, QStringList &params,
                           RecordController *controller)
    :RecordDlg(parent,0), m_state(REC_EMPTY), m_params(),
     m_supported_resolutions(), m_buffer_progress_count(0),
     m_buffer_progress_total(0), m_buffer_progress_timer(this)
{
    /* get initial parameters */
    m_params.fromList(params);

    /* set the icons of the record control buttons */
    btNew->setIconSet(   QIconSet(QPixmap(xpm_filenew)));
    btStop->setIconSet(  QIconSet(QPixmap(xpm_stop)));
    btPause->setIconSet( QIconSet(QPixmap(xpm_pause)));
    btRecord->setIconSet(QIconSet(QPixmap(xpm_krec_record)));

    /* --- set up all controls with their default/startup values --- */

    // pre-record
    STD_SETUP_SLIDER(pre_record_enabled, pre_record_time, RecordPre);

    // record time
    STD_SETUP_SLIDER(record_time_limited, record_time, RecordTime);

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

    // record buffer size and count
    slSourceBufferCount->setValue(m_params.buffer_count);
    slSourceBufferSize->setValue(m_params.buffer_size);
    connect(slSourceBufferSize, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferSizeChanged(int)));
    connect(slSourceBufferCount, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferCountChanged(int)));

    // device combo box
    cbSourceDevice->setEditable(true);
    cbSourceDevice->insertStrList(well_known_devices, -1);
    cbSourceDevice->setCurrentItem(m_params.device_name);
    cbSourceDevice->setEditText(m_params.device_name);
    cbSourceDevice->setMinimumWidth(cbSourceDevice->sizeHint().width()+10);

    // "select device..." button
    connect(btSourceSelect, SIGNAL(clicked()),
            this, SLOT(selectRecordDevice()));
/*    connect(cbSourceDevice, SIGNAL(activated(const QString &)),
            this, SLOT(forwardDeviceChanged(const QString &)));*/
    connect(cbSourceDevice, SIGNAL(textChanged(const QString &)),
            this, SLOT(forwardDeviceChanged(const QString &)));

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

    // set the initial state of the dialog to "Reset/Empty"
    setState(REC_EMPTY);
}

//***************************************************************************
RecordDialog::~RecordDialog()
{
    updateBufferState(0,0);
}

//***************************************************************************
const RecordParams &RecordDialog::params() const
{
    return m_params;
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
void RecordDialog::selectRecordDevice()
{
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
    if (new_device != m_params.device_name) emit deviceChanged(new_device);
}

//***************************************************************************
void RecordDialog::forwardDeviceChanged(const QString &dev)
{
    emit deviceChanged(dev);
}

//***************************************************************************
void RecordDialog::setDevice(const QString &dev)
{
    m_params.device_name = dev;
    cbSourceDevice->setEditText(dev);

    // update the state, maybe we changed from invalid to valid
    setState(m_state);
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
    m_params.tracks = tracks;

    Q_ASSERT(sbFormatTracks);
    if (!sbFormatTracks) return;

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
    m_params.sample_rate = new_rate;

    Q_ASSERT(cbFormatSampleRate);
    if (!cbFormatSampleRate) return;

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
    m_params.compression = compression;

    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

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
void RecordDialog::setSupportedBitsPerSample(
                   const QValueList<unsigned int> &bits)
{
    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    sbFormatResolution->setMinValue(bits.first());
    sbFormatResolution->setMaxValue(bits.last());
    m_supported_resolutions = bits;

    // enable only if there is a choice
    sbFormatResolution->setEnabled(bits.first() != bits.last());
}

//***************************************************************************
void RecordDialog::setBitsPerSample(unsigned int bits)
{
    m_params.bits_per_sample = bits;

    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    sbFormatResolution->setValue(bits);
}

//***************************************************************************
void RecordDialog::bitsPerSampleChanged(int bits)
{
    if (bits < 1) return; // no device
    if (bits == (int)m_params.bits_per_sample) return;

    // round up or down to the next supported resolution in bits per sample
    if (!m_supported_resolutions.isEmpty()) {
	unsigned int last = m_params.bits_per_sample;
	if (bits < (int)last) {
	    // step up to the next supported value
	    QValueList<unsigned int>::Iterator it;
	    for (it=m_supported_resolutions.begin();
	        (it != m_supported_resolutions.end());
		++it)
	    {
		bits = *it;
		if ((int)(*it) >= bits) break;
	    }
	} else {
	    // step down to the next supported value
	    QValueList<unsigned int>::Iterator it;
	    for (it=m_supported_resolutions.end();
	        (it != m_supported_resolutions.begin()); )
	    {
		--it;
		bits = *it;
		if ((int)(*it) <= bits) break;
	    }
	    if ((int)(*it) > bits) bits = m_supported_resolutions.first();
	}
    }

    m_params.bits_per_sample = bits;

    if (sbFormatResolution && (bits != sbFormatResolution->value()))
        sbFormatResolution->setValue(bits);

    emit sigBitsPerSampleChanged(bits);
}

//***************************************************************************
void RecordDialog::setSupportedSampleFormats(const QValueList<int> &formats)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    cbFormatSampleFormat->clear();
    SampleFormat types;

    QValueList<int>::ConstIterator it;
    for (it=formats.begin(); it != formats.end(); ++it) {
	int index = types.findFromData(*it);
	cbFormatSampleFormat->insertItem(types.name(index));
    }

    bool have_choice = (cbFormatSampleFormat->count() > 1);
    cbFormatSampleFormat->setEnabled(have_choice);
}

//***************************************************************************
void RecordDialog::setSampleFormat(int sample_format)
{
    m_params.sample_format = sample_format;

    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    SampleFormat types;
    int index = types.findFromData(sample_format);
    cbFormatSampleFormat->setCurrentItem(types.name(index), true);
}

//***************************************************************************
void RecordDialog::sampleFormatChanged(const QString &name)
{
    SampleFormat types;
    int index = types.findFromName(name);
    int format = types.data(index);
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

    m_state = state;
    switch (state) {
	case REC_EMPTY:
	    lbl_state->setText(i18n("(empty)"));
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = m_params.device_name.length();
	    enable_settings = true;
	    break;
	case REC_BUFFERING:
	    lbl_state->setText(i18n("buffering..."));
	    enable_new      = true; /* throw away current FIFO content */
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    break;
	case REC_PRERECORDING:
	    lbl_state->setText(i18n("prerecording..."));
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true;
	    enable_settings = false;
	    break;
	case REC_WAITING_FOR_TRIGGER:
	    lbl_state->setText(i18n("waiting for trigger..."));
	    enable_new      = false;
	    enable_pause    = false;
	    enable_stop     = true;
	    enable_record   = true; /* acts as "trigger now" */
	    enable_settings = false;
	    break;
	case REC_RECORDING:
	    lbl_state->setText(i18n("recording..."));
	    enable_new      = false;
	    enable_pause    = true;
	    enable_stop     = true;
	    enable_record   = false;
	    enable_settings = false;
	    break;
	case REC_PAUSED:
	    lbl_state->setText(i18n("paused"));
	    enable_new      = true; /* start again */
	    enable_pause    = true; /* used for "continue" */
	    enable_stop     = true;
	    enable_record   = true; /* used for "continue" */
	    enable_settings = false;
	    break;
	case REC_DONE:
	    lbl_state->setText(i18n("done."));
	    enable_new      = true;
	    enable_pause    = false;
	    enable_stop     = false;
	    enable_record   = true;
	    enable_settings = true;
	    break;
    }

    // enable/disable the record control buttons
    btNew->setEnabled(enable_new);
    btPause->setEnabled(enable_pause);
    btStop->setEnabled(enable_stop);
    btRecord->setEnabled(enable_record);

    // enable disable all controls (groups) for setup
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
void RecordDialog::updateBufferProgressBar()
{
//     qDebug("RecordDialog::updateBufferProgressBar(): %u/%u",
//            m_buffer_progress_count, m_buffer_progress_total);

    progress_bar->setPercentageVisible(true);
    progress_bar->setTotalSteps(m_buffer_progress_total);
    progress_bar->setProgress(m_buffer_progress_count);
}

//***************************************************************************
//***************************************************************************
