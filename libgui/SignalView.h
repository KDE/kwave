/***************************************************************************
    SignalView.h  -  base class for widgets for views to a signal
			     -------------------
    begin                : Mon Jan 18 2010
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

#ifndef _SIGNAL_VIEW_H_
#define _SIGNAL_VIEW_H_

#include "config.h"

#include <QObject>
#include <QWidget>

#include "kdemacros.h"

#include "libkwave/Sample.h"

#include "libgui/MouseMark.h"

class QMouseEvent;
class SignalManager; // forward declaration

namespace Kwave {

    class KDE_EXPORT SignalView: public QWidget
    {
	Q_OBJECT
    public:

	/** preferred location of the SignalView */
	typedef enum {
	    UpperDockTop,     /**< upper dock area, top               */
	    UpperDockBottom,  /**< upper dock area, bottom            */
	    Top,              /**< above all others                   */
	    AboveTrackTop,    /**< above the associated track, top    */
	    AboveTrackBottom, /**< above the associated track, bottom */
	    BelowTrackTop,    /**< below the associated track, top    */
	    BelowTrackBottom, /**< below the associated track, bottom */
	    Bottom,           /**< below all others                   */
	    LowerDockTop,     /**< lower dock area, top               */
	    LowerDockBottom   /**< lower dock area, bottom            */
	} Location;

	/** Mode of the mouse cursor */
	typedef enum {
	    MouseNormal = 0,        /**< over the signal [default] */
	    MouseInSelection,       /**< within the selection */
	    MouseAtSelectionBorder, /**< near the border of a selection */
	    MouseSelect             /**< during selection */
	} MouseMode;

	/**
	 * Constructor
	 * @param parent pointer to the parent widget
	 * @param controls container widget for associated controls
	 * @param signal_manager the signal manager
	 * @param preferred_location the location where to insert the view
	 * @param index (optional) index of the associated track or -1 if
	 *              not related to a specific track (default)
	 */
	SignalView(QWidget *parent, QWidget *controls,
	           SignalManager *signal_manager,
	           Location preferred_location,
	           int track = -1);

	/** Destructor */
	virtual ~SignalView();

	/** returns the preferred location */
	Location preferredLocation() const {
	    return m_preferred_location;
	}

	/** returns the index of the associated track (or -1) */
	int track() const {
	    return m_track_index;
	}

	/** returns the current start position */
	sample_index_t offset() const {
	    return m_offset;
	}

	/** returns the current zoom [pixels/sample] */
	double zoom() const {
	    return m_zoom;
	}

	/**
	 * converts a number of samples into a number of pixels,
	 * based on the current zoom factor
	 * @param samples a small number of samples (must be positive)
	 * @return number of pixels
	 */
	int samples2pixels(sample_index_t samples) const;

	/**
	 * Converts a number of pixels into a number of samples,
	 * based on the current zoom factor
	 * @param pixels number of pixels (should be positive)
	 * @return number of samples
	 */
	sample_index_t pixels2samples(int pixels) const;

	/** slot for mouse moves, used for selection and drag&drop */
	virtual void mouseMoveEvent(QMouseEvent *);

    public slots:

	/**
	 * changes the association to a track
	 * @param track the new track index, or -1 if not associated
	 */
	virtual void setTrack(int track);

	/**
	 * sets new zoom factor and offset
	 * @param zoom the new zoom factor in pixels/sample
	 * @param offset the index of the first visible sample
	 */
	virtual void setZoomAndOffset(double zoom, sample_index_t offset);

    protected:

	/**
	 * Relationship between a screen position and the current selection.
	 */
	typedef enum {
	    None        = 0x0000, /**< not near a border           */
	    LeftBorder  = 0x0001, /**< close to start of selection */
	    RightBorder = 0x0002, /**< close to end of selection   */
	    Selection   = 0x8000  /**< within the selection        */
	} SelectionPos;

	/**
	 * Determines the relationship between a screen position and
	 * the current selection.
	 * @param x screen position
	 * @return a SelectionPos
	 */
	int selectionPosition(const int x);

	/**
	 * Checks if a pixel position is near to the left or right border
	 * of a selection. The tolerance is 2% of the currently
	 * visible area.
	 * @param x pixel position to be tested
	 * @return true if the position is within range
	 */
	bool isSelectionBorder(int x);

	/**
	 * Checks if a pixel position is within the left and right border
	 * of a selection. The tolerance is 2% of the currently
	 * visible area.
	 * @param x pixel position to be tested
	 * @return true if the position is within range
	 */
	bool isInSelection(int x);

    protected:

	/** widget for displaying associated controls */
	QWidget *m_controls;

	/** the signal manager */
	SignalManager *m_signal_manager;

	/** the preferred location, as per construction */
	Location m_preferred_location;

	/** index of the associated track or -1 if no relation to a track */
	int m_track_index;

	/**
	 * Offset from which signal is beeing displayed. This is equal to
	 * the index of the first visible sample.
	 */
	sample_index_t m_offset;

	/** number of samples per pixel */
	double m_zoom;

    private:

	/** mode of the mouse cursor */
	MouseMode m_mouse_mode;

	/** selection handler */
	Kwave::MouseMark m_mouse_selection;

	/**
	 * x position where the user last clicked the last time, needed fo
	 * finding out where to start a drag&drop operation [pixel]
	 */
	int m_mouse_down_x;

    };

}

#endif /* _SIGNAL_VIEW_H_ */
