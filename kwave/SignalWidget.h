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
#define _SIGNAL_WIDGET_H_ 1

#include <qwidget.h>
#include <qpainter.h>

#include "PlaybackController.h"
#include "SignalManager.h"

class LabelList;
class LabelType;
class MenuManager;
class MouseMark;
class QBitmap;
class SignalManager;
class ProgressDialog;
class TimeOperation;

/**
 * This class is mainly responsible for displaying
 * signals in the time-domain
 */
class SignalWidget : public QWidget
{
    Q_OBJECT
public:
    SignalWidget(QWidget *parent, MenuManager &menu_manage);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    ~SignalWidget();

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
     */
    void loadSignal(const QString  &filename, int type);

    void saveSignal (const char *filename, int bits,
		     int type, bool selection = false);
    void saveBlocks (int);
    void setSignal (SignalManager *signal);

    /**
     * Closes the current signal
     */
    void close();

    /**
     * sets the display offset [samples], does not refresh the screen
     * @param new_offset new value for the offset in samples, will be
     *                   internally limited to [0...length-1]
     */
    void setOffset(int new_offset);

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

    void addLabelType (LabelType *);
    void addLabelType (const char *);

    bool executeCommand(const QString &command);

    /**
     * Returns the number of tracks of the current signal or
     * 0 if no signal is loaded.
     */
    int tracks();

    /** returns the number of bits per sample of the current signal */
    int getBitsPerSample ();

    /** returns the signal manager of the current signal */
    SignalManager &getSignalManager();

    /** Returns the playback controller */
    PlaybackController &playbackController();

public slots:

    void slot_setOffset(int new_offset);

    /**
     * Sets the left and right selection marker and promotes
     * them to the SignalManager.
     */
    void selectRange(int left, int right);

    void forwardCommand(const QString &command);

    /**
     * Forwards signalChanged.
     * @see signalChanged()
     */
    void forwardSignalChanged(int lmarker, int rmarker);

    /**
     * Forwards sigChannelAdded()
     * @param channel index of the new channel [0...N-1]
     */
    void forwardChannelAdded(unsigned int channel);

    /**
     * Forwards sigChannelDeleted()
     * @param channel index of the deleted channel [0...N-1]
     */
    void forwardChannelDeleted(unsigned int channel);

    void showMessage(const char *caption, const char *text, int flags);

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

private slots:

    void updatePlaybackPointer(unsigned int pos);

    void refreshSelection();

signals:

    void viewInfo (int, int, int);

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
     * Indicates that the signal data within a range
     * has changed.
     * @param lmarker leftmost sample or -1 for "up to the left"
     * @param rmarker rightmost sample or -1 for "up to the right"
     */
    void signalChanged(int lmarker, int rmarker);

    /**
     * Signals that a channel has been added/inserted. The channels
     * at and after this position (if any) have moved to channel+1.
     * @param channel index of the new channel [0...N-1]
     */
    void sigChannelAdded(unsigned int channel);

    /**
     * Signals that a channel has been deleted. All following channels
     * are shifted one channel down.
     * @param channel index of the deleted channel [0...N-1]
     */
    void sigChannelDeleted(unsigned int channel);

protected:
    /**
     * Returns the zoom value that will be used to fit the whole signal
     * into the current window.
     * @return zoom value [samples/pixel]
     */
    double getFullZoom();

    void selectRange ();

    void resizeEvent(QResizeEvent *e);
    void mousePressEvent (QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    void mouseMoveEvent (QMouseEvent *);
    void paintEvent (QPaintEvent *);

    /**
     * Draws the signal as an overview with multiple samples per
     * pixel.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @param first the index of the first sample
     * @param last the index of the last sample
     */
    void drawOverviewSignal(int channel, int middle, int height,
			    int first, int last);

    /**
     * Draws the signal and interpolates the pixels between the
     * samples. The interpolation is done by using a simple FIR
     * lowpass filter.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     * @see #calculateInterpolation()
     */
    void drawInterpolatedSignal(int channel, int middle, int height);

    /**
     * Draws the signal and connects the pixels between the samples
     * by using a simple poly-line. This gets used if the current zoom
     * factor is not suitable for either an overview nor an interpolated
     * signal display.
     * @param channel the index of the channel [0..channels-1]
     * @param middle the y position of the zero line in the drawing
     *               area [pixels]
     * @param height the height of the drawing are [pixels]
     */
    void drawPolyLineSignal(int channel, int middle, int height);

    void loadLabel ();
    void appendLabel ();
    void deleteLabel ();
    void saveLabel (const char *);
    void addLabel (const char *);
    void jumptoLabel ();
    void markSignal (const char *);
    void markPeriods (const char *);
    void savePeriods ();
    void createSignal (const char *);

    /**
     * Connects all needed signals/slots from/to the signal manager.
     * @see disconnectSignalManager
     */
    void connectSignalManager();

    /**
     * Disconnects all signals/slots from/to the signal manager. This
     * is useful in order to prevent the gui to redraw too often if
     * the signal changes many properties (e.g. when closing) and we
     * are not interested.
     */
    void disconnectSignalManager();

////    void showDialog (const char *);

    bool executeLabelCommand(const QString &command);
    bool executeNavigationCommand(const QString &command);

private:

    void refreshLayer(int layer);

    /**
     * (re)starts the timer that is responsible for redrawing
     * the playpointer if we are in playback mode.
     */
    void playback_startTimer();

    /**
     * Converts a sample index into a pixel offset using the current zoom
     * value. Always rounds downwards.
     * @param pixels pixel offset
     * @return index of the sample
     */
    int pixels2samples(int pixels);

    /**
     * Converts a pixel offset into a sample index using the current zoom
     * value. Always rounds downwards.
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

    /**
     * Calculates the parameters for interpolation of the graphical
     * display when zoomed in. Allocates (new) buffer for the
     * filter coefficients of the low pass filter used for interpolation.
     * @see #interpolation_alpha
     */
    void calculateInterpolation();

    /**
     * order of the low pass filter used for interpolation
     */
    int interpolation_order;

    /**
     * buffer for filter coefficients of the low pass used for
     * interpolation
     * @see #calculateInterpolation()
     */
    float *interpolation_alpha;

    QPixmap *layer[3];
    bool update_layer[3];
    RasterOp layer_rop[3];

    int offset;                    //offset from which signal is beeing displayed
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
    int redraw;                           //flag for redrawing pixmap
    MouseMark *select;

    /** our signal manager */
    SignalManager m_signal_manager;

    QTimer *timer;
    QPainter p;
    QPixmap *pixmap;      //pixmap to be blitted to screen
    LabelList *labels;           //linked list of markers
    LabelType *markertype;       //selected marker type

    MenuManager &menu;

    /** the controller for handling of playback */
    PlaybackController m_playback_controller;
};

#endif // _SIGNAL_WIDGET_H_
