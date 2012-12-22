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

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kurl.h>

#include "libkwave/Plugin.h"

class QStringList;

namespace Kwave
{
    class SaveBlocksPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/** Constructor */
	SaveBlocksPlugin(Kwave::PluginManager &plugin_manager);

	/** Destructor */
	virtual ~SaveBlocksPlugin();

	/** Returns the name of the plugin. */
	virtual QString name() const;

	/**
	 * Shows a file saving dialog and emits a command for saving the blocks
	 * when OK has been pressed.
	 * @see Kwave::Plugin::setup
	 */
	virtual QStringList *setup(QStringList &previous_params);

	/**
	 * Saves the files, using the settings made in "setup()"
	 * @see Kwave::Plugin::start()
	 */
	virtual int start(QStringList &params);

	/** mode for numbering the output files */
	typedef enum {
	    CONTINUE      = 0,
	    START_AT_ONE  = 1
	} numbering_mode_t;

    protected:

	/** reads values from the parameter list */
	int interpreteParameters(QStringList &params);

	/**
	 * Returns the number of blocks to save, depending on whether
	 * we save everything or only the selection
	 * @param selection_only if true, save only selected blocks
	 * @return number of blocks, [0...N]
	 */
	unsigned int blocksToSave(bool selection_only);

	/**
	 * create a filename (without extension) out of a given base name,
	 * pattern, index and count
	 * @param base the base name, without indices, extension etc...
	 * @param ext the extension (zero-length is allowed)
	 * @param pattern the pattern for creating the filename
	 * @param index the index of the current file
	 * @param count the number of files to save
	 * @param total the highest index to save (first + count - 1)
	 */
	QString createFileName(const QString &base, const QString &ext,
	                       const QString &pattern,
	                       unsigned int index, int count, int total);

	/**
	 * determines the index of the first file name that matches the
	 * given filename, pattern and mode
	 * @param path the directory for saving
	 * @param base the base name, without indices, extension etc...
	 * @param ext the extension (zero-length is allowed)
	 * @param pattern the pattern for creating the filename
	 * @param mode the numbering mode
	 * @param count the total number of files
	 * @return the index of the first file, [1...count+X]
	 */
	unsigned int firstIndex(const QString &path, const QString &base,
	                        const QString &ext, const QString &pattern,
	                        Kwave::SaveBlocksPlugin::numbering_mode_t mode,
	                        unsigned int count);

	/**
	 * Find out the base name out of a given file name, using a
	 * given filename pattern. If the given file name is already
	 * produced (matched) by this pattern, the base name will be
	 * cut out of the file name.
	 * @param filename the file name to check
	 * @param pattern the selected filename pattern
	 * @return the base name of the file, without path and extension
	 */
	QString findBase(const QString &filename, const QString &pattern);

	/**
	 * determines the first file name that matches the given filename,
	 * pattern, mode and selection
	 * @param filename the currently selected filename
	 * @param pattern the selected filename pattern
	 * @param mode the numbering mode
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
	 * @param filename the currently selected filename, including path
	 * @param pattern the selected filename pattern
	 * @param mode the numbering mode
	 * @param selection_only if true: save only the selection
	 */
	void updateExample(
	    const QString &filename,
	    const QString &pattern,
	    Kwave::SaveBlocksPlugin::numbering_mode_t mode,
	    bool selection_only);

    private:

	/** the URL of the first file (user selection) */
	KUrl m_url;

	/** expression with the filename pattern */
	QString m_pattern;

	/** mode for numbering the output files */
	numbering_mode_t m_numbering_mode;

	/** if true, only save stuff within the selection */
	bool m_selection_only;

    };
}

#endif /* _SAVE_BLOCKS_PLUGIN_H_ */

//***************************************************************************
//***************************************************************************
