/***************************************************************************
             SignalWidget.cpp  -  Widget for displaying the signal
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

#include "config.h"
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

#include <QBitmap>
#include <QContextMenuEvent>
#include <QDragLeaveEvent>
#include <QEvent>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTime>
#include <QToolTip>

#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/FileInfo.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Parser.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/LabelPropertiesWidget.h"
#include "libgui/MenuManager.h"
#include "libgui/SignalWidget.h"
#include "libgui/TrackPixmap.h"

#include "MouseMark.h"

#ifdef DEBUG
#include <time.h>
#include <sys/time.h>
#endif

#define LAYER_SIGNAL    0
#define LAYER_SELECTION 1
#define LAYER_MARKERS   2

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {

/**
 * Limits the zoom to a minimum number of samples visible in one
 * screen.
 */
#define MINIMUM_SAMPLES_PER_SCREEN 5

/**
 * Default widht of the display in seconds when in streaming mode,
 * where no initial length information is available
 */
#define DEFAULT_DISPLAY_TIME 60.0

/** number of milliseconds between repaints */
#define REPAINT_INTERVAL 50

/** number of milliseconds until the position widget disappears */
#define POSITION_WIDGET_TIME 5000

/**
 * tolerance in pixel for snapping to a label or selection border
 * @todo selection tolerance should depend on some KDE setting,
 *        but which ?
 */
#define SELECTION_TOLERANCE 10

/** vertical zoom factor: minimum value */
#define VERTICAL_ZOOM_MIN 1.0

/** vertical zoom factor: maximum value */
#define VERTICAL_ZOOM_MAX 100.0

/** vertical zoom factor: increment/decrement factor */
#define VERTICAL_ZOOM_STEP_FACTOR 1.5

//***************************************************************************
//***************************************************************************
namespace KwaveFileDrag
{
    static bool canDecode(const QMimeData *source) {
	if (!source) return false;

	if (source->hasUrls()) {
	    // dropping URLs
	    foreach (QUrl url, source->urls()) {
		QString filename = url.toLocalFile();
		QString mimetype = CodecManager::whatContains(filename);
		if (CodecManager::canDecode(mimetype)) {
		    return true;
		}
	    }
	}

	foreach (QString format, source->formats()) {
	    // dropping known mime type
	    if (CodecManager::canDecode(format)) {
		qDebug("KwaveFileDrag::canDecode(%s)", format.toLocal8Bit().data());
		return true;
	    }
	}
	return false;
    }
}

//***************************************************************************
//***************************************************************************
SignalWidget::SignalWidget(QWidget *parent)
    :QWidget(parent), m_image(),
    m_offset(0), m_width(0), m_height(0), m_last_width(0), m_last_height(0),
    m_zoom(0.0), m_vertical_zoom(1.0), m_playpointer(-1), m_last_playpointer(-1),
    m_redraw(false), m_inhibit_repaint(0), m_selection(0), m_signal_manager(this),
    m_track_pixmaps(), m_mouse_mode(MouseNormal), m_mouse_down_x(0),
    m_repaint_timer(this), m_position_widget(this),
    m_position_widget_timer(this)
{
//    qDebug("SignalWidget::SignalWidget()");

    for (int i=0; i < 3; i++) {
	m_layer[i] = QImage();
	m_update_layer[i] = true;
    }

    m_selection = new MouseMark();
    Q_ASSERT(m_selection);
    if (!m_selection) return;

    // connect to the signal manager's signals
    SignalManager *sig = &m_signal_manager;

    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track *)),
            this, SLOT(slotTrackInserted(unsigned int, Track *)));
    connect(sig, SIGNAL(sigTrackDeleted(unsigned int)),
            this, SLOT(slotTrackDeleted(unsigned int)));

    connect(sig, SIGNAL(sigSamplesDeleted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesDeleted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesInserted(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesInserted(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigSamplesModified(unsigned int, unsigned int,
	unsigned int)),
	this, SLOT(slotSamplesModified(unsigned int, unsigned int,
	unsigned int)));
    connect(sig, SIGNAL(sigLabelCountChanged()),
            this, SLOT(hidePosition()),
            Qt::QueuedConnection);
    connect(sig, SIGNAL(sigLabelCountChanged()),
            this, SLOT(refreshMarkersLayer()),
            Qt::QueuedConnection);
    connect(sig, SIGNAL(labelsChanged(LabelList)),
            this, SLOT(refreshMarkersLayer()),
            Qt::QueuedConnection);

    connect(&(sig->selection()), SIGNAL(changed(unsigned int, unsigned int)),
	this, SLOT(slotSelectionChanged(unsigned int, unsigned int)));

    // connect to the playback controller
    connect(&(playbackController()), SIGNAL(sigPlaybackPos(unsigned int)),
            this, SLOT(updatePlaybackPointer(unsigned int)));

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(timedRepaint()));

    // connect the timer of the position widget
    connect(&m_position_widget_timer, SIGNAL(timeout()),
            &m_position_widget, SLOT(hide()));

//    m_menu_manager.clearNumberedMenu("ID_LABELS_TYPE");
//    for (LabelType *tmp = globals.markertypes.first(); tmp;
//         tmp = globals.markertypes.next())
//    {
//	m_menu_manager.addNumberedMenuEntry("ID_LABELS_TYPE", (char *)tmp->name);
//    }
//
//    markertype = globals.markertypes.first();

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setMouseTracking(true);
    setAcceptDrops(true); // enable drag&drop

    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setZoom(0.0);
//    qDebug("SignalWidget::SignalWidget(): done.");
}

//***************************************************************************
void SignalWidget::refreshSignalLayer()
{
    refreshLayer(LAYER_SIGNAL);
}

//***************************************************************************
void SignalWidget::refreshMarkersLayer()
{
    refreshLayer(LAYER_MARKERS);
}

//***************************************************************************
bool SignalWidget::isOK()
{
    Q_ASSERT(m_selection);
    return ( m_selection );
}

//***************************************************************************
SignalWidget::~SignalWidget()
{
    inhibitRepaint();

    close();

    labels().clear();

    if (m_selection) delete m_selection;
    m_selection = 0;

    for (int i=0; i < 3; i++)
	m_layer[i] = QImage();

}

//***************************************************************************
void SignalWidget::toggleTrackSelection(int track)
{
    // here we have to convert to unsigned
    Q_ASSERT(track >= 0);
    if (track < 0) return;
    unsigned int t = static_cast<unsigned int>(track);
    bool select = !m_signal_manager.trackSelected(t);
    m_signal_manager.selectTrack(t, select);
}

//***************************************************************************
bool SignalWidget::executeNavigationCommand(const QString &command)
{
    if (!command.length()) return true;
    Parser parser(command);
    const unsigned int visible_samples = pixels2samples(m_width);

    if (false) {
    // zoom
    CASE_COMMAND("zoomin")
	zoomIn();
    CASE_COMMAND("zoomout")
	zoomOut();
    CASE_COMMAND("zoomselection")
	zoomSelection();
    CASE_COMMAND("zoomall")
	zoomAll();
    CASE_COMMAND("zoomnormal")
	zoomNormal();
    // navigation
    CASE_COMMAND("goto")
	unsigned int offset = parser.toUInt();
	setOffset((offset > (visible_samples / 2)) ?
	          (offset - (visible_samples / 2)) : 0);
	selectRange(offset, 0);
    CASE_COMMAND("scrollright")
	setOffset(m_offset + visible_samples / 10);
    CASE_COMMAND("scrollleft")
	setOffset(((visible_samples / 10) < m_offset) ?
	          (m_offset - visible_samples / 10) : 0);
    CASE_COMMAND("viewstart")
	setOffset(0);
	selectRange(0,0);
    CASE_COMMAND("viewend")
	unsigned int len = m_signal_manager.length();
	if (len >= visible_samples) setOffset(len - visible_samples);
    CASE_COMMAND("viewnext")
	setOffset(m_offset + visible_samples);
    CASE_COMMAND("viewprev")
	setOffset((visible_samples < m_offset) ?
	          (m_offset - visible_samples) : 0);
    // selection
    CASE_COMMAND("selectall")
	selectRange(0, m_signal_manager.length());
    CASE_COMMAND("selectnext")
	if (m_signal_manager.selection().length())
	    selectRange(m_signal_manager.selection().last()+1,
	                m_signal_manager.selection().length());
	else
	    selectRange(m_signal_manager.length() - 1, 0);
    CASE_COMMAND("selectprev")
	unsigned int ofs = m_signal_manager.selection().first();
	unsigned int len = m_signal_manager.selection().length();
	if (!len) len = 1;
	if (len > ofs) len = ofs;
	selectRange(ofs-len, len);
    CASE_COMMAND("selecttoleft")
	selectRange(0, m_signal_manager.selection().last()+1);
    CASE_COMMAND("selecttoright")
	selectRange(m_signal_manager.selection().first(),
	    m_signal_manager.length()-m_signal_manager.selection().first()
	);
    CASE_COMMAND("selectvisible")
	selectRange(m_offset, pixels2samples(m_width) - 1);
    CASE_COMMAND("selectnone")
	selectRange(m_offset, 0);
    } else return false;

    return true;
}

//***************************************************************************
bool SignalWidget::executeCommand(const QString &command)
{
    InhibitRepaintGuard inhibit(*this);
    Parser parser(command);

    if (!command.length()) return true;

    if (executeNavigationCommand(command)) {
	return true;
    // label commands
    CASE_COMMAND("label")
	unsigned int pos = parser.toUInt();
	addLabel(pos);
    CASE_COMMAND("deletelabel")
	int index = parser.toInt();
	m_signal_manager.deleteLabel(index, true);
//    CASE_COMMAND("chooselabel")
//	Parser parser(command);
//	markertype = globals.markertypes.at(parser.toInt());
//    CASE_COMMAND("amptolabel")
//	markSignal(command);
//    CASE_COMMAND("pitch")
//	markPeriods(command);
//    CASE_COMMAND("labeltopitch")
//      convertMarkstoPitch(command);
//    CASE_COMMAND("loadlabel")
//	loadLabel();
//    CASE_COMMAND("savelabel")
//	saveLabel(command);
    CASE_COMMAND("expandtolabel")
	UndoTransactionGuard undo(m_signal_manager,
	    i18n("expand selection to label"));
	unsigned int selection_left  = m_signal_manager.selection().first();
	unsigned int selection_right = m_signal_manager.selection().last();
	if (labels().isEmpty()) return false; // we need labels for this
	Label label_left  = Label();
	Label label_right = Label();
	// the last label <= selection start -> label_left
	// the first label >= selection end  -> label_right
	foreach (Label label, labels()) {
	    unsigned int lp = label.pos();
	    if (lp <= selection_left)
		label_left = label;
	    if ((lp >= selection_right) && (label_right.isNull())) {
		label_right = label;
		break; // done
	    }
	}
	// default left label = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default right label = end of file
	selection_right = (label_right.isNull()) ?
	    m_signal_manager.length() - 1 : label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	selectRange(selection_left, length);

    CASE_COMMAND("selectnextlabels")
	UndoTransactionGuard undo(m_signal_manager,
	    i18n("select next labels"));
	unsigned int selection_left;
	unsigned int selection_right = m_signal_manager.selection().last();
	Label label_left  = Label();
	Label label_right = Label();
	if (labels().isEmpty()) return false; // we need labels for this

	// special case: nothing selected -> select up to the first label
	if (selection_right == 0) {
	    label_right = labels().first();
	    selection_left = 0;
	} else {
	    // find the first label starting after the current selection
	    LabelListIterator it(labels());
	    while (it.hasNext()) {
		Label label = it.next();
		if (label.pos() >= selection_right) {
		    // take it as selection start
		    label_left  = label;
		    // and it's next one as selection end (might be null)
		    label_right = it.hasNext() ? it.next() : Label();
		    break;
		}
	    }
	    // default selection start = last label
	    if (label_left.isNull()) label_left = labels().last();
	    if (label_left.isNull()) return false; // no labels at all !?
	    selection_left = label_left.pos();
	}
	// default selection end = end of the file
	selection_right = (label_right.isNull()) ?
	    m_signal_manager.length() - 1 : label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	selectRange(selection_left, length);

    CASE_COMMAND("selectprevlabels")
	UndoTransactionGuard undo(m_signal_manager,
	    i18n("select previous labels"));
	unsigned int selection_left  = m_signal_manager.selection().first();
	unsigned int selection_right = m_signal_manager.selection().last();
	Label label_left  = Label();
	Label label_right = Label();
	if (labels().isEmpty()) return false; // we need labels for this

	// find the last label before the start of the selection
	foreach (Label label, labels()) {
	    if (label.pos() > selection_left)
		break; // done
	    label_left  = label_right;
	    label_right = label;
	}
	// default selection start = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default selection end = first label
	if (label_right.isNull()) label_right = labels().first();
	if (label_right.isNull()) return false; // no labels at all !?
	selection_right = label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	selectRange(selection_left, length);

//    CASE_COMMAND("markperiod")
//	markPeriods(command);
//    CASE_COMMAND("saveperiods")
//	savePeriods();

    // track selection
    CASE_COMMAND("select_all_tracks")
	UndoTransactionGuard undo(m_signal_manager,
	    i18n("select all tracks"));
	foreach (unsigned int track, m_signal_manager.allTracks())
	    m_signal_manager.selectTrack(track, true);
    CASE_COMMAND("invert_track_selection")
	UndoTransactionGuard undo(m_signal_manager,
	    i18n("invert track selection"));
	foreach (unsigned int track, m_signal_manager.allTracks())
	    m_signal_manager.selectTrack(
		track,
		!m_signal_manager.trackSelected(track)
	    );
    } else {
	return m_signal_manager.executeCommand(command);
    }

    return true;
}

//***************************************************************************
void SignalWidget::selectRange(unsigned int offset, unsigned int length)
{
    m_signal_manager.selectRange(offset, length);
}

//***************************************************************************
void SignalWidget::slotSelectionChanged(unsigned int offset,
                                        unsigned int length)
{
    m_signal_manager.selectRange(offset, length);
    offset = m_signal_manager.selection().offset();
    length = m_signal_manager.selection().length();

    refreshLayer(LAYER_SELECTION);
    emit selectedTimeInfo(offset, length, m_signal_manager.rate());
}

//***************************************************************************
void SignalWidget::forwardCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
int SignalWidget::tracks()
{
    return m_signal_manager.tracks();
}

//***************************************************************************
PlaybackController &SignalWidget::playbackController()
{
    return m_signal_manager.playbackController();
}

//***************************************************************************
SignalManager &SignalWidget::signalManager()
{
    return m_signal_manager;
}

//***************************************************************************
int SignalWidget::loadFile(const KUrl &url)
{
    // load a new signal
    int res = m_signal_manager.loadFile(url);
    if (m_signal_manager.isClosed() && res) {
	qWarning("SignalWidget::loadFile() failed:"\
		" zero-length or out of memory?");

	QString reason;
	switch (res) {
	    case -ENOMEM:
		reason = i18n("Out of memory");
		break;
	    case -EIO:
		reason = i18n("unable to open '%1'",
		    url.prettyUrl());
		break;
	    case -EINVAL:
		reason = i18n("invalid or unknown file type: '%1'",
		              url.prettyUrl());
		break;
	    default:
		reason = "";
	}

        // show an error message box if the reason was known
	if (reason.length()) {
	    Kwave::MessageBox::error(this, reason);
	}

	close();
    }

    // if the signal is smaller than expected,
    // zoom it to full size
    if (m_zoom > getFullZoom()) setZoom(getFullZoom());

    return res;
}

//***************************************************************************
void SignalWidget::newSignal(unsigned int samples, double rate,
                             unsigned int bits, unsigned int tracks)
{
    m_signal_manager.newSignal(samples,rate,bits,tracks);
}

//***************************************************************************
void SignalWidget::close()
{
    // first stop playback
    m_signal_manager.playbackController().playbackStop();
    m_signal_manager.playbackController().reset();

    // discard all labels
    labels().clear();

    // clear the display
    qDeleteAll(m_track_pixmaps);
    m_track_pixmaps.clear();

    setZoom(0.0);
    setOffset(0);

    // close the signal manager
    m_signal_manager.close();

    // reset the status, zoom and selection
    selectRange(0, 0);
    zoomAll();
    refreshAllLayers();

    setMouseMode(MouseNormal);
}

//***************************************************************************
void SignalWidget::setOffset(unsigned int new_offset)
{
    InhibitRepaintGuard inhibit(*this);

    m_offset = new_offset;
    fixZoomAndOffset();

    // forward the zoom and offset to all track pixmaps
    foreach (TrackPixmap *pix, m_track_pixmaps) {
	if (!pix) continue;
	pix->setOffset(m_offset);
	pix->setZoom(m_zoom);
    }

    refreshAllLayers();
    emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
                  m_signal_manager.length());
}

//***************************************************************************
double SignalWidget::getFullZoom()
{
    if (m_signal_manager.isEmpty()) return 0.0;    // no zoom if no signal

    unsigned int length = m_signal_manager.length();
    if (!length) {
        // no length: streaming mode -> start with a default
        // zoom, use one minute (just guessed)
        length = static_cast<unsigned int>(ceil(DEFAULT_DISPLAY_TIME *
	    m_signal_manager.rate()));
    }

    // example: width = 100 [pixels] and length = 3 [samples]
    //          -> samples should be at positions 0, 49.5 and 99
    //          -> 49.5 [pixels / sample]
    //          -> zoom = 1 / 49.5 [samples / pixel]
    // => full zoom [samples/pixel] = (length-1) / (width-1)
    return static_cast<double>(length - 1) /
	   static_cast<double>(QWidget::width() - 1);
}

//***************************************************************************
void SignalWidget::setZoom(double new_zoom)
{
    double old_zoom = m_zoom;
    InhibitRepaintGuard inhibit(*this);

    m_zoom = new_zoom;
    fixZoomAndOffset();
    if (m_zoom == old_zoom) return; // nothing to do

    // forward the zoom and offset to all track pixmaps
    foreach (TrackPixmap *pix, m_track_pixmaps) {
	if (!pix) continue;
	pix->setOffset(m_offset);
	pix->setZoom(m_zoom);
    }

    refreshAllLayers();

    emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
                  m_signal_manager.length());

    emit sigZoomChanged(m_zoom);
}

//***************************************************************************
void SignalWidget::fixZoomAndOffset()
{
    double max_zoom;
    double min_zoom;
    unsigned int length;

    length = m_signal_manager.length();
    if (!length) {
	// in streaming mode we have to use a guessed length
	length = static_cast<unsigned int>(ceil(width() * getFullZoom()));
    }

    if (!m_width) return;

    // ensure that m_offset is [0...length-1]
    if (m_offset > length-1) m_offset = length-1;

    // ensure that the zoom is in a proper range
    max_zoom = getFullZoom();
    min_zoom = static_cast<double>(MINIMUM_SAMPLES_PER_SCREEN) /
	       static_cast<double>(m_width);
    if (m_zoom < min_zoom) m_zoom = min_zoom;
    if (m_zoom > max_zoom) m_zoom = max_zoom;

    // try to correct the offset if there is not enough data to fill
    // the current window
    // example: width=100 [pixels], length=3 [samples],
    //          offset=1 [sample], zoom=1/49.5 [samples/pixel] (full)
    //          -> current last displayed sample = length-offset
    //             = 3 - 1 = 2
    //          -> available space = pixels2samples(width-1) + 1
    //             = (99/49.5) + 1 = 3
    //          -> decrease offset by 3 - 2 = 1
    Q_ASSERT(length >= m_offset);
    if (pixels2samples(m_width - 1) + 1 > length-m_offset) {
	// there is space after the signal -> move offset right
	unsigned int shift = pixels2samples(m_width - 1) + 1 -
	                     (length - m_offset);
	if (shift >= m_offset) {
	    m_offset = 0;
	} else {
	    m_offset -= shift;
	}
    }

// not really needed, has side-effects e.g. when a .ogg file
// has been loaded
//    // if reducing the offset was not enough, zoom in
//    if (pixels2samples(m_width - 1) + 1 > length - m_offset) {
//	// there is still space after the signal -> zoom in
//	// (this should never happen as the zoom has been limited before)
//	m_zoom = max_zoom;
//    }

// this made some strange effects, so I disabled it :-[
//    // adjust the zoom factor in order to make a whole number
//    // of samples fit into the current window
//    int samples = pixels2samples(width) + 1;
//    zoom = (double)(samples) / (double)(width - 1);

    // do some final range checking
    if (m_zoom < min_zoom) m_zoom = min_zoom;
    if (m_zoom > max_zoom) m_zoom = max_zoom;

}

//***************************************************************************
void SignalWidget::setMouseMode(MouseMode mode)
{
    if (mode == m_mouse_mode) return;

    m_mouse_mode = mode;
    switch (mode) {
	case MouseNormal:
	    setCursor(Qt::ArrowCursor);
	    break;
	case MouseAtSelectionBorder:
	    setCursor(Qt::SizeHorCursor);
	    break;
	case MouseInSelection:
	    setCursor(Qt::ArrowCursor);
	    break;
	case MouseSelect:
	    setCursor(Qt::SizeHorCursor);
	    break;
    }

    emit sigMouseChanged(static_cast<int>(mode));
}

//***************************************************************************
void SignalWidget::zoomAll()
{
    InhibitRepaintGuard inhibit(*this);

    setZoom(getFullZoom());
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomNormal()
{
    InhibitRepaintGuard inhibit(*this);

    m_offset += pixels2samples(m_width) / 2;
    setZoom(1.0);
    unsigned int shift = pixels2samples(m_width) / 2;
    setOffset((shift < m_offset) ? (m_offset-shift) : 0);
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomOut()
{
    InhibitRepaintGuard inhibit(*this);

    m_offset += pixels2samples(m_width) / 2;
    setZoom(m_zoom*3);
    unsigned int shift = pixels2samples(m_width) / 2;
    setOffset((shift < m_offset) ? (m_offset-shift) : 0);
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomIn()
{
    InhibitRepaintGuard inhibit(*this);

    m_offset += pixels2samples(m_width) / 2;
    setZoom(m_zoom / 3);
    unsigned int shift = pixels2samples(m_width) / 2;
    setOffset((shift < m_offset) ? (m_offset-shift) : 0);

    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomSelection()
{
    InhibitRepaintGuard inhibit(*this);

    unsigned int ofs = m_signal_manager.selection().offset();
    unsigned int len = m_signal_manager.selection().length();

    if (len) {
	m_offset = ofs;
	setZoom((static_cast<double>(len)) / static_cast<double>(m_width - 1));
    }
}

//***************************************************************************
void SignalWidget::resizeEvent(QResizeEvent *)
{
    emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
                  m_signal_manager.length());
}

//***************************************************************************
void SignalWidget::inhibitRepaint()
{
    m_inhibit_repaint++;
}

//***************************************************************************
void SignalWidget::allowRepaint(bool repaint)
{
    Q_ASSERT(m_inhibit_repaint);
    if (!m_inhibit_repaint) return;

    // decrease the number of repaint locks
    m_inhibit_repaint--;

    // if the number reached zero, *do* the repaint if allowed
    if (!m_inhibit_repaint && repaint) {
	if (m_repaint_timer.isActive()) {
	    // repainting is inhibited -> wait until the
	    // repaint timer is elapsed
	    return;
	} else {
	    // start the repaint timer
	    m_repaint_timer.setSingleShot(true);
	    m_repaint_timer.start(REPAINT_INTERVAL);
	}
    }
}

//***************************************************************************
void SignalWidget::timedRepaint()
{
    this->repaint();
}

//***************************************************************************
void SignalWidget::refreshAllLayers()
{
    InhibitRepaintGuard inhibit(*this);

    for (int i=0; i < 3; i++) {
	m_update_layer[i] = true;
    }

    m_redraw = true;
}

//***************************************************************************
void SignalWidget::refreshLayer(int layer)
{
    InhibitRepaintGuard inhibit(*this);

    Q_ASSERT(layer >= 0);
    Q_ASSERT(layer < 3);
    if ((layer < 0) || (layer >= 3)) return;

    m_update_layer[layer] = true;

    m_redraw = true;
}

//***************************************************************************
int SignalWidget::selectionPosition(const int x)
{
    const int tol = SELECTION_TOLERANCE;
    const unsigned int first = m_signal_manager.selection().first();
    const unsigned int last  = m_signal_manager.selection().last();
    const unsigned int ofs   = m_offset;
    Q_ASSERT(first <= last);

    // get pixel coordinates
    const int l = (first < ofs) ? -(2*tol) : (samples2pixels(first-ofs));
    const int r = (last  < ofs) ? -(2*tol) : (samples2pixels(last-ofs));
    const int ll = (l > tol) ? (l-tol) : 0;
    const int lr = l + tol;
    const int rl = (r > tol) ? (r-tol) : 0;
    const int rr = r + tol;

    // the simple cases...
    int pos = None;
    if ((x >= ll) && (x <= lr) && (x <= r)) pos |= LeftBorder;
    if ((x >= rl) && (x <= rr) && (x >= l)) pos |= RightBorder;
    if ((x >   l) && (x <   r)) pos |= Selection;

    if ((pos & LeftBorder) && (pos & RightBorder)) {
	// special case: determine which border is nearer
	if ((x - l) < (r - x))
	    pos &= ~RightBorder; // more on the left
	else
	    pos &= ~LeftBorder;  // more on the right
    }

    return pos;
}

//***************************************************************************
bool SignalWidget::isSelectionBorder(int x)
{
    SelectionPos pos = static_cast<SignalWidget::SelectionPos>(
	selectionPosition(x) & ~Selection);

    return ((pos & LeftBorder) || (pos & RightBorder));
}

//***************************************************************************
bool SignalWidget::isInSelection(int x)
{
    return (selectionPosition(x) & Selection) != 0;
}

//***************************************************************************
void SignalWidget::mousePressEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_selection);
    if (!e) return;
    if (!m_selection) return;

    // abort if no signal is loaded
    if (!m_signal_manager.length()) return;

    // ignore all mouse press events in playback mode
    if (m_signal_manager.playbackController().running()) return;

    if (e->button() == Qt::LeftButton) {
	int mx = e->pos().x();
	if (mx < 0) mx = 0;
	if (mx >= m_width) mx = m_width-1;
	unsigned int x = m_offset + pixels2samples(mx);
	unsigned int len = m_signal_manager.selection().length();
	switch (e->modifiers()) {
	    case Qt::ShiftModifier: {
		// expand the selection to "here"
		m_selection->set(m_signal_manager.selection().first(),
		                 m_signal_manager.selection().last());
		m_selection->grep(x);
		selectRange(m_selection->left(), m_selection->length());
		setMouseMode(MouseSelect);
		break;
	    }
	    case Qt::ControlModifier: {
		if (isInSelection(e->pos().x()) && (len > 1)) {
		    // start a drag&drop operation in "copy" mode
		    startDragging();
		}
		break;
	    }
	    case 0: {
		if (isSelectionBorder(e->pos().x())) {
		    // modify selection border
		    m_selection->set(m_signal_manager.selection().first(),
			             m_signal_manager.selection().last());
		    m_selection->grep(x);
		    selectRange(m_selection->left(), m_selection->length());
		    setMouseMode(MouseSelect);
		} else if (isInSelection(e->pos().x()) && (len > 1)) {
		    // store the x position for later drag&drop
		    m_mouse_down_x = e->pos().x();
		} else {
		    // start a new selection
		    m_selection->set(x, x);
		    selectRange(x, 0);
		    setMouseMode(MouseSelect);
		}
		break;
	    }
	}
    }
}

//***************************************************************************
void SignalWidget::contextMenuEvent(QContextMenuEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_selection);
    if (!e || !m_selection) return;

    SignalManager *manager = &signalManager();
    bool have_signal = !manager->isEmpty();
    if (!have_signal)return;
    bool have_selection = (manager->selection().length() > 1);
    bool have_labels = !labels().isEmpty();

    QMenu *context_menu = new QMenu(this);
    Q_ASSERT(context_menu);
    if (!context_menu) return;

    QMenu *submenu_select = context_menu->addMenu(i18n("&Selection"));
    Q_ASSERT(submenu_select);
    if (!submenu_select) return;

    QMenu *submenu_label = context_menu->addMenu(i18n("&Label"));
    Q_ASSERT(submenu_label);
    if (!submenu_label) return;

    KIconLoader icon_loader;

    /* menu items common to all cases */

    // undo
    QAction *action;
    action = context_menu->addAction(
	icon_loader.loadIcon("edit-undo", KIconLoader::Toolbar),
	i18n("&Undo"), this, SLOT(contextMenuEditUndo()),
	Qt::CTRL + Qt::Key_Z);
    Q_ASSERT(action);
    if (!action) return;
    if (!manager->canUndo())
	action->setEnabled(false);

    // redo
    action = context_menu->addAction(
	icon_loader.loadIcon("edit-redo", KIconLoader::Toolbar),
	i18n("&Redo"), this, SLOT(contextMenuEditRedo()),
	Qt::CTRL + Qt::Key_Y);
    Q_ASSERT(action);
    if (!action) return;
    if (!manager->canRedo())
	action->setEnabled(false);
    context_menu->addSeparator();

    // cut/copy/paste
    QAction *action_cut = context_menu->addAction(
	icon_loader.loadIcon("edit-cut", KIconLoader::Toolbar),
	i18n("Cu&t"), this, SLOT(contextMenuEditCut()),
	Qt::CTRL + Qt::Key_X);
    QAction *action_copy = context_menu->addAction(
	icon_loader.loadIcon("edit-copy", KIconLoader::Toolbar),
	i18n("&Copy"), this, SLOT(contextMenuEditCopy()),
	Qt::CTRL + Qt::Key_C);
    QAction *action_paste = context_menu->addAction(
	icon_loader.loadIcon("edit-paste", KIconLoader::Toolbar),
	i18n("&Paste"), this, SLOT(contextMenuEditPaste()),
	Qt::CTRL + Qt::Key_V);
    context_menu->addSeparator();
    if (action_cut)   action_cut->setEnabled(have_selection);
    if (action_copy)  action_copy->setEnabled(have_selection);
    if (action_paste)
	action_paste->setEnabled(!ClipBoard::instance().isEmpty());

    int mouse_x = mapFromGlobal(e->globalPos()).x();
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= width())  mouse_x = width()  - 1;

    // Selection / &Save
    QAction *action_select_save = submenu_select->addAction(
	icon_loader.loadIcon("document-save", KIconLoader::Toolbar),
	i18n("&Save..."), this, SLOT(contextMenuSaveSelection()));
    Q_ASSERT(action_select_save);
    if (!action_select_save) return;
    action_select_save->setEnabled(have_selection);

    // Selection / &Expand to labels
    QAction *action_select_expand_to_labels = submenu_select->addAction(
	i18n("&Expand to labels"), this,
	SLOT(contextMenuSelectionExpandToLabels()), Qt::Key_E);
    Q_ASSERT(action_select_expand_to_labels);
    if (!action_select_expand_to_labels) return;
    action_select_expand_to_labels->setEnabled(have_labels);

    // Selection / to next labels
    QAction *action_select_next_labels = submenu_select->addAction(
	i18n("to next labels"), this,
	SLOT(contextMenuSelectionNextLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_N);
    Q_ASSERT(action_select_next_labels);
    if (!action_select_next_labels) return;
    action_select_next_labels->setEnabled(have_labels);

    // Selection / to previous labels
    QAction *action_select_prev_labels = submenu_select->addAction(
	i18n("to previous labels"), this,
	SLOT(contextMenuSelectionPrevLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_P);
    Q_ASSERT(action_select_prev_labels);
    if (!action_select_prev_labels) return;
    action_select_prev_labels->setEnabled(have_labels);

    // label handling
    QAction *action_label_new = submenu_label->addAction(
	icon_loader.loadIcon("list-add", KIconLoader::Toolbar),
	i18n("&New"), this, SLOT(contextMenuLabelNew()));
    Q_ASSERT(action_label_new);
    if (!action_label_new) return;
    action_label_new->setEnabled(have_signal);

    QAction *action_label_delete = submenu_label->addAction(
	icon_loader.loadIcon("list-remove", KIconLoader::Toolbar),
	i18n("&Delete"), this, SLOT(contextMenuLabelDelete()));
    Q_ASSERT(action_label_delete);
    if (!action_label_delete) return;
    action_label_delete->setEnabled(false);

    QAction *action_label_properties = submenu_label->addAction(
	icon_loader.loadIcon("configure", KIconLoader::Toolbar),
	i18n("&Properties..."), this, SLOT(contextMenuLabelProperties()));
    Q_ASSERT(action_label_properties);
    if (!action_label_properties) return;
    action_label_properties->setEnabled(false);

    // store the menu position in the mouse selection
    unsigned int pos = m_offset + pixels2samples(mouse_x);
    m_selection->set(pos, pos);

    Label label;
    if (!((label = findLabelNearMouse(mouse_x)).isNull())) {
	// delete label ?
	// label properties ?
	action_label_new->setEnabled(false);
	action_label_properties->setEnabled(true);
	action_label_delete->setEnabled(true);

	pos = label.pos();
	m_selection->set(pos, pos);
    }

    if (isSelectionBorder(mouse_x)) {
	// context menu: do something with the selection border

	// expand to next marker (right) ?
	// expand to next marker (left) ?
    }

//     if (isInSelection(mouse_x) && have_selection) {
// 	// context menu: do something with the selection
//     }

    context_menu->exec(QCursor::pos());
    delete context_menu;
}

//***************************************************************************
void SignalWidget::contextMenuLabelNew()
{
    Q_ASSERT(m_selection);
    if (!m_selection) return;

    forwardCommand(QString("label(%1)").arg(m_selection->left()));
}

//***************************************************************************
void SignalWidget::contextMenuLabelDelete()
{
    Q_ASSERT(m_selection);
    if (!m_selection) return;

    Label label = m_signal_manager.findLabel(m_selection->left());
    if (label.isNull()) return;
    int index = m_signal_manager.labelIndex(label);
    forwardCommand(QString("deletelabel(%1)").arg(index));
}

//***************************************************************************
void SignalWidget::contextMenuLabelProperties()
{
    Q_ASSERT(m_selection);
    if (!m_selection) return;

    Label label = m_signal_manager.findLabel(m_selection->left());
    if (label.isNull()) return;

    labelProperties(label);
}

//***************************************************************************
void SignalWidget::mouseReleaseEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_selection);
    if (!e) return;
    if (!m_selection) return;

    // abort if no signal is loaded
    if (!m_signal_manager.length()) return;

    // ignore all mouse release events in playback mode
    if (m_signal_manager.playbackController().running()) return;

    switch (m_mouse_mode) {
	case MouseSelect: {
	    unsigned int x = m_offset + pixels2samples(e->pos().x());
	    m_selection->update(x);
	    selectRange(m_selection->left(), m_selection->length());
	    setMouseMode(MouseNormal);
	    hidePosition();
	    break;
	}
	case MouseInSelection: {
	    int dmin = KGlobalSettings::dndEventDelay();
	    if ((e->button() & Qt::LeftButton) &&
		    ((e->pos().x() >= m_mouse_down_x - dmin) ||
		     (e->pos().x() <= m_mouse_down_x + dmin)) )
	    {
		// deselect if only clicked without moving
		unsigned int pos = m_offset + pixels2samples(e->pos().x());
		selectRange(pos, 0);
		setMouseMode(MouseNormal);
		hidePosition();
	    }
	    break;
	}
	default: ;
    }

}

//***************************************************************************
SignalWidget::PositionWidget::PositionWidget(QWidget *parent)
    :QWidget(parent), m_label(0), m_alignment(0),
     m_radius(10), m_arrow_length(30), m_last_alignment(Qt::AlignHCenter),
     m_last_size(QSize(0,0)), m_polygon()
{
    hide();

    m_label = new QLabel(this);
    Q_ASSERT(m_label);
    if (!m_label) return;

    m_label->setFrameStyle(QFrame::Panel | QFrame::Plain);
    m_label->setPalette(QToolTip::palette()); // use same colors as a QToolTip
    m_label->setFocusPolicy(Qt::NoFocus);
    m_label->setMouseTracking(true);
    m_label->setLineWidth(0);

    setPalette(QToolTip::palette()); // use same colors as a QToolTip
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
}

//***************************************************************************
SignalWidget::PositionWidget::~PositionWidget()
{
    if (m_label) delete m_label;
    m_label = 0;
}

//***************************************************************************
void SignalWidget::PositionWidget::setText(const QString &text,
                                           Qt::Alignment alignment)
{
    if (!m_label) return;

    m_alignment = alignment;

    m_label->setText(text);
    m_label->setAlignment(m_alignment);
    m_label->resize(m_label->sizeHint());

    switch (m_alignment) {
	case Qt::AlignLeft:
	    resize(m_arrow_length + m_radius + m_label->width() + m_radius,
	           m_radius + m_label->height() + m_radius);
	    m_label->move(m_arrow_length + m_radius, m_radius);
	    break;
	case Qt::AlignRight:
	    resize(m_radius + m_label->width() + m_radius + m_arrow_length,
	           m_radius + m_label->height() + m_radius);
	    m_label->move(m_radius, m_radius);
	    break;
	case Qt::AlignHCenter:
	    resize(m_radius + m_label->width() + m_radius,
	           m_arrow_length + m_radius + m_label->height() + m_radius);
	    m_label->move(m_radius, m_arrow_length + m_radius);
	    break;
	default:
	    ;
    }

    updateMask();
}

//***************************************************************************
bool SignalWidget::PositionWidget::event(QEvent *e)
{
    if (!e) return false;

    // ignore any kind of event that might be of interest
    // for the parent widget (in our case the SignalWidget)
    switch (e->type()) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	case QEvent::MouseMove:
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	case QEvent::Shortcut:
	case QEvent::Wheel:
	case QEvent::Clipboard:
	case QEvent::Speech:
	case QEvent::DragEnter:
	case QEvent::DragMove:
	case QEvent::DragLeave:
	case QEvent::Drop:
	case QEvent::DragResponse:
	case QEvent::TabletMove:
	case QEvent::TabletPress:
	case QEvent::TabletRelease:
	    return false;
	default:
	    ;
    }

    // everything else: let it be handled by QLabel
    return QWidget::event(e);
}

//***************************************************************************
void SignalWidget::PositionWidget::updateMask()
{
    // bail out if nothing has changed
    if ((size() == m_last_size) && (m_alignment == m_last_alignment))
	return;

    QPainter p;
    QBitmap bmp(size());
    bmp.fill(Qt::color0);
    p.begin(&bmp);

    QBrush brush(Qt::color1);
    p.setBrush(brush);
    p.setPen(Qt::color1);

    const int h = height();
    const int w = width();

    // re-create the polygon, depending on alignment
    switch (m_alignment) {
	case Qt::AlignLeft:
	    m_polygon.setPoints(8,
		m_arrow_length, 0,
		w-1, 0,
		w-1, h-1,
		m_arrow_length, h-1,
		m_arrow_length, 2*h/3,
		0, h/2,
		m_arrow_length, h/3,
		m_arrow_length, 0
	    );
	    break;
	case Qt::AlignRight:
	    m_polygon.setPoints(8,
		0, 0,
		w-1-m_arrow_length, 0,
		w-1-m_arrow_length, h/3,
		w-1, h/2,
		w-1-m_arrow_length, 2*h/3,
		w-1-m_arrow_length, h-1,
		0, h-1,
		0, 0
	    );
	    break;
	case Qt::AlignHCenter:
	    break;
	default:
	    ;
    }

    p.drawPolygon(m_polygon);
    p.end();

    // activate the new widget mask
    clearMask();
    setMask(bmp);

    // remember size/alignment for detecing changes
    m_last_alignment = m_alignment;
    m_last_size      = size();
}

//***************************************************************************
void SignalWidget::PositionWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(palette().background().color());
    p.drawPolygon(m_polygon);
}

//***************************************************************************
void SignalWidget::showPosition(const QString &text, unsigned int pos,
                                double ms, const QPoint &mouse)
{
    int x = mouse.x();
    int y = mouse.y();

    // x/y == -1/-1 -> reset/hide the position
    if ((x < 0) && (y < 0)) {
	m_position_widget_timer.stop();
	m_position_widget.hide();
	return;
    }

    setUpdatesEnabled(false);
    m_position_widget.hide();

    unsigned int t, h, m, s, tms;
    t = static_cast<unsigned int>(rint(ms * 10.0));
    tms = t % 10000;
    t /= 10000;
    s = t % 60;
    t /= 60;
    m = t % 60;
    t /= 60;
    h = t;
    QString hms;
    hms.sprintf("%02u:%02u:%02u.%04u", h, m, s, tms);
    QString txt = QString("%1\n%2\n%3").arg(text).arg(pos).arg(hms);

    switch (selectionPosition(mouse.x()) & ~Selection) {
	case LeftBorder:
	    m_position_widget.setText(txt, Qt::AlignRight);
	    x = samples2pixels(pos - m_offset) - m_position_widget.width();
	    if (x < 0) {
		// switch to left aligned mode
		m_position_widget.setText(txt, Qt::AlignLeft);
		x = samples2pixels(pos - m_offset);
	    }
	    break;
	case RightBorder:
	default:
	    m_position_widget.setText(txt, Qt::AlignLeft);
	    x = samples2pixels(pos - m_offset);
	    if (x + m_position_widget.width() > width()) {
		// switch to right aligned mode
		m_position_widget.setText(txt, Qt::AlignRight);
		x = samples2pixels(pos - m_offset) - m_position_widget.width();
	    }
	    break;
    }

    // adjust the position to avoid vertical clipping
    int lh = m_position_widget.height();
    if (y - lh/2 < 0) {
	y = 0;
    } else if (y + lh/2 > height()) {
	y = height() - lh;
    } else {
	y -= lh/2;
    }

    m_position_widget.move(x, y);

    if (!m_position_widget.isVisible())
	m_position_widget.show();

    m_position_widget_timer.stop();
    m_position_widget_timer.setSingleShot(true);
    m_position_widget_timer.start(POSITION_WIDGET_TIME);

    setUpdatesEnabled(true);
}

//***************************************************************************
void SignalWidget::mouseMoveEvent(QMouseEvent *e)
{
    Q_ASSERT(e);
    Q_ASSERT(m_selection);
    Q_ASSERT(m_width);
    if (!e) return;
    if (!m_selection) return;
    if (!m_width) return;

    // abort if no signal is loaded
    if (!m_signal_manager.length()) return;

    // there seems to be a BUG in Qt, sometimes e->pos() produces flicker/wrong
    // coordinates on the start of a fast move!?
    // globalPos() seems not to have this effect
    int mouse_x = mapFromGlobal(e->globalPos()).x();
    int mouse_y = mapFromGlobal(e->globalPos()).y();
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x >= width())  mouse_x = width()  - 1;
    if (mouse_y >= height()) mouse_y = height() - 1;
    QPoint pos(mouse_x, mouse_y);

    // bail out if the position did not change
    static int last_x = -1;
    static int last_y = -1;
    if ((mouse_x == last_x) && (mouse_y == last_y))
	return;
    last_x = mouse_x;
    last_y = mouse_y;

    switch (m_mouse_mode) {
	case MouseSelect: {
	    // in move mode, a new selection was created or an old one grabbed
	    // this does the changes with every mouse move...
	    unsigned int x = m_offset + pixels2samples(mouse_x);
	    m_selection->update(x);
	    selectRange(m_selection->left(), m_selection->length());
	    showPosition(i18n("selection"), x, samples2ms(x), pos);
	    break;
	}
	default: {
	    unsigned int first = m_signal_manager.selection().first();
	    unsigned int last  = m_signal_manager.selection().last();
	    Label label = findLabelNearMouse(mouse_x);

	    // find out what is nearer: label or selection border ?
	    if (!label.isNull() && (first != last) && isSelectionBorder(mouse_x)) {
		const unsigned int pos = m_offset + pixels2samples(mouse_x);
		const unsigned int d_label = (pos > label.pos()) ?
		    (pos - label.pos()) : (label.pos() - pos);
		const unsigned int d_left = (pos > first) ?
		    (pos - first) : (first - pos);
		const unsigned int d_right = (pos > last) ?
		    (pos - last) : (last - pos);
		if ( ((d_label ^ 2) > (d_left ^ 2)) &&
		     ((d_label ^ 2) > (d_right ^ 2)) )
		{
		    // selection borders are nearer
		    label = Label();
		}
	    }

	    // yes, this code gives the nifty cursor change....
	    if (!label.isNull()) {
		setMouseMode(MouseAtSelectionBorder);
		QString text = i18n("label #%1",
		    m_signal_manager.labelIndex(label));
		if (label.name().length())
		    text += i18n(" (%1)", label.name());
		showPosition(text, label.pos(), samples2ms(label.pos()), pos);
		break;
	    } else if ((first != last) && isSelectionBorder(mouse_x)) {
		setMouseMode(MouseAtSelectionBorder);
		switch (selectionPosition(mouse_x) & ~Selection) {
		    case LeftBorder:
			showPosition(i18n("selection, left border"),
			    first, samples2ms(first), pos);
			break;
		    case RightBorder:
			showPosition(i18n("selection, right border"),
			    last, samples2ms(last), pos);
			break;
		    default:
			hidePosition();
		}
	    } else if (isInSelection(mouse_x)) {
		setMouseMode(MouseInSelection);
		hidePosition();
                int dmin = KGlobalSettings::dndEventDelay();
		if ((e->buttons() & Qt::LeftButton) &&
		    ((mouse_x < m_mouse_down_x - dmin) ||
		     (mouse_x > m_mouse_down_x + dmin)) )
		{
		    startDragging();
		}
	    } else {
		setMouseMode(MouseNormal);
		hidePosition();
	    }
	}
    }
}

//***************************************************************************
void SignalWidget::wheelEvent(QWheelEvent *event)
{
    if (!event) return;

    // we currently are only interested in <Ctrl> + <WheelUp/Down>
    if (event->modifiers() != Qt::ControlModifier) {
	event->ignore();
	return;
    }

    if ((event->delta() > 0) && (m_vertical_zoom < VERTICAL_ZOOM_MAX)) {
	// zom in
	m_vertical_zoom *= VERTICAL_ZOOM_STEP_FACTOR;
	if (m_vertical_zoom > VERTICAL_ZOOM_MAX)
	    m_vertical_zoom = VERTICAL_ZOOM_MAX;
	event->accept();
	refreshSignalLayer();
    } else if ((event->delta() < 0) && (m_vertical_zoom > VERTICAL_ZOOM_MIN)) {
	// zoom out
	m_vertical_zoom /= VERTICAL_ZOOM_STEP_FACTOR;
	if (m_vertical_zoom < VERTICAL_ZOOM_MIN)
	    m_vertical_zoom = VERTICAL_ZOOM_MIN;
	event->accept();
	refreshSignalLayer();
    } else {
	// no change
	event->ignore();
    }
}

//***************************************************************************
void SignalWidget::paintEvent(QPaintEvent *)
{
    InhibitRepaintGuard inhibit(*this, false); // avoid recursion

//     qDebug("SignalWidget::paintEvent()");
// #define DEBUG_REPAINT_TIMES
#ifdef DEBUG_REPAINT_TIMES
    QTime time;
    time.start();
#endif /* DEBUG_REPAINT_TIMES */

    QPainter p;

    int n_tracks = m_signal_manager.isClosed() ? 0 : tracks();

    m_width = QWidget::width();
    m_height = QWidget::height();

//     qDebug("SignalWidget::paintEvent(): width=%d, height=%d",m_width,m_height);

    // --- detect size changes and refresh the whole display ---
    if ((m_width != m_last_width) || (m_height != m_last_height)) {
//	qDebug("SignalWidget::paintEvent(): window size changed from "
//	      "%dx%d to %dx%d",lastWidth,lastHeight,m_width,m_height);
	for (int i=0; i<3; i++) {
	    m_layer[i] = QImage(m_width, m_height,
	        QImage::Format_ARGB32_Premultiplied);
	    m_update_layer[i] = true;
	}
	m_image = QImage(m_width, m_height,
	    QImage::Format_ARGB32_Premultiplied);

	m_last_width = m_width;
	m_last_height = m_height;

	// check and correct m_zoom and m_offset
	setZoom(m_zoom);
    }

    // --- repaint of the signal layer ---
    if (m_update_layer[LAYER_SIGNAL]) {
// 	qDebug("SignalWidget::paintEvent(): - redraw of signal layer -");

	int track_height = (n_tracks) ? (m_height / n_tracks) : 0;
	int top = 0;

	p.begin(&m_layer[LAYER_SIGNAL]);

	// all black if empty
	if (!n_tracks)
	    p.fillRect(0, 0, m_width, m_height, Qt::black);

	foreach (TrackPixmap *pix, m_track_pixmaps) {
	    if (!pix) continue; // signal closed ?

	    // fix the width and height of the track pixmap
	    if ((pix->width() != m_width) || (pix->height() != track_height))
		pix->resize(m_width, track_height);

	    pix->setVerticalZoom(m_vertical_zoom);
	    if (pix->isModified())
		pix->repaint();

	    p.setCompositionMode(QPainter::CompositionMode_Source);
	    p.drawPixmap(0, top, pix->pixmap());

	    top += track_height;
	}
	p.end();

	m_update_layer[LAYER_SIGNAL] = false;
    }

    // --- repaint of the markers layer ---
    if (m_update_layer[LAYER_MARKERS]) {
// 	qDebug("SignalWidget::paintEvent(): - redraw of markers layer -");

	p.begin(&m_layer[LAYER_MARKERS]);
	p.fillRect(0, 0, m_width, m_height, Qt::black);

	int last_marker = -1;
	foreach (const Label label, labels()) {
	    unsigned int pos = label.pos();
	    if (pos < m_offset) continue; // outside left
	    int x = samples2pixels(pos - m_offset);
	    if (x >= m_width) continue; // outside right

	    // position must differ from the last one, otherwise we
	    // would wipe out the last one with XOR mode
	    if (x == last_marker) continue;

	    p.setPen(Qt::cyan);
	    p.setCompositionMode(QPainter::CompositionMode_Exclusion);
	    p.drawLine(x, 0, x, m_height);

	    last_marker = x;
	}

	p.end();

	m_update_layer[LAYER_MARKERS] = false;
    }

    // --- repaint of the selection layer ---
    if (m_update_layer[LAYER_SELECTION]) {
// 	qDebug("SignalWidget::paintEvent(): - redraw of selection layer -");

	p.begin(&m_layer[LAYER_SELECTION]);
	p.fillRect(0, 0, m_width, m_height, Qt::black);

	if (n_tracks) {
	    unsigned int left  = m_signal_manager.selection().first();
	    unsigned int right = m_signal_manager.selection().last();
	    if ((right > m_offset) && (left < m_offset+pixels2samples(m_width))) {
		// transform to pixel coordinates
		if (left < m_offset) left = m_offset;
		left  = samples2pixels(left - m_offset);
		right = samples2pixels(right - m_offset);

		if (right >= static_cast<unsigned int>(m_width))
		    right=m_width-1;
		if (left > right)
		    left = right;

		p.setPen(Qt::yellow);
		if (left == right) {
		    p.drawLine(left, 0, left, m_height);
		} else {
		    p.setBrush(Qt::yellow);
		    p.drawRect(left, 0, right-left+1, m_height);
		}
	    }
	}
	p.end();

	m_update_layer[LAYER_SELECTION] = false;
    }

    // bitBlt all layers together
    p.begin(&m_image);
    p.fillRect(0, 0, m_width, m_height, Qt::black);
    const QPainter::CompositionMode layer_rop[3] = {
	QPainter::CompositionMode_Source,    /* LAYER_SIGNAL    */
	QPainter::CompositionMode_Exclusion, /* LAYER_SELECTION */
	QPainter::CompositionMode_Exclusion  /* LAYER_MARKERS   */
    };
    for (int i=0; i < 3; i++) {
	p.setCompositionMode(layer_rop[i]);
	p.drawImage(0, 0, m_layer[i]);
    }
    m_last_playpointer = -2;

    // --- redraw the playpointer if a signal is present ---
    m_playpointer = samples2pixels(
	m_signal_manager.playbackController().currentPos() - m_offset);

    if (n_tracks) {
	p.setPen(Qt::yellow);
	p.setCompositionMode(QPainter::CompositionMode_Exclusion);

	if (m_last_playpointer >= 0)
	    p.drawLine(m_last_playpointer, 0, m_last_playpointer, m_height);

	if ( (m_signal_manager.playbackController().running() ||
	      m_signal_manager.playbackController().paused() ) &&
	     ((m_playpointer >= 0) && (m_playpointer < m_width)) )
	{
	    p.drawLine(m_playpointer, 0, m_playpointer, m_height);
	    m_last_playpointer = m_playpointer;
	} else {
	    m_last_playpointer = -1;
	}
    }
    p.end();

    // draw the result
    p.begin(this);
    p.drawImage(0, 0, m_image);
    p.end();

#ifdef DEBUG_REPAINT_TIMES
   qDebug("SignalWidget::paintEvent() -- done, t=%d ms --", time.elapsed());
#endif /* DEBUG_REPAINT_TIMES */
}

//***************************************************************************
unsigned int SignalWidget::ms2samples(double ms)
{
    return static_cast<unsigned int>(
	rint(ms * m_signal_manager.rate() / 1E3));
}

//***************************************************************************
double SignalWidget::samples2ms(unsigned int samples)
{
    double rate = m_signal_manager.rate();
    if (rate == 0.0) return 0.0;
    return static_cast<double>(samples) * 1E3 / rate;
}

//***************************************************************************
unsigned int SignalWidget::pixels2samples(int pixels) const
{
    if ((pixels < 0) || (m_zoom <= 0.0)) return 0;
    return static_cast<unsigned int>(rint(
	static_cast<double>(pixels) * m_zoom));
}

//***************************************************************************
int SignalWidget::samples2pixels(int samples) const
{
    if (m_zoom == 0.0) return 0;
    return static_cast<int>(rint(static_cast<double>(samples) / m_zoom));
}

//***************************************************************************
int SignalWidget::displaySamples()
{
    return pixels2samples(width()-1);
}

//***************************************************************************
Label SignalWidget::findLabelNearMouse(int x) const
{
    const int tol = SELECTION_TOLERANCE;
    unsigned int pos = m_offset + pixels2samples(x);
    Label nearest;
    unsigned int dmin = pixels2samples(SELECTION_TOLERANCE) + 1;

    foreach (Label label, labels()) {
	unsigned int lp = label.pos();
	if (lp < m_offset) continue; // outside left
	int lx = samples2pixels(lp - m_offset);
	if (lx >= m_width) continue; // outside right

	if ((lx + tol < x) || (lx > x + tol))
	    continue; // out of tolerance

	unsigned int dist = (pos > lp) ? (pos - lp) : (lp - pos);
	if (dist < dmin) {
	    // found a new "nearest" label
	    dmin = dist;
	    nearest = label;
	}
    }

    return nearest;
}

//***************************************************************************
void SignalWidget::addLabel(unsigned int pos)
{
    UndoTransactionGuard undo(m_signal_manager, i18n("add label"));

    // add a new label, with undo
    if (!m_signal_manager.addLabel(pos)) {
	m_signal_manager.abortUndoTransaction();
	return;
    }

    // find the new label
    Label label = m_signal_manager.findLabel(pos);
    if (label.isNull()) return;

    // edit the properties of the new label
    if (!labelProperties(label)) {
	// aborted or failed -> delete (without undo)
	int index = m_signal_manager.labelIndex(label);
	m_signal_manager.deleteLabel(index, false);
	m_signal_manager.abortUndoTransaction();
    }

    refreshMarkersLayer();
    hidePosition();
}

////****************************************************************************
//void SignalWidget::loadLabel()
//{
//    labels().clear();    //remove old Labels...
//    appendLabel ();
//}
//
////****************************************************************************
//void SignalWidget::saveLabel (const char *typestring)
//{
//    QString name = KFileDialog::getSaveFileName (0, "*.label", this);
//    if (!name.isNull()) {
//	FILE *out;
//	out = fopen (name.local8Bit(), "w");
//
//	Parser parser (typestring);
//	Label *tmp;
//	LabelType *act;
//
//	const char *actstring = parser.getFirstParam();
//
//	while (actstring) {
//	    printf ("selecting %s\n", actstring);
//	    for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
//		if (strcmp(act->name, actstring) == 0) {
//		    printf ("selected\n");
//		    act->selected = true;
//		    break;
//		}
//	    actstring = parser.getNextParam();
//	}
//
//	for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
//	    //write out all selected label types
//	    if (act->selected)
//		fprintf (out, "%s\n", act->getCommand());
//
//	//ended writing of types, so go on with the labels...
//
//	for (tmp = labels().first(); tmp; tmp = labels().next())  //write out labels
//	{
//	    fprintf (out, tmp->getCommand());
//	    fprintf (out, "\n");
//	}
//
//	fclose (out);
//    }
//}

//***************************************************************************
bool SignalWidget::labelProperties(Label &label)
{
    if (label.isNull()) return false;
    int index = m_signal_manager.labelIndex(label);
    Q_ASSERT(index >= 0);
    if (index < 0) return false;

    // try to modify the label. just in case that the user moves it
    // to a position where we already have one, catch this situation
    // and ask if he wants to abort, re-enter the label properties
    // dialog or just replace (remove) the label at the target position
    bool accepted;
    unsigned int new_pos  = label.pos();
    QString      new_name = label.name();
    int old_index = -1;
    while (true) {
	// create and prepare the dialog
	LabelPropertiesWidget *dlg = new LabelPropertiesWidget(this);
	Q_ASSERT(dlg);
	if (!dlg) return false;
	dlg->setLabelIndex(index);
	dlg->setLabelPosition(new_pos, m_signal_manager.length(),
	    m_signal_manager.rate());
	dlg->setLabelName(new_name);

	// execute the dialog
	accepted = (dlg->exec() == QDialog::Accepted);
	if (!accepted) {
	    // user pressed "cancel"
	    delete dlg;
	    break;
	}

	// if we get here the user pressed "OK"
	new_pos  = dlg->labelPosition();
	new_name = dlg->labelName();
	dlg->saveSettings();
	delete dlg;

	// check: if there already is a label at the new position
	// -> ask the user if he wants to overwrite that one
	if ((new_pos != label.pos()) &&
	    !m_signal_manager.findLabel(new_pos).isNull())
	{
	    int res = Kwave::MessageBox::warningYesNoCancel(this, i18n(
		"There already is a label at the position "\
		"you have choosen.\n"\
		"Do you want to replace it ?"));
	    if (res == KMessageBox::Yes) {
		// delete the label at the target position (with undo)
		Label old = m_signal_manager.findLabel(new_pos);
		old_index = m_signal_manager.labelIndex(old);
		break;
	    }
	    if (res == KMessageBox::No) {
		// make another try -> re-enter the dialog
		continue;
	    }

	    // cancel -> abort the whole action
	    accepted = false;
	    break;
	} else {
	    // ok, we can put it there
	    break;
	}
    }

    if (accepted) {
	// shortcut: abort if nothing has changed
	if ((new_name == label.name()) && (new_pos == label.pos()))
	    return false;

	UndoTransactionGuard undo(m_signal_manager, i18n("modify label"));

	// if there is a label at the target position, remove it first
	if (old_index >= 0) {
	    m_signal_manager.deleteLabel(old_index, true);
	    // this might have changed the current index!
	    index = m_signal_manager.labelIndex(label);
	}

	// modify the label through the signal manager
	if (!m_signal_manager.modifyLabel(index, new_pos, new_name)) {
	    // position is already occupied
	    m_signal_manager.abortUndoTransaction();
	    return false;
	}

	// reflect the change in the passed label
	label.moveTo(new_pos);
	label.rename(new_name);

	// NOTE: moving might also change the index, so the complete
	//       markers layer has to be refreshed
	refreshMarkersLayer();
    }
    else
	m_signal_manager.abortUndoTransaction();

    return accepted;
}

////****************************************************************************
//void SignalWidget::jumptoLabel ()
// another fine function contributed by Gerhard Zintel
// if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// rmarker jumps to next higher label or end
//{
//    if (signalmanage) {
//	int lmarker = signalmanage->getLMarker();
//	int rmarker = signalmanage->getRMarker();
//	bool RangeSelected = (rmarker - lmarker) > 0;
//	if (!labels().isEmpty()) {
//	    Label *tmp;
//	    int position = 0;
//	    for (tmp = labels->first(); tmp; tmp = labels->next())
//		if (RangeSelected) {
//		    if (tmp->pos < lmarker)
//			if (qAbs(lmarker - position) >
//			    qAbs(lmarker - ms2samples(tmp->pos)))
//			    position = ms2samples(tmp->pos);
//		} else if (qAbs(lmarker - position) >
//			   qAbs(lmarker - ms2samples(tmp->pos)))
//		    position = ms2samples(tmp->pos);
//
//	    lmarker = position;
//	    position = signalmanage->getLength();
//	    for (tmp = labels->first(); tmp; tmp = labels->next())
//		if (tmp->pos > rmarker)
//		    if (qAbs(rmarker - position) >
//			qAbs(rmarker - ms2samples(tmp->pos)))
//			position = ms2samples(tmp->pos);
//	    rmarker = position;
//	    if (RangeSelected) setRange(lmarker, rmarker);
//	    else setRange (lmarker, lmarker);
//	    refresh ();
//	}
//    }
//}
//
////****************************************************************************
//void SignalWidget::savePeriods ()
//{
//    if (signalmanage) {
//	Dialog *dialog =
//	    DynamicLoader::getDialog ("marksave", new DialogOperation(&globals, signalmanage->getRate(), 0, 0));
//
//	if ((dialog) && (dialog->exec())) {
//	    selectMarkers (dialog->getCommand());
//
//	    LabelType *act;
//	    Label *tmp;
//	    int last = 0;
//	    int rate = signalmanage->getRate ();
//
//	    QString name = KFileDialog::getSaveFileName (0, "*.dat", this);
//	    if (!name.isNull()) {
//		QFile out(name.local8Bit());
//		char buf[160];
//		float freq = 0, time, lastfreq = 0;
//		out.open (QIODevice::WriteOnly);
//		int first = true;
//
//		for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
//		    //write only selected label type
//		    if (act->selected)
//			//traverse list of all labels
//			for (tmp = labels->first(); tmp; tmp = labels->next()) {
//			    if (tmp->getType() == act) {
//				freq = tmp->pos - last;
//				time = last * 1000 / rate;
//
//				if ((!first) && (freq != lastfreq)) {
//				    lastfreq = freq;
//				    freq = 1 / (freq / rate);
//				    snprintf (buf, sizeof(buf), "%f %f\n",
//					time, freq);
//				    out.writeBlock (&buf[0], strlen(buf));
//				} else lastfreq = freq;
//				first = false;
//				last = ms2samples(tmp->pos);
//			    }
//			}
//
//		if (!first) //make sure last tone gets its length
//		{
//		    time = last * 1000 / rate;
//		    snprintf (buf, sizeof(buf), "%f %f\n", time, freq);
//		    out.writeBlock (&buf[0], strlen(buf));
//		}
//
//		out.close ();
//	    }
//	}
//    }
//}

////****************************************************************************
//void SignalWidget::markSignal (const char *str)
//{
//    if (signalmanage) {
//	Label *newmark;
//
//	Parser parser (str);
//
//	int level = (int) (parser.toDouble() / 100 * (1 << 23));
//
//	int len = signalmanage->getLength();
//	int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
//	LabelType *start = findMarkerType(parser.getNextParam());
//	LabelType *stop = findMarkerType (parser.getNextParam());
//	int time = (int) (parser.toDouble () * signalmanage->getRate() / 1000);
//
//	printf ("%d %d\n", level, time);
//	printf ("%s %s\n", start->name, stop->name);
//
//	ProgressDialog *dialog =
//	    new ProgressDialog (len, "Searching for Signal portions...");
//
//	if (dialog && start && stop) {
//	    dialog->show();
//
//	    newmark = new Label(0, start);     //generate initial Label
//
//	    labels->inSort (newmark);
//
//	    for (int i = 0; i < len; i++) {
//		if (qAbs(sam[i]) < level) {
//		    int j = i;
//		    while ((i < len) && (qAbs(sam[i]) < level)) i++;
//
//		    if (i - j > time) {
//			//insert labels...
//			newmark = new Label(i, start);
//			labels->inSort (newmark);
//
//			if (start != stop) {
//			    newmark = new Label(j, stop);
//			    labels->inSort (newmark);
//			}
//		    }
//		}
//		dialog->setProgress (i);
//	    }
//
//	    newmark = new Label(len - 1, stop);
//	    labels->inSort (newmark);
//
//	    refresh ();
//	    delete dialog;
//	}
//    }
//}
//
////****************************************************************************
//void SignalWidget::markPeriods (const char *str)
//{
//    if (signalmanage) {
//	Parser parser (str);
//
//	int high = signalmanage->getRate() / parser.toInt();
//	int low = signalmanage->getRate() / parser.toInt();
//	int octave = parser.toBool ("true");
//	double adjust = parser.toDouble ();
//
//	for (int i = 0; i < AUTOKORRWIN; i++)
//	    autotable[i] = 1 - (((double)i * i * i) / (AUTOKORRWIN * AUTOKORRWIN * AUTOKORRWIN));    //generate static weighting function
//
//	if (octave) for (int i = 0; i < AUTOKORRWIN; i++) weighttable[i] = 1;    //initialise moving weight table
//
//	Label *newmark;
//	int next;
//	int len = signalmanage->getLength();
//	int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
//	LabelType *start = markertype;
//	int cnt = findFirstMark (sam, len);
//
//	ProgressDialog *dialog = new ProgressDialog (len - AUTOKORRWIN, "Correlating Signal to find Periods:");
//	if (dialog) {
//	    dialog->show();
//
//	    newmark = new Label(cnt, start);
//	    labels->inSort (newmark);
//
//	    while (cnt < len - 2*AUTOKORRWIN) {
//		if (octave)
//		    next = findNextRepeatOctave (&sam[cnt], high, adjust);
//		else
//		    next = findNextRepeat (&sam[cnt], high);
//
//		if ((next < low) && (next > high)) {
//		    newmark = new Label(cnt, start);
//
//		    labels->inSort (newmark);
//		}
//		if (next < AUTOKORRWIN) cnt += next;
//		else
//		    if (cnt < len - AUTOKORRWIN) {
//			int a = findFirstMark (&sam[cnt], len - cnt);
//			if (a > 0) cnt += a;
//			else cnt += high;
//		    } else cnt = len;
//
//		dialog->setProgress (cnt);
//	    }
//
//	    delete dialog;
//
//	    refresh ();
//	}
//    }
//}

////*****************************************************************************
//int findNextRepeat (int *sample, int high)
////autocorellation of a windowed part of the sample
////returns length of period, if found
//{
//    int i, j;
//    double gmax = 0, max, c;
//    int maxpos = AUTOKORRWIN;
//    int down, up;         //flags
//
//    max = 0;
//    for (j = 0; j < AUTOKORRWIN; j++)
//	gmax += ((double)sample[j]) * sample [j];
//
//    //correlate signal with itself for finding maximum integral
//
//    down = 0;
//    up = 0;
//    i = high;
//    max = 0;
//    while (i < AUTOKORRWIN) {
//	c = 0;
//	for (j = 0; j < AUTOKORRWIN; j++) c += ((double)sample[j]) * sample [i + j];
//	c = c * autotable[i];    //multiply window with weight for preference of high frequencies
//	if (c > max) max = c, maxpos = i;
//	i++;
//    }
//    return maxpos;
//}
//
////*****************************************************************************
//int findNextRepeatOctave (int *sample, int high, double adjust = 1.005)
////autocorellation of a windowed part of the sample
////same as above only with an adaptive weighting to decrease fast period changes
//{
//    int i, j;
//    double gmax = 0, max, c;
//    int maxpos = AUTOKORRWIN;
//    int down, up;         //flags
//
//    max = 0;
//    for (j = 0; j < AUTOKORRWIN; j++)
//	gmax += ((double)sample[j]) * sample [j];
//
//    //correlate signal with itself for finding maximum integral
//
//    down = 0;
//    up = 0;
//    i = high;
//    max = 0;
//    while (i < AUTOKORRWIN) {
//	c = 0;
//	for (j = 0; j < AUTOKORRWIN; j++) c += ((double)sample[j]) * sample [i + j];
//	c = c * autotable[i] * weighttable[i];
//	//multiply window with weight for preference of high frequencies
//	if (c > max) max = c, maxpos = i;
//	i++;
//    }
//
//    for (int i = 0; i < AUTOKORRWIN; i++) weighttable[i] /= adjust;
//
//    weighttable[maxpos] = 1;
//    weighttable[maxpos + 1] = .9;
//    weighttable[maxpos - 1] = .9;
//    weighttable[maxpos + 2] = .8;
//    weighttable[maxpos - 2] = .8;
//
//    float buf[7];
//
//    for (int i = 0; i < 7; buf[i++] = .1)
//
//	//low pass filter
//	for (int i = high; i < AUTOKORRWIN - 3; i++) {
//	    buf[i % 7] = weighttable[i + 3];
//	    weighttable[i] = (buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6]) / 7;
//	}
//
//    return maxpos;
//}
//
//*****************************************************************************
//int findFirstMark (int *sample, int len)
////finds first sample that is non-zero, or one that preceeds a zero crossing
//{
//    int i = 1;
//    int last = sample[0];
//    int act = last;
//    if ((last < 100) && (last > -100)) i = 0;
//    else
//	while (i < len) {
//	    act = sample[i];
//	    if ((act < 0) && (last >= 0)) break;
//	    if ((act > 0) && (last <= 0)) break;
//	    last = act;
//	    i++;
//	}
//    return i;
//}

//***************************************************************************
void SignalWidget::playbackStopped()
{
    InhibitRepaintGuard inhibit(*this);
}

//***************************************************************************
void SignalWidget::updatePlaybackPointer(unsigned int)
{
    InhibitRepaintGuard inhibit(*this);
}

//***************************************************************************
void SignalWidget::slotTrackInserted(unsigned int index, Track *track)
{
    Q_ASSERT(track);
    if (!track) return;

    // insert a new track into the track pixmap list
    TrackPixmap *pix = new TrackPixmap(*track);
    Q_ASSERT(pix);
    m_track_pixmaps.insert(index, pix);
    if (!pix) return;

    // first track: start with full zoom
    if (tracks() == 1) zoomAll();

    // emit the signal sigTrackInserted now, so that the signal widget
    // gets resized if needed, but the new pixmap is still empty
    emit sigTrackInserted(index);

    pix->setOffset(m_offset);
    pix->setZoom(m_zoom);
    if (pix->isModified()) refreshSignalLayer();

    // connect all signals
    connect(pix, SIGNAL(sigModified()),
            this, SLOT(refreshSignalLayer()));

    connect(track, SIGNAL(sigSelectionChanged()),
	    this, SIGNAL(sigTrackSelectionChanged()));
    connect(track, SIGNAL(sigSelectionChanged()),
            this, SLOT(refreshSignalLayer()));
}

//***************************************************************************
void SignalWidget::slotTrackDeleted(unsigned int index)
{
    // delete the track from the list
    if (static_cast<int>(index) < m_track_pixmaps.count()) {
	TrackPixmap *pixmap = m_track_pixmaps.takeAt(index);
	if (pixmap) delete pixmap;
    }

    // emit the signal sigTrackInserted now, so that the signal widget
    // gets resized if needed, but the new pixmap is still empty
    emit sigTrackDeleted(index);

    refreshSignalLayer();
}

//***************************************************************************
void SignalWidget::slotSamplesInserted(unsigned int track,
    unsigned int offset, unsigned int length)
{
//     qDebug("SignalWidget(): slotSamplesInserted(%u, %u,%u)", track,
// 	offset, length);
    Q_UNUSED(track);
    Q_UNUSED(offset);
    Q_UNUSED(length);

    // length has changed -> update overview
    emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
                  m_signal_manager.length());

    refreshMarkersLayer();
}

//***************************************************************************
void SignalWidget::slotSamplesDeleted(unsigned int track,
    unsigned int offset, unsigned int length)
{
//    qDebug("SignalWidget(): slotSamplesDeleted(%u, %u...%u)", track,
// 	offset, offset+length-1);
    Q_UNUSED(track);
    Q_UNUSED(offset);
    Q_UNUSED(length);

    // length has changed -> update overview
    emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
                  m_signal_manager.length());

    refreshMarkersLayer();
}

//***************************************************************************
void SignalWidget::slotSamplesModified(unsigned int /*track*/,
    unsigned int /*offset*/, unsigned int /*length*/)
{
//    qDebug("SignalWidget(): slotSamplesModified(%u, %u,%u)", track,
//	offset, length);
}

//***************************************************************************
void SignalWidget::startDragging()
{
    const unsigned int length = m_signal_manager.selection().length();
    if (!length) return;

    KwaveDrag *d = new KwaveDrag(this);
    Q_ASSERT(d);
    if (!d) return;

    const unsigned int first = m_signal_manager.selection().first();
    const unsigned int last  = m_signal_manager.selection().last();
    const double       rate  = m_signal_manager.rate();
    const unsigned int bits  = m_signal_manager.bits();

    MultiTrackReader src(Kwave::SinglePassForward, m_signal_manager,
	m_signal_manager.selectedTracks(), first, last);

    // create the file info
    FileInfo info = m_signal_manager.fileInfo();
    info.setLength(last - first + 1);
    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(src.tracks());

    if (!d->encode(this, src, info)) {
	delete d;
	return;
    }

    // start drag&drop, mode is determined automatically
    InhibitRepaintGuard inhibit(*this);
    UndoTransactionGuard undo(m_signal_manager, i18n("drag and drop"));
    Qt::DropAction drop = d->exec(Qt::CopyAction | Qt::MoveAction);

    if (drop == Qt::MoveAction) {
	// deleting also affects the selection !
	const unsigned int f = m_signal_manager.selection().first();
	const unsigned int l = m_signal_manager.selection().last();
	const unsigned int len = l-f+1;

	// special case: when dropping into the same widget, before
	// the previous selection, the previous range has already
	// been moved to the right !
	unsigned int src = first;
	if ((d->target() == this) && (f < src)) src += len;

	m_signal_manager.deleteRange(src, len,
		m_signal_manager.selectedTracks());

	// restore the new selection
	selectRange((first < f) ? f-len : f, len);
    }
}

//***************************************************************************
void SignalWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (!event) return;
    if ((event->proposedAction() != Qt::MoveAction) &&
        (event->proposedAction() != Qt::CopyAction))
        return; /* unsupported action */

    if (KwaveFileDrag::canDecode(event->mimeData()))
	event->acceptProposedAction();
}

//***************************************************************************
void SignalWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    setMouseMode(MouseNormal);
}

//***************************************************************************
void SignalWidget::dropEvent(QDropEvent *event)
{
    if (!event) return;
    if (!event->mimeData()) return;

    if (KwaveDrag::canDecode(event->mimeData())) {
	UndoTransactionGuard undo(m_signal_manager, i18n("drag and drop"));
	InhibitRepaintGuard inhibit(*this);
	unsigned int pos = m_offset + pixels2samples(event->pos().x());
	unsigned int len = 0;

	if ((len = KwaveDrag::decode(this, event->mimeData(),
	    m_signal_manager, pos)))
	{
	    // set selection to the new area where the drop was done
	    selectRange(pos, len);
	    event->acceptProposedAction();
	} else {
	    qWarning("SignalWidget::dropEvent(%s): failed !", event->format(0));
	    event->ignore();
	}
    } else if (event->mimeData()->hasUrls()) {
	bool first = true;
	foreach (QUrl url, event->mimeData()->urls()) {
	    QString filename = url.toLocalFile();
	    QString mimetype = CodecManager::whatContains(filename);
	    if (CodecManager::canDecode(mimetype)) {
		if (first) {
		    // first dropped URL -> open in this window
		    emit sigCommand("open(" + filename + ")");
		    first = false;
		} else {
		    // all others -> open a new window
		    emit sigCommand("newwindow(" + filename + ")");
		}
	    }
	}
    }

    qDebug("SignalWidget::dropEvent(): done");
    setMouseMode(MouseNormal);
}

//***************************************************************************
void SignalWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (!event) return;

    const int x = event->pos().x();

    if ((event->source() == this) && isInSelection(x)) {
	// disable drag&drop into the selection itself
	// this would be nonsense

	unsigned int left  = m_signal_manager.selection().first();
	unsigned int right = m_signal_manager.selection().last();
	const unsigned int w = pixels2samples(m_width);
	QRect r(this->rect());

	// crop selection to widget borders
	if (left < m_offset) left = m_offset;
	if (right > m_offset+w) right = m_offset+w-1;

	// transform to pixel coordinates
	left  = samples2pixels(left - m_offset);
	right = samples2pixels(right - m_offset);
	if (right >= static_cast<unsigned int>(m_width))
	    right=m_width-1;
	if (left > right)
	    left = right;

	r.setLeft(left);
	r.setRight(right);
	event->ignore(r);
    } else if (KwaveDrag::canDecode(event->mimeData())) {
	// accept if it is decodeable within the
	// current range (if it's outside our own selection)
	event->acceptProposedAction();
    } else if (KwaveFileDrag::canDecode(event->mimeData())) {
	// file drag
	event->accept();
    } else event->ignore();
}

//***************************************************************************
#include "SignalWidget.moc"
//***************************************************************************
//***************************************************************************
