/***************************************************************************
     SelectTimeWidget.h  -  widget for selecting a time or range
                             -------------------
    begin                : Thu Jan 16 2003
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

#ifndef _SELECT_TIME_WIDGET_H_
#define _SELECT_TIME_WIDGET_H_

#include "config.h"
#include <qobject.h>
#include <qtimer.h>
#include <qwidget.h>

#include "SelectTimeWidgetBase.uih.h"

class SelectTimeWidget: public SelectTimeWidgetBase
{
    Q_OBJECT
public:
    /** possible mode of the selection */
    typedef enum {
	byTime = 0, /**< range in milliseconds */
	bySamples,  /**< number of samples */
	byPercents  /**< percentage of whole signal */
    } Mode;

    /**
     * Constructor
     * @param widget pointer to the parent widget
     * @param mode selectionMode: byTime, bySamples, byPercents
     * @param range length of the selection in ms, samples or percent
     * @param sample_rate number of samples per second, needed for
     *                    converting between samples and time
     * @param offset start of the selection [samples]
     * @param signal_length length of the signal in samples, needed
     *                      for converting samples to percentage
     */
    SelectTimeWidget(QWidget *widget, const char *name);

    /**
     * Constructor
     * @param widget pointer to the parent widget
     * @param mode selectionMode: byTime, bySamples, byPercents
     * @param range length of the selection in ms, samples or percent
     * @param sample_rate number of samples per second, needed for
     *                    converting between samples and time
     * @param offset start of the selection [samples]
     * @param signal_length length of the signal in samples, needed
     *                      for converting samples to percentage
     */
    virtual void init(Mode mode, double range, double sample_rate,
                      unsigned int offset, unsigned int signal_length);

    /** Destructor */
    virtual ~SelectTimeWidget();

    /** Sets a new selection mode */
    void setMode(Mode new_mode);

    /** Returns the current selection mode (byTime, bySamples, byPercents) */
    Mode mode() { return m_mode; };

    /** Returns the number of ms, samples or percents */
    double time() { return m_range; };

    /** Sets the title of the, shown in the frame around the controls */
    void setTitle(const QString title);

signals:

    /** Emitted when the value has been changed */
    void valueChanged(unsigned int samples);

public slots:

    /**
     * Sets/updates the start offset from which the selection starts.
     * The current selection length will be reduced to fit in the
     * available range.
     * @param offset index of the first selected sample
     */
    void setOffset(unsigned int offset);    
    
private slots:

    /** called whenever one of the radio buttons changed it's state */
    void modeChanged(int);

    /** called whenever one of the time controls changed their value */
    void timeChanged(int);

    /** checks for new values in the sample edit field */
    void checkNewSampleEdit();

    /** called when sample count has changed */
    void samplesChanged(double);

    /** called when percentage changed */
    void percentsChanged(int p);

private:

    /** connects all widgets for getting informed about changes */
    void connect();

    /** disconnect all widgets for avoiding recursion */
    void disconnect();
    
private:

    /** selectionMode: byTime, bySamples or byPercent */
    Mode m_mode;

    /** selected range in ms, samples or percent */
    double m_range;

    /** sample rate [samples/second] */
    double m_rate;

    /** start offset of the selectioh [samples] */
    unsigned int m_offset;

    /** length of the whole signal [samples] */
    unsigned int m_length;

    /** timer that checks for changes in the sample edit */
    QTimer m_timer;

};

#endif /* _SELECT_TIME_WIDGET_H_ */
