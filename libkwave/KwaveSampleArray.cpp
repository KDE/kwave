/*************************************************************************
    KwaveSampleArray.cpp  -  array with Kwave's internal sample_t
                             -------------------
    begin                : Wed Jan 02 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
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

#include "config.h"

#include <stdlib.h>

#include "KwaveSampleArray.h"

//***************************************************************************
Kwave::SampleArray::SampleArray()
{
    m_storage = new SampleStorage;
}

//***************************************************************************
Kwave::SampleArray::SampleArray(unsigned int size)
{
    m_storage = new SampleStorage;
    resize(size);
}

//***************************************************************************
Kwave::SampleArray::~SampleArray()
{
    resize(0);
}

//***************************************************************************
sample_t * const Kwave::SampleArray::data() const
{
    return (m_storage) ? (m_storage->m_data) : 0;
}

//***************************************************************************
sample_t *Kwave::SampleArray::data()
{
    return (m_storage) ? (m_storage->m_data) : 0;
}

//***************************************************************************
void Kwave::SampleArray::setRawData(sample_t *data, unsigned int size)
{
    if (!m_storage) return;
    m_storage->m_data = data;
    m_storage->m_size = size;
}

//***************************************************************************
void Kwave::SampleArray::resetRawData()
{
    if (!m_storage) return;
    m_storage->m_data = 0;
    m_storage->m_size = 0;
}

//***************************************************************************
void Kwave::SampleArray::fill(sample_t value)
{
    if (!m_storage) return;
    sample_t *p = m_storage->m_data;
    if (!p) return;
    unsigned int count = m_storage->m_size;
    while (count--)
	*(p++) = value;
}

//***************************************************************************
sample_t & Kwave::SampleArray::operator [] (unsigned int index)
{
    static sample_t dummy;

    if (!m_storage) return dummy;
    if (!m_storage->m_data) return dummy;
    return *(m_storage->m_data + index);
}

//***************************************************************************
const sample_t & Kwave::SampleArray::operator [] (unsigned int index) const
{
    static sample_t dummy;

    if (!m_storage) return dummy;
    if (!m_storage->m_data) return dummy;
    return *(m_storage->m_data + index);
}

//***************************************************************************
void Kwave::SampleArray::resize(unsigned int size)
{
    if (!m_storage) return;
    if (size == m_storage->m_size) return;

    if (m_storage->m_data) free(m_storage->m_data);
    m_storage->m_data = 0;
    m_storage->m_size = 0;

    if (size) {
	m_storage->m_data =
	    static_cast<sample_t *>(malloc(size * sizeof(sample_t)));
	if (m_storage->m_data) {
	    m_storage->m_size = size;
	} else {
	    qWarning("OOM in Kwave::SampleArray::resize(%u)", size);
	}
    }
}

//***************************************************************************
unsigned int Kwave::SampleArray::size() const
{
    return m_storage->m_size;
}

//***************************************************************************
Kwave::SampleArray::SampleStorage::SampleStorage()
    :QSharedData()
{
    m_data = 0;
    m_size = 0;
}

//***************************************************************************
Kwave::SampleArray::SampleStorage::SampleStorage(const SampleStorage &other)
    :QSharedData()
{
    m_data = other.m_data;
    m_size = other.m_size;
}

//***************************************************************************
Kwave::SampleArray::SampleStorage::~SampleStorage()
{
}

//***************************************************************************
//***************************************************************************
