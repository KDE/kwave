/*************************************************************************
         RecordDialog.h  -  dialog window for controlling audio recording
                             -------------------
    begin                : Wed Aug 20 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _RECORD_DIALOG_H_
#define _RECORD_DIALOG_H_

#include "config.h"
#include <qstringlist.h>
#include "RecordDlg.uih.h"
#include "RecordParams.h"

class QWidget;

class RecordDialog: public RecordDlg
{
Q_OBJECT

public:

    enum RecordState {
	REC_EMPTY = 0,
	REC_BUFFERING,
	REC_WAITING_FOR_TRIGGER,
	REC_RECORDING,
	REC_PAUSED,
	REC_DONE
    };

    /** Constructor */
    RecordDialog(QWidget *parent, const RecordParams &params);

    /** Destructor */
    virtual ~RecordDialog();

    /** Returns the list of record parameters, for the next time */
    virtual RecordParams params() const;

private slots:

    /** updates the record buffer size */
    void sourceBufferChanged(int value);

    /** show a "file open" dialog for selecting a record device */
    void selectRecordDevice();

private:

    /** List of parameters */
    RecordParams m_params;

};


#endif /* _RECORD_PLUGIN_H_ */
