/***************************************************************************
        SonagramDialog.cpp  -  dialog for setting up the sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qgroupbox.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qstrlist.h>
#include <qtooltip.h>

#include <libkwave/WindowFunction.h>

#include <libgui/KwavePlugin.h>
#include <libgui/PluginContext.h>
#include <libgui/Slider.h>
#include <libgui/Dialog.h>

#include <kapp.h>

#include "SonagramDialog.h"

//***************************************************************************
static const char *FFT_Sizes[] =
	{"64", "128", "256", "512", "1024", "2048", "4096", 0};

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

//***************************************************************************
SonagramDialog::SonagramDialog(KwavePlugin &p)
    :QDialog(p.getParentWidget(), i18n("sonagram"), true)
{
    bitmaplabel = 0;
    cancel = 0;
    ok = 0;
    pointbox = 0;
    pointlabel = 0;
    pointslider = 0;
    rbColor = 0;
    windowlabel = 0;
    windowtypebox = 0;
    windowtypelabel = 0;

    int h;
    int w;
    int i;
    length = p.getSelection();
    rate   = p.getSignalRate();

    // if nothing selected, select all
    if (length <= 1) length = p.getSignalLength();

    debug("SonagramDialog(): constructor");

    WindowFunction wf(0);
    setCaption (i18n("Set FFT/time resolution parameter"));

    // ----------------------------------------------------------------------
    // ---   create all layout objects   ------------------------------------
    // ----------------------------------------------------------------------

    // create the toplevel layout
    QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
    ASSERT(topLayout);
    if (!topLayout) return;

    QHBoxLayout *pointsLayout = new QHBoxLayout();
    ASSERT(pointsLayout);
    if (!pointsLayout) return;

    QHBoxLayout *windowsizeLayout = new QHBoxLayout();
    ASSERT(windowsizeLayout);
    if (!windowsizeLayout) return;

    QHBoxLayout *bitmapLayout = new QHBoxLayout();
    ASSERT(bitmapLayout);
    if (!bitmapLayout) return;

    QHBoxLayout *windowfuncLayout = new QHBoxLayout();
    ASSERT(windowfuncLayout);
    if (!windowfuncLayout) return;

    // ----------------------------------------------------------------------
    // ---   FFT settings   -------------------------------------------------
    // ----------------------------------------------------------------------

    QGroupBox *fft_frame = new QGroupBox(this);
    ASSERT(fft_frame);
    if (!fft_frame) return;
    fft_frame->setTitle(i18n("FFT settings"));
    topLayout->addWidget(fft_frame);

    QVBoxLayout *fftLayout = new QVBoxLayout(fft_frame, 10);
    ASSERT(fftLayout);
    if (!fftLayout) return;

    pointlabel = new QLabel(i18n("Number of FFT points:"), fft_frame);
    ASSERT(pointlabel);
    if (!pointlabel) return;

    pointbox = new QComboBox (true, fft_frame);
    ASSERT(pointbox);
    if (!pointbox) return;
    pointbox->insertStrList (FFT_Sizes, -1);
    QToolTip::add(pointbox,
	i18n("Try to choose numbers with small prime-factors, "\
	"if choosing big window sizes.\n"\
	"The computation will be much faster !"));

    windowlabel = new QLabel("", fft_frame);
    ASSERT(windowlabel);
    if (!windowlabel) return;

    windowtypebox = new QComboBox (true, fft_frame);
    ASSERT(windowtypebox);
    if (!windowtypebox) return;
    for (i=0; i < wf.getCount(); i++) {
	windowtypebox->insertItem(i18n(wf.getTypes()[i]));
    }
    QToolTip::add(windowtypebox, i18n("Choose windowing function here. "\
	"If fourier transformation should stay reversible, "\
	"use the type <none>"));

    bitmaplabel = new QLabel("", fft_frame);
    ASSERT(bitmaplabel);
    if (!bitmaplabel) return;

    pointslider = new Slider (2, (length / 16), 1, 5,
	Slider::Horizontal, fft_frame);
    ASSERT(pointslider);
    if (!pointslider) return;

    windowtypelabel = new QLabel (i18n("Window Function :"), fft_frame);
    ASSERT(windowtypelabel);
    if (!windowtypelabel) return;

    // -- create the fft frame's layout --

    setPoints(1);    // must set the minimum number of points to get
    setBoxPoints(0); // the largest windowlabel

    /* put together all the layouts */
    fftLayout->addSpacing(fft_frame->fontMetrics().height() );
    fftLayout->addLayout(pointsLayout);
    fftLayout->addLayout(windowfuncLayout);
    fftLayout->addLayout(bitmapLayout);
    fftLayout->addLayout(windowsizeLayout);

    h = max(pointbox->sizeHint().height(),
	    pointlabel->sizeHint().height());

    // number of FFT points
    w = pointlabel->sizeHint().width();
    pointlabel->setFixedSize(w, h);
    pointbox->setFixedHeight(h);
    pointbox->adjustSize();
    pointbox->setMinimumWidth(pointbox->width()+10);
    pointbox->setMaximumWidth(pointbox->width()*2);
    pointsLayout->addWidget(pointlabel, 0, AlignLeft | AlignCenter);
    pointsLayout->addSpacing(10);
    pointsLayout->addStretch(1);
    pointsLayout->addWidget(pointbox, 1, AlignRight | AlignCenter);

    // windowing function
    windowtypelabel->setFixedHeight(h);
    windowtypelabel->setFixedWidth(windowtypelabel->sizeHint().width());
    windowtypebox->setFixedHeight(h);
    windowtypebox->adjustSize();
    windowtypebox->setMinimumWidth(windowtypebox->width()+10);
    windowtypebox->setMaximumWidth(windowtypebox->width()*2);
    windowfuncLayout->addWidget(windowtypelabel, 0, AlignLeft | AlignCenter);
    windowfuncLayout->addSpacing(10);
    windowfuncLayout->addStretch(1);
    windowfuncLayout->addWidget(windowtypebox, 1, AlignRight | AlignCenter);

    // size of the bitmap
    bitmaplabel->setFixedSize(bitmaplabel->sizeHint().width()+20,h);
    pointslider->setFixedHeight(h);
    w = pointslider->sizeHint().width();
    w = max(w, pointslider->sizeHint().height()*8);
    pointslider->setMinimumWidth(w);
    bitmapLayout->addWidget(bitmaplabel, 0, AlignLeft | AlignCenter);
    bitmapLayout->addSpacing(10);
    bitmapLayout->addWidget(pointslider, 1, AlignRight | AlignCenter);

    // resulting window size
    windowlabel->setFixedHeight(h);
    windowlabel->setMinimumWidth(windowlabel->sizeHint().width()+10);
    windowsizeLayout->addWidget(windowlabel, 0, AlignLeft | AlignCenter);

    // ----------------------------------------------------------------------
    // ---   Layout for the lower part: display and update groups   ---------
    // ----------------------------------------------------------------------

    QHBoxLayout *guiLayout = new QHBoxLayout();
    ASSERT(guiLayout);
    if (!guiLayout) return;
    topLayout->addLayout(guiLayout);

    // ----------------------------------------------------------------------
    // ---   display type selection: color/grayscale   ----------------------
    // ----------------------------------------------------------------------

    QButtonGroup *display_group = new QButtonGroup(this);
    ASSERT(display_group);
    if (!display_group) return;
    display_group->setTitle(i18n("display"));
    guiLayout->addWidget(display_group);

    QVBoxLayout *displayLayout = new QVBoxLayout(display_group, 10);
    ASSERT(displayLayout);
    if (!displayLayout) return;
    displayLayout->addSpacing(display_group->fontMetrics().height());

    rbColor = new QRadioButton(display_group);
    ASSERT(rbColor);
    if (!rbColor) return;
    rbColor->setText(i18n("use colors"));
    rbColor->setChecked(TRUE);
    displayLayout->addWidget(rbColor);
    rbColor->setMinimumSize(rbColor->sizeHint());
    QToolTip::add(rbColor, i18n("use different colors for amplitude"));

    QRadioButton *rbGreyScale = new QRadioButton(display_group);
    ASSERT(rbGreyScale);
    if (!rbGreyScale) return;
    rbGreyScale->setText(i18n("greyscale"));
    displayLayout->addWidget(rbGreyScale);
    rbGreyScale->setMinimumSize(rbGreyScale->sizeHint());
    QToolTip::add(rbGreyScale, i18n("use greyscale only for amplitude"));

    // ----------------------------------------------------------------------
    // ---   update settings: track signal changes / follow selection   -----
    // ----------------------------------------------------------------------

    QButtonGroup *update_group = new QButtonGroup(this);
    ASSERT(update_group);
    if (!update_group) return;
    update_group->setTitle(i18n("update"));
    guiLayout->addWidget(update_group);

    QVBoxLayout *updateLayout = new QVBoxLayout(update_group, 10);
    ASSERT(updateLayout);
    if (!updateLayout) return;
    updateLayout->addSpacing(update_group->fontMetrics().height());

    QCheckBox *cbTrackChanges = new QCheckBox(update_group);
    ASSERT(cbTrackChanges);
    if (!cbTrackChanges) return;
    cbTrackChanges->setText(i18n("on modifications"));
    updateLayout->addWidget(cbTrackChanges);
    cbTrackChanges->setMinimumSize(cbTrackChanges->sizeHint());
    QToolTip::add(cbTrackChanges, i18n(
	"automatically update the sonagram\n"\
	"if the signal data has modified"));

    QCheckBox *cbFollowSelection = new QCheckBox(update_group);
    ASSERT(cbFollowSelection);
    if (!cbFollowSelection) return;
    cbFollowSelection->setText(i18n("follow selection"));
    updateLayout->addWidget(cbFollowSelection);
    cbFollowSelection->setMinimumSize(cbFollowSelection->sizeHint());
    QToolTip::add(cbFollowSelection, i18n(
	"automatically update the sonagram if the selection\n"\
	"has been enlarged, reduced or moved"));

    cbTrackChanges->setEnabled(false);    // ###
    cbFollowSelection->setEnabled(false); // ###
	
    // ----------------------------------------------------------------------
    // ---   OK and Cancel buttons   ----------------------------------------
    // ----------------------------------------------------------------------

    // -- separator --
    QFrame *separator = new QFrame(this, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());
    topLayout->addWidget(separator);

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    ASSERT(buttonsLayout);
    if (!buttonsLayout) return;
    topLayout->addLayout(buttonsLayout);

    h = max(ok->sizeHint().height(), cancel->sizeHint().height());
    w = max(ok->sizeHint().width(), cancel->sizeHint().width());
    ok->setFixedSize(w, h);
    cancel->setFixedSize(w, h);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(cancel, 0, AlignLeft | AlignCenter);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(ok, 0, AlignRight | AlignCenter);

    // activate the layout and set the window size

    fftLayout->activate();
    displayLayout->activate();
    updateLayout->activate();
    topLayout->activate();

    w = sizeHint().width();
    h = sizeHint().height();
    topLayout->freeze(w, h);

    setPoints(128);
    setBoxPoints(0);

    ok->setAccel (Key_Return);
    cancel->setAccel(Key_Escape);
    ok->setFocus();

    connect(ok ,         SIGNAL(clicked()),         SLOT(accept()));
    connect(cancel ,     SIGNAL(clicked()),         SLOT(reject()));
    connect(pointslider, SIGNAL(valueChanged(int)), SLOT(setPoints(int)));
    connect(pointbox,    SIGNAL(activated(int)),    SLOT(setBoxPoints(int)));
}

//***************************************************************************
void SonagramDialog::getParameters(QStrList &list)
{
    ASSERT(pointbox);
    ASSERT(windowtypebox);
    ASSERT(rbColor);

    QString param;

    list.setAutoDelete(true);
    list.clear();

    // parameter #0: number of fft points
    param = pointbox ? pointbox->currentText() : 0;
    list.append(param);

    // parameter #1: index of the window function
    param.setNum(windowtypebox ? windowtypebox->currentItem() : 0);
    list.append(param);

    // parameter #2: flag: use color instead of greyscale
    param.setNum(rbColor ? (rbColor->isChecked() ? 1 : 0) : 0);
    list.append(param);

}

//***************************************************************************
void SonagramDialog::setPoints(int points)
{
    char ms_buf[32];
    char buf[512];
    points *= 2;

    snprintf(buf, sizeof(buf), "%d", points);
    pointbox->changeItem (buf, 0);
    pointbox->setCurrentItem (0);

    KwavePlugin::ms2string(ms_buf, sizeof(ms_buf), points * 1.0E3 / rate);
    snprintf(buf, sizeof(buf), i18n("( resulting window size: %s )"), ms_buf);
    windowlabel->setText(buf);
    snprintf(buf, sizeof(buf), i18n("size of bitmap: %dx%d"), 
	(length / points) + 1, points / 2);
    bitmaplabel->setText(buf);
}

//***************************************************************************
void SonagramDialog::setBoxPoints(int num)
{
    int points = strtol(pointbox->text (num), 0, 0);
    pointslider->setValue(points / 2);
}

//***************************************************************************
SonagramDialog::~SonagramDialog ()
{
    debug("SonagramDialog::~SonagramDialog() done");
}

//***************************************************************************
//***************************************************************************
