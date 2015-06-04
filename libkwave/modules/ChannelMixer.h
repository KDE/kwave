/*************************************************************************
         ChannelMixer.h  -  matrix based mixer for multiple channels
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

#ifndef CHANNEL_MIXER_H
#define CHANNEL_MIXER_H

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QPointer>
#include <QtCore/QQueue>

#include "libkwave/Connect.h"
#include "libkwave/SampleSink.h"
#include "libkwave/SampleSource.h"
#include "libkwave/modules/StreamObject.h"
#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
namespace Kwave
{

    class MixerMatrix;

    class Q_DECL_EXPORT ChannelMixer: public Kwave::SampleSource
    {
	Q_OBJECT
	public:
	    /**
	     * Constructor
	     * @param inputs number of input channels
	     * @param outputs number of output channels
	     */
	    ChannelMixer(unsigned int inputs, unsigned int outputs);

	    /** Destructor */
	    virtual ~ChannelMixer();

	    /**
	     * Init function, you must call it once after creating and before
	     * using this object. If the return value is false, you should
	     * delete this object.
	     * @return true if succeeded, false if failed
	     */
	    virtual bool init();

	    /**
	     * Returns the number of tracks of a input or output port.
	     * Can be overwritten for objects that have a different count
	     * of inputs and outputs.
	     * @param port name of the port (name of signal or slot)
	     * @return number of tracks of a input or output, default is
	     *         the same as tracks()
	     */
	    virtual unsigned int tracksOfPort(const char *port) const;

	    /**
	     * Returns an indexed port, identified by name
	     * @param port name of the port (name of signal or slot)
	     * @param track index of the track
	     */
	    virtual Kwave::StreamObject *port(const char *port,
	                                      unsigned int track);

	    /** does nothing, work is done automatically in mix() */
	    virtual void goOn()
	    {
	    }

	signals:

	    /** emits a block with output data */
	    void output(Kwave::SampleArray data);

	public slots:

	    /**
	     * dummy implementation, the real "input" is a multi-track slot
	     * and available through the port(...) interface only
	     */
	    void input(Kwave::SampleArray data) { Q_UNUSED(data); }

	private slots:

	    /** receives a block with index + input data */
	    void idxInput(unsigned int index, Kwave::SampleArray data);

	private:

	    /** does the calculation */
	    virtual void mix();

	private:

	    /** mixer matrix */
	    Kwave::MixerMatrix *m_matrix;

	    /** number of inputs */
	    unsigned int m_inputs;

	    /** number of outputs */
	    unsigned int m_outputs;

	    QVector< QPointer<Kwave::StreamObject> > m_indexer;

	    /** queues for input data */
	    QVector< QQueue<Kwave::SampleArray> > m_input_queue;

	    /** buffers with output data */
	    QVector< QPointer<Kwave::SampleBuffer> > m_output_buffer;

	    /** mutex for locking access to the queues */
	    QMutex m_lock;
    };


}

#endif /* CHANNEL_MIXER_H */

//***************************************************************************
//***************************************************************************
