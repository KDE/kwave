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
#include <endian.h>
#include <byteswap.h>
#include <stdlib.h>

extern "C" {
#include "libaudiofile/audiofile.h" // from libaudiofile
#include "libaudiofile/af_vfs.h"    // from libaudiofile
}

#include <qlist.h>
#include <qprogressdialog.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libgui/ConfirmCancelProxy.h"

#include "RecoveryBuffer.h"
#include "RecoveryMapping.h"
#include "RecoverySource.h"
#include "RepairVirtualAudioFile.h"
#include "RIFFChunk.h"
#include "RIFFParser.h"
#include "VirtualAudioFile.h"
#include "WavDecoder.h"
#include "WavFileFormat.h"
#include "WavFormatMap.h"

#define CHECK(cond) ASSERT(cond); if (!(cond)) { src.close(); return false; }

//***************************************************************************
WavDecoder::WavDecoder()
    :Decoder(), m_source(0), m_src_adapter(0)
{
    LOAD_MIME_TYPES;
}

//***************************************************************************
WavDecoder::~WavDecoder()
{
    if (m_source) close();
    if (m_src_adapter) delete m_src_adapter;
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
    bool need_repair = false;
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

    if (!riff_chunk || !fmt_chunk || !data_chunk || !parser.isSane()) {
	if (KMessageBox::warningContinueCancel(widget,
	    i18n("The file has been structurally damaged or "
	         "is no .wav file.\n"
	         "Should Kwave try to repair it?"),
	    i18n("Kwave auto repair"),
	    i18n("&Repair")) != KMessageBox::Continue)
	{
	    // user didn't let us try :-(
	    return false;
	}

	need_repair = true;
    }

    // collect all missing chunks
    if (!riff_chunk) riff_chunk = parser.findMissingChunk("RIFF");
    if (progress.wasCancelled()) return false;

    if (!fmt_chunk) {
	parser.findMissingChunk("fmt ");
	fmt_chunk = parser.findChunk("/RIFF/fmt ");
	if (progress.wasCancelled()) return false;
	if (!fmt_chunk)  fmt_chunk  = parser.findChunk("fmt ");
	need_repair = true;
    }

    if (!data_chunk) {
	parser.findMissingChunk("data");
	data_chunk = parser.findChunk("/RIFF/data");
	if (progress.wasCancelled()) return false;
	if (!data_chunk) data_chunk = parser.findChunk("data");
	need_repair = true;
    }

    // not everything found -> need heavy repair actions !
    if (!fmt_chunk || !data_chunk) {
	parser.dumpStructure();
	parser.repair();
	parser.dumpStructure();
        if (progress.wasCancelled()) return false;

        if (!fmt_chunk)  fmt_chunk  = parser.findChunk("/RIFF/fmt ");
        if (!fmt_chunk)  fmt_chunk  = parser.findChunk("fmt ");
        if (!data_chunk) data_chunk = parser.findChunk("/RIFF/data");
        if (!data_chunk) data_chunk = parser.findChunk("data");
	need_repair = true;
    }

    u_int32_t fmt_offset = 0;
    if (fmt_chunk) fmt_offset = fmt_chunk->dataStart();
    debug("fmt chunk starts at 0x%08X", fmt_offset);

    u_int32_t data_offset = 0;
    u_int32_t data_size = 0;
    if (data_chunk) {
	data_offset = data_chunk->dataStart();
	data_size   = data_chunk->physLength();
	debug("data chunk at 0x%08X (%u byte)", data_offset, data_size);
    }

    if (data_size <= 4) {
	KMessageBox::sorry(widget,
	    i18n("The opened file is no .WAV file or damaged:\n"
	    "There is not enough valid sound data.\n\n"
	    "It makes no sense to continue now..."));
	return false;
    }

    // final check for structural integrity:
    // we should have exactly one RIFF, fmt and data chunk !
    if ((parser.chunkCount("fmt ") != 1) ||
        (parser.chunkCount("data") != 1))
    {
	if (KMessageBox::warningContinueCancel(widget,
	    i18n("The WAV file seems to be damaged: \n"
	         "some chunks are duplicate or missing! \n\n"
	         "Kwave will only use the first ones and ignores\n"
	         "the rest. This might lead to a loss of data. If\n"
	         "If you want to get your file repaired completely,\n"
	         "please write an e-mail to the Kwave mailing list\n"
	         "and we will help you..."),
	    i18n("Kwave auto-repair"),
	    i18n("&Continue")
	    ) != KMessageBox::Continue)
	{
	    // user decided to abort and repair on his own
	    // good luck!
	    return false;
	}
	need_repair = true;
    }

    // source successfully opened
    m_source = &src;

    // read the wave header
    wav_fmt_header_t header;
    // readWavHeader(src, fmt_offset, fmt_size, &header);

    unsigned int rate = 0;
    unsigned int bits = 0;

    src.at(fmt_offset);

    // get the encoded block of data from the mime source
    CHECK(src.size() > sizeof(wav_header_t)+8);

    // get the header
    src.readBlock((char *)&header, sizeof(wav_fmt_header_t));
#if defined(IS_BIG_ENDIAN)
    header.min.format      = bswap_16(header.min.format);
    header.min.channels    = bswap_16(header.min.channels);
    header.min.samplerate  = bswap_32(header.min.samplerate);
    header.min.bytespersec = bswap_32(header.min.bytespersec);
    header.min.blockalign  = bswap_16(header.min.blockalign);
    header.min.bitwidth    = bswap_16(header.min.bitwidth);
#endif
    unsigned int tracks = header.min.channels;
    rate = header.min.samplerate;
    bits = header.min.bitwidth;

    WavFormatMap known_formats;
    QString format_name = known_formats.findName(header.min.format);

//    debug("-------------------------");
//    debug("wav header:");
//    debug("mode        = 0x%04X, (%s)", header.min.format, format_name.data());
//    debug("channels    = %d", header.min.channels);
//    debug("rate        = %u", header.min.samplerate);
//    debug("bytes/s     = %u", header.min.bytespersec);
//    debug("block align = %d", header.min.blockalign);
//    debug("bits/sample = %d", header.min.bitwidth);
//    debug("-------------------------");

    // open the file through libaudiofile :)
    if (need_repair) {
	QList<RecoverySource> *repair_list = new QList<RecoverySource>();
	ASSERT(repair_list);
	if (!repair_list) return false;
	
	RIFFChunk *root = (riff_chunk) ? riff_chunk : parser.findChunk("");
	parser.dumpStructure();
	debug("riff chunk = %p, parser.findChunk('')=%p", riff_chunk,
	    parser.findChunk(""));
	repair(repair_list, root, fmt_chunk, data_chunk);
	m_src_adapter = new RepairVirtualAudioFile(*m_source, repair_list);
    } else {
	m_src_adapter = new VirtualAudioFile(*m_source);
    }

    ASSERT(m_src_adapter);
    if (!m_src_adapter) return false;

    m_src_adapter->open(m_src_adapter);

    AFfilehandle fh = m_src_adapter->handle();
    ASSERT(fh);
    if (!fh || (m_src_adapter->lastError() >= 0)) {
	QString reason;
	
	switch (m_src_adapter->lastError()) {
	    case AF_BAD_NOT_IMPLEMENTED:
	        reason = i18n("format or function is not implemented");
	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("file header is damaged");
	        break;
	    case AF_BAD_CODEC_TYPE:
	        reason = i18n("invalid codec type");
	        break;
	    case AF_BAD_OPEN:
	        reason = i18n("opening the file failed");
	        break;
	    case AF_BAD_READ:
	        reason = i18n("read access failed");
	        break;
	    case AF_BAD_SAMPFMT:
	        reason = i18n("invalid sample format");
	        break;
	    default:
		reason = reason.number(m_src_adapter->lastError());
	}
	
	QString text= i18n("An error occurred while opening the "\
	    "file:\n'%1'").arg(reason);
	KMessageBox::error(widget, text);
	
	return false;
    }

    unsigned int length = afGetFrameCount(fh, AF_DEFAULT_TRACK);
    tracks = afGetVirtualChannels(fh, AF_DEFAULT_TRACK);

    int sample_format;
    afGetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, &sample_format,
	(int *)(&bits));

    QString sample_format_name;
    switch (sample_format) {
	case AF_SAMPFMT_TWOSCOMP:
	    sample_format_name = "linear two's complement";
	    break;
	case AF_SAMPFMT_UNSIGNED:
	    sample_format_name = "unsigned integer";
	    break;
	case AF_SAMPFMT_FLOAT:
	    sample_format_name = "32-bit IEEE floating-point";
	    break;
	case AF_SAMPFMT_DOUBLE:
	    sample_format_name = "64-bit IEEE double-precision floating-point";
	    break;
	default:
	    format_name = "(unknown)";
    }
    if (static_cast<signed int>(bits) < 0) bits = 0;

    info().setRate(rate);
    info().setBits(bits);
    info().setTracks(tracks);
    info().setLength(length);
    debug("-------------------------");
    debug("info:");
    debug("channels    = %d", info().tracks());
    debug("rate        = %0.0f", info().rate());
    debug("bits/sample = %d", info().bits());
    debug("length      = %d samples", info().length());
    debug("format      = %d (%s)", sample_format, sample_format_name.data());
    debug("-------------------------");

    // set up libaudiofile to produce Kwave's internal sample format
#if defined(IS_BIG_ENDIAN)
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
	AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    return true;
}

//***************************************************************************
bool WavDecoder::decode(QWidget */*widget*/, MultiTrackWriter &dst)
{
    ASSERT(m_src_adapter);
    ASSERT(m_source);
    if (!m_source) return false;
    if (!m_src_adapter) return false;

    AFfilehandle fh = m_src_adapter->handle();
    ASSERT(fh);
    if (!fh) return false;

    unsigned int frame_size = (unsigned int)afGetVirtualFrameSize(fh,
	AF_DEFAULT_TRACK, 1);

    // allocate a buffer for input data
    const unsigned int buffer_frames = (8*1024);
    int32_t *buffer = (int32_t *)malloc(buffer_frames * frame_size);
    ASSERT(buffer);
    if (!buffer) return false;

    // read in from the audiofile source
    const unsigned int tracks = info().tracks();
    unsigned int rest = info().length();
    while (rest) {
	unsigned int frames = buffer_frames;
	if (frames > rest) frames = rest;
	unsigned int buffer_used = afReadFrames(fh,
	    AF_DEFAULT_TRACK, (char *)buffer, frames);
	
	// break if eof reached	
	ASSERT(buffer_used);
	if (!buffer_used) break;
	rest -= buffer_used;
	
	// split into the tracks
	int32_t *p = buffer;
	unsigned int count = buffer_used;
	while (count--) {
	    for (unsigned int track = 0; track < tracks; track++) {
		register int32_t s = *p++;
		
		// adjust precision
		if (SAMPLE_STORAGE_BITS != SAMPLE_BITS) {
		    s /= (1 << (SAMPLE_STORAGE_BITS-SAMPLE_BITS));
		}
		
		// the following cast is only necessary if
		// sample_t is not equal to a u_int32_t
		*(dst[track]) << static_cast<sample_t>(s);
	    }
	}
	
	// abort if the user pressed cancel
	if (dst.isCancelled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    if (buffer) free(buffer);
    return true;
}

//***************************************************************************
bool WavDecoder::repairChunk(QList<RecoverySource> *repair_list,
    RIFFChunk *chunk, u_int32_t &offset)
{
    ASSERT(chunk);
    ASSERT(m_source);
    ASSERT(repair_list);
    if (!chunk) return false;
    if (!m_source) return false;
    if (!repair_list) return false;

    char buffer[16];
    u_int32_t length;
    RecoverySource *repair = 0;

    // create buffer with header
    strncpy(buffer, chunk->name().data(), 4);
    length = (chunk->type() == RIFFChunk::Main) ? chunk->physLength() :
                                                  chunk->dataLength();
    buffer[4] = (length      ) & 0xFF;
    buffer[5] = (length >>  8) & 0xFF;
    buffer[6] = (length >> 16) & 0xFF;
    buffer[7] = (length >> 24) & 0xFF;
    if (chunk->type() == RIFFChunk::Main) {
	strncpy(&(buffer[8]), chunk->format().data(), 4);
	repair = new RecoveryBuffer(offset, 12, buffer);
	debug("[0x%08X-0x%08X] - main header '%s' (%s), len=%u",
	      offset, offset+11, chunk->name().data(),
	      chunk->format().data(), length);
	offset += 12;
    } else {
	repair = new RecoveryBuffer(offset, 8, buffer);
	debug("[0x%08X-0x%08X] - sub header '%s', len=%u",
	      offset, offset+7, chunk->name().data(), length);
	offset += 8;
    }
    ASSERT(repair);
    if (!repair) return false;
    repair_list->append(repair);

    // map the chunk's data if not main or root
    if ((chunk->type() != RIFFChunk::Root) &&
        (chunk->type() != RIFFChunk::Main))
    {
	repair = new RecoveryMapping(offset, chunk->physLength(),
	                             *m_source, chunk->dataStart());
	debug("[0x%08X-0x%08X] - restoring from offset 0x%08X (%u)",
	      offset, offset+chunk->physLength()-1, chunk->dataStart(),
	      chunk->physLength());
	ASSERT(repair);
	if (!repair) return false;
	repair_list->append(repair);
	
	offset += chunk->physLength();
    }

    // recursively go over all sub-chunks
    RIFFChunkList &list = chunk->subChunks();
    QListIterator<RIFFChunk> it(list);
    bool ok = true;
    for (; ok && it.current(); ++it) {
	RIFFChunk *chunk = it.current();
	ok = repairChunk(repair_list, chunk, offset);
    }

    return ok;
}

//***************************************************************************
bool WavDecoder::repair(QList<RecoverySource> *repair_list,
    RIFFChunk *riff_chunk, RIFFChunk *fmt_chunk, RIFFChunk *data_chunk)
{
    ASSERT(fmt_chunk);
    ASSERT(data_chunk);
    if (!fmt_chunk || !data_chunk) return false;

    // --- first create a chunk tree for the structure ---

    // make a new "RIFF" chunk as root
    RIFFChunk new_root(0, "RIFF", "WAVE", 0,0,0);
    new_root.setType(RIFFChunk::Main);

    // create a new "fmt " chunk
    RIFFChunk *new_fmt = new RIFFChunk(&new_root, "fmt ",0,0,
	fmt_chunk->physStart(), fmt_chunk->physLength());
    ASSERT(new_fmt);
    if (!new_fmt) return false;
    new_root.subChunks().append(new_fmt);

    // create a new "data" chunk
    RIFFChunk *new_data = new RIFFChunk(&new_root, "data",0,0,
	data_chunk->physStart(), data_chunk->physLength());
    ASSERT(new_data);
    if (!new_data) return false;
    new_root.subChunks().append(new_data);

    // if we have a RIFF chunk, add it's subchunks, except "fmt " and "data"
    // we keep only pointers here, just for getting the right structure,
    // sizes and start positions.
    // NOTE: The sizes might be re-assigned and get invalid afterwards!!!
    if (riff_chunk) {
	RIFFChunkList &list = riff_chunk->subChunks();
	QListIterator<RIFFChunk> it(list);
	for (; it.current(); ++it) {
	    RIFFChunk *chunk = it.current();
	    if (chunk->name() == "fmt ") continue;
	    if (chunk->name() == "data") continue;
	    if (chunk->name() == "RIFF") continue;
	    if (chunk->type() == RIFFChunk::Empty) continue;
	    if (chunk->type() == RIFFChunk::Garbage) continue;
	
	    new_root.subChunks().append(chunk);
	}
    }

    // fix all node sizes (compress)
    new_root.fixSize();

    // attention: some of the offsets belong to source file, some belong
    // to reconstructed buffers! only the sizes are correct.
//    new_root.dumpStructure();

    // --- set up the repair list ---

    // RIFF chunk length
    u_int32_t offset = 0;
    bool repaired = repairChunk(repair_list, &new_root, offset);

    // clean up...
    new_root.subChunks().setAutoDelete(false);
    new_root.subChunks().clear();
    delete new_fmt;
    delete new_data;

//    QListIterator<RecoverySource> it(repair_list);
//    for (; it.current(); ++it) {
//	RecoverySource *src = it.current();
//	QString name = "";
//	debug("[0x%08X-0x%08X] - %s", src->offset(), src->end(), name.data());
//    }

    return (repaired);
}

//***************************************************************************
void WavDecoder::close()
{
    if (m_src_adapter) delete m_src_adapter;
    m_src_adapter = 0;
    m_source = 0;
}

//***************************************************************************
//***************************************************************************
