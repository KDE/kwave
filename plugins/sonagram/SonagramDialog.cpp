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

#include <math.h>
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
#include <qslider.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtooltip.h>

#include "libkwave/WindowFunction.h"
#include "libgui/KwavePlugin.h"

#include <klocale.h>

#include "SonagramDialog.h"

//***************************************************************************
static const char *FFT_Sizes[] =
	{"64", "128", "256", "512", "1024", "2048", "4096", 0};

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

//***************************************************************************
SonagramDialog::SonagramDialog(KwavePlugin &p)
    :QDialog(p.parentWidget(), i18n("sonagram"), true)
{
    m_bitmaplabel = 0;
    m_cancel = 0;
    m_ok = 0;
    m_pointbox = 0;
    m_pointlabel = 0;
    m_pointslider = 0;
    m_rbColor = 0;
    m_rbGreyScale = 0;
    m_cbTrackChanges = 0;
    m_cbFollowSelection = 0;
    m_windowlabel = 0;
    m_windowtypebox = 0;
    m_windowtypelabel = 0;

    int h;
    int w;
    m_length = p.selection();
    m_rate   = p.signalRate();

    // if nothing selected, select all
    if (m_length <= 1) m_length = p.signalLength();

    setCaption(i18n("Set FFT/time resolution parameter"));

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

    m_pointlabel = new QLabel(i18n("Number of FFT points:"), fft_frame);
    ASSERT(m_pointlabel);
    if (!m_pointlabel) return;

    m_pointbox = new QComboBox (true, fft_frame);
    ASSERT(m_pointbox);
    if (!m_pointbox) return;
    m_pointbox->insertStrList (FFT_Sizes, -1);
    QToolTip::add(m_pointbox,
	i18n("Try to choose numbers with small prime-factors, "\
	"if choosing big window sizes.\n"\
	"The computation will be much faster !"));

    m_windowlabel = new QLabel("", fft_frame);
    ASSERT(m_windowlabel);
    if (!m_windowlabel) return;

    m_windowtypebox = new QComboBox (true, fft_frame);
    ASSERT(m_windowtypebox);
    if (!m_windowtypebox) return;
    window_function_t wf = WINDOW_FUNC_NONE;
    for (unsigned int i=0; i < WindowFunction::count(); i++) {
	m_windowtypebox->insertItem(WindowFunction::description(wf, true));
	++wf;
    }
    QToolTip::add(m_windowtypebox,
	i18n("Choose windowing function here. "\
	"If fourier transformation should stay reversible, "\
	"use the type <none>"));

    m_bitmaplabel = new QLabel("", fft_frame);
    ASSERT(m_bitmaplabel);
    if (!m_bitmaplabel) return;

    m_pointslider = new QSlider (2, (m_length / 16), 1, 5,
	QSlider::Horizontal, fft_frame);
    ASSERT(m_pointslider);
    if (!m_pointslider) return;

    m_windowtypelabel = new QLabel (i18n("Window Function :"), fft_frame);
    ASSERT(m_windowtypelabel);
    if (!m_windowtypelabel) return;

    // -- create the fft frame's layout --

    setPoints(1);    // must set the minimum number of points to get
    setBoxPoints(0); // the largest windowlabel

    /* put together all the layouts */
    fftLayout->addSpacing(fft_frame->fontMetrics().height() );
    fftLayout->addLayout(pointsLayout);
    fftLayout->addLayout(windowfuncLayout);
    fftLayout->addLayout(bitmapLayout);
    fftLayout->addLayout(windowsizeLayout);

    h = max(m_pointbox->sizeHint().height(),
	    m_pointlabel->sizeHint().height());

    // number of FFT points
    w = m_pointlabel->sizeHint().width();
    m_pointlabel->setFixedSize(w, h);
    m_pointbox->setFixedHeight(h);
    m_pointbox->adjustSize();
    m_pointbox->setMinimumWidth(m_pointbox->width()+10);
    m_pointbox->setMaximumWidth(m_pointbox->width()*2);
    pointsLayout->addWidget(m_pointlabel, 0, AlignLeft | AlignCenter);
    pointsLayout->addSpacing(10);
    pointsLayout->addStretch(1);
    pointsLayout->addWidget(m_pointbox, 1, AlignRight | AlignCenter);

    // windowing function
    m_windowtypelabel->setFixedHeight(h);
    m_windowtypelabel->setFixedWidth(m_windowtypelabel->sizeHint().width());
    m_windowtypebox->setFixedHeight(h);
    m_windowtypebox->adjustSize();
    m_windowtypebox->setMinimumWidth(m_windowtypebox->width()+10);
    m_windowtypebox->setMaximumWidth(m_windowtypebox->width()*2);
    windowfuncLayout->addWidget(m_windowtypelabel, 0, AlignLeft | AlignCenter);
    windowfuncLayout->addSpacing(10);
    windowfuncLayout->addStretch(1);
    windowfuncLayout->addWidget(m_windowtypebox, 1, AlignRight | AlignCenter);

    // size of the bitmap
    m_bitmaplabel->setFixedSize(m_bitmaplabel->sizeHint().width()+20,h);
    m_pointslider->setFixedHeight(h);
    w = m_pointslider->sizeHint().width();
    w = max(w, m_pointslider->sizeHint().height()*8);
    m_pointslider->setMinimumWidth(w);
    bitmapLayout->addWidget(m_bitmaplabel, 0, AlignLeft | AlignCenter);
    bitmapLayout->addSpacing(10);
    bitmapLayout->addWidget(m_pointslider, 1, AlignRight | AlignCenter);

    // resulting window size
    m_windowlabel->setFixedHeight(h);
    m_windowlabel->setMinimumWidth(m_windowlabel->sizeHint().width()+10);
    windowsizeLayout->addWidget(m_windowlabel, 0, AlignLeft | AlignCenter);

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

    m_rbColor = new QRadioButton(display_group);
    ASSERT(m_rbColor);
    if (!m_rbColor) return;
    m_rbColor->setText(i18n("use colors"));
    m_rbColor->setChecked(TRUE);
    displayLayout->addWidget(m_rbColor);
    m_rbColor->setMinimumSize(m_rbColor->sizeHint());
    QToolTip::add(m_rbColor, i18n("use different colors for amplitude"));

    m_rbGreyScale = new QRadioButton(display_group);
    ASSERT(m_rbGreyScale);
    if (!m_rbGreyScale) return;
    m_rbGreyScale->setText(i18n("greyscale"));
    displayLayout->addWidget(m_rbGreyScale);
    m_rbGreyScale->setMinimumSize(m_rbGreyScale->sizeHint());
    QToolTip::add(m_rbGreyScale, i18n("use greyscale only for amplitude"));

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

    m_cbTrackChanges = new QCheckBox(update_group);
    ASSERT(m_cbTrackChanges);
    if (!m_cbTrackChanges) return;
    m_cbTrackChanges->setText(i18n("on modifications"));
    updateLayout->addWidget(m_cbTrackChanges);
    m_cbTrackChanges->setMinimumSize(m_cbTrackChanges->sizeHint());
    QToolTip::add(m_cbTrackChanges, i18n(
	"automatically update the sonagram\n"\
	"if the signal data has modified"));

    m_cbFollowSelection = new QCheckBox(update_group);
    ASSERT(m_cbFollowSelection);
    if (!m_cbFollowSelection) return;
    m_cbFollowSelection->setText(i18n("follow selection"));
    updateLayout->addWidget(m_cbFollowSelection);
    m_cbFollowSelection->setMinimumSize(m_cbFollowSelection->sizeHint());
    QToolTip::add(m_cbFollowSelection, i18n(
	"automatically update the sonagram if the selection\n"\
	"has been enlarged, reduced or moved"));

    m_cbTrackChanges->setEnabled(false);    // ###
    m_cbFollowSelection->setEnabled(false); // ###

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

    m_ok = new QPushButton(i18n("OK"), this);
    ASSERT(m_ok);
    if (!m_ok) return;

    m_cancel = new QPushButton(i18n("Cancel"), this);
    ASSERT(m_cancel);
    if (!m_cancel) return;

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    ASSERT(buttonsLayout);
    if (!buttonsLayout) return;
    topLayout->addLayout(buttonsLayout);

    h = max(m_ok->sizeHint().height(), m_cancel->sizeHint().height());
    w = max(m_ok->sizeHint().width(), m_cancel->sizeHint().width());
    m_ok->setFixedSize(w, h);
    m_cancel->setFixedSize(w, h);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(m_cancel, 0, AlignLeft | AlignCenter);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(m_ok, 0, AlignRight | AlignCenter);

    // activate the layout and set the window size

    fftLayout->activate();
    displayLayout->activate();
    updateLayout->activate();
    topLayout->activate();

    w = sizeHint().width();
    h = sizeHint().height();
    topLayout->freeze(w, h);


    // Set a size hint:
    // try to make the image's aspect ratio (a) = sqrt(2)
    //
    // samples: s
    // fft_points: np
    // image_width: w = s / np
    // image_height: h = np / 2
    // a = w / h = 2*s / (np^2)
    // => np = sqrt( 2 * s / a) )
    const double aspect_ratio = sqrt(2);
    double np = sqrt(2.0*(double)m_length/aspect_ratio);

    // round down to an exponent of 2, this makes the image more
    // wide than heigh and gives a fast calculation
    int bits = (int)floor(log(np) / log(2));
    if (bits < 2) bits = 2;
    if (bits > 16) bits = 16;
    setPoints(1 << (bits-1));
    setBoxPoints(0);

    m_ok->setAccel(Key_Return);
    m_cancel->setAccel(Key_Escape);
    m_ok->setFocus();

    connect(m_ok ,         SIGNAL(clicked()),         SLOT(accept()));
    connect(m_cancel ,     SIGNAL(clicked()),         SLOT(reject()));
    connect(m_pointslider, SIGNAL(valueChanged(int)), SLOT(setPoints(int)));
    connect(m_pointbox,    SIGNAL(activated(int)),    SLOT(setBoxPoints(int)));
}

//***************************************************************************
void SonagramDialog::parameters(QStringList &list)
{
    ASSERT(m_pointbox);
    ASSERT(m_windowtypebox);
    ASSERT(m_rbColor);

    QString param;
    list.clear();

    // parameter #0: number of fft points
    param = m_pointbox ? m_pointbox->currentText() : (QString)0;
    list.append(param);

    // parameter #1: index of the window function
    window_function_t wf = WindowFunction::findFromIndex(
    	(m_windowtypebox) ? m_windowtypebox->currentItem() : 0);
    param = WindowFunction::name(wf);
    list.append(param);

    // parameter #2: flag: use color instead of greyscale
    param.setNum(m_rbColor ? (m_rbColor->isChecked() ? 1 : 0) : 0);
    list.append(param);

    // parameter #3: flag: track changes
    param.setNum((m_cbTrackChanges && m_cbTrackChanges->isChecked())
        ? 1 : 0);
    list.append(param);

    // parameter #4: flag: follow selection
    param.setNum((m_cbFollowSelection && m_cbFollowSelection->isChecked())
        ? 1 : 0);
    list.append(param);

}

//***************************************************************************
void SonagramDialog::setPoints(int points)
{
    ASSERT(points >= 0);
    QString text;
    points *= 2;

    text.setNum(points);
    m_pointbox->changeItem(text, 0);
    m_pointbox->setCurrentItem (0);

    m_windowlabel->setText(i18n("( resulting window size: %1 )").arg(
	KwavePlugin::ms2string(points * 1.0E3 / m_rate)));

    text = i18n("size of bitmap: %1x%2");
    text = text.arg((m_length / points) + 1);
    text = text.arg(points/2);
    m_bitmaplabel->setText(text);
}

//***************************************************************************
void SonagramDialog::setWindowFunction(window_function_t type)
{
    ASSERT(m_windowtypebox);
    if (!m_windowtypebox) return;
    m_windowtypebox->setCurrentItem(WindowFunction::index(type));
}

//***************************************************************************
void SonagramDialog::setColorMode(int color)
{
    ASSERT(m_rbColor);
    if (!m_rbColor) return;

    m_rbColor->setChecked(color);
    m_rbGreyScale->setChecked(!color);
}

//***************************************************************************
void SonagramDialog::setTrackChanges(bool track_changes)
{
    ASSERT(m_cbTrackChanges);
    if (!m_cbTrackChanges) return;
    m_cbTrackChanges->setChecked(track_changes);
}

//***************************************************************************
void SonagramDialog::setFollowSelection(bool follow_selection)
{
    ASSERT(m_cbFollowSelection);
    if (!m_cbFollowSelection) return;
    m_cbFollowSelection->setChecked(follow_selection);
}

//***************************************************************************
void SonagramDialog::setBoxPoints(int num)
{
    ASSERT(num >= 0);
    int points = strtol(m_pointbox->text (num), 0, 0);
    m_pointslider->setValue(points / 2);
}

//***************************************************************************
SonagramDialog::~SonagramDialog ()
{
}

//***************************************************************************
//***************************************************************************
