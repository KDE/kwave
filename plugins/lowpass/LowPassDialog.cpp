/***************************************************************************
      LowPassDialog.cpp  -  dialog for the "lowpass" plugin
                             -------------------
    begin                : Fri Mar 07 2003
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
#include "math.h"

#include <qwidget.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <knuminput.h>

#include <klocale.h>
#include "libkwave/Parser.h"
#include "libgui/ScaleWidget.h"
#include "libgui/FrequencyResponseWidget.h"

#include "LowPassDialog.h"

//***************************************************************************
LowPassDialog::LowPassDialog(QWidget *parent, double sample_rate)
    :LowPassDlg(parent, 0, true), m_frequency(3500),
     m_sample_rate(sample_rate)
{

    // set maximum frequency to sample rate / 2
    double f_max = sample_rate / 2.0;

    slider->setMaxValue((int)f_max);
    spinbox->setMaxValue((int)f_max);

    // initialize the frequency scale widget
    scale_freq->setMinMax(0, (int)f_max);
    scale_freq->setLogMode(true);
    scale_freq->setUnit(i18n("Hz"));

    // initialize the attenuation scale widget
    scale_db->setMinMax(-96, +12);
    scale_db->setLogMode(false);
    scale_db->setUnit(i18n("dB"));

    // initialize the frequency response widget
    freq_response->init(f_max, -96, +12);
    
    // initialize the controls and the curve display
    slider->setValue((int)m_frequency);
    spinbox->setValue((int)m_frequency);
    updateDisplay();
    
    // changes in the slider or spinbox
    connect(spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged(int)));

    // set the initial size of the dialog
    int h = (width() * 3) / 5;
    if (height() < h) resize(width(), h);
    int w = (height() * 5) / 3;
    if (width() < w) resize(w, height());
}

//***************************************************************************
LowPassDialog::~LowPassDialog()
{
}

//***************************************************************************
void LowPassDialog::valueChanged(int pos)
{
    if ((int)m_frequency != pos) {
	m_frequency = pos;
	updateDisplay();
    }
}

//***************************************************************************
QStringList LowPassDialog::params()
{
    QStringList list;
    list << QString::number(m_frequency);
    return list;
}

//***************************************************************************
void LowPassDialog::setParams(QStringList &params)
{
    // evaluate the parameter list
    bool ok;
    double frequency = params[0].toDouble(&ok);
    ASSERT(ok);
    if (ok) m_frequency = frequency;

    slider->setValue((int)m_frequency);
    spinbox->setValue((int)m_frequency);

    updateDisplay();
}

//***************************************************************************
void LowPassDialog::updateDisplay()
{
   
}

//***************************************************************************
//***************************************************************************
