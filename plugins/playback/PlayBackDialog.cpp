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
#include <qkeycode.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <kfiledialog.h>
#include <klocale.h>

#include <libgui/KwavePlugin.h>
#include <libgui/Slider.h>

#include "PlayBackDialog.h"

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

static const char *devicetext[] = {
    "/dev/dsp",
    "/dev/audio",
    "/dev/adsp",
    "/dev/dio",
    0
};

//***************************************************************************
PlayBackDialog::PlayBackDialog(KwavePlugin &p, const playback_param_t &params)
    :KDialog(p.parentWidget(), i18n("sonagram"), true),
    m_playback_params(params)
{
    bg = 0;
    b8 = 0;
    b16 = 0;
    b24 = 0;
    bufferlabel = 0;
    buffersize = 0;
    cancel = 0;
    devicelabel = 0;
    devicebox = 0;
    stereo = 0;
    separator = 0;
    select_device = 0;
    test = 0;
    ok = 0;

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
    bg = new QButtonGroup(this, i18n("Resolution"));
    bg->setTitle(i18n("Resolution"));
    ASSERT(bg);
    if (!bg) return;

    topLayout->addWidget(bg);

    // Create layouts for the check boxes
    QVBoxLayout *bitsBoxLayout = new QVBoxLayout(bg, 10);
    bitsBoxLayout->addSpacing(bg->fontMetrics().height() );

    QHBoxLayout *bitsLayout = new QHBoxLayout();
    ASSERT(bitsLayout);
    if (!bitsLayout) return;
    bitsBoxLayout->addLayout(bitsLayout);
    bitsLayout->addSpacing(bg->fontMetrics().height() );

    b8 = new QRadioButton(i18n("&8 Bit"), bg);
    ASSERT(b8);
    if (!b8) return;
    b8->setText(i18n("&8 Bit"));
    b8->setMinimumSize(b8->sizeHint());
    b8->setChecked(m_playback_params.bits_per_sample == 8);
    QToolTip::add(b8,
        i18n("Set Playback-Mode to 8 Bit (poor quality)\n"\
	     "should be supported by all sound hardware !"));

    b16 = new QRadioButton(i18n("1&6 Bit"), bg);
    ASSERT(b16);
    if (!b16) return;
    b16->setMinimumSize(b16->sizeHint());
    b16->setChecked(m_playback_params.bits_per_sample == 16);
    QToolTip::add( b16,
        i18n("Set Playback-Mode to 16 Bit (CD-like quality)\n"\
	     "not supported by all sound hardware !"));

    b24 = new QRadioButton(i18n("2&4 Bit"), bg);
    ASSERT(b24);
    if (!b24) return;
    b24->setMinimumSize(b24->sizeHint());
    b24->setChecked(m_playback_params.bits_per_sample == 24);
    QToolTip::add( b24,
        i18n("Set Playback-Mode to 24 Bit (high quality)\n"\
	     "only supported by some new sound hardware !"));
    b24->setEnabled(false); // ### not implemented yet ###

    // -- playback device --
    devicelabel = new QLabel(i18n("Playback Device :"), this);
    ASSERT(devicelabel);
    if (!devicelabel) return;

    devicebox = new QComboBox(true, this);
    ASSERT(devicebox);
    if (!devicebox) return;
    devicebox->insertStrList(devicetext, -1);
    devicebox->setMinimumWidth(devicebox->sizeHint().width()+10);
    for (int i=0; i < devicebox->count(); i++) {
	if (strcmp(m_playback_params.device, devicebox->text(i))==0) {
	    devicebox->setCurrentItem(i);
	    break;
	}
    }

    h = max(devicelabel->sizeHint().height(),
            devicebox->sizeHint().height());
    w = devicelabel->sizeHint().width();
    devicelabel->setFixedSize(w, h);
    devicebox->setFixedHeight(h);

    select_device = new QPushButton(i18n("se&lect..."), this);
    ASSERT(select_device);
    if (!select_device) return;
    QToolTip::add(select_device,
	i18n("Select a playback device not listed\n"\
	     "in the standard selection.\n"\
	     "(sorry, not implemented yet!)"));
    select_device->setEnabled(false);
    select_device->setFixedWidth(select_device->sizeHint().width());
    // ### not implemented yet ###
    //     -> needs support for ALSA,
    //        OSS/Free only supports up to 16 bits :-(

    // -- buffer size --
    buffersize = new Slider(8, 16, 1, 5, Slider::Horizontal, this);
    ASSERT(buffersize);
    if (!buffersize) return;
    buffersize->setValue(m_playback_params.bufbase);

    bufferlabel = new QLabel("", this);
    ASSERT(bufferlabel);
    if (!bufferlabel) return;
    setBufferSize(m_playback_params.bufbase);
    QToolTip::add(buffersize, i18n("This is the size of the buffer "\
	"used for communication with the sound driver\n"\
	"If your computer is rather slow select a big buffer"));

    w = bufferlabel->sizeHint().width()+20;
    bufferlabel->setFixedWidth(w);
    buffersize->setMinimumWidth(w/2);

    // -- stereo checkbox --
    stereo = new QCheckBox(i18n("&stereo playback"), this);
    ASSERT(stereo);
    if (!stereo) return;
    stereo->setChecked(m_playback_params.channels >= 2);
    stereo->setFixedHeight(stereo->sizeHint().height());

    // -- separator --
    separator = new QFrame(this, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());

    // give all widgets the same height
    h = max(bufferlabel->sizeHint().height(),
            buffersize->sizeHint().height());
    h = max(h, devicelabel->sizeHint().height());
    h = max(h, devicebox->sizeHint().height());
    h = max(h, select_device->sizeHint().height());

    bufferlabel->setFixedHeight(h);
    buffersize->setFixedHeight(h);
    devicelabel->setFixedHeight(h);
    devicebox->setFixedHeight(h);
    select_device->setFixedHeight(h);

    // button for "test settings"
    test = new QPushButton(i18n("&test settings"), this);
    ASSERT(test);
    if (!test) return;
    QToolTip::add(test,
	i18n("Try to play a short sound\n"\
	     "using the current settings.\n"\
	     "(sorry, not implemented yet!)"));
    test->setEnabled(false); // ### not implemented yet ###

    // buttons for OK and Cancel
    ok = new QPushButton(BUTTON_OK, this);
    ASSERT(ok);
    if (!ok) return;

    cancel = new QPushButton(BUTTON_CANCEL, this);
    ASSERT(cancel);
    if (!cancel) return;

    h = max(ok->sizeHint().height(), cancel->sizeHint().height());
    w = max(ok->sizeHint().width(), cancel->sizeHint().width());
    ok->setFixedSize(w, h);
    cancel->setFixedSize(w,h);
    test->setFixedSize(test->sizeHint().width(), h);

    // add controls to their layouts
    bitsLayout->addWidget(b8, 0, AlignLeft | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(b16, 0, AlignCenter | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(b24, 0, AlignRight | AlignCenter);

    topLayout->addLayout(deviceLayout);
    deviceLayout->addWidget(devicelabel, 0, AlignLeft | AlignCenter);
    deviceLayout->addSpacing(10);
    deviceLayout->addWidget(devicebox, 0, AlignRight | AlignCenter);
    deviceLayout->addWidget(select_device, 0, AlignRight | AlignCenter);

    topLayout->addLayout(bufferLayout);
    bufferLayout->addWidget(bufferlabel, 0, AlignLeft | AlignCenter);
    bufferLayout->addSpacing(10);
    bufferLayout->addWidget(buffersize,0, AlignRight | AlignCenter);

    topLayout->addWidget(stereo);
    topLayout->addWidget(separator);

    topLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(test, 0, AlignLeft | AlignCenter);
    buttonsLayout->addSpacing(30);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(ok, 0, AlignRight | AlignCenter);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(cancel, 0, AlignRight | AlignCenter);

    bitsBoxLayout->activate();
    topLayout->activate();
    topLayout->freeze(0,0);

    ok->setFocus();
    connect(ok ,SIGNAL(clicked()), SLOT (accept()));
    connect(cancel, SIGNAL(clicked()), SLOT (reject()));
    connect(buffersize, SIGNAL(valueChanged(int)),
                        SLOT(setBufferSize(int)));
    connect(select_device, SIGNAL(clicked()),
                           SLOT(selectPlaybackDevice()));
}

//***************************************************************************
void PlayBackDialog::setBufferSize(int exp)
{
    ASSERT(bufferlabel);
    if (!bufferlabel) return;

    char buf[256];
    int val = 1 << exp;

    snprintf(buf, sizeof(buf), i18n("Buffer Size : %5d samples"), val);
    bufferlabel->setText (buf);

    m_playback_params.bufbase = exp;
}

//***************************************************************************
void PlayBackDialog::parameters(QStringList &list)
{
    ASSERT(stereo);
    ASSERT(b24);
    ASSERT(b16);
    ASSERT(devicebox);

    QString param;
    list.clear();

    if (!stereo || !b24 || !b16 || !devicebox) return;

    // parameter #0: sample rate [44100]
    param = param.setNum(m_playback_params.rate);
    list.append(param);

    // parameter #1: number of channels [1 | 2]
    m_playback_params.channels = stereo->isChecked() ? 2 : 1;
    param = param.setNum(m_playback_params.channels);
    list.append(param);

    // parameter #2: bits per sample [8 | 16 ]
    m_playback_params.bits_per_sample = b24->isChecked() ? 24 :
	(b16->isChecked() ? 16 : 8);
    param = param.setNum(m_playback_params.bits_per_sample);
    list.append(param);

    // parameter #3: playback device [/dev/dsp , ... ]
    m_playback_params.device = devicebox->currentText();
    param = m_playback_params.device;
    list.append(param);

    // parameter #4: base of buffer size [4...16]
    // m_playback_params.bufbase = ...; already set by setBufferSize
    param = param.setNum(m_playback_params.bufbase);
    list.append(param);

}


//***************************************************************************
void PlayBackDialog::selectPlaybackDevice()
{
    QString new_device = KFileDialog::getOpenFileName("/dev/", 0, this);
    if (new_device.isNull()) return;

}

//***************************************************************************
PlayBackDialog::~PlayBackDialog()
{
}

//***************************************************************************
//***************************************************************************
