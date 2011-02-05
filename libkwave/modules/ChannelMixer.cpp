/*************************************************************************
       ChannelMixer.cpp  -  matrix based mixer for multiple channels
                             -------------------
    begin                : Sun Oct 10 2010
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

#include "config.h"

#include <QObject>
#include <QMutexLocker>

#include "libkwave/modules/ChannelMixer.h"
#include "libkwave/modules/Indexer.h"
#include "libkwave/modules/KwaveStreamObject.h"

//***************************************************************************
Kwave::ChannelMixer::ChannelMixer(unsigned int inputs, unsigned int outputs)
    :Kwave::SampleSource(),
     m_inputs(inputs),
     m_outputs(outputs),
     m_indexer(),
     m_input_queue(),
     m_output_buffer(),
     m_lock()
{
    bool ok = true;

    // create indexing proxies and connect their output to this mixer
    for (unsigned int index = 0; index < inputs; index++) {
	QPointer<Kwave::StreamObject> indexer = new Kwave::Indexer(index);
	Q_ASSERT(indexer);
	if (!indexer) { ok = false; break; }

	m_indexer.append(indexer);
	ok = Kwave::connect(
	    *indexer, SIGNAL(output(unsigned int, Kwave::SampleArray)),
	    *this,    SLOT(  input( unsigned int, Kwave::SampleArray)));
	Q_ASSERT(ok);
	if (!ok) break;
    }

    m_input_queue.resize(inputs);
    Q_ASSERT(m_input_queue.count() == static_cast<int>(inputs));
    ok &= (m_input_queue.count() == static_cast<int>(inputs));

    // create buffers for the input data
    for (unsigned int index = 0; index < inputs; index++) {
	QPointer<Kwave::SampleBuffer> in_buffer = new Kwave::SampleBuffer();
	Q_ASSERT(in_buffer);
	if (!in_buffer) { ok = false; break; }
    }

    // create the buffers for the output data
    for (unsigned int index = 0; index < inputs; index++) {
	// create a buffer for the input
	QPointer<Kwave::SampleBuffer> out_buffer = new Kwave::SampleBuffer();
	Q_ASSERT(out_buffer);
	if (!out_buffer) { ok = false; break; }
	m_output_buffer.append(out_buffer);
    }

}

//***************************************************************************
Kwave::ChannelMixer::~ChannelMixer()
{
    QMutexLocker _lock(&m_lock);

    while (!m_indexer.isEmpty()) {
	QPointer<Kwave::StreamObject> indexer = m_indexer[0];
	if (indexer) delete indexer;
	m_indexer.remove(0);
    }

    m_input_queue.clear();

    while (!m_output_buffer.isEmpty()) {
	QPointer<Kwave::SampleBuffer> buffer = m_output_buffer[0];
	if (buffer) delete buffer;
	m_output_buffer.remove(0);
    }
}

//***************************************************************************
unsigned int Kwave::ChannelMixer::tracksOfPort(const QString &port) const
{
    QMutexLocker _lock(const_cast<QMutex *>(&m_lock));

    if (port == QString(SLOT(input(Kwave::SampleArray)))) {
	// input ports
	return m_inputs;
    } else if (port == QString(SIGNAL(output(Kwave::SampleArray)))) {
	// output ports
	return m_outputs;
    }
    return 0;
}

//***************************************************************************
Kwave::StreamObject *Kwave::ChannelMixer::port(const QString &port,
                                               unsigned int track)
{
    QMutexLocker _lock(&m_lock);

    if (port == QString(SLOT(input(Kwave::SampleArray)))) {
	// input proxy
	Q_ASSERT(static_cast<int>(track) < m_indexer.count());
	if (static_cast<int>(track) >= m_indexer.count()) return 0;
	return m_indexer.at(track);
    } else if (port == QString(SIGNAL(output(Kwave::SampleArray)))) {
	// output proxy
	Q_ASSERT(static_cast<int>(track) < m_output_buffer.count());
	if (static_cast<int>(track) >= m_output_buffer.count()) return 0;
	return m_output_buffer[track];
    }
    return 0;
}

//***************************************************************************
void Kwave::ChannelMixer::input(unsigned int index, Kwave::SampleArray data)
{
    QMutexLocker _lock(&m_lock);

    // put the data into the corresponding input queue
    Q_ASSERT(index < m_inputs);
    Q_ASSERT(static_cast<int>(index) < m_input_queue.count());
    if (static_cast<int>(index) < m_input_queue.count())
	m_input_queue[index].enqueue(data);

    // check: if there is one empty queue we are not yet ready for mixing
    bool ready = true;
    foreach (const QQueue<Kwave::SampleArray> &queue, m_input_queue) {
	if (queue.isEmpty()) {
	    ready = false;
	    break;
	}
    }

    // mix if we are ready
    if (ready) mix();
}

//***************************************************************************
void Kwave::ChannelMixer::mix()
{
    // all inputs should contain a buffer
    
    // mix all channels together, using the mixer matrix
    
    // emit the output

}

//***************************************************************************
//***************************************************************************
