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

#ifndef K3B_EXPORT_DIALOG_H
#define K3B_EXPORT_DIALOG_H

#include <QDialog>

#include "K3BExportPlugin.h"
#include "ui_K3BExportDialogBase.h"

using namespace Qt::StringLiterals;

namespace Kwave
{
    class K3BExportDialog: public QDialog, public Ui::K3BExportDialogBase
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
         * @param url current file url, used to set the path for the file dialogs
         */
        K3BExportDialog(
            QWidget *parent,
            QString &pattern,
            bool selection_only,
            bool have_selection,
            Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy,
            QUrl url
        );

        /** Destructor */
        virtual ~K3BExportDialog() override;

        /** returns the title/artist detection pattern (as is, not escaped) */
        QString pattern() const;

        /** returns true if only the selection should be saved */
        bool selectionOnly() const;

        /** returns the file overwrite policy */
        Kwave::K3BExportPlugin::overwrite_policy_t overwritePolicy() const;

        /** returns the K3b project file name */
        QUrl projectFile() const;

        /** returns the audio file export location */
        QUrl exportLocation() const;

        Q_SLOT void accept() override;

    private:
        const QString m_cfgGroup = u"KwaveFileDialog-kwave_export_k3b"_s;
    };
}

#endif /* K3B_EXPORT_DIALOG_H */

//***************************************************************************
//***************************************************************************
