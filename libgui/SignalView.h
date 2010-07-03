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
#include <QLabel>
#include <QSize>
#include <QPolygon>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "kdemacros.h"

#include "libkwave/Sample.h"

#include "libgui/MouseMark.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;
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

	/**
	 * Converts a number of samples to a time in milliseconds, based on the
	 * current signal rate.
	 * @param samples number of samples
	 * @return time in milliseconds
	 */
	double samples2ms(sample_index_t samples);

	/**
	 * Should be overwritten by subclasses that can display the currently
	 * selected range and allow the user to change the selection by mouse.
	 * @return true if mouse selection is handled
	 */
	virtual bool canHandleSelection() const { return false; }

	/**
	 * Tries to find the nearest object that is visible in this view
	 * (needed for showing a tooltip at that location).
	 * @param offset position to look at [samples]
	 * @param tolerance search tolerance [samples]
	 * @param position will receive the exact position of the object if
	 *                 found, otherwise remains unchanged
	 * @param description receives the description of the object, as it
	 *                    should be shown in the tool tip (translated),
	 *                    otherwise remains unchanged
	 * @return distance to the nearest object if something was found,
	 *         the value of the tolerance parameter or more otherwise
	 */
	virtual double findObject(double offset,
	                          double tolerance,
	                          sample_index_t &position,
	                          QString &description);

	/** slot for mouse moves, used for selection and drag&drop */
	virtual void mouseMoveEvent(QMouseEvent *e);

	/** slot for mouse press, used for selection and drag&drop */
	virtual void mousePressEvent(QMouseEvent *e);

	/** slot for mouse release, used for selection and drag&drop */
	virtual void mouseReleaseEvent(QMouseEvent *e);

	/** slot when the mouse leaves the widget */
	virtual void leaveEvent(QEvent *e);

        /**
	 * Sets the mode of the mouse cursor and emits sigMouseChanged
	 * if it differs from the previous value.
	 */
	virtual void setMouseMode(Kwave::MouseMark::Mode mode);

    signals:

	/**
	 * Emits a change in the mouse cursor. This can be used to change
	 * the content of a status bar if the mouse moves over a selected
	 * area or a marker. The "mode" parameter is one of the modes in
	 * enum MouseMode.
	 */
	void sigMouseChanged(Kwave::MouseMark::Mode mode);

	/** forward a sigCommand to the next layer */
	void sigCommand(const QString &command);

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

    protected slots:

	/**
	 * Shows the current cursor position as a tooltip
	 * @param text description of the position
	 * @param pos marker position [samples]
	 * @param ms marker position in milliseconds
	 * @param mouse the coordinates of the mouse cursor,
	 *              relative to this widget [pixel]
	 */
	virtual void showPosition(const QString &text, sample_index_t pos,
				  double ms, const QPoint &mouse);

	/** Hide the current position marker */
	virtual void hidePosition() {
	    showPosition(0, 0, 0, QPoint(-1,-1));
	}

    protected:

	/** Starts a drag & drop operation */
	virtual void startDragging();

	/** @see Qt XDND documentation */
	virtual void dragEnterEvent(QDragEnterEvent *event);

	/** @see Qt XDND documentation */
	virtual void dragLeaveEvent(QDragLeaveEvent *);

	/** @see Qt XDND documentation */
	virtual void dropEvent(QDropEvent *event);

	/** @see Qt XDND documentation */
	virtual void dragMoveEvent(QDragMoveEvent *event);

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

	class PositionWidget: public QWidget
	{
	public:
	    /** Constructor */
	    PositionWidget(QWidget *parent);

	    /** Destructor */
	    virtual ~PositionWidget();

	    /**
	     * set a new label text and alignment
	     * @param text the text of the label, can be multiline and rtf/html
	     * @param alignment the alignment of the label and the widget,
	     *                  can be Qt::AlignLeft, Qt::AlignRight or Qt::AlignHCenter
	     */
	    virtual void setText(const QString &text, Qt::Alignment alignment);

	protected:

	    /** paint event: draws the text and the arrow */
	    virtual void paintEvent(QPaintEvent *);

	    /**
	     * re-creates the mask and the polygon when
	     * size/alignment has changed
	     */
	    virtual void updateMask();

	private:

	    /** the label that contains the text */
	    QLabel *m_label;

	    /** alignment of the label / text */
	    Qt::Alignment m_alignment;

	    /** the radius of the corners [pixel] */
	    int m_radius;

	    /** the length of the arrows [pixel] */
	    int m_arrow_length;

	    /** for detecting changes: previous width */
	    Qt::Alignment m_last_alignment;

	    /** for detecting changes: previous size */
	    QSize m_last_size;

	    /** polygon used as widget outline */
	    QPolygon m_polygon;
	};

    private:
	/** mode of the mouse cursor */
	MouseMark::Mode m_mouse_mode;

	/** selection handler */
	Kwave::MouseMark m_mouse_selection;

	/**
	 * x position where the user last clicked the last time, needed fo
	 * finding out where to start a drag&drop operation [pixel]
	 */
	int m_mouse_down_x;

	/** small widget for showing the mouse cursor position */
	PositionWidget m_position_widget;

	/** timer for automatic hiding */
	QTimer m_position_widget_timer;

    };

}

#endif /* _SIGNAL_VIEW_H_ */
