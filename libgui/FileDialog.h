/*************************************************************************
      FileDialog.h  -  enhanced KFileDialog
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

#include <QtGlobal>
#include <QDialog>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QVBoxLayout>

#include <KFileWidget>

class QWidget;

namespace Kwave
{
    /**
     * An improved version of KFileWidget that does not forget the previous
     * directory and pre-selects the previous file extension.
     */
    class Q_DECL_EXPORT FileDialog: public QDialog
    {
	Q_OBJECT
    public:
	typedef enum {
	    Saving = 0,
	    Opening
	} OperationMode;

	/**
	 * Constructor.
	 * @see KFileWidget
	 * @param startDir directory to start with, can start with the scheme
	 *                 "kfiledialog://scope/path"
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
	virtual ~FileDialog()
	{
	}

	/**
	 * Returns the previously used extension, including "*."
	 */
	QString selectedExtension();

	/**
	 * Returns the first selected URL (if any)
	 */
	QUrl selectedUrl() const;

	/**
	 * Sets the current directory
	 * @param directory the new directory to show
	 */
	void setDirectory(const QString &directory);

	/**
	 * Sets the currently selected URL
	 * @param url the new URL to show
	 */
	void selectUrl(const QUrl &url);

    protected:

	/** load previous settings */
	void loadConfig(const QString &section);

    protected slots:

	/** overwritten to call accept() of the KFileWidget and saveConfig() */
	virtual void accept();

	/** save current settings */
	void saveConfig();

    private:

	/**
	 * Try to guess a file filter from a given file extension
	 * @param pattern a file extension, e.g. "*.wav *.snd"
	 * @return a filter string suitable for a KFileDialog
	 */
	QString guessFilterFromFileExt(const QString &pattern,
	                               OperationMode mode);

    private:

	/** layout for holding the KFileWidget */
	QVBoxLayout m_layout;

	/** the KFileWidget that we wrap */
	KFileWidget m_file_widget;

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
