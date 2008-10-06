/***************************************************************************
      BitrateWidget.cpp  -  widget selecting a bitrate for MP3 or Ogg/Vorbis
			     -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
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

#include <QtGlobal>
#include <QSlider>
#include <QSpinBox>

#include "BitrateSpinBox.h"
#include "BitrateWidget.h"

/***************************************************************************/
BitrateWidget::BitrateWidget(QWidget *parent)
    :QWidget(parent),
     Ui::BitrateWidgetBase(), m_rates()
{
    setupUi(this);
    m_rates.append(0); // don't let it stay empty, that makes life easier

    connect(slider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderChanged(int)));
    connect(spinbox, SIGNAL(valueChanged(int)),
            this, SLOT(spinboxChanged(int)));
    connect(slider, SIGNAL(sliderReleased()),
            this, SLOT(snapInSlider()));
    connect(spinbox, SIGNAL(snappedIn(int)),
            slider, SLOT(setValue(int)));
}

/***************************************************************************/
BitrateWidget::~BitrateWidget()
{
}

/***************************************************************************/
void BitrateWidget::setValue(int bitrate)
{
    slider->setValue(bitrate);
    spinbox->setValue(bitrate);
}

/***************************************************************************/
int BitrateWidget::value()
{
    int value = slider->value();
    int index = nearestIndex(value);
    return m_rates[index];
}

/***************************************************************************/
void BitrateWidget::setSpecialValueText(const QString &text)
{
    spinbox->setSpecialValueText(text);
}

/***************************************************************************/
void BitrateWidget::allowRates(const QList<int> &list)
{
    int old_value = value();

    m_rates = list;
    if (m_rates.isEmpty()) m_rates.append(0);

    // set new ranges
    spinbox->allowRates(m_rates);
    slider->setMinimum(m_rates.first());
    slider->setMaximum(m_rates.last());

    setValue(old_value);
}

//***************************************************************************
int BitrateWidget::nearestIndex(int rate)
{
    // find the nearest value
    int nearest = 0;
    foreach(int i, m_rates)
	if (qAbs(i - rate) < qAbs(nearest - rate)) nearest = i;

    // find the index
    int index = m_rates.contains(nearest) ? m_rates.indexOf(nearest) : 0;

    // limit the index into a reasonable range
    if (index < 0)
	index = 0;
    if (index >= static_cast<int>(m_rates.size()))
	index = m_rates.size()-1;

    return index;
}

//***************************************************************************
void BitrateWidget::sliderChanged(int value)
{
    int index = nearestIndex(value);
    spinbox->setValue(m_rates[index]);

    emit valueChanged(value);
}

//***************************************************************************
void BitrateWidget::spinboxChanged(int value)
{
    emit valueChanged(value);
}

//***************************************************************************
void BitrateWidget::snapInSlider()
{
    int value = slider->value();
    int index = nearestIndex(value);
    slider->setValue(m_rates[index]); // snap in
}

//***************************************************************************
#include "BitrateWidget.moc"
//***************************************************************************
//***************************************************************************
