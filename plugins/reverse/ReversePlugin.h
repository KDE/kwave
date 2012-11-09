/*************************************************************************
        ReversePlugin.h  -  reverses the current selection
                             -------------------
    begin                : Tue Jun 09 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef _REVERSE_PLUGIN_H_
#define _REVERSE_PLUGIN_H_

#include "config.h"

#include <QString>
#include <QStringList>

#include "libkwave/KwavePlugin.h"
#include "libkwave/SampleArray.h"
#include "libkwave/Sample.h"

//***************************************************************************
/**
 * @class ReversePlugin
 * Reverts the current selection by exchanging blocks of samples
 * from front and back and reversing their content
 */
class ReversePlugin: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    ReversePlugin(const PluginContext &c);

    /** Destructor */
    virtual ~ReversePlugin();

    /** reverses the selection */
    virtual void run(QStringList);

    /**
     * this plugin must be persistent so that the undo class that it
     * contains survives after the plugin has finished running.
     */
    virtual bool isPersistent() { return true; }

private slots:

    /**
     * multiplies the progress by factor two and
     * calls Kwave::Plugin::updateProgress
     */
    virtual void updateProgress(qreal progress);

};

#endif /* _REVERSE_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
