/***************************************************************************
      NewSignalPlugin.h  -  plugin for creating a new signal
                             -------------------
    begin                : Wed Jul 18 2001
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

#ifndef _NEW_SIGNAL_PLUGIN_H_
#define _NEW_SIGNAL_PLUGIN_H_

#include "libgui/KwavePlugin.h"

class QStringList;

class NewSignalPlugin: public KwavePlugin
{
public:

    /** Constructor */
    NewSignalPlugin(PluginContext &context);

    /** virtual Destructor */
    virtual ~NewSignalPlugin();

    /**
     * Shows a dialog for creating a new signal and emits sigCommand if
     * OK has been pressed.
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

};

#endif /* _NEW_SIGNAL_PLUGIN_H_ */
