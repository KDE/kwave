/*************************************************************************
        WavDecoder.cpp  -  decoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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
#include <qprogressdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libgui/ConfirmCancelProxy.h"

#include "RIFFChunk.h"
#include "RIFFParser.h"
#include "WavDecoder.h"
#include "WavFileFormat.h"

#include <qapplication.h> // ###

#define CHECK(cond) ASSERT(cond); if (!(cond)) { src.close(); return false; }

//***************************************************************************
WavDecoder::WavDecoder()
    :Decoder(), m_source(0)
{
    LOAD_MIME_TYPES;
}

//***************************************************************************
WavDecoder::~WavDecoder()
{
    if (m_source) close();
}

//***************************************************************************
Decoder *WavDecoder::instance()
{
    return new WavDecoder();
}

//***************************************************************************
bool WavDecoder::open(QWidget *widget, QIODevice &src)
{
    info().clear();
    ASSERT(!m_source);
    if (m_source) warning("WavDecoder::open(), already open !");

    // try to open the source
    if (!src.open(IO_ReadOnly)) {
	warning("failed to open source !");
	return false;
    }

    QStringList main_chunks;
    main_chunks.append("RIFF"); /* RIFF, little-endian */
    main_chunks.append("RIFX"); /* RIFF, big-endian */
    main_chunks.append("FORM"); /* used in AIFF, big-endian or IFF/.lbm */
    main_chunks.append("LIST"); /* additional information */
    main_chunks.append("adtl"); /* Associated Data */

    QStringList known_chunks;

    // native WAVE chunk names
    known_chunks.append("cue "); /* Markers */
    known_chunks.append("data"); /* Sound Data */
    known_chunks.append("fmt "); /* Format */
    known_chunks.append("inst"); /* Instrument */
    known_chunks.append("labl"); /* label */
    known_chunks.append("ltxt"); /* labeled text */
    known_chunks.append("note"); /* note chunk */
    known_chunks.append("plst"); /* Play List */
    known_chunks.append("smpl"); /* Sampler */

    // some sub-chunks from the LIST chunk
    known_chunks.append("IART");
    known_chunks.append("ICMT");
    known_chunks.append("ICOP");
    known_chunks.append("IENG");
    known_chunks.append("IGNR");
    known_chunks.append("IKEY");
    known_chunks.append("IMED");
    known_chunks.append("INAM");
    known_chunks.append("ISRC");
    known_chunks.append("ITCH");
    known_chunks.append("ISBJ");
    known_chunks.append("ISRF");

    // some chunks known from AIFF format
    known_chunks.append("FVER");
    known_chunks.append("COMM");
    known_chunks.append("wave");
    known_chunks.append("SSND");

    // chunks of .lbm image files, IFF format
    known_chunks.append("BMHD");
    known_chunks.append("CMAP");
    known_chunks.append("BODY");

    RIFFParser parser(src, main_chunks, known_chunks);

    // prepare a progress dialog
    QProgressDialog progress(widget, "Auto-Repair", true);
    progress.setMinimumDuration(0);
    progress.setTotalSteps(100);
    progress.setAutoClose(true);
    progress.setProgress(0);
    progress.setLabelText("reading...");
    connect(&parser,   SIGNAL(progress(int)),
            &progress, SLOT(setProgress(int)));
    connect(&parser,   SIGNAL(action(const QString &)),
            &progress, SLOT(setLabelText(const QString &)));
    ConfirmCancelProxy confirm_cancel(widget,
                       &progress, SIGNAL(cancelled()),
                       &parser,   SLOT(cancel()));

    // parse, including endianness detection
    parser.parse();
    progress.reset();
    if (progress.wasCancelled()) return false;

    debug("--- RIFF file structure after first pass ---");
    parser.dumpStructure();

    // check if there is a RIFF chunk at all...
    RIFFChunk *riff_chunk = parser.findChunk("/RIFF");
    RIFFChunk *fmt_chunk  = parser.findChunk("/RIFF/fmt ");
    RIFFChunk *data_chunk = parser.findChunk("/RIFF/data");

    if (!riff_chunk || !fmt_chunk || !data_chunk) {
	warning("structural damage detected!");
	if (KMessageBox::warningContinueCancel(widget,
	    i18n("The file has been structurally damaged or "
	         "is no .wav file.\n"
	         "Kwave will try to recover it!"),
	    i18n("Kwave auto repair"),
	    i18n("&Repair")) != KMessageBox::Continue)
	{
	    // user didn't let us try :-(
	    return false;
	}
    }

    // collect all missing chunks
    if (!riff_chunk) riff_chunk = parser.findMissingChunk("RIFF");
    if (progress.wasCancelled()) return false;

    if (!fmt_chunk) {
        parser.findMissingChunk("fmt ");
        fmt_chunk = parser.findChunk("/RIFF/fmt ");
        if (progress.wasCancelled()) return false;
    }

    if (!data_chunk) {
        parser.findMissingChunk("data");
        data_chunk = parser.findChunk("/RIFF/data");
        if (progress.wasCancelled()) return false;
    }

    // not everything found -> need heavy repair actions !
    if (!fmt_chunk || !data_chunk) {
	parser.dumpStructure();
	parser.repair();
	parser.dumpStructure();
        if (progress.wasCancelled()) return false;
    }

    u_int32_t fmt_offset = 0;
    if (fmt_chunk) fmt_offset = fmt_chunk->dataStart();
    debug("fmt chunk starts at 0x%08X", fmt_offset);

    u_int32_t data_offset = 0;
    if (data_chunk) data_offset = data_chunk->dataStart();
    debug("data chunk starts at 0x%08X", data_offset);

    // source successfully opened
    m_source = &src;

    unsigned int rate = 0;
    unsigned int bits = 0;

    src.at(fmt_offset);

    // get the encoded block of data from the mime source
    CHECK(src.size() > sizeof(wav_header_t)+8);

    wav_fmt_header_t header;
    unsigned int datalen = src.size() - (sizeof(wav_header_t) + 8);

    // get the header
    src.readBlock((char *)&header, sizeof(wav_fmt_header_t));
#if defined(IS_BIG_ENDIAN)
    header.mode = bswap_16(header.mode);
    header.channels = bswap_16(header.channels);
    header.rate = bswap_32(header.rate);
    header.AvgBytesPerSec = bswap_32(header.AvgBytesPerSec);
    header.BlockAlign = bswap_16(header.BlockAlign);
    header.bitspersample = bswap_16(header.bitspersample);
#endif
    const unsigned int tracks = header.channels;
    rate = header.rate;
    bits = header.bitspersample;
    const unsigned int bytes = (bits >> 3);

    debug("-------------------------");
    debug("wav header:");
    debug("mode        = %d", header.mode);
    debug("channels    = %d", header.channels);
    debug("rate        = %u", header.rate);
    debug("bytes/s     = %u", header.AvgBytesPerSec);
    debug("block align = %d", header.BlockAlign);
    debug("bits/sample = %d", header.bitspersample);
    debug("-------------------------");

//    if (src.size() != header.filelength+8) {
//	debug("WavDecoder::open(), header=%d, rest of file=%d",
//	      header.filelength, src.size());
//	KMessageBox::error(widget,
//	    i18n("Error in input: file is smaller than stated "
//	         "in the header. \nFile will be truncated."));
//	
//	datalen = src.size();
//	header.filelength = src.size()-8;
//	datalen = header.filelength - sizeof(wav_header_t);
//    }

    // some sanity checks
    CHECK(header.AvgBytesPerSec == rate * bytes * tracks);
    CHECK(static_cast<unsigned int>(header.BlockAlign) == bytes*tracks);
//    CHECK(header.filelength == (datalen + sizeof(wav_header_t)));
//    CHECK(header.fmtlength == 16);
    CHECK(header.mode == 1); /* currently only mode 1 is supported :-( */

    info().setRate(rate);
    info().setBits(bits);
    info().setTracks(tracks);
    info().setLength(datalen / bytes / tracks);

    src.at(data_offset);
    return true;
}

//***************************************************************************
bool WavDecoder::decode(QWidget */*widget*/, MultiTrackWriter &dst)
{
    ASSERT(m_source);
    if (!m_source) return false;

    const unsigned int bits   = info().bits();
    const unsigned int tracks = info().tracks();
    const unsigned int bytes = (bits >> 3);

    // fill the signal with data
    const __uint32_t sign = 1 << (24-1);
    const unsigned int negative = ~(sign - 1);
    const unsigned int shift = 24-bits;

    unsigned int rest = info().length();
    while (rest--) {
	__uint32_t s = 0; // raw 32bit value
	for (unsigned int track = 0; track < tracks; track++) {
	    SampleWriter *stream = dst[track];
	
	    if (bytes == 1) {
		// 8-bit files are always unsigned !
		s = (static_cast<__uint8_t>(m_source->getch() - 128))<<shift;
	    } else {
		// >= 16 bits is signed
		s = 0;
		for (register unsigned int byte = 0; byte < bytes; byte++) {
		    s |= (static_cast<__uint8_t>(m_source->getch())
		        << ((byte << 3) + shift));
		}
		// sign correcture for negative values
		if (s & sign) s |= negative;
	    }
	
	    // the following cast is only necessary if
	    // sample_t is not equal to a 32bit int
	    *stream << static_cast<sample_t>(s);
	}
	
	// abort if the user pressed cancel
	if (dst.isCancelled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void WavDecoder::close()
{
    m_source = 0;
}

/* ###

below comes some old code from the SignalManager. It's features should
get merged into the new code again...

int SignalManager::loadWavChunk(QFile &sigfile, unsigned int length,
                                unsigned int channels, int bits)
{
    debug("SignalManager::loadWavChunk(): offset     = %d", sigfile.at());
    debug("SignalManager::loadWavChunk(): length     = %d samples", length);
    debug("SignalManager::loadWavChunk(): tracks     = %d", channels);
    debug("SignalManager::loadWavChunk(): resoultion = %d bits/sample", bits);

    // check if the file is large enough for "length" samples
    size_t file_rest = sigfile.size() - sigfile.at();
    if (length > file_rest/bytes_per_sample) {
	debug("SignalManager::loadWavChunk: "\
	      "length=%d, rest of file=%d",length,file_rest);
	KMessageBox::error(m_parent_widget,
	    i18n("Error in input: file is smaller than stated "\
	    "in the header. \n"\
	    "File will be truncated."));
	length = file_rest/bytes_per_sample;
    }

	    warning("SignalManager::loadWavChunk:EOF reached?"\
		    " (at sample %ld, expected length=%d",
		    sigfile.at() / bytes_per_sample - start_offset, length);
	    break;
	}
}
### */

//    /**
//     * Try to find a chunk within a RIFF file. If the chunk
//     * was found, the current position will be at the start
//     * of the chunk's data.
//     * @param sigfile file to read from
//     * @param chunk name of the chunk
//     * @param offset the file offset for start of the search
//     * @return the size of the chunk, 0 if not found
//     */
//    __uint32_t findChunk(QFile &sigfile, const char *chunk,
//	__uint32_t offset = 12);
//
//    /**
//     * Imports ascii file with one sample per line and only one
//     * track. Everything that cannot be parsed by strod will be ignored.
//     * @return 0 if succeeded or error number if failed
//     */
//    int loadAscii();
//
//    /**
//     * Loads a .wav-File.
//     * @return 0 if succeeded or a negative error number if failed:
//     *           -ENOENT if file does not exist,
//     *           -ENODATA if the file has no data chunk or is zero-length,
//     *           -EMEDIUMTYPE if the file has an invalid/unsupported format
//     */
//    int loadWav();
//
//    /**
//     * Reads in the wav data chunk from a .wav-file. It creates
//     * a new empty Signal for each track and fills it with
//     * data read from an opened file. The file's actual position
//     * must already be set to the correct position.
//     * @param sigin reference to the already opened file
//     * @param length number of samples to be read
//     * @param tracks number of tracks [1..n]
//     * @param number of bits per sample [8,16,24,...]
//     * @return 0 if succeeded or error number if failed
//     */
//    int loadWavChunk(QFile &sigin, unsigned int length,
//                     unsigned int tracks, int bits);
//
//    /**
//     * Writes the chunk with the signal to a .wav file (not including
//     * the header!).
//     * @param sigout reference to the already opened file
//     * @param offset start position from where to save
//     * @param length number of samples to be written
//     * @param bits number of bits per sample [8,16,24,...]
//     * @return 0 if succeeded or error number if failed
//     */
//    int writeWavChunk(QFile &sigout, unsigned int offset, unsigned int length,
//                      unsigned int bits);

/* ###
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
//	KMessageBox::error(m_parent_widget,
//	    i18n("File has incorrect length! (maybe truncated?)"));
	// will be warned anyway later...
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
	warning("length = 0, but file size = %u, current pos = %u",
	    sigfile.size(), sigfile.at());
	
	if (sigfile.size() - sigfile.at() > 0) {
	    if (KMessageBox::warningContinueCancel(m_parent_widget,
		i18n("File is damaged: the file header reports zero length\n"\
		     "but the file seems to contain some data. \n\n"\
		     "Kwave can try to recover the file, but the\n"\
		     "result might contain trash at it's start and/or\n"\
		     "it's end. It is strongly advisable to edit the\n"\
		     "damaged parts manually and save the file again\n"\
		     "to fix the problem.\n\n"\
		     "Try to recover?"),
		     i18n("Damaged File"),
		     i18n("&Recover")) == KMessageBox::Continue)
	    {
		length = sigfile.size() - sigfile.at();
	    } else return -EMEDIUMTYPE;
	} else {
	    KMessageBox::error(m_parent_widget,
	        i18n("File does not contain data!"));
	    return -EMEDIUMTYPE;
	}
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
### */

//***************************************************************************
//***************************************************************************
