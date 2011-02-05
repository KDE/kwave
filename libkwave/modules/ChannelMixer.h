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

#ifndef _CHANNEL_MIXER_H_
#define _CHANNEL_MIXER_H_

#include "config.h"

#include <QObject>
#include <QString>
#include <QVector>
#include <QPointer>
#include <QQueue>

#include "libkwave/KwaveConnect.h"
#include "libkwave/KwaveSampleSink.h"
#include "libkwave/KwaveSampleSource.h"
#include "libkwave/modules/KwaveStreamObject.h"
#include "libkwave/modules/SampleBuffer.h"

//***************************************************************************
namespace Kwave {

    class KDE_EXPORT ChannelMixer: public Kwave::SampleSource
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
	     * Returns the number of tracks of a input or output port.
	     * Can be overwritten for objects that have a different count
	     * of inputs and outputs.
	     * @param port name of the port (name of signal or slot)
	     * @return number of tracks of a input or output, default is
	     *         the same as tracks()
	     */
	    virtual unsigned int tracksOfPort(const QString &port) const;

	    /**
	     * Returns an indexed port, identified by name
	     * @param port name of the port (name of signal or slot)
	     * @param track index of the track
	     */
	    virtual Kwave::StreamObject *port(const QString &port,
	                                      unsigned int track);

	    /** does nothing, work is done automatically in mix() */
	    virtual void goOn()
	    {
	    }

	signals:

	    /** emits a block with output data */
	    void output(Kwave::SampleArray data);

	private slots:

	    /** receives a block with index + input data */
	    void input(unsigned int index, Kwave::SampleArray data);

	private:

	    /** does the calculation */
	    virtual void mix();

	private:

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

#endif /* _CHANNEL_MIXER_H_ */

//***************************************************************************
//***************************************************************************
