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

#ifndef _FILE_INFO_DIALOG_H_
#define _FILE_INFO_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include "libkwave/FileInfo.h"
#include "FileInfoDlg.uih.h"

class FileInfoDialog: public FileInfoDlg
{
    Q_OBJECT
public:
    /** Constructor */
    FileInfoDialog(QWidget *parent, FileInfo &info);

    /** Destructor */
    virtual ~FileInfoDialog();

    /** Returns the current file info */
    FileInfo &info() { return m_info; };

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

    /** auto-generate the list of keywords */
    void autoGenerateKeywords();

    /** compression tab: (de)selected ABR mode */
    void compressionSelectABR(bool checked);

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
    void initInfo(QLabel *label, QWidget *widget, FileProperty property);

    /**
     * Same as initInfo, but works only for text edit controls and sets
     * the current text
     */
    void initInfoText(QLabel *label, QLineEdit *edit, FileProperty property);

private:

    /**
     * takes the content of an edit field or similar into the current
     * info ore removes it if the text is zero-length
     */
    void acceptEdit(FileProperty property, QString value);

    /** initializes the "File" tab */
    void setupFileInfoTab();

    /** initialize the "Compression" tab */
    void setupCompressionTab();

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
    FileInfo m_info;

    /** if true, we have an MPEG file */
    bool m_is_mpeg;

    /** if true, we have an Ogg/Vorbis file */
    bool m_is_ogg;
    
};

#endif /* _FILE_INFO_DIALOG_H_ */
