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

#include <QBitmap>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include <klocale.h>

#include "libkwave/KwavePlugin.h" // for some helper functions
#include "libkwave/Parser.h"
#include "libkwave/SignalManager.h"

#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"
#include "libgui/SignalWidget.h"

#include "ShortcutWrapper.h"
#include "MainWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {

/** table of keyboard shortcuts 0...9 */
static const int tbl_keys[10] = {
    Qt::Key_1,
    Qt::Key_2,
    Qt::Key_3,
    Qt::Key_4,
    Qt::Key_5,
    Qt::Key_6,
    Qt::Key_7,
    Qt::Key_8,
    Qt::Key_9,
    Qt::Key_0
};

#define MIN_PIXELS_PER_CHANNEL 50

//***************************************************************************
MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent), m_slider(0), m_signal_frame(this),
     m_signal_widget(&m_signal_frame),
     m_frm_channel_controls(0),
     m_scrollbar(0), m_lamps(), m_last_tracks(0)
{
//    qDebug("MainWidget::MainWidget()");

    // create the layout objects
    QGridLayout* topLayout = new QGridLayout(this);
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

    m_frm_channel_controls = new QFrame(frmChannelsBack);
    Q_ASSERT(m_frm_channel_controls);
    if (!m_frm_channel_controls) return;
    m_frm_channel_controls->setFrameStyle(0);
    m_frm_channel_controls->setFixedWidth(30);

    // background for the SignalWidget, does the clipping
    m_signal_frame.setFrameStyle(0);
    m_signal_frame.setMinimumSize(20, MIN_PIXELS_PER_CHANNEL);
    signalLayout->addWidget(&m_signal_frame, 100, 0);

    // -- accelerator keys for 1...9 --
    for (int i=0; i < 10; i++) {
	Kwave::ShortcutWrapper *shortcut =
	    new Kwave::ShortcutWrapper(this, tbl_keys[i], i);
	connect(shortcut, SIGNAL(activated(int)),
	        this, SLOT(parseKey(int)));
    }

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
    m_scrollbar->setOrientation(Qt::Vertical);
    m_scrollbar->setFixedWidth(m_scrollbar->sizeHint().width());
    m_scrollbar->hide();
    m_scrollbar->setFixedWidth(0);
    signalLayout->addWidget(m_scrollbar, 0, Qt::AlignRight);

    // -- signal widget --

    m_signal_widget.setMinimumSize(100, MIN_PIXELS_PER_CHANNEL);

    // -- do all the geometry management stuff --

    topLayout->addWidget(frmChannelsBack, 0, 0);

    topLayout->setRowStretch(0, 1);
    topLayout->setRowStretch(1, 0);
    topLayout->setRowStretch(2, 0);
    topLayout->setColumnStretch(0, 0);
    topLayout->setColumnStretch(1, 1);
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
    connect(&m_signal_widget, SIGNAL(sigTrackSelectionChanged()),
	    this, SLOT(refreshChannelControls()));

    connect(&m_signal_widget, SIGNAL(sigMouseChanged(int)),
	    this, SLOT(forwardMouseChanged(int)));

    refreshChannelControls();

//    qDebug("MainWidget::MainWidget(): done.");
}

//***************************************************************************
bool MainWidget::isOK()
{
    return (m_frm_channel_controls && m_scrollbar && m_slider);
}

//***************************************************************************
MainWidget::~MainWidget()
{
    foreach (MultiStateWidget *lamp, m_lamps)
	if (lamp) delete lamp;
    m_lamps.clear();
}

//***************************************************************************
void MainWidget::resizeEvent(QResizeEvent *)
{
    refreshChannelControls();
}

//***************************************************************************
void MainWidget::scrollbarMoved(int newval)
{
    Q_ASSERT(m_frm_channel_controls);
    if (!m_frm_channel_controls) return;

    // move the signal and the channel controls
    m_signal_widget.move(0, -newval);
    m_frm_channel_controls->move(0, -newval);
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
int MainWidget::loadFile(const KUrl &url)
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
    foreach (MultiStateWidget *lamp, m_lamps)
	if (lamp) lamp->setState(0);
}

//***************************************************************************
void MainWidget::parseKey(int key)
{
    if ((key < 0) || (key >= m_lamps.count()))
	return;
    if (m_lamps.at(key)) m_lamps.at(key)->nextState();
}

//***************************************************************************
void MainWidget::forwardCommand(const QString &command)
{
    emit sigCommand(command);
}

//***************************************************************************
bool MainWidget::executeCommand(const QString &command)
{
    if (!command.length()) return false;
    Parser parser(command);

    if (false) {
//    CASE_COMMAND("...")
//	;
    } else {
	if (parser.command() == "selectchannels")
	    foreach (MultiStateWidget *lamp, m_lamps)
		if (lamp) lamp->setState(0);

	if (parser.command() == "invertchannels")
	    foreach (MultiStateWidget *lamp, m_lamps)
		if (lamp) lamp->nextState();

	return m_signal_widget.executeCommand(command);
    }

    return true;
}

//***************************************************************************
void MainWidget::refreshChannelControls()
{
    Q_ASSERT(m_frm_channel_controls);
    if (!m_frm_channel_controls) return;

    unsigned int channels = tracks();
    int min_height = qMax(channels, (unsigned int)1) * MIN_PIXELS_PER_CHANNEL;
    bool need_scrollbar = (m_signal_frame.height() < min_height);
    bool scrollbar_visible = m_scrollbar->isVisible();
    int h = qMax(min_height, m_signal_frame.height());
    int w = m_signal_frame.width();
    int b = m_scrollbar->sizeHint().width();

    if (need_scrollbar && !scrollbar_visible) {
	// -- show the scrollbar --
	m_scrollbar->setFixedWidth(b);
	m_scrollbar->setValue(0);
	m_scrollbar->show();
	w -= b;
	scrollbar_visible = true;
    } else if (!need_scrollbar && scrollbar_visible) {
	// -- hide the scrollbar --
	m_scrollbar->hide();
	m_scrollbar->setFixedWidth(0);
	w += b;
	scrollbar_visible = false;
    }

    if (scrollbar_visible) {
	// adjust the limits of the scrollbar
	int min = m_scrollbar->minimum();
	int max = m_scrollbar->maximum();
	double val = (m_scrollbar->value()-(double)min) / (double)(max-min);

	min = 0;
	max = h-m_signal_frame.height();
	m_scrollbar->setRange(min, max);
	m_scrollbar->setValue((int)floor(val * (double)max));
	m_scrollbar->setSingleStep(1);
	m_scrollbar->setPageStep(m_signal_frame.height());
    }

    // resize the signal widget and the frame with the channel controls
    m_frm_channel_controls->resize(m_frm_channel_controls->width(), h);
    if ((m_signal_widget.width() != w) || (m_signal_widget.height() != h)) {
	m_signal_widget.resize(w, h);
    }

    h = m_frm_channel_controls->height();
    int x = (m_frm_channel_controls->width()-20) / 2;
    int y;

    // delete now unused lamps
    while (m_lamps.count() > static_cast<int>(channels)) {
	MultiStateWidget *lamp = m_lamps.takeLast();
	if (lamp) delete lamp;
    }

    // move the existing lamps and speakers to their new position
    int i = 0;
    foreach (MultiStateWidget *lamp, m_lamps) {
	y = (i++ * h) / qMax((unsigned int)1, channels);
	if (lamp) lamp->setGeometry(x, y + 5, 20, 20);
    }

    // add lamps+speakers for new channels
    for (unsigned int i = m_last_tracks; i < channels; i++) {
	MultiStateWidget *msw =
	    new MultiStateWidget(m_frm_channel_controls, i);
	Q_ASSERT(msw);
	if (!msw) continue;

	m_lamps.append(msw);
	Q_ASSERT(m_lamps.at(i));
	if (!m_lamps.at(i)) continue;
	m_lamps.at(i)->addPixmap("light_on.xpm");
	m_lamps.at(i)->addPixmap("light_off.xpm");
	QObject::connect(
	    m_lamps.at(i), SIGNAL(clicked(int)),
	    &m_signal_widget, SLOT(toggleTrackSelection(int))
	);

	y = (i * h) / qMax(channels, (unsigned int)1);
	m_lamps.at(i)->setGeometry(x, y + 5, 20, 20);
	m_lamps.at(i)->show();
    }

    // set the updated identifiers of the widgets
    i = 0;
    foreach (MultiStateWidget *lamp, m_lamps) {
	if (!lamp) continue;
	lamp->setID(i);
	lamp->setState(signalManager().trackSelected(i) ? 0 : 1);
	++i;
    }

    m_last_tracks = channels;
}

//***************************************************************************
unsigned int MainWidget::tracks()
{
    return m_signal_widget.tracks();
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
#include "MainWidget.moc"
//***************************************************************************
//***************************************************************************
