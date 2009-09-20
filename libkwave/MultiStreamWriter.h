/***************************************************************************
      MultiStreamWriter.h - stream writer for multi-track signals
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

#ifndef _MULTI_STREAM_WRITER_H_
#define _MULTI_STREAM_WRITER_H_

#include "config.h"

#include <QObject>
#include <QList>

#include <kdemacros.h>

#include "libkwave/MultiWriter.h"

namespace Kwave {

    /**
     * A MultiStreamWriter encapsulates a set of <c>StreamWriter</c>s for
     * easier use of multi-track signals.
     */
    class KDE_EXPORT MultiStreamWriter: public Kwave::MultiWriter
    {
	Q_OBJECT

    public:

	/** Default constructor */
	MultiStreamWriter(unsigned int tracks);

	/** Destructor */
	virtual ~MultiStreamWriter();

    };
}

#endif /* _MULTI_STREAM_WRITER_H_ */