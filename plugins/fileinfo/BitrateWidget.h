/***************************************************************************
        BitrateWidget.h  -  widget selecting a bitrate for MP3 or Ogg/Vorbis
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

#ifndef _BITRATE_WIDGET_H_
#define _BITRATE_WIDGET_H_

#include "config.h"

#include <QWidget>
#include <QList>

#include "ui_BitrateWidgetBase.h"

class BitrateWidget: public QWidget,
                     public Ui::BitrateWidgetBase
{
    Q_OBJECT
public:
    /** Constructor */
    BitrateWidget(QWidget *parent);

    /** Destructor */
    virtual ~BitrateWidget();

    /** sets a new current value */
    virtual void setValue(int bitrate);

    /** returns the currently selected value */
    virtual int value();

    /** set a new special value text */
    virtual void setSpecialValueText(const QString &text);

    /** sets a list of allowed bitrates */
    virtual void allowRates(const QList<int> &list);

signals:

    /** emitted whenever the bitrate setting has been modified */
    void valueChanged(int value);

protected slots:

    /** slider of ABR bitrate changed */
    void sliderChanged(int value);

    /** spinbox of ABR bitrate changed */
    void spinboxChanged(int value);

    /** slider has been released -> snap to nearest available value */
    void snapInSlider();

protected:

    /** find the nearest bitrate index of a current position */
    int nearestIndex(int rate);

private:

    /** list of allowed bitrates, sorted ascending */
    QList<int> m_rates;

};

#endif /* _BITRATE_WIDGET_H_ */
