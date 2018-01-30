/***************************************************************************
    FrequencyResponseWidget.h  -  displays a frequency response
			     -------------------
    begin                : Mar 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef FREQUENCY_RESPONSE_WIDGET_H
#define FREQUENCY_RESPONSE_WIDGET_H

#include "config.h"

#include <QtGlobal>
#include <QObject>
#include <QWidget>

class QPaintEvent;

namespace Kwave
{

    class TransmissionFunction;

    /**
     * displays a frequency response
     */
    class Q_DECL_EXPORT FrequencyResponseWidget: public QWidget
    {
	Q_OBJECT

    public:

	/** Constructor */
	explicit FrequencyResponseWidget(QWidget *parent);

	/** Destructor */
	virtual ~FrequencyResponseWidget();

	/**
	 * Initialize the widget dimensions.
	 * @param freq highest frequency, will be internally rounded
	 *             up to the next decade if necessary
	 * @param db_min lowest amplitude in decibel
	 * @param db_max highest amplitude in decibel
	 */
	virtual void init(double freq, int db_min, int db_max);

	/** Set a new transmission function and update the display */
	virtual void setFilter(Kwave::TransmissionFunction *func);

	/** @see QWidget::paintEvent() */
        virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

    private:

	/** highest frequency, rounded up to the next decade */
	double m_f_max;

	/** lowest amplitude in decibel */
	int m_db_min;

	/** highest amplitude in decibel */
	int m_db_max;

	/** number of decades, calculated from m_f_max */
	int m_decades;

	/**
	 * Pointer to a transmission function object, used for
	 * painting the frequency response.
	 */
	Kwave::TransmissionFunction *m_function;

    };
}

#endif /* FREQUENCY_RESPONSE_WIDGET_H */

//***************************************************************************
//***************************************************************************
