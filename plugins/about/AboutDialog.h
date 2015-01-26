/***************************************************************************
          AboutDialog.h  -  dialog for Kwave's "Help-About"
                             -------------------
    begin                : Sun Feb 10 2002
    copyright            : (C) 2002 by Ralf Waspe
    email                : rwaspe@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ABOUT_KWAVE_DIALOG_H_
#define _ABOUT_KWAVE_DIALOG_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtGui/QDialog>
#include <QtCore/QList>

#include "libkwave/PluginManager.h"

#include "ui_AboutDialogBase.h"

class QWidget;

namespace Kwave
{
    /**
    * Dialog for Help/About
    */
    class AboutDialog: public QDialog,
                       public Ui::AboutDialogBase
    {
	Q_OBJECT

    public:

	/**
	* Constructor
	* @param parent the parent widget
	* @param plugin_info list of plugin info structures (unsorted)
	*/
	AboutDialog(QWidget *parent,
	    const QList<Kwave::PluginManager::PluginModule> &plugin_info);

	/** Destructor */
	virtual ~AboutDialog();

    };
}

#endif  /* _ABOUT_KWAVE_DIALOG_H_ */

//***************************************************************************
//***************************************************************************
