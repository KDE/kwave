// SPDX-FileCopyrightText: 2002 Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
// SPDX-FileCopyrightText: 2024 Mark Penner <mrp@markpenner.space>
// SPDX-License-Identifier: GPL-2.0-or-later
/*************************************************************************
      FileDialog.h  -  wrapper for QFileDialog
                             -------------------
    begin                : Thu May 30 2002
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

#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H

#include "config.h"
#include "libkwavegui_export.h"

#include <QtGlobal>
#include <QFileDialog>
#include <QObject>
#include <QString>
#include <QUrl>

class QWidget;

namespace Kwave
{
    /**
     * A wrapper for QFileDialog that remembers the previous directory
     * and pre-selects the previous file extension for multiple contexts.
     */
    class LIBKWAVEGUI_EXPORT FileDialog: public QObject
    {
        Q_OBJECT
    public:
        typedef enum {
            SaveFile = 0, /**< save a file */
            OpenFile,     /**< open a file */
            SelectDir     /**< select a directory */
        } OperationMode;

        /**
         * Constructor.
         * @param startDir directory to start with, can start with the scheme
         *                 "kfiledialog://scope/path"
         * @param mode determines the mode in which the dialog is used, either
         *             to open a file, save a file or select a directory
         * @param filter a "\n" separated list of file filters, each filter
         *               consists of a space separated list of file patterns
         *               (including "*.") and a "|" as separator followed
         *               by a verbose description of the file type
         * @param parent the parent widget of the dialog
         * @param last_url URL used for the last call, optional
         * @param last_ext file extension (pattern) used for the last
         *                 call (optional), including a "*."
         */
        FileDialog(const QString& startDir,
                   OperationMode mode,
                   const QString& filter,
                   QWidget *parent,
                   const QUrl last_url = QUrl(),
                   const QString last_ext = QString());

        /** Destructor */
        virtual ~FileDialog() override
        {
        }

        /**
         * execute the QFileDialog
         */
        int exec();

        /**
         * Returns the previously used extension, including "*."
         */
        QString selectedExtension();

        /**
         * Returns the first selected URL (if any)
         */
        QUrl selectedUrl() const;

        /**
         * Returns the URL of the currently visible directory
         */
        QUrl baseUrl() const;

        /**
         * Sets the current directory
         * @param directory the new directory to show
         */
        void setDirectory(const QString &directory);

        /**
         * Sets the dialog title
         * @param title the new dialog title
         */
        void setWindowTitle(const QString &title);

        /**
         * Sets the currently selected URL
         * @param url the new URL to show
         */
        void selectUrl(const QUrl &url);

    protected:

        /** load previous settings */
        void loadConfig(const QString &section);

        /** save current settings */
        void saveConfig();

    private:

        /**
         * Try to guess a file filter from a given file extension
         * @param pattern a file extension, e.g. "*.wav *.snd"
         * @param mode determines the mode in which the dialog is used,
         *             see constructor
         * @return a filter string suitable for a KFileDialog
         */
        QString guessFilterFromFileExt(const QString &pattern,
                                       OperationMode mode);

    private:
        /** the QFileDialog that we wrap */
        QFileDialog *m_file_dialog;

        /** name of the group in the config file */
        QString m_config_group;

        /** URL of the previously opened file or directory */
        QUrl m_last_url;

        /** extension of the last selected single URL or file */
        QString m_last_ext;

    };
}

#endif /* FILE_DIALOG_H */

//***************************************************************************
//***************************************************************************
