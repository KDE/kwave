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

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QList>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QPointer>
#include <QPolygon>
#include <QSize>
#include <QTimer>
#include <QWidget>

#include "kdemacros.h"

#include "libkwave/LabelList.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/SignalManager.h"

#include "libgui/MouseMark.h"

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
class QVBoxLayout;
class QResizeEvent;
class QWheelEvent;

class KUrl;

class LabelType;
namespace Kwave { class MouseMark; }
class SignalManager;
class TimeOperation;
class Track;
namespace Kwave { class TrackPixmap; }

/**
 * The SignalWidget class is responsible for displaying a signal with
 * multiple tracks. It provides control over selecton, zoom factor and
 * the signal data itself by containing a SignalManager.
 */
class KDE_EXPORT SignalWidget : public QWidget
{
    Q_OBJECT

    friend class InhibitRepaintGuard;

public:

    /** Constructor */
    SignalWidget(QWidget *parent);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    bool isOK();

    /** Destructor */
    virtual ~SignalWidget();

    /**
     * Converts a time in milliseconds to a number of samples, based
     * on the current signal rate.
     * @param ms time in milliseconds
     * @return number of samples (rounded)
     */
    sample_index_t ms2samples(double ms);

    /**
     * Converts a number of samples to a time in milliseconds, based on the
     * current signal rate.
     * @param samples number of samples
     * @return time in milliseconds
     */
    double samples2ms(sample_index_t samples);

    /**
     * Closes the current signal and loads a new one from a file.
     * @param url URL of the file to be loaded
     * @return 0 if succeeded or error code < 0
     */
    int loadFile(const KUrl &url);

    /**
     * Forwards information for creation of a new signal to the
     * signal manager.
     * @see TopWidget::newSignal
     */
    void newSignal(sample_index_t samples, double rate,
                   unsigned int bits, unsigned int tracks);

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

    /** Executes a Kwave text command */
    int executeCommand(const QString &command);

    /**
     * Returns the number of tracks of the current signal or
     * 0 if no signal is loaded.
     */
    int tracks();

    /** returns the signal manager of the current signal */
    SignalManager &signalManager();

    /** Returns the playback controller */
    PlaybackController &playbackController();

    /** returns the current display offset */
    inline unsigned int offset() const {
	return m_offset;
    }

    /**
     * Converts a sample index into a pixel offset using the current zoom
     * value. Always rounds up or downwards. If the number of pixels or the
     * current zoom is less than zero, the return value will be zero.
     * @param pixels pixel offset
     * @return index of the sample
     */
    sample_index_t pixels2samples(int pixels) const;

    /**
     * Converts a pixel offset into a sample index using the current zoom
     * value. Always rounds op or downwards.
     * @param samples number of samples to be converted
     * @return pixel offset
     */
    int samples2pixels(sample_index_t samples) const;

public slots:

    /**
     * Sets the display offset [samples] and refreshes the screen.
     * @param new_offset new value for the offset in samples, will be
     *                   internally limited to [0...length-1]
     */
    void setOffset(sample_index_t new_offset);

    /**
     * Sets a new selected range of samples. If the length of the
     * area is given as zero, nothing will be selected.
     * @param offset index of the first sample
     * @param length number of samples
     */
    void selectRange(unsigned int offset, unsigned int length);

    /** forward a sigCommand to the next layer */
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
    inline double zoom() { return m_zoom; }

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

    friend class UndoModifyLabelAction;

protected slots:

    /** Refreshes the layer with the markers */
    void refreshMarkersLayer();


    /**
     * Allows repainting of the display by decrementing the repaint
     * inhibit counter. If the counter reaches zero, the widget
     * will be refreshed.
     * @see m_inhibit_repaint
     * @see inhibitRepaint()
     */
    void allowRepaint(bool repaint);

    /** Handler for context menus */
    void contextMenuEvent(QContextMenuEvent *e);

private slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param index numeric index of the inserted track
     * @param track reference to the inserted track
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track *track);

    /**
     * Connected to the signal's sigTrackDeleted.
     * @param index numeric index of the deleted track
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
    void slotSamplesInserted(unsigned int track, sample_index_t offset,
                             sample_index_t length);

    /**
     * Connected to the signal's sigSamplesDeleted.
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was removed
     * @param length number of samples deleted
     * @see Signal::sigSamplesDeleted
     * @internal
     */
    void slotSamplesDeleted(unsigned int track, sample_index_t offset,
                            sample_index_t length);

    /**
     * Connected to the signal's sigSamplesModified
     * @param track index of the source track [0...tracks-1]
     * @param offset position from which the data was modified
     * @param length number of samples modified
     * @see Signal::sigSamplesModified
     * @internal
     */
    void slotSamplesModified(unsigned int track, sample_index_t offset,
                             sample_index_t length);

    /**
     * Connected to the changed() signal of the SignalManager's selection.
     * @see Selection
     * @internal
     */
    void slotSelectionChanged(sample_index_t offset, sample_index_t length);


    /**
     * Updates the vertical line that represents the current playback
     * position during playback.
     * @param pos last played sample position [0...length-1]
     */
    void updatePlaybackPointer(sample_index_t pos);

    /**
     * Refreshes the signal layer. Shortcut to refreshLayer(LAYER_SIGNAL).
     * @see #refreshLayer()
     */
    void refreshSignalLayer();

    /**
     * connected to the m_repaint_timer, called when it has
     * elapsed and the signal has to be repainted
     */
    void timedRepaint();

    /** Hide the current position marker */
    void hidePosition() {
	showPosition(0, 0, 0, QPoint(-1,-1));
    }

    /** context menu: "edit/undo" */
    void contextMenuEditUndo()   { forwardCommand("undo()"); }

    /** context menu: "edit/redo" */
    void contextMenuEditRedo()   { forwardCommand("redo()"); }

    /** context menu: "edit/cut" */
    void contextMenuEditCut()    { forwardCommand("cut()"); }

    /** context menu: "edit/copy" */
    void contextMenuEditCopy()   { forwardCommand("copy()"); }

    /** context menu: "edit/paste" */
    void contextMenuEditPaste()  { forwardCommand("paste()"); }

    /** context menu: "save selection" */
    void contextMenuSaveSelection()  { forwardCommand("saveselect()"); }

    /** context menu: "expand to labels" */
    void contextMenuSelectionExpandToLabels()  {
	forwardCommand("expandtolabel()");
    }

    /** context menu: "select next labels" */
    void contextMenuSelectionNextLabels()  {
	forwardCommand("selectnextlabels()");
    }

    /** context menu: "select previous labels" */
    void contextMenuSelectionPrevLabels()  {
	forwardCommand("selectprevlabels()");
    }

    /** context menu: "label / new" */
    void contextMenuLabelNew();

    /** context menu: "label / delete" */
    void contextMenuLabelDelete();

    /** context menu: "label / properties..." */
    void contextMenuLabelProperties();

signals:

    /**
     * Signals a change in the current visible area.
     * @param offset index of the first visible sample [samples]
     * @param width the width of the widget in [pixels]
     * @param length size of the whole signal [samples]
     */
    void viewInfo(sample_index_t offset, unsigned int width,
                  sample_index_t length);

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

    /**
     * Will be emitted if the zoom factor has changed due to a zoom
     * command or resize.
     * @param zoom value [samples/pixel]
     */
    void sigZoomChanged(double zoom);

    /**
     * Emits a change in the mouse cursor. This can be used to change
     * the content of a status bar if the mouse moves over a selected
     * area or a marker.
     * @param mode one of the modes in Kwave::MouseMark::Mode
     */
    void sigMouseChanged(Kwave::MouseMark::Mode mode);

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

    /** The selection state of at least one track has changed */
    void sigTrackSelectionChanged();

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
	}

	/** Destructor, allows repaints */
	~InhibitRepaintGuard() {
	    m_widget.allowRepaint(m_repaint);
	}

	/** reference to our owner */
	SignalWidget &m_widget;

	/** true if repaint is needed after allow */
	bool m_repaint;
    };

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

	/** event filter */
	virtual bool event(QEvent *e);

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

    /**
     * Returns the zoom value that will be used to fit the whole signal
     * into the current window.
     * @return zoom value [samples/pixel]
     */
    double getFullZoom();

    /** slot for detecting resizing of the widget */
    virtual void resizeEvent(QResizeEvent *);

    /** slot for mouse press, used for selection and drag&drop */
    virtual void mousePressEvent(QMouseEvent *);

    /** slot for mouse release, used for selection and drag&drop */
    virtual void mouseReleaseEvent(QMouseEvent *);

    /** slot for mouse moves, used for selection and drag&drop */
    virtual void mouseMoveEvent(QMouseEvent *);

    /** slot for mouse wheel events, used for vertical zoom */
    virtual void wheelEvent(QWheelEvent *event);

    /** slot for repainting the widget or portions of it */
    void paintEvent(QPaintEvent *);

    /**
     * Returns the label that is nearest to the given mouse position
     * and is visible or null if none found.
     * @param x mouse position, X coordinate, relative to the widget
     * @return nearest label or null if none found.
     */
    Label findLabelNearMouse(int x) const;

    /**
     * Opens a dialog for editing the properties of a label
     * @param label a Label that should be edited
     * @return true if the dialog has been accepted,
     *         otherwise false (canceled)
     */
    bool labelProperties(Label &label);

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
     * Fixes the zoom and the offset of the display so that no non-existing
     * samples (index < 0 or index >= length) have to be displayed and the
     * current display window of the signal fits into the screen.
     */
    void fixZoomAndOffset();

    /**
     * Sets the mode of the mouse cursor and emits sigMouseChanged
     * if it differs from the previous value.
     */
    void setMouseMode(Kwave::MouseMark::Mode mode);

    /**
     * Shows the current cursor position as a tooltip
     * @param text description of the position
     * @param pos marker position [samples]
     * @param ms marker position in milliseconds
     * @param mouse the coordinates of the mouse cursor,
     *              relative to this widget [pixel]
     */
    void showPosition(const QString &text, sample_index_t pos, double ms,
                      const QPoint &mouse);

    /** Shortcut for accessing the label list @note can be modified */
    inline LabelList &labels() {
	return m_signal_manager.fileInfo().labels();
    }

    /** Shortcut for accessing the label list @note cannot be modified */
    inline const LabelList &labels() const {
	return m_signal_manager.fileInfo().labels();
    }

    /**
     * add a new label
     * @param pos position of the label [samples]
     */
    void addLabel(sample_index_t pos);

private:

    /** QImage used for composition */
    QImage m_image;

    /** QImage for buffering each layer */
    QImage m_layer[3];

    /** flags for updating each layer */
    bool m_update_layer[3];

    /**
     * Offset from which signal is beeing displayed. This is equal to
     * the index of the first visible sample.
     */
    unsigned int m_offset;

    /** width of the widget in pixels, cached value */
    int m_width;

    /** height of the widget in pixels, cached value */
    int m_height;

    /** last/previous width of the widget, for detecting size changes */
    int m_last_width;

    /** last/previous height of the widget, for detecting size changes */
    int m_last_height;

    /** number of samples per pixel */
    double m_zoom;

    /** vertical zoom factor */
    double m_vertical_zoom;

    /**
     * position of the vertical line that indicates the current
     * playback position in [pixels] from 0...m_width-1. If no playback
     * is running the value is negative.
     */
    int m_playpointer;

    /** last/previous value of m_playpointer, for detecting changes */
    int m_last_playpointer;

    /** if set, m_pixmap has to be redrawn */
    bool m_redraw;

    /**
     * Counter for inhibiting repaints. If not zero, repaints should
     * be inhibited.
     * @see allowRepaint()
     * @see inhibitRepaint()
     */
    unsigned int m_inhibit_repaint;

    Kwave::MouseMark *m_selection;

    /** our signal manager */
    SignalManager m_signal_manager;

    /** list of track pixmaps */
    QList<Kwave::TrackPixmap *> m_track_pixmaps;

    /** mode of the mouse cursor */
    Kwave::MouseMark::Mode m_mouse_mode;

    /**
     * x position where the user last clicked the last time, needed fo
     * finding out where to start a drag&drop operation
     */
    int m_mouse_down_x;

    /** timer for limiting the number of repaints per second */
    QTimer m_repaint_timer;

    /** small widget for showing the mouse cursor position */
    PositionWidget m_position_widget;

    /** timer for automatic hiding */
    QTimer m_position_widget_timer;

};

#endif /* _SIGNAL_WIDGET_H_ */
