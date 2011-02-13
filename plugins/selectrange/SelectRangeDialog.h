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

#include <QDialog>
#include <QObject>
#include <QTimer>

#include "libgui/SelectTimeWidget.h"
#include "ui_SelectRangeDlg.h"

class SelectRangeDialog: public QDialog,
                         public Ui::SelectRangeDlg
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
                      unsigned int range, double sample_rate,
                      sample_index_t offset, sample_index_t signal_length);

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
    void setMode(SelectTimeWidget::Mode new_mode);

    /**
     * Returns the current start position (byTime, bySamples, byPercents)
     */
    unsigned int start() { return select_start ? select_start->time() : 0; };

    /** Returns the number of ms, samples or percents */
    unsigned int range() { return select_range ? select_range->time() : 0; };

};

#endif /* _SELECT_RANGE_DIALOG_H_ */
