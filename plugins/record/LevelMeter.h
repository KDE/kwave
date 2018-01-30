/*************************************************************************
           LevelMeter.h  -  multi-track audio level meter
                             -------------------
    view copyright       : Copyright 2002 Rik Hemsley (rikkus) <rik@kde.org>

    begin                : Mon Nov 17 2003
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

#ifndef LEVEL_METER_H
#define LEVEL_METER_H

#include "config.h"

#include <QColor>
#include <QQueue>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

class QPaintEvent;
class QTimer;

namespace Kwave
{
    class LevelMeter: public QWidget
    {
	Q_OBJECT
    public:
	/** Constructor */
	explicit LevelMeter(QWidget *parent);

	/** Destructor */
	virtual ~LevelMeter();

	/** @see QWidget::paintEvent */
        virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;

	/** @see QWidget::resizeEvent */
        virtual void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

    public slots:

	/** sets the number of tracks that the display should use */
	virtual void setTracks(unsigned int tracks);

	/**
	 * sets the sample rate for interpreting the samples used
	 * for updating the display.
	 */
	virtual void setSampleRate(double rate);

	/**
	 * Updates a apecific track
	 * @param track index of the track
	 * @param buffer array with samples
	 */
	virtual void updateTrack(unsigned int track,
	                         const Kwave::SampleArray &buffer);

	/**
	 * Resets all meters to zero
	 */
	virtual void reset();

	/**
	 * Redraws the whole widget
	 * @author (original idea taken from) Rik Hemsley (rikkus) <rik@kde.org>
	 *          Copyright 2002
	 */
	virtual void drawContents();

    protected slots:

	/** Called via m_timer to update the bar(s) */
	virtual void timedUpdate();

    protected:

	/**
	 * Enqueue a pair of fast and peak value of a track for later timed
	 * update. If the queue already contains the maximum number of elements,
	 * the oldest ones will be removed.
	 *
	 * @param track index of the track [0...m_tracks-1]
	 * @param fast value of the fast level bar [0.0 ... 1.0]
	 * @param peak value of the peak level [0.0 ... 1.0]
	 * @param queue_depth maximum number of elements to queue
	 */
	virtual void enqueue(unsigned int track, float fast, float peak,
	                     unsigned int queue_depth);

	/**
	 * Dequeue a pair of fast and peek value of a track.
	 * @param track index of the track [0...m_tracks-1]
	 * @param fast receives the value of the fast level bar [0.0 ... 1.0]
	 * @param peak receives value of the peak level [0.0 ... 1.0]
	 * @return true if there was something to dequeue, false if
	 *         the queue was empty.
	 * @note fast and peak will not be modified if the queue was empty
	 */
	virtual bool dequeue(unsigned int track, float &fast, float &peak);

	/**
	 * Draw some scale into the meter, using 3dB steps
	 * @param p an already opened QPainter
	 */
	void drawScale(QPainter &p);

    private:

	/** number of tracks */
	int m_tracks;

	/** sample rate used for interpreting the received buffers */
	float m_sample_rate;

	/** last output value of the filter for fast updates */
	QVector<float> m_yf;

	/** last output value of the filter for peak updates */
	QVector<float> m_yp;

	/** queues with fast update values for each track */
	QVector< QQueue<float> > m_fast_queue;

	/** queues with peak values for each track */
	QVector< QQueue<float> > m_peak_queue;

	/** current fast value for each track */
	QVector<float> m_current_fast;

	/** current peak value for each track */
	QVector<float> m_current_peak;

	/** timer for display updates */
	QTimer *m_timer;

	/** color for low levels, below -3dB */
	QColor m_color_low;

	/** color for normal levels, -3dB...-1.5dB */
	QColor m_color_normal;

	/** color high levels, above -1.5dB */
	QColor m_color_high;

    };
}

#endif /* LEVEL_METER_H */

//***************************************************************************
//***************************************************************************
