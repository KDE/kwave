/***************************************************************************
    PluginSetupDialog.h  -  abstract interface for plugin setup dialogs
                             -------------------
    begin                : Sat Jun 07 2003
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

#ifndef _PLUGIN_SETUP_DIALOG_H_
#define _PLUGIN_SETUP_DIALOG_H_

#include "config.h"
#include <QStringList>

class QDialog;

namespace Kwave
{

    class PluginSetupDialog
    {
    public:

	/** virtual destructor, really needed for proper shutdown */
	virtual ~PluginSetupDialog() {}

	/**
	 * Sets the parameters, from a string list representation
	 */
	virtual void setParams(QStringList &params) = 0;

	/**
	 * Returns the current parameters as string list
	 */
	virtual QStringList params() = 0;

	/**
	 * Needed due to problems with inheritance.
	 * @return a pointer to the derived dialog, as a QDialog
	 * @see PitchShiftDialog::dialog()
	 */
	virtual QDialog *dialog() = 0;
    };
}

#endif /* _PLUGIN_SETUP_DIALOG_H_ */

//***************************************************************************
//***************************************************************************
