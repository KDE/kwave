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

#include <qframe.h>
#include <qlist.h>
#include <qwidget.h>

class QAccel;
class QComboBox;
class QScrollBar;

class KButtonBox;
class KStatusBar;

class MenuManager;
class MultiStateWidget;
class OverViewWidget;
class PlaybackController;
class SignalManager;

#include "kwave/SignalWidget.h"

//***************************************************************************
class MainWidget : public QWidget
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent parent widget
     */
    MainWidget (QWidget *parent);

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
     * @param filename name of the .wav or .asc file
     * @param type one of WAV or ASCII
     * @todo use QUrl instead of QString
     */
    void loadFile(const QString &filename, int type = 0);

    /**
     * @todo: re-implementation
     */
    void saveFile(const QString &filename, unsigned int bits,
                  int type, bool selection);

    /**
     * Closes the current signal.
     */
    void closeSignal();

    /**
     * Forwards information for creation of a new signal to the
     * signal widget and then to the signal manager.
     * @see TopWidget::newSignal
     */
    void newSignal(unsigned int samples, double rate,
                   unsigned int bits, unsigned int tracks);

    /** Returns the current zoom factor. */
    double zoom();

    /**
     * Returns the current number of tracks of the signal or 0 if
     * no signal is loaded.
     */
    unsigned int tracks();

    /** Returns the resolution in bits per sample */
    unsigned int bits();

    /** Returns the signal manager of the current signal */
    SignalManager &signalManager();

    /** Returns the playback controller */
    PlaybackController &playbackController();

protected:

    /**
     * Called if the main widget has been resized and resizes/moves
     * the signal widget and the channel controls
     */
    virtual void resizeEvent(QResizeEvent *);

    void refreshChannelControls();

public slots:

    bool executeCommand(const QString &command);

    void forwardCommand(const QString &command);

    void resetChannels();

    void parseKey(int key);

    /** calls setZoom() of the signal widget */
    inline void setZoom(double new_zoom) {
	m_signal_widget.setZoom(new_zoom);
    };

    /** calls zoomSelection() of the signal widget */
    inline void zoomSelection() { m_signal_widget.zoomSelection(); };

    /** calls zoomIn() of the signal widget */
    inline void zoomIn()        { m_signal_widget.zoomIn(); };

    /** calls zoomOut() of the signal widget */
    inline void zoomOut()       { m_signal_widget.zoomOut(); };

    /** calls zoomAll() of the signal widget */
    inline void zoomAll()       { m_signal_widget.zoomAll(); };

    /** calls zoomNormal() of the signal widget */
    inline void zoomNormal()    { m_signal_widget.zoomNormal(); };

private slots:

    /**
     * Forwards the zoomChanged signal of the iternal view window
     * by emitting sigZoomChanged.
     */
    void forwardZoomChanged(double zoom);

    /**
     * Forwards the selectedTimeInfo signal of the internal view window
     * by emitting selectedTimeInfo.
     */
    void forwardSelectedTimeInfo(unsigned int samples, double ms);

    /**
     * Forwards the sigMouseChanged signal of the internal view window
     * by emitting sigMouseChanged again.
     */
    void forwardMouseChanged(int mode);

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

    /**
     * Connected to the vertical scrollbar and called if the value
     * has changed so that the signal display and the channel
     * controls have to be moved.
     */
    void scrollbarMoved(int newval);

signals:

    /**
     * Will be emitted if the zoom factor of the
     * view has changed.
     */
    void sigZoomChanged(double zoom);

    /**
     * Emits the length of the current selection in samples
     * and in milliseconds
     */
    void selectedTimeInfo(unsigned int samples, double ms);

    /** Emits the current number of tracks */
    void sigTrackCount(unsigned int tracks);

    /**
     * Emits a change in the mouse cursor. This can be used to change
     * the content of a status bar if the mouse moves over a selected
     * area or a marker.
     */
    void sigMouseChanged(int mode);

    void sigCommand(const QString &command);

protected:

    /** Updates the overview in the horizontal slider */
    void refreshOverView();

private:

    QAccel *keys;

    OverViewWidget *m_slider;

    /** QFrame that contains the signal widget. */
    QFrame m_signal_frame;

    /** the widget that shows the signal */
    SignalWidget m_signal_widget;

    QFrame *frmChannelControls;

    /** vertical scrollbar, only visible if channels do not fit vertically */
    QScrollBar *scrollbar;

    /** array of lamps, one for each channel */
    QList<MultiStateWidget> lamps;

    /** array of speaker icons, one for each channel */
    QList<MultiStateWidget> speakers;

    /** the last number of channels (for detecting changes) */
    unsigned int lastChannels;

};

#endif /* _MAIN_WIDGET_H_ */
