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

#include <math.h>
#include <new>

#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>

#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FrequencyResponseWidget.h"
#include "libgui/ScaleWidget.h"

#include "LowPassDialog.h"
#include "LowPassFilter.h"

//***************************************************************************
Kwave::LowPassDialog::LowPassDialog(QWidget *parent, double sample_rate)
    :QDialog(parent), Ui::LowPassDlg(), Kwave::PluginSetupDialog(),
     m_frequency(3500),
     m_sample_rate(sample_rate), m_filter(Q_NULLPTR)
{
    setupUi(this);
    setModal(true);

    // set maximum frequency to sample rate / 2
    double f_max = sample_rate / 2.0;

    slider->setMaximum(Kwave::toInt(f_max));
    spinbox->setMaximum(Kwave::toInt(f_max));

    // initialize the frequency scale widget
    scale_freq->setMinMax(0, Kwave::toInt(f_max));
    scale_freq->setLogMode(false);
    scale_freq->setUnit(i18n("Hz"));

    // initialize the attenuation scale widget
    scale_db->setMinMax(-24, +6);
    scale_db->setLogMode(false);
    scale_db->setUnit(i18n("dB"));

    // initialize the frequency response widget
    freq_response->init(f_max, -24, +6);

    // set up the low pass filter dunction
    m_filter = new(std::nothrow) Kwave::LowPassFilter();
    freq_response->setFilter(m_filter);

    // initialize the controls and the curve display
    slider->setValue(Kwave::toInt(m_frequency));
    spinbox->setValue(Kwave::toInt(m_frequency));
    updateDisplay();

    // changes in the slider or spinbox
    connect(spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(valueChanged(int)));
    // click to the "Listen" button
    connect(btListen, SIGNAL(toggled(bool)),
            this, SLOT(listenToggled(bool)));

    // expand the "Listen" button to it's maximum width
    listenToggled(true);
    if (btListen->width() > btListen->minimumWidth())
        btListen->setMinimumWidth(btListen->width());
    listenToggled(false);
    if (btListen->width() > btListen->minimumWidth())
        btListen->setMinimumWidth(btListen->width());

    // set the initial size of the dialog
    int h = (width() * 3) / 5;
    if (height() < h) resize(width(), h);
    int w = (height() * 5) / 3;
    if (width() < w) resize(w, height());

    connect(buttonBox_Help->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::LowPassDialog::~LowPassDialog()
{
    // better stop pre-listen now
    listenToggled(false);

    if (freq_response) freq_response->setFilter(Q_NULLPTR);
    if (m_filter) delete m_filter;
}

//***************************************************************************
void Kwave::LowPassDialog::valueChanged(int pos)
{
    if (Kwave::toInt(m_frequency) != pos) {
	m_frequency = pos;
	updateDisplay();

	emit changed(m_frequency);
    }
}

//***************************************************************************
QStringList Kwave::LowPassDialog::params()
{
    QStringList list;
    list << QString::number(m_frequency);
    return list;
}

//***************************************************************************
void Kwave::LowPassDialog::setParams(QStringList &params)
{
    // evaluate the parameter list
    bool ok;
    double frequency = params[0].toDouble(&ok);
    Q_ASSERT(ok);
    if (ok) m_frequency = frequency;

    slider->setValue(Kwave::toInt(m_frequency));
    spinbox->setValue(Kwave::toInt(m_frequency));

    updateDisplay();
}

//***************************************************************************
void Kwave::LowPassDialog::updateDisplay()
{
    double fs = m_sample_rate;
    if (m_filter && (fs > 0.0))
    {
        m_filter->setFrequency(QVariant(2.0 * M_PI * m_frequency / fs));
        if (freq_response) freq_response->repaint();
    }
}

//***************************************************************************
void Kwave::LowPassDialog::listenToggled(bool listen)
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
void Kwave::LowPassDialog::listenStopped()
{
    if (btListen) btListen->setChecked(false);
}

//***************************************************************************
void Kwave::LowPassDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_lowpass"));
}

//***************************************************************************
//***************************************************************************
