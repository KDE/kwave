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

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include <QBitmap>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtGlobal>
#include <QWheelEvent>

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
    :QWidget(parent), m_overview(0), m_signal_frame(this),
     m_signal_widget(&m_signal_frame),
     m_frm_channel_controls(0), m_vertical_scrollbar(0),
     m_horizontal_scrollbar(0), m_lamps(), m_last_tracks(0)
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

    // -- horizontal scrollbar --

    m_horizontal_scrollbar = new QScrollBar(this);
    Q_ASSERT(m_horizontal_scrollbar);
    if (!m_horizontal_scrollbar) return;
    m_horizontal_scrollbar->setOrientation(Qt::Horizontal);
    m_horizontal_scrollbar->setFixedHeight(
	m_horizontal_scrollbar->sizeHint().height());
    topLayout->addWidget(m_horizontal_scrollbar, 2, 1);

    // -- overview widget --
    m_overview = new OverViewWidget(m_signal_widget.signalManager(), this);
    Q_ASSERT(m_overview);
    if (!m_overview) return;
    topLayout->addWidget(m_overview, 1, 1);
    connect(&m_signal_widget, SIGNAL(selectedTimeInfo(sample_index_t,
	sample_index_t, double)),
	m_overview, SLOT(setSelection(sample_index_t, sample_index_t, double)));
    m_overview->labelsChanged(m_signal_widget.signalManager().labels());
    connect(&(playbackController()), SIGNAL(sigPlaybackPos(sample_index_t)),
	m_overview, SLOT(playbackPositionChanged(sample_index_t)));
    connect(&(playbackController()), SIGNAL(sigPlaybackStopped()),
	m_overview, SLOT(playbackStopped()));

    // -- scrollbar for the signal widget and the channel controls --

    m_vertical_scrollbar = new QScrollBar(this);
    Q_ASSERT(m_vertical_scrollbar);
    if (!m_vertical_scrollbar) return;
    m_vertical_scrollbar->setOrientation(Qt::Vertical);
    m_vertical_scrollbar->setFixedWidth(
	m_vertical_scrollbar->sizeHint().width());
    m_vertical_scrollbar->hide();
    m_vertical_scrollbar->setFixedWidth(0);
    signalLayout->addWidget(m_vertical_scrollbar, 0, Qt::AlignRight);
    m_vertical_scrollbar->hide();

    // -- signal widget --

    m_signal_widget.setMinimumSize(100, MIN_PIXELS_PER_CHANNEL);

    // -- do all the geometry management stuff --
    m_overview->hide();

    topLayout->addWidget(frmChannelsBack, 0, 0);

    topLayout->setRowStretch(0, 1);
    topLayout->setRowStretch(1, 0);
    topLayout->setRowStretch(2, 0);
    topLayout->setColumnStretch(0, 0);
    topLayout->setColumnStretch(1, 1);
    signalLayout->activate();
    topLayout->activate();
    m_horizontal_scrollbar->hide();

    // -- connect all signals from/to the signal widget --

    connect(m_vertical_scrollbar, SIGNAL(valueChanged(int)),
            this, SLOT(verticalScrollBarMoved(int)));
    connect(m_horizontal_scrollbar, SIGNAL(valueChanged(int)),
	    this, SLOT(horizontalScrollBarMoved(int)));

    connect(m_overview, SIGNAL(valueChanged(sample_index_t)),
	    &m_signal_widget, SLOT(setOffset(sample_index_t)));
    connect(m_overview, SIGNAL(sigCommand(const QString &)),
            this, SLOT(forwardCommand(const QString &)));
    connect(&m_signal_widget, SIGNAL(viewInfo(sample_index_t,
	    unsigned int, sample_index_t)),
	    m_overview, SLOT(setRange(sample_index_t, unsigned int,
	    sample_index_t)));
    connect(&m_signal_widget, SIGNAL(viewInfo(sample_index_t,
	    unsigned int, sample_index_t)),
	    this, SLOT(updateViewInfo(sample_index_t, unsigned int,
	    sample_index_t)));

    connect(&m_signal_widget, SIGNAL(sigZoomChanged(double)),
	    this, SIGNAL(sigZoomChanged(double)));
    connect(&m_signal_widget, SIGNAL(sigCommand(const QString &)),
	    this, SLOT(forwardCommand(const QString &)));
    connect(&m_signal_widget, SIGNAL(selectedTimeInfo(sample_index_t,
            sample_index_t, double)),
	    this, SIGNAL(selectedTimeInfo(sample_index_t,
            sample_index_t, double)));
    connect(&m_signal_widget, SIGNAL(sigTrackInserted(unsigned int)),
	    this, SLOT(slotTrackInserted(unsigned int)));
    connect(&m_signal_widget, SIGNAL(sigTrackDeleted(unsigned int)),
	    this, SLOT(slotTrackDeleted(unsigned int)));
    connect(&m_signal_widget, SIGNAL(sigTrackSelectionChanged()),
	    this, SLOT(refreshChannelControls()));

    connect(&m_signal_widget, SIGNAL(sigMouseChanged(Kwave::MouseMark::Mode)),
	    this, SIGNAL(sigMouseChanged(Kwave::MouseMark::Mode)));

    refreshChannelControls();

//    qDebug("MainWidget::MainWidget(): done.");
}

//***************************************************************************
bool MainWidget::isOK()
{
    return (m_frm_channel_controls && m_vertical_scrollbar &&
            m_horizontal_scrollbar && m_overview);
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
void MainWidget::wheelEvent(QWheelEvent *event)
{
    if (!event || !m_overview) return;

    // process only wheel events on the signal and overview frame,
    // not on the channel controls or scrollbars
    if (!m_signal_frame.geometry().contains(event->pos()) &&
	!m_overview->geometry().contains(event->pos()) )
    {
	event->ignore();
	return;
    }

    switch (event->modifiers()) {
	case Qt::NoModifier: {
	    // no modifier + <WheelUp/Down> => scroll left/right
	    if (event->delta() > 0)
		m_signal_widget.executeCommand("scrollleft()");
	    else if (event->delta() < 0)
		m_signal_widget.executeCommand("scrollright()");
	    event->accept();
	    break;
	}
	case Qt::ShiftModifier:
	    // <Shift> + <WheelUp/Down> => page up/down
	    if (event->delta() > 0)
		m_signal_widget.executeCommand("viewprev()");
	    else if (event->delta() < 0)
		m_signal_widget.executeCommand("viewnext()");
	    event->accept();
	    break;
	case Qt::ControlModifier:
	    // <Ctrl> + <WheelUp/Down> => zoom in/out
	    if (event->delta() > 0)
		m_signal_widget.executeCommand("zoomin()");
	    else if (event->delta() < 0)
		m_signal_widget.executeCommand("zoomout()");
	    event->accept();
	    break;
	default:
	    event->ignore();
    }
}

//***************************************************************************
void MainWidget::verticalScrollBarMoved(int newval)
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
    int res = m_signal_widget.loadFile(url);
    refreshChannelControls();
    return res;
}

//***************************************************************************
void MainWidget::closeSignal()
{
    m_signal_widget.close();
    refreshChannelControls();
}

//***************************************************************************
void MainWidget::newSignal(sample_index_t samples, double rate,
                           unsigned int bits, unsigned int tracks)
{
    m_signal_widget.newSignal(samples, rate, bits, tracks);
    refreshChannelControls();
}

//***************************************************************************
double MainWidget::zoom()
{
    return m_signal_widget.zoom();
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
int MainWidget::executeCommand(const QString &command)
{
    if (!command.length()) return -EINVAL;
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

    return 0;
}

//***************************************************************************
void MainWidget::refreshChannelControls()
{
    Q_ASSERT(m_frm_channel_controls);
    if (!m_frm_channel_controls) return;

    unsigned int channels = tracks();
    int min_height = qMax(channels, 1U) * MIN_PIXELS_PER_CHANNEL;
    bool need_vertical_scrollbar = (m_signal_frame.height() < min_height);
    bool vertical_scrollbar_visible = m_vertical_scrollbar->isVisible();
    int h = qMax(min_height, m_signal_frame.height());
    int w = m_signal_frame.width();
    int b = m_vertical_scrollbar->sizeHint().width();

    if (need_vertical_scrollbar && !vertical_scrollbar_visible) {
	// -- show the scrollbar --
	m_vertical_scrollbar->setFixedWidth(b);
	m_vertical_scrollbar->setValue(0);
	m_vertical_scrollbar->show();
	w -= b;
	vertical_scrollbar_visible = true;
    } else if (!need_vertical_scrollbar && vertical_scrollbar_visible) {
	// -- hide the scrollbar --
	m_vertical_scrollbar->hide();
	m_vertical_scrollbar->setFixedWidth(0);
	w += b;
	vertical_scrollbar_visible = false;
    }

    if (vertical_scrollbar_visible) {
	// adjust the limits of the vertical scrollbar
	int min = m_vertical_scrollbar->minimum();
	int max = m_vertical_scrollbar->maximum();
	double val = (m_vertical_scrollbar->value() -
	    static_cast<double>(min)) / static_cast<double>(max - min);

	min = 0;
	max = h-m_signal_frame.height();
	m_vertical_scrollbar->setRange(min, max);
	m_vertical_scrollbar->setValue(static_cast<int>(floor(val *
	    static_cast<double>(max))));
	m_vertical_scrollbar->setSingleStep(1);
	m_vertical_scrollbar->setPageStep(m_signal_frame.height());
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
	y = (i++ * h) / qMax(1U, channels);
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

	y = (i * h) / qMax(channels, static_cast<unsigned int>(1));
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
void MainWidget::updateViewInfo(sample_index_t, unsigned int, sample_index_t)
{
    refreshHorizontalScrollBar();
}

//***************************************************************************
void MainWidget::refreshHorizontalScrollBar()
{
    if (!m_horizontal_scrollbar) return;

    m_horizontal_scrollbar->blockSignals(true);

    // show/hide the overview widget
    if (!signalManager().isEmpty() && m_overview && !m_overview->isVisible()) {
	m_overview->show();
	m_horizontal_scrollbar->show();
    }
    if (signalManager().isEmpty() && m_overview && m_overview->isVisible()) {
	m_overview->hide();
	m_horizontal_scrollbar->hide();
    }

    // adjust the limits of the horizontal scrollbar
    if (signalManager().length() > 1) {
	// get the view information in samples
	unsigned int length  = signalManager().length();
	unsigned int offset  = m_signal_widget.offset();
	unsigned int visible = m_signal_widget.pixels2samples(displayWidth());
	if (visible > length) visible = length;
	unsigned int range   = length - visible;
	if (range) range--;
	// in samples:
	// min  => 0
	// max  => length - visible - 1
	// page => visible
// 	qDebug("range = 0...%u, visible=%u, offset=%u, offset+visible=%d range+visible=%d",
// 	    range, visible, offset, offset+visible, range+visible);

	// calculate ranges in samples
	int width = displayWidth();
	int page  = static_cast<int>(floor(static_cast<double>(width) *
	    static_cast<double>(visible) / static_cast<double>(length)));
	if (page < 1) page = 1;
	if (page > width) page = width;
	int min   = 0;
	int max   = width - page;
	int pos   = (range) ? static_cast<int>(floor(
	    static_cast<double>(offset) * static_cast<double>(max) /
	    static_cast<double>(range))) : 0;
	int single = (page / 10);
	if (!single) single = 1;
// 	qDebug("width=%d,max=%d, page=%d, single=%d, pos=%d",
// 	       width, max, page, single, pos);

	m_horizontal_scrollbar->setRange(min, max);
	m_horizontal_scrollbar->setValue(pos);
	m_horizontal_scrollbar->setSingleStep(single);
	m_horizontal_scrollbar->setPageStep(page);
    } else {
	m_horizontal_scrollbar->setRange(0,0);
    }

    m_horizontal_scrollbar->blockSignals(false);
}

//***************************************************************************
void MainWidget::horizontalScrollBarMoved(int newval)
{
    if (!m_horizontal_scrollbar) return;

    unsigned int max = m_horizontal_scrollbar->maximum();
    if (max < 1) {
	m_signal_widget.setOffset(0);
	return;
    }

    // convert the current position into samples
    sample_index_t length = signalManager().length();
    sample_index_t visible = m_signal_widget.pixels2samples(displayWidth());
    if (visible > length) visible = length;
    sample_index_t range   = length - visible;
    if (range) range--;

    sample_index_t pos = static_cast<sample_index_t>(floor(static_cast<double>(range) *
	static_cast<double>(newval) / static_cast<double>(max)));
//     qDebug("horizontalScrollBarMoved(%d) -> %u", newval, pos);
    m_signal_widget.setOffset(pos);
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
