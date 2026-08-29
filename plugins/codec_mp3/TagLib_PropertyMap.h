/*************************************************************************
    TagLib_PropertyMap.h  -  map for translating properties to ID3 frame tags
                             -------------------
    begin                : Fri Aug 28 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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

#ifndef TAGLIB_PROPERTY_MAP_H
#define TAGLIB_PROPERTY_MAP_H

#include "config.h"

#include <taglib/tbytevector.h>

#include <QList>

#include "libkwave/FileInfo.h"

namespace Kwave
{

    class TagLib_PropertyMap
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
            ENC_TEXT_URL,       /**< url                               */
            ENC_TEXT_PARTINSET, /**< part in set (x/y)                 */
            ENC_TEXT_TIMESTAMP, /**< iso 8601 timestamp                */
            ENC_TRACK_NUM,      /**< track/tracks (x/y)                */
            ENC_BINARY          /**< binary/custom data                */
        } Encoding;

        /** default constructor, with initializing */
        TagLib_PropertyMap();

        /** destructor */
        virtual ~TagLib_PropertyMap() {}

        /**
         * returns the frame ID of a property or an empty byte vector
         * if nothing found (reverse lookup).
         */
        TagLib::ByteVector findProperty(const Kwave::FileProperty property) const;

        /** returns true if the map contains a given property */
        bool containsProperty(const Kwave::FileProperty property) const;

        /**
         * insert a new property / frame ID mapping
         *
         * @param property a Kwave FileProperty
         * @param id a ID3 frame ID
         * @param encoding the type of the encoding of the tag
         */
        void insert(const Kwave::FileProperty property,
                    const TagLib::ByteVector &id,
                    const Encoding encoding);

        /**
         * returns true if a given ID3 frame ID is in the map
         *
         * @param id a ID3 frame ID
         * @return true if found, false if not
         */
        bool containsID(const TagLib::ByteVector &id) const;

        /**
         * returns the encoding of the ID3 frame
         *
         * @param id a ID3 frame ID
         * @return the encoding of the content of the frame
         */
        Encoding encoding(const TagLib::ByteVector &id) const;

        /** returns a list of all known ID3 frame IDs */
        QList<TagLib::ByteVector> knownIDs() const;

        /**
         * returns the first FileProperty that matches a given ID3 frame ID
         *
         * @param id a ID3 frame ID
         * @return a FileProperty
         */
        Kwave::FileProperty property(const TagLib::ByteVector &id) const;

        /** returns a list with all supported properties */
        QList<Kwave::FileProperty> properties() const;

    private:

        /** returns true if a frame ID is valid */
        bool supported(const TagLib::ByteVector &id) const;

    private:

        /** container for one mapping */
        typedef struct
        {
            Kwave::FileProperty m_property; /**< the Kwave property */
            TagLib::ByteVector  m_frame_id; /**< id3 frame ID       */
            Encoding            m_encoding; /**< data encoding      */
        } Mapping;

        /** list of mappings */
        QList<Mapping> m_list;
    };
}

#endif /* TAGLIB_PROPERTY_MAP_H */
