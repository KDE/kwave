/***************************************************************************
        PlayerToolBar.h  -  Toolbar with control logic for playback/record
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

#ifndef _PLAYER_TOOL_BAR_H_
#define _PLAYER_TOOL_BAR_H_

#include "config.h"

#include <QObject>
#include <QTimer>

#include "ktoolbar.h"

#include "libkwave/LabelList.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"

class QAction;
class KMainWindow;
class MenuManager;
class PlaybackController;

namespace Kwave {

    class PlayerToolBar: public KToolBar
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param parent a KMainWidget
	 * @param name the name of the toolbar (for config)
	 * @param playback the playback controller
	 * @param menu_manager the MenuManager
	 */
	PlayerToolBar(KMainWindow *parent, const QString &name,
	              PlaybackController &playback,
	              MenuManager &menu_manager
	);

	/** Destructor */
	virtual ~PlayerToolBar();

    signals:

	/** Tells this widget's parent to execute a command */
	void sigCommand(const QString &command);

	/** Emitted when a action has been enabled/disabled */
	void sigSetMenuItemEnabled(const QString &uid, bool enable);

    public slots:

	/**
	 * Executes a playback command
	 * @param command string with the command
	 */
	int executeCommand(const QString &command);

	/**
	 * Called when the meta data of the current signal has changed, to
	 * track changes in signal length
	 * @param meta_data the new meta data, after the change
	 */
	void metaDataChanged(Kwave::MetaDataList meta_data);

	/**
	 * Updates the enabled/disabled state of the seek buttons after
	 * changes of the currently visible view range
	 * @param index of the first visible sample
	 * @param visible number of visible samples
	 * @param total length of the whole signal
	 */
	void visibleRangeChanged(sample_index_t offset,
	                         sample_index_t visible,
	                         sample_index_t total);

    private slots:

	/** toolbar button for "rewind to start" pressed */
	void toolbarRewindPrev();

	/** toolbar button for "rewind" pressed */
	void toolbarRewind();

	/** toolbar button for "record" pressed */
	void toolbarRecord();

	/** playback has been paused */
	void playbackPaused();

	/** connected to the clicked() signal of the pause button */
	void pausePressed();

	/** toggles the state of the pause button */
	void blinkPause();

	/** toolbar button for "forward" pressed */
	void toolbarForward();

	/** toolbar button for "forward to end" pressed */
	void toolbarForwardNext();

	/** update the state of all toolbar buttons */
	void updateState();

	/** updates the current playback position */
	void updatePlaybackPos(sample_index_t pos);

    private:

	/** action of the "rewind to start" toolbar button */
	QAction *m_action_prev;

	/** action of the "rewind" toolbar button */
	QAction *m_action_rewind;

	/** action of the "start record" toolbar button */
	QAction *m_action_record;

	/** action of the "start playback" toolbar button */
	QAction *m_action_play;

	/** action of the "start playback and loop" toolbar button */
	QAction *m_action_loop;

	/** action of the "pause playback" toolbar button */
	QAction *m_action_pause;

	/** action of the "stop playback" toolbar button */
	QAction *m_action_stop;

	/** action of the "forward" toolbar button */
	QAction *m_action_forward;

	/** action of the "forward to end" toolbar button */
	QAction *m_action_next;

	/** Timer used to let the pause button blink... */
	QTimer *m_pause_timer;

	/** determines the state of blinking toolbar buttons */
	bool m_blink_on;

	/** reference to a playback controller */
	PlaybackController &m_playback;

	/** reference to a menu manager */
	MenuManager &m_menu_manager;

	/** list of labels (sorted) */
	LabelList m_labels;

	/** last number of tracks */
	unsigned int m_last_tracks;

	/** last offset of the current view */
	sample_index_t m_last_offset;

	/** last number of visible samples */
	sample_index_t m_last_visible;

	/** last length of the signal */
	sample_index_t m_last_length;

    };

}

#endif /* _PLAYER_TOOL_BAR_H_ */
