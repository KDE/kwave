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

#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include <qstrlist.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qcombobox.h>

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

    pointlabel = new QLabel(i18n("Number of FFT points:"), this);
    pointbox = new QComboBox (true, this);
    pointbox->insertStrList (FFT_Sizes, -1);
    QToolTip::add(pointbox,
	i18n("Try to choose numbers with small prime-factors, "\
	"if choosing big window sizes.\n"\
	"The computation will be much faster !"));

    windowlabel = new QLabel("", this);

    windowtypebox = new QComboBox (true, this);
    for (i=0; i < wf.getCount(); i++) {
	windowtypebox->insertItem(i18n(wf.getTypes()[i]));
    }


    QToolTip::add(windowtypebox, i18n("Choose windowing function here. "\
	"If fourier transformation should stay reversible, "\
	"use the type <none>"));

    bitmaplabel = new QLabel ("", this);
    pointslider = new Slider (2, (length / 16), 1, 5, Slider::Horizontal, this);
    windowtypelabel = new QLabel (i18n("Window Function :"), this);

    setPoints (50);
    setBoxPoints (0);

    // -- separator --
    QFrame *separator = new QFrame(this, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());

    ok = new QPushButton (OK, this);
    cancel = new QPushButton (CANCEL, this);

    // -- create the layout object --

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

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    ASSERT(buttonsLayout);
    if (!buttonsLayout) return;

    /* put together all the layouts */
    topLayout->addLayout(pointsLayout);
    topLayout->addLayout(windowsizeLayout);
    topLayout->addLayout(bitmapLayout);
    topLayout->addLayout(windowfuncLayout);
    topLayout->addWidget(separator);
    topLayout->addLayout(buttonsLayout);

    h = max(pointbox->sizeHint().height(),
	    pointlabel->sizeHint().height());

    // number of FFT points
    w = pointlabel->sizeHint().width();
    pointlabel->setFixedSize(w, h);
    pointbox->setFixedHeight(h);
    pointbox->adjustSize();
    pointbox->setMinimumWidth(pointbox->width()+10);
    pointsLayout->addWidget(pointlabel, 0, AlignLeft | AlignCenter);
    pointsLayout->addSpacing(10);
    pointsLayout->addWidget(pointbox, 0, AlignRight | AlignCenter);

    // resulting window size
    windowlabel->setFixedHeight(h);
    windowlabel->setMinimumWidth(windowlabel->sizeHint().width()+10);
    windowsizeLayout->addWidget(windowlabel, 0, AlignLeft | AlignCenter);

    // size of the bitmap
    bitmaplabel->setFixedSize(bitmaplabel->sizeHint().width()+20,h);
    pointslider->setFixedHeight(h);
    bitmapLayout->addWidget(bitmaplabel, 0, AlignLeft | AlignCenter);
    bitmapLayout->addSpacing(10);
    bitmapLayout->addWidget(pointslider, 0, AlignRight | AlignCenter);

    // windowing function
    windowtypelabel->setFixedHeight(h);
    windowtypelabel->setFixedWidth(windowtypelabel->sizeHint().width());
    windowtypebox->setFixedHeight(h);
    windowtypebox->adjustSize();
    windowtypebox->setMinimumWidth(windowtypebox->width()+10);
    windowfuncLayout->addWidget(windowtypelabel, 0, AlignLeft | AlignCenter);
    windowfuncLayout->addSpacing(10);
    windowfuncLayout->addWidget(windowtypebox, 0, AlignRight | AlignCenter);

    // OK and Cancel buttons
    h = max(ok->sizeHint().height(), cancel->sizeHint().height());
    w = max(ok->sizeHint().width(), cancel->sizeHint().width());
    ok->setFixedSize(w, h);
    cancel->setFixedSize(w, h);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(cancel, 0, AlignLeft | AlignCenter);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(ok, 0, AlignRight | AlignCenter);

    topLayout->activate();

    /* try to make the size 5x3 */
    h = minimumSize().height();
    w = max(h*5/3, minimumSize().width());
    topLayout->freeze(w,h);

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
    QString param;

    list.setAutoDelete(true);
    list.clear();

    // parameter #1: number of fft points
    ASSERT(pointbox);
    param = pointbox ? pointbox->currentText() : 0;
    list.append(param);

    // parameter #2: index of the window function
    ASSERT(windowtypebox);
    param.setNum(windowtypebox ? windowtypebox->currentItem() : 0);
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
    snprintf(buf, sizeof(buf), i18n("resulting window size: %s"), ms_buf);
    windowlabel->setText(buf);
    snprintf(buf, sizeof(buf), i18n("size of bitmap: %dx%d"), 
	(length / points) + 1, points / 2);
    bitmaplabel->setText(buf);
}

//***************************************************************************
void SonagramDialog::setBoxPoints(int num)
{
    int points = strtol(pointbox->text (num), 0, 0);
    pointslider->setValue (points / 2);
}

//***************************************************************************
SonagramDialog::~SonagramDialog ()
{
    debug("SonagramDialog::~SonagramDialog() done");
}

//***************************************************************************
//***************************************************************************
