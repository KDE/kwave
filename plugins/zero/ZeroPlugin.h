/*************************************************************************
           ZeroPlugin.h  -  wipes out the selected range of samples to zero
                             -------------------
    begin                : Fri Jun 01 2001
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

#ifndef _ZERO_PLUGIN_H_
#define _ZERO_PLUGIN_H_

#include <qstring.h>
#include "libkwave/Sample.h"
#include "libgui/KwavePlugin.h"

//***************************************************************************
/**
 * @class ZeroPlugin
 * This is a very simple plugin that blanks the currently selected range of
 * samples with zeroes.
 */
class ZeroPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    ZeroPlugin(PluginContext &c);

    /**
     * Does playback in a thread.
     */
    virtual void run(QStringList);

    /**
     * Aborts the process (if running).
     */
    virtual int stop();

private:

    /** flag for stopping the process */
    bool m_stop;

    /** use an array with zeroes for faster filling */
    QArray<sample_t> m_zeroes;

};

#endif /* _ZERO_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
