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

//***************************************************************************
class MainWidget : public QWidget
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent parent widget
     */
    MainWidget(QWidget *parent);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /** Destructor. */
    virtual ~MainWidget();

    /**
     * Closes the current signal and loads a new one
     * from a file.
     * @param url URL of the file to be loaded
     * @return 0 if succeeded or error code < 0
     */
    int loadFile(const KUrl &url);

    /**
     * Closes the current signal.
     */
    void closeSignal();

    /**
     * Forwards information for creation of a new signal to the
     * signal widget and then to the signal manager.
     * @see TopWidget::newSignal
     */
    void newSignal(sample_index_t samples, double rate,
                   unsigned int bits, unsigned int tracks);

    /** Returns the current zoom factor. */
    double zoom();

    /**
     * Returns the current number of tracks of the signal or 0 if
     * no signal is loaded.
     */
    unsigned int tracks();

    /** Returns the signal manager of the current signal */
    SignalManager &signalManager();

    /** Returns the playback controller */
    PlaybackController &playbackController();

    /** Returns the width of the current view in samples */
    inline int displaySamples() {
	return m_signal_widget.displaySamples();
    }

    /** Returns the width of the current view in pixels */
    inline int displayWidth() {
	return m_signal_widget.width();
    }

protected:

    /**
     * Called if the main widget has been resized and resizes/moves
     * the signal widget and the channel controls
     */
    virtual void resizeEvent(QResizeEvent *);

    /** slot for mouse wheel events, for scrolling/zooming */
    virtual void wheelEvent(QWheelEvent *event);

protected slots:

    /** refresh the state of the lamps */
    void refreshChannelControls();

public slots:

    /**
     * Execute a Kwave text command
     * @param command a text command
     * @return zero if succeeded or negative error code if failed
     * @retval -ENOSYS is returned if the command is unknown in this component
     */
    int executeCommand(const QString &command);

    void forwardCommand(const QString &command);

    void parseKey(int key);

    /**
     * sets a new zoom factor [samples/pixel], does not refresh the screen
     * @param new_zoom new zoom value, will be internally limited
     *                 to [length/width...1/width] (from full display to
     *                 one visible sample only)
     */
    inline void setZoom(double new_zoom) {
	m_signal_widget.setZoom(new_zoom);
    }

    /**
     * Zooms into the selected range between the left and right marker.
     */
    inline void zoomSelection() { m_signal_widget.zoomSelection(); }

    /**
     * Zooms the signal to be fully visible. Equivalent to
     * setZoom(getFullZoom()).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    inline void zoomAll()       { m_signal_widget.zoomAll(); }

    /**
     * Zooms the signal to one-pixel-per-sample. Equivalent to
     * setZoom(1.0).
     * @see #setZoom()
     * @see #getFullZoom()
     */
    inline void zoomNormal()    { m_signal_widget.zoomNormal(); }

    /**
     * Zooms into the signal, the new display will show the middle
     * 33% of the current display.
     */
    inline void zoomIn()        { m_signal_widget.zoomIn(); }

    /**
     * Zooms the signal out, the current display will become the
     * middle 30% of the new display.
     */
    inline void zoomOut()       { m_signal_widget.zoomOut(); }

private slots:

    /**
     * Called if a track has been added. Updates the display by
     * resizing/re-positioning the channel controls and the signal
     * display.
     * @param track index of the inserted track
     * @see SignalWidget::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int track);

    /**
     * Called if a track has been deleted. Updates the display by
     * resizing/re-positioning the channel controls and the signal
     * display.
     * @param track index of the deleted track
     * @see SignalWidget::sigTrackDeleted
     * @internal
     */
    void slotTrackDeleted(unsigned int track);

    /** updates the scrollbar */
    void updateViewInfo(sample_index_t, unsigned int, sample_index_t);

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
    void selectedTimeInfo(sample_index_t offset, sample_index_t length,
                          double rate);

    /** Emits the current number of tracks */
    void sigTrackCount(unsigned int tracks);

    /**
     * Emits a change in the mouse cursor. This can be used to change
     * the content of a status bar if the mouse moves over a selected
     * area or a marker.
     */
    void sigMouseChanged(Kwave::MouseMark::Mode mode);

    void sigCommand(const QString &command);

private:

    /** overview widget */
    OverViewWidget *m_overview;

    /** QFrame that contains the signal widget. */
    QFrame m_signal_frame;

    /** the widget that shows the signal, scrolled within the view port */
    SignalWidget m_signal_widget;

    QFrame *m_frm_channel_controls;

    /** vertical scrollbar, only visible if channels do not fit vertically */
    QScrollBar *m_vertical_scrollbar;

    /** horizontal scrollbar, always visible */
    QScrollBar *m_horizontal_scrollbar;

    /** array of lamps, one for each channel */
    QList<MultiStateWidget *> m_lamps;

    /** the last number of channels (for detecting changes) */
    unsigned int m_last_tracks;

};

#endif /* _MAIN_WIDGET_H_ */
