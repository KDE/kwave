
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

#include <qobject.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qvaluelist.h>
#include <knuminput.h>

#include "BitrateWidget.h"
#include "CompressionWidget.h"

//***************************************************************************
CompressionWidget::CompressionWidget(QWidget *parent, const char *name)
    :CompressionWidgetBase(parent, name)
{
    // use well-known bitrates from MP3
    QValueList<int> rates;
    rates.append(  8000);
    rates.append( 16000);
    rates.append( 24000);
    rates.append( 32000);
    rates.append( 40000);
    rates.append( 56000);
    rates.append( 64000);
    rates.append( 80000);
    rates.append( 96000);
    rates.append(112000);
    rates.append(128000);
    rates.append(144000);
    rates.append(160000);
    rates.append(176000);
    rates.append(192000);
    rates.append(224000);
    rates.append(256000);
    rates.append(288000);
    rates.append(320000);
    rates.append(352000);
    rates.append(384000);
    rates.append(416000);
    rates.append(448000);
    abrBitrate->allowRates(rates);

    abrHighestBitrate->setSpecialValueText(i18n("no limit"));
    abrLowestBitrate->setSpecialValueText(i18n("no limit"));

    connect(rbABR, SIGNAL(toggled(bool)),
            this, SLOT(selectABR(bool)));
}

//***************************************************************************
CompressionWidget::~CompressionWidget()
{
}

//***************************************************************************
void CompressionWidget::enableABR(bool enable)
{
    rbABR->setEnabled(enable);
    if (!enable) rbABR->setChecked(false);
    
    if (!rbABR->isChecked()) {
	lblCompressionNominalBitrate->setEnabled(false);
	abrBitrate->setEnabled(false);
	abrHighestBitrate->setEnabled(false);
	abrLowestBitrate->setEnabled(false);
	chkHighestBitrate->setEnabled(false);
	chkLowestBitrate->setEnabled(false);
    }
}

//***************************************************************************
void CompressionWidget::enableVBR(bool enable)
{
    rbVBR->setEnabled(enable);
    if (!enable) rbVBR->setChecked(false);

    if (!rbVBR->isChecked()) {
	lblCompressionBaseQuality->setEnabled(false);
	sbBaseQuality->setEnabled(false);
	slBaseQuality->setEnabled(false);
    }
}

//***************************************************************************
void CompressionWidget::selectABR(bool checked)
{
    abrHighestBitrate->setEnabled(checked && chkHighestBitrate->isChecked());
    abrLowestBitrate->setEnabled( checked && chkLowestBitrate->isChecked());
}

//***************************************************************************
void CompressionWidget::setBitrates(int nominal, int lower, int upper)
{
    qDebug("CompressionWidget::setBitrates(%d,%d,%d)",nominal,lower,upper); // ###
    Q_ASSERT((!lower) || (nominal >= lower));
    Q_ASSERT((!upper) || (nominal <= upper));
    if (lower && (nominal < lower)) nominal = lower;
    if (upper && (nominal > upper)) nominal = upper;
    
    abrHighestBitrate->setValue(upper);
    abrLowestBitrate->setValue(lower);
    abrBitrate->setValue(nominal);
}

//***************************************************************************
void CompressionWidget::setQuality(int quality)
{
    sbBaseQuality->setValue(quality);
}

//***************************************************************************
CompressionWidget::Mode CompressionWidget::mode()
{
    return (rbABR->isChecked()) ? ABR_MODE : VBR_MODE;
}

//***************************************************************************
void CompressionWidget::setMode(CompressionWidget::Mode mode)
{
    switch (mode) {
	case ABR_MODE:
	    rbVBR->setChecked(false);
	    rbABR->setChecked(true);
	    break;
	case VBR_MODE:
	    rbABR->setChecked(false);
	    rbVBR->setChecked(true);
	    break;
    }
}

//***************************************************************************
void CompressionWidget::getABRrates(int &nominal, int &lower, int &upper)
{
    nominal = abrBitrate->value();
    lower = (chkLowestBitrate->isChecked())  ? abrLowestBitrate->value() : 0;
    upper = (chkHighestBitrate->isChecked()) ? abrHighestBitrate->value() : 0;
}

int CompressionWidget::baseQuality()
{
    return sbBaseQuality->value();
}

//***************************************************************************
//***************************************************************************
