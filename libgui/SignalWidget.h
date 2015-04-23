/***************************************************************************
                SignalWidget.h - Widget for displaying the signal
			     -------------------
    begin                : Sun Nov 12 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef SIGNAL_WIDGET_H
#define SIGNAL_WIDGET_H

#include "config.h"

#include <QtGui/QGridLayout>
#include <QtGui/QImage>
#include <QtGui/QLabel>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtCore/QPointer>
#include <QtGui/QPolygon>
#include <QtCore/QQueue>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtGui/QWidget>

#include "kdemacros.h"

#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"

#include "libgui/MouseMark.h"
#include "libgui/SignalView.h"

class QBitmap;
class QContextMenuEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QDragLeaveEvent;
class QEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QPoint;
class QVBoxLayout;
class QWheelEvent;

class KUrl;

class LabelType;
class TimeOperation;
class TrackPixmap;

namespace Kwave
{

    class FileContext;
    class SignalManager;
    class Track;

    /**
     * The SignalWidget class is responsible for displaying and managing the
     * views that belong to a signal.
     */
    class KDE_EXPORT SignalWidget : public QWidget, public Kwave::ViewManager
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param parent parent widget
	 * @param signal_manager signal manager of the file context
	 * @param upper_dock layout of the upper docking area
	 * @param lower_dock layout of the lower docking area
	 */
	SignalWidget(QWidget *parent,
	             Kwave::SignalManager *signal_manager,
	             QVBoxLayout *upper_dock, QVBoxLayout *lower_dock);

	/** Destructor */
	virtual ~SignalWidget();

	/**
	 * sets new zoom factor and offset
	 * @param zoom the new zoom factor in pixels/sample
	 * @param offset the index of the first visible sample
	 */
	void setZoomAndOffset(double zoom, sample_index_t offset);

	/**
	 * Returns the x coordinate of an absolute pixel position to a relative
	 * coordinate within the viewport area (without controls)
	 */
	int mapToViewPort(const QPoint &pos) const;

	/**
	 * Returns the width of the viewport with the signal (without
	 * controls)
	 * @return width of the signal area [pixels]
	 */
	int visibleWidth() const;

	/**
	 * Insert a new signal view into this widget (or the upper/lower
	 * dock area.
	 * @param view the signal view, must not be a null pointer
	 * @param controls a widget with controls, optionally, can be null
	 */
	void insertView(Kwave::SignalView *view, QWidget *controls);

    signals:

	/** child views can connected to this signal to synchronize repaints */
	void sigRepaint();

	/**
	 * Emits a change in the mouse cursor (forwarded from SignalView).
	 * @see Kwave::SignalView
	 * @param mode one of the modes in enum MouseMode
	 * @param offset selection start (not valid if mode is MouseNormal)
	 * @param length selection length (not valid if mode is MouseNormal)
	 */
	void sigMouseChanged(Kwave::MouseMark::Mode mode,
	                     sample_index_t offset, sample_index_t length);

    public slots:

	/**
	 * can be called by views to request a repaint, synchronized and
	 * throttled through our repaint timer
	 */
	void requestRepaint(Kwave::SignalView *view);

	/** forward a sigCommand to the next layer */
	void forwardCommand(const QString &command);

    protected slots:

	/** Handler for context menus */
	void contextMenuEvent(QContextMenuEvent *e);

    private slots:

	/**
	 * called when the repaint timer has elapsed, to refresh all views that
	 * have requested a repaint and are in the repaint queue
	 */
	void repaintTimerElapsed();

	/**
	 * Connected to the signal's sigTrackInserted.
	 * @param index numeric index of the inserted track
	 * @param track reference to the inserted track
	 * @see Signal::sigTrackInserted
	 * @internal
	 */
	void slotTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Connected to the signal's sigTrackDeleted.
	 * @param index numeric index of the deleted track
	 * @param track reference to the deleted track
	 * @see Signal::sigTrackInserted
	 * @internal
	 */
	void slotTrackDeleted(unsigned int index, Kwave::Track *track);

	/** context menu: "edit/undo" */
	void contextMenuEditUndo() { forwardCommand(_("undo()")); }

	/** context menu: "edit/redo" */
	void contextMenuEditRedo() { forwardCommand(_("redo()")); }

	/** context menu: "edit/cut" */
	void contextMenuEditCut()  { forwardCommand(_("cut()"));  }

	/** context menu: "edit/copy" */
	void contextMenuEditCopy() { forwardCommand(_("copy()")); }

	/** context menu: "edit/paste" */
	void contextMenuEditPaste() {forwardCommand(_("paste()")); }

	/** context menu: "save selection" */
	void contextMenuSaveSelection() {
	    forwardCommand(_("saveselect()"));
	}

	/** context menu: "expand to labels" */
	void contextMenuSelectionExpandToLabels() {
	    forwardCommand(_("expandtolabel()"));
	}

	/** context menu: "select next labels" */
	void contextMenuSelectionNextLabels()  {
	    forwardCommand(_("selectnextlabels()"));
	}

	/** context menu: "select previous labels" */
	void contextMenuSelectionPrevLabels()  {
	    forwardCommand(_("selectprevlabels()"));
	}

	/**
	 * updates the minimum height of the widget
	 * according to the number of rows
	 */
	void updateMinimumHeight();

    signals:

	/**
	 * Emits the offset and length of the current selection and the
	 * sample rate for converting it into milliseconds
	 * @param offset index of the first selected sample
	 * @param length number of selected samples
	 * @param rate sample rate
	 */
	void selectedTimeInfo(sample_index_t offset, sample_index_t length,
	                      double rate);

	/**
	 * Emits a command to be processed by the next higher instance.
	 */
	void sigCommand(const QString &command);

    protected:

	/** slot for mouse wheel events, used for vertical zoom */
	virtual void wheelEvent(QWheelEvent *event);

    private:

	/** propagates the vertical zoom to all views */
	void setVerticalZoom(double zoom);

	/**
	 * insert a row in the m_layout, shifting all following rows to
	 * the end by one
	 * @param index the index if the row
	 * @param view a view widget, must not be null
	 * @param controls a widget with controls, can be null
	 */
	void insertRow(int index, Kwave::SignalView *view, QWidget *controls);

	/**
	 * delete a row in the m_layout, shifting all following rows back to
	 * the start by one
	 * @param index the index if the row
	 */
	void deleteRow(int index);

    private:

	/** the signal manager of the corresponding context */
	Kwave::SignalManager *m_signal_manager;

	/**
	 * list of signal views. Contains one entry for each signal view, starting
	 * with the ones in m_upper_dock, then the ones in m_layout, and at
	 * the end the ones from m_lower_dock.
	 * The list is sorted in the order of the appearance in the GUI.
	 */
	QList< QPointer<Kwave::SignalView> > m_views;

	/** the central layout with the views */
	QGridLayout m_layout;

	/** layout of the upper docking area */
	QPointer<QVBoxLayout> m_upper_dock;

	/** layout of the lower docking area */
	QPointer<QVBoxLayout> m_lower_dock;

	/**
	 * Offset from which signal is being displayed. This is equal to
	 * the index of the first visible sample.
	 */
	sample_index_t m_offset;

	/** number of samples per pixel */
	double m_zoom;

	/** vertical zoom factor */
	double m_vertical_zoom;

	/** timer for limiting the number of repaints per second */
	QTimer m_repaint_timer;

	/** queue with pointers to all signal views that need a repaint */
	QQueue< QPointer<Kwave::SignalView> > m_repaint_queue;
    };
}

#endif /* SIGNAL_WIDGET_H */

//***************************************************************************
//***************************************************************************
