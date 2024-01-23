/*************************************************************************
   RecordController.cpp  -  controller/state matching for the audio recorder
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
Kwave::RecordController::RecordController()
    :QObject(),
     m_state(Kwave::REC_UNINITIALIZED),
     m_next_state(Kwave::REC_EMPTY),
     m_trigger_set(false),
     m_enable_prerecording(false),
     m_empty(true)
{
}

//***************************************************************************
Kwave::RecordController::~RecordController()
{
}

//***************************************************************************
void Kwave::RecordController::setInitialized(bool initialized)
{
    if (initialized) {
        m_next_state = (m_empty) ? Kwave::REC_EMPTY : Kwave::REC_DONE;
        emit stateChanged(m_state = Kwave::REC_EMPTY);
    } else {
        m_next_state = Kwave::REC_UNINITIALIZED;
        emit stateChanged(Kwave::REC_UNINITIALIZED);
    }
}

//***************************************************************************
void Kwave::RecordController::setEmpty(bool empty)
{
    m_empty = empty;
}

//***************************************************************************
void Kwave::RecordController::enablePrerecording(bool enable)
{
    m_enable_prerecording = enable;
}

//***************************************************************************
void Kwave::RecordController::actionReset()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
            // already empty, nothing to do
            break;
        case Kwave::REC_RECORDING:
#if 0
            // fall back to REC_WAITING_FOR_TRIGGER
            m_next_state = Kwave::REC_EMPTY;
            emit stateChanged(m_state = Kwave::REC_WAITING_FOR_TRIGGER);
            break;
#endif
        case Kwave::REC_EMPTY:
        case Kwave::REC_BUFFERING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_PAUSED:
        case Kwave::REC_DONE:
            bool accepted = true;
            emit sigReset(accepted);
            if (accepted) emit stateChanged(m_state = Kwave::REC_EMPTY);
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::actionStop()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_DONE:
            // already stopped, nothing to do
            break;
        case Kwave::REC_BUFFERING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
        case Kwave::REC_PRERECORDING:
            // abort, change to REC_EMPTY
            emit sigStopRecord(0);
            break;
        case Kwave::REC_RECORDING:
        case Kwave::REC_PAUSED:
            // abort, change to REC_DONE
            m_next_state = Kwave::REC_DONE;
            emit sigStopRecord(0);
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::actionPause()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_DONE:
            // what do you want ?
            break;
        case Kwave::REC_BUFFERING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
        case Kwave::REC_PRERECORDING:
            // this should never happen
            qWarning("RecordController::actionPause(): "
                     "state = %s ???", stateName(m_state));
            break;
        case Kwave::REC_RECORDING:
            // pause recording
            emit stateChanged(m_state = Kwave::REC_PAUSED);
            break;
        case Kwave::REC_PAUSED:
            // continue recording
            emit stateChanged(m_state = Kwave::REC_RECORDING);
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::actionStart()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
            break; // impossible
        case Kwave::REC_EMPTY:
        case Kwave::REC_DONE:
            // interpret this as manual trigger
            emit sigStartRecord();
            break;
        case Kwave::REC_BUFFERING:
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
            // interpret as "trigger now"
            m_next_state = Kwave::REC_EMPTY;
            emit stateChanged(m_state = Kwave::REC_RECORDING);
            break;
        case Kwave::REC_PAUSED:
            // interpret this as "continue"
            m_next_state = Kwave::REC_RECORDING;
            emit stateChanged(m_state = Kwave::REC_RECORDING);
            break;
        case Kwave::REC_RECORDING:
            // already recording...
            m_next_state = Kwave::REC_DONE;
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::deviceRecordStarted()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
            break; // impossible
        case Kwave::REC_EMPTY:
        case Kwave::REC_PAUSED:
        case Kwave::REC_DONE:
            // continue, pre-recording or trigger
            m_next_state = (m_empty) ? Kwave::REC_EMPTY : Kwave::REC_DONE;
            emit stateChanged(m_state = Kwave::REC_BUFFERING);
            break;
        case Kwave::REC_BUFFERING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_RECORDING:
            // this should never happen
            qWarning("RecordController::deviceRecordStarted(): "
                     "state = %s ???", stateName(m_state));
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::deviceBufferFull()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
            break; // impossible
        case Kwave::REC_EMPTY:
            // we are only "recording" for updating the level
            // meters and other effects -> no state change
            break;
        case Kwave::REC_WAITING_FOR_TRIGGER:
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_RECORDING:
            // this should never happen
            qWarning("RecordController::deviceBufferFull(): "
                     "state = %s ???", stateName(m_state));
            break;
        case Kwave::REC_PAUSED: /* == buffering again after pause */
            // -> will change to "REC_BUFFERING" soon...
            break;
        case Kwave::REC_BUFFERING:
            if (m_enable_prerecording) {
                // prerecording was set
                m_state = Kwave::REC_PRERECORDING;
            } else if (m_trigger_set) {
                // trigger was set
                m_state = Kwave::REC_WAITING_FOR_TRIGGER;
            } else {
                // default: just start recording
                m_next_state = Kwave::REC_DONE;
                m_state = Kwave::REC_RECORDING;
            }
            emit stateChanged(m_state);
            break;
        case Kwave::REC_DONE:
            // might occur when the buffer content is flushed
            // after a stop
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::enableTrigger(bool enable)
{
    m_trigger_set = enable;
}

//***************************************************************************
void Kwave::RecordController::deviceTriggerReached()
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_BUFFERING:
        case Kwave::REC_RECORDING:
        case Kwave::REC_PAUSED:
        case Kwave::REC_DONE:
            // this should never happen
            qWarning("RecordController::deviceTriggerReached(): "
                     "state = %s ???", stateName(m_state));
            break;
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
            Q_ASSERT(m_trigger_set);
            if ((m_enable_prerecording) &&
                (m_state == Kwave::REC_WAITING_FOR_TRIGGER))
            {
                // prerecording was set
                m_state = Kwave::REC_PRERECORDING;
            } else {
                // default: just start recording
                m_state = Kwave::REC_RECORDING;
                m_next_state = Kwave::REC_DONE;
            }
            emit stateChanged(m_state);
            break;
    }
}

//***************************************************************************
void Kwave::RecordController::deviceRecordStopped(int)
{
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_DONE:
            // this could happen when an abort occurs during buffering
            emit stateChanged(m_state);
            break;
        case Kwave::REC_BUFFERING:
        case Kwave::REC_PRERECORDING:
        case Kwave::REC_WAITING_FOR_TRIGGER:
            // abort, no real data produced
            if (m_empty) {
                emit stateChanged(m_state = Kwave::REC_EMPTY);
            } else {
                emit stateChanged(m_state = Kwave::REC_DONE);
            }
            break;
        case Kwave::REC_RECORDING:
            // recording -> pause or done
            switch (m_next_state) {
                case Kwave::REC_EMPTY:
                    // something went wrong when starting the recorder
                    if (m_empty) {
                        emit stateChanged(m_state = Kwave::REC_EMPTY);
                    } else {
                        emit stateChanged(m_state = Kwave::REC_DONE);
                    }
                    break;
                case Kwave::REC_PAUSED:
                    emit stateChanged(m_state = Kwave::REC_PAUSED);
                    break;
                case Kwave::REC_DONE:
                    emit stateChanged(m_state = Kwave::REC_DONE);
                    break;
                default:
                    qWarning("RecordController::deviceRecordStopped(): "
                             "next state = %s ???", stateName(m_next_state));
            }
            break;
        case Kwave::REC_PAUSED:
            // pause -> done
            emit stateChanged(m_state = Kwave::REC_DONE);
            break;
    }
}

//***************************************************************************
const char *Kwave::RecordController::stateName(const Kwave::RecordState state)
{
    switch (state) {
        case Kwave::REC_UNINITIALIZED:       return "REC_UNINITIALIZED";
        case Kwave::REC_EMPTY:               return "REC_EMPTY";
        case Kwave::REC_BUFFERING:           return "REC_BUFFERING";
        case Kwave::REC_WAITING_FOR_TRIGGER: return "REC_WAITING_FOR_TRIGGER";
        case Kwave::REC_PRERECORDING:        return "REC_PRERECORDING";
        case Kwave::REC_RECORDING:           return "REC_RECORDING";
        case Kwave::REC_PAUSED:              return "REC_PAUSED";
        case Kwave::REC_DONE:                return "REC_DONE";
    }
    return "-INVALID-";
}

//***************************************************************************
//***************************************************************************
