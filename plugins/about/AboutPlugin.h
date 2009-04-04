/***************************************************************************
          AboutPlugin.h  -  plugin that shows the Kwave's about dialog
                             -------------------
    begin                : Sun Oct 29 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _ABOUT_PLUGIN_H_
#define _ABOUT_PLUGIN_H_

#include "config.h"

#include "libkwave/KwavePlugin.h"

class QStringList;
class PluginContext;

class AboutPlugin: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    AboutPlugin(const PluginContext &c);

    /** Destructor */
    virtual ~AboutPlugin() {};

    /**
     * shows the about dialog,
     * @see Kwave::Plugin::start()
     */
    virtual int start(QStringList &params);

};

#endif /* _ABOUT_PLUGIN_H_ */

/* end of AboutPlugin.h */
