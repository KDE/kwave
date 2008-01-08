/*************************************************************************
    LabelPropertiesWidget.cpp  -  dialog for editing label properties
                             -------------------
    begin                : Sun Sep 03 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#include <QLineEdit>

#include <kapplication.h>
#include <kconfig.h>

#include "libgui/SelectTimeWidget.h"
#include "libgui/LabelPropertiesWidget.h"

/** name of the section in the config file */
#define CONFIG_SECTION "LabelProperties"

//***************************************************************************
LabelPropertiesWidget::LabelPropertiesWidget(QWidget *parent)
    :QDialog(parent), LabelPropertiesWidgetBase(),
    m_length(0), m_sample_rate(0)
{
    Q_ASSERT(time);
    if (time) time->setTitle(0);
    setFixedSize(sizeHint());
}

//***************************************************************************
LabelPropertiesWidget::~LabelPropertiesWidget()
{
}

//***************************************************************************
void LabelPropertiesWidget::setLabelIndex(unsigned int index)
{
    Q_ASSERT(lblIndex);
    if (lblIndex) lblIndex->setText(QString::number(index));
}

//***************************************************************************
void LabelPropertiesWidget::setLabelPosition(unsigned int pos,
	unsigned int length, qreal rate)
{
    Q_ASSERT(time);
    if (!time) return;

    // store the length and rate, for calls to labelPosition()
    m_length = length;
    m_sample_rate = rate;

    // set the current position, always by samples
    time->init(SelectTimeWidget::bySamples, pos, rate, 0, length);

    // restore the previous selection mode and set it
    const KConfigGroup cfg = KGlobal::config()->group(CONFIG_SECTION);
    bool ok;
    QString str = cfg.readEntry("mode");
    int m = str.toInt(&ok);
    if (ok) time->setMode(static_cast<SelectTimeWidget::Mode>(m));
}

//***************************************************************************
void LabelPropertiesWidget::setLabelName(const QString &name)
{
    Q_ASSERT(edDescription);
    if (edDescription) edDescription->setText(name);
}

//***************************************************************************
unsigned int LabelPropertiesWidget::labelPosition()
{
    Q_ASSERT(time);
    if (!time) return 0;

    qreal pos = time->time();
    unsigned int pos_in_samples = 0;
    switch (time->mode()) {
	case SelectTimeWidget::bySamples:
	    // already in samples
	    pos_in_samples = (unsigned int)pos;
	    break;
	case SelectTimeWidget::byTime:
	    // convert milliseconds to samples
	    pos_in_samples = (unsigned int)rint(pos / 1E3 * m_sample_rate);
	    break;
	case SelectTimeWidget::byPercents:
	    // convert from percents to samples
	    pos_in_samples =
		(unsigned int)((qreal)m_length * (pos / 100.0));
	    break;
    }

    return pos_in_samples;
}

//***************************************************************************
QString LabelPropertiesWidget::labelName()
{
    Q_ASSERT(edDescription);
    return (edDescription) ? edDescription->text() : "";
}

//***************************************************************************
void LabelPropertiesWidget::saveSettings()
{
    // restore the previous selection mode and set it
    KConfigGroup cfg = KGlobal::config()->group(CONFIG_SECTION);
    QString str;
    str.setNum(static_cast<int>(time->mode()));
    cfg.writeEntry("mode", str);
    cfg.sync();
}

//***************************************************************************
#include "LabelPropertiesWidget.moc"
//***************************************************************************
//***************************************************************************
