/***************************************************************************
     PlayBackDialog.cpp  -  dialog for configuring the playback
			     -------------------
    begin                : Sun May 13 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include <stdio.h>
#include <stdlib.h>

#include <qlabel.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kfiledialog.h>
#include <klocale.h>

#include "libgui/KwaveFileDialog.h"
#include "libkwave/KwavePlugin.h"
#include "PlayBackDialog.h"

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

static const char *devicetext[] = {
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
PlayBackDialog::PlayBackDialog(KwavePlugin &p, const playback_param_t &params)
    :KDialog(p.parentWidget(), i18n("playback"), true),
    m_playback_params(params)
{
    m_bg = 0;
    m_b8 = 0;
    m_b16 = 0;
    m_b24 = 0;
    m_buffer_label = 0;
    m_buffer_size = 0;
    m_cancel = 0;
    m_device_label = 0;
    m_device_box = 0;
    m_stereo = 0;
    m_separator = 0;
    m_select_device = 0;
    m_test = 0;
    m_ok = 0;

    int h=0, w=0;

    setCaption(i18n("Playback Options :"));

    // -- create all layout objects --

    QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
    ASSERT(topLayout);
    if (!topLayout) return;

    QHBoxLayout *deviceLayout = new QHBoxLayout();
    ASSERT(deviceLayout);
    if (!deviceLayout) return;

    QHBoxLayout *bufferLayout = new QHBoxLayout();
    ASSERT(bufferLayout);
    if (!bufferLayout) {
	delete deviceLayout;
	return;
    }

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    ASSERT(buttonsLayout);
    if (!buttonsLayout) {
	delete deviceLayout;
	delete bufferLayout;
	return;
    }

    // -- checkboxes for 8/16/24 Bits --
    m_bg = new QButtonGroup(this, i18n("Resolution"));
    m_bg->setTitle(i18n("Resolution"));
    ASSERT(m_bg);
    if (!m_bg) return;

    topLayout->addWidget(m_bg);

    // Create layouts for the check boxes
    QVBoxLayout *bitsBoxLayout = new QVBoxLayout(m_bg, 10);
    bitsBoxLayout->addSpacing(m_bg->fontMetrics().height() );

    QHBoxLayout *bitsLayout = new QHBoxLayout();
    ASSERT(bitsLayout);
    if (!bitsLayout) return;
    bitsBoxLayout->addLayout(bitsLayout);
    bitsLayout->addSpacing(m_bg->fontMetrics().height() );

    m_b8 = new QRadioButton(i18n("&8 Bit"), m_bg);
    ASSERT(m_b8);
    if (!m_b8) return;
    m_b8->setText(i18n("&8 Bit"));
    m_b8->setMinimumSize(m_b8->sizeHint());
    m_b8->setChecked(m_playback_params.bits_per_sample == 8);
    QToolTip::add(m_b8,
        i18n("Set Playback-Mode to 8 Bit (poor quality)\n"\
	     "should be supported by all sound hardware !"));

    m_b16 = new QRadioButton(i18n("1&6 Bit"), m_bg);
    ASSERT(m_b16);
    if (!m_b16) return;
    m_b16->setMinimumSize(m_b16->sizeHint());
    m_b16->setChecked(m_playback_params.bits_per_sample == 16);
    QToolTip::add( m_b16,
        i18n("Set Playback-Mode to 16 Bit (CD-like quality)\n"\
	     "not supported by all sound hardware !"));

    m_b24 = new QRadioButton(i18n("2&4 Bit"), m_bg);
    ASSERT(m_b24);
    if (!m_b24) return;
    m_b24->setMinimumSize(m_b24->sizeHint());
    m_b24->setChecked(m_playback_params.bits_per_sample == 24);
    QToolTip::add( m_b24,
        i18n("Set Playback-Mode to 24 Bit (high quality)\n"\
	     "only supported by some new sound hardware !"));
    m_b24->setEnabled(false); // ### not implemented yet ###
    //     -> needs support for ALSA,
    //        OSS/Free only supports up to 16 bits :-(

    // -- playback device --
    m_device_label = new QLabel(i18n("Playback Device :"), this);
    ASSERT(m_device_label);
    if (!m_device_label) return;

    m_device_box = new QComboBox(true, this);
    ASSERT(m_device_box);
    if (!m_device_box) return;
    m_device_box->insertStrList(devicetext, -1);
    m_device_box->setMinimumWidth(m_device_box->sizeHint().width()+10);
    for (int i=0; i < m_device_box->count(); i++) {
	if (m_playback_params.device == m_device_box->text(i)) {
	    m_device_box->setCurrentItem(i);
	    break;
	}
    }
    m_device_box->setEditText(m_playback_params.device);

    h = max(m_device_label->sizeHint().height(),
            m_device_box->sizeHint().height());
    w = m_device_label->sizeHint().width();
    m_device_label->setFixedSize(w, h);
    m_device_box->setFixedHeight(h);

    m_select_device = new QPushButton(i18n("se&lect..."), this);
    ASSERT(m_select_device);
    if (!m_select_device) return;
    QToolTip::add(m_select_device,
	i18n("Select a playback device not listed\n"\
	     "in the standard selection."));
    m_select_device->setEnabled(true);
    m_select_device->setFixedWidth(m_select_device->sizeHint().width());

    // -- buffer size --
    m_buffer_size = new QSlider(8, 16, 1, 5, QSlider::Horizontal, this);
    ASSERT(m_buffer_size);
    if (!m_buffer_size) return;
    m_buffer_size->setValue(m_playback_params.bufbase);

    m_buffer_label = new QLabel("", this);
    ASSERT(m_buffer_label);
    if (!m_buffer_label) return;
    setBufferSize(m_playback_params.bufbase);
    QToolTip::add(m_buffer_size, i18n("This is the size of the buffer "\
	"used for communication with the sound driver\n"\
	"If your computer is rather slow select a big buffer"));

    w = m_buffer_label->sizeHint().width()+20;
    m_buffer_label->setFixedWidth(w);
    m_buffer_size->setMinimumWidth(w/2);

    // -- m_stereo checkbox --
    m_stereo = new QCheckBox(i18n("&stereo playback"), this);
    ASSERT(m_stereo);
    if (!m_stereo) return;
    m_stereo->setChecked(m_playback_params.channels >= 2);
    m_stereo->setFixedHeight(m_stereo->sizeHint().height());

    // -- separator --
    m_separator = new QFrame(this, "separator line");
    ASSERT(m_separator);
    if (!m_separator) return;
    m_separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    m_separator->setFixedHeight(m_separator->sizeHint().height());

    // give all widgets the same height
    h = max(m_buffer_label->sizeHint().height(),
            m_buffer_size->sizeHint().height());
    h = max(h, m_device_label->sizeHint().height());
    h = max(h, m_device_box->sizeHint().height());
    h = max(h, m_select_device->sizeHint().height());

    m_buffer_label->setFixedHeight(h);
    m_buffer_size->setFixedHeight(h);
    m_device_label->setFixedHeight(h);
    m_device_box->setFixedHeight(h);
    m_select_device->setFixedHeight(h);

    // button for "test settings"
    m_test = new QPushButton(i18n("&test settings"), this);
    ASSERT(m_test);
    if (!m_test) return;
    QToolTip::add(m_test,
	i18n("Try to play a short sound\n"\
	     "using the current settings.\n"\
	     "(sorry, not implemented yet!)"));
    m_test->setEnabled(false); // ### not implemented yet ###

    // buttons for OK and m_cancel
    m_ok = new QPushButton(i18n("&OK"), this);
    ASSERT(m_ok);
    if (!m_ok) return;

    m_cancel = new QPushButton(i18n("&Cancel"), this);
    ASSERT(m_cancel);
    if (!m_cancel) return;

    h = max(m_ok->sizeHint().height(), m_cancel->sizeHint().height());
    w = max(m_ok->sizeHint().width(), m_cancel->sizeHint().width());
    m_ok->setFixedSize(w, h);
    m_cancel->setFixedSize(w,h);
    m_test->setFixedSize(m_test->sizeHint().width(), h);

    // add controls to their layouts
    bitsLayout->addWidget(m_b8, 0, AlignLeft | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(m_b16, 0, AlignCenter | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(m_b24, 0, AlignRight | AlignCenter);

    topLayout->addLayout(deviceLayout);
    deviceLayout->addWidget(m_device_label, 0, AlignLeft | AlignCenter);
    deviceLayout->addSpacing(10);
    deviceLayout->addWidget(m_device_box, 0, AlignRight | AlignCenter);
    deviceLayout->addWidget(m_select_device, 0, AlignRight | AlignCenter);

    topLayout->addLayout(bufferLayout);
    bufferLayout->addWidget(m_buffer_label, 0, AlignLeft | AlignCenter);
    bufferLayout->addSpacing(10);
    bufferLayout->addWidget(m_buffer_size,0, AlignRight | AlignCenter);

    topLayout->addWidget(m_stereo);
    topLayout->addWidget(m_separator);

    topLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(m_test, 0, AlignLeft | AlignCenter);
    buttonsLayout->addSpacing(30);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(m_ok, 0, AlignRight | AlignCenter);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(m_cancel, 0, AlignRight | AlignCenter);

    bitsBoxLayout->activate();
    topLayout->activate();
    topLayout->freeze(0,0);

    m_ok->setFocus();
    connect(m_ok ,SIGNAL(clicked()), SLOT (accept()));
    connect(m_cancel, SIGNAL(clicked()), SLOT (reject()));
    connect(m_buffer_size, SIGNAL(valueChanged(int)),
            SLOT(setBufferSize(int)));
    connect(m_select_device, SIGNAL(clicked()),
            SLOT(selectPlaybackDevice()));
}

//***************************************************************************
void PlayBackDialog::setBufferSize(int exp)
{
    ASSERT(m_buffer_label);
    if (!m_buffer_label) return;

    char buf[256];
    int val = 1 << exp;

    snprintf(buf, sizeof(buf), i18n("Buffer Size : %5d samples"), val);
    m_buffer_label->setText (buf);

    m_playback_params.bufbase = exp;
}

//***************************************************************************
void PlayBackDialog::parameters(QStringList &list)
{
    ASSERT(m_stereo);
    ASSERT(m_b24);
    ASSERT(m_b16);
    ASSERT(m_device_box);

    QString param;
    list.clear();

    if (!m_stereo || !m_b24 || !m_b16 || !m_device_box) return;

    // parameter #0: number of channels [1 | 2]
    m_playback_params.channels = m_stereo->isChecked() ? 2 : 1;
    param = param.setNum(m_playback_params.channels);
    list.append(param);

    // parameter #1: bits per sample [8 | 16 ]
    m_playback_params.bits_per_sample = m_b24->isChecked() ? 24 :
	(m_b16->isChecked() ? 16 : 8);
    param = param.setNum(m_playback_params.bits_per_sample);
    list.append(param);

    // parameter #2: playback device [/dev/dsp , ... ]
    m_playback_params.device = m_device_box->currentText();
    param = m_playback_params.device;
    list.append(param);

    // parameter #3: base of buffer size [4...16]
    // m_playback_params.bufbase = ...; already set by setBufferSize
    param = param.setNum(m_playback_params.bufbase);
    list.append(param);

}


//***************************************************************************
void PlayBackDialog::selectPlaybackDevice()
{
    QString filter;
    filter += QString("dsp*|") + i18n("OSS playback device (dsp*)");
    filter += QString("\nadsp*|") + i18n("ALSA playback device (adsp*)");
    filter += QString("\n*|") + i18n("Any device (*)");

    KwaveFileDialog dlg(":<kwave_playback_device>", filter, this,
	"Kwave select playback device", true, "file:/dev");
    dlg.setKeepLocation(true);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Select Playback Device"));
    dlg.setURL("file:"+m_playback_params.device);
    if (dlg.exec() != QDialog::Accepted) return;

    QString new_device = dlg.selectedFile();

    // selected new device
    m_device_box->setEditText(new_device);
}

//***************************************************************************
PlayBackDialog::~PlayBackDialog()
{
}

//***************************************************************************
//***************************************************************************
