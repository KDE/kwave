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

#include <qdragobject.h>
#include <qstrlist.h>

#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>

#include "libkwave/FileInfo.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "libgui/MenuManager.h"
#include "libgui/TrackPixmap.h"

#include "CodecManager.h"
#include "SignalWidget.h"
#include "SignalManager.h"
#include "MouseMark.h"
#include "UndoTransactionGuard.h"

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
#define REPAINT_INTERVAL 333

//***************************************************************************
//***************************************************************************
class KwaveFileDrag: public QUriDrag
{
public:
    static bool canDecode(const QMimeSource *source) {
	if (!source) return false;
	if (!QUriDrag::canDecode(source)) return false;

	QStringList files;
	decodeLocalFiles(source, files);
	QStringList::Iterator it;
	for (it = files.begin(); it != files.end(); ++it) {
	    QString mimetype = CodecManager::whatContains(*it);
	    if (CodecManager::canDecode(mimetype)) return true;
	}
	return false;
    };
};

//***************************************************************************
//***************************************************************************
SignalWidget::SignalWidget(QWidget *parent)
    :QWidget(parent),
    m_offset(0), m_width(0), m_height(0), m_last_width(0), m_last_height(0),
    m_zoom(0.0), m_playpointer(-1), m_last_playpointer(-1), m_redraw(false),
    m_inhibit_repaint(0), m_selection(0),
    m_signal_manager(this),
    m_track_pixmaps(), m_pixmap(0),
    m_mouse_mode(MouseNormal),
    m_mouse_down_x(0), m_repaint_timer()
{
//    qDebug("SignalWidget::SignalWidget()");

    for (int i=0; i < 3; i++) {
	m_layer[i] = 0;
	m_update_layer[i] = true;
	m_layer_rop[i] = CopyROP;
    }

    m_selection = new MouseMark();
    Q_ASSERT(m_selection);
    if (!m_selection) return;

    // connect to the signal manager's signals
    SignalManager *sig = &m_signal_manager;

    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track &)),
            this, SLOT(slotTrackInserted(unsigned int, Track &)));
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

    connect(&(sig->selection()), SIGNAL(changed(unsigned int, unsigned int)),
	this, SLOT(slotSelectionChanged(unsigned int, unsigned int)));

    // connect to the playback controller
    connect(&(playbackController()), SIGNAL(sigPlaybackPos(unsigned int)),
            this, SLOT(updatePlaybackPointer(unsigned int)));

    // connect repaint timer
    connect(&m_repaint_timer, SIGNAL(timeout()),
            this, SLOT(timedRepaint()));

//    labels = new LabelList();
//    Q_ASSERT(labels);
//    if (!labels) return;
//    labels->setAutoDelete (true);

//    m_menu_manager.clearNumberedMenu("ID_LABELS_TYPE");
//    for (LabelType *tmp = globals.markertypes.first(); tmp;
//         tmp = globals.markertypes.next())
//    {
//	m_menu_manager.addNumberedMenuEntry("ID_LABELS_TYPE", (char *)tmp->name);
//    }
//
//    markertype = globals.markertypes.first();

    setBackgroundColor(black);
    setBackgroundMode(NoBackground); // this avoids flicker :-)

    setMouseTracking(true);
    setAcceptDrops(true); // enable drag&drop

    setZoom(0.0);
//    qDebug("SignalWidget::SignalWidget(): done.");
}

//***************************************************************************
void SignalWidget::refreshSelection()
{
    refreshLayer(LAYER_SELECTION);
}

//***************************************************************************
void SignalWidget::refreshSignalLayer()
{
    refreshLayer(LAYER_SIGNAL);
}

//***************************************************************************
bool SignalWidget::isOK()
{
////    Q_ASSERT(labels);
    Q_ASSERT(m_selection);
    return ( /* labels && */ m_selection );
}

//***************************************************************************
SignalWidget::~SignalWidget()
{
    close();

    if (m_pixmap) delete m_pixmap;
    m_pixmap = 0;

////    if (labels) delete labels;
////    labels = 0;

    if (m_selection) delete m_selection;
    m_selection = 0;

    for (int i=0; i < 3; i++) {
	if (m_layer[i]) delete m_layer[i];
	m_layer[i] = 0;
    }
}

//***************************************************************************
int SignalWidget::saveFile(const KURL &url, bool selection)
{
    return m_signal_manager.save(url, selection);
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

    unsigned int visible_samples = pixels2samples(m_width);

    if (false) {
    // zoom
    CASE_COMMAND("zoomin")
	zoomIn();
    CASE_COMMAND("zoomout")
	zoomOut();
    CASE_COMMAND("zoomselection")
	zoomSelection();
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
	selectRange(m_signal_manager.selection().last()+1,
	            m_signal_manager.selection().length());
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
//bool SignalWidget::executeLabelCommand(const QString &command)
//{
//    if (false) {
//    CASE_COMMAND("chooselabel")
//	Parser parser(command);
//	markertype = globals.markertypes.at(parser.toInt());
//    CASE_COMMAND("amptolabel")
//	markSignal(command);
//    CASE_COMMAND("pitch")
//	markPeriods(command);
////    CASE_COMMAND("labeltopitch")
////      convertMarkstoPitch(command);
//    CASE_COMMAND("deletelabel")
//	deleteLabel();
//    CASE_COMMAND("insertlabel")
//	appendLabel();
//    CASE_COMMAND("loadlabel")
//	loadLabel();
//    CASE_COMMAND("savelabel")
//	saveLabel(command);
//    CASE_COMMAND("label")
//	addLabel(command);
//    CASE_COMMAND("newlabeltype")
//	addLabelType(command);
//    CASE_COMMAND("expandtolabel")
//	jumptoLabel();
//    CASE_COMMAND("mark")
//	markSignal(command);
//    CASE_COMMAND("markperiod")
//	markPeriods(command);
//    CASE_COMMAND("saveperiods")
//	savePeriods();
//    } else return false;
//
//    return true;
//}

//***************************************************************************
bool SignalWidget::executeCommand(const QString &command)
{
    InhibitRepaintGuard inhibit(*this);

    if (!command.length()) return true;

    if (executeNavigationCommand(command)) {
	return true;
    } else {
	return m_signal_manager.executeCommand(command);
    }

    return true;
}

//***************************************************************************
void SignalWidget::selectRange(unsigned int offset, unsigned int length)
{
    m_signal_manager.selectRange(offset, length);

    // if the selection has been changed through a command we also
    // have to update the mouse selection
    if ( (m_selection->left()  != m_signal_manager.selection().first()) ||
         (m_selection->right() != m_signal_manager.selection().last()))
    {
	m_selection->set(m_signal_manager.selection().first(),
	                 m_signal_manager.selection().last());
    }
}

//***************************************************************************
void SignalWidget::slotSelectionChanged(unsigned int offset,
                                        unsigned int length)
{
    m_signal_manager.selectRange(offset, length);
    offset = m_signal_manager.selection().offset();
    length = m_signal_manager.selection().length();

    refreshSelection();
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
int SignalWidget::loadFile(const KURL &url)
{
    // close the previous signal
    close();

    // load a new signal
    int res = m_signal_manager.loadFile(url);
    if (m_signal_manager.isClosed() || (res)) {
	qWarning("SignalWidget::loadFile() failed:"\
		" zero-length or out of memory?");

	QString reason;
	switch (res) {
	    case -ENOMEM:
		reason = i18n("Out of memory");
		break;
	    case -EIO:
		reason = i18n("unable to open '%1'").arg(
		    url.prettyURL());
		break;
	    case -EINVAL:
		reason = i18n("invalid or unknown file type: '%1'").arg(
		              url.prettyURL());
		break;
	    default:
		reason = "";
	}

        // show an error message box if the reason was known
	if (reason.length()) {
	    KMessageBox::error(this, reason);
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
    close();
    m_signal_manager.newSignal(samples,rate,bits,tracks);
}

//***************************************************************************
void SignalWidget::close()
{
    // first stop playback
    m_signal_manager.playbackController().playbackStop();
    m_signal_manager.playbackController().reset();

    // clear the display
    m_track_pixmaps.setAutoDelete(true);
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
    unsigned int n_tracks = m_track_pixmaps.count();
    for (unsigned int i = 0; i < n_tracks; i++) {
	TrackPixmap *pix = m_track_pixmaps.at(i);
	Q_ASSERT(pix);
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
        length = (unsigned int)ceil(DEFAULT_DISPLAY_TIME *
                                    m_signal_manager.rate());
    }

    // example: width = 100 [pixels] and length = 3 [samples]
    //          -> samples should be at positions 0, 49.5 and 99
    //          -> 49.5 [pixels / sample]
    //          -> zoom = 1 / 49.5 [samples / pixel]
    // => full zoom [samples/pixel] = (length-1) / (width-1)
    return (double)(length-1) / (double)(QWidget::width()-1);
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
    unsigned int n_tracks = m_track_pixmaps.count();
    for (unsigned int i = 0; i < n_tracks; i++) {
	TrackPixmap *pix = m_track_pixmaps.at(i);
	Q_ASSERT(pix);
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
	length = (unsigned int)ceil(width() * getFullZoom());
    }

    if (!m_width) return;

    // ensure that m_offset is [0...length-1]
    if (m_offset > length-1) m_offset = length-1;

    // ensure that the zoom is in a proper range
    max_zoom = getFullZoom();
    min_zoom = (double)MINIMUM_SAMPLES_PER_SCREEN / (double)m_width;
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
	    setCursor(KCursor::arrowCursor());
	    break;
	case MouseAtSelectionBorder:
	    setCursor(KCursor::sizeHorCursor());
	    break;
	case MouseInSelection:
	    setCursor(KCursor::arrowCursor());
	    break;
	case MouseSelect:
	    setCursor(KCursor::sizeHorCursor());
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
	setZoom(((double)len) / (double)(m_width - 1));
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
	    m_repaint_timer.start(REPAINT_INTERVAL, true);
	}
    }
}

//***************************************************************************
void SignalWidget::timedRepaint()
{
    this->repaint(false);
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
    Q_ASSERT(m_selection);
    if (!m_selection) return None;

    /** @todo selection tolerance should depend on some KDE setting,
              but which ? */
    const int tol = 10;
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

    // position is in the selection while left and right tolerance
    // areas overlap or leave an area that is smaller than the
    // tolerance ?
    if ((rl-lr < tol) && (x >= l) && (x <= r)) return Selection;

    // the simple cases...
    int pos = None;
    if ((x >= ll) && (x <= lr)) pos = LeftBorder;
    if ((x >= rl) && (x <= rr)) pos = RightBorder;
    if ((x >   l) && (x <   r)) pos |= Selection;

    return pos;
}

//***************************************************************************
bool SignalWidget::isSelectionBorder(int x)
{
    SelectionPos pos = static_cast<SignalWidget::SelectionPos>(
	selectionPosition(x) & ~Selection);

    return ((pos == LeftBorder) || (pos == RightBorder));
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

    if ((e->button() & MouseButtonMask) == LeftButton) {
	int mx = e->pos().x();
	if (mx < 0) mx = 0;
	if (mx >= m_width) mx = m_width-1;
	unsigned int x = m_offset + pixels2samples(mx);
	unsigned int len = m_selection->right() - m_selection->left() + 1;
	switch (e->state() & KeyButtonMask) {
	    case ShiftButton: {
		// expand the selection to "here"
		selectRange(m_selection->left(), len);
		m_selection->grep(x);
		setMouseMode(MouseSelect);
		break;
	    }
	    case ControlButton: {
		if (isInSelection(e->pos().x()) && (len > 1)) {
		    // start a drag&drop operation in "copy" mode
		    startDragging();
		}
		break;
	    }
	    case 0: {
		if (isSelectionBorder(e->pos().x())) {
		    // modify selection border
		    m_selection->grep(x);
		    setMouseMode(MouseSelect);
		} else if (isInSelection(e->pos().x()) && (len > 1)) {
		    // store the x position for later drag&drop
		    m_mouse_down_x = e->pos().x();
		} else {
		    // start a new selection
		    m_selection->set(x, x);
		    setMouseMode(MouseSelect);
		}
		break;
	    }
	}
    }
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

	    unsigned int len = m_selection->right() - m_selection->left() + 1;
	    selectRange(m_selection->left(), len);

	    setMouseMode(MouseNormal);
	    break;
	}
	case MouseInSelection: {
	    int dmin = KGlobalSettings::dndEventDelay();
	    if ((e->state() & Qt::LeftButton) &&
		    ((e->pos().x() >= m_mouse_down_x-dmin) ||
		     (e->pos().x() <= m_mouse_down_x+dmin)) )
	    {
		// deselect if only clicked without moving
		unsigned int x = m_offset + pixels2samples(e->pos().x());
		selectRange(x, 0);
		setMouseMode(MouseNormal);
	    }
	    break;
	}
	default: ;
    }

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

    switch (m_mouse_mode) {
	case MouseSelect: {
	    // in move mode, a new selection was created or an old one grabbed
	    // this does the changes with every mouse move...
	    int mx = e->pos().x();
	    if (mx < 0) mx = 0;
	    if (mx >= m_width) mx = m_width-1;

	    unsigned int x = m_offset + pixels2samples(mx);
	    m_selection->update(x);

	    unsigned int len = m_selection->right() - m_selection->left() + 1;
	    selectRange(m_selection->left(), len);
	    break;
	}
	default: {
	    // yes, this code gives the nifty cursor change....
	    int x = e->pos().x();
	    if (isSelectionBorder(x)) {
		setMouseMode(MouseAtSelectionBorder);
	    } else if (isInSelection(x)) {
		setMouseMode(MouseInSelection);
                int dmin = KGlobalSettings::dndEventDelay();
		if ((e->state() & Qt::LeftButton) &&
		    ((e->pos().x() < m_mouse_down_x-dmin) ||
		     (e->pos().x() > m_mouse_down_x+dmin)) )
		{
		    startDragging();
		}
	    } else {
		setMouseMode(MouseNormal);
	    }
	}
    }
}

void SignalWidget::paintEvent(QPaintEvent *)
{
    InhibitRepaintGuard inhibit(*this, false); // avoid recursion

//     qDebug("SignalWidget::paintEvent()");
//#ifdef DEBUG
//    static struct timeval t_start;
//    static struct timeval t_end;
//    double t_elapsed;
//    gettimeofday(&t_start,0);
//#endif

    unsigned int n_tracks = m_signal_manager.isClosed() ? 0 : tracks();
    bool update_pixmap = false;

    m_layer_rop[LAYER_SIGNAL] = CopyROP;
    m_layer_rop[LAYER_SELECTION] = XorROP;
    m_layer_rop[LAYER_MARKERS] = XorROP;

    m_width = QWidget::width();
    m_height = QWidget::height();

//     qDebug("SignalWidget::paintEvent(): width=%d, height=%d",m_width,m_height);

    // --- detect size changes and refresh the whole display ---
    if ((m_width != m_last_width) || (m_height != m_last_height)) {
//	qDebug("SignalWidget::paintEvent(): window size changed from "
//	      "%dx%d to %dx%d",lastWidth,lastHeight,m_width,m_height);
	for (int i=0; i<3; i++) {
	    if (m_layer[i]) delete m_layer[i];
	    m_layer[i] = 0;
	    m_update_layer[i] = true;
	}
	if (m_pixmap) delete m_pixmap;
	m_pixmap = 0;
	update_pixmap = true;

	m_last_width = m_width;
	m_last_height = m_height;

	// check and correct m_zoom and m_offset
	setZoom(m_zoom);
    }

    // --- repaint of the signal layer ---
    if ( m_update_layer[LAYER_SIGNAL] || !m_layer[LAYER_SIGNAL]) {
	if (!m_layer[LAYER_SIGNAL])
	     m_layer[LAYER_SIGNAL] = new QPixmap(size());
	Q_ASSERT(m_layer[LAYER_SIGNAL]);
	if (!m_layer[LAYER_SIGNAL]) return;

// 	qDebug("SignalWidget::paintEvent(): - redraw of signal layer -");

	// all black if empty
 	if (!n_tracks) m_layer[LAYER_SIGNAL]->fill(black);

	int track_height = (n_tracks) ? (m_height / n_tracks) : 0;
	int top = 0;
	for (unsigned int i = 0; i < n_tracks; i++) {
	    if (i >= m_track_pixmaps.count()) break; // closed or not ready
	    TrackPixmap *pix = m_track_pixmaps.at(i);
	    if (!pix) continue; // signal closed ?

	    // fix the width and height of the track pixmap
	    if ((pix->width() != m_width) || (pix->height() != track_height)) {
		pix->resize(m_width, track_height);
	    }
	    if (pix->isModified()) {
//		qDebug("SignalWidget::paintEvent(): track %d",i); // ###
		pix->repaint();
	    }

	    bitBlt(m_layer[LAYER_SIGNAL], 0, top,
		pix, 0, 0, m_width, track_height, CopyROP);

	    top += track_height;
	}

	m_update_layer[LAYER_SIGNAL] = false;
	update_pixmap = true;
    }

    // --- repaint of the markers layer ---
    if ( m_update_layer[LAYER_MARKERS] || !m_layer[LAYER_MARKERS] ) {
	if (!m_layer[LAYER_MARKERS])
	     m_layer[LAYER_MARKERS] = new QPixmap(size());
	Q_ASSERT(m_layer[LAYER_MARKERS]);
	if (!m_layer[LAYER_MARKERS]) return;

//	qDebug("SignalWidget::paintEvent(): - redraw of markers layer -");
	m_layer[LAYER_MARKERS]->fill(black);

// 	p.begin(m_layer[LAYER_MARKERS]);
	// ### nothing to do yet
// 	p.end();

	m_update_layer[LAYER_MARKERS] = false;
	update_pixmap = true;
    }

    // --- repaint of the selection layer ---
    if (( m_update_layer[LAYER_SELECTION] || !m_layer[LAYER_SELECTION] )) {
	if (!m_layer[LAYER_SELECTION])
	    m_layer[LAYER_SELECTION] = new QPixmap(size());
	Q_ASSERT(m_layer[LAYER_SELECTION]);
	if (!m_layer[LAYER_SELECTION]) return;

// 	qDebug("SignalWidget::paintEvent(): - redraw of selection layer -");

	m_layer[LAYER_SELECTION]->fill(black);

	QPainter p;
 	p.begin(m_layer[LAYER_SELECTION]);

	if (n_tracks) {
	    unsigned int left  = m_signal_manager.selection().first();
	    unsigned int right = m_signal_manager.selection().last();
	    if ((right > m_offset) && (left < m_offset+pixels2samples(m_width))) {
		// transform to pixel coordinates
		if (left < m_offset) left = m_offset;
		left  = samples2pixels(left - m_offset);
		right = samples2pixels(right - m_offset);

		if (right >= (unsigned int)(m_width)) right=m_width-1;
		if (left > right) left = right;

		if (left == right) {
		    p.setPen (green);
		    p.drawLine(left, 0, left, m_height);
		} else {
		    p.setBrush(yellow);
		    p.drawRect(left, 0, right-left+1, m_height);
		}
	    }
	}
	p.end();

	m_update_layer[LAYER_SELECTION] = false;
	update_pixmap = true;
    }

    // --- re-create the buffer pixmap if it has been deleted ---
    if (!m_pixmap) {
	m_pixmap = new QPixmap(size());
	Q_ASSERT(m_pixmap);
	if (!m_pixmap) return;
	update_pixmap = true;
    }
    Q_ASSERT(m_pixmap->width() == m_width);
    Q_ASSERT(m_pixmap->height() == m_height);

    // bitBlt all layers together
    if (update_pixmap) {
	m_pixmap->fill(black);
	for (int i=0; i < 3; i++) {
	    if (!m_layer[i]) continue;
	    bitBlt(m_pixmap, 0, 0, m_layer[i], 0, 0,
		m_width, m_height, m_layer_rop[i]);
	}
	m_last_playpointer = -2;
    }

    // --- redraw the playpointer if a signal is present ---
    m_playpointer = samples2pixels(
	m_signal_manager.playbackController().currentPos() - m_offset);

    if (n_tracks) {
	QPainter p;
	p.begin(m_pixmap);
	p.setPen(yellow);
	p.setRasterOp(XorROP);

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

	p.end();
    }

    bitBlt(this, 0, 0, m_pixmap, 0, 0, m_width, m_height, CopyROP);

//#ifdef DEBUG
//    gettimeofday(&t_end,0);
//    t_elapsed = ((double)t_end.tv_sec*1.0E6+(double)t_end.tv_usec -
//	((double)t_start.tv_sec*1.0E6+(double)t_start.tv_usec)) * 1E-3;
//    qDebug("SignalWidget::paintEvent() -- done, t=%0.3fms --",
//	t_elapsed); // ###
//#endif
}

//***************************************************************************
unsigned int SignalWidget::ms2samples(double ms)
{
    return (unsigned int)rint(ms * m_signal_manager.rate() / 1E3);
}

//***************************************************************************
double SignalWidget::samples2ms(unsigned int samples)
{
    double rate = m_signal_manager.rate();
    if (rate == 0.0) return 0.0;
    return (double)samples * 1E3 / rate;
}

//***************************************************************************
unsigned int SignalWidget::pixels2samples(int pixels)
{
    if ((pixels < 0) || (m_zoom <= 0.0)) return 0;
    return (unsigned int)rint((double)pixels * m_zoom);
}

//***************************************************************************
int SignalWidget::samples2pixels(int samples)
{
    if (m_zoom==0.0) return 0;
    return (int)rint((double)samples / m_zoom);
}

//***************************************************************************
int SignalWidget::displaySamples()
{
    return pixels2samples(width()-1);
}

//***************************************************************************
void selectMarkers(const char */*command*/)
{
//    Parser parser(command);
}

//***************************************************************************
LabelType *findMarkerType (const char */*txt*/)
{
//    int cnt = 0;
//    LabelType *act;
//
//    for (act = globals.markertypes.first(); act; act = globals.markertypes.next()) {
//	if (strcmp (act->name, txt) == 0) return act;
//	cnt++;
//    }
//    qWarning("could not find Labeltype %s\n", txt);
    return 0;
}

////***************************************************************************
//void SignalWidget::deleteLabel ()
//{
//    if (!signalmanage) return ;
//
//    Label *tmp;
//    int l = signalmanage->getLMarker();
//    int r = signalmanage->getRMarker();
//
//    for (tmp = labels->first(); tmp; tmp = labels->next()) {
//	int pos = ms2samples(tmp->pos);
//	if ( (pos >= l) && (pos < r) ) {
//	    labels->remove(tmp);
//	    tmp = labels->first();
//	}
//    }
//    refreshLayer(LAYER_MARKERS);
//}
//
////****************************************************************************
//void SignalWidget::loadLabel()
//{
//    labels->clear();    //remove old Label...
//    appendLabel ();
//}
//
////****************************************************************************
//void SignalWidget::appendLabel()
//{
//    QString name = KFileDialog::getOpenFileName (0, "*.label", this);
//    //  if (!name.isNull())
//    //    {
//    //      char *comstr=catString ("loadbatch (",name,")");
//    //      globals.port->putMessage (comstr);
//    //    }
//    refresh ();
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
//	for (tmp = labels->first(); tmp; tmp = labels->next())  //write out labels
//	{
//	    fprintf (out, tmp->getCommand());
//	    fprintf (out, "\n");
//	}
//
//	fclose (out);
//    }
//}
//
////****************************************************************************
//void SignalWidget::addLabel (const char *params)
//{
//    if (signalmanage && markertype) {
//	Parser parser(params);
//	Label *newmark;
//
//	if (parser.countParams() > 0) {
//	    newmark = new Label (params);
//	} else {
//	    double pos = ((double)signalmanage->getLMarker()) * 1000 / signalmanage->getRate();
//	    newmark = new Label (pos, markertype);
//
//	    //should it need a name ?
//	    if (markertype->named) {
//		Dialog *dialog =
//		    DynamicLoader::getDialog ("stringenter", new DialogOperation("Enter name of label :", true));    //create a modal dialog
//
//		if (dialog) {
//		    dialog->show ();
//
//		    if (dialog->result()) {
//			printf ("dialog:%s\n", dialog->getCommand());
//			newmark->setName (dialog->getCommand());
//			delete dialog;
//		    } else {
//			delete newmark;
//			newmark = 0;
//		    }
//		} else {
//		    KMessageBox::message (this, "Error", i18n("Dialog not loaded !"));
//		    delete newmark;
//		    newmark = 0;
//		}
//	    }
//	}
//
//	if (newmark) {
//	    labels->inSort (newmark);
//
//	    refreshLayer(LAYER_MARKERS);
//	}
//    }
//}
//
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
//	if (labels) {
//	    Label *tmp;
//	    int position = 0;
//	    for (tmp = labels->first(); tmp; tmp = labels->next())
//		if (RangeSelected) {
//		    if (tmp->pos < lmarker)
//			if (abs(lmarker - position) >
//			    abs(lmarker - ms2samples(tmp->pos)))
//			    position = ms2samples(tmp->pos);
//		} else if (abs(lmarker - position) >
//			   abs(lmarker - ms2samples(tmp->pos)))
//		    position = ms2samples(tmp->pos);
//
//	    lmarker = position;
//	    position = signalmanage->getLength();
//	    for (tmp = labels->first(); tmp; tmp = labels->next())
//		if (tmp->pos > rmarker)
//		    if (abs(rmarker - position) >
//			abs(rmarker - ms2samples(tmp->pos)))
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
//		out.open (IO_WriteOnly);
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
//
////****************************************************************************
//void SignalWidget::saveBlocks (int bit)
//{
//    if (signalmanage) {
//	Dialog *dialog =
//	    DynamicLoader::getDialog("saveblock", new DialogOperation(&globals, signalmanage->getRate(), 0, 0));
//
//	if ((dialog) && (dialog->exec())) {
//	    Parser parser (dialog->getCommand());
//
//	    const char *filename = parser.getFirstParam();
//	    QDir *savedir = new QDir (parser.getNextParam());
//
//	    LabelType *start = findMarkerType(parser.getNextParam());
//	    LabelType *stop = findMarkerType (parser.getNextParam());
//
//	    Label *tmp;
//	    Label *tmp2;
//	    int count = 0;
//	    int l = signalmanage->getLMarker();    //save old marker positions...
//	    int r = signalmanage->getRMarker();    //
//
//	    for (tmp = labels->first(); tmp; tmp = labels->next())  //traverse list of labels
//	    {
//		if (tmp->getType() == start) {
//		    for (tmp2 = tmp; tmp2; tmp2 = labels->next())  //traverse rest of list to find next stop marker
//			if (tmp2->getType() == stop) {
//			    char buf[256];
//			    snprintf (buf, sizeof(buf), "%s%04d.wav", filename, count);
//			    //lets hope noone tries to save more than 10000 blocks...
//
//			    signalmanage->setRange (tmp->pos, tmp2->pos);    //changes don't have to be visible...
//			    filename = savedir->absFilePath(buf);
//			    signalmanage->save (filename, bit, true);     //save selected range...
//			    count++;
//			    break;
//			}
//		}
//	    }
//	    signalmanage->setRange (l, r);
//	}
//    }
//}
//
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
//		if (abs(sam[i]) < level) {
//		    int j = i;
//		    while ((i < len) && (abs(sam[i]) < level)) i++;
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
//
////*****************************************************************************
//void SignalWidget::addLabelType (LabelType *marker)
//{
//    globals.markertypes.append (marker);
//    if (manage) manage->addNumberedMenuEntry ("ID_LABELS_TYPE", (char *)marker->name);
//}
//
////*****************************************************************************
//void SignalWidget::addLabelType (const char *str)
//{
//    LabelType *marker = new LabelType(str);
//    if (marker) addLabelType (marker);
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
void SignalWidget::slotTrackInserted(unsigned int index, Track &track)
{
    // insert a new track into the track pixmap list
    TrackPixmap *pix = new TrackPixmap(track);
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
    connect(pix, SIGNAL(sigModified()), this, SLOT(refreshSignalLayer()));
}

//***************************************************************************
void SignalWidget::slotTrackDeleted(unsigned int index)
{
    // delete the track from the list
    m_track_pixmaps.setAutoDelete(true);
    if (index < m_track_pixmaps.count()) m_track_pixmaps.remove(index);

    // emit the signal sigTrackInserted now, so that the signal widget
    // gets resized if needed, but the new pixmap is still empty
    emit sigTrackDeleted(index);

    refreshSignalLayer();
}

//***************************************************************************
void SignalWidget::slotSamplesInserted(unsigned int /*track*/,
    unsigned int /*offset*/, unsigned int /*length*/)
{
//    qDebug("SignalWidget(): slotSamplesInserted(%u, %u,%u)", track,
//	offset, length);
}

//***************************************************************************
void SignalWidget::slotSamplesDeleted(unsigned int /*track*/,
    unsigned int /*offset*/, unsigned int /*length*/)
{
//    qDebug("SignalManager(): slotSamplesDeleted(%u, %u,%u)", track,
//	offset, length);
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

    MultiTrackReader src;
    const unsigned int first = m_signal_manager.selection().first();
    const unsigned int last  = m_signal_manager.selection().last();
    const double       rate  = m_signal_manager.rate();
    const unsigned int bits  = m_signal_manager.bits();

    m_signal_manager.openMultiTrackReader(src,
	m_signal_manager.selectedTracks(), first, last);

    // create the file info
    FileInfo info;
    info.setLength(last - first + 1);
    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(src.count());

    if (!d->encode(this, src, info)) {
	delete d;
	return;
    }

    // start drag&drop, mode is determined automatically
    InhibitRepaintGuard inhibit(*this);
    UndoTransactionGuard undo(m_signal_manager, i18n("drag and drop"));
    if (d->drag()) {
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
void SignalWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (QUriDrag::canDecode(event) || KwaveFileDrag::canDecode(event)) {
	event->accept(rect());
    } else event->ignore(rect());
}

//***************************************************************************
void SignalWidget::dragLeaveEvent(QDragLeaveEvent *)
{
    setMouseMode(MouseNormal);
}

//***************************************************************************
void SignalWidget::dropEvent(QDropEvent* event)
{
    if (KwaveDrag::canDecode(event)) {
	UndoTransactionGuard undo(m_signal_manager, i18n("drag and drop"));
	MultiTrackReader src;
	MultiTrackWriter dst;
	Signal sig;

	if (KwaveDrag::decode(this, event, sig)) {
	    InhibitRepaintGuard inhibit(*this);
	    unsigned int pos = m_offset + pixels2samples(event->pos().x());
	    unsigned int len = sig.length();

	    /**
	     * @todo after the drop operation: enter the new file info into
	     * the signal manager if our own file was empty
	     */

	    sig.openMultiTrackReader(src, sig.allTracks(), 0, len-1);
	    m_signal_manager.openMultiTrackWriter(dst,
		m_signal_manager.selectedTracks(), Insert,
		pos, pos+len-1);
	    /** @todo add a converter if rate does not match */
	    dst << src;

	    // set selection to the new area where the drop was done
	    selectRange(pos, len);
	} else {
	    qDebug("SignalWidget::dropEvent(%s): failed !", event->format(0));
	    /** @todo abort the current undo transaction */
	}
    } else if (KwaveFileDrag::canDecode(event)) {
	QStringList files;
	KwaveFileDrag::decodeLocalFiles(event, files);
	QStringList::Iterator it;
	for (it = files.begin(); it != files.end(); ++it) {
	    emit sigCommand("open(" + *it + ")");
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
	if (right >= (unsigned int)(m_width)) right=m_width-1;
	if (left > right) left = right;

	r.setLeft(left);
	r.setRight(right);
	event->ignore(r);
    } else if (KwaveDrag::canDecode(event)) {
	// accept if it is decodeable within the
	// current range (if it's outside our own selection)
	event->accept();
    } else if (KwaveFileDrag::canDecode(event)) {
	// file drag
	event->accept(rect());
    } else event->ignore(rect());
}

//***************************************************************************
//***************************************************************************
/* end of src/SignalWidget.cpp */
