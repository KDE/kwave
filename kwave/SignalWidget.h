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

#include "config.h"
#include <qdragobject.h>
#include <qlist.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qwidget.h>

#include "PlaybackController.h"
#include "SignalManager.h"

class KURL;
class LabelList;
class LabelType;
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
    /** Mode of the mouse cursor */
    enum MouseMode {
	MouseNormal = 0,        /**< over the signal [default] */
	MouseInSelection,       /**< within the selection */
	MouseAtSelectionBorder, /**< near the border of a selection */
	MouseSelect             /**< during selection */
    };

    /** Constructor */
    SignalWidget(QWidget *parent);

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
    unsigned int ms2samples(double ms);

    /**
     * Converts a number of samples to a time in milliseconds, based on the
     * current signal rate.
     * @param samples number of samples
     * @return time in milliseconds
     */
    double samples2ms(unsigned int samples);

    /**
     * Closes the current signal and loads a new one from a file.
     * @param url URL of the file to be loaded
     * @return 0 if succeeded or error code < 0
     */
    int loadFile(const KURL &url);

    /**
     * Forwards information for creation of a new signal to the
     * signal manager.
     * @see TopWidget::newSignal
     */
    void newSignal(unsigned int samples, double rate,
                   unsigned int bits, unsigned int tracks);

    /** @todo (re)implementation */
    int saveFile(const KURL &url, unsigned int bits, bool selection = false);

    /** @todo (re)implementation */
    void saveBlocks (int);

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
     * Checks if a pixel position is near to the left or right border
     * of a selection. The tolerance is 2% of the currently
     * visible area.
     * @param x pixel position to be tested
     * @return true if the position is within range
     */
    bool isSelectionBorder(int x);

    /**
     * Checks if a gpixel position is within the left and right border
     * of a selection. The tolerance is 2% of the currently
     * visible area.
     * @param x pixel position to be tested
     * @return true if the position is within range
     */
    bool isInSelection(int x);

//    void addLabelType (LabelType *);
//    void addLabelType (const char *);

    bool executeCommand(const QString &command);

    /**
     * Returns the number of tracks of the current signal or
     * 0 if no signal is loaded.
     */
    int tracks();

    /** returns the number of bits per sample of the current signal */
    unsigned int bits();

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
     * Sets a new selected range of samples. If the length of the
     * area is given as zero, nothing will be selected.
     * @param offset index of the first sample
     * @param length number of samples
     */
    void selectRange(unsigned int offset, unsigned int length);

    void forwardCommand(const QString &command);

    /**
     * Toggles the "selected" flag of a track.
     * @param track index of the track [0...tracks()-1]
     */
    void toggleTrackSelection(int track);

    /**
     * Called if the playback has been stopped.
     */
    void playbackStopped();

    /**
     * Returns the current number of pixels per sample
     */
    inline double zoom() { return m_zoom; };

    /** Returns the width of the current view in samples */
    int displaySamples();

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

    void refreshAllLayers();

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

protected slots:

    /**
     * Allows repainting of the display by decrementing the repaint
     * inhibit counter. If the counter reaches zero, the widget
     * will be refreshed.
     * @see m_inhibit_repaint
     * @see inhibitRepaint()
     */
    void allowRepaint(bool repaint);

private slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param track index of the inserted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track &track);

    /**
     * Connected to the signal's sigTrackDeleted.
     * @param track index of the deleted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackDeleted(unsigned int index);

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

    /**
     * Connected to the changed() signal of the SignalManager's selection.
     * @see Selection
     * @internal
     */
    void slotSelectionChanged(unsigned int offset, unsigned int length);


    /**
     * Updates the vertical line that represents the current playback
     * position during playback.
     * @param pos last played sample position [0...length-1]
     */
    void updatePlaybackPointer(unsigned int pos);

    /* ### */
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

    /**
     * Emits the length of the current selection in
     * samples and in milliseconds
     */
    void selectedTimeInfo(unsigned int samples, double ms);

    /**
     * Emits a command to be processed by the next higher instance.
     */
    void sigCommand(const QString &command);

    /**
     * Will be emitted if the zoom factor has changed due to a zoom
     * command or resize.
     * @param zoom value [samples/pixel]
     */
    void sigZoomChanged(double zoom);

    /**
     * Emits a change in the mouse cursor. This can be used to change
     * the content of a status bar if the mouse moves over a selected
     * area or a marker. The "mode" parameter is one of the modes in
     * enum MouseMode, but casted to int for simplicity.
     * @todo find out how to use a forward declaration of both the
     * class SignalWidget *and* of SignalWidget::MouseMouse to avoid the
     * need of including SignalWidget.h in too many files.
     */
    void sigMouseChanged(int mode);

    /**
     * Signals that a track has been inserted.
     * @param track index of the new track [0...tracks()-1]
     */
    void sigTrackInserted(unsigned int track);

    /**
     * Signals that a track has been deleted.
     * @param track index of the deleted track [0...tracks()-1]
     */
    void sigTrackDeleted(unsigned int track);

protected:

    /**
     * Relationship between a screen position and the current selection.
     */
    typedef enum {
	None        = 0x0000,
	LeftBorder  = 0x0001,
	RightBorder = 0x0002,
	Selection   = 0x8000
    } SelectionPos;

    /**
     * Determines the relationship between a screen position and
     * the current selection.
     * @param x screen position
     * @return a SelectionPos
     */
    int selectionPosition(const int x);

    /**
     * Simple internal guard class for inhibiting and allowing
     * repaints in a SignalWidget.
     */
    class InhibitRepaintGuard
    {
    public:
        /** Constructor, inhibits repaints */
	InhibitRepaintGuard(SignalWidget &widget, bool repaint=true)
	    :m_widget(widget), m_repaint(repaint)
	{
	    m_widget.inhibitRepaint();
	};
	
	/** Destructor, allows repaints */
	~InhibitRepaintGuard() {
	    m_widget.allowRepaint(m_repaint);
	};
	
	/** reference to our owner */
	SignalWidget &m_widget;
	
	/** true if repaint is needed after allow */
	bool m_repaint;
    };

    /**
     * Returns the zoom value that will be used to fit the whole signal
     * into the current window.
     * @return zoom value [samples/pixel]
     */
    double getFullZoom();

    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

//    void loadLabel ();
//    void appendLabel ();
//    void deleteLabel ();
//    void saveLabel (const char *);
//    void addLabel (const char *);
//    void jumptoLabel ();
//    void markSignal (const char *);
//    void markPeriods (const char *);
//    void savePeriods ();

//    bool executeLabelCommand(const QString &command);

    /**
     * Handles commands for navigation and selection.
     * @param command the string with the command
     * @return true if the command has been handles, false if unknown
     */
    bool executeNavigationCommand(const QString &command);

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

    /**
     * Sets the mode of the mouse cursor and emits sigMouseChanged
     * if it differs from the previous value.
     */
    void setMouseMode(MouseMode mode);

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

    int m_width;
    int m_height;            //of this widget
    int lastWidth;
    int lastHeight;

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

    MouseMark *m_selection;

    /** our signal manager */
    SignalManager m_signal_manager;

    /** timer for delayed screen refreshes (don't refresh too often) */
    QTimer m_refresh_timer;

    /** list of track pixmaps */
    QList<TrackPixmap> m_track_pixmaps;

    /** pixmap for avoiding flicker */
    QPixmap *m_pixmap;

//    LabelList *labels;           //linked list of markers
//    LabelType *markertype;       //selected marker type

    /** mode of the mouse cursor */
    MouseMode m_mouse_mode;

    /**
     * x position where the user last clicked the last time, needed fo
     * finding out where to start a drag&drop operation
     */
    int m_mouse_down_x;

};

#endif /* _SIGNAL_WIDGET_H_ */
