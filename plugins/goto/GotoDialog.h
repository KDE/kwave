/***************************************************************************
           GotoDialog.h  -  dialog for selecting a position
                             -------------------
    begin                : Sat Dec 06 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#ifndef GOTO_DIALOG_H
#define GOTO_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QObject>

#include "libgui/SelectTimeWidget.h"
#include "ui_GotoDlg.h"

namespace Kwave
{

    class GotoDialog: public QDialog,
                      public Ui::GotoDlg
    {
        Q_OBJECT
    public:
        /** shortcut typedef */
        typedef Kwave::SelectTimeWidget::Mode Mode;

        /**
         * Constructor
         * @param widget pointer to the parent widget
         * @param mode selectionMode for the position,
         *             byTime, bySamples, byPercents
         * @param position position in ms, samples or percent
         * @param sample_rate number of samples per second, needed for
         *                    converting between samples and time
         * @param signal_length length of the signal in samples, needed
         *                      for converting samples to percentage
         * @param help_section section within the manual
         */
        GotoDialog(QWidget *widget, Mode mode, sample_index_t position,
                   double sample_rate, sample_index_t signal_length,
                   const QString &help_section
                );

        /** Destructor */
        ~GotoDialog() override;

        /**
         * Returns the current selection mode for the position
         * (byTime, bySamples, byPercents)
         */
        Mode mode() {
            return select_pos ? select_pos->mode() :
                Kwave::SelectTimeWidget::bySamples;
        }

        /** Set a new position mode */
        void setMode(Kwave::SelectTimeWidget::Mode new_mode);

        /**
         * Returns the current position (byTime, bySamples, byPercents)
         */
        quint64 pos() const { return (select_pos) ? select_pos->time() : 0; }

    private slots:

        /** invoke the online help */
        void invokeHelp();

    private:

        /** name of the section in the handbook */
        QString m_help_section;

    };
}

#endif /* GOTO_DIALOG_H */

//***************************************************************************
//***************************************************************************
