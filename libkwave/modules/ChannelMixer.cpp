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

#include <QLatin1String>
#include <QMetaObject>
#include <QMutexLocker>
#include <QObject>
#include <QtGlobal>
#include <QVarLengthArray>

#include "libkwave/Matrix.h"
#include "libkwave/Sample.h"
#include "libkwave/modules/ChannelMixer.h"
#include "libkwave/modules/Indexer.h"
#include "libkwave/modules/KwaveStreamObject.h"

//***************************************************************************
Kwave::ChannelMixer::ChannelMixer(unsigned int inputs, unsigned int outputs)
    :Kwave::SampleSource(),
     m_matrix(0),
     m_inputs(inputs),
     m_outputs(outputs),
     m_indexer(),
     m_input_queue(),
     m_output_buffer(),
     m_lock()
{
}

//***************************************************************************
bool Kwave::ChannelMixer::init()
{
    if (!m_inputs || !m_outputs) return false;

    // create queues for the input data
    m_input_queue.resize(m_inputs);
    Q_ASSERT(m_input_queue.count() == static_cast<int>(m_inputs));
    if (m_input_queue.count() != static_cast<int>(m_inputs)) return false;

    // create the buffers for the output data
    for (unsigned int index = 0; index < m_outputs; index++) {
	// create a buffer for the input
	QPointer<Kwave::SampleBuffer> out_buffer = new Kwave::SampleBuffer();
	Q_ASSERT(out_buffer);
	if (!out_buffer) return false;
	m_output_buffer.append(out_buffer);
    }
    
    // create indexing proxies and connect their output to this mixer
    for (unsigned int index = 0; index < m_inputs; index++) {
	QPointer<Kwave::StreamObject> indexer = new Kwave::Indexer(index);
	Q_ASSERT(indexer);
	if (!indexer) return false;

	m_indexer.append(indexer);
	bool ok = Kwave::connect(
	    *indexer, SIGNAL(output(unsigned int, Kwave::SampleArray)),
	    *this,    SLOT(idxInput(unsigned int, Kwave::SampleArray)));
	Q_ASSERT(ok);
	if (!ok) return false;
    }

    // create the mixer matrix
    // create a translation matrix for mixing up/down to the desired
    // number of output channels
    m_matrix = new Matrix<double>(m_inputs, m_outputs);
    Q_ASSERT(m_matrix);
    if (!m_matrix) return false;

    //   -------------
    // x |     |     | 
    //   -------------
    // y |   |   |   | x*y
    //   -------------
    const double scale = static_cast<double>(m_inputs);
    for (unsigned int y = 0; y < m_outputs; y++) {
	const unsigned int y1 = y * m_inputs;
	const unsigned int y2 = y1 + m_inputs;

	for (unsigned int x = 0; x < m_inputs; x++) {
	    const unsigned int x1 = x * m_outputs;
	    const unsigned int x2 = x1 + m_outputs;

	    // get the common area of [x1 .. x2] and [y1 .. y2]
	    const unsigned int l = qMax(x1, y1);
	    const unsigned int r = qMin(x2, y2);

	    (*m_matrix)[x][y] = (r > l) ? 
		(static_cast<double>(r - l) / scale) : 0.0;
	}
    }
    
    // everything succeeded
    return true;
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
static QByteArray _sig(const QString &sig)
{
    return QMetaObject::normalizedSignature(sig.toLatin1());
}

//***************************************************************************
unsigned int Kwave::ChannelMixer::tracksOfPort(const QString &port) const
{
    QMutexLocker _lock(const_cast<QMutex *>(&m_lock));

    if (_sig(port) == _sig(SLOT(input(Kwave::SampleArray)))) {
	// input ports
	return m_inputs; // init is done
    } else if (_sig(port) == _sig(SIGNAL(output(Kwave::SampleArray)))) {
	// output ports
	return m_outputs;
    } else if (_sig(port) == 
               _sig(SLOT(idxInput(unsigned int, Kwave::SampleArray)))) {
	return 1;
    }
    qFatal("unknown port");
    return 0;
}

//***************************************************************************
Kwave::StreamObject *Kwave::ChannelMixer::port(const QString &port,
                                               unsigned int track)
{
    QMutexLocker _lock(&m_lock);

    if (_sig(port) == _sig(SLOT(input(Kwave::SampleArray)))) {
	// input proxy
	Q_ASSERT(static_cast<int>(track) < m_indexer.count());
	if (static_cast<int>(track) >= m_indexer.count()) return 0;
	return m_indexer.at(track);
    } else if (_sig(port) == _sig(SIGNAL(output(Kwave::SampleArray)))) {
	// output proxy
	Q_ASSERT(static_cast<int>(track) < m_output_buffer.count());
	if (static_cast<int>(track) >= m_output_buffer.count()) return 0;
	return m_output_buffer[track];
    } else if (_sig(port) == 
	       _sig(SLOT(idxInput(unsigned int, Kwave::SampleArray)))) {
	return this;
    }
    qFatal("unknown port");
    return 0;
}

//***************************************************************************
void Kwave::ChannelMixer::idxInput(unsigned int index, Kwave::SampleArray data)
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
    if (ready && m_matrix) mix();
}

//***************************************************************************
void Kwave::ChannelMixer::mix()
{
    Q_ASSERT(m_matrix);

    // all inputs should contain a buffer, dequeue them into a vector
    // and form an array of pointers to the raw data, for speeding up
    QVector<Kwave::SampleArray> v_input(m_inputs);
    QVarLengthArray<const sample_t *> input(m_inputs);
    unsigned int min_len = UINT_MAX;
    for (unsigned int track = 0; track < m_inputs; track++) {
	// dequeue the buffer with input data
	QQueue<Kwave::SampleArray> &queue = m_input_queue[track];
	Q_ASSERT(!queue.isEmpty());
	Kwave::SampleArray buffer = queue.dequeue();
	v_input[track] = buffer;

	// get a pointer for quick access
	const sample_t *raw_data = v_input[track].data();
	input[track] = raw_data;

	// detect minimum input length
	min_len = qMin(min_len, buffer.size());
    }
    Q_ASSERT(min_len);
    if (!min_len) return; // zero length buffer in the queue, data underrun?

    // make sure all output buffers are large enough
    // and build an array of pointers to the raw data, for speeding up
    QVarLengthArray<sample_t *> output(m_outputs);
    for (unsigned int track = 0; track < m_outputs; track++) {
	QPointer<Kwave::SampleBuffer> buffer = m_output_buffer[track];
	Q_ASSERT(buffer);
	if (!buffer) return;
	if (buffer->data().size() < min_len)
	    buffer->data().resize(min_len);
	if (buffer->data().size() < min_len) {
	    Q_ASSERT(buffer->data().size() >= min_len);
	    qWarning("ChannelMixer: failed to increase buffer size to %u",
		     min_len);
	    return; // OOM ?
	}
	output[track] = buffer->data().data();
    }

    // mix all channels together, using the mixer matrix
    QVarLengthArray<sample_t> v_in(m_inputs);
    for (unsigned int pos = 0; pos < min_len; pos++) {
	
	// fill the vector with raw input data
	for (unsigned int x = 0; x < m_inputs; x++) {
	    v_in[x] = *(input[x]);
	    ++(input[x]);
	}

	// multiply matrix with input to get output
	for (unsigned int y = 0; y < m_outputs; y++) {
	    double sum = 0.0;
	    for (unsigned int x = 0; x < m_inputs; x++) {
		const double f = (*m_matrix)[x][y];
		const double i = static_cast<double>(v_in[x]);
		sum += (f * i);
	    }
	    *(output[y]) = static_cast<double>(sum);
	    ++(output[y]);
	}
    }

    // emit the output
    foreach (QPointer<Kwave::SampleBuffer> buffer, m_output_buffer) {
	buffer->done();
    }
}

//***************************************************************************
//***************************************************************************
