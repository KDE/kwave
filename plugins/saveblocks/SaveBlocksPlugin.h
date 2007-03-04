/***************************************************************************
     SaveBlocksPlugin.h  -  Plugin for saving blocks between labels
                             -------------------
    begin                : Thu Mar 01 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#ifndef _SAVE_BLOCKS_PLUGIN_H_
#define _SAVE_BLOCKS_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>

#include "libkwave/KwavePlugin.h"

class QStringList;

class SaveBlocksPlugin: public KwavePlugin
{
    Q_OBJECT

public:

    /** Constructor */
    SaveBlocksPlugin(const PluginContext &context);

    /** Destructor */
    virtual ~SaveBlocksPlugin();

    /**
     * Shows a dialog for selecting the range and emits a command
     * for applying the selection if OK has been pressed.
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /**
     * selects the range
     * @see KwavePlugin::start()
     */
    virtual int start(QStringList &params);

    /** mode for numbering the output files */
    typedef enum {
	CONTINUE      = 0,
	START_AT_ZERO = 1,
	START_AT_ONE  = 2
    } numbering_mode_t;

protected:

    /** Reads values from the parameter list */
    int interpreteParameters(QStringList &params);

private:

    /** expression with the filename pattern */
    QString m_pattern;

    /** mode for numbering the output files */
    numbering_mode_t m_numbering_mode;

    /** if true, only save stuff within the selection */
    bool m_selection_only;

};

#endif /* _SAVE_BLOCKS_PLUGIN_H_ */
