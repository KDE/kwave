
#include "config.h"
#include <math.h>

#include <qkeycode.h>
#include <qframe.h>
#include <qimage.h>
#include <qaccel.h>
#include <qwidget.h>

#include <kapp.h>
#include <kbuttonbox.h>

#include <libkwave/String.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>

#include "libgui/Dialog.h"
#include "libgui/MenuManager.h"
#include "libgui/MultiStateWidget.h"
#include "libgui/OverViewWidget.h"

#include "sampleop.h"
#include "SignalWidget.h"
#include "SignalManager.h"
#include "MainWidget.h"

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (matchCommand(command, x)) {

static const int tbl_keys[10] = {
    Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0
};

static const char *zoomtext[] = {
    "400 %", "200 %", "100 %", "33 %", "10 %", "3 %", "1 %", "0.1 %"
};

//*****************************************************************************
MainWidget::MainWidget(QWidget *parent, MenuManager &manage,
                       KStatusBar &status)
    :QWidget(parent),
     status(status),
     menu(manage)
{
    debug("MainWidget::MainWidget()");
    int s[3];
    MultiStateWidget *msw;

    bsize = 0;
    buttons = 0;
    keys = 0;
    loopbutton = 0;
    lastChannels = 0;
    minusbutton = 0;
    nozoombutton = 0;
    playbutton = 0;
    plusbutton = 0;
    signalview = 0;
    slider = 0;
    zoomallbutton = 0;
    zoombutton = 0;
    zoomselect = 0;

    // -- multistate widgets for lamps and speakers --

    lamps.setAutoDelete(true);
    lamps.clear();
    msw = new MultiStateWidget(this, 0);
    ASSERT(msw);
    if (!msw) return;
    lamps.append(msw);
    s[0] = lamps.at(0)->addPixmap("light_on.xpm");
    s[1] = lamps.at(0)->addPixmap("light_off.xpm");
    lamps.at(0)->setStates(s);

    speakers.setAutoDelete(true);
    speakers.clear();
    msw = new MultiStateWidget(this, 0, 3);
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

    slider = new OverViewWidget(this);
    ASSERT(slider);
    if (!slider) return;

    // -- buttons for playback and zoom --

    buttons = new KButtonBox(this,KButtonBox::HORIZONTAL);
    ASSERT(buttons);
    if (!buttons) return;

    zoomselect = new QComboBox(true, this);
    ASSERT(zoomselect);
    if (!zoomselect) return;
    zoomselect->insertStrList(zoomtext, sizeof(zoomtext) / sizeof(char *));
    zoomselect->setEditText("");

    buttons->addStretch();

    // [Play]
    playbutton = buttons->addButton(i18n("Play"));
    ASSERT(playbutton);
    if (!playbutton) return;
    playbutton->setAccel(Key_Space);

    // [Loop]
    loopbutton = buttons->addButton(i18n("&Loop"));
    ASSERT(loopbutton);
    if (!loopbutton) return;
    loopbutton->setAccel(Key_L);

    buttons->addStretch();

    // [Zoom]
    zoombutton = buttons->addButton (i18n("&Zoom"));
    ASSERT(zoombutton);
    if (!zoombutton) return;
    zoombutton->setAccel(Key_Z);

    // [+]
    plusbutton = buttons->addButton("+");
    ASSERT(plusbutton);
    if (!plusbutton) return;
    plusbutton->setAccel(Key_Plus);

    // [-]
    minusbutton = buttons->addButton ("-");
    ASSERT(minusbutton);
    if (!minusbutton) return;
    minusbutton->setAccel (Key_Minus);

    // [All]
    zoomallbutton = buttons->addButton(i18n("All"));
    ASSERT(zoomallbutton);
    if (!zoomallbutton) return;

    // [1:1]	
    nozoombutton = buttons->addButton(i18n("1:1"));
    ASSERT(nozoombutton);
    if (!nozoombutton) return;

    buttons->addStretch();

    // -- signal widget --

    signalview = new SignalWidget(this, menu);
    ASSERT(signalview);
    if (!signalview) return;
    if (!signalview->isOK()) {
	warning("MainWidget::MainWidget: failed in creating SignalWidget !");
	delete signalview;
	signalview = 0;
	return;
    }

    // -- connect all signals from/to the signal widget --

    connect(playbutton, SIGNAL(pressed()),
	    this, SLOT(play()));
    connect(loopbutton, SIGNAL(pressed()),
	    this, SLOT(loop()));
    connect(zoombutton, SIGNAL(pressed()),
	    signalview, SLOT(zoomRange()));
    connect(plusbutton, SIGNAL(pressed()),
	    signalview, SLOT(zoomIn()));
    connect(minusbutton, SIGNAL(pressed()),
	    signalview, SLOT(zoomOut()));
    connect(zoomallbutton, SIGNAL(pressed()),
	    signalview, SLOT(zoomAll()));
    connect(nozoombutton, SIGNAL(pressed()),
	    signalview, SLOT(zoomNormal()));

    connect(slider, SIGNAL(valueChanged(int)),
	    signalview, SLOT(slot_setOffset(int)));
    connect(zoomselect, SIGNAL(activated(int)),
	    this, SLOT(zoomSelected(int)));
    connect(signalview, SIGNAL(viewInfo(int, int, int)),
	    slider, SLOT(setRange(int, int, int)));
    connect(signalview, SIGNAL(zoomInfo(double)),
	    this, SLOT(slot_ZoomChanged(double)));
    connect(signalview, SIGNAL(playingfinished()),
	    this, SLOT(stop()));
    connect(this, SIGNAL(setOperation(int)),
	    signalview, SLOT(playback_setOp(int)));
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
    connect(signalview, SIGNAL(sigChannelAdded(unsigned int)),
            this, SLOT(channelAdded(unsigned int)));
    connect(signalview, SIGNAL(sigChannelDeleted(unsigned int)),
            this, SLOT(channelDeleted(unsigned int)));

    refreshControls();
    debug("MainWidget::MainWidget(): done.");
}

//*****************************************************************************
bool MainWidget::isOK()
{
    ASSERT(buttons);
    ASSERT(keys);
    ASSERT(loopbutton);
    ASSERT(minusbutton);
    ASSERT(nozoombutton);
    ASSERT(playbutton);
    ASSERT(signalview);
    ASSERT(slider);
    ASSERT(zoomallbutton);
    ASSERT(zoombutton);
    ASSERT(zoomselect);

    return ( buttons && keys && loopbutton && minusbutton &&
             nozoombutton && playbutton && signalview &&
             slider && zoomallbutton && zoombutton &&
             zoomselect );
}

//*****************************************************************************
MainWidget::~MainWidget()
{
    debug("MainWidget::~MainWidget()");
    ASSERT(KApplication::getKApplication());

    if (signalview) delete signalview;
    signalview = 0;

    if (buttons) delete buttons;
    buttons = 0;

    // do not delete the buttons themselfes, the
    // KButtonBox has "auto-deletion" turned on !

    if (keys) delete keys;
    lamps.clear();
    speakers.clear();

    debug("MainWidget::~MainWidget(): done.");
}

//*****************************************************************************
void MainWidget::refreshControls()
{
    bool have_signal = (getChannelCount() != 0);

    // enable/disable all items that depend on having a signal
    menu.setItemEnabled("@SIGNAL", have_signal);

    // update the list of deletable channels
    menu.clearNumberedMenu ("ID_EDIT_CHANNEL_DELETE");
    for (unsigned int i = 0; i < getChannelCount(); i++) {
	char buf[64];
	snprintf(buf, sizeof(buf), "%d", i);
	menu.addNumberedMenuEntry("ID_EDIT_CHANNEL_DELETE", buf);
    }

    // enable/disable the buttons
    if (loopbutton) loopbutton->setEnabled(have_signal);
    if (playbutton) playbutton->setEnabled(have_signal);
    if (minusbutton) minusbutton->setEnabled(have_signal);
    if (nozoombutton) nozoombutton->setEnabled(have_signal);
    if (plusbutton) plusbutton->setEnabled(have_signal);
    if (zoomallbutton) zoomallbutton->setEnabled(have_signal);
    if (zoombutton) zoombutton->setEnabled(have_signal);
    if (zoomselect) zoomselect->setEnabled(have_signal);
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
    ASSERT(slider);

    closeSignal();
    if (signalview) signalview->setSignal(filename, type);
    if (slider) slider->refresh();
    refreshControls();
}

//*****************************************************************************
void MainWidget::setSignal(SignalManager *signal)
{
    ASSERT(signalview);
    ASSERT(slider);

    closeSignal();
    if (signalview) signalview->setSignal(signal);
    refreshControls();
}

//*****************************************************************************
void MainWidget::closeSignal()
{
    ASSERT(signalview);
    ASSERT(zoomselect);

    if (signalview) signalview->closeSignal();

    setTimeInfo(0);
    setSelectedTimeInfo(0);
    setLengthInfo(0);
    setRateInfo(0);

    if (zoomselect) zoomselect->setEditText("");

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
void MainWidget::zoomSelected(int index)
{
    ASSERT(signalview);
    if (!signalview) return;

    double new_zoom;
    if ((index >= 0) && (index < (int)(sizeof(zoomtext)/sizeof(char *)))) {
	new_zoom = 100.0 / (double)strtod(zoomtext[index], 0);
	signalview->setZoom(new_zoom);
	signalview->refreshAllLayers();
    }
}

//*****************************************************************************
void MainWidget::slot_ZoomChanged(double zoom)
{
    if (zoom <= 0.0) return;
    if (!zoomselect) return;

    double percent = 100.0 / zoom;
    char buf[256];

    if (getChannelCount() == 0) {
	buf[0] = 0;
    } else if (percent < 1.0) {
	char format[128];
	int digits = (int)ceil(1.0 - log10(percent));

	snprintf(format, sizeof(format), "%%0.%df %%%%", digits);
	snprintf(buf, sizeof(buf), format, percent);
    } else if (percent < 10.0) {
	snprintf(buf, sizeof(buf), "%0.1f %%", percent);
    } else if (percent < 1000.0) {
	snprintf(buf, sizeof(buf), "%0.0f %%", percent);
    } else {
	snprintf(buf, sizeof(buf), "x %d", (int)(percent / 100.0));
    }
    if (zoomselect) zoomselect->setEditText(buf);
}

//*****************************************************************************
unsigned char *MainWidget::getOverView(int val)
{
    ASSERT(signalview);
    return (signalview) ? signalview->getOverview(val) : 0;
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
    debug("MainWidget::forwardCommand(%s)", command);    // ###
    emit sigCommand(command);
}

//*****************************************************************************
bool MainWidget::executeCommand(const char *command)
{
    ASSERT(command);
    debug("MainWidget::executeCommand(%s)", command);    // ###
    if (!command) return false;

    if (false) {
//    CASE_COMMAND("refreshchannels")
//	//      resetChannels();
//	setChannelInfo(signalview->getChannelCount());
//      signalview->refresh();
//      updateMenu();
//    CASE_COMMAND("setplayback")
//	Parser parser(command);
//	playbit = parser.toInt();
//	bufbase = parser.toInt();
//    CASE_COMMAND("setmemory")
//	Parser parser(command);
//	
//	mmap_threshold = parser.toInt();
//	mmap_dir = strdup(parser.getNextParam());
    } else {
	if (matchCommand(command, "selectchannels"))
	    for (unsigned int i = 0; i < getChannelCount(); i++)
		if (lamps.at(i)) lamps.at(i)->setState(0);

	if (matchCommand(command, "invertchannels"))
	    for (unsigned int i = 0; i < getChannelCount(); i++)
		if (lamps.at(i)) lamps.at(i)->nextState();

	bool result = signalview->executeCommand(command);
	if (signalview) signalview->refreshAllLayers();
	return result;
    }

    return true;
}

//*****************************************************************************
void MainWidget::loop()
{
    ASSERT(playbutton);
    ASSERT(loopbutton);
    if (!playbutton) return;
    if (!loopbutton) return;

    emit setOperation(LOOP);
    playbutton->setText(i18n("Stop"));
    loopbutton->setText(i18n("Halt"));     // halt feature by gerhard Zint
    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
}

//*****************************************************************************
void MainWidget::play ()
{
    ASSERT(playbutton);
    ASSERT(loopbutton);
    if (!playbutton) return;
    if (!loopbutton) return;

    emit setOperation (PLAY);
    playbutton->setText (i18n("Stop"));
    loopbutton->setText (i18n("Halt"));     // halt feature by gerhard Zint
    this->disconnect (playbutton, SIGNAL(pressed()), this, SLOT(play()));
    this->disconnect (loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
    this->connect (playbutton, SIGNAL(pressed()), this, SLOT(stop()));
    this->connect (loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
}

//*****************************************************************************
void MainWidget::halt ()
{
    ASSERT(playbutton);
    ASSERT(loopbutton);
    if (!playbutton) return;
    if (!loopbutton) return;

    playbutton->setText(i18n("Play"));
    loopbutton->setText(i18n("&Loop"));
    loopbutton->setAccel(Key_L);    //seems to neccessary

    emit setOperation (PHALT);

    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
}

//*****************************************************************************
void MainWidget::stop ()
{
    ASSERT(playbutton);
    ASSERT(loopbutton);
    if (!playbutton) return;
    if (!loopbutton) return;

    playbutton->setText(i18n("Play"));
    loopbutton->setText(i18n("&Loop"));
    loopbutton->setAccel(Key_L);    //seems to be neccessary

    emit setOperation(PSTOP);

    this->disconnect(playbutton, SIGNAL(pressed()), this, SLOT(stop()));
    this->disconnect(loopbutton, SIGNAL(pressed()), this, SLOT(halt()));
    this->connect(playbutton, SIGNAL(pressed()), this, SLOT(play()));
    this->connect(loopbutton, SIGNAL(pressed()), this, SLOT(loop()));
}

//*****************************************************************************
void MainWidget::setSelectedTimeInfo(double ms)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), i18n("selected: %0.1f ms"), ms);
    status.changeItem(buffer, 4);
}

//*****************************************************************************
void MainWidget::setTimeInfo(double ms)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), i18n("Length: %0.1f ms"), ms);
    status.changeItem(buffer, 1);
}

//*****************************************************************************
void MainWidget::refreshChannelControls()
{
//    ASSERT(parent);
//    debug("MainWidget::refreshChannelControls()");
    unsigned int channels = getChannelCount();

    if (lastChannels == channels) {
	// nothing to do...
	return ;
    }

//    // resize the TopWidget if one signal is smaller than the size
//    // of the lamp/speaker icon
//    if ( (lastChannels < channels) &&
//	 ((unsigned int)parent->height() < (channels + 4)*40) ) {
//	debug("MainWidget::setChannelInfo() -> resizing topwidget ");
//	parent->resize(width(), (channels + 4)*40);
//    }

    // move the existing lamps and speakers to their new position
    for (unsigned int i = 0; (i < lastChannels) && (i < channels); i++) {
	ASSERT(lamps.at(i));
	ASSERT(speakers.at(i));
	if (!lamps.at(i)) continue;
	if (!speakers.at(i)) continue;

	lamps.at(i)->setGeometry(0, i*(height() - bsize - 20) /
	    channels, 20, 20);

	speakers.at(i)->setGeometry(0, i*(height() - bsize - 20) /
	    channels + 20, 20, 20);
    }

    // delete now unused lamps and speakers
    while (lamps.count() > channels)
	lamps.remove(lamps.last());
    while (speakers.count() > channels)
	speakers.remove(speakers.last());

    // add lamps+speakers for new channels
    for (unsigned int i = lastChannels; i < channels; i++) {
	int s[3];
	MultiStateWidget *msw = new MultiStateWidget(this, i);
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

        msw = new MultiStateWidget(this, 0, 3);
        ASSERT(msw);
        if (!msw) continue;
	speakers.append(msw);
	ASSERT(speakers.at(i));
	if (!speakers.at(i)) continue;
	s[0] = speakers.at(i)->addPixmap("rspeaker.xpm");
	s[1] = speakers.at(i)->addPixmap("lspeaker.xpm");
	s[2] = speakers.at(i)->addPixmap("xspeaker.xpm");
	speakers.at(i)->setStates(s);

	lamps.at(i)->setGeometry(0, i*(height() - bsize - 20) / channels,
	                         20, 20);
	speakers.at(i)->setGeometry(0, i*(height()-bsize-20) / channels + 20,
	                            20, 20);
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
void MainWidget::resizeEvent(QResizeEvent *)
{
    ASSERT(buttons);
    ASSERT(zoomselect);
    ASSERT(slider);
    ASSERT(signalview);

    if (!buttons) return;
    if (!zoomselect) return;
    if (!slider) return;
    if (!signalview) return;

    bsize = buttons->sizeHint().height();

    buttons->setGeometry(0, height() - bsize,
                         width()*3 / 4, bsize);
    zoomselect->setGeometry(width()*3 / 4, height() - bsize,
                            width() / 4, bsize);
    slider->setGeometry (20, height() - (bsize + 20), width() - 20, 20);
    signalview->setGeometry (20, 0, width() - 20, height() - (bsize + 20));

    for (unsigned int i = 0; i < getChannelCount(); i++) {
	if (lamps.count() < getChannelCount()) continue;
	if (speakers.count() < getChannelCount()) continue;
	ASSERT(lamps.at(i));
	if (lamps.at(i)) lamps.at(i)->setGeometry(
		0, i*(height() - bsize - 20) / getChannelCount(), 20, 20);

	ASSERT(speakers.at(i));		
	if (speakers.at(i)) speakers.at(i)->setGeometry(
	    0, i*(height() - bsize - 20) / getChannelCount() + 20, 20, 20);
    }
//    debug("MainWidget::resizeEvent(): done.");
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
//*****************************************************************************
