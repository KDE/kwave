/*************************************************************************
 *      K3BExportPlugin.h  -  export of K3b project files
 *                             -------------------
 *    begin                : Thu Apr 13 2017
 *    copyright            : (C) 2017 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef K3B_EXPORT_PLUGIN_H
#define K3B_EXPORT_PLUGIN_H

#include "config.h"

#include <QObject>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVector>

#include "libkwave/Plugin.h"

class QDomElement;

namespace Kwave
{

    class K3BExportPlugin: public Kwave::Plugin
    {
	Q_OBJECT
    public:

	typedef enum {
	    EXPORT_TO_SAME_DIR = 0,
	    EXPORT_TO_SUB_DIR  = 1
	} export_location_t ;

	typedef enum {
	    OVERWRITE_EXISTING_FILES = 0,
	    USE_NEW_FILE_NAMES       = 1
	} overwrite_policy_t;

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	K3BExportPlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~K3BExportPlugin();

	/** @see Kwave::Plugin::load() */
        virtual void load(QStringList &params) Q_DECL_OVERRIDE;

	/**
	 * Normally this method is used to set up all necessary parameters
	 * for executing the plugin. This plugin uses it for performing
	 * actions in the context of the GUI thread.
	 *
	 * @param params some parameters
	 * @return string list with parameters or null pointer
	 */
        virtual QStringList *setup(QStringList &params) Q_DECL_OVERRIDE;

	/**
	 * Saves the K3b project file, using the settings made in "setup()"
	 * @see Kwave::Plugin::start()
	 */
        virtual int start(QStringList &params) Q_DECL_OVERRIDE;

	/** returns a list of all known detection patterns */
	static QStringList knownPatterns();

    protected:

	typedef struct {
	    unsigned int   m_index;    /**< track index [1...N]           */
	    QString        m_filename; /**< file name for saving          */
	    sample_index_t m_start;    /**< start of the block [samples]  */
	    sample_index_t m_length;   /**< length of the block [samples] */
	    QString        m_title;    /**< title of the block            */
	    QString        m_artist;   /**< artist of the song            */
	} BlockInfo;

	/** reads values from the parameter list */
	int interpreteParameters(QStringList &params);

	/**
	 * determines the blocks which should be saved, including
	 * start position, length and title.
	 * @param base the base name, without indices, extension etc...
	 * @param selection_left index of the first sample
	 * @param selection_right index of the last sample
	 */
	void scanBlocksToSave(const QString &base,
	                      sample_index_t selection_left,
	                      sample_index_t selection_right);

	/**
	 * create a filename (with extension) out of a given name pattern
	 * and index
	 * @param pattern the pattern for creating the filename
	 * @param index the index of the current file
	 * @return the name of the file
	 */
	QString createFileName(const QString &pattern, unsigned int index);

	/**
	 * detects the meta data of a block from splitting the description
	 * text of a label
	 * @param text the description of a label
	 * @param pattern a pattern describing the format of the text
	 * @param block a BlockInfo structure that receives artist and title
	 * @return true if the pattern did match, false otherwise
	 */
	bool detectBlockMetaData(const QString &text,
	                         const QString &pattern,
	                         BlockInfo &block);

	/** save the "general" section */
	void saveGeneralDocumentData(QDomElement *part);

	/** save the K3b project document data */
	void saveDocumentData(QDomElement *docElem);

	/**
	 * save the blocks through the saveblocks plugin
	 * @param selection_only if true, save only the selection
	 * @param out_dir output directory for saving the blocks
	 * @param out_pattern the pattern for creating the block filenames
	 * @return zero if succeeded, or error code if failed
	 */
	int saveBlocks(bool selection_only,
	               const QString &out_dir,
	               const QString &out_pattern);

	/**
	 * save the *.k3b file
	 * @param k3b_filename path to the *.k3b file
	 * @return zero if succeeded, or error code if failed
	 */
	int saveK3BFile(const QString &k3b_filename);

    private:

	/** the URL of the project file */
	QUrl m_url;

	/** pattern for detecting title and artist */
	QString m_pattern;

	/** if true, export only the selected range */
	bool m_selection_only;

	/** where to export the files of the tracks, subdir, same dir, ... */
	export_location_t m_export_location;

	/** overwrite existing files or create a new file name */
	overwrite_policy_t m_overwrite_policy;

	/** list of all blocks to save */
	QVector<BlockInfo> m_block_info;
    };
}

#endif /* K3B_EXPORT_PLUGIN_H */

//***************************************************************************
//***************************************************************************

