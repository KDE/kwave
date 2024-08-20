/***************************************************************************
  CompressionWidget.cpp  -  widget for setting ogg or mp3 compression rates
                             -------------------
    begin                : Sat Jun 14 2003
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <QObject>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolTip>
#include <QWhatsThis>

#include <KLocalizedString>

#include "libkwave/StandardBitrates.h"
#include "libkwave/String.h"

#include "BitrateWidget.h"
#include "CompressionWidget.h"

//***************************************************************************
Kwave::CompressionWidget::CompressionWidget(QWidget *parent)
    :QWidget(parent), Ui::CompressionWidgetBase()
{
    setupUi(this);

    // use well-known bitrates from MP3
    const Kwave::StandardBitrates &rates = Kwave::StandardBitrates::instance();
    abrBitrate->allowRates(rates);
    abrHighestBitrate->allowRates(rates);
    abrLowestBitrate->allowRates(rates);

    connect(rbABR, SIGNAL(toggled(bool)),
            this,  SLOT(selectABR(bool)));
    connect(chkLowestBitrate, SIGNAL(toggled(bool)),
            this,  SLOT(lowestToggled(bool)));
    connect(chkHighestBitrate, SIGNAL(toggled(bool)),
            this,  SLOT(highestToggled(bool)));
    connect(abrBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(abrChanged(int)));
    connect(abrLowestBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(lowestChanged(int)));
    connect(abrHighestBitrate, SIGNAL(valueChanged(int)),
            this, SLOT(highestChanged(int)));

    enableABR(false, false, false);
    enableVBR(false);
}

//***************************************************************************
Kwave::CompressionWidget::~CompressionWidget()
{
}

//***************************************************************************
void Kwave::CompressionWidget::init(Kwave::FileInfo &info)
{
    initInfo(lblCompressionNominalBitrate, abrBitrate,
             Kwave::INF_BITRATE_NOMINAL, info);
    initInfo(nullptr, abrHighestBitrate,
             Kwave::INF_BITRATE_UPPER, info);
    initInfo(nullptr, abrLowestBitrate,
             Kwave::INF_BITRATE_LOWER, info);
    initInfo(lblCompressionBaseQuality, sbBaseQuality,
             Kwave::INF_VBR_QUALITY, info);
    initInfo(nullptr, slBaseQuality,
             Kwave::INF_VBR_QUALITY, info);
}

//***************************************************************************
void Kwave::CompressionWidget::describeWidget(QWidget *widget,
                                              const QString &name,
                                              const QString &description)
{
    if (!widget) return;
    widget->setToolTip(description);
    widget->setWhatsThis(_("<b>") + name + _("</b><br>") + description);
}

//***************************************************************************
void Kwave::CompressionWidget::initInfo(QLabel *label, QWidget *widget,
                                        Kwave::FileProperty property,
                                        Kwave::FileInfo &info)
{
    Q_ASSERT(widget);
    if (label) label->setText(i18n(info.name(property).toLatin1()) + _(":"));
    describeWidget(widget, i18n(info.name(property).toLatin1()),
                   i18n(info.description(property).toLatin1()));
}

//***************************************************************************
void Kwave::CompressionWidget::enableABR(bool enable, bool lowest, bool highest)
{
    rbABR->setEnabled(enable);
    if (!enable) rbABR->setChecked(false);

    const bool on = (rbABR->isChecked() && enable);
    lblCompressionNominalBitrate->setEnabled(on);
    abrBitrate->setEnabled(on);
    abrHighestBitrate->setEnabled(on);
    abrLowestBitrate->setEnabled(on);
    chkHighestBitrate->setEnabled(on);
    chkLowestBitrate->setEnabled(on);

    chkLowestBitrate->setChecked(lowest);
    chkHighestBitrate->setChecked(highest);
}

//***************************************************************************
void Kwave::CompressionWidget::enableVBR(bool enable)
{
    rbVBR->setEnabled(enable);
    if (!enable) rbVBR->setChecked(false);

    const bool on = (rbVBR->isChecked() && enable);
    lblCompressionBaseQuality->setEnabled(on);
    sbBaseQuality->setEnabled(on);
    slBaseQuality->setEnabled(on);

}

//***************************************************************************
void Kwave::CompressionWidget::selectABR(bool checked)
{
    abrHighestBitrate->setEnabled(checked && chkHighestBitrate->isChecked());
    abrLowestBitrate->setEnabled( checked && chkLowestBitrate->isChecked());
}

//***************************************************************************
void Kwave::CompressionWidget::lowestToggled(bool on)
{
    if (on) {
        // if previous state was off: transition off->on
        // make sure that the lowest ABR is below the current ABR
        int abr = abrBitrate->value();
        if (abrLowestBitrate->value() > abr)
            abrLowestBitrate->setValue(abr);
    }
    abrLowestBitrate->setEnabled(chkLowestBitrate->isEnabled() && on);
}


//***************************************************************************
void Kwave::CompressionWidget::highestToggled(bool on)
{
    if (on) {
        // if previous state was off: transition off->on
        // make sure that the highest ABR is above the current ABR
        int abr = abrBitrate->value();
        if (abrHighestBitrate->value() < abr)
            abrHighestBitrate->setValue(abr);
    }
    abrHighestBitrate->setEnabled(chkHighestBitrate->isEnabled() && on);
}


//***************************************************************************
void Kwave::CompressionWidget::abrChanged(int value)
{
    if (value < abrLowestBitrate->value())
        abrLowestBitrate->setValue(value);
    if (value > abrHighestBitrate->value())
        abrHighestBitrate->setValue(value);
}

//***************************************************************************
void Kwave::CompressionWidget::lowestChanged(int value)
{
    if (value > abrBitrate->value())
        abrBitrate->setValue(value);
    if (value > abrHighestBitrate->value())
        abrHighestBitrate->setValue(value);
}

//***************************************************************************
void Kwave::CompressionWidget::highestChanged(int value)
{
    if (value < abrLowestBitrate->value())
        abrLowestBitrate->setValue(value);
    if (value < abrBitrate->value())
        abrBitrate->setValue(value);
}

//***************************************************************************
void Kwave::CompressionWidget::setBitrates(int nominal, int lower, int upper)
{
    abrLowestBitrate->setValue(lower);
    abrHighestBitrate->setValue(upper);
    abrBitrate->setValue(nominal);
}

//***************************************************************************
void Kwave::CompressionWidget::setQuality(int quality)
{
    sbBaseQuality->setValue(quality);
}

//***************************************************************************
Kwave::CompressionWidget::Mode Kwave::CompressionWidget::mode()
{
    return (rbABR->isChecked()) ? ABR_MODE : VBR_MODE;
}

//***************************************************************************
void Kwave::CompressionWidget::setMode(Kwave::CompressionWidget::Mode mode)
{
    bool abr = rbABR->isEnabled();
    bool vbr = rbVBR->isEnabled();
    switch (mode) {
        case ABR_MODE:
            rbVBR->setChecked(false);
            rbVBR->setChecked(true);
            rbVBR->setChecked(false);

            rbABR->setChecked(true);
            rbABR->setChecked(false);
            rbABR->setChecked(true);
            rbABR->setChecked(abr);
            break;
        case VBR_MODE:
            rbABR->setChecked(false);
            rbABR->setChecked(true);
            rbABR->setChecked(false);

            rbVBR->setChecked(true);
            rbVBR->setChecked(false);
            rbVBR->setChecked(true);
            rbVBR->setChecked(vbr);
            break;
    }
}

//***************************************************************************
bool Kwave::CompressionWidget::lowestEnabled()
{
    return chkLowestBitrate->isChecked();
}

//***************************************************************************
bool Kwave::CompressionWidget::highestEnabled()
{
    return chkHighestBitrate->isChecked();
}

//***************************************************************************
void Kwave::CompressionWidget::getABRrates(int &nominal,
                                           int &lowest, int &highest)
{
    nominal = abrBitrate->value();
    lowest  = abrLowestBitrate->value();
    highest = abrHighestBitrate->value();
}

//***************************************************************************
int Kwave::CompressionWidget::baseQuality()
{
    return sbBaseQuality->value();
}

//***************************************************************************
//***************************************************************************

#include "moc_CompressionWidget.cpp"
