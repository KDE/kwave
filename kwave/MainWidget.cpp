/***************************************************************************
         MainWidget.cpp  -  main widget of the Kwave TopWidget
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

#include <qaccel.h>
#include <qbitmap.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qscrollbar.h>
#include <qwidget.h>

#include <kapp.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kstatusbar.h>

#include "libkwave/Parser.h"

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"
#include "libgui/KwavePlugin.h" // for some helper functions

#include "SignalWidget.h"
#include "SignalManager.h"
#include "MainWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {

#ifndef max
#define max(x,y) (( x > y ) ? (x) : (y) )
#endif

#ifndef min
#define min(x,y) (( x < y ) ? (x) : (y) )
#endif

// ### static const int tbl_keys[10] = {
// ###     Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0
// ### };

#define MIN_PIXELS_PER_CHANNEL 50

//***************************************************************************
MainWidget::MainWidget(QWidget *parent, MenuManager &manage)
    :QWidget(parent), keys(0), m_slider(0), m_signal_frame(this),
     m_signal_widget(&m_signal_frame, manage),
     menu(manage), frmChannelControls(0),
     scrollbar(0), lamps(), speakers(), lastChannels(0)
{
//    debug("MainWidget::MainWidget()");
    int s[3];

    // create the layout objects
    QGridLayout *topLayout = new QGridLayout(this, 3, 2, 0);
    ASSERT(topLayout);
    if (!topLayout) return;

    QHBoxLayout *signalLayout = new QHBoxLayout();
    ASSERT(signalLayout);
    if (!signalLayout) return;
    topLayout->addLayout(signalLayout, 0, 1);

    // -- frames for the channel controls and the signal --

    // background frame of the channel controls, does the
    // clipping for us
    QFrame *frmChannelsBack = new QFrame(this);
    ASSERT(frmChannelsBack);
    if (!frmChannelsBack) return;
    frmChannelsBack->setFrameStyle(0);
    frmChannelsBack->setFixedWidth(30);

    frmChannelControls = new QFrame(frmChannelsBack);
    ASSERT(frmChannelControls);
    if (!frmChannelControls) return;
    frmChannelControls->setFrameStyle(0);
    frmChannelControls->setFixedWidth(30);

    // background for the SignalWidget, does the clipping
    m_signal_frame.setFrameStyle(0);
    m_signal_frame.setMinimumSize(20, MIN_PIXELS_PER_CHANNEL);
    signalLayout->addWidget(&m_signal_frame, 100, 0);

    // -- multistate widgets for lamps and speakers (channel controls) --

    lamps.setAutoDelete(true);
    lamps.clear();
    MultiStateWidget *msw =
	new MultiStateWidget(frmChannelControls, 0);
    ASSERT(msw);
    if (!msw) return;
    lamps.append(msw);
    s[0] = lamps.at(0)->addPixmap("light_on.xpm");
    s[1] = lamps.at(0)->addPixmap("light_off.xpm");
    lamps.at(0)->setStates(s);

    speakers.setAutoDelete(true);
    speakers.clear();
    msw = new MultiStateWidget(frmChannelControls, 0, 3);
    ASSERT(msw);
    if (!msw) return;
    speakers.append(msw);
    s[0] = speakers.at(0)->addPixmap("rspeaker.xpm");
    s[1] = speakers.at(0)->addPixmap("lspeaker.xpm");
    s[2] = speakers.at(0)->addPixmap("xspeaker.xpm");
    speakers.at(0)->setStates(s);

    // -- accelerator keys for 1...9 --

// ###     keys = new QAccel(this);
// ###     ASSERT(keys);
// ###     if (!keys) return;
// ###     for (int i=0; i < 10; i++) {
// ### 	keys->insertItem(tbl_keys[i]);
// ###     }
// ###     connect(keys, SIGNAL(activated(int)),
// ### 	    this, SLOT(parseKey(int)));

    // -- slider for horizontal scrolling --

    m_slider = new OverViewWidget(this);
    ASSERT(m_slider);
    if (!m_slider) return;
    m_slider->setFixedHeight(20/*playbutton->height()*3/4*/);
    topLayout->addWidget(m_slider, 1, 1);

    // -- scrollbar for the signal widget and the channel controls --

    scrollbar = new QScrollBar(this);
    ASSERT(scrollbar);
    if (!scrollbar) return;
    scrollbar->setOrientation(QScrollBar::Vertical);
    scrollbar->setFixedWidth(scrollbar->sizeHint().width());
    scrollbar->hide();
    scrollbar->setFixedWidth(0);
    signalLayout->addWidget(scrollbar,   0, AlignRight);

    // -- signal widget --

    m_signal_widget.setMinimumSize(100, MIN_PIXELS_PER_CHANNEL);

    // -- do all the geometry management stuff --

    topLayout->addWidget(frmChannelsBack, 0, 0);

    topLayout->setRowStretch(0, 1);
    topLayout->setRowStretch(1, 0);
    topLayout->setRowStretch(2, 0);
    topLayout->setColStretch(0, 0);
    topLayout->setColStretch(1, 1);
    signalLayout->activate();
    topLayout->activate();

    // -- connect all signals from/to the signal widget --

    connect(scrollbar, SIGNAL(valueChanged(int)),
            this, SLOT(scrollbarMoved(int)));

    connect(m_slider, SIGNAL(valueChanged(unsigned int)),
	    &m_signal_widget, SLOT(setOffset(unsigned int)));
    connect(&m_signal_widget, SIGNAL(viewInfo(unsigned int,
	    unsigned int, unsigned int)),
	    m_slider, SLOT(setRange(unsigned int, unsigned int,
	    unsigned int)));

    connect(&m_signal_widget, SIGNAL(sigZoomChanged(double)),
	    this, SLOT(forwardZoomChanged(double)));
    connect(&m_signal_widget, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(forwardCommand(const QString &)));
    connect(&m_signal_widget, SIGNAL(selectedTimeInfo(double)),
	    this, SLOT(forwardSelectedTimeInfo(double)));
    connect(&m_signal_widget, SIGNAL(sigTrackInserted(unsigned int)),
	    this, SLOT(slotTrackInserted(unsigned int)));

    connect(&m_signal_widget, SIGNAL(sigMouseChanged(int)),
	    this, SLOT(forwardMouseChanged(int)));

    refreshChannelControls();
    refreshControls();
//    debug("MainWidget::MainWidget(): done.");
}

//***************************************************************************
bool MainWidget::isOK()
{
    ASSERT(frmChannelControls);
//    ASSERT(keys); ###
    ASSERT(scrollbar);
    ASSERT(m_slider);

    return ( frmChannelControls /* ### && keys */ &&
    	scrollbar && m_slider );
}

//***************************************************************************
MainWidget::~MainWidget()
{
    if (keys) delete keys;
    lamps.clear();
    speakers.clear();
}

//***************************************************************************
void MainWidget::resizeEvent(QResizeEvent *)
{
    refreshChannelControls();
    refreshOverView();
}

//***************************************************************************
void MainWidget::scrollbarMoved(int newval)
{
    ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    // move the signal and the channel controls
    m_signal_widget.move(0, -newval);
    frmChannelControls->move(0, -newval);
}

//***************************************************************************
void MainWidget::slotTrackInserted(unsigned int /*track*/)
{
    if (tracks() == 1) {
	zoomAll();
    }

    refreshChannelControls();
}

//***************************************************************************
void MainWidget::refreshControls()
{
//    bool have_signal = (tracks() != 0);
//
//    // enable/disable all items that depend on having a signal
//    menu.setItemEnabled("@SIGNAL", have_signal);
//
//    // update the list of deletable channels
//    menu.clearNumberedMenu("ID_EDIT_CHANNEL_DELETE");
//    for (unsigned int i = 0; i < tracks(); i++) {
//	QString num = QSstring::num(i);
//	menu.addNumberedMenuEntry("ID_EDIT_CHANNEL_DELETE", buf);
//    }
//
//    // refresh the overview (slider)
//    refreshOverView();
}

//***************************************************************************
void MainWidget::refreshOverView()
{
//    ASSERT(m_slider);
//    if (!m_slider) return;
//
//    QBitmap *overview = m_signal_widget.overview(
//	m_slider->width(), m_slider->height());
//    m_slider->setOverView(overview);
//
//    if (overview) delete overview;
}

//**********************************************************
void MainWidget::saveFile(const QString &filename, unsigned int bits,
                          int type, bool selection)
{
    m_signal_widget.saveFile(filename, bits, type, selection);
}

//***************************************************************************
void MainWidget::loadFile(const QString &filename, int type)
{
    closeSignal();
    m_signal_widget.loadFile(filename, type);
//    refreshControls();
}

//***************************************************************************
void MainWidget::closeSignal()
{
    m_signal_widget.close();
    refreshChannelControls();
    refreshControls();
}

//***************************************************************************
void MainWidget::forwardZoomChanged(double zoom)
{
    emit sigZoomChanged(zoom);
}

//***************************************************************************
void MainWidget::forwardSelectedTimeInfo(double ms)
{
    emit selectedTimeInfo(ms);
}

//***************************************************************************
void MainWidget::forwardMouseChanged(int mode)
{
    emit sigMouseChanged(mode);
}

//***************************************************************************
double MainWidget::zoom()
{
    return m_signal_widget.zoom();
}

//***************************************************************************
void MainWidget::resetChannels()
{
    unsigned int t = tracks();
    for (unsigned int i = 0; i < t; i++) {
	ASSERT(lamps.at(i));
	if (lamps.at(i)) lamps.at(i)->setState(0);
    }
}

//***************************************************************************
void MainWidget::parseKey(int key)
{
    if ((key < 0) || ((unsigned int)key >= tracks()))
	return;
    ASSERT(lamps.at(key));
    if (!lamps.at(key)) return;
    lamps.at(key)->nextState();
}

//***************************************************************************
void MainWidget::forwardCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
bool MainWidget::executeCommand(const QString &command)
{
//    debug("MainWidget::executeCommand(%s)", command);

    if (!command.length()) return false;
    Parser parser(command);

    if (false) {
//    CASE_COMMAND("...")
//	;
    } else {
	if (parser.command() == "selectchannels")
	    for (unsigned int i = 0; i < tracks(); i++)
		if (lamps.at(i)) lamps.at(i)->setState(0);

	if (parser.command() == "invertchannels")
	    for (unsigned int i = 0; i < tracks(); i++)
		if (lamps.at(i)) lamps.at(i)->nextState();

	return m_signal_widget.executeCommand(command);
    }

    return true;
}

//***************************************************************************
void MainWidget::refreshChannelControls()
{
    ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    unsigned int channels = tracks();
    int min_height = (max(channels, 1) * MIN_PIXELS_PER_CHANNEL);
    bool need_scrollbar = (m_signal_frame.height() < min_height);
    bool scrollbar_visible = scrollbar->isVisible();
    int h = max(min_height, m_signal_frame.height());
    int w = m_signal_frame.width();
    int b = scrollbar->sizeHint().width();

    if (need_scrollbar && !scrollbar_visible) {
	// -- show the scrollbar --
	scrollbar->setFixedWidth(b);
	scrollbar->setValue(0);
	scrollbar->show();
	w -= b;
	scrollbar_visible = true;
	debug(" - scrollbar shown");
    } else if (!need_scrollbar && scrollbar_visible) {
	// -- hide the scrollbar --
	scrollbar->hide();
	scrollbar->setFixedWidth(0);
	w += b;
	scrollbar_visible = false;
	debug(" - scrollbar hidden");
    }

    if (scrollbar_visible) {
	// adjust the limits of the scrollbar
	int min = scrollbar->minValue();
	int max = scrollbar->maxValue();
	double val = (scrollbar->value()-(double)min) / (double)(max-min);
	
	min = 0;
	max = h-m_signal_frame.height();
	scrollbar->setRange(min, max);
	scrollbar->setValue(floor(val * (double)max));
	scrollbar->setSteps(1, m_signal_frame.height());
    }

    // resize the signal widget and the frame with the channel controls
    m_signal_widget.resize(w, h);
    frmChannelControls->resize(frmChannelControls->width(), h);

    h = frmChannelControls->height();
    int x = (frmChannelControls->width()-20) / 2;
    int y;

    // move the existing lamps and speakers to their new position
    for (unsigned int i = 0; (i < lastChannels) && (i < channels); i++) {
	ASSERT(lamps.at(i));
	ASSERT(speakers.at(i));
	if (!lamps.at(i)) continue;
	if (!speakers.at(i)) continue;

	y = (i*h)/max(channels,1);
	lamps.at(i)->setGeometry(x, y+5, 20, 20);
	speakers.at(i)->setGeometry(x, y+25, 20, 20);
    }

    // delete now unused lamps and speakers
    while (lamps.count() > channels)
	lamps.remove(lamps.last());
    while (speakers.count() > channels)
	speakers.remove(speakers.last());

    // add lamps+speakers for new channels
    for (unsigned int i = lastChannels; i < channels; i++) {
	int s[3];
	MultiStateWidget *msw = new MultiStateWidget(frmChannelControls, i);
	ASSERT(msw);
	if (!msw) continue;

	lamps.append(msw);
	ASSERT(lamps.at(i));
	if (!lamps.at(i)) continue;
	s[0] = lamps.at(i)->addPixmap("light_on.xpm");
	s[1] = lamps.at(i)->addPixmap("light_off.xpm");
	QObject::connect(
	    lamps.at(i), SIGNAL(clicked(int)),
	    &m_signal_widget, SLOT(toggleChannel(int))
	);
	lamps.at(i)->setStates(s);

        msw = new MultiStateWidget(frmChannelControls, 0, 3);
        ASSERT(msw);
        if (!msw) continue;
	speakers.append(msw);
	ASSERT(speakers.at(i));
	if (!speakers.at(i)) continue;
	s[0] = speakers.at(i)->addPixmap("rspeaker.xpm");
	s[1] = speakers.at(i)->addPixmap("lspeaker.xpm");
	s[2] = speakers.at(i)->addPixmap("xspeaker.xpm");
	speakers.at(i)->setStates(s);

	y = (i*h)/max(channels,1);
	lamps.at(i)->setGeometry(x, y+5, 20, 20);
	speakers.at(i)->setGeometry(x, y+25, 20, 20);
	
	lamps.at(i)->show();
	speakers.at(i)->show();
    }

    // set the updated identifiers of the widgets
    for (unsigned int i = 0; i < channels; i++) {
	ASSERT(lamps.at(i));
	ASSERT(speakers.at(i));
	if (!lamps.at(i)) continue;
	if (!speakers.at(i)) continue;

	lamps.at(i)->setNumber(i);
	speakers.at(i)->setNumber(i);
    }

    lastChannels = channels;
    refreshControls();
}

//***************************************************************************
unsigned int MainWidget::tracks()
{
    return m_signal_widget.tracks();
}

//***************************************************************************
unsigned int MainWidget::bits()
{
    return m_signal_widget.bits();
}

//***************************************************************************
SignalManager &MainWidget::signalManager()
{
    return m_signal_widget.signalManager();
}

//***************************************************************************
PlaybackController &MainWidget::playbackController()
{
    return m_signal_widget.playbackController();
}

//***************************************************************************
//***************************************************************************
