/***************************************************************************
       VolumeDialog.cpp  -  dialog for the "volume" plugin
                             -------------------
    begin                : Sun Oct 27 2002
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

#include "config.h"
#include "math.h"

#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>

#include <klocale.h>
#include "libkwave/Parser.h"
#include "libgui/CurveWidget.h"
#include "libgui/ScaleWidget.h"

#include "InvertableSpinBox.h"
#include "VolumeDialog.h"

//***************************************************************************
VolumeDialog::VolumeDialog(QWidget *parent)
    :VolumeDlg(parent, 0, true), m_factor(0.5), m_mode(MODE_PERCENT),
     m_enable_updates(true)
{
    setMode(m_mode);

    // process changed in mode selection
    connect(rbFactor, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));
    connect(rbPercentage, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));
    connect(rbLogarithmic, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));

    // changes in the slider or spinbox
    connect(slider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(spinboxChanged(int)));

    // set the initial size of the dialog
    setFixedWidth(minimumWidth());
    int h = (width() * 5) / 3;
    if (height() < h) resize(width(), h);
}

//***************************************************************************
VolumeDialog::~VolumeDialog()
{
}

//***************************************************************************
void VolumeDialog::setMode(Mode mode)
{
//  debug("VolumeDialog::setMode(%d), f=%g", (int)mode, m_factor); // ###
    double value = m_factor;
    m_mode = mode;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    switch (m_mode) {
	case MODE_FACTOR: {
	    rbFactor->setChecked(true);
	    slider->setMinValue(-9);
	    slider->setMaxValue(+9);
	    slider->setPageStep(1);
	    slider->setTickInterval(1);
	    spinbox->setMinValue(-10);
	    spinbox->setMaxValue(+10);
	    break;
	}
	case MODE_PERCENT: {
	    rbPercentage->setChecked(true);

	    slider->setMinValue(1);
	    slider->setMaxValue(10*100);
	    slider->setPageStep(100);
	    slider->setTickInterval(1*100);
	    spinbox->setMinValue(1);
	    spinbox->setMaxValue(+10*100);
	    break;
	}
	case MODE_DECIBEL: {
	    rbLogarithmic->setChecked(true);

	    slider->setMinValue(-21);
	    slider->setMaxValue(+21);
	    slider->setPageStep(6);
	    slider->setTickInterval(6);
	    spinbox->setMinValue(-21);
	    spinbox->setMaxValue(+21);
	    break;
	}
    }

    // update the value in the display
    m_factor = value;
    updateDisplay(value);
    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void VolumeDialog::modeChanged(bool)
{
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    if (rbFactor->isChecked())      setMode(MODE_FACTOR);
    if (rbPercentage->isChecked())  setMode(MODE_PERCENT);
    if (rbLogarithmic->isChecked()) setMode(MODE_DECIBEL);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void VolumeDialog::updateDisplay(double value)
{
//    debug("VolumeDialog::updateDisplay(%f)", value); // ###
    int new_spinbox_value = 0;
    int new_slider_value  = 0;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;
    m_factor = value;

    switch (m_mode) {
	case MODE_FACTOR: {
	    // -1 => /2
	    //  0 => x1
	    // +1 => x2
	    if ((int)rint(m_factor) >= 1) {
		// greater or equal to one -> multiply
		int new_value = (int)rint(value);
		spinbox->setPrefix("x ");
		spinbox->setSuffix("");
		spinbox->setInverse(false);
		
		new_spinbox_value = new_value;
		new_slider_value = new_value-1;
		
//		debug("VolumeDialog::updateDisplay(): factor = x%d", new_value); // ###
	    } else {
		// less than one -> divide
		int new_value = (int)rint(-1.0 / value);
		
		spinbox->setPrefix("1/");
		spinbox->setSuffix("");
		spinbox->setInverse(true);
		
		new_spinbox_value = -1*new_value;
		new_slider_value  = (new_value+1);
		
//		debug("VolumeDialog::updateDisplay(): factor = 1/%d", -1*new_value); // ###
	    }
	    
	    m_enable_updates = old_enable_updates;
	    break;
	    // return;
	}
	case MODE_PERCENT: {
	    // factor 1.0 means 100%
	    new_spinbox_value = (int)rint(value * (double)100.0);
	    new_slider_value = new_spinbox_value;
	    spinbox->setPrefix("");
	    spinbox->setSuffix("%");
	    spinbox->setInverse(false);
//	    debug("VolumeDialog::updateDisplay(): percent = %d", new_slider_value); // ###
	    break;
	}
	case MODE_DECIBEL: {
	    // factor 1.0 means 0dB
	    new_slider_value = (int)rint(20.0 * log10(value));
	    new_spinbox_value = new_slider_value;
	    if (new_spinbox_value >= 0) {
		spinbox->setPrefix(new_spinbox_value ? "+" : "+/- ");
	    } else {
		// negative value
		spinbox->setPrefix("");
	    }
	    spinbox->setSuffix(" dB");
	    spinbox->setInverse(false);
//	    debug("VolumeDialog::updateDisplay(): decibel = %d", new_spinbox_value); // ###
	    break;
	}
    }

    // update the spinbox    
    if (spinbox->value() != new_spinbox_value) spinbox->setValue(new_spinbox_value);

    // update the slider, it's inverse => top=maximum, bottom=minimum !
    int sv = slider->maxValue() + slider->minValue() - new_slider_value;
    if (slider->value() != sv) slider->setValue(sv);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void VolumeDialog::sliderChanged(int pos)
{
    if (!m_enable_updates) return;
    
    int sv = slider->maxValue() + slider->minValue() - pos;
//    debug("sliderChanged(%d), sv=%d",pos,sv); // ###
    switch (m_mode) {
	case MODE_FACTOR: {
	    // -1 <=> /2
	    //  0 <=> x1
	    // +1 <=> x2
	    if (sv >= 0) {
		m_factor = (sv + 1);
	    } else {
		m_factor = (double)-1.0 / (double)(sv - 1);
	    }
//	    debug("factor=%g, sv=%d",m_factor, sv);
	    updateDisplay(m_factor);
	    break;
	}
	case MODE_PERCENT:
	    spinboxChanged(sv);
	    break;
	case MODE_DECIBEL:
	    spinboxChanged(sv);
	    break;
    }
}

//***************************************************************************
void VolumeDialog::spinboxChanged(int pos)
{
    if (!m_enable_updates) return;
//    debug("spinboxChanged(%d)",pos); // ###

    int sv = spinbox->value();
    
    switch (m_mode) {
	case MODE_FACTOR: {
	    // multiply or divide by factor
	    // -1 <=> /2
	    //  0 <=> x1
	    // +1 <=> x2
	    if (m_factor >= 1) {
		m_factor = sv ? sv : 0.5;
	    } else {
		if (!sv) sv = 1;
		m_factor = (double)1.0 / (double)(sv);
	    }
	    break;
	}
	case MODE_PERCENT: {
	    // percentage
	    m_factor = (double)pos / (double)100.0;
	    break;
	}
	case MODE_DECIBEL: {
	    // decibel
	    m_factor = pow(10.0, pos / 20.0);
	    break;
	}
    }
    
    updateDisplay(m_factor);
}

//***************************************************************************
QStringList VolumeDialog::params()
{
    QStringList list;
    list << QString::number(m_factor);
    list << QString::number((int)m_mode);
    return list;
}

//***************************************************************************
void VolumeDialog::setParams(QStringList &params)
{
    // evaluate the parameter list
    double factor = params[0].toDouble();
    switch (params[1].toUInt()) {
	case 0: m_mode = MODE_FACTOR;  break;
	case 1: m_mode = MODE_PERCENT; break;
	case 2: m_mode = MODE_DECIBEL; break;
	default: m_mode = MODE_DECIBEL;
    }

    // update mode, using default factor 1.0
    m_factor = 1.0; // works with every mode
    setMode(m_mode);

    // update factor
    m_factor = factor;
    updateDisplay(factor);
}

//***************************************************************************
//***************************************************************************
