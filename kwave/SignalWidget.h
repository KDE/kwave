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

#ifndef _SIGNAL_WIDGET_H_
#define _SIGNAL_WIDGET_H_

#include <qlist.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qwidget.h>

#include "PlaybackController.h"
#include "SignalManager.h"

class LabelList;
class LabelType;
class MenuManager;
class MouseMark;
class QBitmap;
class SignalManager;
class TimeOperation;
class Track;
class TrackPixmap;

/**
 * The SignalWidget class is responsible for displaying a signal with
 * multiple tracks. It provides control over selecton, zoom factor and
 * the signal data itself by containing a SignalManager.
 */
class SignalWidget : public QWidget
{
    Q_OBJECT

    class InhibitRepaintGuard;
    friend class InhibitRepaintGuard;

public:
    SignalWidget(QWidget *parent, MenuManager &menu_manage);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /** Destructor */
    virtual ~SignalWidget();

    /**
     * Converts a time in milliseconds to a number of samples, based
     * on the current signal rate.
     * @param ms time in milliseconds
     * @return number of samples (rounded)
     */
    int ms2samples(double ms);

    /**
     * Converts a number of samples to a time in milliseconds, based on the
     * current signal rate.
     * @param samples number of samples (negative values allowed)
     * @return time in milliseconds
     */
    double samples2ms(int samples);

    /**
     * Closes the current signal and loads a new one
     * from a file.
     * @param filename name of the .wav or .asc file
     * @param type one of WAV or ASCII
     * @todo use QUrl instead of QString
     */
    void loadFile(const QString  &filename, int type);

    /** @todo (re)implementation */
    void saveSignal (const char *filename, int bits,
		     int type, bool selection = false);

    /** @todo (re)implementation */
    void saveBlocks (int);

    /** @todo (re)implementation */
    void setSignal (SignalManager *signal);

    /**
     * Closes the current signal
     */
    void close();

    /**
     * sets a new zoom factor [samples/pixel], does not refresh the screen
     * @param new_zoom new zoom value, will be internally limited
     *                 to [length/width...1/width] (from full display to
     *                 one visible sample only)
     */
    void setZoom(double new_zoom);

    /**
     * Returns a QBitmap with an overview of all currently present
     * signals.
     */
    QBitmap *overview(unsigned int width, unsigned int height);

    /**
     * Checks if a given sample position is near to the left or
     * the right border. The tolerance is 2% of the currently
     * visible area.
     * @param x sample position to be tested
     * @return true if the position is within range
     */
    bool checkPosition(int x);

//    void addLabelType (LabelType *);
//    void addLabelType (const char *);

    bool executeCommand(const QString &command);

    /**
     * Returns the number of tracks of the current signal or
     * 0 if no signal is loaded.
     */
    int tracks();

    /** returns the number of bits per sample of the current signal */
    int getBitsPerSample();

    /** returns the signal manager of the current signal */
    SignalManager &signalManager();

    /** Returns the playback controller */
    PlaybackController &playbackController();

public slots:

    /**
     * Sets the display offset [samples] and refreshes the screen.
     * @param new_offset new value for the offset in samples, will be
     *                   internally limited to [0...length-1]
     */
    void setOffset(unsigned int new_offset);

    /**
     * Sets the left and right selection marker and promotes
     * them to the SignalManager.
     */
    void selectRange(int left, int right);

    void forwardCommand(const QString &command);

    void signalinserted(int, int);
    void signaldeleted(int, int);
    void estimateRange(int, int);

    void toggleChannel(int);

    void playback_time();

    /**
     * (Re-)starts the playback. If playback has successfully been
     * started, the signal sigPlaybackStarted() will be emitted.
     */
    void playbackStart();

    /**
     * Called if the playback has been stopped.
     */
    void playbackStopped();

    /**
     * Returns the current number of pixels per sample
     */
    inline double zoom() { return m_zoom; };

    /**
     * Zooms into the selected range between the left and right marker.
     */
    void zoomRange();

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

    void refreshAllLayers();

protected slots:

    /**
     * Allows repainting of the display by decrementing the repaint
     * inhibit counter. If the counter reaches zero, the widget
     * will be refreshed.
     * @see m_inhibit_repaint
     * @see inhibitRepaint()
     */
    void allowRepaint();

private slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::trackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &track);

    /**
     * Connected to the signal's sigSamplesInserted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was inserted
     * @param length number of samples inserted
     * @see Signal::sigSamplesInserted
     * @internal
     */
    void slotSamplesInserted(unsigned int track, unsigned int offset,
                             unsigned int length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, unsigned int offset,
                            unsigned int length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, unsigned int offset,
                             unsigned int length);

//    /**
//     * Starts the timer for repainting the signal if necessary. If
//     * the timer is currently running then it does nothing.
//     */
//    void startRepaintTimer();

    void updatePlaybackPointer(unsigned int pos);

    void refreshSelection();

    /**
     * Refreshes the signal layer. Shortcut to refreshLayer(LAYER_SIGNAL).
     * @see #refreshLayer()
     */
    void refreshSignalLayer();

signals:

    /**
     * Signals a change in the current visible area.
     * @param offset index of the first visible sample [samples]
     * @param width the width of the widget in [pixels]
     * @param length size of the whole signal [samples]
     */
    void viewInfo(unsigned int offset, unsigned int width,
                  unsigned int length);

    void selectedTimeInfo(double ms);

    void timeInfo(double ms);

    void rateInfo(int);

    void lengthInfo(int);

    /**
     * Emits a command to be processed by the next higher instance.
     */
    void sigCommand(const QString &command);

    /**
     * Will be emitted if the zoom factor has changed due to a zoom
     * command or resize.
     * @param zoom value [samples/pixel]
     */
    void zoomInfo(double zoom);

    /**
     * Signals that a track has been inserted.
     * @param track index of the new track [0...tracks()-1]
     */
    void sigTrackInserted(unsigned int track);

protected:

    /**
     * Simple internal guard class for inhibiting and allowing
     * repaints in a SignalWidget.
     */
    class InhibitRepaintGuard
    {
    public:
        /** Constructor, inhibits repaints */
	InhibitRepaintGuard(SignalWidget &widget)
	    :m_widget(widget)
	{
	    m_widget.inhibitRepaint();
	};
	
	/** Destructor, allows repaints */
	~InhibitRepaintGuard() {
	    m_widget.allowRepaint();
	};
	
	/** reference to our owner */
	SignalWidget &m_widget;
    };

    /**
     * Returns the zoom value that will be used to fit the whole signal
     * into the current window.
     * @return zoom value [samples/pixel]
     */
    double getFullZoom();

    void selectRange ();

    void resizeEvent(QResizeEvent *);
    void mousePressEvent (QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    void mouseMoveEvent (QMouseEvent *);
    void paintEvent (QPaintEvent *);

//    void loadLabel ();
//    void appendLabel ();
//    void deleteLabel ();
//    void saveLabel (const char *);
//    void addLabel (const char *);
//    void jumptoLabel ();
//    void markSignal (const char *);
//    void markPeriods (const char *);
//    void savePeriods ();
    void createSignal (const char *);

////    void showDialog (const char *);
//
//    bool executeLabelCommand(const QString &command);
//    bool executeNavigationCommand(const QString &command);

    /**
     * Inhibits repainting by increasing the repaint inhibit counter.
     * @see m_inhibit_repaint
     * @see allowRepaint()
     */
    void inhibitRepaint();

private:

    /**
     * Refreshes a single display layer.
     */
    void refreshLayer(int layer);

    /**
     * (re)starts the timer that is responsible for redrawing
     * the playpointer if we are in playback mode.
     */
    void playback_startTimer();

    /**
     * Converts a sample index into a pixel offset using the current zoom
     * value. Always rounds up or downwards. If the number of pixels or the
     * current zoom is less than zero, the return value will be zero.
     * @param pixels pixel offset
     * @return index of the sample
     */
    unsigned int pixels2samples(int pixels);

    /**
     * Converts a pixel offset into a sample index using the current zoom
     * value. Always rounds op or downwards.
     * @param sample index of the sample
     * @return pixel offset
     */
    int samples2pixels(int samples);

    /**
     * Fixes the zoom and the offset of the display so that no non-existing
     * samples (index < 0 or index >= length) have to be displayed and the
     * current display window of the signal fits into the screen.
     */
    void fixZoomAndOffset();

private:
    /** Pixmaps for buffering each layer */
    QPixmap *m_layer[3];

    /** flags for updating each layer */
    bool m_update_layer[3];

    /** raster operation for each layer (XOR, AND, OR, ...) */
    RasterOp m_layer_rop[3];

    /**
     * Offset from which signal is beeing displayed. This is equal to
     * the index of the first visible sample.
     */
    unsigned int m_offset;

    int width, height;            //of this widget
    int lastWidth;
    int lastHeight;
    int down;                     //flags if mouse is pressed
    double lasty;
    double zoomy;

    /** number of samples per pixel */
    double m_zoom;

    //vertical line on the screen
    int playpointer, lastplaypointer;

    // flag for redrawing pixmap
    bool redraw;

    /**
     * Counter for inhibiting repaints. If not zero, repaints should
     * be inhibited.
     * @see allowRepaint()
     * @see inhibitRepaint()
     */
    unsigned int m_inhibit_repaint;

    MouseMark *select;

    /** our signal manager */
    SignalManager m_signal_manager;

    /** timer for delayed screen refreshes (don't refresh too often) */
    QTimer m_refresh_timer;

    /** list of track pixmaps */
    QList<TrackPixmap> m_track_pixmaps;

    QPainter p;
    QPixmap *pixmap;      //pixmap to be blitted to screen
    LabelList *labels;           //linked list of markers
    LabelType *markertype;       //selected marker type

    MenuManager &menu;

    /** the controller for handling of playback */
    PlaybackController m_playback_controller;
};

#endif // _SIGNAL_WIDGET_H_
