/***************************************************************************
        NoiseDialog.cpp  -  dialog for the "noise" plugin
                             -------------------
    begin                : Sat Sep 28 2013
    copyright            : (C) 2013 by Thomas Eschenbacher
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

#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>

#include <KI18n/KLocalizedString>
#include <ktoolinvocation.h>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/Colors.h"
#include "libgui/CurveWidget.h"
#include "libgui/InvertableSpinBox.h"
#include "libgui/ImageView.h"
#include "libgui/OverViewCache.h"
#include "libgui/ScaleWidget.h"

#include "NoiseDialog.h"

//***************************************************************************
Kwave::NoiseDialog::NoiseDialog(QWidget *parent,
                                  Kwave::OverViewCache *overview_cache)
    :QDialog(parent), Kwave::PluginSetupDialog(), Ui::NoiseDlg(),
     m_noise(0.1), m_mode(MODE_DECIBEL),
     m_enable_updates(true), m_overview_cache(overview_cache)
{
    setupUi(this);
    setModal(true);

    // process changed in mode selection
    connect(rbPercentage, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));
    connect(rbLogarithmic, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));

    // changes in the slider or spinbox
    connect(slider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(spinboxChanged(int)));
    // click to the "Listen" button
    connect(btListen, SIGNAL(toggled(bool)),
            this, SLOT(listenToggled(bool)));

    // force activation of the layout
    layout()->activate();

    // give the preview image a odd height, for better symmetry
    int h = preview->height();
    if (~h & 1) h++;
    preview->setFixedHeight(h);

    // expand the "Listen" button to it's maximum width
    listenToggled(true);
    if (btListen->width() > btListen->minimumWidth())
        btListen->setMinimumWidth(btListen->width());
    listenToggled(false);
    if (btListen->width() > btListen->minimumWidth())
        btListen->setMinimumWidth(btListen->width());

    // set the initial size of the dialog
    h     = (sizeHint().height() * 12) / 10;
    int w = (3 * h) / 4;
    if (sizeHint().width() > w) w = sizeHint().width();
    setFixedSize(w, h);

    // set default: 10%
    setMode(m_mode);
    updateDisplay(+0.1);

    connect(buttonBox_Help->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::NoiseDialog::~NoiseDialog()
{
    // better stop pre-listen now
    listenToggled(false);

    delete m_overview_cache;
    m_overview_cache = 0;
}

//***************************************************************************
void Kwave::NoiseDialog::setMode(Mode mode)
{
    double value = m_noise;
    m_mode = mode;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    switch (m_mode) {
	case MODE_PERCENT: {
	    rbPercentage->setChecked(true);

	    slider->setMinimum(1);
	    slider->setMaximum(100);
	    slider->setPageStep(100);
	    slider->setTickInterval(10);
	    spinbox->setMinimum(1);
	    spinbox->setMaximum(100);
	    break;
	}
	case MODE_DECIBEL: {
	    rbLogarithmic->setChecked(true);

	    slider->setMinimum(-21);
	    slider->setMaximum(0);
	    slider->setPageStep(6);
	    slider->setTickInterval(3);
	    spinbox->setMinimum(-21);
	    spinbox->setMaximum(0);
	    break;
	}
    }

    // update the value in the display
    updateDisplay(value);
    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::NoiseDialog::modeChanged(bool)
{
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    if (rbPercentage->isChecked())  setMode(MODE_PERCENT);
    if (rbLogarithmic->isChecked()) setMode(MODE_DECIBEL);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::NoiseDialog::updateDisplay(double value)
{
    int new_spinbox_value = 0;
    int new_slider_value  = 0;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    Kwave::OverViewCache::MinMaxArray m_minmax;

    if (!qFuzzyCompare(m_noise, value)) {

	// take over the new factor
	m_noise = value;

	// update the preview widget
	if (m_overview_cache && preview) {
	    int width  = preview->width();
	    int height = preview->height();
	    QColor color_bg    = Kwave::Colors::Normal.background;
	    QColor color_sig   = Kwave::Colors::Normal.interpolated;
	    QColor color_noise = Kwave::Colors::Normal.sample;

	    // get the min/max information
	    int count = m_overview_cache->getMinMax(width, m_minmax);

	    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
	    QPainter p;
	    p.begin(&image);
	    p.fillRect(image.rect(), color_bg);

	    // calculate scaling factor and (noise level / 2) in pixel
	    const int middle = height >> 1;
	    const int noise_2 = Kwave::toInt(middle * m_noise);

	    /*
	     * mixing of noise goes like this:
	     *
	     * y[t] = x[t] * (1 - n) + noise(t) * n;
	     *
	     * and has this effect on min/max:
	     *
	     * [---noise---]          [---noise---]
	     *     [min -------------------max]
	     * ^     ^    ^           ^     ^     ^
	     * |     |    |           |     |     |
	     * y1    y2   y3          y4    y5    y6
	     */

	    for (int x = 0; x < count; ++x) {
		int y2 = Kwave::toInt((sample2double(m_minmax[x].min) *
		    (1.0 - m_noise)) * middle);
		int y5 = Kwave::toInt((sample2double(m_minmax[x].max) *
		    (1.0 - m_noise)) * middle);
		int y1 = y2 - noise_2;
		int y3 = y2 + noise_2;
		int y4 = y5 - noise_2;
		int y6 = y5 + noise_2;

		if (y4 > y3) {
		    // noise around "min" [y1 ... y3]
		    p.setPen(color_noise);
		    p.drawLine(x, middle - y3, x, middle - y1);

		    // noise around "max" [y4 ... y6]
		    p.drawLine(x, middle - y6, x, middle - y4);

		    // original signal [y3 ... y4 ]
		    p.setPen(color_sig);
		    p.drawLine(x, middle - y4, x, middle - y3);
		} else {
		    // only noise [y1 ... y6]
		    p.setPen(color_noise);
		    p.drawLine(x, middle - y6, x, middle - y1);
		}
	    }

	    // zero line
	    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	    p.setPen(Kwave::Colors::Normal.zero);
	    p.drawLine(0, middle, width - 1, middle);

	    p.end();

	    // update the image view
	    preview->setImage(image);
	}

	// emit the noise level change to the plugin
	emit levelChanged(m_noise);
    }

    switch (m_mode) {
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
	    if (!qFuzzyIsNull(value))
		new_slider_value = Kwave::toInt(rint(20.0 * log10(value)));
	    else
		new_slider_value = 0;
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
void Kwave::NoiseDialog::sliderChanged(int pos)
{
    if (!m_enable_updates) return;

    int sv = slider->maximum() + slider->minimum() - pos;
    spinboxChanged(sv);
}

//***************************************************************************
void Kwave::NoiseDialog::spinboxChanged(int pos)
{
    if (!m_enable_updates) return;

    double factor = m_noise;

    switch (m_mode) {
	case MODE_PERCENT:
	    // percentage
	    factor = static_cast<double>(pos) / 100.0;
	    break;
	case MODE_DECIBEL:
	    // decibel
	    factor = pow(10.0, pos / 20.0);
	    break;
    }

    updateDisplay(factor);
}

//***************************************************************************
QStringList Kwave::NoiseDialog::params()
{
    QStringList list;
    list << QString::number(m_noise);
    list << QString::number(static_cast<int>(m_mode));
    return list;
}

//***************************************************************************
void Kwave::NoiseDialog::setParams(QStringList &params)
{
    // evaluate the parameter list
    double factor = params[0].toDouble();
    factor = qBound<double>(0.0, factor, 1.0);

    switch (params[1].toUInt()) {
	case 0:  m_mode = MODE_PERCENT; break;
	case 1:  m_mode = MODE_DECIBEL; break;
	default: m_mode = MODE_DECIBEL;
    }

    // update mode, using default factor 1.0
    m_noise = 1.0; // works with every mode
    setMode(m_mode);

    // update factor
    updateDisplay(factor);
}

//***************************************************************************
void Kwave::NoiseDialog::listenToggled(bool listen)
{
    Q_ASSERT(btListen);
    if (!btListen) return;

    if (listen) {
	// start pre-listen mode
	emit startPreListen();
	btListen->setText(i18n("&Stop"));
    } else {
	// stop pre-listen mode
	emit stopPreListen();
	btListen->setText(i18n("&Listen"));
    }
}

//***************************************************************************
void Kwave::NoiseDialog::listenStopped()
{
    if (btListen) btListen->setChecked(false);
}

//***************************************************************************
void Kwave::NoiseDialog::invokeHelp()
{
    KToolInvocation::invokeHelp(_("plugin_sect_noise"));
}

//***************************************************************************
#include "NoiseDialog.moc"
//***************************************************************************
//***************************************************************************
