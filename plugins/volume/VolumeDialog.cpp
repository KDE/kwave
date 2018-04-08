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

#include <math.h>

#include <QColor>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>

#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/CurveWidget.h"
#include "libgui/ImageView.h"
#include "libgui/InvertableSpinBox.h"
#include "libgui/OverViewCache.h"
#include "libgui/ScaleWidget.h"

#include "VolumeDialog.h"

//***************************************************************************
Kwave::VolumeDialog::VolumeDialog(QWidget *parent,
                                  Kwave::OverViewCache *overview_cache)
    :QDialog(parent), Ui::VolumeDlg(), m_factor(1.0), m_mode(MODE_DECIBEL),
     m_enable_updates(true), m_overview_cache(overview_cache)
{
    setupUi(this);
    setModal(true);

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

    // force activation of the layout
    layout()->activate();

    // give the preview image a odd height, for better symmetry
    int h = preview->height();
    if (~h & 1) h++;
    preview->setFixedHeight(h);

    // set the initial size of the dialog
    h     = (sizeHint().height() * 12) / 10;
    int w = (3 * h) / 4;
    if (sizeHint().width() > w) w = sizeHint().width();
    setFixedSize(w, h);

    // set default: +3dB
    setMode(m_mode);
    updateDisplay(+1.412538);

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::VolumeDialog::~VolumeDialog()
{
}

//***************************************************************************
void Kwave::VolumeDialog::setMode(Mode mode)
{
    double value = m_factor;
    m_mode = mode;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    switch (m_mode) {
	case MODE_FACTOR: {
	    rbFactor->setChecked(true);
	    slider->setMinimum(-9);
	    slider->setMaximum(+9);
	    slider->setPageStep(1);
	    slider->setTickInterval(1);
	    spinbox->setMinimum(-10);
	    spinbox->setMaximum(+10);
	    break;
	}
	case MODE_PERCENT: {
	    rbPercentage->setChecked(true);

	    slider->setMinimum(1);
	    slider->setMaximum(10*100);
	    slider->setPageStep(100);
	    slider->setTickInterval(1*100);
	    spinbox->setMinimum(1);
	    spinbox->setMaximum(+10*100);
	    break;
	}
	case MODE_DECIBEL: {
	    rbLogarithmic->setChecked(true);

	    slider->setMinimum(-21);
	    slider->setMaximum(+21);
	    slider->setPageStep(6);
	    slider->setTickInterval(6);
	    spinbox->setMinimum(-21);
	    spinbox->setMaximum(+21);
	    break;
	}
    }

    // update the value in the display
    updateDisplay(value);
    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::VolumeDialog::modeChanged(bool)
{
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    if (rbFactor->isChecked())      setMode(MODE_FACTOR);
    if (rbPercentage->isChecked())  setMode(MODE_PERCENT);
    if (rbLogarithmic->isChecked()) setMode(MODE_DECIBEL);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::VolumeDialog::updateDisplay(double value)
{
    int new_spinbox_value = 0;
    int new_slider_value  = 0;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    if (!qFuzzyCompare(m_factor, value)) {
	// take over the new factor
	m_factor = value;

	// update the preview widget
	if (m_overview_cache && preview) {
	    int width  = preview->width();
	    int height = preview->height();
	    QColor fg = Qt::white;
	    QColor bg = Qt::black;

	    // get the raw preview image
	    QImage image = m_overview_cache->getOverView(
		width, height, fg, bg, m_factor);

	    // color transformation: mark the peaks in light red
	    int middle = height >> 1;
	    int red    = Kwave::toInt(middle * 0.841); // -1.5dB
	    int orange = Kwave::toInt(middle * 0.707); // -3.0dB

	    QPainter p;
	    p.begin(&image);
	    for (int y = 0; y < height; y++) {
		QColor color;

		if (y == middle) {
		    // zero line
		    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		    color = Qt::green;
		} else {
		    p.setCompositionMode(QPainter::CompositionMode_Multiply);
		    if ((y < middle - red) || (y > middle + red))
			// "red" level, -1.5dB from border
			color = Qt::red;
		    else if ((y < middle - orange) || (y > middle + orange))
			// "orange" level, -3.0dB from border
			color = Qt::yellow;
		    else
			// "normal" level
			color = preview->palette().light().color();
		}

		p.setPen(color);
		p.drawLine(0, y, width-1, y);
	    }
	    p.end();

	    // update the image view
	    preview->setImage(image);
	}
    }

    switch (m_mode) {
	case MODE_FACTOR: {
	    // -1 => /2
	    //  0 => x1
	    // +1 => x2
	    if (Kwave::toInt(rint(m_factor)) >= 1) {
		// greater or equal to one -> multiply
		int new_value = Kwave::toInt(rint(value));
		spinbox->setPrefix(_("x "));
		spinbox->setSuffix(_(""));
		spinbox->setInverse(false);

		new_spinbox_value = new_value;
		new_slider_value = new_value-1;
	    } else {
		// less than one -> divide
		int new_value = Kwave::toInt(rint(-1.0 / value));

		spinbox->setPrefix(_("1/"));
		spinbox->setSuffix(_(""));
		spinbox->setInverse(true);

		new_spinbox_value = -1*new_value;
		new_slider_value  = (new_value+1);
	    }

	    m_enable_updates = old_enable_updates;
	    break;
	    // return;
	}
	case MODE_PERCENT: {
	    // factor 1.0 means 100%
	    new_spinbox_value = Kwave::toInt(rint(value * 100.0));
	    new_slider_value = new_spinbox_value;
	    spinbox->setPrefix(_(""));
	    spinbox->setSuffix(_("%"));
	    spinbox->setInverse(false);
	    break;
	}
	case MODE_DECIBEL: {
	    // factor 1.0 means 0dB
	    new_slider_value = Kwave::toInt(rint(20.0 * log10(value)));
	    new_spinbox_value = new_slider_value;
	    if (new_spinbox_value >= 0) {
		spinbox->setPrefix(new_spinbox_value ? _("+") : _("+/- "));
	    } else {
		// negative value
		spinbox->setPrefix(_(""));
	    }
	    spinbox->setSuffix(_(" ") + i18n("dB"));
	    spinbox->setInverse(false);
	    break;
	}
    }

    // update the spinbox
    if (spinbox->value() != new_spinbox_value) spinbox->setValue(new_spinbox_value);

    // update the slider, it's inverse => top=maximum, bottom=minimum !
    int sv = slider->maximum() + slider->minimum() - new_slider_value;
    if (slider->value() != sv) slider->setValue(sv);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::VolumeDialog::sliderChanged(int pos)
{
    if (!m_enable_updates) return;

    int sv = slider->maximum() + slider->minimum() - pos;
//    qDebug("sliderChanged(%d), sv=%d",pos,sv); // ###
    switch (m_mode) {
	case MODE_FACTOR: {
	    double factor = m_factor;
	    // -1 <=> /2
	    //  0 <=> x1
	    // +1 <=> x2
	    if (sv >= 0) {
		factor = (sv + 1);
	    } else {
		factor = -1.0 / static_cast<double>(sv - 1);
	    }
//	    qDebug("factor=%g, sv=%d",factor, sv);
	    updateDisplay(factor);
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
void Kwave::VolumeDialog::spinboxChanged(int pos)
{
    if (!m_enable_updates) return;
//    qDebug("spinboxChanged(%d)",pos); // ###

    int sv = spinbox->value();
    double factor = m_factor;

    switch (m_mode) {
	case MODE_FACTOR: {
	    // multiply or divide by factor
	    // -1 <=> /2
	    //  0 <=> x1
	    // +1 <=> x2
	    if (factor >= 1) {
		factor = sv ? sv : 0.5;
	    } else {
		if (!sv) sv = 1;
		factor = 1.0 / static_cast<double>(sv);
	    }
	    break;
	}
	case MODE_PERCENT: {
	    // percentage
	    factor = static_cast<double>(pos) / 100.0;
	    break;
	}
	case MODE_DECIBEL: {
	    // decibel
	    factor = pow(10.0, pos / 20.0);
	    break;
	}
    }

    updateDisplay(factor);
}

//***************************************************************************
QStringList Kwave::VolumeDialog::params()
{
    QStringList list;
    list << QString::number(m_factor);
    list << QString::number(static_cast<int>(m_mode));
    return list;
}

//***************************************************************************
void Kwave::VolumeDialog::setParams(QStringList &params)
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
    updateDisplay(factor);
}


//***************************************************************************
void Kwave::VolumeDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_volume"));
}

//***************************************************************************
//***************************************************************************
