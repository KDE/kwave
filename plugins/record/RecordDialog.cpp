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
#include <qlabel.h>
#include <qpixmap.h>
#include <qslider.h>

#include <kcombobox.h>
#include <kpushbutton.h>
#include <knuminput.h>

#include "libgui/HMSTimeWidget.h"
#include "libgui/KwaveFileDialog.h"

#include "RecordDialog.h"
#include "RecordParams.h"

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
    "[aRts sound daemon]",
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
RecordDialog::RecordDialog(QWidget *parent, const RecordParams &params)
    :RecordDlg(parent,0), m_params(params)
{
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
    QString s;
    cbFormatSampleRate->setCurrentItem(s.setNum(m_params.sample_rate));
    sbFormatResolution->setValue(m_params.bits_per_sample);
    sbFormatTracks->setValue(m_params.tracks);

    // device name
    cbSourceDevice->setCurrentItem(m_params.device_name);

    // power of buffer size
    slSourceBuffer->setValue(m_params.buffer_size);
    sourceBufferChanged(m_params.buffer_size);

    // various displays: level meter, oscilloscope, FFT, Overview
    chkDisplayLevelMeter->setChecked(m_params.display_level_meter);
    chkDisplayOscilloscope->setChecked(m_params.display_oscilloscope);
    chkDisplayFFT->setChecked(m_params.display_fft);
    chkDisplayOverview->setChecked(m_params.display_overview);

    // after this point all controls have their initial values

    /* --- connect some missing lowlevel gui functionality --- */

    // record buffer size
    connect(slSourceBuffer, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferChanged(int)));

    // device combo box
    cbSourceDevice->insertStrList(well_known_devices, -1);
    cbSourceDevice->setMinimumWidth(cbSourceDevice->sizeHint().width()+10);
    cbSourceDevice->setCurrentItem(m_params.device_name);
    cbSourceDevice->setEditText(m_params.device_name);

    // "select device..." button
    connect(btSourceSelect, SIGNAL(clicked()),
            this, SLOT(selectRecordDevice()));

}

//***************************************************************************
RecordDialog::~RecordDialog()
{
}

//***************************************************************************
RecordParams RecordDialog::params() const
{
    return m_params;
}

//***************************************************************************
void RecordDialog::sourceBufferChanged(int value)
{
    if (value < 8)  value = 8;
    if (value > 20) value = 20;

    // take the value into our struct
    m_params.buffer_size = value;

    // update the text
    unsigned int buffer_size = (1 << value);
    QString text;
    text = i18n("%1 samples");
    txtSourceBuffer->setText(text.arg(buffer_size));
}

//***************************************************************************
void RecordDialog::selectRecordDevice()
{
    QString filter;
    filter += QString("dsp*|") + i18n("OSS playback device (dsp*)");
    filter += QString("\nadsp*|") + i18n("ALSA playback device (adsp*)");
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
    cbSourceDevice->setEditText(new_device);

}

//***************************************************************************
//***************************************************************************

