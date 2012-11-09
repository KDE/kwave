/***************************************************************************
      MultiTrackWriter.h - writer for multi-track signals
			     -------------------
    begin                : Sat Jun 30 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MULTI_TRACK_WRITER_H_
#define _MULTI_TRACK_WRITER_H_

#include "config.h"

#include <QObject>
#include <QList>

#include <kdemacros.h>

#include "libkwave/InsertMode.h"
#include "libkwave/MultiWriter.h"

namespace Kwave {

    class SignalManager;

    /**
     * A MultiTrackWriter encapsulates a set of <c>TrackWriter</c>s for
     * easier use of multi-track signals.
     */
    class KDE_EXPORT MultiTrackWriter: public Kwave::MultiWriter
    {
	Q_OBJECT

    private:
	/** Default constructor */
	MultiTrackWriter();

    public:
	/**
	 * Constructor
	 * @param signal_manager reference to a SignalManager
	 * @param track_list array of indices of tracks for reading
	 * @param mode specifies where and how to insert
	 * @param left index of the first sample
	 * @param right index of the last sample
	 */
	MultiTrackWriter(Kwave::SignalManager &signal_manager,
	                 const QList<unsigned int> &track_list,
	                 InsertMode mode,
	                 sample_index_t left, sample_index_t right);

	/**
	 * Constructor that opens a set of Writers using the currently
	 * selected list of tracks and the current selection. If nothing is
	 * selected, the whole signal will be selected.
	 *
	 * @param signal_manager reference to a SignalManager
	 * @param mode specifies where and how to insert
	 */
	MultiTrackWriter(Kwave::SignalManager &signal_manager, InsertMode mode);

	/** Destructor */
	virtual ~MultiTrackWriter();

    private:

	/**
	 * Intializer, see constructor...
	 *
	 * @param signal_manager reference to a SignalManager
	 * @param track_list array of indices of tracks for reading
	 * @param mode specifies where and how to insert
	 * @param left index of the first sample
	 * @param right index of the last sample
	 * @internal
	 */
	bool init(Kwave::SignalManager &signal_manager,
	          const QList<unsigned int> &track_list,
	          InsertMode mode,
	          sample_index_t left, sample_index_t right);

    };
}

#endif /* _MULTI_TRACK_WRITER_H_ */
