#include <stdio.h>
#include <stdlib.h>

#include <qpushbutton.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qfiledlg.h>

#include "module.h"
#include <kapp.h>

const char *version = "1.1";
const char *author = "Thomas Eschenbacher";
const char *name = "playback";
static const char *devicetext[] = {
    "/dev/dsp",
    "/dev/audio",
    "/dev/adsp",
    "/dev/dio",
    0
};

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

//**********************************************************
Dialog *getDialog (DialogOperation *operation) 
{
    PlayBackDialog *pb = new PlayBackDialog(operation->isModal());
    ASSERT(pb);
    if (!pb) return 0;
    
    if (!pb->isOK()) {
	delete pb;
	return 0;
    } else {
	return pb;
    }
}

//**********************************************************
PlayBackDialog::PlayBackDialog(bool modal)
    :Dialog(modal) 
{
    // get the current options from the main application
    playback_params = KwaveApp::getPlaybackParams();

    bg = 0;
    b8 = 0;
    b16 = 0;
    b24 = 0;
    bufferlabel = 0;
    buffersize = 0;
    cancel = 0;
    comstr = 0;
    devicelabel = 0;
    devicebox = 0;
    stereo = 0;
    separator = 0;
    select_device = 0;
    test = 0;
    ok = 0;

    int h=0, w=0;

    setCaption (i18n("Playback Options :"));

    // -- checkboxes for 8/16/24 Bits --
    bg = new QButtonGroup(i18n("Resolution"), this);
    ASSERT(bg);
    if (!bg) return;

    b8 = new QRadioButton(i18n("&8 Bit"), bg);
    ASSERT(b8);
    if (!b8) return;
    b8->setFixedSize(b8->sizeHint());
    b8->setChecked(playback_params.bits_per_sample == 8);
    QToolTip::add(b8,
        i18n("Set Playback-Mode to 8 Bit (poor quality)\n"\
	     "should be supported by all sound hardware !"));

    b16 = new QRadioButton(i18n("1&6 Bit"), bg);
    ASSERT(b16);
    if (!b16) return;
    b16->setFixedSize(b16->sizeHint());
    b16->setChecked(playback_params.bits_per_sample == 16);
    QToolTip::add( b16,
        i18n("Set Playback-Mode to 16 Bit (CD-like quality)\n"\
	     "not supported by all sound hardware !"));

    b24 = new QRadioButton(i18n("2&4 Bit"), bg);
    ASSERT(b24);
    if (!b24) return;
    b24->setFixedSize(b24->sizeHint());
    b24->setChecked(playback_params.bits_per_sample == 24);
    QToolTip::add( b24,
        i18n("Set Playback-Mode to 24 Bit (high quality)\n"\
	     "only supported by some new sound hardware !"));
    b24->setEnabled(false); // ### not implemented yet ###

    // -- playback device --
    devicelabel = new QLabel(i18n("Playback Device :"), this);
    ASSERT(devicelabel);
    if (!devicelabel) return;

    devicebox = new QComboBox(true, this);
    ASSERT(devicebox);
    if (!devicebox) return;
    devicebox->insertStrList(devicetext, -1);
    devicebox->setMinimumWidth(devicebox->sizeHint().width()+10);
    for (int i=0; i < devicebox->count(); i++) {
	if (strcmp(playback_params.device, devicebox->text(i))==0) {
	    devicebox->setCurrentItem(i);
	    break;
	}
    }

    h = max(devicelabel->sizeHint().height(),
            devicebox->sizeHint().height());
    w = devicelabel->sizeHint().width();
    devicelabel->setFixedSize(w, h);
    devicebox->setFixedHeight(h);

    select_device = new QPushButton(i18n("se&lect..."), this);
    ASSERT(select_device);
    if (!select_device) return;
    select_device->setFixedWidth(select_device->sizeHint().width());
    QToolTip::add(select_device,
	i18n("Select a playback device not listed\n"\
	     "in the standard selection.\n"\
	     "(sorry, not implemented yet!)"));
    select_device->setEnabled(false);
    // ### not implemented yet ###
    //     -> needs support for ALSA,
    //        OSS/Free only supports up to 16 bits :-(

    // -- create all layout objects --
    QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
    ASSERT(topLayout);
    if (!topLayout) return;

    QVBoxLayout *bitsBoxLayout = new QVBoxLayout(bg, 10);
    ASSERT(bitsBoxLayout);
    if (!bitsBoxLayout) return;
    bitsBoxLayout->addSpacing( bg->fontMetrics().height() );

    QHBoxLayout *bitsLayout = new QHBoxLayout();
    ASSERT(bitsLayout);
    if (!bitsLayout) return;

    QHBoxLayout *deviceLayout = new QHBoxLayout();
    ASSERT(deviceLayout);
    if (!deviceLayout) return;

    QHBoxLayout *bufferLayout = new QHBoxLayout();
    ASSERT(bufferLayout);
    if (!bufferLayout) {
	delete deviceLayout;
	return;
    }

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    ASSERT(buttonsLayout);
    if (!buttonsLayout) {
	delete deviceLayout;
	delete bufferLayout;
	return;
    }

    // -- buffer size --
    buffersize = new Slider(4, 16, 1, 5, Slider::Horizontal, this);
    ASSERT(buffersize);
    if (!buffersize) return;
    buffersize->setValue(playback_params.bufbase);

    bufferlabel = new QLabel("", this);
    ASSERT(bufferlabel);
    if (!bufferlabel) return;
    setBufferSize(playback_params.bufbase);
    QToolTip::add(buffersize, i18n("This is the size of the buffer "\
	"used for communication with the sound driver\n"\
	"If your computer is rather slow select a big buffer"));

    w = bufferlabel->sizeHint().width()+20;
    bufferlabel->setFixedWidth(w);
    buffersize->setMinimumWidth(w/2);

    // -- stereo checkbox --
    stereo = new QCheckBox(i18n("&stereo playback"), this);
    ASSERT(stereo);
    if (!stereo) return;
    stereo->setChecked(playback_params.channels >= 2);
    stereo->setFixedHeight(stereo->sizeHint().height());

    // -- separator --
    separator = new QFrame(this, "separator line");
    ASSERT(separator);
    if (!separator) return;
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());

    // give all widgets the same height
    h = max(bufferlabel->sizeHint().height(),
            buffersize->sizeHint().height());
    h = max(h, devicelabel->sizeHint().height());
    h = max(h, devicebox->sizeHint().height());
    h = max(h, select_device->sizeHint().height());

    bufferlabel->setFixedHeight(h);
    buffersize->setFixedHeight(h);
    devicelabel->setFixedHeight(h);
    devicebox->setFixedHeight(h);
    select_device->setFixedHeight(h);

    // button for "test settings"
    test = new QPushButton(i18n("&test settings"), this);
    ASSERT(test);
    if (!test) return;
    QToolTip::add(test,
	i18n("Try to play a short sound\n"\
	     "using the current settings.\n"\
	     "(sorry, not implemented yet!)"));
    test->setEnabled(false); // ### not implemented yet ###

    // buttons for OK and Cancel
    ok = new QPushButton(OK, this);
    ASSERT(ok);
    if (!ok) return;

    cancel = new QPushButton(CANCEL, this);
    ASSERT(cancel);
    if (!cancel) return;

    h = max(ok->sizeHint().height(), cancel->sizeHint().height());
    w = max(ok->sizeHint().width(), cancel->sizeHint().width());
    ok->setFixedSize(w, h);
    cancel->setFixedSize(w,h);
    test->setFixedSize(test->sizeHint().width(), h);

    // add controls to their layouts
    bitsBoxLayout->addLayout(bitsLayout);

    topLayout->addWidget(bg);
    bitsLayout->addWidget(b8, 0, AlignLeft | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(b16, 0, AlignCenter | AlignCenter);
    bitsLayout->addStretch(10);
    bitsLayout->addWidget(b24, 0, AlignRight | AlignCenter);

    topLayout->addLayout(deviceLayout);
    deviceLayout->addWidget(devicelabel, 0, AlignLeft | AlignCenter);
    deviceLayout->addSpacing(10);
    deviceLayout->addWidget(devicebox, 0, AlignRight | AlignCenter);
    deviceLayout->addWidget(select_device, 0, AlignRight | AlignCenter);

    topLayout->addLayout(bufferLayout);
    bufferLayout->addWidget(bufferlabel, 0, AlignLeft | AlignCenter);
    bufferLayout->addSpacing(10);
    bufferLayout->addWidget(buffersize,0, AlignRight | AlignCenter);

    topLayout->addWidget(stereo);
    topLayout->addWidget(separator);

    topLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(test, 0, AlignLeft);
    buttonsLayout->addSpacing(30);
    buttonsLayout->addStretch(10);
    buttonsLayout->addWidget(ok, 0, AlignRight);
    buttonsLayout->addSpacing(10);
    buttonsLayout->addWidget(cancel, 0, AlignRight);

    topLayout->activate();

    resize(minimumSize());

    ok->setFocus();
    connect(ok ,SIGNAL(clicked()),
                SLOT (accept()));
    connect(cancel, SIGNAL(clicked()),
                    SLOT (reject()));
    connect(buffersize, SIGNAL(valueChanged(int)),
                        SLOT(setBufferSize(int)));
    connect(select_device, SIGNAL(clicked()),
                           SLOT(selectPlaybackDevice()));
}

//**********************************************************
bool PlayBackDialog::isOK()
{
    ASSERT(bufferlabel);
    ASSERT(buffersize);
    ASSERT(devicelabel);
    ASSERT(devicebox);
    ASSERT(stereo);
    ASSERT(bg);
    ASSERT(b8);
    ASSERT(b16);
    ASSERT(b24);
    ASSERT(separator);
    ASSERT(select_device);
    ASSERT(test);
    ASSERT(ok);
    ASSERT(cancel);

    return (bufferlabel && buffersize && devicelabel &&
	devicebox && stereo && bg && b8 && b16 && b24 &&
	separator && select_device && test && ok && cancel );
}

//**********************************************************
void PlayBackDialog::setBufferSize(int exp) 
{
    ASSERT(bufferlabel);
    if (!bufferlabel) return;

    char buf[256];
    int val = 1 << exp;

    snprintf(buf, sizeof(buf), i18n("Buffer Size : %5d samples"), val);
    bufferlabel->setText (buf);

    playback_params.bufbase = exp;
}

//**********************************************************
// format of the return string:
// playback(
//    rate,            [44100]
//    channels,        [1 | 2]
//    bits_per_sample, [8, 16]
//    device,          [/dev/dsp , ... ]
//    bufbase          [4...16]
// )
const char *PlayBackDialog::getCommand() 
{
    ASSERT(stereo);
    ASSERT(b16);
    ASSERT(devicebox);
    if (!stereo || !b16 || !devicebox) return 0;

    char buf[256];

    // playback_params.rate = ...; ### not changeable yet
    playback_params.channels = stereo->isChecked() ? 2 : 1;
    playback_params.bits_per_sample = b24->isChecked() ? 24 :
	(b16->isChecked() ? 16 : 8);
    playback_params.device = devicebox->currentText();
    // playback_params.bufbase = ...; already set by setBufferSize

    snprintf(buf, sizeof(buf),
	"playback (%d,%d,%d,%s,%d)",
	playback_params.rate,
	playback_params.channels,
	playback_params.bits_per_sample,
	playback_params.device,
	playback_params.bufbase
    );

    if (comstr) delete[] comstr;
    comstr = duplicateString(buf);
    debug("plugin dialog 'playback': return string = '%s'",comstr);
    return comstr;
}

//**********************************************************
void PlayBackDialog::selectPlaybackDevice()
{
    if (!isOK()) return;

    QString new_device = QFileDialog::getOpenFileName("/dev/", 0, this);
    if (new_device.isNull()) return;

}

//**********************************************************
PlayBackDialog::~PlayBackDialog() 
{
    if (comstr) delete[] comstr;
}

//**********************************************************
