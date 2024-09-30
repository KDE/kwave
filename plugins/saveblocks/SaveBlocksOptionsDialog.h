// SPDX-FileCopyrightText: 2007 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/***************************************************************************
     SaveBlocksOptionsDialog.h  -  dialog for extra options for saving blocks
                             -------------------
    begin                : Fri Mar 02 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef SAVE_BLOCKS_OPTIONS_DIALOG_H
#define SAVE_BLOCKS_OPTIONS_DIALOG_H

#include <QDialog>

#include "SaveBlocksPlugin.h"
#include "ui_SaveBlocksOptionsDialogBase.h"

namespace Kwave
{
    class SaveBlocksOptionsDialog: public QDialog,
                            public Ui::SaveBlocksOptionsDialogBase
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param widget pointer to the parent widget
         * @param filename the current file name
         * @param filename_pattern pattern used for generating the file names
         * @param numbering_mode the way the numbers are given
         * @param selection_only if true, save only the selection
         * @param have_selection if true, there is a selection
         */
        SaveBlocksOptionsDialog(QWidget *widget,
            QString filename,
            QString filename_pattern,
            Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
            bool selection_only,
            bool have_selection
        );

        /** Destructor */
        ~SaveBlocksOptionsDialog() override;

        /** returns the file name pattern */
        QString pattern();

        /** returns the numbering mode */
        Kwave::SaveBlocksPlugin::numbering_mode_t numberingMode();

        /** returns true if only the selection should be saved */
        bool selectionOnly();

    signals:

        /** emitted whenever a new example has to be shown.
         * @param filename the current filename
         * @param pattern the selected filename pattern
         * @param mode the numbering mode
         * @param selection_only if true: save only the selection
         */
        void sigSelectionChanged(
            const QString &filename,
            const QString &pattern,
            Kwave::SaveBlocksPlugin::numbering_mode_t mode,
            bool selection_only);

        /** emitted whenever one of the input controls has changed */
            void somethingChanged();

    public slots:

        /**
         * update the filename preview
         * @param example the example filename
         */
        void setNewExample(const QString &example);

        /** collects all needed data and emits a sigSelectionChanged */
        void emitUpdate();

    private:
        QString m_filename;
    };
}

#endif /* SAVE_BLOCKS_OPTIONS_DIALOG_H */

//***************************************************************************
//***************************************************************************
