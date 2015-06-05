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

#include <QAction>
#include <QtGlobal>

#include <KLocalizedString>
#include <KIconLoader>
#include <KMainWindow>

#include "libkwave/FileInfo.h"
#include "libkwave/Parser.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/SignalManager.h"

#include "libgui/MenuManager.h"

#include "FileContext.h"
#include "PlayerToolBar.h"

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
     m_context(0),
     m_action_prev(0),
     m_action_rewind(0),
     m_action_record(0),
     m_action_play(0),
     m_action_loop(0),
     m_action_pause(0),
     m_action_stop(0),
     m_action_forward(0),
     m_action_next(0),
     m_pause_timer(0),
     m_blink_on(false),
     m_playback(0),
     m_menu_manager(menu_manager),
     m_labels(),
     m_last_tracks(0),
     m_last_offset(0),
     m_last_visible(0),
     m_last_length(0)
{
    KIconLoader icon_loader;
    const int max_s = KIconLoader::SizeEnormous;

    m_action_prev = addAction(
	icon_loader.loadIcon(_("kwave_player_start"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Previous"),
	this, SLOT(toolbarRewindPrev()));

    m_action_rewind = addAction(
	icon_loader.loadIcon(_("kwave_player_rew"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Rewind"),
	this, SLOT(toolbarRewind()));

    m_action_record = addAction(
	icon_loader.loadIcon(_("kwave_player_record"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Record"),
	this, SLOT(toolbarRecord()));

    m_action_play = addAction(
	icon_loader.loadIcon(_("kwave_player_play"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Start playback"),
	this, SLOT(toolbarStart()));

    m_action_loop = addAction(
	icon_loader.loadIcon(_("kwave_player_loop"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Start playback and loop"),
	this, SLOT(toolbarLoop()));

    m_action_pause = addAction(
	icon_loader.loadIcon(_("kwave_player_pause"),
	                     KIconLoader::Toolbar, max_s),
	QString(),
	this, SLOT(toolbarPause()));

    m_action_stop = addAction(
	icon_loader.loadIcon(_("kwave_player_stop"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Stop playback or loop"),
	this, SLOT(toolbarStop()));

    m_action_forward = addAction(
	icon_loader.loadIcon(_("kwave_player_fwd"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Forward"),
	this, SLOT(toolbarForward()));

    m_action_next = addAction(
	icon_loader.loadIcon(_("kwave_player_end"),
	                     KIconLoader::Toolbar, max_s),
	i18n("Next"),
	this, SLOT(toolbarForwardNext()));

    // initial state update
    updateState();
}

//***************************************************************************
Kwave::PlayerToolBar::~PlayerToolBar()
{
    if (m_pause_timer) delete m_pause_timer;
    m_pause_timer = 0;
    m_context = 0;
}

//***************************************************************************
void Kwave::PlayerToolBar::contextSwitched(Kwave::FileContext *context)
{
    if (context == m_context) return; // nothing to do

    // disconnect the playback controller of the previous context
    if (m_context && m_playback) {
	disconnect(m_playback, SIGNAL(sigPlaybackStarted()),
	           this,       SLOT(updateState()));
	disconnect(m_playback, SIGNAL(sigPlaybackPaused()),
	           this,       SLOT(playbackPaused()));
	disconnect(m_playback, SIGNAL(sigPlaybackStopped()),
	           this,       SLOT(updateState()));
	disconnect(m_playback, SIGNAL(sigPlaybackPos(sample_index_t)),
	           this,       SLOT(updatePlaybackPos(sample_index_t)));
	disconnect(m_context, SIGNAL(sigVisibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)),
	    this, SLOT(visibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)) );
    }

    // use the new context
    m_context = context;

    Kwave::SignalManager *signal = m_context ? m_context->signalManager() : 0;
    m_playback = (signal) ? &signal->playbackController() : 0;

    // connect the playback controller of the new context
    if (m_context && m_playback) {
	connect(m_playback, SIGNAL(sigPlaybackStarted()),
	        this,       SLOT(updateState()));
	connect(m_playback, SIGNAL(sigPlaybackPaused()),
	        this,       SLOT(playbackPaused()));
	connect(m_playback, SIGNAL(sigPlaybackStopped()),
	        this,       SLOT(updateState()));
	connect(m_playback, SIGNAL(sigPlaybackPos(sample_index_t)),
	        this,       SLOT(updatePlaybackPos(sample_index_t)));
	connect(m_context, SIGNAL(sigVisibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)),
	    this, SLOT(visibleRangeChanged(sample_index_t,
	    sample_index_t, sample_index_t)) );
    }

    updateState();
}

//***************************************************************************
void Kwave::PlayerToolBar::contextDestroyed(Kwave::FileContext *context)
{
    if (context != m_context) return; // not of interest
    contextSwitched(0);
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
	emit sigCommand(_("view:scroll_prev_label()"));
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarRewind()
{
    if (!m_action_rewind || !m_action_rewind->isEnabled() || !m_playback) return;

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
	emit sigCommand(_("view:scroll_left()"));
    }
}

//***************************************************************************
void Kwave::PlayerToolBar::toolbarRecord()
{
    if (!m_action_record || !m_action_record->isEnabled()) return;

    emit sigCommand(_("plugin(record)"));
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
void Kwave::PlayerToolBar::playbackPaused()
{
    updateState();

    if (!m_pause_timer) {
	m_pause_timer = new QTimer(this);
	Q_ASSERT(m_pause_timer);
	if (!m_pause_timer) return;

	m_blink_on = true;
	m_pause_timer->start(500);
	connect(m_pause_timer, SIGNAL(timeout()),
	        this, SLOT(blinkPause()));
	blinkPause();
    }
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
void Kwave::PlayerToolBar::blinkPause()
{
    KIconLoader icon_loader;
    const int max_s   = KIconLoader::SizeEnormous;
    const bool paused = m_playback && m_playback->paused();

    Q_ASSERT(m_action_pause);
    if (!m_action_pause) return;

    m_action_pause->setIcon(icon_loader.loadIcon(
	_((paused && m_blink_on) ?
	    "kwave_player_pause_2" : "kwave_player_pause"),
	KIconLoader::Toolbar, max_s)
    );

    m_blink_on = !m_blink_on;
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
	emit sigCommand(_("view:scroll_right()"));
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
	emit sigCommand(_("view:scroll_next_label()"));
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
	m_menu_manager.setItemEnabled(_("ID_SCROLL_START"), false);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_END"),   false);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_PREV"),  false);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_NEXT"),  false);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_RIGHT"), false);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_LEFT"),  false);
    } else {
	// seek buttons in normal mode
	m_action_prev->setEnabled(    have_signal && !at_start);
	m_action_rewind->setEnabled(  have_signal && !at_start);
	m_action_forward->setEnabled( have_signal && !at_end);
	m_action_next->setEnabled(    have_signal && !at_end);
	m_menu_manager.setItemEnabled(_("ID_PLAYBACK_PREV"),    false);
	m_menu_manager.setItemEnabled(_("ID_PLAYBACK_REWIND"),  false);
	m_menu_manager.setItemEnabled(_("ID_PLAYBACK_FORWARD"), false);
	m_menu_manager.setItemEnabled(_("ID_PLAYBACK_NEXT"),    false);

	// scroll controls per menu
	m_menu_manager.setItemEnabled(_("ID_SCROLL_START"), !at_start);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_END"),   !at_end);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_PREV"),  !at_start);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_NEXT"),  !at_end);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_RIGHT"), !at_end);
	m_menu_manager.setItemEnabled(_("ID_SCROLL_LEFT"),  !at_start);
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
    m_menu_manager.setItemEnabled(_("ID_PLAYBACK_PAUSE"),    playing);
    m_menu_manager.setItemEnabled(_("ID_PLAYBACK_CONTINUE"), paused);

    /* handling of blink timer */
    if (m_pause_timer && !paused) {
	// stop blinking
	m_pause_timer->stop();
	delete m_pause_timer;
	m_pause_timer = 0;

	m_blink_on = false;
	blinkPause();
	m_blink_on = false;
    }

    /* pause button: continue/pause playback */
    if (paused) {
	m_action_pause->setToolTip(i18n("Continue playback"));
    } else {
	m_action_pause->setToolTip(i18n("Pause playback"));
    }

}

//***************************************************************************
void Kwave::PlayerToolBar::updatePlaybackPos(sample_index_t pos)
{
    Q_UNUSED(pos);
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
//***************************************************************************
