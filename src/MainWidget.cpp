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
#include <kbuttonbox.h>
#include <kstatusbar.h>

#include <libkwave/String.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"
#include "libgui/KwavePlugin.h" // for some helper functions

#include "sampleop.h"
#include "SignalWidget.h"
#include "SignalManager.h"
#include "MainWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (matchCommand(command, x)) {

#ifndef max
#define max(x,y) (( x > y ) ? (x) : (y) )
#endif

#ifndef min
#define min(x,y) (( x < y ) ? (x) : (y) )
#endif

static const int tbl_keys[10] = {
    Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0
};

#define MIN_PIXELS_PER_CHANNEL 50

//***************************************************************************
MainWidget::MainWidget(QWidget *parent, MenuManager &manage,
                       KStatusBar &status)
    :QWidget(parent),
     status(status),
     menu(manage)
{
    debug("MainWidget::MainWidget()");
    int s[3];
    int w;
    int h;
    MultiStateWidget *msw;

    bsize = 0;
//    buttons = 0;
    frmChannelControls = 0;
    frmSignal = 0;
    keys = 0;
//    loopbutton = 0;
    lastChannels = 0;
//    playbutton = 0;
    scrollbar = 0;
    signalview = 0;
    m_slider = 0;

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
    frmSignal = new QFrame(this);
    ASSERT(frmSignal);
    if (!frmSignal) return;
    frmSignal->setFrameStyle(0);
    frmSignal->setMinimumSize(20, MIN_PIXELS_PER_CHANNEL);

    // -- multistate widgets for lamps and speakers (channel controls) --

    lamps.setAutoDelete(true);
    lamps.clear();
    msw = new MultiStateWidget(frmChannelControls, 0);
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

    keys = new QAccel(this);
    ASSERT(keys);
    if (!keys) return;
    for (int i=0; i < 10; i++) {
	keys->insertItem(tbl_keys[i]);
    }
    connect(keys, SIGNAL(activated(int)),
	    this, SLOT(parseKey(int)));

    // -- slider for horizontal scrolling --

    m_slider = new OverViewWidget(this);
    ASSERT(m_slider);
    if (!m_slider) return;

//    // -- buttons for playback and zoom --
//
//    buttons = new QHBoxLayout();
//    ASSERT(buttons);
//    if (!buttons) return;
//
//    // [Play]
//    playbutton = new QPushButton(i18n("Play"), this);
//    ASSERT(playbutton);
//    if (!playbutton) return;
//    playbutton->setAccel(Key_Space);
//
//    // [Loop]
//    loopbutton = new QPushButton(i18n("&Loop"), this);
//    ASSERT(loopbutton);
//    if (!loopbutton) return;
//    loopbutton->setAccel(Key_L);

    // create the layout objects
    QGridLayout *topLayout = new QGridLayout(this, 3, 2, 0);
    ASSERT(topLayout);
    if (!topLayout) return;

    QHBoxLayout *signalLayout = new QHBoxLayout();
    ASSERT(signalLayout);
    if (!signalLayout) return;

    // -- scrollbar for the signal widget and the channel controls --

    scrollbar = new QScrollBar(this);
    ASSERT(scrollbar);
    if (!scrollbar) return;
    scrollbar->setOrientation(QScrollBar::Vertical);
    scrollbar->setFixedWidth(scrollbar->sizeHint().width());
    scrollbar->hide();
    scrollbar->setFixedWidth(0);

    // -- signal widget --

    signalview = new SignalWidget(frmSignal, menu);
    ASSERT(signalview);
    if (!signalview) return;
    if (!signalview->isOK()) {
	warning("MainWidget::MainWidget: failed in creating SignalWidget !");
	delete signalview;
	signalview = 0;
	return;
    }
    signalview->setMinimumSize(100, MIN_PIXELS_PER_CHANNEL);

    // -- do all the geometry management stuff --

//    h = playbutton->sizeHint().height();
//    w = max(playbutton->sizeHint().width(), loopbutton->sizeHint().width());
//    playbutton->setFixedSize(w, h);
//    loopbutton->setFixedSize(w, h);

    topLayout->addWidget(frmChannelsBack, 0, 0);

    topLayout->addLayout(signalLayout, 0, 1);
    signalLayout->addWidget(frmSignal, 1, AlignLeft);
    signalLayout->addWidget(scrollbar, 0, AlignRight);

    m_slider->setFixedHeight(20/*playbutton->height()*3/4*/);
    topLayout->addWidget(m_slider, 1, 1);

//    topLayout->addLayout(buttons, 2, 1);
//    buttons->addWidget(playbutton);
//    buttons->addSpacing(10);
//    buttons->addWidget(loopbutton);
//    buttons->addSpacing(20);
//    buttons->addStretch(10);

    topLayout->setRowStretch(0, 1);
    topLayout->setRowStretch(1, 0);
    topLayout->setRowStretch(2, 0);
    topLayout->setColStretch(0, 0);
    topLayout->setColStretch(1, 1);
    topLayout->activate();

    // -- connect all signals from/to the signal widget --

    connect(scrollbar, SIGNAL(valueChanged(int)),
            this, SLOT(scrollbarMoved(int)));


//    connect(playbutton, SIGNAL(pressed()),
//	    this, SLOT(play()));
//    connect(loopbutton, SIGNAL(pressed()),
//	    this, SLOT(loop()));
//    connect(signalview, SIGNAL(playingfinished()),
//	    this, SLOT(stop()));
//    connect(this, SIGNAL(setOperation(int)),
//	    signalview, SLOT(playback_setOp(int)));


    connect(m_slider, SIGNAL(valueChanged(int)),
	    signalview, SLOT(slot_setOffset(int)));
    connect(signalview, SIGNAL(viewInfo(int, int, int)),
	    m_slider, SLOT(setRange(int, int, int)));
    connect(signalview, SIGNAL(zoomInfo(double)),
	    this, SLOT(forwardZoomChanged(double)));
    connect(signalview, SIGNAL(sigCommand(const char*)),
	    this, SLOT(forwardCommand(const char*)));
    connect(signalview, SIGNAL(selectedTimeInfo(double)),
	    this, SLOT(setSelectedTimeInfo(double)));
    connect(signalview, SIGNAL(rateInfo(int)),
	    this, SLOT(setRateInfo(int)));
    connect(signalview, SIGNAL(lengthInfo(int)),
	    this, SLOT(setLengthInfo(int)));
    connect(signalview, SIGNAL(timeInfo(double)),
	    this, SLOT(setTimeInfo(double)));

    connect(signalview, SIGNAL(signalChanged(int,int)),
            this, SLOT(slot_SignalChanged(int,int)));
    connect(signalview, SIGNAL(sigChannelAdded(unsigned int)),
            this, SLOT(channelAdded(unsigned int)));
    connect(signalview, SIGNAL(sigChannelDeleted(unsigned int)),
            this, SLOT(channelDeleted(unsigned int)));

    refreshChannelControls();
    refreshControls();
    debug("MainWidget::MainWidget(): done.");
}

//*****************************************************************************
bool MainWidget::isOK()
{
//    ASSERT(buttons);
    ASSERT(frmChannelControls);
    ASSERT(frmSignal);
    ASSERT(keys);
//    ASSERT(loopbutton);
//    ASSERT(playbutton);
    ASSERT(scrollbar);
    ASSERT(signalview);
    ASSERT(m_slider);

    return ( /* buttons && */frmChannelControls && frmSignal && keys &&
             /*loopbutton && playbutton && */scrollbar && signalview &&
             m_slider );
}

//*****************************************************************************
MainWidget::~MainWidget()
{
    debug("MainWidget::~MainWidget()");
    ASSERT(KApplication::getKApplication());

    if (signalview) delete signalview;
    signalview = 0;

//    if (buttons) delete buttons;
//    buttons = 0;

    // do not delete the buttons themselfes, the
    // KButtonBox has "auto-deletion" turned on !

    if (keys) delete keys;
    lamps.clear();
    speakers.clear();

    debug("MainWidget::~MainWidget(): done.");
}

//*****************************************************************************
void MainWidget::resizeEvent(QResizeEvent *)
{
    refreshChannelControls();
//    debug("void MainWidget::resizeEvent(QResizeEvent *)"); // ###
//    refreshOverView();
}

//*****************************************************************************
void MainWidget::scrollbarMoved(int newval)
{
    ASSERT(signalview);
    ASSERT(frmChannelControls);
    if (!signalview) return;
    if (!frmChannelControls) return;

    // move the signal and the channel controls
    signalview->move(0, -newval);
    frmChannelControls->move(0, -newval);
}

//*****************************************************************************
void MainWidget::refreshControls()
{
    bool have_signal = (getChannelCount() != 0);

    // enable/disable all items that depend on having a signal
    menu.setItemEnabled("@SIGNAL", have_signal);

    // update the list of deletable channels
    menu.clearNumberedMenu("ID_EDIT_CHANNEL_DELETE");
    for (unsigned int i = 0; i < getChannelCount(); i++) {
	char buf[64];
	snprintf(buf, sizeof(buf), "%d", i);
	menu.addNumberedMenuEntry("ID_EDIT_CHANNEL_DELETE", buf);
    }

    // refresh the overview (slider)
    refreshOverView();
}

//***************************************************************************
void MainWidget::refreshOverView()
{
    ASSERT(m_slider);
    ASSERT(signalview);
    if (!m_slider) return;
    if (!signalview) return;

    QBitmap *overview = signalview->overview(
	m_slider->width(), m_slider->height());

    m_slider->setOverView(overview);

    if (overview) delete overview;
}

//**********************************************************
void MainWidget::saveSignal(const char *filename, int bits,
                            int type, bool selection)
{
    ASSERT(signalview);
    if (signalview) signalview->saveSignal(filename, bits, type, selection);
}

//*****************************************************************************
void MainWidget::setSignal (const char *filename, int type)
{
    ASSERT(signalview);

    closeSignal();
    if (signalview) signalview->setSignal(filename, type);
    refreshControls();
}

//*****************************************************************************
void MainWidget::setSignal(SignalManager *signal)
{
    ASSERT(signalview);

    closeSignal();
    if (signalview) signalview->setSignal(signal);
    refreshControls();
}

//*****************************************************************************
void MainWidget::closeSignal()
{
    ASSERT(signalview);

    if (signalview) signalview->closeSignal();

    setTimeInfo(0);
    setSelectedTimeInfo(0);
    setLengthInfo(0);
    setRateInfo(0);

    refreshChannelControls();
    refreshControls();
}

//*****************************************************************************
void MainWidget::setRateInfo(int rate)
{
    char buf[128];
    snprintf(buf, sizeof(buf), i18n("Rate: %d.%d kHz"), 
	rate / 1000, (rate % 1000) / 100);
    status.changeItem(buf, 2);
}

//*****************************************************************************
void MainWidget::setLengthInfo(int len)
{
    char buf[128];
    snprintf(buf, sizeof(buf), i18n("Samples: %d"), len);
    status.changeItem(buf, 3);
}

//*****************************************************************************
void MainWidget::forwardZoomChanged(double zoom)
{
    emit sigZoomChanged(zoom);
}

//*****************************************************************************
double MainWidget::zoom()
{
    return ( (signalview) ? signalview->zoom() : 0 );
}

//*****************************************************************************
void MainWidget::setZoom(double new_zoom)
{
    if (signalview) {
	if (signalview->zoom() == new_zoom) return; // nothing to do

	signalview->setZoom(new_zoom);
	signalview->refreshAllLayers();
    }
}

//*****************************************************************************
void MainWidget::zoomRange()
{
    if (signalview) signalview->zoomRange();
}

//*****************************************************************************
void MainWidget::zoomIn()
{
    if (signalview) signalview->zoomIn();
}

//*****************************************************************************
void MainWidget::zoomOut()
{
    if (signalview) signalview->zoomOut();
}

//*****************************************************************************
void MainWidget::zoomAll()
{
    if (signalview) signalview->zoomAll();
}

//*****************************************************************************
void MainWidget::zoomNormal()
{
    if (signalview) signalview->zoomNormal();
}

//*****************************************************************************
void MainWidget::slot_SignalChanged(int left, int right)
{
    debug("MainWidget::slot_SignalChanged(%d, %d)", left, right);
    if (m_slider) refreshOverView();
}

//*****************************************************************************
void MainWidget::channelAdded(unsigned int channel)
{
    debug("MainWidget::channelAdded(%u)", channel);
    refreshChannelControls();
}

//*****************************************************************************
void MainWidget::channelDeleted(unsigned int channel)
{
    debug("MainWidget::channelDeleted(%u)", channel);
    lamps.remove(channel);
    speakers.remove(channel);
    refreshChannelControls();
}

//*****************************************************************************
void MainWidget::resetChannels()
{
    int channels = getChannelCount();
    for (int i = 0; i < channels; i++) {
	ASSERT(lamps.at(i));
	if (lamps.at(i)) lamps.at(i)->setState(0);
    }
}

//*****************************************************************************
void MainWidget::parseKey(int key)
{
    if ((key < 0) || ((unsigned int)key >= getChannelCount()))
	return;
    ASSERT(lamps.at(key));
    if (!lamps.at(key)) return;
    lamps.at(key)->nextState();
    emit setOperation(TOGGLECHANNEL + key);
}

//****************************************************************************
void MainWidget::forwardCommand(const char *command)
{
    ASSERT(command);
    if (!command) return;
//    debug("MainWidget::forwardCommand(%s)", command);
    emit sigCommand(command);
}

//*****************************************************************************
bool MainWidget::executeCommand(const char *command)
{
    ASSERT(command);
//    debug("MainWidget::executeCommand(%s)", command);
    if (!command) return false;

    if (false) {
    CASE_COMMAND("zoomrange")
	if (signalview) signalview->zoomRange();
    } else {
	if (matchCommand(command, "selectchannels"))
	    for (unsigned int i = 0; i < getChannelCount(); i++)
		if (lamps.at(i)) lamps.at(i)->setState(0);

	if (matchCommand(command, "invertchannels"))
	    for (unsigned int i = 0; i < getChannelCount(); i++)
		if (lamps.at(i)) lamps.at(i)->nextState();

	bool result = (signalview)?signalview->executeCommand(command):false;
	if (signalview) signalview->refreshAllLayers();
	return result;
    }

    return true;
}

////*****************************************************************************
//void MainWidget::loop()
//{
//    ASSERT(playbutton);
//    ASSERT(loopbutton);
//    if (!playbutton) return;
//    if (!loopbutton) return;
//
//    emit setOperation(LOOP);
//    playbutton->setText(i18n("Stop"));
//    loopbutton->setText(i18n("Halt"));     // halt feature by gerhard Zint
//    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
//    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
//    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
//    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
//}
//
////*****************************************************************************
//void MainWidget::play ()
//{
//    ASSERT(playbutton);
//    ASSERT(loopbutton);
//    if (!playbutton) return;
//    if (!loopbutton) return;
//
//    emit setOperation (PLAY);
//    playbutton->setText (i18n("Stop"));
//    loopbutton->setText (i18n("Halt"));     // halt feature by gerhard Zint
//    this->disconnect (playbutton, SIGNAL(pressed()), this, SLOT(play()));
//    this->disconnect (loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
//    this->connect (playbutton, SIGNAL(pressed()), this, SLOT(stop()));
//    this->connect (loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
//}
//
////*****************************************************************************
//void MainWidget::halt ()
//{
//    ASSERT(playbutton);
//    ASSERT(loopbutton);
//    if (!playbutton) return;
//    if (!loopbutton) return;
//
//    playbutton->setText(i18n("Play"));
//    loopbutton->setText(i18n("&Loop"));
//    loopbutton->setAccel(Key_L);    //seems to neccessary
//
//    emit setOperation (PHALT);
//
//    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
//    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
//    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
//    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
//}
//
////*****************************************************************************
//void MainWidget::stop ()
//{
//    ASSERT(playbutton);
//    ASSERT(loopbutton);
//    if (!playbutton) return;
//    if (!loopbutton) return;
//
//    playbutton->setText(i18n("Play"));
//    loopbutton->setText(i18n("&Loop"));
//    loopbutton->setAccel(Key_L);    //seems to be neccessary
//
//    emit setOperation(PSTOP);
//
//    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
//    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
//    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
//    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
//}

//*****************************************************************************
void MainWidget::setSelectedTimeInfo(double ms)
{
    char buffer[128];
    char ms_string[64];

    KwavePlugin::ms2string(ms_string, sizeof(ms_string), ms);
    snprintf(buffer, sizeof(buffer), i18n("selected: %s"), ms_string);
    status.changeItem(buffer, 4);
}

//*****************************************************************************
void MainWidget::setTimeInfo(double ms)
{
    char buffer[128];
    char ms_string[64];
    KwavePlugin::ms2string(ms_string, sizeof(ms_string), ms);
    snprintf(buffer, sizeof(buffer), i18n("Length: %s"), ms_string);
    status.changeItem(buffer, 1);
}

//*****************************************************************************
void MainWidget::refreshChannelControls()
{
    ASSERT(frmChannelControls);
    if (!frmChannelControls) return;

    ASSERT(frmSignal);
    ASSERT(frmChannelControls);
    ASSERT(signalview);
    if (!frmSignal) return;
    if (!frmChannelControls) return;
    if (!signalview) return;

    unsigned int channels = getChannelCount();
    int min_height = (max(channels, 1) * MIN_PIXELS_PER_CHANNEL);
    bool need_scrollbar = (frmSignal->height() < min_height);
    bool scrollbar_visible = scrollbar->isVisible();
    int h = max(min_height, frmSignal->height());
    int w = frmSignal->width();
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
	max = h-frmSignal->height();
	scrollbar->setRange(min, max);
	scrollbar->setValue(floor(val * (double)max));
	scrollbar->setSteps(1, frmSignal->height());
    }

    // resize the signal widget and the frame with the channel controls
    signalview->resize(w, h);
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
	    signalview, SLOT(toggleChannel(int))
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

//*****************************************************************************
unsigned int MainWidget::getChannelCount()
{
    ASSERT(signalview);
    return (signalview) ? signalview->getChannelCount() : 0;
}

//*****************************************************************************
int MainWidget::getBitsPerSample()
{
    ASSERT(signalview);
    return (signalview) ? signalview->getBitsPerSample() : 0;
}

//*****************************************************************************
SignalManager *MainWidget::getSignalManager()
{
    return (signalview ? signalview->getSignalManager() : 0);
}

//*****************************************************************************
void MainWidget::playbackStart()
{
    ASSERT(signalview);
    if (!signalview) return;
    debug("void MainWidget::playbackStart()");
    signalview->playbackStart();
}

//*****************************************************************************
void MainWidget::playbackLoop()
{
    ASSERT(signalview);
    if (!signalview) return;
    debug("void MainWidget::playbackLoop()");
    signalview->playbackLoop();
}

//*****************************************************************************
void MainWidget::playbackPause()
{
    ASSERT(signalview);
    if (!signalview) return;
    debug("void MainWidget::playbackPause()");
    signalview->playbackPause();
}

//*****************************************************************************
void MainWidget::playbackContinue()
{
    ASSERT(signalview);
    if (!signalview) return;
    debug("void MainWidget::playbackContinue()");
    signalview->playbackContinue();
}

//*****************************************************************************
void MainWidget::playbackStop()
{
    ASSERT(signalview);
    if (!signalview) return;
    debug("void MainWidget::playbackStop()");
    signalview->playbackStop();
}

//*****************************************************************************
//*****************************************************************************
