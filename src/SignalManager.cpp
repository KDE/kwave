#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <byteswap.h>
#include <limits.h>
#include <math.h>

#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include <kmsgbox.h>

#include <libkwave/Signal.h>
#include <libkwave/DynamicLoader.h>
#include <libkwave/TimeOperation.h>
#include <libkwave/Global.h>
#include <libkwave/DialogOperation.h>
#include <libkwave/Parser.h>
#include <libkwave/FileFormat.h>

#include "sampleop.h"

#include "ClipBoard.h"
#include "ProgressDialog.h"
#include "SignalManager.h"
#include "SignalWidget.h"

extern int play16bit;
extern int bufbase;
extern Global globals;

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define processid       0
#define stopprocess     1
#define samplepointer   2

#define CASE_COMMAND(x) } else if (matchCommand(command, x)) {

//**********************************************************
void threadStub(TimeOperation *obj)
{
    debug("thread started");

    ASSERT(obj);
    if (obj) {
	ASSERT(obj->getSignal());
	if (obj->getSignal()) {
	    (obj->getSignal())->command(obj);
	}
    }
    debug("thread done");
}

//**********************************************************
SignalManager::SignalManager(Signal *sig)
    :QObject()
{
    initialize();
    ASSERT(sig);
    if (!sig) return;

    signal.append(sig);
    selected.append(new bool(true));
    this->channels = 1;
    this->rate = sig->getRate();
}

//**********************************************************
SignalManager::SignalManager(unsigned int numsamples,
                             int rate, unsigned int channels)
    :QObject()
{
    initialize();

//    if (channels > MAXCHANNELS) channels = MAXCHANNELS;
//    this->channels = channels;    //store how many channels are linked to this
    this->rate = rate;

    for (unsigned int i = 0; i < channels; i++) {
	Signal *new_signal = new Signal(numsamples, rate);
	ASSERT(new_signal);
	signal.append(new_signal);
	selected.append(new bool(true));
    }
}

//**********************************************************
SignalManager::SignalManager(const char *name, int type)
    :QObject()
{
    debug("KMsgBox::message(...)");
    initialize();
    ASSERT(name);
    this->name = duplicateString(name);

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

//**********************************************************
void SignalManager::initialize()
{
    name = 0;
    lmarker = 0;
    rmarker = 0;
    channels = 0;
    rate = 0;
    for (unsigned int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
	msg[i] = 0;

    selected.setAutoDelete(true);
    selected.clear();

    signal.setAutoDelete(false);
    signal.clear();
}

//**********************************************************
void SignalManager::getMaxMin(unsigned int channel, int &max, int &min,
                              unsigned int begin, unsigned int len)
{
    ASSERT(channel >= 0);
    ASSERT(channel < signal.count());
    if (channel >= signal.count()) return;
    ASSERT(signal.at(channel));
    if (!signal.at(channel)) return;

    unsigned int sig_len;
    sig_len = signal.at(channel)->getLength();
    ASSERT(sig_len);

    if (begin + len > sig_len) len = sig_len - begin;

    signal.at(channel)->getMaxMin(max, min, begin, len);
}

//**********************************************************
unsigned int SignalManager::getLength()
{
    ASSERT(signal.count());
    if (!signal.count()) return 0;
    return signal.at(0) ? signal.at(0)->getLength() : 0;
}

//**********************************************************
int SignalManager::getSingleSample(unsigned int channel, unsigned int offset)
{
    ASSERT(channel >= 0);
    ASSERT(channel < signal.count());
    if ((channel < 0) || (channel >= signal.count())) return 0;

    return signal.at(channel) ?
	   signal.at(channel)->getSingleSample(offset) : 0;
}

//**********************************************************
int SignalManager::getBitsPerSample()
{
    int max_bps = 0;
    for (unsigned int i = 0; i < channels; i++) {
	int bps = (signal.at(i)) ? signal.at(i)->getBits() : 0;
	if (bps > max_bps) max_bps = bps;
    }
    return max_bps;
}

//**********************************************************
void SignalManager::deleteChannel(unsigned int channel)
{
    signal.setAutoDelete(true);
    signal.remove(channel);
    signal.setAutoDelete(false);

    selected.setAutoDelete(true);
    selected.remove(channel);

    channels--;                 //decrease number of channels...

    emit sigChannelDeleted(channel);
    emit signalChanged( -1, -1);
}

//**********************************************************
void SignalManager::addChannel()
{
//    if (!getLength()) return ;
//
//    signal.append(new Signal(getLength(), rate));
//    selected.append(new bool(true));
//    channels++;                 //increase number of channels...
//
//    emit sigChannelAdded(signal.count()-1);
//    emit signalChanged( -1, -1);
}

//**********************************************************
void SignalManager::appendChannel(Signal *newsig)
{
//    signal.append(newsig);
//    selected.append(new bool(true));
//    channels++;
//
//    emit sigChannelAdded(signal.count()-1);
//    emit signalChanged( -1, -1);
}

//**********************************************************
void SignalManager::setOp(int id)
{
//    stopplay();           //every operation cancels playing...
//
//    //decode dynamical allocated menu id's
//    //into the ones used by the switch statement below
//
//    int loop = false;
//    switch (id) {
//	    case LOOP:
//	    loop = true;
//	    case PLAY:
//	    play (loop);
//	    break;
//    }
//
//    // chaos is about to come
//    // check for ranges of id that have to be decoded into a parameter
//
//    if ((id >= TOGGLECHANNEL) && (id < TOGGLECHANNEL + 10))
//	toggleChannel (id - TOGGLECHANNEL);
}

//**********************************************************
bool SignalManager::executeCommand(const char *command)
{
    ASSERT(command);
    if (!command) return false;

    debug("SignalManager::executeCommand(%s)", command);    // ###

    if (false) {
//    CASE_COMMAND("copy")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < channels; i++)
//		globals.clipboard->appendChannel(
//			signal.at(i)->copyRange());
//	}
//    CASE_COMMAND("cut")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < channels; i++)
//		globals.clipboard->appendChannel(
//			signal.at(i)->cutRange());
//	}
//	emit signalChanged(getLMarker(), -1);
//    CASE_COMMAND("crop")
//	if (globals.clipboard) delete globals.clipboard;
//	globals.clipboard = new ClipBoard();
//	if (globals.clipboard) {
//	    for (unsigned int i = 0; i < channels; i++)
//		(signal.at(i)->cropRange());
//	}
//	emit signalChanged( -1, -1);
    CASE_COMMAND("delete")
	for (unsigned int i = 0; i < channels; i++) {
	    ASSERT(signal.at(i));
	    if (signal.at(i)) signal.at(i)->deleteRange();
	}
	emit signalChanged(getLMarker(), -1);
//    CASE_COMMAND("paste")
//	if (globals.clipboard) {
//	    SignalManager *toinsert = globals.clipboard->getSignal();
//	    if (toinsert) {
//		unsigned int clipchan = toinsert->getChannelCount();
//		unsigned int sourcechan = 0;
//
//		for (unsigned int i = 0; i < channels; i++) {
//		    signal.at(i)->insertPaste(
//			toinsert->getSignal(sourcechan++)
//		    );
//		    sourcechan %= clipchan;
//		}
//	    }
//	    emit signalChanged(getLMarker(), -1);
//	}
//    CASE_COMMAND("mixpaste")
//	if (globals.clipboard) {
//	    SignalManager *toinsert = globals.clipboard->getSignal();
//	    if (toinsert) {
//		unsigned int clipchan = toinsert->getChannelCount();
//		unsigned int sourcechan = 0;
//
//		for (unsigned int i = 0; i < channels; i++) {
//		    signal.at(i)->mixPaste(
//			toinsert->getSignal(sourcechan++)
//		    );
//		    sourcechan %= clipchan;
//		}
//	    }
//	    emit signalChanged(getLMarker(), -1);
//	}
//    CASE_COMMAND("addchannel")
//	addChannel();
//    CASE_COMMAND("deletechannel")
//	Parser parser(command);
//	unsigned int i = atoi(parser.getFirstParam());
//	deleteChannel(i);
//    CASE_COMMAND("selectchannels")
//	for (unsigned int i = 0; i < channels; i++)
//	    *selected.at(i) = true;
//    CASE_COMMAND("invertchannels")
//	for (unsigned int i = 0; i < channels; i++)
//	    *selected.at(i) = !(*selected.at(i));
    } else {
	bool result = promoteCommand(command);
	emit signalChanged( -1, -1);
	return result;
    }

    return true;
}

//**********************************************************
bool SignalManager::promoteCommand (const char *command)
{
    debug("SignalManager::promoteCommand(%s)", command);    // ###

    // loop over all channels
    unsigned int i;
    for (i = 0; i < channels; i++) {
        ASSERT(signal.at(i));
        if (!signal.at(i)) continue;

	// skip channel if not selected
	if (!(*selected.at(i))) continue;

	int begin, len;
	if (lmarker != rmarker) {
	    begin = lmarker;
	    len = rmarker - lmarker + 1;
	} else {
	    begin = 0;
	    len = signal.at(i)->getLength();
	}

	char buf[64];
	snprintf(buf, sizeof(buf), "%d", i + 1);
	char *caption = catString(i18n(command), i18n(" on channel "), buf);

// =====================================================
// === under construction ==============================
	
	// create a nice little Object that should contain everything important
	TimeOperation *operation =
	    new TimeOperation(signal.at(i), command, begin, len);
	ASSERT(operation);
	if (!operation) {
	    warning("out of memory: could not allocate TimeOperation");
	    continue;
	}
	
	debug("lmarker=%d, rmarker=%d, begin=%d, len=%d",
	    lmarker,rmarker,begin,len);
	
	//create a new progress dialog, that watches an memory address
	//that is updated by the modules
	ProgressDialog *dialog = createProgressDialog(operation, caption);
	ASSERT(dialog);
//	if (dialog) {
//	    // connect the signal for "command done"
//	    connect(dialog, SIGNAL(commandDone()),
//		    this, SLOT(commandDone()));
//		
#ifdef USE_THREADS
	    pthread_t thread;
	    // emit modifyingSignal(signal.at(i), begin, length);

//	extern int pthread_create __P ((pthread_t *__thread,
//				__const pthread_attr_t *__attr,
//				void *(*__start_routine) (void *),
//				void *__arg));

	    //create the new thread
	    if (pthread_create (&thread,
				0,
				(void *(*)(void *))(threadStub),
				(void *)operation) != 0) {
		warning("thread creation failed");
		if (dialog) delete dialog;
		return false;
	    }
#else /* USE_THREADS */
	    threadStub(operation);
#endif /* USE_THREADS */

	    if (dialog) delete dialog;

// === under construction ==============================
// =====================================================

//	}
//	else warning("out of memory: could not allocate ProgressDialog");
    }



    // could not promote command to modules or an error occured
    if (i < channels) return false;

    return true;
}

//**********************************************************
void SignalManager::commandDone()
{
//    debug("SignalManager::commandDone()");    // ###
//    emit signalChanged( -1, -1);
}

//**********************************************************
SignalManager::~SignalManager()
{
    while (channels) deleteChannel(channels-1);
    if (name) delete name;
}

//**********************************************************
void SignalManager::setRange(unsigned int l, unsigned int r)
{
    for (unsigned int i = 0; i < channels; i++) {
	ASSERT(signal.at(i));
	if (signal.at(i)) signal.at(i)->setMarkers(l, r);
    }
    ASSERT(signal.at(0));
    if (signal.at(0)) {
	lmarker = signal.at(0)->getLMarker();
	rmarker = signal.at(0)->getRMarker();
    }
}

//**********************************************************
ProgressDialog *SignalManager::createProgressDialog(TimeOperation *operation,
                                                    const char *caption)
{
    ProgressDialog *dialog = new ProgressDialog (operation, caption);
    ASSERT(dialog);
    if (dialog) {
	dialog->show();
	return dialog;
    }
    return 0;
}

//**********************************************************
//below are all methods of Class SignalManager that deal with I/O
//such as loading and saving.

//**********************************************************
int SignalManager::loadAscii()
{
    float value;
    int cnt = 0;
    float max = 0;
    float amp;
    int *sample;

    FILE *sigin = fopen(name, "r");
    if (!sigin) {
	KMsgBox::message(0, i18n("Info"), i18n("File does not exist !"), 2);
	return -ENOENT;
    }

    //loop over all samples in file to get maximum and minimum
    while (!feof(sigin)) {
	if (fscanf (sigin, "%e\n", &value) == 1) {
	    if ( value > max) max = value;
	    if ( -value > max) max = -value;
	    cnt++;
	}
    }
    debug("SignalManager::loadAscii(): reading ascii file with %d samples",
	  cnt);    // ###

    // get the maximum and the scale
    amp = (float)((1 << 23)-1) / max;
    rate = 10000;        //will be asked in requester
    channels = 1;

    signal.append(new Signal(cnt, rate));
    signal.last()->setBits(24);
    sample = signal.last()->getSample();
    if (sample) {
	fseek (sigin, 0, SEEK_SET);    //seek to beginning
	cnt = 0;
	while (!feof(sigin)) {
	    if (fscanf(sigin, "%e\n", &value) == 1) {
		sample[cnt++] = (int)(value * amp);
	    }
	}
    }

    fclose (sigin);
    return 0;
}

//**********************************************************
int SignalManager::loadWav()
{
    int result = 0;
    __uint32_t length = 0;
    wavheader header;

    FILE *sigfile = fopen(name, "r");
    if (!sigfile) {
	KMsgBox::message(0, i18n("Info"), i18n("File does not exist !"), 2);
	return -ENOENT;
    }

    int num = fread(&header, 1, sizeof(wavheader), sigfile);
    if (num == sizeof(struct wavheader))
    {
	if ( (strncmp("RIFF", (char*)&(header.riffid), 4) == 0) &&
	     (strncmp("WAVE", (char*)&(header.wavid), 4) == 0) &&
	     (strncmp("fmt ", (char*)&(header.fmtid), 4) == 0) )
	{
#if defined(IS_BIG_ENDIAN)
	    header.mode = bswap_16(header.mode);
	    header.rate = bswap_32(header.rate);
	    header.channels = bswap_16(header.channels);
	    header.bitspersample = bswap_16(header.bitspersample);
	    length = bswap_32(length);
#endif
	    debug("filelength     = 0x%08X", header.filelength);
	    debug("fmtlength      = %d", header.fmtlength);
	    debug("mode           = %d", header.mode);
	    debug("channels       = %d", header.channels);
	    debug("rate           = %d", header.rate);
	    debug("AvgBytesPerSec = %d", header.AvgBytesPerSec);
	    debug("BlockAlign     = %d", header.BlockAlign);
	    debug("bitspersample  = %d", header.bitspersample);

	    if (header.mode == 1) {
		rate = header.rate;
		int res = findWavChunk(sigfile);
		if (res == 0) {
		    KMsgBox::message(0, i18n("Info"),
		                    i18n("File has no data chunk!"),
		                    2);
		    result = -ENODATA;
		} else {
		    //seek after DATA
		    fseek(sigfile, res, SEEK_SET);
		    //load length of data chunk
		    fread(&length, 1, sizeof(__uint32_t), sigfile);
#if defined(IS_BIG_ENDIAN)
		    length = bswap_32(length);
#endif
		    debug("SignalManager::loadWav():length is %d,res is %d",
			  length, res);

		    length = (length/(header.bitspersample/8))/header.channels;
		    switch (header.bitspersample) {
			    case 8:
			    case 16:
			    case 24:
			    result = loadWavChunk(sigfile, length,
						  header.channels,
						  header.bitspersample);
			    break;
			    default:
				KMsgBox::message(0, i18n("Info"),
				    i18n("Sorry only 8/16/24 Bits per Sample"\
				    " are supported !"), 2);
			    result = -EMEDIUMTYPE;
			    break;
		    }
		    channels = header.channels;
		}
	    } else {
		KMsgBox::message(0, i18n("Info"),
		    i18n("File must be uncompressed (Mode 1) !"), 2);
		result = -EMEDIUMTYPE;
	    }
	} else {
	    KMsgBox::message(0, i18n("Info"),
		i18n("File is no RIFF Wave File !"), 2);
	    result = -EMEDIUMTYPE;
	}
    } else {
	KMsgBox::message(0, i18n("Info"),
	    i18n("File does not contain enough data !"), 2);
	result = -ENODATA;
    }

    fclose(sigfile);

    return result;
}

//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
//**********************************************************
void SignalManager::exportAscii(const char *name)
{
//    if (!name) return ;
//
//    int length = getLength();
//    FILE *sigout = fopen (name, "w");
//
//    int *sample = signal.at(0)->getSample();
//    if (sample) {
//	for (int i = 0; i < length ; i++) {
//	    fprintf(sigout, "%0.8e\n", (float)sample[i] / (float)((1 << 23)-1));
//	}
//    }
//
//    fclose(sigout);
}

//**********************************************************
int SignalManager::writeWavChunk(FILE *sigout, unsigned int begin,
                                 unsigned int length, int bits)
{
//    unsigned int bufsize = 16 * 1024 * sizeof(int);
//    unsigned char *savebuffer = 0;
//    sample_t **sample = 0;    // array of pointers to samples
//    int bytes = bits / 8;
//    int bytes_per_sample = bytes * channels;
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
//    if (channels == 1)
//	strncpy(str_channels, i18n("Mono"), sizeof(str_channels));
//    else if (channels == 2)
//	strncpy(str_channels, i18n("Stereo"), sizeof(str_channels));
//    else
//	snprintf(str_channels, sizeof(str_channels), "%d-channel", channels);
//
//    snprintf(progress_title, sizeof(progress_title), "Saving %d-Bit-%s File :",
//	    bits, str_channels);
//
//    char *title = duplicateString(i18n(progress_title));
//    ProgressDialog *dialog = new ProgressDialog (100, title);
//    delete title;
//    if (dialog) dialog->show();
//
//    //prepare the store loop
//    int percent_count = length / 200;
//    unsigned int shift = 24-bits;
//
//    sample = new (int*)[channels];
//    for (unsigned int channel = 0; channel < channels; channel++)
//	sample[channel] = signal.at(channel)->getSample();
//
//    //loop for writing data
//    for (unsigned int pos = begin; pos < length; ) {
//	unsigned char *buf = savebuffer;
//	unsigned int nsamples = 0;
//
//	while (pos < length && (nsamples < bufsize / bytes_per_sample)) {
//	    for (unsigned int channel = 0; channel < channels; channel++) {
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
//    if (sample) delete sample;
//    if (dialog) delete dialog;
//    if (savebuffer) delete savebuffer;
    return 0;
}

//**********************************************************
void SignalManager::save(const char *filename, int bits, bool selection)
{
//    debug("SignalManager::save(): %d Bit to %s ,%d", bits, filename, selection);
//    int begin = 0;
//    int length = this->getLength();
//    struct wavheader header;
//
//    if (selection && (lmarker != rmarker)) {
//	begin = lmarker;
//	length = rmarker - lmarker + 1;
//    }
//
//    FILE *sigout = fopen (filename, "w");
//    if (name) deleteString (name);
//    name = duplicateString (filename);
//
//    if (sigout) {
//	fseek (sigout, 0, SEEK_SET);
//
//	strncpy ((char*)&(header.riffid), "RIFF", 4);
//	strncpy ((char*)&(header.wavid), "WAVE", 4);
//	strncpy ((char*)&(header.fmtid), "fmt ", 4);
//	header.fmtlength = 16;
//	header.filelength = (length * bits / 8 * channels + sizeof(struct wavheader));
//	header.mode = 1;
//	header.channels = channels;
//	header.rate = rate;
//	header.AvgBytesPerSec = rate * bits / 8 * channels;
//	header.BlockAlign = bits * channels / 8;
//	header.bitspersample = bits;
//
//	int datalen = length * header.channels * header.bitspersample / 8;
//
//#if defined(IS_BIG_ENDIAN)
//	header.mode = bswap_16(header.mode);
//	header.rate = bswap_32(header.rate);
//	header.channels = bswap_16(header.channels);
//	header.bitspersample = bswap_16(header.bitspersample);
//	header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
//	header.fmtlength = bswap_32(header.fmtlength);
//	header.filelength = bswap_32(header.filelength);
//	datalen = bswap_32(datalen);
//#endif
//
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
//		KMessageBox::message(0, i18n("Info"),
//		    i18n("Sorry only 8/16/24 Bits per Sample are supported !"),
//		    2);
//	    break;
//	}
//
//	fclose(sigout);
//    }
}

//**********************************************************
int SignalManager::loadWavChunk(FILE *sigfile, unsigned int length,
                                unsigned int channels, int bits)
{
    unsigned int bufsize = 16 * 1024 * sizeof(int);
    unsigned char *loadbuffer = 0;
    sample_t **sample = 0;    // array of pointers to samples

    // try to allocate memory for the load buffer
    // if failed, try again with the half buffer size as long
    // as <1kB is not reached (then we are really out of memory)
    while (loadbuffer == 0) {
	if (bufsize < 1024) {
	    debug("SignalManager::loadWavChunk:not enough memory for buffer");
	    return -ENOMEM;
	}
	loadbuffer = new unsigned char[bufsize];
	if (!loadbuffer) bufsize >>= 1;
    }

    sample = new (sample_t*)[channels];
    for (unsigned int channel = 0; channel < channels; channel++) {
	Signal *new_signal = new Signal(length, rate);
	ASSERT(new_signal);
	if (!new_signal) {
	    KMsgBox::message(0, i18n("Info"), i18n("Out of Memory!"), 2);
	    return -ENOMEM;
	}
	signal.append(new_signal);
	signal.at(channel)->setBits(bits);
	selected.append(new bool(true));
	sample[channel] = signal.at(channel)->getSample();
    }

    //prepare and show the progress dialog
    char progress_title[256];
    char str_channels[32];
    if (channels == 1)
	strncpy(str_channels, i18n("Mono"), sizeof(str_channels));
    else if (channels == 2)
	strncpy(str_channels, i18n("Stereo"), sizeof(str_channels));
    else
	snprintf(str_channels, sizeof(str_channels),
	    i18n("%d voices"), channels);

    snprintf(progress_title, sizeof(progress_title),
	i18n("Loading %d-Bit (%s) File :"), bits, str_channels);
    char *title = duplicateString(progress_title);
    ProgressDialog *dialog = new ProgressDialog (100, title);
    ASSERT(dialog);
    delete title;

    if (dialog) dialog->show();

    //prepare the load loop
    int percent_count = length / 1000;
    int bytes = bits / 8;
    unsigned int sign = 1 << (24-1);
    unsigned int negative = ~(sign - 1);
    unsigned int shift = 24-bits;
    unsigned int bytes_per_sample = bytes * channels;
    unsigned int max_samples = bufsize / bytes_per_sample;
    long int start_offset = ftell(sigfile);

    ASSERT(bytes);
    ASSERT(channels);

    // debug("sign=%08X, negative=%08X, shift=%d",sign,negative,shift);

    for (unsigned int pos = 0; pos < length; ) {
	int read_samples = fread((char *)loadbuffer, bytes_per_sample,
				 max_samples, sigfile);
	percent_count -= read_samples;
	// debug("read %d samples", read_samples);
	if (read_samples <= 0) {
	    warning("SignalManager::loadWavChunk:EOF reached?"\
		    " (at sample %ld, expected length=%d",
		    ftell(sigfile) / bytes_per_sample - start_offset, length);
	    break;
	}

	unsigned char *buffer = loadbuffer;
	while (read_samples--) {
	    for (register unsigned int channel = 0;
	         channel < channels;
	         channel++)
	    {
		register int *smp = sample[channel] + pos;
		if (bytes == 1) {
		    // 8-bit files are always unsigned !
		    *smp = (*(buffer++) - 128) << shift;
		} else {
		    // >= 16 bits is signed
		    for (register int byte = 0; byte < bytes; byte++)
			*smp |= *(buffer++) << ((byte << 3) + shift);
		    // sign correcture for negative values
		    if ((unsigned int)*smp & sign)
			*smp |= negative;
		}
	    }
	    pos++;
	}

	if (dialog && (percent_count <= 0)) {
	    percent_count = length / 1000;
	    float percent = (float)pos;
	    percent /= (float)length;
	    percent *= 100.0;
	    dialog->setProgress (percent);
	}
    }

    if (dialog) delete dialog;
    if (loadbuffer) delete loadbuffer;
    if (sample) delete sample;
    return 0;
}

//**********************************************************
int SignalManager::findWavChunk(FILE *sigin)
{
    ASSERT(sigin);
    if (!sigin) return 0;

    char buffer[4096];
    int point = 0, max = 0;
    int filecount = ftell(sigin);
    int res = (filecount >= 0) ? 0 : -1;

    while ((point == max) && (res == 0)) {
	max = fread (buffer, 1, 4096, sigin);
	//      max=sigin->readBlock (buffer,4096);
	point = 0;
	while (point < max)
	    if (strncmp (&buffer[point++], "data", 4) == 0) break;

	if (point == max) {
	    filecount += point - 4;       //rewind 4 Bytes;
	    res = fseek (sigin, filecount, SEEK_SET);
	} else return filecount + point + 3;
    }

    return 0;
}

/***************************************************************************/
//below  all methods of Class SignalManager that deal with sound playback

#define processid       0
#define stopprocess     1
#define samplepointer   2

int play16bit = false;
int bufbase = 5;
const char *sounddevice = {"/dev/dsp" };

//**********************************************************
//following are the playback routines
struct Play {
    SignalManager *manage;
    bool loop;
};

//**********************************************************
void playThread(struct Play *par)
{
//    if (play16bit) par->manage->play16 (par->loop);
//    else par->manage->play8 (par->loop);
//
//    delete par;
}

//**********************************************************
void SignalManager::play(bool loop)
{
//    Play *par = new Play;
//    pthread_t thread;
//
//    par->manage = this;
//    par->loop = loop;
//
//    pthread_create (&thread, 0, (void * (*) (void *))playThread, par);
}

//**********************************************************
void SignalManager::stopplay()
{
//    if (msg[processid] >= 0)    //if there is a process running
//	msg[stopprocess] = true;          //set flag for stopping
}

//**********************************************************
int SignalManager::setSoundParams(int audio, int bitspersample,
                                  unsigned int channels, int rate,
                                  int bufbase)
{
//    if ( ioctl(audio, SNDCTL_DSP_SAMPLESIZE, &bitspersample) != -1) {
//	if ( ioctl(audio, SNDCTL_DSP_STEREO, &channels) != -1) {
//	    if (ioctl(audio, SNDCTL_DSP_SPEED, &rate) != -1) {
//		if (ioctl(audio, SNDCTL_DSP_SETFRAGMENT, &bufbase) != -1) {
//		    int size;
//		    ioctl(audio, SNDCTL_DSP_GETBLKSIZE, &size);
//		    return size;
//		} else warning("unusable buffer size");
//	    } else warning("unusable rate");
//	} else warning("wrong number of channels");
//    } else warning("number of bits per samples not supported");
    return 0;
}

//**********************************************************
void SignalManager::play8(bool loop)
{
//    debug("SignalManager::play8 (%d)", loop);    // ###
//
//    int audio;                    //file handle for /dev/dsp
//    int act;
//    char *buffer = 0;
//    unsigned int bufsize;
//
//    msg[stopprocess] = false;
//    if ((audio = open(sounddevice, O_WRONLY)) != -1 ) {
//	bufsize = setSoundParams(audio, 8, 0, rate, bufbase);
//
//	if (bufsize) {
//	    buffer = new char[bufsize];
//	    memset(buffer, 0, bufsize);
//
//	    if (buffer) {
//		unsigned int &pointer = msg[samplepointer];
//		unsigned int last = rmarker;
//		unsigned int cnt = 0;
//		pointer = lmarker;
//
//		if (loop) {
//		    if (lmarker == rmarker) last = getLength();
//
//		    while (msg[stopprocess] == false) {
//			for (cnt = 0; cnt < bufsize; cnt++) {
//			    if (pointer >= last) pointer = lmarker;
//			    act = 0;
//			    for (unsigned int i = 0; i < channels; i++)
//				act += signal.at(i)->getSingleSample(pointer);
//			    act /= channels;
//			    act += 1 << 23;
//			    buffer[cnt] = act >> 16;
//			    pointer++;
//			}
//			write (audio, buffer, bufsize);
//		    }
//		} else {
//		    if (last == pointer) last = getLength();
//
//		    while ((last - pointer > bufsize) && (msg[stopprocess] == false)) {
//			for (cnt = 0; cnt < bufsize; cnt++) {
//
//			    act = 0;
//			    for (unsigned int i = 0; i < channels; i++)
//				act += signal.at(i)->getSingleSample(pointer);
//
//			    act /= channels;
//			    act += 1 << 23;
//			    buffer[cnt] = act >> 16;
//			    pointer++;
//			}
//
//			write (audio, buffer, bufsize);
//		    }
//		    if (msg[stopprocess] == false) {
//			for (cnt = 0; cnt < last - pointer; cnt++) {
//			    act = 0;
//			    for (unsigned int i = 0; i < channels; i++)
//				act += signal.at(i)->getSingleSample(pointer);
//
//			    act /= channels;
//			    act += 1 << 23;
//			    buffer[cnt] = act >> 16;
//			    pointer++;
//			}
//			for (; cnt < bufsize; cnt++)
//			    buffer[cnt] = 128;    // fill up last buffer
//			write (audio, buffer, bufsize);
//		    }
//		}
//	    }
//	}
//
//	close (audio);
//	if (buffer) delete [] buffer;
//	msg[stopprocess] = false;
//	msg[samplepointer] = 0;
//    }
}

//**********************************************************
void SignalManager::play16(bool loop)
{
//    debug("SignalManager::play16 (%d)", loop);    // ###
//    int audio;                    //file handle for /dev/dsp
//    unsigned char *buffer = 0;
//    unsigned int bufsize;
//
//    msg[stopprocess] = false;
//    if ( (audio = open(sounddevice, O_WRONLY)) != -1 ) {
//	bufsize = setSoundParams(audio, 16, 0, rate, bufbase);
//
//	buffer = new unsigned char[bufsize];
//	memset(buffer, 0, bufsize);
//
//	if (buffer) {
//	    unsigned int &pointer = msg[samplepointer];
//	    unsigned int last = rmarker;
//	    unsigned int act, cnt = 0;
//	    pointer = lmarker;
//	    if (last == pointer) last = getLength();
//
//	    if (loop) {
//		while (msg[stopprocess] == false) {
//		    for (cnt = 0; cnt < bufsize; ) {
//			if (pointer >= last) pointer = lmarker;
//
//			act = 0;
//			for (unsigned int i = 0; i < channels; i++)
//			    act += signal.at(i)->getSingleSample(pointer);
//
//			act /= channels;
//			act += 1 << 23;
//			buffer[cnt++] = act >> 8;
//			buffer[cnt++] = (act >> 16) + 128;
//			pointer++;
//		    }
//		    write (audio, buffer, bufsize);
//		}
//	    } else {
//		while ((last - pointer > bufsize) && (msg[stopprocess] == false)) {
//		    for (cnt = 0; cnt < bufsize; ) {
//			act = 0;
//			for (unsigned int i = 0; i < channels; i++)
//			    act += signal.at(i)->getSingleSample(pointer);
//
//			act /= channels;
//			act += 1 << 23;
//			buffer[cnt++] = act >> 8;
//			buffer[cnt++] = (act >> 16) + 128;
//			pointer++;
//		    }
//		    write (audio, buffer, bufsize);
//		}
//		// playing not aborted and still something left, so play the rest...
//		if (msg[stopprocess] == false) {
//		    for (cnt = 0; cnt < last - pointer; cnt++) {
//			act = 0;
//			for (unsigned int i = 0; i < channels; i++)
//			    act += signal.at(i)->getSingleSample(pointer);
//
//			act /= channels;
//			act += 1 << 23;
//			buffer[cnt++] = act >> 8;
//			buffer[cnt++] = (act >> 16) + 128;
//			pointer++;
//		    }
//
//		    while (cnt < bufsize) {
//			buffer[cnt++] = 0x00;
//			buffer[cnt++] = 0x80;
//		    }
//		    write (audio, buffer, bufsize);
//		}
//	    }
//	}
//	close (audio);
//	if (buffer) delete [] buffer;
//	msg[stopprocess] = false;
//	msg[samplepointer] = 0;
//    }
}

//**********************************************************
