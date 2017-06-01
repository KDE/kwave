/***************************************************************************
            MainWidget.h  -  main widget of the Kwave TopWidget
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include "config.h"

#include <QSize>
#include <QString>
#include <QTimer>

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "libkwave/CommandHandler.h"

#include "libgui/SignalWidget.h"
#include "libgui/Zoomable.h"

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QScrollBar;
class QWheelEvent;

namespace Kwave
{

    class FileContext;
    class OverViewWidget;
    class SignalManager;

    //**************************************************************************
    /**
     * The main widget is responsible for controlling the zoom on the time axis
     * and the scrolling. For this purpose it has an optionally enabled vertical
     * scroll bar at the right side and a horizontal scroll bar plus an overview
     * widget at the lower side. The main view contains a viewport that contains
     * a signal widget, which is fit in horizontally and has variable vertical
     * size (scrolled via the vertical scroll bar if necessary).
     * @p
     * The layout looks like this:
     * @code
     * /-----------------------------------------------------------------------\
     * | m_upper_dock                                                          |
     * |-----------------------------------------------------------------------|
     * | /- hbox ------------------------------------------------------------\ |
     * | +--- m_scroll_area------------------------------------------------+-| |
     * | |/---- m_signal_widget-------------------------------------------\|^| |
     * | ||          |                                                    ||#| |
     * | || controls |  SignalView                                        ||#| |
     * | ||          |                                                    ||#| |
     * | ||----------+----------------------------------------------------|||| |
     * | ||    .     |      .                                             |||| |
     * | ||    .     |      .                                             |||| |
     * | ||    .     |      .                                             ||v| |
     * | \-----------------------------------------------------------------+-/ |
     * |-----------------------------------------------------------------------|
     * | m_lower_dock                                                          |
     * |-----------------------------------------------------------------------|
     * | ############    m_overview                                            |
     * |-----------------------------------------------------------------------|
     * | <##### --------- m_horizontal_scrollbar ----------------------------> |
     * \-----------------------------------------------------------------------/
     * @endcode
     */
    class MainWidget: public QWidget,
                      public CommandHandler,
                      public Zoomable
    {
	Q_OBJECT
    public:

	/**
	 * Constructor.
	 * @param parent parent widget
	 * @param context reference to the context of this instance
	 * @param preferred_size preferred size of the widget,
	 *                       needed in MDI mode, otherwise ignored
	 */
	MainWidget(QWidget *parent,
	           Kwave::FileContext &context,
	           const QSize &preferred_size);

	/**
	 * Returns true if this instance was successfully initialized, or
	 * false if something went wrong during initialization.
	 */
	virtual bool isOK();

	/** Destructor. */
	virtual ~MainWidget();

	/** Returns the current zoom factor [samples/pixel] */
	double zoom() const Q_DECL_OVERRIDE;

	/** Returns the width of the current view in pixels */
	int visibleWidth() const Q_DECL_OVERRIDE;

	/** Returns the width of the current view in samples */
	sample_index_t visibleSamples() const Q_DECL_OVERRIDE;

	/** Returns the current start position of the visible area [samples] */
	virtual sample_index_t visibleOffset() { return m_offset; }

	/** Returns the preferred size of the widget */
	QSize sizeHint () const Q_DECL_OVERRIDE { return m_preferred_size; }

    protected:

	/** @see Qt XDND documentation */
	void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;

	/**
	 * For dropping data into an empty signal
	 * @see Qt XDND documentation
	 */
	void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;

	/**
	 * Called if the main widget has been resized and resizes/moves
	 * the signal widget and the channel controls
	 */
	void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

	/** slot for mouse wheel events, for scrolling/zooming */
	void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

	/** @see QWidget::closeEvent() */
	void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

    protected slots:

	/** updates all widgets that depend on the current view range */
	void updateViewRange();

    public slots:

	/**
	 * Execute a Kwave text command
	 * @param command a text command
	 * @return zero if succeeded or negative error code if failed
	 * @retval -ENOSYS is returned if the command is unknown in this
	 *                 component
	 */
	int executeCommand(const QString &command) Q_DECL_OVERRIDE;

	/**
	 * Sets the display offset [samples] and refreshes the screen.
	 * @param new_offset new value for the offset in samples, will be
	 *                   internally limited to [0...length-1]
	 */
	void setOffset(sample_index_t new_offset);

	/**
	 * Scrolls the display so that the given position gets visible,
	 * centered within the display if possible.
	 */
	void scrollTo(sample_index_t pos) Q_DECL_OVERRIDE;

	/**
	 * sets a new zoom factor [samples/pixel], does not refresh the screen
	 * @param new_zoom new zoom value, will be internally limited
	 *                 to [length/width...1/width] (from full display to
	 *                 one visible sample only)
	 */
	void setZoom(double new_zoom) Q_DECL_OVERRIDE;

	/**
	 * Zooms into the selected range between the left and right marker.
	 */
	void zoomSelection();

	/**
	 * Zooms the signal to be fully visible.
	 * @see #setZoom()
	 */
	void zoomAll();

	/**
	 * Zooms the signal to one-pixel-per-sample. Equivalent to
	 * setZoom(1.0).
	 * @see #setZoom()
	 */
	void zoomNormal();

	/**
	 * Zooms into the signal, the new display will show the signal
	 * zoomed at the position given as parameter (if >= 0) or centered
	 * if a position < 0 is given.
	 */
	void zoomIn(int pos = -1);

	/**
	 * Zooms out the signal, the new display will show the signal
	 * zoomed at the position given as parameter (if >= 0) or centered
	 * if a position < 0 is given.
	 */
	void zoomOut(int pos = -1);

    private slots:

	/**
	 * Called if a track has been added. Updates the display by
	 * resizing/re-positioning the signal views.
	 * @param index the index of the inserted track [0...tracks-1]
	 * @param track pointer to the track object (ignored here)
	 * @see SignalManager::sigTrackInserted
	 * @internal
	 */
	void slotTrackInserted(unsigned int index, Kwave::Track *track);

	/**
	 * Called if a track has been deleted. Updates the display by
	 * resizing/re-positioning the signal views.
	 * @param index the index of the inserted track [0...tracks-1]
	 * @param track pointer to the track object (ignored here)
	 * @see SignalManager::sigTrackDeleted
	 * @internal
	 */
	void slotTrackDeleted(unsigned int index, Kwave::Track *track);

	/** refresh the scale and position of the horizontal scrollbar */
	void refreshHorizontalScrollBar();

	/** Connected to the horizontal scrollbar for scrolling left/right */
	void horizontalScrollBarMoved(int newval);

    signals:

	/**
	 * Will be emitted if the zoom factor of the
	 * view has changed.
	 */
	void sigZoomChanged(double zoom);

	/** forward a sigCommand to the next layer */
	void sigCommand(const QString &command);

	/** emitted when the visible range has changed */
	void sigVisibleRangeChanged(sample_index_t offset,
	                            sample_index_t visible,
	                            sample_index_t total);

    private:

	/**
	 * Converts a time in milliseconds to a number of samples, based
	 * on the current signal rate.
	 * @param ms time in milliseconds
	 * @return number of samples (rounded)
	 */
	sample_index_t ms2samples(double ms);

	/**
	 * Converts a sample index into a pixel offset using the current zoom
	 * value. Always rounds up or downwards. If the number of pixels or the
	 * current zoom is less than zero, the return value will be zero.
	 * @param pixels pixel offset
	 * @return index of the sample
	 */
	sample_index_t pixels2samples(unsigned int pixels) const;

	/**
	 * Converts a pixel offset into a sample index using the current zoom
	 * value. Always rounds op or downwards.
	 * @param samples number of samples to be converted
	 * @return pixel offset
	 */
	int samples2pixels(sample_index_t samples) const;

	/**
	 * Returns the zoom value that will be used to fit the whole signal
	 * into the current window.
	 * @return zoom value [samples/pixel]
	 */
	double fullZoom() const;

	/**
	 * Fixes the zoom and the offset of the display so that no non-existing
	 * samples (index < 0 or index >= length) have to be displayed and the
	 * current display window of the signal fits into the screen.
	 * @param zoom new zoom value [samples/pixel]
	 * @param offset new offset value [samples]
	 */
	void fixZoomAndOffset(double zoom, sample_index_t offset);

	/**
	 * add a new label
	 * @param pos position of the label [samples]
	 * @param description optional label description
	 */
	void addLabel(sample_index_t pos, const QString &description);

	/**
	 * Opens a dialog for editing the properties of a label
	 * @param label a Label that should be edited
	 * @return true if the dialog has been accepted,
	 *         otherwise false (canceled)
	 */
	bool labelProperties(Kwave::Label &label);

	/**
	 * load labels from a file
	 * @param filename file name from which to load the labels,
	 *                 a file open dialog will be shown if zero length
	 * @return zero if succeeded or negative error code if failed
	 */
	int loadLabels(const QString &filename);

	/**
	 * save all labels to a file
	 * @param filename file name from which to load the labels,
	 *                 a file open dialog will be shown if zero length
	 * @return zero if succeeded or negative error code if failed
	 */
	int saveLabels(const QString &filename);

    private:

	/** context of the Kwave application instance */
	Kwave::FileContext &m_context;

	/** upper docking area, managed by the signal widget */
	QVBoxLayout m_upper_dock;

	/** lower docking area, managed by the signal widget */
	QVBoxLayout m_lower_dock;

	/** container widget that contains the signal widget. */
	QScrollArea m_scroll_area;

	/** horizontal scrollbar */
	QScrollBar *m_horizontal_scrollbar;

	/** the widget that shows the signal, scrolled within the view port */
	Kwave::SignalWidget m_signal_widget;

	/** overview widget */
	Kwave::OverViewWidget *m_overview;

	/**
	* Offset from which signal is being displayed. This is equal to
	* the index of the first visible sample.
	*/
	sample_index_t m_offset;

	/** number of samples per pixel */
	double m_zoom;

	/** preferred size of the widget */
	QSize m_preferred_size;

	/** timer for delayed update */
	QTimer m_delayed_update_timer;
    };
}

#endif /* MAIN_WIDGET_H */

//***************************************************************************
//***************************************************************************
