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

#ifndef _ABOUT_PLUGIN_
#define _ABOUT_PLUGIN_

#include <qarray.h>
#include <qstring.h>
#include <libgui/KwavePlugin.h>

class QStrList;
class PluginContext;

class AboutPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    AboutPlugin(PluginContext &c);

    /**
     * shows the about dialog,
     * @see KwavePlugin::start()
     */
    virtual int start(QStrList &params);

};

#endif /* _ABOUT_PLUGIN_H_ */

/* end of AboutPlugin.h */
