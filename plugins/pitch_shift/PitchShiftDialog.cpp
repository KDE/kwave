/***************************************************************************
   PitchShiftDialog.cpp  -  dialog for the "pitch_shift" plugin
                             -------------------
    begin                : Sun Mar 23 2003
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

#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>

#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/InvertableSpinBox.h"

#include "PitchShiftDialog.h"

//***************************************************************************
Kwave::PitchShiftDialog::PitchShiftDialog(QWidget *parent)
    :QDialog(parent), Ui::PitchShiftDlg(), Kwave::PluginSetupDialog(),
     m_speed(1.0), m_frequency(5.0), m_mode(MODE_FACTOR),
     m_enable_updates(true)
{
    setupUi(this);
    setModal(true);

    setMode(m_mode);

    // process changed in mode selection
    connect(rbFactor, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));
    connect(rbPercentage, SIGNAL(toggled(bool)),
            this, SLOT(modeChanged(bool)));

    // changes in the speed slider or spinbox
    connect(slSpeed, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(sbSpeed, SIGNAL(valueChanged(int)),
            this, SLOT(spinboxChanged(int)));

    // changes in the frequency spinbox
    connect(sbFrequency, SIGNAL(valueChanged(int)),
            this, SLOT(frequencyChanged(int)));

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
    setFixedHeight(sizeHint().height());
    int w = (height() * 3) / 5;
    if (width() < w) resize(w, height());

    connect(btHelp->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::PitchShiftDialog::~PitchShiftDialog()
{
    // better stop pre-listen now
    listenToggled(false);
}

//***************************************************************************
void Kwave::PitchShiftDialog::setMode(Mode mode)
{
    double speed = m_speed;
    m_mode = mode;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    switch (m_mode) {
        case MODE_FACTOR: {
            rbFactor->setChecked(true);

            slSpeed->setMinimum(-9);
            slSpeed->setMaximum(+4);
            slSpeed->setPageStep(1);
            slSpeed->setTickInterval(1);

            sbSpeed->setMinimum(-10);
            sbSpeed->setMaximum(+10);
            sbSpeed->setSingleStep(1);
            break;
        }
        case MODE_PERCENT: {
            rbPercentage->setChecked(true);

            slSpeed->setMinimum(1);
            slSpeed->setMaximum(400);
            slSpeed->setPageStep(10);
            slSpeed->setTickInterval(50);

            sbSpeed->setMinimum(1);
            sbSpeed->setMaximum(400);
            sbSpeed->setSingleStep(1);
            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    // update the spped value in the display
    m_speed = speed;
    updateSpeed(m_speed);
    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::PitchShiftDialog::modeChanged(bool)
{
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    if (rbFactor->isChecked())      setMode(MODE_FACTOR);
    if (rbPercentage->isChecked())  setMode(MODE_PERCENT);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::PitchShiftDialog::updateSpeed(double speed)
{
    int new_spinbox_value = 0;
    int new_slider_value  = 0;
    bool old_enable_updates = m_enable_updates;
    m_enable_updates = false;

    switch (m_mode) {
        case MODE_FACTOR: {
            // -1 => /2
            //  0 => x1
            // +1 => x2
            if (Kwave::toInt(m_speed) >= 1) {
                // greater or equal to one -> multiply
                int new_value = Kwave::toInt(speed);
                sbSpeed->setPrefix(_("x "));
                sbSpeed->setSuffix(_(""));
                sbSpeed->setInverse(false);

                new_spinbox_value = new_value;
                new_slider_value = new_value-1;
            } else {
                // less than one -> divide
                int new_value = Kwave::toInt(-1.0 / speed);

                sbSpeed->setPrefix(_("1/"));
                sbSpeed->setSuffix(_(""));
                sbSpeed->setInverse(true);

                new_spinbox_value = -1*new_value;
                new_slider_value  = (new_value+1);
            }

            m_enable_updates = old_enable_updates;
            break;
        }
        case MODE_PERCENT: {
            // factor 1.0 means 100%
            new_spinbox_value = Kwave::toInt(rint(speed * 100.0));
            new_slider_value = new_spinbox_value;
            sbSpeed->setPrefix(_(""));
            sbSpeed->setSuffix(_("%"));
            sbSpeed->setInverse(false);
            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    // update the spinbox
    if (sbSpeed->value() != new_spinbox_value)
        sbSpeed->setValue(new_spinbox_value);

    // update the slider
    if (slSpeed->value() != new_slider_value)
        slSpeed->setValue(new_slider_value);

    m_enable_updates = old_enable_updates;
}

//***************************************************************************
void Kwave::PitchShiftDialog::sliderChanged(int pos)
{
    if (!m_enable_updates) return;

    double last_speed = m_speed;

    switch (m_mode) {
        case MODE_FACTOR: {
            // -1 <=> /2
            //  0 <=> x1
            // +1 <=> x2
            if (pos >= 0) {
                m_speed = (pos + 1);
            } else {
                m_speed = -1.0 / static_cast<double>(pos - 1);
            }
            updateSpeed(m_speed);
            break;
        }
        case MODE_PERCENT:
            spinboxChanged(pos);
            break;
        DEFAULT_IMPOSSIBLE;
    }

    // emit changes
    if (!qFuzzyCompare(m_speed, last_speed)) {
        emit changed(m_speed, m_frequency);
    }
}

//***************************************************************************
void Kwave::PitchShiftDialog::spinboxChanged(int pos)
{
    if (!m_enable_updates) return;

    double last_speed = m_speed;
    int sv = sbSpeed->value();
    switch (m_mode) {
        case MODE_FACTOR: {
            // multiply or divide by factor
            // -1 <=> /2
            //  0 <=> x1
            // +1 <=> x2
            if (m_speed >= 1) {
                m_speed = sv ? sv : 0.5;
            } else {
                if (!sv) sv = 1;
                m_speed = 1.0 / static_cast<double>(sv);
            }
            break;
        }
        case MODE_PERCENT: {
            // percentage
            m_speed = static_cast<double>(pos) / 100.0;
            break;
        }
        DEFAULT_IMPOSSIBLE;
    }

    // emit changes
    if (!qFuzzyCompare(m_speed, last_speed)) {
        emit changed(m_speed, m_frequency);
    }

    updateSpeed(m_speed);
}

//***************************************************************************
void Kwave::PitchShiftDialog::frequencyChanged(int pos)
{
    // emit changes
    if (!qFuzzyCompare(m_frequency, pos)) {
        m_frequency = pos;
        emit changed(m_speed, m_frequency);
    }
}

//***************************************************************************
QStringList Kwave::PitchShiftDialog::params()
{
    QStringList list;
    list << QString::number(m_speed);
    list << QString::number(m_frequency);
    list << QString::number(static_cast<int>(m_mode));
    return list;
}

//***************************************************************************
void Kwave::PitchShiftDialog::setParams(QStringList &params)
{
    // evaluate the parameter list
    double speed = params[0].toDouble();
    m_frequency  = params[1].toDouble();
    switch (params[2].toUInt()) {
        case 0: m_mode = MODE_FACTOR;  break;
        case 1: m_mode = MODE_PERCENT; break;
        default: m_mode = MODE_PERCENT;
    }

    // update mode, using default factor 1.0
    m_speed = 1.0; // works with every mode
    setMode(m_mode);

    // update speed factor
    m_speed = speed;
    updateSpeed(speed);
}

//***************************************************************************
void Kwave::PitchShiftDialog::listenToggled(bool listen)
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
void Kwave::PitchShiftDialog::listenStopped()
{
    Q_ASSERT(btListen);
    if (!btListen) return;

    btListen->setChecked(false);
}


//***************************************************************************
void Kwave::PitchShiftDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("plugin_sect_pitch_shift"));
}

//***************************************************************************
//***************************************************************************

#include "moc_PitchShiftDialog.cpp"
