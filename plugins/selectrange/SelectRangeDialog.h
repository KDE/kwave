/***************************************************************************
    SelectRangeDialog.h  -  dialog for selecting a range of samples
                             -------------------
    begin                : Sat Jun 15 2002
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

#ifndef _SELECT_RANGE_DIALOG_H_
#define _SELECT_RANGE_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include <qtimer.h>

#include "SelectRangeDlg.uih.h"
#include "SelectRangeDialog.h"
#include "SelectRangePlugin.h"

class SelectRangeDialog: public SelectRangeDlg
{
    Q_OBJECT
public:
    /** shortcut typedef */
    typedef SelectRangePlugin::SelectionMode Mode;

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
    SelectRangeDialog(QWidget *widget, Mode mode,
                      double range, double sample_rate,
                      unsigned int offset, unsigned int signal_length);

    /** Destructor */
    virtual ~SelectRangeDialog();

    /** Returns the current selection mode (byTime, bySamples, byPercents) */
    SelectRangePlugin::SelectionMode mode() { return m_mode; };

    /** Returns the number of ms, samples or percents */
    double range() { return m_range; };

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

protected:

    /** Sets a new selection mode */
    void setMode(Mode new_mode);

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

#endif /* _SELECT_RANGE_DIALOG_H_ */
