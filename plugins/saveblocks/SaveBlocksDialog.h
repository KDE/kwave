/***************************************************************************
     SaveBlocksDialog.h  -  Extended KwaveFileDialog for saving blocks
                             -------------------
    begin                : Thu Mar 01 2007
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

#ifndef SAVE_BLOCKS_DIALOG_H
#define SAVE_BLOCKS_DIALOG_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QWidget>

#include "SaveBlocksPlugin.h"
#include "libgui/FileDialog.h"

class QUrl;

namespace Kwave
{

    class SaveBlocksWidget;

    class SaveBlocksDialog: public Kwave::FileDialog
    {
        Q_OBJECT
    public:

        /**
         * Constructor.
         * @see KFileFialog
         * @param startDir the start directory
         * @param filter string with a file type filter
         * @param parent the parent widget
         * @param last_url the last used URL
         * @param last_ext the last used extension (preset only)
         * @param filename_pattern pattern used for generating the file names
         * @param numbering_mode the way the numbers are given
         * @param selection_only if true, save only the selection
         * @param have_selection if true, there is a selection
         */
        SaveBlocksDialog(const QString &startDir,
            const QString &filter,
            QWidget *parent,
            const QUrl &last_url,
            const QString &last_ext,
            QString &filename_pattern,
            Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
            bool selection_only,
            bool have_selection
        );

        /** Destructor */
        ~SaveBlocksDialog() override;

        /** returns the file name pattern (as is, not escaped) */
        QString pattern();

        /** returns the numbering mode */
        Kwave::SaveBlocksPlugin::numbering_mode_t numberingMode();

        /** returns true if only the selection should be saved */
        bool selectionOnly();

    signals:

        /**
         * emitted whenever the selection has changed and a new example
         * has to be shown.
         * @param filename the currently selected filename
         * @param pattern the selected filename pattern
         * @param mode the numbering mode
         * @param selection_only if true: save only the selection
         */
        void sigSelectionChanged(
            const QString &filename,
            const QString &pattern,
            Kwave::SaveBlocksPlugin::numbering_mode_t mode,
            bool selection_only);

    public slots:

        /**
         * update the filename preview
         * @param example the example filename
         */
        void setNewExample(const QString &example);

        /** collects all needed data and emits a sigSelectionChanged */
        void emitUpdate();

    private:

        /** the widget with extra settings for saving the blocks */
        Kwave::SaveBlocksWidget *m_widget;

    };
}

#endif /* SAVE_BLOCKS_DIALOG_H */

//***************************************************************************
//***************************************************************************
