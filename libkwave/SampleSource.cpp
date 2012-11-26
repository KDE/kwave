/*************************************************************************
    SampleSource.cpp -  base class with a generic sample source
                             -------------------
    begin                : Sun Oct 07 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include <QtCore/QThread>
#include <threadweaver/Job.h>
#include <threadweaver/ThreadWeaver.h>

#include "libkwave/SampleSource.h"
#include "libkwave/Utils.h"

//***************************************************************************
Kwave::SampleSource::SampleSource(QObject *parent)
    :Kwave::StreamObject(parent)
{
}

//***************************************************************************
Kwave::SampleSource::~SampleSource()
{
}

//***************************************************************************
//***************************************************************************
namespace Kwave {
    class SourceJob: public ThreadWeaver::Job
    {
    public:
	/** Constructor */
	SourceJob(Kwave::SampleSource *source);

	/** Destructor */
	virtual ~SourceJob();

	/**
	 * overloaded 'run' function that runns goOn() in the context
	 * of the worker thread.
	 */
	virtual void run();

    private:

	/** reference to the Kwave::SampleSource */
	Kwave::SampleSource *m_source;

    };
}

//***************************************************************************
Kwave::SourceJob::SourceJob(Kwave::SampleSource *source)
    :ThreadWeaver::Job(), m_source(source)
{
}

//***************************************************************************
Kwave::SourceJob::~SourceJob()
{
    int i = 0;
    while (!isFinished()) {
	qDebug("job %p waiting... #%u", static_cast<void *>(this), i++);
	Kwave::yield();
    }
    Q_ASSERT(isFinished());
}

//***************************************************************************
void Kwave::SourceJob::run()
{
    if (!m_source) return;
    m_source->goOn();
}

//***************************************************************************
ThreadWeaver::Job *Kwave::SampleSource::enqueue(ThreadWeaver::Weaver *weaver)
{
    Kwave::SourceJob *job = 0;

//     weaver=0;
    if (weaver) job = new Kwave::SourceJob(this);

    if (job && weaver) {
	// async operation in a separate thread
	weaver->enqueue(job);
    } else {
	// fallback -> synchronous/sequential execution
	goOn();
    }

    return job;
}

//***************************************************************************
#include "SampleSource.moc"
//***************************************************************************
//***************************************************************************
