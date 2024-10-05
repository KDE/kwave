// SPDX-FileCopyrightText: 2017 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/***************************************************************************
 * K3BExportOptionsDialog.h -  dialog for K3b export options
 *                             -------------------
 *    begin                : Thu Apr 13 2017
 *    copyright            : (C) 2017 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef K3B_EXPORT_OPTIONS_DIALOG_H
#define K3B_EXPORT_OPTIONS_DIALOG_H

#include <QDialog>

#include "K3BExportPlugin.h"
#include "ui_K3BExportOptionsDialogBase.h"

namespace Kwave
{
    class K3BExportOptionsDialog: public QDialog, public Ui::K3BExportOptionsDialogBase
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param parent pointer to the parent widget
         * @param pattern the pattern used for detecting title and artist
         * @param selection_only if true, save only the selection
         * @param have_selection if true, there is a selection
         * @param overwrite_policy overwrite existing files or use a new name
         */
        K3BExportOptionsDialog(
            QWidget *parent,
            QString &pattern,
            bool selection_only,
            bool have_selection,
            Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy
        );

        /** Destructor */
        virtual ~K3BExportOptionsDialog() override;

        /** returns the title/artist detection pattern (as is, not escaped) */
        QString pattern() const;

        /** returns true if only the selection should be saved */
        bool selectionOnly() const;

        /** returns the file overwrite policy */
        Kwave::K3BExportPlugin::overwrite_policy_t overwritePolicy() const;

    };
}

#endif /* K3B_EXPORT_OPTIONS_DIALOG_H */

//***************************************************************************
//***************************************************************************
