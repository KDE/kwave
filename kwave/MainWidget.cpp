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

#include "libkwave/KwavePlugin.h" // for some helper functions
#include "libkwave/Parser.h"

#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"

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

// ### static const int tbl_keys[10] = {
// ###     Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0
// ### };

#define MIN_PIXELS_PER_CHANNEL 50

//***************************************************************************
MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent), keys(0), m_slider(0), m_signal_frame(this),
     m_signal_widget(&m_signal_frame),
     frmChannelControls(0),
     m_scrollbar(0), m_lamps()/*, m_speakers()*/, m_last_tracks(0)
{
//    debug("MainWidget::MainWidget()");
    int s[3];

    // create the layout objects
    QGridLayout *topLayout = new QGridLayout(this, 3, 2, 0);
    Q_ASSERT(topLayout);
    if (!topLayout) return;

    QHBoxLayout *signalLayout = new QHBoxLayout();
    Q_ASSERT(signalLayout);
    if (!signalLayout) return;
    topLayout->addLayout(signalLayout, 0, 1);

    // -- frames for the channel controls and the signal --

    // background frame of the channel controls, does the
    // clipping for us
    QFrame *frmChannelsBack = new QFrame(this);
    Q_ASSERT(frmChannelsBack);
    if (!frmChannelsBack) return;
    frmChannelsBack->setFrameStyle(0);
    frmChannelsBack->setFixedWidth(30);

    frmChannelControls = new QFrame(frmChannelsBack);
    Q_ASSERT(frmChannelControls);
    if (!frmChannelControls) return;
    frmChannelControls->setFrameStyle(0);
    frmChannelControls->setFixedWidth(30);

    // background for the SignalWidget, does the clipping
    m_signal_frame.setFrameStyle(0);
    m_signal_frame.setMinimumSize(20, MIN_PIXELS_PER_CHANNEL);
    signalLayout->addWidget(&m_signal_frame, 100, 0);

    // -- multistate widgets for lamps and speakers (channel controls) --

    m_lamps.setAutoDelete(true);
    m_lamps.clear();
    MultiStateWidget *msw =
	new MultiStateWidget(frmChannelControls, 0);
    Q_ASSERT(msw);
    if (!msw) return;
    m_lamps.append(msw);
    s[0] = m_lamps.at(0)->addPixmap("light_on.xpm");
    s[1] = m_lamps.at(0)->addPixmap("light_off.xpm");
    m_lamps.at(0)->setStates(s);

//    m_speakers.setAutoDelete(true);
//    m_speakers.clear();
//    msw = new MultiStateWidget(frmChannelControls, 0, 3);
//    Q_ASSERT(msw);
//    if (!msw) return;
//    m_speakers.append(msw);
//    s[0] = m_speakers.at(0)->addPixmap("rspeaker.xpm");
//    s[1] = m_speakers.at(0)->addPixmap("lspeaker.xpm");
//    s[2] = m_speakers.at(0)->addPixmap("xspeaker.xpm");
//    m_speakers.at(0)->setStates(s);

    // -- accelerator keys for 1...9 --

// ###     keys = new QAccel(this);
// ###     Q_ASSERT(keys);
// ###     if (!keys) return;
// ###     for (int i=0; i < 10; i++) {
// ### 	keys->insertItem(tbl_keys[i]);
// ###     }
// ###     connect(keys, SIGNAL(activated(int)),
// ### 	    this, SLOT(parseKey(int)));

    // -- slider for horizontal scrolling --

    m_slider = new OverViewWidget(m_signal_widget.signalManager(), this);
    Q_ASSERT(m_slider);
    if (!m_slider) return;
    m_slider->setFixedHeight(20/*playbutton->height()*3/4*/);
    topLayout->addWidget(m_slider, 1, 1);

    // -- scrollbar for the signal widget and the channel controls --

    m_scrollbar = new QScrollBar(this);
    Q_ASSERT(m_scrollbar);
    if (!m_scrollbar) return;
    m_scrollbar->setOrientation(QScrollBar::Vertical);
    m_scrollbar->setFixedWidth(m_scrollbar->sizeHint().width());
    m_scrollbar->hide();
    m_scrollbar->setFixedWidth(0);
    signalLayout->addWidget(m_scrollbar,   0, AlignRight);

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

    connect(m_scrollbar, SIGNAL(valueChanged(int)),
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
    connect(&m_signal_widget, SIGNAL(selectedTimeInfo(unsigned int,
            unsigned int, double)),
	    this, SLOT(forwardSelectedTimeInfo(unsigned int,
            unsigned int, double)));
    connect(&m_signal_widget, SIGNAL(sigTrackInserted(unsigned int)),
	    this, SLOT(slotTrackInserted(unsigned int)));
    connect(&m_signal_widget, SIGNAL(sigTrackDeleted(unsigned int)),
	    this, SLOT(slotTrackDeleted(unsigned int)));

    connect(&m_signal_widget, SIGNAL(sigMouseChanged(int)),
	    this, SLOT(forwardMouseChanged(int)));

    refreshChannelControls();

//    debug("MainWidget::MainWidget(): done.");
}

//***************************************************************************
bool MainWidget::isOK()
{
    Q_ASSERT(frmChannelControls);
//    Q_ASSERT(keys); ###
    Q_ASSERT(m_scrollbar);
    Q_ASSERT(m_slider);

    return ( frmChannelControls /* ### && keys */ &&
    	m_scrollbar && m_slider );
}

//***************************************************************************
MainWidget::~MainWidget()
{
    if (keys) delete keys;
    m_lamps.clear();
//    m_speakers.clear();
}

//***************************************************************************
void MainWidget::resizeEvent(QResizeEvent *)
{
    refreshChannelControls();
}

//***************************************************************************
void MainWidget::scrollbarMoved(int newval)
{
    Q_ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    // move the signal and the channel controls
    m_signal_widget.move(0, -newval);
    frmChannelControls->move(0, -newval);
}

//***************************************************************************
void MainWidget::slotTrackInserted(unsigned int /*track*/)
{
    refreshChannelControls();
    emit sigTrackCount(tracks());
}

//***************************************************************************
void MainWidget::slotTrackDeleted(unsigned int /*track*/)
{
    refreshChannelControls();
    emit sigTrackCount(tracks());
}

//***************************************************************************
int MainWidget::loadFile(const KURL &url)
{
    closeSignal();
    int res = m_signal_widget.loadFile(url);
    if (res) closeSignal();
    return res;
}

//***************************************************************************
void MainWidget::closeSignal()
{
    m_signal_widget.close();
    refreshChannelControls();
}

//***************************************************************************
void MainWidget::newSignal(unsigned int samples, double rate,
                           unsigned int bits, unsigned int tracks)
{
    closeSignal();
    m_signal_widget.newSignal(samples, rate, bits, tracks);
}

//***************************************************************************
void MainWidget::forwardZoomChanged(double zoom)
{
    emit sigZoomChanged(zoom);
}

//***************************************************************************
void MainWidget::forwardSelectedTimeInfo(unsigned int offset,
                                         unsigned int length, double rate)
{
    emit selectedTimeInfo(offset, length, rate);
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
	Q_ASSERT(m_lamps.at(i));
	if (m_lamps.at(i)) m_lamps.at(i)->setState(0);
    }
}

//***************************************************************************
void MainWidget::parseKey(int key)
{
    if ((key < 0) || ((unsigned int)key >= tracks()))
	return;
    Q_ASSERT(m_lamps.at(key));
    if (!m_lamps.at(key)) return;
    m_lamps.at(key)->nextState();
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
		if (m_lamps.at(i)) m_lamps.at(i)->setState(0);

	if (parser.command() == "invertchannels")
	    for (unsigned int i = 0; i < tracks(); i++)
		if (m_lamps.at(i)) m_lamps.at(i)->nextState();

	return m_signal_widget.executeCommand(command);
    }

    return true;
}

//***************************************************************************
void MainWidget::refreshChannelControls()
{
    Q_ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    Q_ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    unsigned int channels = tracks();
    int min_height = (max(channels, 1) * MIN_PIXELS_PER_CHANNEL);
    bool need_scrollbar = (m_signal_frame.height() < min_height);
    bool scrollbar_visible = m_scrollbar->isVisible();
    int h = max(min_height, m_signal_frame.height());
    int w = m_signal_frame.width();
    int b = m_scrollbar->sizeHint().width();

    if (need_scrollbar && !scrollbar_visible) {
	// -- show the scrollbar --
	m_scrollbar->setFixedWidth(b);
	m_scrollbar->setValue(0);
	m_scrollbar->show();
	w -= b;
	scrollbar_visible = true;
	debug(" - scrollbar shown");
    } else if (!need_scrollbar && scrollbar_visible) {
	// -- hide the scrollbar --
	m_scrollbar->hide();
	m_scrollbar->setFixedWidth(0);
	w += b;
	scrollbar_visible = false;
	debug(" - scrollbar hidden");
    }

    if (scrollbar_visible) {
	// adjust the limits of the scrollbar
	int min = m_scrollbar->minValue();
	int max = m_scrollbar->maxValue();
	double val = (m_scrollbar->value()-(double)min) / (double)(max-min);
	
	min = 0;
	max = h-m_signal_frame.height();
	m_scrollbar->setRange(min, max);
	m_scrollbar->setValue((int)floor(val * (double)max));
	m_scrollbar->setSteps(1, m_signal_frame.height());
    }

    // resize the signal widget and the frame with the channel controls
    frmChannelControls->resize(frmChannelControls->width(), h);
    if ((m_signal_widget.width() != w) || (m_signal_widget.height() != h)) {
	m_signal_widget.resize(w, h);
    }

    h = frmChannelControls->height();
    int x = (frmChannelControls->width()-20) / 2;
    int y;

    // move the existing lamps and speakers to their new position
    for (unsigned int i = 0; (i < m_last_tracks) && (i < channels); i++) {
	Q_ASSERT(m_lamps.at(i));
//	Q_ASSERT(m_speakers.at(i));
	if (!m_lamps.at(i)) continue;
//	if (!m_speakers.at(i)) continue;

	y = (i*h)/max(channels,1);
	m_lamps.at(i)->setGeometry(x, y+5, 20, 20);
//	m_speakers.at(i)->setGeometry(x, y+25, 20, 20);
    }

    // delete now unused lamps and speakers
    while (m_lamps.count() > channels)
	m_lamps.remove(m_lamps.last());
//    while (m_speakers.count() > channels)
//	m_speakers.remove(m_speakers.last());

    // add lamps+speakers for new channels
    for (unsigned int i = m_last_tracks; i < channels; i++) {
	int s[3];
	MultiStateWidget *msw = new MultiStateWidget(frmChannelControls, i);
	Q_ASSERT(msw);
	if (!msw) continue;

	m_lamps.append(msw);
	Q_ASSERT(m_lamps.at(i));
	if (!m_lamps.at(i)) continue;
	s[0] = m_lamps.at(i)->addPixmap("light_on.xpm");
	s[1] = m_lamps.at(i)->addPixmap("light_off.xpm");
	QObject::connect(
	    m_lamps.at(i), SIGNAL(clicked(int)),
	    &m_signal_widget, SLOT(toggleTrackSelection(int))
	);
	m_lamps.at(i)->setStates(s);

//        msw = new MultiStateWidget(frmChannelControls, 0, 3);
//        Q_ASSERT(msw);
//        if (!msw) continue;
//	m_speakers.append(msw);
//	Q_ASSERT(m_speakers.at(i));
//	if (!m_speakers.at(i)) continue;
//	s[0] = m_speakers.at(i)->addPixmap("rspeaker.xpm");
//	s[1] = m_speakers.at(i)->addPixmap("lspeaker.xpm");
//	s[2] = m_speakers.at(i)->addPixmap("xspeaker.xpm");
//	m_speakers.at(i)->setStates(s);

	y = (i*h)/max(channels,1);
	m_lamps.at(i)->setGeometry(x, y+5, 20, 20);
//	m_speakers.at(i)->setGeometry(x, y+25, 20, 20);
	
	m_lamps.at(i)->show();
//	m_speakers.at(i)->show();
    }

    // set the updated identifiers of the widgets
    for (unsigned int i = 0; i < channels; i++) {
	Q_ASSERT(m_lamps.at(i));
//	Q_ASSERT(m_speakers.at(i));
	if (!m_lamps.at(i)) continue;
//	if (!m_speakers.at(i)) continue;

	m_lamps.at(i)->setNumber(i);
//	m_speakers.at(i)->setNumber(i);
    }

    m_last_tracks = channels;
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
