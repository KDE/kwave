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

#include "config.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QString>
#include <QStringList>

#include <kcombobox.h>
#include <klocale.h>
#include <ktoolinvocation.h>

#include "libkwave/KwavePlugin.h"
#include "libkwave/WindowFunction.h"
#include "libkwave/Utils.h"

#include "SonagramDialog.h"

//***************************************************************************
SonagramDialog::SonagramDialog(Kwave::Plugin &p)
    :QDialog(p.parentWidget()), Ui::SonagramDlg(),
    m_length(p.selection(0, 0, 0,true)), m_rate(p.signalRate())
{
    setupUi(this);
    setModal(true);

    Q_ASSERT(pointbox);
    Q_ASSERT(pointslider);
    Q_ASSERT(windowtypebox);
    if (!pointbox) return;
    if (!pointslider) return;
    if (!windowtypebox) return;

    pointslider->setMaximum(m_length / 16);

    window_function_t wf = WINDOW_FUNC_NONE;
    for (unsigned int i=0; i < WindowFunction::count(); i++) {
	windowtypebox->addItem(WindowFunction::description(wf, true));
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
    double np = sqrt(2.0 * static_cast<double>(m_length) / aspect_ratio);

    // round down to an exponent of 2, this makes the image more
    // wide than heigh and gives a fast calculation
    int bits = static_cast<int>(floor(log(np) / log(2)));
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
    param = pointbox ? pointbox->currentText() : QString();
    list.append(param);

    // parameter #1: index of the window function
    window_function_t wf = WindowFunction::findFromIndex(
    	(windowtypebox) ? windowtypebox->currentIndex() : 0);
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
    pointbox->setEditText(text);

    windowlabel->setText(i18n("(resulting window size: %1)",
	Kwave::ms2string(points * 1.0E3 / m_rate)));

    bitmaplabel->setText(i18n("Size of bitmap: %1x%2",
	(m_length / points) + 1,
	points/2));
}

//***************************************************************************
void SonagramDialog::setWindowFunction(window_function_t type)
{
    Q_ASSERT(windowtypebox);
    if (!windowtypebox) return;
    windowtypebox->setCurrentIndex(WindowFunction::index(type));
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
    int points = pointbox->itemText(num).toInt();
    pointslider->setValue(points / 2);
}

//***************************************************************************
SonagramDialog::~SonagramDialog ()
{
}

//***************************************************************************
void SonagramDialog::invokeHelp()
{
    KToolInvocation::invokeHelp("sonagram");
}

//***************************************************************************
#include "SonagramDialog.moc"
//***************************************************************************
//***************************************************************************
