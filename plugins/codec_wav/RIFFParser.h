/*************************************************************************
           RIFFParser.h  -  parser for the RIFF format
                             -------------------
    begin                : Tue Mar 20 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _RIFF_PARSER_H_
#define _RIFF_PARSER_H_

#include "config.h"
#include <qlist.h>

class QIODevice;
class QCString;
class RIFFChunk;

class RIFFParser
{
public:

    RIFFParser(QIODevice &device);

    ~RIFFParser();

    /**
     * Parses the whole source.
     * @return true if passed without any error
     */
    bool parse();

    /*
     * Parses a range of the source and adds all found chunks to a
     * list of chunks. Also parses recursively for sub-chunks and
     * builds a tree.
     * @param parent path of the parent node
     * @param offset start of the chunk name in the source
     * @param length size of the area where to search
     * @param chunks list of chunks that receives all found chunks
     * @return true if passed without any error
     */
    bool parse(const QCString &parent, u_int32_t offset, u_int32_t length,
               QList<RIFFChunk> &chunks);

    /**
     * Tries very hard to find a missing chunk by stepping through
     * the whole file or source.
     * @param name the 4-byte name of the chunk
     * @return true if found.
     */
    bool findMissingChunk(const QCString &name);

private:

    /** I/O device with the source of the file */
    QIODevice &m_dev;

    /** list of sub-chunks, will be empty on a "leaf" */
    QList<RIFFChunk> m_chunks;
};

#endif /* _RIFF_PARSER_H_ */
