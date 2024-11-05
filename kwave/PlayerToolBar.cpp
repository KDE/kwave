/***************************************************************************
      PlayerToolBar.cpp  -  Toolbar with control logic for playback/record
                             -------------------
    begin                : 2012-04-23
    copyright            : (C) 2012 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

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

#include <QtGlobal>
#include <QAction>
#include <QIcon>

#include <KLocalizedString>
#include <KMainWindow>

#include "libkwave/FileInfo.h"
#include "libkwave/Parser.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/SignalManager.h"

#include "libgui/MenuManager.h"

#include "FileContext.h"
#include "PlayerToolBar.h"

using namespace Qt::Literals::StringLiterals;

/** default width to skip when doing a "seek" = 1/10 of visible range */
#define SEEK_LENGTH (m_last_visible / 10)

/** shortcut for coupling the "enable" of a menu item to a toolbar action */
#define UPDATE_MENU(__action__,__entry__) \
    m_menu_manager.setItemEnabled(_(__entry__), \
                                  m_action_##__action__->isEnabled())

/** useful macro for command parsing */
#define CASE_COMMAND(x) } else if (command == _(x)) {

//***************************************************************************
Kwave::PlayerToolBar::PlayerToolBar(KMainWindow *parent, const QString &name,
                                    Kwave::MenuManager &menu_manager)
    :KToolBar(name, parent, true),
     m_context(nullptr),
     m_action_prev(nullptr),
     m_action_rewind(nullptr),
     m_action_record(nullptr),
     m_action_play(nullptr),
     m_action_loop(nullptr),
     m_action_pause(nullptr),
     m_action_stop(nullptr),
     m_action_forward(nullptr),
     m_action_next(nullptr),
     m_playback(nullptr),
     m_menu_manager(menu_manager),
     m_labels(),
     m_last_tracks(0),
     m_last_offset(0),
     m_last_visible(0),
     m_last_length(0)
{
    m_action_record = addAction(
        QIcon::fromTheme(u"media-record"_s),
        i18n("Record"),
        this, &Kwave::PlayerToolBar::toolbarRecord);

    m_action_play = addAction(
        QIcon::fromTheme(u"media-playback-start"_s),
        i18n("Start playback"),
        this, &Kwave::PlayerToolBar::toolbarStart);

    m_action_pause = addAction(
        QIcon::fromTheme(u"media-playback-pause"_s),
        QString(),
        this, &Kwave::PlayerToolBar::toolbarPause);
    m_action_pause->setCheckable(true);

    m_action_stop = addAction(
        QIcon::fromTheme(u"media-playback-stop"_s),
        i18n("Stop playback or loop"),
        this, &Kwave::PlayerToolBar::toolbarStop);

    m_action_loop = addAction(
        QIcon::fromTheme(u"media-repeat-all"_s),
        i18n("Start playback and loop"),
        this, &Kwave::PlayerToolBar::toolbarLoop);

    m_action_prev = addAction(
        QIcon::fromTheme(u"media-skip-backward"_s),
        i18n("Previous"),
        this, &Kwave::PlayerToolBar::toolbarRewindPrev);

    m_action_rewind = addAction(
        QIcon::fromTheme(u"media-seek-backward"_s),
        i18n("Rewind"),
        this, &Kwave::PlayerToolBar::toolbarRewind);

    m_action_forward = addAction(
        QIcon::fromTheme(u"media-seek-forward"_s),
        i18n("Forward"),
        this, &Kwave::PlayerToolBar::toolbarForward);

    m_action_next = addAction(
        QIcon::fromTheme(u"media-skip-forward"_s),
        i18n("Next"),
        this, &Kwave::PlayerToolBar::toolbarForwardNext);

    // initial state update
    updateState();
}

//***************************************************************************
Kwave::PlayerToolBar::~PlayerToolBar()
{
    m_context = nullptr;
}

//***************************************************************************
void Kwave::PlayerToolBar::contextSwitched(Kwave::FileContext *context)
{
    if (context == m_context) return; // nothing to do

    // disconnect the playback controller of the previous context
    if (m_context && m_playback) {
        disconnect(m_playback, &Kwave::PlaybackController::sigPlaybackStarted,
                   this,       &Kwave::PlayerToolBar::updateState);
        disconnect(m_playback, &Kwave::PlaybackController::sigPlaybackPaused,
                   this,       &Kwave::PlayerToolBar::updateState);
        disconnect(m_playback, &Kwave::PlaybackController::sigPlaybackStopped,
                   this,       &Kwave::PlayerToolBar::updateState);
        disconnect(m_playback, &Kwave::PlaybackController::sigPlaybackPos,
                   this,       &Kwave::PlayerToolBar::updatePlaybackPos);
        disconnect(m_context,  &Kwave::FileContext::sigVisibleRangeChanged,
                   this,       &Kwave::PlayerToolBar::visibleRangeChanged);
    }

    // use the new context
    m_context = context;

    Kwave::SignalManager *signal =
        (m_context) ? m_context->signalManager() : nullptr;
    m_playback = (signal) ? &signal->playbackController() : nullptr;

    // connect the playback controller of the new context
    if (m_context && m_playback) {
        connect(m_playback, &Kwave::PlaybackController::sigPlaybackStarted,
                this,       &Kwave::PlayerToolBar::updateState);
        connect(m_playback, &Kwave::PlaybackController::sigPlaybackPaused,
                this,       &Kwave::PlayerToolBar::updateState);
        connect(m_playback, &Kwave::PlaybackController::sigPlaybackStopped,
                this,       &Kwave::PlayerToolBar::updateState);
        connect(m_playback, &Kwave::PlaybackController::sigPlaybackPos,
                this,       &Kwave::PlayerToolBar::updatePlaybackPos);
        connect(m_context,  &Kwave::FileContext::sigVisibleRangeChanged,
                this,       &Kwave::PlayerToolBar::visibleRangeChanged);
    }

    updateState();
}

//***************************************************************************
void Kwave::PlayerToolBar::contextDestroyed(Kwave::FileContext *context)
{
    if (context != m_context) return; // not of interest
    contextSwitched(nullptr);
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarRewindPrev()
{
    if (!m_action_prev || !m_action_prev->isEnabled() || !m_playback) return;

    const bool play = m_playback->running() || m_playback->paused();
    if (play) {
        const sample_index_t first = m_playback->startPos();
        sample_index_t prev = m_labels.nextLabelLeft(m_playback->currentPos());

        if (!m_labels.isEmpty()) {
            // go to the start marker of the current block
            if (prev > first) {
                // seek back to the start of the previous block
                prev = m_labels.nextLabelLeft(prev);
            } else if (m_playback->loop()) {
                // in loop mode: wrap around to last block
                prev = m_labels.nextLabelLeft(m_playback->endPos());
            }
        }

        if (prev < first) prev = first;
        m_playback->seekTo(prev);
    } else {
        emit sigCommand(u"view:scroll_prev_label()"_s);
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarRewind()
{
    if (!m_action_rewind || !m_action_rewind->isEnabled() || !m_playback)
        return;

    const bool play = m_playback->running() || m_playback->paused();
    if (play) {
        sample_index_t first = m_playback->startPos();
        sample_index_t last  = m_playback->endPos();
        sample_index_t pos   = m_playback->currentPos();

        if (pos < first) pos = first;
        if (pos > last)  pos = last;

        if ((first + SEEK_LENGTH) <= pos) {
            // simple case: seek without wraparound
            pos = pos - SEEK_LENGTH;
        } else if (m_playback->loop() && ((first + SEEK_LENGTH) <= last)) {
            // loop mode: wrap around
            pos = last - (first + SEEK_LENGTH - pos);
        } else {
            // normal mode: stick to left border
            pos = first;
        }

        m_playback->seekTo(pos);
    } else {
        emit sigCommand(u"view:scroll_left()"_s);
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarRecord()
{
    if (!m_action_record || !m_action_record->isEnabled()) return;

    emit sigCommand(u"plugin(record)"_s);
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarStart()
{
    if (m_playback) m_playback->playbackStart();
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarLoop()
{
    if (m_playback) m_playback->playbackLoop();
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarPause()
{
    if (!m_action_pause || !m_action_pause->isEnabled() || !m_playback) return;

    const bool have_signal = (m_last_length && m_last_tracks);
    const bool playing     = m_playback->running();

    if (!have_signal) return;

    if (playing) {
        m_playback->playbackPause();
    } else {
        m_playback->playbackContinue();
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarStop()
{
    if (m_playback) m_playback->playbackStop();
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarForward()
{
    if (!m_action_forward || !m_action_forward->isEnabled() ||
        !m_playback) return;

    const bool play = m_playback->running() || m_playback->paused();
    if (play) {
        sample_index_t first = m_playback->startPos();
        sample_index_t last  = m_playback->endPos();
        sample_index_t pos   = m_playback->currentPos();

        if (pos < first) pos = first;
        if (pos > last)  pos = last;

        if ((pos + SEEK_LENGTH) <= last) {
            // simple case: seek without wraparound
            pos = pos + SEEK_LENGTH;
        } else if (m_playback->loop() && ((first + SEEK_LENGTH) <= last)) {
            // loop mode: wrap around
            pos = first + SEEK_LENGTH - (last - pos);
        } else {
            // must be loop mode but selection too small: wrap to left border
            pos = first;
        }

        m_playback->seekTo(pos);
    } else {
        emit sigCommand(u"view:scroll_right()"_s);
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarForwardNext()
{
    if (!m_action_next || !m_action_next->isEnabled() || !m_playback) return;

    const bool play = m_playback->running() || m_playback->paused();
    if (play) {
        sample_index_t last = m_playback->endPos();
        sample_index_t next = m_labels.nextLabelRight(m_playback->currentPos());
        if (next >= last)
            next = m_playback->startPos(); // wrap around to start
        m_playback->seekTo(next);
    } else {
        emit sigCommand(u"view:scroll_next_label()"_s);
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::updateState()
{
    if (!m_playback) return;

    bool have_signal    = (m_last_length && m_last_tracks);
    bool playing        = m_playback->running();
    bool paused         = m_playback->paused();
    bool at_start       = (m_last_offset == 0);
    bool at_end         = ((m_last_offset + m_last_visible) >= m_last_length);

    /* --- seek buttons, depending on mode --- */
    if (playing || paused) {
        // seek buttons in playback mode
        bool           loop       = m_playback->loop();
        sample_index_t pos        = m_playback->currentPos();
        sample_index_t first      = m_playback->startPos();
        sample_index_t last       = m_playback->endPos();
        sample_index_t prev       = m_labels.nextLabelLeft(pos);
        sample_index_t next       = m_labels.nextLabelRight(pos);
        sample_index_t seek_len   = SEEK_LENGTH;
        bool           have_label = (prev > first) || (next < last);

        m_action_prev->setEnabled(    (pos > first));
        m_action_rewind->setEnabled(  (pos > first) || (pos > prev));
        m_action_forward->setEnabled(
            loop ||                         // wrap around in loop mode
            ((pos + seek_len) < last)       // forward a bit, by range
        );
        m_action_next->setEnabled(
            (loop && have_label) ||         // wrap around in loop mode
            (next < last)                   // snap to next label
        );

        UPDATE_MENU(prev,   "ID_PLAYBACK_PREV");
        UPDATE_MENU(rewind, "ID_PLAYBACK_REWIND");
        UPDATE_MENU(forward,"ID_PLAYBACK_FORWARD");
        UPDATE_MENU(next,   "ID_PLAYBACK_NEXT");

        // scroll controls per menu
        m_menu_manager.setItemEnabled(u"ID_SCROLL_START"_s, false);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_END"_s,   false);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_PREV"_s,  false);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_NEXT"_s,  false);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_RIGHT"_s, false);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_LEFT"_s,  false);
    } else {
        // seek buttons in normal mode
        m_action_prev->setEnabled(    have_signal && !at_start);
        m_action_rewind->setEnabled(  have_signal && !at_start);
        m_action_forward->setEnabled( have_signal && !at_end);
        m_action_next->setEnabled(    have_signal && !at_end);
        m_menu_manager.setItemEnabled(u"ID_PLAYBACK_PREV"_s,    false);
        m_menu_manager.setItemEnabled(u"ID_PLAYBACK_REWIND"_s,  false);
        m_menu_manager.setItemEnabled(u"ID_PLAYBACK_FORWARD"_s, false);
        m_menu_manager.setItemEnabled(u"ID_PLAYBACK_NEXT"_s,    false);

        // scroll controls per menu
        m_menu_manager.setItemEnabled(u"ID_SCROLL_START"_s, !at_start);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_END"_s,   !at_end);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_PREV"_s,  !at_start);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_NEXT"_s,  !at_end);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_RIGHT"_s, !at_end);
        m_menu_manager.setItemEnabled(u"ID_SCROLL_LEFT"_s,  !at_start);
    }

    /* --- standard record/playback controls --- */
    m_action_record->setEnabled(                 !playing && !paused);
    m_action_play->setEnabled(    have_signal && !playing);
    m_action_loop->setEnabled(    have_signal && !playing);
    m_action_pause->setEnabled(                   playing ||  paused);
    m_action_stop->setEnabled(                    playing ||  paused);
    UPDATE_MENU(record, "ID_RECORD");
    UPDATE_MENU(play,   "ID_PLAYBACK_START");
    UPDATE_MENU(loop,   "ID_PLAYBACK_LOOP");
    UPDATE_MENU(stop,   "ID_PLAYBACK_STOP");
    m_menu_manager.setItemEnabled(u"ID_PLAYBACK_PAUSE"_s,    playing);
    m_menu_manager.setItemEnabled(u"ID_PLAYBACK_CONTINUE"_s, paused);

    /* pause button: continue/pause playback */
    if (paused) {
        m_action_pause->setToolTip(i18n("Continue playback"));
        m_action_pause->setChecked(true);
    } else {
        m_action_pause->setToolTip(i18n("Pause playback"));
        m_action_pause->setChecked(false);
    }

}

//***************************************************************************
void Kwave::PlayerToolBar::updatePlaybackPos(sample_index_t pos)
{
    Q_UNUSED(pos)
    updateState();
}

//***************************************************************************
int Kwave::PlayerToolBar::executeCommand(const QString &command)
{
    int result = 0;
    Kwave::Parser parser(command);

    if (false) {
    CASE_COMMAND("prev")
        m_action_prev->activate(QAction::Trigger);
    CASE_COMMAND("rewind")
        m_action_rewind->activate(QAction::Trigger);
    CASE_COMMAND("start")
        m_action_play->activate(QAction::Trigger);
    CASE_COMMAND("loop")
        m_action_loop->activate(QAction::Trigger);
    CASE_COMMAND("pause")
        m_action_pause->activate(QAction::Trigger);
    CASE_COMMAND("continue")
        m_action_pause->activate(QAction::Trigger);
    CASE_COMMAND("stop")
        m_action_stop->activate(QAction::Trigger);
    CASE_COMMAND("forward")
        m_action_forward->activate(QAction::Trigger);
    CASE_COMMAND("next")
        m_action_next->activate(QAction::Trigger);
    } else {
        result = -1; // unknown command ?
    }

    return result;
}

//***************************************************************************
void Kwave::PlayerToolBar::metaDataChanged(Kwave::MetaDataList meta_data)
{
    const Kwave::FileInfo info(meta_data);
    sample_index_t length = info.length();
    unsigned int tracks   = info.tracks();
    bool playing          = m_playback && m_playback->running();

    m_labels = LabelList(meta_data);

    if (!playing && (length == m_last_length) && (tracks == m_last_tracks))
        return; // nothing interesting for us changed

    // transition empty <-> not empty
    m_last_length = length;
    m_last_tracks = tracks;

    updateState();
}

//***************************************************************************
void Kwave::PlayerToolBar::visibleRangeChanged(sample_index_t offset,
                                               sample_index_t visible,
                                               sample_index_t total)
{
    bool changed = (offset  != m_last_offset)  ||
                   (visible != m_last_visible) ||
                   (total   != m_last_length);
    if (!changed) return;

    m_last_offset  = offset;
    m_last_visible = visible;
    m_last_length  = total;

    updateState();
}

//***************************************************************************
//***************************************************************************

#include "moc_PlayerToolBar.cpp"
