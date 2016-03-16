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

#ifndef SAVE_BLOCKS_PLUGIN_H
#define SAVE_BLOCKS_PLUGIN_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QUrl>

#include "libkwave/Plugin.h"

class QStringList;

namespace Kwave
{
    class SaveBlocksPlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	SaveBlocksPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~SaveBlocksPlugin();

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
	 * determines the blocks which should be saved, including
	 * start position, length and title.
	 * @param base the base name, without indices, extension etc...
	 * @param selection_only if true, save only selected blocks
	 */
	void scanBlocksToSave(const QString &base, bool selection_only);

	/**
	 * create a filename (without extension) out of a given base name,
	 * pattern, index and count
	 * @param base the base name, without indices, extension etc...
	 * @param ext the extension (zero-length is allowed)
	 * @param pattern the pattern for creating the filename
	 * @param index the index of the current file
	 * @param count the number of files to save
	 * @param total the highest index to save (first + count - 1)
	 * @return the name of the file, escaped
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
	 * @return name of the first file, escaped
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

	typedef struct {
	    sample_index_t m_start;  /**< start of the block [samples] */
	    sample_index_t m_length; /**< length of the block [samples] */
	    QString        m_title;  /**< title of the block */
	} BlockInfo;

    private:

	/**
	 * internal helper to create a string that contains a HTML
	 * formated list of file names or directories
	 * @param list a list of file names or directories
	 * @param max_entries maximum number of entries to render
	 * @return the list as a single string, separated by "\<br\>"
	 */
	QString createDisplayList(const QStringList &list,
                                  unsigned int max_entries) const;

    private:

	/** the URL of the first file (user selection) */
	QUrl m_url;

	/** expression with the filename pattern */
	QString m_pattern;

	/** mode for numbering the output files */
	numbering_mode_t m_numbering_mode;

	/** if true, only save stuff within the selection */
	bool m_selection_only;

	/** list of all blocks to save */
	QList<BlockInfo> m_block_info;

    };
}

#endif /* SAVE_BLOCKS_PLUGIN_H */

//***************************************************************************
//***************************************************************************
