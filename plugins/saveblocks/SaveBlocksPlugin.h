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

    /** reads values from the parameter list */
    int interpreteParameters(QStringList &params);

    /**
     * determines the first file name that matches the given filename,
     * pattern, mode and selection
     * @param filename the currently selected filename
     * @param pattern the selected filename pattern
     * @param the numbering mode
     * @param selection_only if true: save only the selection
     */
    QString firstFileName(const QString &filename, const QString &pattern,
	numbering_mode_t mode, bool selection_only);

signals:

    /** emitted by updateExample to update the filename preview */
    void sigNewExample(const QString &example);

private slots:

    /**
     * called whenever the selection has changed and a new example
     * has to be shown.
     * @param filename the currently selected filename
     * @param pattern the selected filename pattern
     * @param the numbering mode
     * @param selection_only if true: save only the selection
     */
    void updateExample(
	const QString &filename,
	const QString &pattern,
	SaveBlocksPlugin::numbering_mode_t mode,
	bool selection_only);

private:

    /** expression with the filename pattern */
    QString m_pattern;

    /** mode for numbering the output files */
    numbering_mode_t m_numbering_mode;

    /** if true, only save stuff within the selection */
    bool m_selection_only;

    /** the number of selected blocks to save */
    unsigned int m_selected_blocks;

};

#endif /* _SAVE_BLOCKS_PLUGIN_H_ */
