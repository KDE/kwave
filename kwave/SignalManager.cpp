/***************************************************************************
          SignalManager.cpp -  manager class for multi-channel signals
			     -------------------
    begin                : Sun Oct 15 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
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
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <byteswap.h>
#include <limits.h>
#include <math.h>
#include <sched.h> // for sched_yield()

#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include <qbitmap.h>
#include <qfile.h>
#include <qlist.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtimer.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>

#include "libkwave/FileFormat.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleInputStream.h"
#include "libkwave/Signal.h"

#include "libgui/FileProgress.h"

#include "KwaveApp.h"
#include "sampleop.h"
#include "ClipBoard.h"
#include "SignalManager.h"
#include "SignalWidget.h"

/** use at least 2^8 = 256 bytes for playback buffer !!! */
#define MIN_PLAYBACK_BUFFER 8

/** use at most 2^16 = 65536 bytes for playback buffer !!! */
#define MAX_PLAYBACK_BUFFER 16

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define min(x,y) ((x<y) ? x : y)
#define max(x,y) ((x>y) ? x : y)

#define CASE_COMMAND(x) } else if (QString(command) == x) {

//***************************************************************************
//void threadStub(TimeOperation *)
//{
//    debug("thread started");
//
//    ASSERT(obj);
//    if (obj) {
//	ASSERT(obj->getSignal());
//	if (obj->getSignal()) {
//	    (obj->getSignal())->command(obj);
//	}
//    }
//    debug("thread done");
//}
//
//***************************************************************************
SignalManager::SignalManager(QWidget *parent)
    :QObject(),
    m_parent_widget(parent),
    m_spx_playback_pos(this, SLOT(updatePlaybackPos())),
    m_spx_playback_done(this, SLOT(forwardPlaybackDone()))
{
    initialize();
}

//***************************************************************************
//SignalManager::SignalManager(unsigned int numsamples,
//                             int rate, unsigned int channels)
//    :QObject(),
//    m_spx_playback_pos(this, SLOT(updatePlaybackPos())),
//    m_spx_playback_done(this, SLOT(forwardPlaybackDone()))
//{
//    initialize();
//    this->rate = rate;
//    m_name = i18n("noname");
//
//    for (unsigned int i = 0; i < channels; i++) {
//	Signal *new_signal = 0; // new Signal(numsamples, rate);
//	ASSERT(new_signal);
//	appendChannel(new_signal);
//    }
//
//}

//***************************************************************************
void SignalManager::loadFile(const QString &filename, int type)
{
    initialize();
    ASSERT(filename.length());
    m_name = filename;

    debug("SignalManager::loadFile(%s, %d)", filename.data(), type); // ###
    switch (type) {
	case WAV:
	    loadWav();
	    break;
	case ASCII:
	    loadAscii();
	    break;
	default:
	    ASSERT("unknown file type");
    }
}

//***************************************************************************
void SignalManager::initialize()
{
    m_name = "";
    lmarker = 0;
    rmarker = 0;
    rate = 0;
    for (unsigned int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
	msg[i] = 0;

    m_spx_playback_pos.setLimit(32); // limit for the queue
}

//***************************************************************************
void SignalManager::close()
{
    debug("SignalManager::close()");
    m_name = "";
    m_signal.close();
}

//***************************************************************************
void SignalManager::getMaxMin(unsigned int channel, int &max, int &min,
                              unsigned int begin, unsigned int len)
{
//    ASSERT(channel < signal.count());
//    if (channel >= signal.count()) return;
//    ASSERT(signal.at(channel));
//    if (!signal.at(channel)) return;
//
//    unsigned int sig_len;
//    sig_len = signal.at(channel)->length();
//    ASSERT(sig_len);
//
//    if (begin + len > sig_len) len = sig_len - begin;
// ###
//    signal.at(channel)->getMaxMin(max, min, begin, len);
}

//***************************************************************************
unsigned int SignalManager::length()
{
    return m_signal.length();
}

//***************************************************************************
const QArray<unsigned int> SignalManager::selectedChannels()
{
//    unsigned int channel;
//    unsigned int count = 0;
    QArray<unsigned int> list(0); // signal.count());

//    for (channel=0; channel < signal.count(); channel++) {
//	Signal *sig = signal.at(channel);
//	if (!sig) continue;
//	if (!sig->isSelected()) continue;
//	
//	list[count]=channel;
//	count++;
//    }
//
//    list.resize(count);
    return list;
}

//***************************************************************************
int SignalManager::singleSample(unsigned int channel, unsigned int offset)
{
//    ASSERT(channel < signal.count());
//    if (channel >= signal.count()) return 0;
//
//    return signal.at(channel) ?
//	   signal.at(channel)->getSingleSample(offset) : 0;
    return 0;
}

//***************************************************************************
int SignalManager::averageSample(unsigned int offset,
                                 const QArray<unsigned int> *channels)
{
//    unsigned int count = 0;
//    unsigned int channel;
//    double value = 0.0;
//
//    if (channels) {
//	for (count=0; count < channels->size(); count++) {
//	    channel = (*channels)[count];
//	    Signal *sig = signal.at(channel);
//	    if (!sig) continue;
//	    value += sig->getSingleSample(offset);
//	}
//    } else {
//	for (channel=0; channel < signal.count(); channel++) {
//	    Signal *sig = signal.at(channel);
//	    if (!sig) continue;
//	    if (!sig->isSelected()) continue;
//
//	    value += sig->getSingleSample(offset);
//	    count++;
//	}
//    }
//
//    return (count) ? (int)(value/(double)count) : 0;
    return 0;
}

//****************************************************************************
QBitmap *SignalManager::overview(unsigned int width, unsigned int height,
                                 unsigned int offset, unsigned int length)
{
    return 0;
//    QBitmap *overview = new QBitmap(width, height);
//    ASSERT(overview);
//    if (!overview) return 0;
//
//    unsigned int channel;
//    unsigned int left;
//    unsigned int right;
//    unsigned int x;
//    unsigned int channels = m_channels;
//    float samples_per_pixel = (float)(length-1) / (float)(width-1);
//    int min;
//    int max;
//    float scale_y = (float)height / (float)(1 << 24);
//    QPainter p;
//
//    overview->fill(color0);
//
//    p.begin(overview);
//    p.setPen(color1);
//    left = offset;
//    for (x=0; x < width; x++) {
//	right = offset + (unsigned int)((x+1) * samples_per_pixel);
//        // find minimum and maximum over all channels
//        min =  (1<<24);
//        max = -(1<<24);
//        for (channel = 0; channel < channels; channel++) {
//	    int min2 =  (1<<24);
//	    int max2 = -(1<<24);
//	    getMaxMin(channel, max2, min2, left, right-left+1);
//	    if (min2 < min) min = min2;
//	    if (max2 > max) max = max2;
//        }
//
//        // transform min/max into pixel coordinates
//        min = (height >> 1) - (int)(min * scale_y);
//        max = (height >> 1) - (int)(max * scale_y);
//
//        // draw the line between min and max
//        p.drawLine(x, min, x, max);
//
//	left = right+1;
//    }
//    p.end ();
//
//    return overview;
}

//***************************************************************************
int SignalManager::getBitsPerSample()
{
    int max_bps = 0;
//    for (unsigned int i = 0; i < m_channels; i++) {
//	int bps = (signal.at(i)) ? signal.at(i)->bits() : 0;
//	if (bps > max_bps) max_bps = bps;
//    }
    return max_bps;
}

//***************************************************************************
void SignalManager::deleteChannel(unsigned int channel)
{
//    ASSERT(m_channels);
//    if (m_channels <= 1) return; // deleting the last channel is forbidden!
//
//    signal.setAutoDelete(true);
//    signal.remove(channel);
//    signal.setAutoDelete(false);
//
//    m_channels--;                 //decrease number of channels...
//
//    emit sigChannelDeleted(channel);
//    emit signalChanged( -1, -1);
}

//***************************************************************************
void SignalManager::addChannel()
{
//    if (!getLength()) return ;
//    appendChannel(new Signal(getLength(), rate)); ###
}

//***************************************************************************
void SignalManager::toggleChannel(const unsigned int channel)
{
//    ASSERT(channel < signal.count());
//    if (channel >= signal.count()) return;
//    ASSERT(signal.at(channel));
//    if (!signal.at(channel)) return;
//
//    signal.at(channel)->select(!signal.at(channel)->isSelected());
//    debug("SignalManager::toggleChannel(%d): selected=%d",
//	channel, signal.at(channel)->isSelected());
}

//***************************************************************************
bool SignalManager::executeCommand(const QString &command)
{
//    ASSERT(command);
//    if (!command) return false;
//
//    debug("SignalManager::executeCommand(%s)", command.data());    // ###
//
//    if (false) {
//    CASE_COMMAND("playback")
//	playback_param_t &params = KwaveApp::getPlaybackParams();
//	Parser parser(command);
//	
//	params.rate = parser.toInt();
//	params.channels = parser.toInt();
//	params.bits_per_sample = parser.toInt();
//	if (params.device) delete[] params.device;
//	params.device = parser.nextParam();
//	params.bufbase = parser.toInt();
//    CASE_COMMAND("copy")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	ASSERT(globals.clipboard);
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < m_channels; i++) {
//		ASSERT(signal.at(i));
//		if (signal.at(i)) globals.clipboard->appendChannel(
//		    signal.at(i)->copyRange());
//	    }
//	}
//    CASE_COMMAND("cut")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	ASSERT(globals.clipboard);
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < m_channels; i++) {
//		ASSERT(signal.at(i));
//		if (signal.at(i)) globals.clipboard->appendChannel(
//		    signal.at(i)->cutRange());
//	    }
//	}
//	rmarker = lmarker;
//	emit signalChanged(getLMarker(), -1);
//    CASE_COMMAND("crop")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	ASSERT(globals.clipboard);
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < m_channels; i++) {
//		ASSERT(signal.at(i));
//		if (signal.at(i)) (signal.at(i)->cropRange());
//	    }
//	}
//	emit signalChanged( -1, -1);
//    CASE_COMMAND("delete")
//	for (unsigned int i = 0; i < m_channels; i++) {
//	    ASSERT(signal.at(i));
//	    if (signal.at(i)) signal.at(i)->deleteRange();
//	}
//	rmarker = lmarker;
//	emit signalChanged(getLMarker(), -1);
//    CASE_COMMAND("paste")
//	if (globals.clipboard) {
//	    SignalManager *toinsert = globals.clipboard->getSignal();
//	    if (toinsert) {
//		unsigned int clipchan = toinsert->channels();
//		unsigned int sourcechan = 0;
//
//		/* ### check if the signal has to be re-sampled ### */
//		
//		for (unsigned int i = 0; i < m_channels; i++) {
//		    ASSERT(signal.at(i));
//		    if (signal.at(i)) {
//			signal.at(i)->insertPaste(toinsert->getSignal(
//			    sourcechan)
//			);
//		    }
//		    sourcechan++;
//		    sourcechan %= clipchan;
//		}
//		if (toinsert->getLength()) {
//		    rmarker = lmarker + toinsert->getLength() - 1;
//		} else {
//		    rmarker = lmarker;
//		}
//	    }
//	    emit signalChanged(getLMarker(), -1);
//	}
//    CASE_COMMAND("mixpaste")
//	if (globals.clipboard) {
//	    SignalManager *toinsert = globals.clipboard->getSignal();
//	    if (toinsert) {
//		unsigned int clipchan = toinsert->channels();
//		unsigned int sourcechan = 0;
//
//		/* ### check if the signal has to be re-sampled ### */
//
//		for (unsigned int i = 0; i < m_channels; i++) {
//		    ASSERT(signal.at(i));
//		    if (signal.at(i)) {
//			signal.at(i)->mixPaste(
//			    toinsert->getSignal(sourcechan)
//			);
//		    }
//		    sourcechan++;
//		    sourcechan %= clipchan;
//		}
//	    }
//	    emit signalChanged(getLMarker(), -1);
//	}
//    CASE_COMMAND("addchannel")
//	addChannel();
//    CASE_COMMAND("deletechannel")
//	Parser parser(command);
//	unsigned int i = parser.toInt();
//	deleteChannel(i);
//    CASE_COMMAND("selectchannels")
//	for (unsigned int i = 0; i < m_channels; i++)
//	    if (signal.at(i)) signal.at(i)->select(true);
//    CASE_COMMAND("invertchannels")
//	for (unsigned int i = 0; i < m_channels; i++)
//	    toggleChannel(i);
//    } else {
//	bool result = promoteCommand(command);
//	emit signalChanged( -1, -1);
//	return result;
//    }

    return true;
}

//***************************************************************************
bool SignalManager::promoteCommand(const QString &command)
{
//    debug("SignalManager::promoteCommand(%s)", command.data());    // ###
//
////    // loop over all channels
////    unsigned int i;
////    for (i = 0; i < m_channels; i++) {
////        ASSERT(signal.at(i));
////        if (!signal.at(i)) continue;
////
////	// skip channel if not selected
////	if (!signal.at(i)->isSelected()) continue;
////
////	int begin, len;
////	if (lmarker != rmarker) {
////	    begin = lmarker;
////	    len = rmarker - lmarker + 1;
////	} else {
////	    begin = 0;
////	    len = signal.at(i)->getLength();
////	}
////
////	char buf[64];
////	snprintf(buf, sizeof(buf), "%d", i + 1);
////	char *caption = catString(i18n(command), i18n(" on channel "), buf);
////
////// =====================================================
////// === under construction ==============================
////	
////	// create a nice little Object that should contain everything important
////	TimeOperation *operation =
////	    new TimeOperation(signal.at(i), command, begin, len);
////	ASSERT(operation);
////	if (!operation) {
////	    warning("out of memory: could not allocate TimeOperation");
////	    continue;
////	}
////	
////	debug("lmarker=%d, rmarker=%d, begin=%d, len=%d",
////	    lmarker,rmarker,begin,len);
////	
////	//create a new progress dialog, that watches an memory address
////	//that is updated by the modules
////	ProgressDialog *dialog = createProgressDialog(operation, caption);
////	ASSERT(dialog);
//////	if (dialog) {
//////	    // connect the signal for "command done"
//////	    connect(dialog, SIGNAL(commandDone()),
//////		    this, SLOT(commandDone()));
//////		
////#ifdef USE_THREADS
////	    pthread_t thread;
////	    // emit modifyingSignal(signal.at(i), begin, length);
////
//////	extern int pthread_create __P ((pthread_t *__thread,
//////				__const pthread_attr_t *__attr,
//////				void *(*__start_routine) (void *),
//////				void *__arg));
////
////	    //create the new thread
////	    if (pthread_create (&thread,
////				0,
////				(void *(*)(void *))(threadStub),
////				(void *)operation) != 0) {
////		warning("thread creation failed");
////		if (dialog) delete dialog;
////		return false;
////	    }
////#else /* USE_THREADS */
////	    threadStub(operation);
////#endif /* USE_THREADS */
////
////	    if (dialog) delete dialog;
////
////// === under construction ==============================
////// =====================================================
////
//////	}
//////	else warning("out of memory: could not allocate ProgressDialog");
////    }
////
////
////
////    // could not promote command to modules or an error occured
////    if (i < m_channels) return false;

    return true;
}

//***************************************************************************
void SignalManager::commandDone()
{
    debug("SignalManager::commandDone()");    // ###
    emit signalChanged( -1, -1);
}

//***************************************************************************
SignalManager::~SignalManager()
{
    close();
}

//***************************************************************************
void SignalManager::setRange(unsigned int l, unsigned int r)
{
//    for (unsigned int i = 0; i < m_channels; i++) {
//	ASSERT(signal.at(i));
//	if (signal.at(i)) signal.at(i)->setMarkers(l, r);
//    }
//    if (!signal.count()) return;
//    ASSERT(signal.at(0));
//    if (signal.at(0)) {
//	lmarker = signal.at(0)->getLMarker();
//	rmarker = signal.at(0)->getRMarker();
//    }
}

//***************************************************************************
ProgressDialog *SignalManager::createProgressDialog(TimeOperation *operation,
                                                    const char *caption)
{
////    ProgressDialog *dialog = new ProgressDialog (operation, caption);
////    ASSERT(dialog);
////    if (dialog) {
////	dialog->show();
////	return dialog;
////    }
    return 0;
}

//**********************************************************
//below are all methods of Class SignalManager that deal with I/O
//such as loading and saving.

//***************************************************************************
int SignalManager::loadAscii()
{
//    float value;
//    int cnt = 0;
//    float max = 0;
//    float amp;
//    int *sample = 0;
//
//    FILE *sigin = fopen(m_name.data(), "r");
//    if (!sigin) {
//	KMessageBox::error(0, i18n("File does not exist !"), i18n("Info"), 2);
//	return -ENOENT;
//    }
//
//    // scan in the first line for the sample rate
//    // ### to be done ###
//    rate = 44100;        //will be asked in requester
//
//    // scan for number of channels
//    m_channels = 1;
//
//    // loop over all samples in file to get maximum value
//    while (!feof(sigin)) {
//	if (fscanf (sigin, "%e\n", &value) == 1) {
//	    if ( value > max) max = value;
//	    if ( -value > max) max = -value;
//	    cnt++;
//	}
//    }
//    debug("SignalManager::loadAscii(): reading ascii file with %d samples",
//	  cnt);    // ###
//
//    // get the maximum and the scale
//    amp = (float)((1 << 23)-1) / max;
//    Signal *new_signal = new Signal(cnt, rate);
//    ASSERT(new_signal);
//
//    if (new_signal) {
//	signal.append(new_signal);
//	new_signal->setBits(24);
//	sample = new_signal->getSample();
//    }
//
//    if (sample) {
//	fseek (sigin, 0, SEEK_SET);    //seek to beginning
//	cnt = 0;
//	while (!feof(sigin)) {
//	    if (fscanf(sigin, "%e\n", &value) == 1) {
//		sample[cnt++] = (int)(value * amp);
//	    }
//	}
//    }
//
//    fclose (sigin);

    return 0;
}

//***************************************************************************
int SignalManager::loadWav()
{
    return 1234;
//    wav_fmt_header_t fmt_header;
//    int result = 0;
//    __uint32_t num;
//    __uint32_t length;
//
//    QFile sigfile(m_name);
//    if (!sigfile.open(IO_ReadOnly)) {
//	KMessageBox::error(m_parent_widget,
//		i18n("File does not exist !"), i18n("Error"), 2);
//	return -ENOENT;
//    }
//
//    // --- check if the file starts with "RIFF" ---
//    num = sigfile.size();
//    length = findChunk(sigfile, "RIFF", 0);
//    if ((length == 0) || (sigfile.at() != 8)) {
//	KMessageBox::error(m_parent_widget,
//	    i18n("File is no RIFF File !"), i18n("Warning"), 2);
//	// maybe recoverable...
//    } else if (length+8 != num) {
//	KMessageBox::error(m_parent_widget,
//	    i18n("File has incorrect length! (maybe truncated?)"),
//	    i18n("Warning"), 2);
//	// maybe recoverable...
//    } else {
//	// check if the chunk data contains "WAVE"
//	char file_type[16];
//	num = sigfile.readBlock((char*)(&file_type), 4);
//	if ((num != 4) || strncmp("WAVE", file_type, 4)) {
//	    KMessageBox::error(m_parent_widget,
//		i18n("File is no WAVE File !"),
//		i18n("Warning"), 2);
//	    // maybe recoverable...
//	}
//    }
//
//    // ------- read the "fmt " chunk -------
//    ASSERT(sizeof(fmt_header) == 16);
//    num = findChunk(sigfile, "fmt ");
//    if (num != sizeof(fmt_header)) {
//	debug("SignalManager::loadWav(): length of fmt chunk = %d", num);
//	KMessageBox::error(m_parent_widget,
//	    i18n("File does not contain format information!"),
//	    i18n("Error"), 2);
//	return -EMEDIUMTYPE;
//    }
//    num = sigfile.readBlock((char*)(&fmt_header), sizeof(fmt_header));
//#ifdef IS_BIG_ENDIAN
//    fmt_header.length = bswap_32(fmt_header.length);
//    fmt_header.mode = bswap_16(fmt_header.mode);
//    fmt_header.channels = bswap_16(fmt_header.channels);
//    fmt_header.rate = bswap_32(fmt_header.rate);
//    fmt_header.AvgBytesPerSec = bswap_32(fmt_header.AvgBytesPerSec);
//    fmt_header.BlockAlign = bswap_32(fmt_header.BlockAlign);
//    fmt_header.bitspersample = bswap_16(fmt_header.bitspersample);
//#endif
//    if (fmt_header.mode != 1) {
//	KMessageBox::error(m_parent_widget,
//	    i18n("File must be uncompressed (Mode 1) !"),
//	    i18n("Sorry"), 2);
//	return -EMEDIUMTYPE;
//    }
//    rate = fmt_header.rate;
//
//    // ------- search for the data chunk -------
//    length = findChunk(sigfile, "data");
//    if (!length) {
//	KMessageBox::error(m_parent_widget,
//	    i18n("File does not contain data!"),
//	    i18n("Error"), 2);
//	return -EMEDIUMTYPE;
//    }
//
//    length = (length/(fmt_header.bitspersample/8))/fmt_header.channels;
//    switch (fmt_header.bitspersample) {
//	case 8:
//	case 16:
//	case 24:
////	    result = loadWavChunk(sigfile, length,
////				  fmt_header.channels,
////				  fmt_header.bitspersample);
//	    break;
//	default:
//	    KMessageBox::error(m_parent_widget,
//		i18n("Sorry only 8/16/24 Bits per Sample"\
//		" are supported !"), i18n("Sorry"), 2);
//	    result = -EMEDIUMTYPE;
//    }
//
//    return result;
}

//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
//***************************************************************************
void SignalManager::exportAscii(const char *name)
{
//    ASSERT(name);
//    if (!name) return ;
//
//    unsigned int length = getLength();
//    ASSERT(length);
//    if (!length) return;
//
//    ASSERT(m_channels);
//    if (!m_channels) return;
//
//    FILE *sigout = fopen(name, "w");
//    ASSERT(sigout);
//    if (!sigout) return;
//
//    //prepare and show the progress dialog
//    char progress_title[256];
//    char str_channels[128];
//    if (m_channels == 1)
//	strncpy(str_channels, i18n("Mono"), sizeof(str_channels));
//    else if (m_channels == 2)
//	strncpy(str_channels, i18n("Stereo"), sizeof(str_channels));
//    else
//	snprintf(str_channels, sizeof(str_channels), "%d-channel", m_channels);
//    snprintf(progress_title, sizeof(progress_title),
//	i18n("Exporting %s ASCII file :"),
//	str_channels);
//
//    QString title = i18n(progress_title);
//    ProgressDialog *dialog = new ProgressDialog(100, title);
//    delete[] title;
//    if (dialog) dialog->show();
//
//    // loop for writing data
//    int *sample;
//    int percent_count = 0;
//    const double scale_y = (1 << 23)-1;
//    for (unsigned int pos = 0; pos < length ; pos++) {
//	// loop over all channels
//	for (unsigned int channel=0; channel < m_channels; channel++) {
//	    sample = signal.at(channel)->getSample();
//	    if (!sample) continue;
//	
//	    if (channel != 0) fprintf(sigout, ",");
//	    fprintf(sigout, "%0.8e", (double)sample[pos]/scale_y);
//	}
//	fprintf(sigout,"\n");
//
//	// update the progress bar
//	percent_count--;
//	if (dialog && (percent_count <= 0)) {
//	    percent_count = length / 200;
//	    float percent = (float)pos;
//	    percent /= (float)length;
//	    percent *= 100.0;
//	    dialog->setProgress (percent);
//	}
//    }
//
//    if (dialog) delete dialog;
//    fclose(sigout);
}

//***************************************************************************
int SignalManager::writeWavChunk(FILE *sigout, unsigned int begin,
                                 unsigned int length, int bits)
{
// ### not ported yet ###
//    unsigned int bufsize = 16 * 1024 * sizeof(int);
//    unsigned char *savebuffer = 0;
//    sample_t **sample = 0;    // array of pointers to samples
//    int bytes = bits / 8;
//    int bytes_per_sample = bytes * m_channels;
//    bufsize -= bufsize % bytes_per_sample;
//
//    // try to allocate memory for the save buffer
//    // if failed, try again with the half buffer size as long
//    // as <1kB is not reached (then we are really out of memory)
//    while (savebuffer == 0) {
//	if (bufsize < 1024) {
//	    debug("SignalManager::writeWavSignal:not enough memory for buffer");
//	    return -ENOMEM;
//	}
//	savebuffer = new unsigned char[bufsize];
//	if (!savebuffer) {
//	    bufsize >>= 1;
//	    bufsize -= bufsize % bytes_per_sample;
//	}
//    }
//
//    //prepare and show the progress dialog
//    char progress_title[256];
//    char str_channels[128];
//    if (m_channels == 1)
//	strncpy(str_channels, i18n("Mono"), sizeof(str_channels));
//    else if (m_channels == 2)
//	strncpy(str_channels, i18n("Stereo"), sizeof(str_channels));
//    else
//	snprintf(str_channels, sizeof(str_channels), "%d-channel", m_channels);
//
//    snprintf(progress_title, sizeof(progress_title), "Saving %d-Bit-%s File :",
//	    bits, str_channels);
//
//    QString title = i18n(progress_title);
//    ProgressDialog *dialog = new ProgressDialog (100, title);
//    if (dialog) dialog->show();
//
//    // prepare the store loop
//    int percent_count = length / 200;
//    unsigned int shift = 24-bits;
//
//    sample = new (int*)[m_channels];
//    for (unsigned int channel = 0; channel < m_channels; channel++) {
//	sample[channel] = signal.at(channel)->getSample();
//	ASSERT(sample[channel]);
//	if (!sample[channel]) {
//	    KMessageBox::error(m_parent_widget,
//		i18n("empty channel detected, channel count reduced by one"),
//		i18n("Warning"), 2);
//	    m_channels--;
//	}
//    }
//
//    // loop for writing data
//    for (unsigned int pos = begin; pos < length; ) {
//	unsigned char *buf = savebuffer;
//	unsigned int nsamples = 0;
//
//	while (pos < length && (nsamples < bufsize / bytes_per_sample)) {
//	    for (unsigned int channel = 0; channel < m_channels; channel++) {
//		int *smp = sample[channel] + pos;
//		int act = (*smp) >> shift;
//		if (bytes == 1) {
//		    // 8 bits -> unsigned
//		    *(buf++) = (char)((act - 128) & 0xFF);
//		} else {
//		    // >= 16 bits -> signed
//		    for (register int byte = bytes; byte; byte--) {
//			*(buf++) = (char)(act & 0xFF);
//			act >>= 8;
//		    }
//		}
//	    }
//	    nsamples++;
//	    pos++;
//	}
//
//	int written_bytes = fwrite(savebuffer,
//				   bytes_per_sample, nsamples, sigout);
//
//	percent_count -= written_bytes;
//	if (dialog && (percent_count <= 0)) {
//	    percent_count = length / 200;
//	    float percent = (float)pos;
//	    percent /= (float)length;
//	    percent *= 100.0;
//	    dialog->setProgress (percent);
//	}
//    }
//
//    if (sample) delete[] sample;
//    if (dialog) delete dialog;
//    if (savebuffer) delete[] savebuffer;
    return 0;
}

//***************************************************************************
void SignalManager::save(const char *filename, int bits, bool selection)
{
// ### not ported yet ###
//    debug("SignalManager::save(): %d Bit to %s ,%d", bits, filename, selection);
//
//    __uint32_t begin = 0;
//    __uint32_t length = this->getLength();
//    wav_header_t header;
//
//    ASSERT(filename);
//    if (!filename) return;
//
//    if (selection && (lmarker != rmarker)) {
//	begin = lmarker;
//	length = rmarker - lmarker + 1;
//    }
//
//    FILE *sigout = fopen(filename, "w");
//    if (name) delete[] name;
//    name = duplicateString(filename);
//    ASSERT(name);
//    if (!name) return;
//
//    if (!m_channels) KMsgBox(0, i18n("info"),
//	i18n("Signal is empty, nothing to save !"), 2);
//
//    if (sigout) {
//	__uint32_t datalen = (bits >> 3) * length * m_channels;
//
//	strncpy((char*)&(header.riffid), "RIFF", 4);
//	strncpy((char*)&(header.wavid), "WAVE", 4);
//	strncpy((char*)&(header.fmtid), "fmt ", 4);
//	header.filelength = datalen + sizeof(struct wavheader);
//	header.fmtlength = 16;
//	header.mode = 1;
//	header.channels = m_channels;
//	header.rate = rate;
//	header.AvgBytesPerSec = rate * (bits >> 3) * m_channels;
//	header.BlockAlign = (bits >> 3) * m_channels;
//	header.bitspersample = bits;
//
//#if defined(IS_BIG_ENDIAN)
//	header.filelength = bswap_32(header.filelength);
//	header.fmtlength = bswap_32(header.fmtlength);
//	header.mode = bswap_16(header.mode);
//	header.channels = bswap_16(header.channels);
//	header.rate = bswap_32(header.rate);
//	header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
//	header.BlockAlign = bswap_16(header.BlockAlign);
//	header.bitspersample = bswap_16(header.bitspersample);
//	datalen = bswap_32(datalen);
//#endif
//
//	fseek (sigout, 0, SEEK_SET);
//	fwrite ((char *) &header, 1, sizeof (struct wavheader), sigout);
//	fwrite ("data", 1, 4, sigout);
//	fwrite ((char *)&datalen, 1, 4, sigout);
//
//	switch (bits) {
//	    case 8:
//	    case 16:
//	    case 24:
//		writeWavChunk(sigout, begin, length, bits);
//		break;
//	    default:
//		KMsgBox::message(m_parent_widget, i18n("Info"),
//		    i18n("Sorry only 8/16/24 Bits per Sample are supported !"),
//		    2);
//	    break;
//	}
//
//	fclose(sigout);
//    }
}

//***************************************************************************
__uint32_t SignalManager::findChunk(QFile &sigfile, const char *chunk,
	__uint32_t offset)
{
    char current_name[16];
    __uint32_t length = 0;
    int len;

    ASSERT(sizeof(length) == 4);
    ASSERT(sizeof(int) == 4);

    sigfile.at(offset);
    while (!sigfile.atEnd()) {
	debug("findChunk('%s'): position=%u", chunk, sigfile.at());
	// get name of the chunk
	len = sigfile.readBlock((char*)(&current_name), 4);
	if (len < 4) {
	    debug("findChunk('%s'): not found, reached EOF while reading name",
	    	chunk);
	    return 0; // reached EOF
	}

	// get length of the chunk
	len = sigfile.readBlock((char*)(&length), sizeof(length));
	if (len < 4) {
	    debug("findChunk('%s'): not found, reached EOF :-(", chunk);
	    return 0; // reached EOF
	}
#ifdef IS_BIG_ENDIAN
	length = bswap_32(length);
#endif

	if (strncmp(chunk, current_name, 4) == 0) {
	    // chunk found !
	    debug("findChunk('%s'): found chunk with len=%d", chunk, length);
	    return length;
        } else {
	    debug("findChunk('%s'): skipping '%c%c%c%c'",
		chunk, current_name[0], current_name[1],
		current_name[2], current_name[3]);
        }

	// not found -> skip
	sigfile.at(sigfile.at()+length);
    };

    debug("findChunk('%s'): not found :-(", chunk);
    return 0;
}

//***************************************************************************
int SignalManager::loadWavChunk(QFile &sigfile, unsigned int length,
                                unsigned int channels, int bits)
{
//    unsigned int bufsize = 16 * 1024 * sizeof(sample_t);
//    unsigned char *loadbuffer = 0;
//    int bytes = bits >> 3;
//    unsigned int sign = 1 << (24-1);
//    unsigned int negative = ~(sign - 1);
//    unsigned int shift = 24-bits;
//    unsigned int bytes_per_sample = bytes * channels;
//    unsigned int max_samples = bufsize / bytes_per_sample;
//    long int start_offset = ftell(sigfile);
//
//    ASSERT(bytes);
//    ASSERT(channels);
//    ASSERT(length);
//    if (!bytes || !channels || !length) return -EINVAL;
//
//    // try to allocate memory for the load buffer
//    // if failed, try again with the half buffer size as long
//    // as <1kB is not reached (then we are really out of memory)
//    while (loadbuffer == 0) {
//	if (bufsize < 1024) {
//	    debug("SignalManager::loadWavChunk:not enough memory for buffer");
//	    return -ENOMEM;
//	}
//	loadbuffer = new unsigned char[bufsize];
//	if (!loadbuffer) bufsize >>= 1;
//    }
//
//    // check if the file is large enough for "length" samples
//    fseek(sigfile, 0, SEEK_END);
//    size_t file_rest = ftell(sigfile) - start_offset;
//    fseek(sigfile, start_offset, SEEK_SET);
//    if (length > file_rest/bytes_per_sample) {
//	debug("SignalManager::loadWavChunk: "\
//	      "length=%d, rest of file=%d",length,file_rest);
//	KMessageBox::error(m_parent_widget,
//	    i18n("Warning"),
//	    i18n("Error in input: file is smaller than stated "\
//	    "in the header. \n"\
//	    "File will be truncated."), 2);
//	length = file_rest/bytes_per_sample;
//    }
//
//    QList<SampleInputStream> samples;
//    samples.setAutoDelete(true);
//
//    for (unsigned int channel = 0; channel < channels; channel++) {
//	Track *new_track = m_signal.appendTrack(length);
//	ASSERT(new_track);
//	if (!new_track) {
//	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"),
//	    	i18n("Warning"), 2);
//	    return -ENOMEM;
//	}
//
//	SampleInputStream *s = m_signal.openInputStream(channel, Overwrite);
//	ASSERT(s);
//	if (!s) {
//	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"),
//	    	i18n("Warning"), 2);
//	    return -ENOMEM;
//	}
//	samples.append(s);
//    }
//
//    //prepare and show the progress dialog
//    QString title;
//    title = "Loading %1-Bit (%2) File: %3";
//    title = title.arg(bits);
//    switch (channels) {
//	case 1:
//	    title = title.arg(i18n("Mono"));
//	    break;
//	case 2:
//	    title = title.arg(i18n("Stereo"));
//	    break;
//	case 4:
//	    title = title.arg(i18n("Quadro"));
//	    break;
//	default:
//	    title = title.arg(i18n("%1 tracks"));
//	    title = title.arg(channels);
//    }
//    title = title.arg(m_name);
//    FileProgress *dialog = new FileProgress(m_parent_widget);
//    ASSERT(dialog);
//    if (dialog) {
//	dialog->setCaption(title);
//	dialog->setFixedWidth(400/* ###dialog->sizeHint().width() */);
//	dialog->show();
//    }
//
//    //prepare the load loop
//    int percent_count = length / 1000;
//
//    // debug("sign=%08X, negative=%08X, shift=%d",sign,negative,shift);
//
//    for (unsigned int pos = 0; pos < length; ) {
//	// limit reading to end of wav chunk length
//	if ((pos + max_samples) > length) max_samples=length-pos;
//
//	int read_samples = fread((char *)loadbuffer, bytes_per_sample,
//				 max_samples, sigfile);
//	percent_count -= read_samples;
//	// debug("read %d samples", read_samples);
//	if (read_samples <= 0) {
//	    warning("SignalManager::loadWavChunk:EOF reached?"\
//		    " (at sample %ld, expected length=%d",
//		    ftell(sigfile) / bytes_per_sample - start_offset, length);
//	    break;
//	}
//
//	unsigned char *buffer = loadbuffer;
//	sample_t s = 0;
//	while (read_samples--) {
//	    for (register unsigned int channel = 0;
//	         channel < channels;
//	         channel++)
//	    {
//		SampleInputStream *stream = samples.at(channel);
//		if (bytes == 1) {
//		    // 8-bit files are always unsigned !
//		    s = (*(buffer++) - 128) << shift;
//		} else {
//		    // >= 16 bits is signed
//		    for (register int byte = 0; byte < bytes; byte++)
//			s |= *(buffer++) << ((byte << 3) + shift);
//		    // sign correcture for negative values
//		    if ((unsigned int)s & sign)
//			s |= negative;
//		}
//		*stream << s;
//	    }
//	    pos++;
//	}
//
//	if (dialog && (percent_count <= 0)) {
//	    percent_count = length / 1000;
//	    float percent = (float)pos;
//	    percent /= (float)length;
//	    percent *= 100.0;
//	    dialog->setValue(percent);
//	}
//    }
//
//    // close all sample input streams
//    samples.clear();
//
//    if (dialog) delete dialog;
//    if (loadbuffer) delete[] loadbuffer;
    return 0;
}

/***************************************************************************/
//below  all methods of Class SignalManager that deal with sound playback

//***************************************************************************
////following are the playback routines
//struct Play {
//    SignalManager *manage;
//    playback_param_t params;
//    unsigned int start;
//    bool loop;
//};
//
//***************************************************************************
//void playThread(struct Play *par)
//{
//    ASSERT(par);
//    if (!par) return;
//    ASSERT(par->manage);
//    if (!par->manage) return;
//
//    int device; // handle of the playback device
//    unsigned char *buffer = 0;
//    unsigned int bufsize;
//
//    // prepeare for playback by opening the sound device
//    // and initializing with the proper settings
//    if ((device = open(par->params.device, O_WRONLY)) != -1 ) {
//	bufsize = par->manage->setSoundParams(device,
//	    par->params.bits_per_sample,
//	    par->params.channels,
//	    par->params.rate, par->params.bufbase);
//
//	if (bufsize) {
//	    buffer = new unsigned char[bufsize];
//	    ASSERT(buffer);
//	    if (buffer) {
//		par->manage->playback(device, par->params,
//		    buffer, bufsize, par->start, par->loop);
//		delete[] buffer;
//	    }
//	} else {
//	    // simulate playback done
//	    par->manage->m_spx_playback_done.AsyncHandler();
//	}
//	close (device);
//    } else {
//	warning("SignalManager::playback(): unable to open device");
//    }
//
//    par->manage->msg[stopprocess] = true;
//    par->manage->msg[processid] = 0;
//
//    if (par) delete par;
//
//    return;
//}

//***************************************************************************
void SignalManager::startplay(unsigned int start, bool loop)
{
//    msg[processid] = 1;
//    msg[stopprocess] = false;
//
//    Play *par = new Play;
//    pthread_t thread;
//
//    par->manage = this;
//    par->start = start;
//    par->loop = loop;
//    par->params = KwaveApp::getPlaybackParams();
//    par->params.rate = this->getRate();
//
//    m_playback_error = 0;
//    pthread_create(&thread, 0, (void * (*) (void *))playThread, par);
}

//***************************************************************************
void SignalManager::stopplay()
{
//    msg[stopprocess] = true;          //set flag for stopping
//
//    QTimer timeout;
//    timeout.start(5000, true);
//    while (msg[processid] != 0) {
//	sched_yield();
//	// wait for termination
//	if (!timeout.isActive()) {
//	    warning("SignalManager::stopplay(): TIMEOUT");
//	    break;
//	}
//    }
//    debug("SignalManager::stopplay(): threads stopped");
}

//***************************************************************************
int SignalManager::setSoundParams(int audio, int bitspersample,
                                  unsigned int channels, int rate,
                                  int bufbase)
{
    return 0;
//    const char *trouble = i18n("playback problem");
//
//    debug("SignalManager::setSoundParams(fd=%d,bps=%d,channels=%d,"\
//	"rate=%d, bufbase=%d)", audio, bitspersample, channels,
//	rate, bufbase);
//
//// ### under construction ###
//
//// from standard oss interface (linux/soundcard.h)
//
/////*	Audio data formats (Note! U8=8 and S16_LE=16 for compatibility) */
////#define SNDCTL_DSP_GETFMTS		_SIOR ('P',11, int) /* Returns a mask */
////#define SNDCTL_DSP_SETFMT		_SIOWR('P',5, int) /* Selects ONE fmt*/
////#	define AFMT_QUERY		0x00000000	/* Return current fmt */
////#	define AFMT_MU_LAW		0x00000001
////#	define AFMT_A_LAW		0x00000002
////#	define AFMT_IMA_ADPCM		0x00000004
////#	define AFMT_U8			0x00000008
////#	define AFMT_S16_LE		0x00000010	/* Little endian signed 16*/
////#	define AFMT_S16_BE		0x00000020	/* Big endian signed 16 */
////#	define AFMT_S8			0x00000040
////#	define AFMT_U16_LE		0x00000080	/* Little endian U16 */
////#	define AFMT_U16_BE		0x00000100	/* Big endian U16 */
////#	define AFMT_MPEG		0x00000200	/* MPEG (2) audio */
//
//// from ALSA interface (asound.h)
//
////#define SND_PCM_SFMT_S8			0
////#define SND_PCM_SFMT_U8			1
////#define SND_PCM_SFMT_S16		SND_PCM_SFMT_S16_LE
////#define SND_PCM_SFMT_U16		SND_PCM_SFMT_U16_LE
////#define SND_PCM_SFMT_S24		SND_PCM_SFMT_S24_LE
////#define SND_PCM_SFMT_U24		SND_PCM_SFMT_U24_LE
////#define SND_PCM_SFMT_S32		SND_PCM_SFMT_S32_LE
////#define SND_PCM_SFMT_U32		SND_PCM_SFMT_U32_LE
////#define SND_PCM_SFMT_FLOAT		SND_PCM_SFMT_FLOAT_LE
////#define SND_PCM_SFMT_FLOAT64		SND_PCM_SFMT_FLOAT64_LE
////#define SND_PCM_SFMT_IEC958_SUBFRAME	SND_PCM_SFMT_IEC958_SUBFRAME_LE
//
//    int format = (bitspersample == 8) ? AFMT_U8 : AFMT_S16_LE;
//
//    // number of bits per sample
//    if (ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &format) == -1) {
//	m_playback_error = i18n("number of bits per samples not supported");
//	return 0;
//    }
//
//    // mono/stereo selection
//    int stereo = (channels >= 2) ? 1 : 0;
//    if (ioctl(audio, SNDCTL_DSP_STEREO, &stereo) == -1) {
//	m_playback_error = i18n("stereo not supported");
//	return 0;
//    }
//
//    // playback rate
//    if (ioctl(audio, SNDCTL_DSP_SPEED, &rate) == -1) {
//	m_playback_error = i18n("playback rate not supported");
//	return 0;
//    }
//
//    // buffer size
//    ASSERT(bufbase >= MIN_PLAYBACK_BUFFER);
//    ASSERT(bufbase <= MAX_PLAYBACK_BUFFER);
//    if (bufbase < MIN_PLAYBACK_BUFFER) bufbase = MIN_PLAYBACK_BUFFER;
//    if (bufbase > MAX_PLAYBACK_BUFFER) bufbase = MAX_PLAYBACK_BUFFER;
//    if (ioctl(audio, SNDCTL_DSP_SETFRAGMENT, &bufbase) == -1) {
//	m_playback_error = i18n("unusable buffer size");
//	return 0;
//    }
//
//    // return the buffer size in bytes
//    int size;
//    ioctl(audio, SNDCTL_DSP_GETBLKSIZE, &size);
//    return size;
}

//***************************************************************************
void SignalManager::playback(int device, playback_param_t &param,
                             unsigned char *buffer, unsigned int bufsize,
                             unsigned int start, bool loop)
{
//    ASSERT(buffer);
//    ASSERT(bufsize);
//    ASSERT(param.channels);
//    if (!buffer || !bufsize || !param.channels) return;
//
//    unsigned int i;
//    unsigned int active_channels = 0;
//    unsigned int in_channels = m_channels;
//    unsigned int out_channels = param.channels;
//    unsigned int active_channel[in_channels]; // list of active channels
//
//    // get the number of channels with enabled playback
//    for (i=0; i < in_channels; i++) {
//	if (!signal.at(i)) continue;
//	// ### TODO: use state of play widget instead of "enabled" ###
//	if (!signal.at(i)->isSelected()) continue;
//
//	active_channel[active_channels++] = i;
//    }
//
//    // abort if no channels -> nothing to do
//    if (!active_channels) {
//	debug("SignalManager::playback(): no active channel, nothing to do");
//	msg[stopprocess] = true;
//    }
//
//    // set up the matrix for channel mixing
//    int matrix[active_channels][out_channels];
//    unsigned int x, y;
//    for (y=0; y < out_channels; y++) {
//	unsigned int m1, m2;
//	m1 = y * active_channels;
//	m2 = (y+1) * active_channels;
//	
//	for (x=0; x < active_channels; x++) {
//	    unsigned int n1, n2;
//	    n1 = x * out_channels;
//	    n2 = (x+1) * out_channels;
//
//	    // get the common area of [n1..n2] and [m1..m2]
//	    unsigned int left = max(n1, m1);
//	    unsigned int right = min(n2, m2);
//
//	    matrix[x][y] = (right > left) ? (right-left) : 0;
//	}
//    }
//
//    // loop until process is stopped
//    // or run once if not in loop mode
//    unsigned int pointer = start;
//    unsigned int last = rmarker;
//    int samples[active_channels];
//
//    if (lmarker == rmarker) last = getLength()-1;
//    m_spx_playback_pos.enqueue(pointer);
//    do {
//
//	while ((pointer <= last) && !msg[stopprocess]) {
//	
//	    // fill the buffer with audio data
//	    unsigned int cnt;
//	    for (cnt = 0; (cnt < bufsize) && (pointer <= last); pointer++) {
//                unsigned int channel;
//
//		for (y=0; y < out_channels; y++) {
//		    double s = 0;
//		    for (x=0; x < active_channels; x++) {
//			s += signal.at(
//				active_channel[x])->getSingleSample(
//				pointer) * matrix[x][y];
//		    }
//		    samples[y] = (int)(s / active_channels);
//		}
//
//		for (channel=0; channel < out_channels; channel++) {
//		    int sample = samples[channel];
//		
//		    switch (param.bits_per_sample) {
//			case 8:
//			    sample += 1 << 23;
//			    buffer[cnt++] = sample >> 16;
//			    break;
//			case 16:
//			    sample += 1 << 23;
//			    buffer[cnt++] = sample >> 8;
//			    buffer[cnt++] = (sample >> 16) + 128;
//			    break;
//			case 24:
//			    // play in 32 bit format
//			    buffer[cnt++] = 0x00;
//			    buffer[cnt++] = sample & 0x0FF;
//			    buffer[cnt++] = sample >> 8;
//			    buffer[cnt++] = (sample >> 16) + 128;
//
//			    break;
//			default:
//			    // invalid bits per sample
//			    msg[stopprocess] = true;
//			    pointer = last;
//			    cnt = 0;
//			    break;
//		    }
//		}
//	    }
//
//	    // write buffer to the playback device
//	    write(device, buffer, cnt);
//	    m_spx_playback_pos.enqueue(pointer);
//	}
//	
//	// maybe we loop. in this case the playback starts
//	// again from the left marker
//	if (loop && !msg[stopprocess]) pointer = lmarker;
//
//    } while (loop && !msg[stopprocess]);
//
//    // playback is done
//    m_spx_playback_done.AsyncHandler();
}

//***************************************************************************
void SignalManager::updatePlaybackPos()
{
//    unsigned int count = m_spx_playback_pos.count();
//    unsigned int pos = 0;
//
//    // dequeue all pointers and keep the latest one
//    if (!count) return;
//    while (count--) {
//	unsigned int *ppos = m_spx_playback_pos.dequeue();
//	ASSERT(ppos);
//	if (!ppos) continue;
//	pos = *ppos;
//	delete ppos;
//    }
//
//    emit sigPlaybackPos(pos);
}

//***************************************************************************
void SignalManager::forwardPlaybackDone()
{
//    emit sigPlaybackDone();
}

//***************************************************************************
//***************************************************************************
