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

#include "libgui/SelectTimeWidget.h"
#include "SelectRangeDlg.uih.h"
#include "SelectRangeDialog.h"

class SelectRangeDialog: public SelectRangeDlg
{
    Q_OBJECT
public:
    /** shortcut typedef */
    typedef SelectTimeWidget::Mode Mode;

    /**
     * Constructor
     * @param widget pointer to the parent widget
     * @param start_mode selectionMode for the start position,
     *                   byTime, bySamples, byPercents
     * @param range_mode selectionMode for the range
     * @param range length of the selection in ms, samples or percent
     * @param sample_rate number of samples per second, needed for
     *                    converting between samples and time
     * @param offset start of the selection [samples]
     * @param signal_length length of the signal in samples, needed
     *                      for converting samples to percentage
     */
    SelectRangeDialog(QWidget *widget, Mode start_mode, Mode range_mode,
                      double range, double sample_rate,
                      unsigned int offset, unsigned int signal_length);

    /** Destructor */
    virtual ~SelectRangeDialog();

    /**
     * Returns the current selection mode for the start position
     * (byTime, bySamples, byPercents)
     */
    Mode startMode() {
        return select_start ? select_start->mode() :
               SelectTimeWidget::bySamples;
    };

    /**
     * Returns the current selection mode for the range
     * (byTime, bySamples, byPercents)
     */
    Mode rangeMode() {
        return select_range ? select_range->mode() :
               SelectTimeWidget::bySamples;
    };

    /** Set a new selection mode */
    void setMode(Mode new_mode);

    /** Returns the number of ms, samples or percents */
    double range() { return select_range ? select_range->time() : 0.0; };

};

#endif /* _SELECT_RANGE_DIALOG_H_ */
