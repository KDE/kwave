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
Kwave::LabelPropertiesWidget::LabelPropertiesWidget(QWidget *parent)
    :QDialog(parent), LabelPropertiesWidgetBase(),
     m_length(0), m_sample_rate(0)
{
    setupUi(this);
    Q_ASSERT(time);
    if (time) time->setTitle(QString());
    setFixedSize(sizeHint());
}

//***************************************************************************
Kwave::LabelPropertiesWidget::~LabelPropertiesWidget()
{
}

//***************************************************************************
void Kwave::LabelPropertiesWidget::setLabelIndex(unsigned int index)
{
    Q_ASSERT(lblIndex);
    if (lblIndex) lblIndex->setText(QString::number(index));
}

//***************************************************************************
void Kwave::LabelPropertiesWidget::setLabelPosition(sample_index_t pos,
	sample_index_t length, double rate)
{
    Q_ASSERT(time);
    if (!time) return;

    // store the length and rate, for calls to labelPosition()
    m_length = length;
    m_sample_rate = rate;

    // set the current position, always by samples
    time->init(Kwave::SelectTimeWidget::bySamples, pos, rate, 0, length);

    // restore the previous selection mode and set it
    const KConfigGroup cfg = KGlobal::config()->group(CONFIG_SECTION);
    bool ok;
    QString str = cfg.readEntry("mode");
    int m = str.toInt(&ok);
    if (ok) time->setMode(static_cast<Kwave::SelectTimeWidget::Mode>(m));
}

//***************************************************************************
void Kwave::LabelPropertiesWidget::setLabelName(const QString &name)
{
    Q_ASSERT(edDescription);
    if (edDescription) edDescription->setText(name);
}

//***************************************************************************
sample_index_t Kwave::LabelPropertiesWidget::labelPosition()
{
    Q_ASSERT(time);
    return (time) ? time->samples() : 0;
}

//***************************************************************************
QString Kwave::LabelPropertiesWidget::labelName()
{
    Q_ASSERT(edDescription);
    return (edDescription) ? edDescription->text() : "";
}

//***************************************************************************
void Kwave::LabelPropertiesWidget::saveSettings()
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
