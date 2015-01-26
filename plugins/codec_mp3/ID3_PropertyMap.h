/*************************************************************************
    ID3_PropertyMap.h    -  map for translating properties to ID3 frame tags
                             -------------------
    begin                : Sat Jul 30 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef _ID3_PROPERTY_MAP_H_
#define _ID3_PROPERTY_MAP_H_

#include <config.h>

#include <QtCore/QList>
#include <QtCore/QPair>

#include <id3/globals.h>

#include "libkwave/FileInfo.h"

namespace Kwave
{

    class ID3_PropertyMap
    {
    public:

	/** encoding of the ID3 tag */
	typedef enum {
	    ENC_NONE = 0,
	    ENC_COMMENT,        /**< comment frame                     */
	    ENC_GENRE_TYPE,     /**< genre type, numeric or text       */
	    ENC_LENGTH,         /**< string with length in ms          */
	    ENC_TERMS_OF_USE,   /**< terms of use                      */
	    ENC_TEXT,           /**< text, appended by ';'             */
	    ENC_TEXT_SLASH,     /**< text list, separated by slash '/' */
	    ENC_TEXT_LIST,      /**< list of zero terminated strings   */
	    ENC_TEXT_URL,       /**< URL                               */
	    ENC_TEXT_PARTINSET, /**< part in set (x/y)                 */
	    ENC_TEXT_TIMESTAMP, /**< ISO 8601 timestamp                */
	    ENC_TRACK_NUM       /**< track/tracks (x/y)                */
	} Encoding;

	/** Default constructor, with initializing */
	ID3_PropertyMap();

	/** Destructor */
	virtual ~ID3_PropertyMap() {}

	/**
	 * Returns the frame ID of a property or an empty string
	 * if nothing found (reverse lookup).
	 */
	ID3_FrameID findProperty(const Kwave::FileProperty property) const;

	/** Returns true if the map contains a given property */
	bool containsProperty(const Kwave::FileProperty property) const;

	/**
	 * insert a new property / frame ID mapping
	 *
	 * @param property a Kwave FileProperty
	 * @param id a ID3 frame ID
	 * @param encoding the type of the encoding of the tag
	 */
	void insert(const Kwave::FileProperty property, const ID3_FrameID id,
	            const Encoding encoding);

	/**
	 * returns true if a given ID3 frame ID is in the map
	 *
	 * @param id a ID3 frame ID
	 * @return true if found, false if not
	 */
	bool containsID(const ID3_FrameID id) const;

	/**
	 * returns the encoding of the ID3 frame
	 *
	 * @param id a ID3 frame ID
	 * @return the encoding of the content of the frame
	 */
	Encoding encoding(const ID3_FrameID id) const;

	/** returns a list of all known ID3 frame IDs */
	QList<ID3_FrameID> knownIDs() const;

	/**
	 * Returns the first FileProperty that matches a given ID3 frame ID
	 *
	 * @param id a ID3 frame ID
	 * @return a FileProperty
	 */
	Kwave::FileProperty property(const ID3_FrameID id) const;

	/** Returns a list with all supported properties */
	QList<Kwave::FileProperty> properties() const;

    private:

	/** returns true if a frame is supported by id3lib */
	bool supported(const ID3_FrameID id) const;

    private:
	/** container for one mapping */
	typedef struct
	{
	    Kwave::FileProperty m_property; /**< the Kwave property */
	    ID3_FrameID  m_frame_id;        /**< ID3 frame ID       */
	    Encoding     m_encoding;        /**< data encoding      */
	} Mapping;

	/** list of mappings */
	QList<Mapping> m_list;
    };
}

#endif /* _ID3_PROPERTY_MAP_H_ */

//***************************************************************************
//***************************************************************************
