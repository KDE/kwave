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

#include "memcpy.h"
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
}

//***************************************************************************
sample_t * const Kwave::SampleArray::data() const
{
    if (!m_storage) return 0;
    if (m_storage->m_raw_data) return m_storage->m_raw_data;
    return m_storage->m_data;
}

//***************************************************************************
sample_t *Kwave::SampleArray::data()
{
    if (!m_storage) return 0;
    if (m_storage->m_raw_data) return m_storage->m_raw_data;
    return m_storage->m_data;
}

//***************************************************************************
void Kwave::SampleArray::setRawData(sample_t *data, unsigned int size)
{
    if (!m_storage) return;
    resize(0);
    m_storage->m_raw_data = data;
    m_storage->m_size     = size;
}

//***************************************************************************
void Kwave::SampleArray::resetRawData()
{
    if (!m_storage) return;
    m_storage->m_raw_data = 0;
    m_storage->m_size = 0;
}

//***************************************************************************
void Kwave::SampleArray::fill(sample_t value)
{
    if (!m_storage) return;
    sample_t *p = data();
    Q_ASSERT(p);
    if (!p) return;
    unsigned int count = m_storage->m_size;
    Q_ASSERT(count);
    while (count--) {
	*p = value;
	p++;
    }
}

//***************************************************************************
sample_t & Kwave::SampleArray::operator [] (unsigned int index)
{
    static sample_t dummy;

    if (!m_storage) return dummy;
    if (!m_storage->m_data && !m_storage->m_raw_data) return dummy;
    sample_t *p = data();
    return *(p + index);
}

//***************************************************************************
const sample_t & Kwave::SampleArray::operator [] (unsigned int index) const
{
    static sample_t dummy;

    if (!m_storage) return dummy;
    if (!m_storage->m_data && !m_storage->m_raw_data) return dummy;
    const sample_t *p = data();
    return *(p + index);
}

//***************************************************************************
void Kwave::SampleArray::resize(unsigned int size)
{
    if (!m_storage) return;
    if (size == m_storage->m_size) return;

    Q_ASSERT(m_storage->m_raw_data == 0);
    m_storage->resize(size);
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
    m_size     = 0;
    m_data     = 0;
    m_raw_data = 0;
}

//***************************************************************************
Kwave::SampleArray::SampleStorage::SampleStorage(const SampleStorage &other)
    :QSharedData(other)
{
    m_data     = 0;
    m_size     = 0;
    m_raw_data = 0;
    Q_ASSERT(other.m_raw_data == 0);

    if (other.m_size) {
	m_data = static_cast<sample_t *>(
	    ::malloc(other.m_size * sizeof(sample_t))
	);
	if (m_data) {
	    m_size = other.m_size;
	    MEMCPY(m_data,
		(other.m_raw_data) ? other.m_raw_data : other.m_data,
		m_size * sizeof(sample_t)
	    );
	}
    }
}

//***************************************************************************
Kwave::SampleArray::SampleStorage::~SampleStorage()
{
    Q_ASSERT(m_raw_data == 0);
    if (m_data) ::free(m_data);
}

//***************************************************************************
void Kwave::SampleArray::SampleStorage::resize(unsigned int size)
{
    Q_ASSERT(m_raw_data == 0);

    if (size) {
	// resize using realloc, keep existing data
	sample_t *new_data = static_cast<sample_t *>(
	    ::realloc(m_data, size * sizeof(sample_t)));
	if (new_data) {
	    // successful
	    // NOTE: if we grew, the additional memory is *not* initialized!
	    m_data = new_data;
	    m_size = size;
	} else {
	    qWarning("OOM in Kwave::SampleArray::SampleStorage::resize(%u)",
		size);
	}
    } else {
	// resize to zero == delete/free memory
	Q_ASSERT(m_data);
	::free(m_data);
	m_data = 0;
	m_size = 0;
    }
}

//***************************************************************************
//***************************************************************************