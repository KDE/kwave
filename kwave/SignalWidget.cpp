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
#include <math.h>
#include <stdlib.h>

#include <kglobal.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kfiledialog.h>

#include <libkwave/Label.h>
#include <libkwave/LabelList.h>
#include <libkwave/Parser.h>
#include <libkwave/Signal.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/TrackPixmap.h"

#include "SignalWidget.h"
#include "SignalManager.h"
#include "MouseMark.h"
#include "sampleop.h"

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

//***************************************************************************
SignalWidget::SignalWidget(QWidget *parent, MenuManager &menu_manager)
    :QWidget(parent),
    m_signal_manager(this),
    m_refresh_timer(),
    m_track_pixmaps(),
    menu(menu_manager),
    m_playback_controller()
{
//    debug("SignalWidget::SignalWidget()");
    down = false;
    height = 0;
////    labels = 0;
    lastHeight = 0;
    lastplaypointer = -1;
    lastWidth = 0;
    lasty = -1;
    markertype = 0;
    m_offset = 0;
    pixmap = 0;
    playpointer = -1;
    redraw = false;
    select = 0;
    width = 0;
    m_zoom = 0.0;
    zoomy = 1;

    for (int i=0; i < 3; i++) {
	m_layer[i] = 0;
	m_update_layer[i] = true;
	m_layer_rop[i] = CopyROP;
    }

    select = new MouseMark(this);
    ASSERT(select);
    if (!select) return;

    connect(select, SIGNAL(selection(int, int)),
	    this, SLOT(estimateRange(int, int)));
    connect(select, SIGNAL(refresh()),
            this, SLOT(refreshSelection()));
    connect(&m_playback_controller, SIGNAL(sigStartPlayback()),
            this, SLOT(playbackStart()));
    connect(&m_playback_controller, SIGNAL(sigPlaybackStopped()),
            this, SLOT(playbackStopped()));
    connect(&m_playback_controller, SIGNAL(sigPlaybackPos(unsigned int)),
            this, SLOT(updatePlaybackPointer(unsigned int)));

    // connect to the track's signals
    Signal *sig = &(m_signal_manager.signal());

    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track &)),
            this, SLOT(slotTrackInserted(unsigned int, Track &)));

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


//    labels = new LabelList();
//    ASSERT(labels);
//    if (!labels) return;
//    labels->setAutoDelete (true);

//    menu.clearNumberedMenu("ID_LABELS_TYPE");
//    for (LabelType *tmp = globals.markertypes.first(); tmp;
//         tmp = globals.markertypes.next())
//    {
//	menu.addNumberedMenuEntry("ID_LABELS_TYPE", (char *)tmp->name);
//    }
//
//    markertype = globals.markertypes.first();

    setBackgroundColor(black);
    setBackgroundMode(NoBackground); // this avoids flicker :-)
    setMouseTracking(true);

    zoomAll();

//    debug("SignalWidget::SignalWidget(): done.");
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
////    ASSERT(labels);
    ASSERT(select);
    return ( /* labels && */ select );
}

//***************************************************************************
SignalWidget::~SignalWidget()
{
//    debug("SignalWidget::~SignalWidget()");

    close();

    m_refresh_timer.stop();

    if (pixmap == 0) delete pixmap;
    pixmap = 0;

////    if (labels) delete labels;
////    labels = 0;

    if (select) delete select;
    select = 0;

    for (int i=0; i < 3; i++) {
	m_layer[i] = 0;
    }

//    debug("SignalWidget::~SignalWidget(): done");
}

//***************************************************************************
void SignalWidget::saveSignal(const char *filename, int bits,
			      int type, bool selection)
{
    if (type == ASCII) {
	m_signal_manager.exportAscii(filename);
    } else {
	m_signal_manager.save(filename, bits, selection);
    }
}

//***************************************************************************
QBitmap *SignalWidget::overview(unsigned int /*width*/, unsigned int /*height*/)
{
    return 0;
//    return (signalmanage) ?
//	signalmanage->overview(width, height,0,signalmanage->getLength())
//	: 0;
}

//***************************************************************************
void SignalWidget::toggleChannel(int /*channel*/)
{
//    ASSERT(signalmanage);
//    if (signalmanage) signalmanage->toggleChannel(channel);
}

//***************************************************************************
//bool SignalWidget::executeNavigationCommand(const QString &command)
//{
//    if (!signalmanage) return false;
//    if (!command.length()) return false;
//    Parser parser(command);
//
//    if (false) {
//    CASE_COMMAND("zoomin")
//	zoomIn();
//    CASE_COMMAND("zoomout")
//	zoomOut();
//    CASE_COMMAND("zoomrange")
//	zoomRange();
//    CASE_COMMAND("scrollright")
//	setOffset(m_offset + pixels2samples(width / 10));
//	refreshAllLayers();
//    CASE_COMMAND("scrollleft")
//	setOffset(m_offset - pixels2samples(width / 10));
//	refreshAllLayers();
//    CASE_COMMAND("viewnext")
//	setOffset(m_offset + pixels2samples(width));
//	refreshAllLayers();
//    CASE_COMMAND("viewprev")
//	setOffset(m_offset - pixels2samples(width));
//	refreshAllLayers();
//    CASE_COMMAND("selectall")
//	selectRange(0, signalmanage->getLength() - 1);
//    CASE_COMMAND("selectnext")
//	int r = signalmanage->getRMarker();
//	int l = signalmanage->getLMarker();
//	selectRange(r + 1, r + 1 + (r - l));
//    CASE_COMMAND("selectprev")
//	int r = signalmanage->getRMarker();
//	int l = signalmanage->getLMarker();
//	selectRange(l - (r - l) - 1, l - 1);
//    CASE_COMMAND("selecttoleft")
//	int l = 0;
//	int r = signalmanage->getRMarker();
//	selectRange(l, r);
//    CASE_COMMAND("selecttoright")
//	int l = signalmanage->getLMarker();
//	int r = signalmanage->getLength() - 1;
//	selectRange(l, r);
//    CASE_COMMAND("selectvisible")
//	selectRange(m_offset, m_offset + pixels2samples(width) - 1);
//    CASE_COMMAND("selectnone")
//	selectRange(m_offset, m_offset);
//    CASE_COMMAND("selectrange")
//	selectRange();
//    } else return false;
//
//    return true;
//}
//
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
    if (!command.length()) return true;
    Parser parser(command);
    debug("SignalWidget::executeCommand(%s)", command.data());    // ###

    if (false) {
////    CASE_COMMAND("dialog")
////	Parser parser(command);
////	const char *name = parser.getFirstParam();
////	debug("SignalWidget::executeCommand(): loading dialog %s", name);
////	showDialog(name);
//    CASE_COMMAND("refresh")
//	refreshAllLayers();
    CASE_COMMAND("newsignal")
	createSignal(command);
//    } else if (executeNavigationCommand(command)) {
//	return true;
    } else {
	bool res = m_signal_manager.executeCommand(command);
//	selectRange(signalmanage->getLMarker(), signalmanage->getRMarker());
	return res;
    };

    return true;
}

//////**********************************************************
////void SignalWidget::showDialog(const char *name)
////{
////    debug("SignalWidget::showDialog(%s)", name);
////    int length = 0;
////    int rate = 44100;
////    int channels = 0;
////    if (signalmanage) length = signalmanage->getLength ();
////    if (signalmanage) rate = signalmanage->getRate ();
////    if (signalmanage) channels = signalmanage->channels();
////
////    DialogOperation *operation =
////	new DialogOperation(&globals, length, rate, channels, true);
////    ASSERT(operation);
////
////    if (operation) {
////	Dialog *dialog = DynamicLoader::getDialog(name, operation);
////	if (dialog) {
////	    connect(dialog, SIGNAL(command(const char*)),
////		    this, SLOT(forwardCommand(const char *)));
////	    // ### dialog->show();
////	    dialog->exec();
////	    delete dialog;
////	} else debug ("error: could not get dialog !\n");
////
////	delete operation;
////    }
////}

//***************************************************************************
void SignalWidget::playback_startTimer()
{
////    debug("void SignalWidget::playback_startTimer()");
//
//    if (timer == 0) {
//	timer = new QTimer(this);
//	ASSERT(timer);
//
//	if (timer) connect(timer, SIGNAL(timeout()),
//	                   this, SLOT(playback_time()));
//    } else {
//	timer->stop();
//    }
//    if (!timer) return;
//
//    // start a timer that refreshes after each pixel
//    int ms;
//    double rate = signalmanage->getRate();
//    if (rate >= 0) {
//	double samples_per_pixel = pixels2samples(1);
//	double time_per_sample = (double)1.0/rate;
//	double time = samples_per_pixel * time_per_sample;
//	ms = (int)ceil(time*1000);
//	if (ms < 50) ms = 50;
//    } else {
//	ms = 100;
//    }
//
//    // start timer as single-shot
//    timer->start(ms, true);
}

//***************************************************************************
void SignalWidget::selectRange()
{

// >>> should be converted to a v2 plugin <<<

////    ASSERT(signalmanage);
////    if (!signalmanage) return;
////    if (!tracks()) return ;
////
////    int rate = signalmanage->getRate();
////
////    Dialog *dialog = DynamicLoader::getDialog
////		     ("time", new DialogOperation (rate, true));
////    ASSERT(dialog);
////    if (!dialog) return ;
////
////    if ( dialog->exec() != 0 ) {
////	int l = signalmanage->getLMarker();
////
////	Parser parser(dialog->getCommand());
////	int len = ms2samples( parser.toDouble() );
////	int siglen = signalmanage->getLength();
////
////	if ((l + len - 1) >= siglen)
////	    selectRange(l, siglen - 1);    //overflow check
////	else
////	    selectRange(l, l + len - 1);
////    }
}

//***************************************************************************
void SignalWidget::selectRange(int /*left*/, int /*right*/)
{
////    debug("SignalWidget::selectRange(%d,%d)",left,right);
//
//    ASSERT(select);
//    ASSERT(m_zoom);
//    if (select && m_zoom) {
//	if ((left != select->left()) || (right != select->right()))
//	    select->set(left, right);
//    }
//
//    signalmanage->setRange(left, right);
//    estimateRange(left, right);
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
int SignalWidget::getBitsPerSample()
{
//    ASSERT(signalmanage);
//    return (signalmanage) ? signalmanage->getBitsPerSample() : 0;
    return 16;
}

//***************************************************************************
PlaybackController &SignalWidget::playbackController()
{
    return m_playback_controller;
}

//***************************************************************************
SignalManager &SignalWidget::signalManager()
{
    return m_signal_manager;
}

//***************************************************************************
void SignalWidget::createSignal(const char */*str*/)
{
//    closeSignal();
//
//    Parser parser (str);
//
//    int rate = parser.toInt();
//    double ms = parser.toDouble();
//
//    int numsamples = (int)(ms * rate / 1000);
//
//    if (signalmanage) delete signalmanage;
//
//////    ASSERT(labels);
//////    if (labels) labels->clear ();
//    m_offset = 0;
//
//    signalmanage = new SignalManager(numsamples, rate, 1);
//    if (signalmanage) {
//	connectSignal();
//	selectRange(0, 0);
//	zoomAll();
//	emit sigChannelAdded(0);;
//    }
}

//***************************************************************************
void SignalWidget::estimateRange(int l, int r)
{
    emit selectedTimeInfo(samples2ms(r - l + 1));
}

//***************************************************************************
void SignalWidget::setSignal(SignalManager */*sigs*/)
{
//    closeSignal();
//    if (!sigs) return ;
//
//    signalmanage = sigs;
//    m_offset = 0;
//    if ((signalmanage) && (signalmanage->getLength())) {
//	connectSignal();
//	zoomAll();
//	for (unsigned int i=0; i < signalmanage->channels(); i++)
//	    emit sigChannelAdded(i);
//    }
}

//***************************************************************************
void SignalWidget::loadFile(const QString &filename, int type)
{
    // close the previous signal
    close();

    // load a new signal
    m_signal_manager.loadFile(filename, type);
    ASSERT(m_signal_manager.length());
    if (m_signal_manager.isClosed()) {
	warning("SignalWidget::loadFile() failed:"\
		" zero-length or out of memory?");
	close();
	return ;
    }
}

//***************************************************************************
void SignalWidget::close()
{
    // first stop playback
    m_playback_controller.playbackStop();
    m_playback_controller.reset();

    // clear the display
    m_track_pixmaps.setAutoDelete(true);
    m_track_pixmaps.clear();

    // close the signal manager
    m_signal_manager.close();

    down = false; // ###
    m_offset = 0;

    // reset the status, zoom and selection
    selectRange(0, 0);
    zoomAll();
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::setOffset(int new_offset)
{
    m_offset = new_offset;
    fixZoomAndOffset();

    // forward the zoom and offset to all track pixmaps
    unsigned int n_tracks = m_track_pixmaps.count();
    for (unsigned int i = 0; i < n_tracks; i++) {
	TrackPixmap *pix = m_track_pixmaps.at(i);
	ASSERT(pix);
	if (!pix) continue;
	
	pix->setOffset(m_offset);
	pix->setZoom(m_zoom);
    }

    refreshAllLayers();
}

//***************************************************************************
double SignalWidget::getFullZoom()
{
//    if (m_signal_manager.isEmpty()) return 0.0;    // no zoom if no signal

    // example: width = 100 [pixels] and length = 3 [samples]
    //          -> samples should be at positions 0, 49.5 and 99
    //          -> 49.5 [pixels / sample]
    //          -> zoom = 1 / 49.5 [samples / pixel]
    // => full zoom [samples/pixel] = (length-1) / (width-1)
    return (double)(m_signal_manager.length() - 1) / (double)(width - 1);
}

//***************************************************************************
void SignalWidget::setZoom(double new_zoom)
{
    m_zoom = new_zoom;
    fixZoomAndOffset();

    // forward the zoom and offset to all track pixmaps
    unsigned int n_tracks = m_track_pixmaps.count();
    for (unsigned int i = 0; i < n_tracks; i++) {
	TrackPixmap *pix = m_track_pixmaps.at(i);
	ASSERT(pix);
	if (!pix) continue;
	
	pix->setOffset(m_offset);
	pix->setZoom(m_zoom);
    }

    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::fixZoomAndOffset()
{
    double max_zoom;
    double min_zoom;
    int length;
//    double last_zoom = m_zoom;
//    int last_offset = m_offset;

    length = m_signal_manager.length();

    if (!width) return;

    // ensure that m_offset is [0...length-1]
    if (m_offset < 0) m_offset = 0;
    if (m_offset > length - 1) m_offset = length - 1;

    // ensure that the zoom is in a proper range
    max_zoom = getFullZoom();
    min_zoom = (double)MINIMUM_SAMPLES_PER_SCREEN / (double)width;
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
    if ( (m_offset > 0) && (pixels2samples(width - 1) + 1 > length - m_offset)) {
	// there is space after the signal -> move offset left
	m_offset -= pixels2samples(width - 1) + 1 - (length - m_offset);
	if (m_offset < 0) m_offset = 0;
    }

    // if reducing the offset was not enough, zoom in
    if (pixels2samples(width - 1) + 1 > length - m_offset) {
	// there is still space after the signal -> zoom in
	// (this should never happen as the zoom has been limited before)
	m_zoom = max_zoom;
    }

// this made some strange effects, so I disabled it :-[
//    // adjust the zoom factor in order to make a whole number
//    // of samples fit into the current window
//    int samples = pixels2samples(width) + 1;
//    zoom = (double)(samples) / (double)(width - 1);

    // do some final range checking
    if (m_zoom < min_zoom) m_zoom = min_zoom;
    if (m_zoom > max_zoom) m_zoom = max_zoom;

//    if ((m_zoom != last_zoom) || (m_offset != last_offset))
//	emit zoomInfo(m_zoom);
}

//***************************************************************************
void SignalWidget::zoomAll()
{
    setZoom(getFullZoom());
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomNormal()
{
    setOffset(m_offset + pixels2samples(width) / 2);
    setZoom(1.0);
    setOffset(m_offset - pixels2samples(width) / 2);
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomOut()
{
    setOffset(m_offset + pixels2samples(width) / 2);
    setZoom(m_zoom*3);
    setOffset(m_offset - pixels2samples(width) / 2);
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomIn()
{
    setOffset(m_offset + pixels2samples(width) / 2);
    setZoom(m_zoom / 3);
    setOffset(m_offset - pixels2samples(width) / 2);
    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::zoomRange()
{
//    if (!signalmanage) return ;
//
//    int lmarker = signalmanage->getLMarker();
//    int rmarker = signalmanage->getRMarker();
//
//    if (lmarker != rmarker) {
//	setOffset(lmarker);
//	setZoom(((double)(rmarker - lmarker)) / (double)(width - 1));
//	refreshAllLayers();
//    }
}

//***************************************************************************
void SignalWidget::resizeEvent(QResizeEvent *)
{
//    debug("SignalWidget::resizeEvent");
//    int maxofs = pixels2samples(width - 1) + 1;
//    int length = (signalmanage) ? signalmanage->getLength() : 0;
//    emit viewInfo(m_offset, maxofs, length);
}

//***************************************************************************
void SignalWidget::refreshAllLayers()
{
//    debug("SignalWidget::refreshAllLayers()");

    for (int i=0; i < 3; i++) {
	m_update_layer[i] = true;
    }

    fixZoomAndOffset();

//    int rate = m_signal_manager.getRate() : 0;
//    int length = m_signal_manager.getLength() : 0;
//
//    if (rate) emit timeInfo(samples2ms(length));
//    if (rate) emit rateInfo(rate);
//
//    int maxofs = pixels2samples(width - 1) + 1;
//    emit viewInfo(m_offset, maxofs, length);
//    emit lengthInfo(length);
//    emit zoomInfo(m_zoom);

    redraw = true;
    repaint(false);
};

//***************************************************************************
void SignalWidget::refreshLayer(int layer)
{
    ASSERT(layer >= 0);
    ASSERT(layer < 3);
    if ((layer < 0) || (layer >= 3)) return;

    m_update_layer[layer] = true;

    redraw = true;
    repaint(false);
}

//***************************************************************************
//void SignalWidget::timedRepaint()
//{
//    if (m_timer.isActive()) return;
//    m_timer.start(300, true);
//}

//***************************************************************************
void SignalWidget::slot_setOffset(int /*new_offset*/)
{
//    if (new_offset != m_offset) {
//	setOffset(new_offset);
//	refreshAllLayers();
//    }
}

//***************************************************************************
bool SignalWidget::checkPosition(int x)
{
    ASSERT(select);
    if (!select) return false;

    // 2 % of width tolerance
    return select->checkPosition(x, pixels2samples(width / 50));
}

//***************************************************************************
void SignalWidget::mousePressEvent(QMouseEvent *)
{
//    ASSERT(e);
//    ASSERT(select);
//    if (!e) return;
//    if (!select) return;
//
//    // abort if no signal is loaded
//    if (!signalmanage) return;
//
//    // ignore all mouse press event in playback mode
//    if (m_playback_controller.running()) return;
//
//    if (e->button() == LeftButton) {
//	int x = m_offset + pixels2samples(e->pos().x());
//	down = true;
//	(checkPosition(x)) ? select->grep(x) : select->set(x, x);
//    }
//
//    if (e->button() == RightButton) lasty = e->pos().y();
}

//***************************************************************************
void SignalWidget::mouseReleaseEvent(QMouseEvent */*e*/)
{
//    ASSERT(e);
//    ASSERT(select);
//    if (!e) return;
//    if (!select) return;
//
//    // abort if no signal is loaded
//    if (!signalmanage) return;
//
//    // ignore all mouse release events in playback mode
//    if (m_playback_controller.running()) return;
//
//    if (down) {
//	int x = m_offset + pixels2samples(e->pos().x());
////	if (x >= length) x = length - 1;    //check for some bounds
//	if (x < 0) x = 0;
//	select->update(x);
//	selectRange(select->left(), select->right() /* , false */);
//	down = false;
//    }
}

//***************************************************************************
void SignalWidget::mouseMoveEvent( QMouseEvent */*e*/ )
{
//    ASSERT(e);
//    ASSERT(select);
//    if (!e) return;
//    if (!select) return;
//
//    // abort if no signal is loaded
//    if (!signalmanage) return;
//
////    if ( (e->state() == RightButton) && height) {
////	//zooming on y axis... not very useful, will perhaps be replaced by
////	//more useful funcitonality...
////	//also very time consuming, because the hole viewable range of signal
////	//has to be redisplayed with every mousemove...
////	double old = zoomy;
////
////	zoomy += (double (e->pos().y() - lasty)) * 2 / height;
////
////	if (zoomy < 1) zoomy = 1;
////	if (zoomy > 10) zoomy = 10;
////
////	lasty = e->pos().y();
////
////	if (zoomy != old) {
////	    redraw = true;
////	    repaint();
////	}
////    }
//
//    if (down) {
//	// in move mode, a new selection was created or an old one grabbed
//	// this does the changes with every mouse move...
//	int mx = e->pos().x();
//	if (mx < 0) mx = 0;
//	if (mx >= width) mx = width-1;
//	int x = m_offset + pixels2samples(mx);
//	if (x < 0) x = 0;
//	select->update(x);
//	selectRange(select->left(), select->right());
//    } else {
//	//yes, this code gives the nifty cursor change....
//	if (checkPosition(m_offset+pixels2samples(e->pos().x())))
//	    setCursor(sizeHorCursor);
//	else
//	    setCursor(arrowCursor);
//    }
}

//***************************************************************************
void SignalWidget::paintEvent(QPaintEvent *)
{
//    return; //
////    debug("SignalWidget::paintEvent()");
////#ifdef DEBUG
//    static struct timeval t_start;
//    static struct timeval t_end;
//    double t_elapsed;
//    double t_rest_of_system;
//    gettimeofday(&t_start,0);
//
//    t_elapsed = ((double)t_start.tv_sec*1.0E6+(double)t_start.tv_usec -
//	((double)t_end.tv_sec*1.0E6+(double)t_end.tv_usec)) * 1E-3;
//    debug("SignalWidget::paintEvent() -- starting, since last: t=%0.3fms --",
//	t_rest_of_system); // ###
//
//    if (t_rest_of_system < 1000.0) return;
////#endif
	
    unsigned int n_tracks = /*m_signal_manager.isClosed() ? 0 :*/ tracks();
    bool update_pixmap = false;

    m_layer_rop[LAYER_SIGNAL] = CopyROP;
    m_layer_rop[LAYER_SELECTION] = XorROP;
    m_layer_rop[LAYER_MARKERS] = XorROP;

    width = QWidget::width();
    height = QWidget::height();

//    debug("SignalWidget::paintEvent(): width=%d, height=%d",width,height);

    // --- detect size changes and refresh the whole display ---
    if ((width != lastWidth) || (height != lastHeight)) {
//	debug("SignalWidget::paintEvent(): window size changed");
	for (int i=0; i<3; i++) {
	    if (m_layer[i]) delete m_layer[i];
	    m_layer[i] = 0;
	    m_update_layer[i] = true;
	}
	if (!pixmap) delete pixmap;
	pixmap = 0;
	update_pixmap = true;
	
	lastWidth = width;
	lastHeight = height;
    }

    // --- repaint of the signal layer ---
    if ( m_update_layer[LAYER_SIGNAL] || !m_layer[LAYER_SIGNAL]) {
	if (!m_layer[LAYER_SIGNAL])
	     m_layer[LAYER_SIGNAL] = new QPixmap(size());
	ASSERT(m_layer[LAYER_SIGNAL]);
	if (!m_layer[LAYER_SIGNAL]) return;
	
//	debug("SignalWidget::paintEvent(): - redraw of signal layer -");
	p.begin(m_layer[LAYER_SIGNAL]);
	p.setRasterOp(CopyROP);
	
	// check and correct m_zoom and m_offset
	fixZoomAndOffset();
	
	int track_height = (n_tracks) ? (height / n_tracks) : 0;
	int top = 0;
	for (unsigned int i = 0; i < n_tracks; i++) {
	    TrackPixmap *pix = m_track_pixmaps.at(i);
	    ASSERT(pix);
	    if (!pix) continue;
	
	    // fix the width and height of the track pixmap
	    if ((pix->width() != width) || (pix->height() != track_height)) {
		pix->resize(width, track_height);
	    }
	    if (pix->isModified()) pix->repaint();
	
//	    debug("SignalWidget::paintEvent(): redrawing track %d",i); // ###
	    bitBlt(m_layer[LAYER_SIGNAL], 0, top,
		pix, 0, 0, width, track_height, CopyROP);
	
	    top += track_height;
	}
	
	p.end();

	m_update_layer[LAYER_SIGNAL] = false;
	update_pixmap = true;
    }

    // --- repaint of the markers layer ---
    if ( m_update_layer[LAYER_MARKERS] || !m_layer[LAYER_MARKERS] ) {
	if (!m_layer[LAYER_MARKERS])
	     m_layer[LAYER_MARKERS] = new QPixmap(size());
	ASSERT(m_layer[LAYER_MARKERS]);
	if (!m_layer[LAYER_MARKERS]) return;
	
//	debug("SignalWidget::paintEvent(): - redraw of markers layer -");
	p.begin(m_layer[LAYER_MARKERS]);
	p.fillRect(0, 0, width, height, black);
	
	// ### nothing to do yet
	
//	p.flush();
	p.end();
	
	m_update_layer[LAYER_MARKERS] = false;
	update_pixmap = true;
    }

    // --- repaint of the selection layer ---
    if (( m_update_layer[LAYER_SELECTION] || !m_layer[LAYER_SELECTION] )) {
	if (!m_layer[LAYER_SELECTION])
	    m_layer[LAYER_SELECTION] = new QPixmap(size());
	ASSERT(m_layer[LAYER_SELECTION]);
	if (!m_layer[LAYER_SELECTION]) return;

//	debug("SignalWidget::paintEvent(): - redraw of selection layer -");
	
	p.begin(m_layer[LAYER_SELECTION]);
	p.fillRect(0, 0, width, height, black);
	p.setRasterOp(CopyROP);
	
	if (select && n_tracks) {
	    int left  = select->left();
	    int right = select->right();
	    if ((right > m_offset) && (left < m_offset+pixels2samples(width))) {
		// transform to pixel coordinates
		left  = samples2pixels(left - m_offset);
		right = samples2pixels(right - m_offset);
		
		if (left < 0) left = 0;
		if (right >= width) right = width-1;
		if (left > right) left = right;
		
		if (left == right) {
		    p.setPen (green);
		    p.drawLine(left, 0, left, height);
		} else {
		    p.setBrush(yellow);
		    p.drawRect(left, 0, right-left+1, height);
		}
	    }
	}
	p.end();
	
	m_update_layer[LAYER_SELECTION] = false;
	update_pixmap = true;
    }

    // --- re-create the buffer pixmap if it has been deleted ---
    if (!pixmap) {
	pixmap = new QPixmap(size());
	ASSERT(pixmap);
	if (!pixmap) return;
	update_pixmap = true;
    }

    if (update_pixmap) {
	for (int i=0; i < 3; i++) {
	    if (!m_layer[i]) continue;
	    bitBlt(pixmap, 0, 0, m_layer[i], 0, 0,
		width, height, m_layer_rop[i]);
	}
	lastplaypointer = -2;
    }

    // --- redraw the playpointer if a signal is present ---
    playpointer = samples2pixels(
	m_playback_controller.currentPos() - m_offset);

    if (n_tracks) {
	p.begin(pixmap);
	p.setPen(yellow);
	p.setRasterOp(XorROP);
	
	if (lastplaypointer >= 0) p.drawLine(lastplaypointer, 0,
	                                     lastplaypointer, height);
	
	if ( (m_playback_controller.running() ||
	      m_playback_controller.paused() ) &&
	     ((playpointer >= 0) && (playpointer < width)) )
	{
	    p.drawLine(playpointer, 0, playpointer, height);
	    lastplaypointer = playpointer;
	} else {
	    lastplaypointer = -1;
	}
	
	p.end();
    }

    bitBlt(this, 0, 0, pixmap, 0, 0, width, height, CopyROP);

////#ifdef DEBUG
//    gettimeofday(&t_end,0);
//    t_elapsed = ((double)t_end.tv_sec*1.0E6+(double)t_end.tv_usec -
//	((double)t_start.tv_sec*1.0E6+(double)t_start.tv_usec)) * 1E-3;
//    debug("SignalWidget::paintEvent() -- done, t=%0.3fms --",
//	t_elapsed); // ###
////#endif

//    // restart the timer for refreshing the playpointer
//    if (m_playback_controller.running()) playback_startTimer();
}

////below are the methods of class SignalWidget that deal with labels
//#define AUTOKORRWIN 320
////windowsize for autocorellation, propably a little bit too short for
////lower frequencies, but this will get configurable somewhere in another
////dimension or for those of you who can't zap to other dimensions, it will
////be done in future
//
//int findNextRepeat (int *, int);
//int findNextRepeatOctave (int *, int, double = 1.005);
//int findFirstMark (int *, int);
//
//float autotable [AUTOKORRWIN];
//float weighttable[AUTOKORRWIN];

//***************************************************************************
int SignalWidget::ms2samples(double /*ms*/)
{
//    ASSERT(signalmanage);
//    if (!signalmanage) return 0;
//
//    return (int)(ms * signalmanage->getRate() / 1000.0);
    return 0;
}

//***************************************************************************
double SignalWidget::samples2ms(int /*samples*/)
{
//    if (!signalmanage) return 0.0;
//    return (double)samples*1000.0 / (double)signalmanage->getRate();
    return 0.0;
}

//***************************************************************************
int SignalWidget::pixels2samples(int pixels)
{
    return (int)(pixels*m_zoom);
}

//***************************************************************************
int SignalWidget::samples2pixels(int samples)
{
    if (m_zoom==0.0) return 0;
    return (int)(samples / m_zoom);
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
//    warning("could not find Labeltype %s\n", txt);
    return 0;
}
//***************************************************************************
void SignalWidget::signalinserted(int /*start*/, int /*len*/)
{
//    Label *tmp;
//    for (tmp = labels->first(); tmp; tmp = labels->next())
//	if (tmp->pos > start) tmp->pos += len;
//    setRange (start, start + len);
//    refreshAllLayers();
}

//***************************************************************************
void SignalWidget::signaldeleted(int /*start*/, int /*len*/)
{
//    Label *tmp;
//    for (tmp = labels->first(); tmp; tmp = labels->next()) {
//	if ((tmp->pos > start) && (tmp->pos < start + len)) //if Label position is within selected boundaries
//	{
//	    labels->remove ();
//	    tmp = labels->first();
//	}
//	if (tmp->pos >= start + len) tmp->pos -= len;     //if it is greater correct position
//    }
//
//    setRange (start, start);
//    refreshAllLayers();
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
//	out = fopen (name.data(), "w");
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
//		QFile out(name.data());
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
void SignalWidget::playbackStart()
{
//    ASSERT(signalmanage);
//    if (!signalmanage) return;
//
//    unsigned int start = m_playback_controller.currentPos();
//    bool loop = m_playback_controller.loop();
//    bool paused = m_playback_controller.paused();
//
//    if (!paused) {
//	unsigned int l = signalmanage->getLMarker();
//	unsigned int r = signalmanage->getRMarker();
//	
//	// start from left marker or zero if nothing selected
//	start = (l == r) ? 0 : l;
//	m_playback_controller.setStartPos(start);
//    }
//
//    signalmanage->startplay(start, loop);
//    playback_startTimer();
}

//***************************************************************************
void SignalWidget::playbackStopped()
{
//    m_playback_timer->stop();
    repaint(false);
}

//****************************************************************************
void SignalWidget::playback_time()
{
    repaint(false);
}

//***************************************************************************
void SignalWidget::updatePlaybackPointer(unsigned int /*pos*/)
{
//    if (timer && !timer->isActive()) playback_startTimer();
}

//***************************************************************************
void SignalWidget::slotTrackInserted(unsigned int index, Track &track)
{
//    debug("SignalWidget(): slotTrackInserted(%u)", track);

    // insert a new track into the track pixmap list
    TrackPixmap *pix = new TrackPixmap(track);
    ASSERT(pix);
    m_track_pixmaps.insert(index, pix);
    if (!pix) return;

    // connect all signals
    connect(pix, SIGNAL(sigModified()), this, SLOT(refreshSignalLayer()));

    // emit the signal sigTrackInserted now, so that the signal widget
    // gets resized if needed, but the new pixmap is still empty
    emit sigTrackInserted(index);

    // redraw the signal if the track has not already been drawn
    if ((pix->height() <= 0) && (pix->width() <= 0)) {
	pix->setOffset(m_offset);
	pix->setZoom(m_zoom);
	debug("SignalWidget(): slotTrackInserted(): need refresh"); // ###
	refreshLayer(LAYER_SIGNAL);
    }

}

//***************************************************************************
void SignalWidget::slotSamplesInserted(unsigned int track,
    unsigned int offset, unsigned int length)
{
//    debug("SignalWidget(): slotSamplesInserted(%u, %u,%u)", track,
//	offset, length);
}

//***************************************************************************
void SignalWidget::slotSamplesDeleted(unsigned int track,
    unsigned int offset, unsigned int length)
{
//    debug("SignalManager(): slotSamplesDeleted(%u, %u,%u)", track,
//	offset, length);
}

//***************************************************************************
void SignalWidget::slotSamplesModified(unsigned int track,
    unsigned int offset, unsigned int length)
{
//    debug("SignalWidget(): slotSamplesModified(%u, %u,%u)", track,
//	offset, length);

    TrackPixmap *pix = m_track_pixmaps.at(track);
    ASSERT(pix);
    if (!pix) return;
//
//    pix->samplesModified(offset, length);
}

//***************************************************************************
//***************************************************************************
/* end of src/SignalWidget.cpp */
