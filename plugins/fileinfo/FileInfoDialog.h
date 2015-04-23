/***************************************************************************
       FileInfoDialog.h  -  dialog for editing file properties
                             -------------------
    begin                : Sat Jul 20 2002
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

#ifndef FILE_INFO_DIALOG_H
#define FILE_INFO_DIALOG_H

#include "config.h"

#include <QtCore/QObject>

#include "libkwave/FileInfo.h"
#include "ui_FileInfoDlg.h"

class KConfigGroup;
class KLineEdit;

namespace Kwave
{
    class FileInfoDialog: public QDialog,
                          public Ui::FileInfoDlg
    {
	Q_OBJECT
    public:
	/** Constructor */
	FileInfoDialog(QWidget *parent, Kwave::FileInfo &info);

	/** Destructor */
	virtual ~FileInfoDialog();

	/** Returns the current file info */
	Kwave::FileInfo &info() { return m_info; }

    public slots:

	/** applies the settings and closes the dialog (OK button) */
	virtual void accept();

    private slots:

	/** select a date for the "creation date" setting */
	void selectDate();

	/** sets the "creation date" to today */
	void setDateNow();

	/** updates the verbose description of the number of tracks */
	void tracksChanged(int tracks);

	/**
	 * update the combo box with the list of available compressions,
	 * according to the current mime type
	 */
	void updateAvailableCompressions();

	/** called when the compression mode has changed */
	void compressionChanged();

	/** called when the MPEG layer combo box has changed */
	void mpegLayerChanged();

	/** called when the MPEG "copyrighted" check box has changed */
	void mpegCopyrightedChanged(bool checked);

	/** called when the MPEG "original" check box has changed */
	void mpegOriginalChanged(bool checked);

	/** auto-generate the list of keywords */
	void autoGenerateKeywords();

	/** invoke the online help */
	void invokeHelp();

    protected:

	/**
	 * Sets the tooltip and "what's this" of a widget.
	 * @param widget any QWidget derived widget
	 * @param name of the setting, normally equal to it's label
	 * @param description verbose descriptive text that says
	 *        what can be set
	 */
	void describeWidget(QWidget *widget, const QString &name,
	                    const QString &description);

	/**
	 * Sets the text of the label to the name of a file property and
	 * initializes the tool tip of the corresponding edit/display control.
	 * @param label the label to be set
	 * @param widget the control to get the tool tip
	 * @param property the file property which it belongs to
	 */
	void initInfo(QLabel *label, QWidget *widget,
	              Kwave::FileProperty property);

	/**
	 * Same as initInfo, but works only for text edit controls and sets
	 * the current text
	 */
	void initInfoText(QLabel *label, KLineEdit *edit,
	                  Kwave::FileProperty property);

    private:

	/**
	 * takes the content of an edit field or similar into the current
	 * info ore removes it if the text is zero-length
	 */
	void acceptEdit(Kwave::FileProperty property, QString value);

	/** initializes the "File" tab */
	void setupFileInfoTab();

	/** initialize the "Compression" tab */
	void setupCompressionTab(KConfigGroup &cfg);

	/** returns true if the current compression is MPEG I/II/III */
	bool isMpeg() const;

	/** initialize the "MPEG" tab */
	void setupMpegTab();

	/** initializes the "Content" tab */
	void setupContentTab();

	/** initialize the "Source" tab */
	void setupSourceTab();

	/** initialize the "Author/Copyright" tab */
	void setupAuthorCopyrightTab();

	/** initialize the "Miscellaneous" tab */
	void setupMiscellaneousTab();


    private:

	/** FileInfo to be edited */
	Kwave::FileInfo m_info;

    };
}

#endif /* FILE_INFO_DIALOG_H */

//***************************************************************************
//***************************************************************************
