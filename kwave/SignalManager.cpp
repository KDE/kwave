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

#include <errno.h>
#include <endian.h>
#include <byteswap.h>
#include <limits.h>
#include <math.h>

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
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"

#include "libgui/FileProgress.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "SignalManager.h"
#include "SignalWidget.h"
#include "UndoAction.h"
#include "UndoModifyAction.h"
#include "UndoTransaction.h"
#include "UndoTransactionGuard.h"

#if __BYTE_ORDER==__BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif

#define min(x,y) (((x)<(y)) ? (x) : (y))
#define max(x,y) (((x)>(y)) ? (x) : (y))

#define CASE_COMMAND(x) } else if (parser.command() == x) {


//***************************************************************************
SignalManager::SignalManager(QWidget *parent)
    :QObject(),
    m_parent_widget(parent),
    m_name(""),
    m_closed(true),
    m_empty(true),
    m_signal(),
    m_selection(0,0),
    m_rate(0),
    m_playback_controller(),
    m_undo_enabled(false),
    m_undo_buffer(),
    m_redo_buffer(),
    m_undo_transaction(0),
    m_undo_transaction_level(0),
    m_undo_transaction_lock(),
    m_spx_undo_redo(this, SLOT(emitUndoRedoInfo())),
    m_undo_limit(5*1024*1024) // 5 MB (for testing) ###
{
    // connect to the track's signals
    Signal *sig = &m_signal;
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
}

//***************************************************************************
void SignalManager::loadFile(const QString &filename, int type)
{
    ASSERT(filename.length());
    m_name = filename;

    // disable undo (discards all undo/redo data)
    disableUndo();

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

    // from now on, undo is enabled
    enableUndo();
}

//***************************************************************************
void SignalManager::close()
{
    // disable undo and discard all undo buffers
    // undo will be re-enabled when a signal is loaded or created
    disableUndo();

    m_empty = true;
    m_name = "";
    m_signal.close();

    m_closed = true;
    m_rate = 0;
    m_selection.select(0,0);
    emitStatusInfo();
}

//***************************************************************************
const QArray<unsigned int> SignalManager::selectedTracks()
{
    unsigned int track;
    unsigned int count = 0;
    QArray<unsigned int> list(tracks());

    for (track=0; track < list.count(); track++) {
//	### TODO ### selection of tracks
//	if (!m_track_selected(track)) continue;
	list[count++] = track;
    }

    list.resize(count);
    return list;
}

//***************************************************************************
int SignalManager::singleSample(unsigned int /*channel*/, unsigned int /*offset*/)
{
//    ASSERT(channel < signal.count());
//    if (channel >= signal.count()) return 0;
//
//    return signal.at(channel) ?
//	   signal.at(channel)->getSingleSample(offset) : 0;
    return 0;
}

//***************************************************************************
int SignalManager::averageSample(unsigned int /*offset*/,
                                 const QArray<unsigned int> */*channels*/)
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
QBitmap *SignalManager::overview(unsigned int /*width*/, unsigned int /*height*/,
                                 unsigned int /*offset*/, unsigned int /*length*/)
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
void SignalManager::toggleChannel(const unsigned int /*channel*/)
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
SampleWriter *SignalManager::openSampleWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right)
{
    return m_signal.openSampleWriter(track, mode, left, right);
}

//***************************************************************************
bool SignalManager::executeCommand(const QString &command)
{
    debug("SignalManager::executeCommand(%s)", command.data());    // ###

    unsigned int offset = m_selection.offset();
    unsigned int length = m_selection.length();

    if (!command.length()) return true;
    Parser parser(command);

    if (false) {
    CASE_COMMAND("undo")
	undo();
    CASE_COMMAND("redo")
	redo();
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
    CASE_COMMAND("delete")
//	deleteRange(offset, length);

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
    } else {
	return false;
    }

    return true;
}

//***************************************************************************
void SignalManager::slotTrackInserted(unsigned int index,
	Track &track)
{
    emit sigTrackInserted(index, track);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesInserted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    emit sigSamplesInserted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesDeleted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    emit sigSamplesDeleted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesModified(unsigned int track,
	unsigned int offset, unsigned int length)
{
    emit sigSamplesModified(track, offset, length);
}

//***************************************************************************
SignalManager::~SignalManager()
{
    close();
}

//***************************************************************************
void SignalManager::selectRange(unsigned int offset, unsigned int length)
{
    // first do some range checking
    unsigned int len = this->length();

    if (offset >= len) offset = len ? (len-1) : 0;
    if ((offset+length) > len) length = len - offset;

    m_selection.select(offset, length);
}

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
void SignalManager::emitStatusInfo()
{
    emit sigStatusInfo(length(), tracks(), rate(), bits());
}

//***************************************************************************
int SignalManager::loadWav()
{
    wav_fmt_header_t fmt_header;
    int result = 0;
    __uint32_t num;
    __uint32_t length;

    ASSERT(m_closed);
    ASSERT(m_empty);

    QFile sigfile(m_name);
    if (!sigfile.open(IO_ReadOnly)) {
	KMessageBox::error(m_parent_widget,
		i18n("File does not exist !"));
	return -ENOENT;
    }

    // --- check if the file starts with "RIFF" ---
    num = sigfile.size();
    length = findChunk(sigfile, "RIFF", 0);
    if ((length == 0) || (sigfile.at() != 8)) {
	KMessageBox::error(m_parent_widget,
	    i18n("File is no RIFF File !"));
	// maybe recoverable...
    } else if (length+8 != num) {
	KMessageBox::error(m_parent_widget,
	    i18n("File has incorrect length! (maybe truncated?)"));
	// maybe recoverable...
    } else {
	// check if the chunk data contains "WAVE"
	char file_type[16];
	num = sigfile.readBlock((char*)(&file_type), 4);
	if ((num != 4) || strncmp("WAVE", file_type, 4)) {
	    KMessageBox::error(m_parent_widget,
		i18n("File is no WAVE File !"));
	    // maybe recoverable...
	}
    }

    // ------- read the "fmt " chunk -------
    ASSERT(sizeof(fmt_header) == 16);
    num = findChunk(sigfile, "fmt ");
    if (num != sizeof(fmt_header)) {
	debug("SignalManager::loadWav(): length of fmt chunk = %d", num);
	KMessageBox::error(m_parent_widget,
	    i18n("File does not contain format information!"));
	return -EMEDIUMTYPE;
    }
    num = sigfile.readBlock((char*)(&fmt_header), sizeof(fmt_header));
#ifdef IS_BIG_ENDIAN
    fmt_header.length = bswap_32(fmt_header.length);
    fmt_header.mode = bswap_16(fmt_header.mode);
    fmt_header.channels = bswap_16(fmt_header.channels);
    fmt_header.rate = bswap_32(fmt_header.rate);
    fmt_header.AvgBytesPerSec = bswap_32(fmt_header.AvgBytesPerSec);
    fmt_header.BlockAlign = bswap_32(fmt_header.BlockAlign);
    fmt_header.bitspersample = bswap_16(fmt_header.bitspersample);
#endif
    if (fmt_header.mode != 1) {
	KMessageBox::error(m_parent_widget,
	    i18n("File must be uncompressed (Mode 1) !"),
	    i18n("Sorry"), 2);
	return -EMEDIUMTYPE;
    }

    m_rate = fmt_header.rate;
    m_signal.setBits(fmt_header.bitspersample);

    // ------- search for the data chunk -------
    length = findChunk(sigfile, "data");
    if (!length) {
	KMessageBox::error(m_parent_widget,
	    i18n("File does not contain data!"));
	return -EMEDIUMTYPE;
    }

    length = (length/(fmt_header.bitspersample/8))/fmt_header.channels;
    switch (fmt_header.bitspersample) {
	case 8:
	case 16:
	case 24:
	    ASSERT(m_empty);
	    // currently the signal should be closed and empty
	    // now make it opened but empty
	    m_closed = false;
	    emitStatusInfo();
	    result = loadWavChunk(sigfile, length,
				  fmt_header.channels,
				  fmt_header.bitspersample);
	    break;
	default:
	    KMessageBox::error(m_parent_widget,
		i18n("Sorry only 8/16/24 Bits per Sample"\
		" are supported !"), i18n("Sorry"), 2);
	    result = -EMEDIUMTYPE;
    }

    if (result == 0) {
	debug("SignalManager::loadWav(): successfully opened");
    }

    return result;
}

//**********************************************************
// the following routines are for loading and saving in dataformats
// specified by names little/big endian problems are dealt with at compile time
// The corresponding header should have already been written to the file before
// invocation of this methods
//***************************************************************************
void SignalManager::exportAscii(const char */*name*/)
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
int SignalManager::writeWavChunk(QFile &sigout, unsigned int offset,
                                 unsigned int length, unsigned int bits)
{
    unsigned int bufsize = 16 * 1024 * sizeof(int);
    unsigned char *savebuffer = 0;
    int bytes = bits / 8;
    unsigned int tracks = this->tracks();
    int bytes_per_sample = bytes * tracks;
    bufsize -= bufsize % bytes_per_sample;

    // try to allocate memory for the save buffer
    // if failed, try again with the half buffer size as long
    // as <1kB is not reached (then we are really out of memory)
    while (savebuffer == 0) {
	if (bufsize < 1024) {
	    warning("SignalManager::writeWavChunk:no memory for buffer");
	    return -ENOMEM;
	}
	savebuffer = new unsigned char[bufsize];
	if (!savebuffer) {
	    bufsize >>= 1;
	    bufsize -= bufsize % bytes_per_sample;
	}
    }

    // prepare and show the progress dialog
    unsigned int file_rest = tracks * length * bytes_per_sample;
    FileProgress *dialog = new FileProgress(m_parent_widget,
	m_name, file_rest, length, m_rate, bits, tracks);
    ASSERT(dialog);

    // prepare the store loop
    int percent_count = length / 200;
    unsigned int shift = 24-bits;

    QList<SampleReader> samples;
    samples.setAutoDelete(true);
    for (unsigned int track = 0; track < tracks; track++) {
	SampleReader *s = m_signal.openSampleReader(track,
	    offset, offset+length-1);
	ASSERT(s);
	if (!s) {
	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"));
	    return -ENOMEM;
	}
	samples.append(s);
    }

    // loop for writing data
    for (unsigned int pos = offset; pos < offset+length; ) {
	unsigned char *buf = savebuffer;
	unsigned int nsamples = 0;
	
	while (pos < offset+length &&
	      (nsamples < (bufsize/bytes_per_sample)))
	{
	    for (unsigned int track = 0; track < tracks; track++) {
		SampleReader *stream = samples.at(track);
		sample_t sample;
		(*stream) >> sample;
		
		// the following cast is only necessary if
		// sample_t is not equal to a 32bit int
		__uint32_t act = static_cast<__uint32_t>(sample);
		
		act >>= shift;
		if (bytes == 1) {
		    // 8 bits -> unsigned
		    *(buf++) = (char)((act - 128) & 0xFF);
		} else {
		    // >= 16 bits -> signed
		    for (register int byte = bytes; byte; byte--) {
			*(buf++) = (char)(act & 0xFF);
			act >>= 8;
		    }
		}
	    }
	    nsamples++;
	    pos++;
	}
	
	int written_bytes = sigout.writeBlock(
	    reinterpret_cast<char *>(savebuffer),
	    bytes_per_sample * nsamples);
	
	percent_count -= written_bytes;
	if (dialog && (percent_count <= 0)) {
	    percent_count = length / 200;
	    dialog->setValue(pos*tracks*bytes_per_sample);
	}
    }

    if (dialog) delete dialog;
    if (savebuffer) delete[] savebuffer;

    // close all SampleReaders
    samples.clear();

    return 0;
}

//***************************************************************************
void SignalManager::save(const QString &filename, unsigned int bits,
                         bool selection)
{
    debug("SignalManager::save(): %u Bit to %s ,%d",
    	bits, filename.data(), selection);

    __uint32_t ofs = 0;
    __uint32_t len = length();
    unsigned int tracks = this->tracks();
    wav_header_t header;

    ASSERT(filename);
    if (!filename) return;

    if (selection) {
	if (m_selection.length() == 0) return; // zero-length -> nothing to do
	ofs = m_selection.offset();
	len = m_selection.length();
    }

    if (!tracks || !len) {
	KMessageBox::error(m_parent_widget,
	    i18n("Signal is empty, nothing to save !"));
	return;
    }

    QFile sigout(filename);
    if (!sigout.open(IO_WriteOnly | IO_Truncate)) {
	KMessageBox::error(m_parent_widget,
	    i18n("Opening the file for writing failed!"));
	return;
    };

    __uint32_t datalen = (bits >> 3) * len * tracks;
    strncpy((char*)&(header.riffid), "RIFF", 4);
    strncpy((char*)&(header.wavid), "WAVE", 4);
    strncpy((char*)&(header.fmtid), "fmt ", 4);
    header.filelength = datalen + sizeof(wav_header_t);
    header.fmtlength = 16;
    header.mode = 1;
    header.channels = tracks;
    header.rate = m_rate;
    header.AvgBytesPerSec = m_rate * (bits >> 3) * tracks;
    header.BlockAlign = (bits >> 3) * tracks;
    header.bitspersample = bits;

#if defined(IS_BIG_ENDIAN)
    header.filelength = bswap_32(header.filelength);
    header.fmtlength = bswap_32(header.fmtlength);
    header.mode = bswap_16(header.mode);
    header.channels = bswap_16(header.channels);
    header.rate = bswap_32(header.rate);
    header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
    header.BlockAlign = bswap_16(header.BlockAlign);
    header.bitspersample = bswap_16(header.bitspersample);
    datalen = bswap_32(datalen);
#endif

    sigout.at(0);
    sigout.writeBlock((char *) &header, sizeof(wav_header_t));
    sigout.writeBlock("data", 4);
    sigout.writeBlock((char *)&datalen, 4);

    switch (bits) {
	case 8:
	case 16:
	case 24:
	    writeWavChunk(sigout, ofs, len, bits);
	    break;
	default:
	    KMessageBox::sorry(m_parent_widget,
		i18n("Sorry only 8/16/24 Bits per Sample are supported !"));
	break;
    }

    sigout.close();

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
    unsigned int bufsize = 64 * 1024 * sizeof(sample_t);
    unsigned char *loadbuffer = 0;
    int bytes = bits >> 3;
    unsigned int sign = 1 << (24-1);
    unsigned int negative = ~(sign - 1);
    unsigned int shift = 24-bits;
    unsigned int bytes_per_sample = bytes * channels;
    unsigned int max_samples = bufsize / bytes_per_sample;
    long int start_offset = sigfile.at();

    debug("SignalManager::loadWavChunk(): offset     = %d", sigfile.at());
    debug("SignalManager::loadWavChunk(): length     = %d samples", length);
    debug("SignalManager::loadWavChunk(): tracks     = %d", channels);
    debug("SignalManager::loadWavChunk(): resoultion = %d bits/sample", bits);

    ASSERT(bytes);
    ASSERT(channels);
    ASSERT(length);
    if (!bytes || !channels || !length) return -EINVAL;

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

    // check if the file is large enough for "length" samples
    size_t file_rest = sigfile.size() - sigfile.at();
    if (length > file_rest/bytes_per_sample) {
	debug("SignalManager::loadWavChunk: "\
	      "length=%d, rest of file=%d",length,file_rest);
	KMessageBox::error(m_parent_widget,
	    i18n("Warning"),
	    i18n("Error in input: file is smaller than stated "\
	    "in the header. \n"\
	    "File will be truncated."), 2);
	length = file_rest/bytes_per_sample;
    }

    QList<SampleWriter> samples;
    samples.setAutoDelete(true);

    for (unsigned int track = 0; track < channels; track++) {
	Track *new_track = m_signal.appendTrack(length);
	ASSERT(new_track);
	if (!new_track) {
	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"));
	    return -ENOMEM;
	}
	
	SampleWriter *s = openSampleWriter(track, Overwrite);
	ASSERT(s);
	if (!s) {
	    KMessageBox::sorry(m_parent_widget, i18n("Out of Memory!"));
	    return -ENOMEM;
	}
	samples.append(s);
    }

    // now the signal is considered not to be empty
    m_empty = false;

    //prepare and show the progress dialog
    FileProgress *dialog = new FileProgress(m_parent_widget,
	m_name, file_rest, length, m_rate, bits, channels);
    ASSERT(dialog);

    // prepare the loader loop
    int percent_count = length / 100;

    // debug("sign=%08X, negative=%08X, shift=%d",sign,negative,shift);

    for (unsigned int pos = 0; pos < length; ) {
	// break the loop if the user has pressed "cancel"
	if (dialog && dialog->isCancelled()) break;
	
	// limit reading to end of wav chunk length
	if ((pos + max_samples) > length) max_samples=length-pos;
	
	// read the samples into a temporary buffer
	int read_samples = sigfile.readBlock(
	    reinterpret_cast<char *>(loadbuffer),
	    bytes_per_sample*max_samples
	) / bytes_per_sample;
	percent_count -= read_samples;
	
	// debug("read %d samples", read_samples);
	if (read_samples <= 0) {
	    warning("SignalManager::loadWavChunk:EOF reached?"\
		    " (at sample %ld, expected length=%d",
		    sigfile.at() / bytes_per_sample - start_offset, length);
	    break;
	}
	
	unsigned char *buffer = loadbuffer;
	__uint32_t s = 0; // raw 32bit value
	while (read_samples--) {
	    for (register unsigned int channel = 0;
		 channel < channels;
		channel++)
	    {
		SampleWriter *stream = samples.at(channel);
		
		if (bytes == 1) {
		    // 8-bit files are always unsigned !
		    s = (*(buffer++) - 128) << shift;
		} else {
		    // >= 16 bits is signed
		    s = 0;
		    for (register int byte = 0; byte < bytes; byte++) {
			s |= *(buffer++) << ((byte << 3) + shift);
		    }
		    // sign correcture for negative values
		    if ((unsigned int)s & sign)
			s |= negative;
		}
		
		// the following cast is only necessary if
		// sample_t is not equal to a 32bit int
		sample_t sample = static_cast<sample_t>(s);
		
		*stream << sample;
	    }
	    pos++;
	}
	
	if (dialog && (percent_count <= 0)) {
	    percent_count = length / 100;
	    dialog->setValue(pos * bytes_per_sample);
	}
    }

    // close all sample input streams
    samples.clear();

    if (dialog) delete dialog;
    if (loadbuffer) delete[] loadbuffer;
    return 0;
}

//***************************************************************************
PlaybackController &SignalManager::playbackController()
{
    return m_playback_controller;
}

//***************************************************************************
void SignalManager::startUndoTransaction(const QString &name)
{
    MutexGuard lock(m_undo_transaction_lock);

    // increase recursion level
    m_undo_transaction_level++;

//    debug("SignalManager::startUndoTransaction(): increased to %u",
//          m_undo_transaction_level);

    // start/create a new transaction if none existing
    if (!m_undo_transaction) {
	m_undo_transaction = new UndoTransaction(name);
	ASSERT(m_undo_transaction);
//	debug("SignalManager::startUndoTransaction(): started new");
    }
}

//***************************************************************************
void SignalManager::closeUndoTransaction()
{
    MutexGuard lock(m_undo_transaction_lock);

    // decrease recursion level
    ASSERT(m_undo_transaction_level);
    if (m_undo_transaction_level) m_undo_transaction_level--;

    debug("SignalManager::closeUndoTransaction(): reduced to %u",
          m_undo_transaction_level);

    if (!m_undo_transaction_level) {
	// append the current transaction to the undo buffer if
	// not empty
	if (m_undo_transaction) {
	    if (!m_undo_transaction->isEmpty()) {
		debug("SignalManager::closeUndoTransaction(): not empty");
		m_undo_buffer.append(m_undo_transaction);
	    } else {
		debug("SignalManager::closeUndoTransaction(): empty");
		delete m_undo_transaction;
	    }
	}
	
	// declare the current transaction as "closed"
	m_undo_transaction = 0;
//	debug("SignalManager::closeUndoTransaction(): closed");
	m_spx_undo_redo.AsyncHandler();
    }
}

//***************************************************************************
void SignalManager::enableUndo()
{
    m_undo_enabled = true;
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::disableUndo()
{
    ASSERT(m_undo_transaction_level == 0);

    m_undo_enabled = false;
    flushUndoBuffers();
}

//***************************************************************************
void SignalManager::flushUndoBuffers()
{
    MutexGuard lock(m_undo_transaction_lock);

    ASSERT(m_undo_transaction_level == 0);

    // close the current transaction
    if (m_undo_transaction) delete m_undo_transaction;
    m_undo_transaction = 0;
    m_undo_transaction_level = 0;

    // clear all buffers
    m_undo_buffer.clear();
    m_redo_buffer.clear();

    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::flushRedoBuffer()
{
    m_redo_buffer.clear();
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
bool SignalManager::registerUndoAction(UndoAction *action)
{
    MutexGuard lock(m_undo_transaction_lock);
    ASSERT(action);
    if (!action) return false;

    // if undo is not enabled, return a faked "ok"
    if (!m_undo_enabled) return true;

    ASSERT(m_undo_transaction);
    if (!m_undo_transaction) return false;

    unsigned int needed_size  = action->undoSize();
    unsigned int needed_mb = needed_size  >> 20;
    unsigned int limit_mb  = m_undo_limit >> 20;

    // Print a warning if the needed memory exceeds the limit for undo.
    // The user should have the chance to decide between:
    // a) abort the current operation
    // b) permit the operation, but continue without undo
    // c) ignore the memory limit and store undo data nevertheless
    if (needed_mb > limit_mb) {
	int choice = KMessageBox::warningYesNoCancel(m_parent_widget,
	    /* HTML version */
	    QString("<HTML>"+
	    i18n("The operation '%1' requires %2 MB for storing undo data.\n"\
	    "This would exceed the limit of %3 MB.")+
	    "<BR><BR>"+
	    i18n("You can now either press:")+
	    "<TABLE NOBORDER><TR>"\
	    "<TD><B>"+
	    i18n("Allow")+"</B></TD><TD>"+
	    i18n("to continue without the possibility to undo, or")+
	    "</TD></TR><TR>"\
	    "<TD><B>"+i18n("Ignore")+"</B></TD><TD>"+
	    i18n("to continue, discard all previous undo data "\
	                    "and ignore the memory limit")+
	    "</TD></TR><TR>"\
	    "<td><B>"+
	    i18n("Cancel")+
	    "</B></TD><TD>"+
	    i18n("to abort the current operation.")+
	    "</td></TR></TABLE></HTML>").arg(
	    m_undo_transaction->description()).arg(needed_mb).arg(limit_mb),
	
	    /* plain text version
	    i18n("The command '%1' requires %2 MB for storing undo data.\n"\
	    "This would exceed the limit of %3 MB. \n\n"\
	    "You can now either press:\n"\
	    "* <Allow>  to continue without the possibility to undo, or\n"\
	    "* <Ignore> to continue and ignore the memory limit, or\n"\
	    "* <Cancel> to abort the current operation.").arg(
	    m_undo_transaction->description()).arg(needed_mb).arg(limit_mb),
	    */
	
	    QString::null,
	    i18n("&Allow"),
	    i18n("&Ignore"),
	    true
	);
	
	switch (choice) {
	    case KMessageBox::Yes:
		// Allow: discard buffers and omit undo
		m_undo_buffer.clear();
		m_redo_buffer.clear();
		return true;
	    case KMessageBox::No:
		// Ignore: discard buffers and ignore limit
		m_undo_buffer.clear();
		m_redo_buffer.clear();
		break;
	    default:
		// Cancel: abort the action
		return false;
	}
    }

    // now make room...
    freeUndoMemory(needed_size);

    // now we have enough place to append the undo action
    // and store all undo info
    m_undo_transaction->append(action);
    action->store(*this);

    return true;
}

//***************************************************************************
unsigned int SignalManager::usedUndoRedoMemory()
{
    unsigned int size = 0;

    QListIterator<UndoTransaction> undo_it(m_undo_buffer);
    for ( ; undo_it.current(); ++undo_it ) {
	size += undo_it.current()->undoSize();
    }
    QListIterator<UndoTransaction> redo_it(m_redo_buffer);
    for ( ; redo_it.current(); ++redo_it ) {
	size += redo_it.current()->undoSize();
    }

    return size;
}

//***************************************************************************
void SignalManager::freeUndoMemory(unsigned int needed)
{
    unsigned int size = usedUndoRedoMemory() + needed;

    // remove old undo actions if not enough free memory
    m_undo_buffer.setAutoDelete(true);
    while (!m_undo_buffer.isEmpty() && (size > m_undo_limit)) {
	size -= m_undo_buffer.first()->undoSize();
	m_undo_buffer.removeFirst();
    }

    // remove old redo actions if still not enough memory
    m_redo_buffer.setAutoDelete(true);
    while (!m_redo_buffer.isEmpty() && (size > m_undo_limit)) {
	size -= m_redo_buffer.last()->undoSize();
	m_redo_buffer.removeLast();
    }

}

//***************************************************************************
void SignalManager::emitUndoRedoInfo()
{
    QString undo_name = 0;
    QString redo_name = 0;

    if (m_undo_enabled) {
	UndoTransaction *transaction;
	
	// get the description of the last undo action
	if (!m_undo_buffer.isEmpty()) {
	    transaction = m_undo_buffer.last();
	    if (transaction) undo_name = transaction->description();
	    if (!undo_name.length()) undo_name = i18n("last action");
	}
	
	// get the description of the last redo action
	if (!m_redo_buffer.isEmpty()) {
	    transaction = m_redo_buffer.first();
	    if (transaction) redo_name = transaction->description();
	    if (!redo_name.length()) redo_name = i18n("last action");
	}
    }

    // now emit the undo/redo transaction names
    emit sigUndoRedoInfo(undo_name, redo_name);
}

//***************************************************************************
void SignalManager::undo()
{
    MutexGuard lock(m_undo_transaction_lock);

    // get the last undo transaction and abort if none present
    UndoTransaction *undo_transaction = m_undo_buffer.last();
    ASSERT(undo_transaction);
    if (!undo_transaction) return;

    // remove the undo transaction from the list without deleting it
    m_undo_buffer.setAutoDelete(false);
    m_undo_buffer.removeLast();
    m_undo_buffer.setAutoDelete(true);

    // get free memory for redo
    // also bear in mid that the undo transaction we removed is still
    // allocated and has to be subtracted from the undo limit. As we
    // don't want to modify the limit, we increase the needed size.
    unsigned int redo_size = undo_transaction->redoSize();
    unsigned int undo_size = undo_transaction->undoSize();

    UndoTransaction *redo_transaction = 0;
    if (undo_size + redo_size > m_undo_limit) {
	// not enough memory for redo
	warning("SignalManager::undo(): not enough memory for redo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(undo_size + redo_size);
	
	// create a new redo transaction
	QString name = undo_transaction->description();
	redo_transaction = new UndoTransaction(name);
	ASSERT(redo_transaction);
    }

    // if *one* redo fails, all following redoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!redo_transaction) {
	flushRedoBuffer();
	debug("SignalManager::undo(): redo buffer flushed!"); // ###
    } else {
	m_redo_buffer.prepend(redo_transaction);
    }

    // execute all undo actions and store the resulting redo
    // actions into the redo transaction
    while (!undo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;
	
	// unqueue the undo action
	undo_transaction->setAutoDelete(false);
	undo_action = undo_transaction->last();
	undo_transaction->removeLast();
	ASSERT(undo_action);
	if (!undo_action) continue;
	
	// execute the undo operation
	redo_action = undo_action->undo(*this);
	
	// remove the old undo action if no longer used
	if (redo_action != undo_action) {
	    delete undo_action;
	}
	
	// queue the action into the redo transaction
	if (redo_transaction) {
	    redo_transaction->prepend(redo_action);
	} else {
	    // redo is not usable :-(
	    if (redo_action) delete redo_action;
	}
    }

    // finished / buffers have changed, emit new undo/redo info
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::redo()
{
    MutexGuard lock(m_undo_transaction_lock);

    // get the last redo transaction and abort if none present
    UndoTransaction *redo_transaction = m_redo_buffer.first();
    ASSERT(redo_transaction);
    if (!redo_transaction) return;

    // remove the redo transaction from the list without deleting it
    m_redo_buffer.setAutoDelete(false);
    m_redo_buffer.removeFirst();
    m_redo_buffer.setAutoDelete(true);

    // get free memory for undo
    // also bear in mid that the redo transaction we removed is still
    // allocated and has to be subtracted from the undo limit. As we
    // don't want to modify the limit, we increase the needed size.
    unsigned int undo_size = redo_transaction->undoSize();
    unsigned int redo_size = redo_transaction->redoSize();

    UndoTransaction *undo_transaction = 0;
    if (undo_size + redo_size > m_undo_limit) {
	// not enough memory for undo
	warning("SignalManager::redo(): not enough memory for undo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(undo_size + redo_size);
	
	// create a new undo transaction
	QString name = redo_transaction->description();
	undo_transaction = new UndoTransaction(name);
	ASSERT(undo_transaction);
    }

    // if *one* undo fails, all following undoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!undo_transaction) {
	m_undo_buffer.setAutoDelete(true);
	m_undo_buffer.clear();
	debug("SignalManager::redo(): undo buffer flushed!");
    } else {
	m_undo_buffer.append(undo_transaction);
    }

    // execute all redo actions and store the resulting undo
    // actions into the undo transaction
    while (!redo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;
	
	// unqueue the undo action
	redo_transaction->setAutoDelete(false);
	redo_action = redo_transaction->first();
	redo_transaction->removeFirst();
	ASSERT(redo_action);
	if (!redo_action) continue;
	
	// execute the redo operation
	undo_action = redo_action->undo(*this);
	
	// remove the old redo action if no longer used
	if (redo_action != undo_action) {
	    delete redo_action;
	}
	
	// queue the action into the undo transaction
	if (undo_transaction) {
	    undo_transaction->append(undo_action);
	} else {
	    // undo is not usable :-(
	    if (undo_action) delete undo_action;
	}
    }

    // finished / buffers have changed, emit new undo/redo info
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
//***************************************************************************
