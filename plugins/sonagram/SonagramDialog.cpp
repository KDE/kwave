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
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kapplication.h> // for invokeHelp
#include <kcombobox.h>
#include <klocale.h>

#include "libkwave/KwavePlugin.h"
#include "libkwave/WindowFunction.h"

#include "SonagramDlg.uih.h"
#include "SonagramDialog.h"

//***************************************************************************
SonagramDialog::SonagramDialog(KwavePlugin &p)
    :SonagramDlg(p.parentWidget(), i18n("sonagram"), true),
    m_length(p.selection()), m_rate(p.signalRate())
{
    Q_ASSERT(pointbox);
    Q_ASSERT(pointslider);
    Q_ASSERT(windowtypebox);
    if (!pointbox) return;
    if (!pointslider) return;
    if (!windowtypebox) return;

    // if nothing selected, select all
    if (m_length <= 1) m_length = p.signalLength();

    pointslider->setMaxValue(m_length / 16);

    window_function_t wf = WINDOW_FUNC_NONE;
    for (unsigned int i=0; i < WindowFunction::count(); i++) {
	windowtypebox->insertItem(WindowFunction::description(wf, true));
	++wf;
    }

    setPoints(1);    // must set the minimum number of points to get
    setBoxPoints(0); // the largest windowlabel

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

    connect(btOK,        SIGNAL(clicked()),         SLOT(accept()));
    connect(btCancel,    SIGNAL(clicked()),         SLOT(reject()));
    connect(btHelp,      SIGNAL(clicked()),         SLOT(invokeHelp()));
    connect(pointslider, SIGNAL(valueChanged(int)), SLOT(setPoints(int)));
    connect(pointbox,    SIGNAL(activated(int)),    SLOT(setBoxPoints(int)));
}

//***************************************************************************
void SonagramDialog::parameters(QStringList &list)
{
    Q_ASSERT(pointbox);
    Q_ASSERT(windowtypebox);
    Q_ASSERT(rbColor);

    QString param;
    list.clear();

    // parameter #0: number of fft points
    param = pointbox ? pointbox->currentText() : (QString)0;
    list.append(param);

    // parameter #1: index of the window function
    window_function_t wf = WindowFunction::findFromIndex(
    	(windowtypebox) ? windowtypebox->currentItem() : 0);
    param = WindowFunction::name(wf);
    list.append(param);

    // parameter #2: flag: use color instead of greyscale
    param.setNum(rbColor ? (rbColor->isChecked() ? 1 : 0) : 0);
    list.append(param);

    // parameter #3: flag: track changes
    param.setNum((cbTrackChanges && cbTrackChanges->isChecked())
        ? 1 : 0);
    list.append(param);

    // parameter #4: flag: follow selection
    param.setNum((cbFollowSelection && cbFollowSelection->isChecked())
        ? 1 : 0);
    list.append(param);

}

//***************************************************************************
void SonagramDialog::setPoints(int points)
{
    Q_ASSERT(points >= 0);
    QString text;
    points *= 2;

    text.setNum(points);
    pointbox->changeItem(text, 0);
    pointbox->setCurrentItem (0);

    windowlabel->setText(i18n("( resulting window size: %1 )").arg(
	KwavePlugin::ms2string(points * 1.0E3 / m_rate)));

    text = i18n("size of bitmap: %1x%2");
    text = text.arg((m_length / points) + 1);
    text = text.arg(points/2);
    bitmaplabel->setText(text);
}

//***************************************************************************
void SonagramDialog::setWindowFunction(window_function_t type)
{
    Q_ASSERT(windowtypebox);
    if (!windowtypebox) return;
    windowtypebox->setCurrentItem(WindowFunction::index(type));
}

//***************************************************************************
void SonagramDialog::setColorMode(int color)
{
    Q_ASSERT(rbColor);
    if (!rbColor) return;

    rbColor->setChecked(color);
    rbGreyScale->setChecked(!color);
}

//***************************************************************************
void SonagramDialog::setTrackChanges(bool track_changes)
{
    Q_ASSERT(cbTrackChanges);
    if (!cbTrackChanges) return;
    cbTrackChanges->setChecked(track_changes);
}

//***************************************************************************
void SonagramDialog::setFollowSelection(bool follow_selection)
{
    Q_ASSERT(cbFollowSelection);
    if (!cbFollowSelection) return;
    cbFollowSelection->setChecked(follow_selection);
}

//***************************************************************************
void SonagramDialog::setBoxPoints(int num)
{
    Q_ASSERT(num >= 0);
    int points = strtol(pointbox->text (num), 0, 0);
    pointslider->setValue(points / 2);
}

//***************************************************************************
SonagramDialog::~SonagramDialog ()
{
}

//***************************************************************************
void SonagramDialog::invokeHelp()
{
    kapp->invokeHelp("sonagram");
}

//***************************************************************************
//***************************************************************************
