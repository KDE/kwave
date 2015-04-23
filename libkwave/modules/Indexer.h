/*************************************************************************
              Indexer.h  -  add an index to a stream
                             -------------------
    begin                : Sat Oct 16 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#ifndef INDEXER_H
#define INDEXER_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QString>

#include "libkwave/SampleArray.h"
#include "libkwave/modules/StreamObject.h"

//***************************************************************************
namespace Kwave
{

    class Indexer :public Kwave::StreamObject
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param index an index that is attached to the input data
	 */
	Indexer(unsigned int index);

	/** Destructor */
	virtual ~Indexer();

    public slots:

	/** receives input data */
	void input(Kwave::SampleArray data);

    signals:

	/** forwards index + data as output */
	void output(unsigned int index, Kwave::SampleArray data);

    private:

	/** the index that is attached to each output data */
	unsigned int m_index;
    };

}

#endif /* INDEXER_H */

//***************************************************************************
//***************************************************************************
