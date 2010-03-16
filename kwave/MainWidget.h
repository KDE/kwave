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

#ifndef _MAIN_WIDGET_H_
#define _MAIN_WIDGET_H_

#include "config.h"

#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "libgui/SignalWidget.h"

class QScrollBar;
class QWheelEvent;

class OverViewWidget;
class SignalManager;

namespace Kwave { class ApplicationContext; }

//***************************************************************************
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
 * /------------------------------------------------------------------------\
 * | m_upper_dock                                                           |
 * |------------------------------------------------------------------------|
 * | /- hbox -------------------------------------------------------------\ |
 * | +--- m_view_port---------------------------------------------------+-| |
 * | |/---- m_signal_widget--------------------------------------------\|^| |
 * | ||          |                                                     ||#| |
 * | || controls |  SignalView                                         ||#| |
 * | ||          |                                                     ||#| |
 * | ||----------+-----------------------------------------------------|||| |
 * | ||    .     |      .                                              |||| |
 * | ||    .     |      .                                              |||| |
 * | ||    .     |      .                                              ||v| |
 * | \------------------------------------------------------------------+-/ |
 * |------------------------------------------------------------------------|
 * | m_lower_dock                                                           |
 * |------------------------------------------------------------------------|
 * | ############    m_overview                                             |
 * |------------------------------------------------------------------------|
 * | <##### --------- m_horizontal_scrollbar -----------------------------> |
 * \------------------------------------------------------------------------/
 * @endcode
 */
class MainWidget : public QWidget
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent parent widget
     * @param context reference to the context of this instance
     */
    MainWidget(QWidget *parent, Kwave::ApplicationContext &context);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /** Destructor. */
    virtual ~MainWidget();

    /** Returns the current zoom factor [samples/pixel] */
    double zoom() const;

    /** Returns the width of the current view in pixels */
    int displayWidth() const;

    /** Returns the width of the current view in samples */
    sample_index_t displaySamples() const;

protected:

    /**
     * Called if the main widget has been resized and resizes/moves
     * the signal widget and the channel controls
     */
    virtual void resizeEvent(QResizeEvent *);

    /** slot for mouse wheel events, for scrolling/zooming */
    virtual void wheelEvent(QWheelEvent *event);

protected slots:

    /** resize the viewport and its contents */
    void resizeViewPort();

    /** updates all widgets that depend on the current view range */
    void updateViewRange();

public slots:

    /**
     * Execute a Kwave text command
     * @param command a text command
     * @return zero if succeeded or negative error code if failed
     * @retval -ENOSYS is returned if the command is unknown in this component
     */
    int executeCommand(const QString &command);

    /**
     * Sets the display offset [samples] and refreshes the screen.
     * @param new_offset new value for the offset in samples, will be
     *                   internally limited to [0...length-1]
     */
    void setOffset(sample_index_t new_offset);

    /**
     * sets a new zoom factor [samples/pixel], does not refresh the screen
     * @param new_zoom new zoom value, will be internally limited
     *                 to [length/width...1/width] (from full display to
     *                 one visible sample only)
     */
    void setZoom(double new_zoom);

    /**
     * Zooms into the selected range between the left and right marker.
     */
    void zoomSelection();

    /**
     * Zooms the signal to be fully visible. Equivalent to
     * setZoom(getFullZoom()).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    void zoomAll();

    /**
     * Zooms the signal to one-pixel-per-sample. Equivalent to
     * setZoom(1.0).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    void zoomNormal();

    /**
     * Zooms into the signal, the new display will show the middle
     * 33% of the current display.
     */
    void zoomIn();

    /**
     * Zooms the signal out, the current display will become the
     * middle 30% of the new display.
     */
    void zoomOut();

private slots:

    /**
     * Called if a track has been added. Updates the display by
     * resizing/re-positioning the signal views.
     * @param index the index of the inserted track [0...tracks-1]
     * @param track pointer to the track object (ignored here)
     * @see SignalManager::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track *track);

    /**
     * Called if a track has been deleted. Updates the display by
     * resizing/re-positioning the signal views.
     * @param index the index of the inserted track [0...tracks-1]
     * @see SignalManager::sigTrackDeleted
     * @internal
     */
    void slotTrackDeleted(unsigned int index);

    /** updates the scrollbar */
    void updateViewInfo(sample_index_t, sample_index_t, sample_index_t);

    /**
     * Connected to the vertical scrollbar and called if the value
     * has changed so that the signal display and the channel
     * controls have to be moved.
     */
    void verticalScrollBarMoved(int newval);

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
    double fullZoom();

    /**
     * Fixes the zoom and the offset of the display so that no non-existing
     * samples (index < 0 or index >= length) have to be displayed and the
     * current display window of the signal fits into the screen.
     * @return true if either offset or zoom has changed, otherwise false
     */
    bool fixZoomAndOffset();

private:

    /** context of the Kwave application instance */
    Kwave::ApplicationContext &m_context;

    /** upper docking area, managed by the signal widget */
    QVBoxLayout m_upper_dock;

    /** lower docking area, managed by the signal widget */
    QVBoxLayout m_lower_dock;

    /** container widget that contains the signal widget. */
    QWidget m_view_port;

    /** the widget that shows the signal, scrolled within the view port */
    SignalWidget m_signal_widget;

    /** overview widget */
    OverViewWidget *m_overview;

    /** vertical scrollbar, only visible if channels do not fit vertically */
    QScrollBar *m_vertical_scrollbar;

    /** horizontal scrollbar, always visible */
    QScrollBar *m_horizontal_scrollbar;

    /**
     * Offset from which signal is beeing displayed. This is equal to
     * the index of the first visible sample.
     */
    sample_index_t m_offset;

    /** width of the widget in pixels, cached value */
    int m_width;

    /** number of samples per pixel */
    double m_zoom;

};

#endif /* _MAIN_WIDGET_H_ */
