/***************************************************************************
    AmplifyFreeDialog.h  -  dialog for the "amplifyfree" plugin
                             -------------------
    begin                : Sun Sep 02 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _AMPLIFY_FREE_DIALOG_H_
#define _AMPLIFY_FREE_DIALOG_H_

#include "config.h"
#include <qobject.h>
#include "AmplifyFreeDlg.uih.h"

class AmplifyFreeDialog: public AmplifyFreeDlg
{
    Q_OBJECT
public:

    /** Constructor */
    AmplifyFreeDialog(QWidget *parent);

    /** Destructor */
    virtual ~AmplifyFreeDialog();

};

#endif /* _AMPLIFY_FREE_DIALOG_H_ */
