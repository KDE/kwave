/*************************************************************************
   RecordController.cpp  -  controller/state maching for the audio recorder
                             -------------------
    begin                : Sat Oct 04 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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
#include "RecordController.h"
#include "RecordState.h"

//***************************************************************************
RecordController::RecordController()
    :QObject(), m_state(REC_UNINITIALIZED), m_next_state(REC_EMPTY),
     m_trigger_set(false), m_enable_prerecording(false),
     m_empty(true)
{
}

//***************************************************************************
RecordController::~RecordController()
{
}

//***************************************************************************
void RecordController::setInitialized(bool initialized)
{
    if (initialized) {
	m_next_state = (m_empty) ? REC_EMPTY : REC_DONE;
	emit stateChanged(m_state = REC_EMPTY);
    } else {
	m_next_state = REC_UNINITIALIZED;
	emit stateChanged(REC_UNINITIALIZED);
    }
}

//***************************************************************************
void RecordController::setEmpty(bool empty)
{
    m_empty = empty;
}

//***************************************************************************
void RecordController::enablePrerecording(bool enable)
{
    m_enable_prerecording = enable;
}

//***************************************************************************
void RecordController::actionReset()
{
    qDebug("RecordController::actionReset()");

    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	    // already empty, nothing to do
	    break;
	case REC_RECORDING:
#if 0
	    // fall back to REC_WAITING_FOR_TRIGGER
	    m_next_state = REC_EMPTY;
	    emit stateChanged(m_state = REC_WAITING_FOR_TRIGGER);
	    break;
#endif
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_PAUSED:
	case REC_DONE:
	    bool accepted = true;
	    emit sigReset(accepted);
	    qDebug("RecordController::actionReset() - %d", accepted);
	    if (accepted) emit stateChanged(m_state = REC_EMPTY);
	    break;
    }
}

//***************************************************************************
void RecordController::actionStop()
{
    qDebug("RecordController::actionStop");
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_DONE:
	    // already stopped, nothing to do
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	    // abort, change to REC_EMPTY
	    emit sigStopRecord(0);
	    break;
	case REC_RECORDING:
	case REC_PAUSED:
	    // abort, change to REC_DONE
	    m_next_state = REC_DONE;
	    emit sigStopRecord(0);
	    break;
    }
}

//***************************************************************************
void RecordController::actionPause()
{
    qDebug("RecordController::actionPause");
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_DONE:
	    // what do you want ?
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	    // this should never happen
	    qWarning("RecordController::actionPause(): "
	             "state = %s ???", stateName(m_state));
	    break;
	case REC_RECORDING:
	    // pause recording
	    emit stateChanged(m_state = REC_PAUSED);
	    break;
	case REC_PAUSED:
	    // continue recording
	    emit stateChanged(m_state = REC_RECORDING);
	    break;
    }
}

//***************************************************************************
void RecordController::actionStart()
{
    qDebug("RecordController::actionStart");
    switch (m_state) {
	case REC_UNINITIALIZED:
	    break; // impossible
	case REC_EMPTY:
	case REC_DONE:
	    // interprete this as manual trigger
	    emit sigStartRecord();
	    break;
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    // interprete as "trigger now"
	    qDebug("RecordController::actionStart -> REC_RECORDING");
	    m_next_state = REC_EMPTY;
	    emit stateChanged(m_state = REC_RECORDING);
	    break;
	case REC_PAUSED:
	    // interprete this as "continue"
	    m_next_state = REC_RECORDING;
	    emit sigStartRecord();
	    break;
	case REC_RECORDING:
	    // already recording...
	    m_next_state = REC_DONE;
	    break;
    }
}

//***************************************************************************
void RecordController::deviceRecordStarted()
{
    qDebug("RecordController::deviceRecordStarted");
    switch (m_state) {
	case REC_UNINITIALIZED:
	    break; // impossible
	case REC_EMPTY:
	case REC_PAUSED:
	case REC_DONE:
	    // continue, pre-recording or trigger
	    qDebug("RecordController::deviceRecordStarted -> REC_BUFFERING");
	    m_next_state = (m_empty) ? REC_EMPTY : REC_DONE;
	    emit stateChanged(m_state = REC_BUFFERING);
	    break;
	case REC_BUFFERING:
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_RECORDING:
	    // this should never happen
	    qWarning("RecordController::deviceRecordStarted(): "
	             "state = %s ???", stateName(m_state));
	    break;
    }
}

//***************************************************************************
void RecordController::deviceBufferFull()
{
    qDebug("RecordController::deviceBufferFull");
    switch (m_state) {
	case REC_UNINITIALIZED:
	    break; // impossible
	case REC_EMPTY:
	    // we are only "recording" for updating the level
	    // meters and other effects -> no state change
	    break;
	case REC_WAITING_FOR_TRIGGER:
	case REC_PRERECORDING:
	case REC_RECORDING:
	    // this should never happen
	    qWarning("RecordController::deviceBufferFull(): "
	             "state = %s ???", stateName(m_state));
	    break;
	case REC_PAUSED: /* == buffering again after pause */
	    // -> will change to "REC_BUFFERING" soon...
	    break;
	case REC_BUFFERING:
	    if (m_enable_prerecording) {
		// prerecording was set
		qDebug("RecordController::deviceBufferFull "\
		       "-> REC_PRERECORDING");
		m_state = REC_PRERECORDING;
	    } else if (m_trigger_set) {
		// trigger was set
		qDebug("RecordController::deviceBufferFull "\
		       "-> REC_WAITING_FOR_TRIGGER");
		m_state = REC_WAITING_FOR_TRIGGER;
	    } else {
		// default: just start recording
		qDebug("RecordController::deviceBufferFull "\
		       "-> REC_RECORDING");
		m_next_state = REC_DONE;
		m_state = REC_RECORDING;
	    }
	    emit stateChanged(m_state);
	    break;
	case REC_DONE:
	    // might occur when the buffer content is flushed
	    // after a stop
	    break;
    }
}

//***************************************************************************
void RecordController::enableTrigger(bool enable)
{
    m_trigger_set = enable;
}

//***************************************************************************
void RecordController::deviceTriggerReached()
{
    qDebug("RecordController::deviceTriggerReached");
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_BUFFERING:
	case REC_RECORDING:
	case REC_PAUSED:
	case REC_DONE:
	    // this should never happen
	    qWarning("RecordController::deviceTriggerReached(): "
	             "state = %s ???", stateName(m_state));
	    break;
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    Q_ASSERT(m_trigger_set);
	    if ((m_enable_prerecording) &&
	        (m_state == REC_WAITING_FOR_TRIGGER))
	    {
		// prerecording was set
		qDebug("RecordController::deviceTriggerReached "\
		       "-> REC_PRERECORDING");
		m_state = REC_PRERECORDING;
	    } else {
		// default: just start recording
		m_state = REC_RECORDING;
		m_next_state = REC_DONE;
		qDebug("RecordController::deviceTriggerReached "\
		       "-> REC_RECORDING");
	    }
	    emit stateChanged(m_state);
	    break;
    }
}

//***************************************************************************
void RecordController::deviceRecordStopped(int)
{
    qDebug("RecordController::deviceRecordStopped()");
    switch (m_state) {
	case REC_UNINITIALIZED:
	case REC_EMPTY:
	case REC_DONE:
	    // this could happen when an abort occurs during buffering
	    emit stateChanged(m_state);
	    break;
	case REC_BUFFERING:
	case REC_PRERECORDING:
	case REC_WAITING_FOR_TRIGGER:
	    // abort, no real data produced
	    if (m_empty) {
		qDebug("RecordController::deviceRecordStopped -> REC_EMPTY");
		emit stateChanged(m_state = REC_EMPTY);
	    } else {
		qDebug("RecordController::deviceRecordStopped -> REC_DONE");
		emit stateChanged(m_state = REC_DONE);
	    }
	    break;
	case REC_RECORDING:
	    // recording -> pause or done
	    switch (m_next_state) {
		case REC_EMPTY:
		    // something went wrong when starting the recorder
		    if (m_empty) {
			qDebug("RecordController::deviceRecordStopped"\
			       " -> REC_EMPTY");
			emit stateChanged(m_state = REC_EMPTY);
		    } else {
			qDebug("RecordController::deviceRecordStopped"\
			       " -> REC_DONE");
			emit stateChanged(m_state = REC_DONE);
		    }
		    break;
		case REC_PAUSED:
		    qDebug("RecordController::deviceRecordStopped "\
		           "-> REC_PAUSED");
		    emit stateChanged(m_state = REC_PAUSED);
		    break;
		case REC_DONE:
		    qDebug("RecordController::deviceRecordStopped "\
		           "-> REC_DONE");
		    emit stateChanged(m_state = REC_DONE);
		    break;
		default:
		    qWarning("RecordController::deviceRecordStopped(): "
		             "next state = %s ???", stateName(m_next_state));
	    }
	    break;
	case REC_PAUSED:
	    // pause -> done
	    qDebug("RecordController::deviceRecordStopped -> REC_DONE");
	    emit stateChanged(m_state = REC_DONE);
	    break;
    }
}

//***************************************************************************
const char *RecordController::stateName(const RecordState state)
{
    switch (state) {
	case REC_UNINITIALIZED:		return "REC_UNINITIALIZED";
	case REC_EMPTY:			return "REC_EMPTY";
	case REC_BUFFERING:		return "REC_BUFFERING";
	case REC_WAITING_FOR_TRIGGER:	return "REC_WAITING_FOR_TRIGGER";
	case REC_PRERECORDING:		return "REC_PRERECORDING";
	case REC_RECORDING:		return "REC_RECORDING";
	case REC_PAUSED:		return "REC_PAUSED";
	case REC_DONE:			return "REC_DONE";
    }
    return "-INVALID-";
}

//***************************************************************************
#include "RecordController.moc"
//***************************************************************************
//***************************************************************************
