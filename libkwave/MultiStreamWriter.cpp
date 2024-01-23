/***************************************************************************
    MultiStreamWriter.cpp - stream writer for multi-track signals
                             -------------------
    begin                : Sun Aug 23 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <new>

#include "libkwave/MultiStreamWriter.h"
#include "libkwave/StreamWriter.h"

//***************************************************************************
Kwave::MultiStreamWriter::MultiStreamWriter(unsigned int tracks)
    :Kwave::MultiWriter()
{
    for (unsigned int index = 0; index < tracks; index++) {
        Kwave::StreamWriter *s = new(std::nothrow) Kwave::StreamWriter();
        if (s) {
            insert(index, s);
        } else {
            // out of memory or aborted
            qWarning("MultiStreamWriter constructor: "\
                     "out of memory or aborted");
            clear();
            break;
        }
    }
}

//***************************************************************************
Kwave::MultiStreamWriter::~MultiStreamWriter()
{
    clear();
}

//***************************************************************************
//***************************************************************************
