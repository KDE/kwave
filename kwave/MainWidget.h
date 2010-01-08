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

#include <QDockWidget>
#include <QFrame>
#include <QList>
#include <QWidget>

#include "libgui/SignalWidget.h"

class QComboBox;
class QScrollBar;
class QWheelEvent;

class KStatusBar;
class KUrl;

class MenuManager;
class MultiStateWidget;
class OverViewWidget;
class PlaybackController;
class SignalManager;

namespace Kwave { class ApplicationContext; }

//***************************************************************************
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

    /** Returns the current zoom factor. */
    double zoom() const;

    /** Returns the width of the current view in pixels */
    inline int displayWidth() {
	return m_signal_widget.width();
    }

    /** Returns the width of the current view in samples */
    int displaySamples() const;

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

    bool executeCommand(const QString &command);

    void forwardCommand(const QString &command);

//  void parseKey(int key);

    /**
     * Sets the display offset [samples] and refreshes the screen.
     * @param new_offset new value for the offset in samples, will be
     *                   internally limited to [0...length-1]
     */
    void setOffset(unsigned int new_offset);

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
    void updateViewInfo(unsigned int, unsigned int, unsigned int);

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

    /**
     * Emits the offset and length of the current selection and the
     * sample rate for converting it into milliseconds
     */
    void selectedTimeInfo(unsigned int offset, unsigned int length,
                          double rate);

    /** Emits the current number of tracks */
    void sigTrackCount(unsigned int tracks);

    /**
     * Emits a change in the mouse cursor. This can be used to change
     * the content of a status bar if the mouse moves over a selected
     * area or a marker.
     */
    void sigMouseChanged(int mode);

    void sigCommand(const QString &command);

private:

    /**
     * Converts a time in milliseconds to a number of samples, based
     * on the current signal rate.
     * @param ms time in milliseconds
     * @return number of samples (rounded)
     */
    unsigned int ms2samples(double ms);

    /**
     * Converts a number of samples to a time in milliseconds, based on the
     * current signal rate.
     * @param samples number of samples
     * @return time in milliseconds
     */
    double samples2ms(unsigned int samples);

    /**
     * Converts a sample index into a pixel offset using the current zoom
     * value. Always rounds up or downwards. If the number of pixels or the
     * current zoom is less than zero, the return value will be zero.
     * @param pixels pixel offset
     * @return index of the sample
     */
    unsigned int pixels2samples(int pixels) const;

    /**
     * Converts a pixel offset into a sample index using the current zoom
     * value. Always rounds op or downwards.
     * @param samples number of samples to be converted
     * @return pixel offset
     */
    int samples2pixels(int samples) const;

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

    /** overview widget */
    OverViewWidget *m_overview;

    /** QFrame that contains the signal widget. */
    QFrame m_view_port;

    /** the widget that shows the signal, scrolled within the view port */
    SignalWidget m_signal_widget;

    /** vertical scrollbar, only visible if channels do not fit vertically */
    QScrollBar *m_vertical_scrollbar;

    /** horizontal scrollbar, always visible */
    QScrollBar *m_horizontal_scrollbar;

    /**
     * Offset from which signal is beeing displayed. This is equal to
     * the index of the first visible sample.
     */
    unsigned int m_offset;

    /** width of the widget in pixels, cached value */
    int m_width;

    /** number of samples per pixel */
    double m_zoom;

};

#endif /* _MAIN_WIDGET_H_ */
