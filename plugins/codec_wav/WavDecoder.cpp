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
#include <stdlib.h>
#include <new>

#include <audiofile.h>

#include <QList>
#include <QVector>
#include <QtEndian>
#include <QtGlobal>

#include <QApplication>
#include <QProgressDialog>

#include <KLocalizedString>

#include "libkwave/Compression.h"
#include "libkwave/ConfirmCancelProxy.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/VirtualAudioFile.h"
#include "libkwave/Writer.h"

#include "RIFFChunk.h"
#include "RIFFParser.h"
#include "RecoveryBuffer.h"
#include "RecoveryMapping.h"
#include "RecoverySource.h"
#include "RepairVirtualAudioFile.h"
#include "WavDecoder.h"
#include "WavFileFormat.h"
#include "WavFormatMap.h"

#define CHECK(cond) Q_ASSERT(cond); if (!(cond)) { src.close(); return false; }

//***************************************************************************
Kwave::WavDecoder::WavDecoder()
    :Kwave::Decoder(),
     m_source(Q_NULLPTR),
     m_src_adapter(Q_NULLPTR),
     m_known_chunks(),
     m_property_map()
{
    REGISTER_MIME_TYPES
    REGISTER_COMPRESSION_TYPES

    // native WAVE chunk names
    m_known_chunks.append(_("cue ")); /* Markers */
    m_known_chunks.append(_("data")); /* Sound Data */
    m_known_chunks.append(_("fact")); /* Fact (length in samples) */
    m_known_chunks.append(_("fmt ")); /* Format */
    m_known_chunks.append(_("inst")); /* Instrument */
    m_known_chunks.append(_("labl")); /* label */
    m_known_chunks.append(_("ltxt")); /* labeled text */
    m_known_chunks.append(_("note")); /* note chunk */
    m_known_chunks.append(_("plst")); /* Play List */
    m_known_chunks.append(_("smpl")); /* Sampler */

    // add all sub-chunks of the LIST chunk (properties)
    foreach (const QByteArray &name, m_property_map.chunks())
	m_known_chunks.append(QLatin1String(name));

    // some chunks known from AIFF format
    m_known_chunks.append(_("FVER"));
    m_known_chunks.append(_("COMM"));
    m_known_chunks.append(_("wave"));
    m_known_chunks.append(_("SSND"));

    // chunks of .lbm image files, IFF format
    m_known_chunks.append(_("BMHD"));
    m_known_chunks.append(_("CMAP"));
    m_known_chunks.append(_("BODY"));
}

//***************************************************************************
Kwave::WavDecoder::~WavDecoder()
{
    if (m_source) close();
    if (m_src_adapter) delete m_src_adapter;
}

//***************************************************************************
Kwave::Decoder *Kwave::WavDecoder::instance()
{
    return new(std::nothrow) Kwave::WavDecoder();
}

//***************************************************************************
bool Kwave::WavDecoder::open(QWidget *widget, QIODevice &src)
{
    Kwave::FileInfo info;
    Kwave::LabelList labels;
    metaData().clear();

    Q_ASSERT(!m_source);
    bool need_repair = false;
    if (m_source) qWarning("WavDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
	qWarning("failed to open source !");
	return false;
    }

    QStringList main_chunks;
    main_chunks.append(_("RIFF")); /* RIFF, little-endian */
    main_chunks.append(_("RIFX")); /* RIFF, big-endian */
    main_chunks.append(_("FORM")); /* used in AIFF, BE or IFF/.lbm */
    main_chunks.append(_("LIST")); /* additional information */
    main_chunks.append(_("adtl")); /* Associated Data */

    Kwave::RIFFParser parser(src, main_chunks, m_known_chunks);

    // prepare a progress dialog
    QProgressDialog progress(widget);
    progress.setWindowTitle(i18n("Auto Repair"));
    progress.setModal(true);
    progress.setMinimumDuration(0);
    progress.setMaximum(100);
    progress.setAutoClose(true);
    progress.setValue(0);
    progress.setLabelText(i18n("Reading..."));
    connect(&parser,   SIGNAL(progress(int)),
            &progress, SLOT(setValue(int)));
    connect(&parser,   SIGNAL(action(QString)),
            &progress, SLOT(setLabelText(QString)));
    Kwave::ConfirmCancelProxy confirm_cancel(widget,
                       &progress, SIGNAL(canceled()),
                       &parser,   SLOT(cancel()));

    // parse, including endianness detection
    parser.parse();
    progress.reset();
    if (progress.wasCanceled()) return false;

//     qDebug("--- RIFF file structure after first pass ---");
//     parser.dumpStructure();

    // check if there is a RIFF chunk at all...
    Kwave::RIFFChunk *riff_chunk = parser.findChunk("/RIFF:WAVE");
    Kwave::RIFFChunk *fact_chunk = parser.findChunk("/RIFF:WAVE/fact");
    Kwave::RIFFChunk *fmt_chunk  = parser.findChunk("/RIFF:WAVE/fmt ");
    Kwave::RIFFChunk *data_chunk = parser.findChunk("/RIFF:WAVE/data");

    if (!riff_chunk || !fmt_chunk || !data_chunk || !parser.isSane()) {
	if (Kwave::MessageBox::warningContinueCancel(widget,
	    i18n("The file has been structurally damaged or "
	         "it is no WAV file.\n"
	         "Should Kwave try to repair it?"),
	    i18n("Kwave Auto Repair"),
	    i18n("&Repair")) != KMessageBox::Continue)
	{
	    // user didn't let us try :-(
	    return false;
	}

	need_repair = true;
    }

    // collect all missing chunks
    if (!riff_chunk) riff_chunk = parser.findMissingChunk("RIFF:WAVE");
    if (progress.wasCanceled()) return false;

    if (!fmt_chunk) {
	parser.findMissingChunk("fmt ");
	fmt_chunk = parser.findChunk("/RIFF:WAVE/fmt ");
	if (progress.wasCanceled()) return false;
	if (!fmt_chunk)  fmt_chunk  = parser.findChunk("fmt ");
	need_repair = true;
    }

    if (!data_chunk) {
	parser.findMissingChunk("data");
	data_chunk = parser.findChunk("/RIFF:WAVE/data");
	if (progress.wasCanceled()) return false;
	if (!data_chunk) data_chunk = parser.findChunk("data");
	need_repair = true;
    }

    // not everything found -> need heavy repair actions !
    if (!fmt_chunk || !data_chunk || need_repair) {
	qDebug("doing heavy repair actions...");
	parser.dumpStructure();
	parser.repair();
	parser.dumpStructure();
	if (progress.wasCanceled()) return false;

	if (!fmt_chunk)  fmt_chunk  = parser.findChunk("/RIFF:WAVE/fmt ");
	if (!fmt_chunk)  fmt_chunk  = parser.findChunk("/RIFF/fmt ");
	if (!fmt_chunk)  fmt_chunk  = parser.findChunk("fmt ");
	if (!data_chunk) data_chunk = parser.findChunk("/RIFF:WAVE/data");
	if (!data_chunk) data_chunk = parser.findChunk("/RIFF/data");
	if (!data_chunk) data_chunk = parser.findChunk("data");
	need_repair = true;
    }

    quint32 fmt_offset = 0;
    if (fmt_chunk) fmt_offset = fmt_chunk->dataStart();
//     qDebug("fmt chunk starts at 0x%08X", fmt_offset);

//     quint32 data_offset = 0;
    quint32 data_size = 0;
    if (data_chunk) {
// 	data_offset = data_chunk->dataStart();
	data_size   = data_chunk->physLength();
// 	qDebug("data chunk at 0x%08X (%u byte)", data_offset, data_size);
    }

    if (!data_size) {
	// hide progress bar, finally
	progress.hide();
	qApp->processEvents();

	// show final error message
	Kwave::MessageBox::sorry(widget,
	    i18n("The opened file is no WAV file or it is damaged:\n"
	    "There is not enough valid sound data.\n\n"
	    "It makes no sense to continue now."));
	return false;
    }

    // WORKAROUND: if there is a fact chunk, it must not contain zero
    //             for the moment the only workaround known is to open
    //             such broken files in repair mode
    if (fact_chunk) {
	qint64 offset = fact_chunk->dataStart();
	union {
	    quint32 len;
	    char    bytes[4];
	} fact;
	fact.len = 0;
	src.seek(offset);
	src.read(&(fact.bytes[0]), 4);
	if ((fact.len == 0x00000000) || (fact.len == 0xFFFFFFFF)) {
	    qWarning("WARNING: invalid 'fact' chunk content: 0x%08X "
	             "-> silent repair mode", fact.len);
	    need_repair = true;
	}
    }

    // final check for structural integrity:
    // we should have exactly one RIFF, fmt and data chunk !
    if ((parser.chunkCount("fmt ") != 1) ||
        (parser.chunkCount("data") != 1))
    {
	// hide progress bar, temporarily
	progress.hide();
	qApp->processEvents();

	if (Kwave::MessageBox::warningContinueCancel(widget,
	    i18n("The WAV file seems to be damaged:\n"
	         "Some chunks are duplicate or missing.\n\n"
	         "Kwave will only use the first ones and ignore\n"
	         "the rest. This might lead to loss of data."),
	    i18n("Kwave Auto Repair")
	    ) != KMessageBox::Continue)
	{
	    // user decided to abort and repair on his own
	    // good luck!
	    return false;
	}
	need_repair = true;

	// show progress bar again
	progress.show();
	qApp->processEvents();
    }

    // source successfully opened
    m_source = &src;

    // read the wave header
    Kwave::wav_fmt_header_t header;

    unsigned int rate = 0;
    unsigned int bits = 0;

    src.seek(fmt_offset);

    // get the encoded block of data from the mime source
    CHECK(src.size() > static_cast<qint64>(sizeof(Kwave::wav_header_t) + 8));

    // get the header
    src.read(reinterpret_cast<char *>(&header), sizeof(Kwave::wav_fmt_header_t));
    header.min.format      = qFromLittleEndian<quint16>(header.min.format);
    header.min.channels    = qFromLittleEndian<quint16>(header.min.channels);
    header.min.samplerate  = qFromLittleEndian<quint32>(header.min.samplerate);
    header.min.bytespersec = qFromLittleEndian<quint32>(header.min.bytespersec);
    header.min.blockalign  = qFromLittleEndian<quint16>(header.min.blockalign);
    header.min.bitwidth    = qFromLittleEndian<quint16>(header.min.bitwidth);

    unsigned int tracks = header.min.channels;
    rate = header.min.samplerate;
    bits = header.min.bitwidth;

    Kwave::WavFormatMap known_formats;
    QString format_name = known_formats.findName(header.min.format);

//     qDebug("-------------------------");
//     qDebug("wav header:");
//     qDebug("format      = 0x%04X, (%s)", header.min.format,
//                                          DBG(format_name));
//     qDebug("channels    = %d", header.min.channels);
//     qDebug("rate        = %u", header.min.samplerate);
//     qDebug("bytes/s     = %u", header.min.bytespersec);
//     qDebug("block align = %d", header.min.blockalign);
//     qDebug("bits/sample = %d", header.min.bitwidth);
//     qDebug("-------------------------");

    // open the file through libaudiofile :)
    if (need_repair) {
	QList<Kwave::RecoverySource *> *repair_list =
	    new(std::nothrow) QList<Kwave::RecoverySource *>();
	Q_ASSERT(repair_list);
	if (!repair_list) return false;

	Kwave::RIFFChunk *root = (riff_chunk) ? riff_chunk : parser.findChunk("");
// 	parser.dumpStructure();
//	qDebug("riff chunk = %p, parser.findChunk('')=%p", riff_chunk,
//	    parser.findChunk(""));
	repair(repair_list, root, fmt_chunk, data_chunk);
	m_src_adapter = new(std::nothrow)
	    Kwave::RepairVirtualAudioFile(*m_source, repair_list);
    } else {
	m_src_adapter = new(std::nothrow) Kwave::VirtualAudioFile(*m_source);
    }

    Q_ASSERT(m_src_adapter);
    if (!m_src_adapter) return false;

    m_src_adapter->open(m_src_adapter, Q_NULLPTR);

    AFfilehandle fh = m_src_adapter->handle();
    if (!fh || (m_src_adapter->lastError() >= 0)) {
	QString reason;

	switch (m_src_adapter->lastError()) {
	    case AF_BAD_NOT_IMPLEMENTED:
	        reason = i18n("Format or function is not implemented") +
		         _("\n(") + format_name + _(")");
	        break;
	    case AF_BAD_MALLOC:
	        reason = i18n("Out of memory");
	        break;
	    case AF_BAD_HEADER:
	        reason = i18n("file header is damaged");
	        break;
	    case AF_BAD_CODEC_TYPE:
	        reason = i18n("Invalid codec type") +
		         _("\n(") + format_name + _(")");
	        break;
	    case AF_BAD_OPEN:
	        reason = i18n("Opening the file failed");
	        break;
	    case AF_BAD_READ:
	        reason = i18n("Read access failed");
	        break;
	    case AF_BAD_SAMPFMT:
	        reason = i18n("Invalid sample format");
	        break;
	    default:
		reason = i18n("internal libaudiofile error #%1: '%2'",
		    m_src_adapter->lastError(),
		    m_src_adapter->lastErrorText()
		);
	}

	QString text= i18n("An error occurred while opening the file:\n'%1'",
	                    reason);
	Kwave::MessageBox::error(widget, text);

	return false;
    }

    AFframecount length = afGetFrameCount(fh, AF_DEFAULT_TRACK);
    tracks = afGetVirtualChannels(fh, AF_DEFAULT_TRACK);

    int af_sample_format;
    afGetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, &af_sample_format,
	reinterpret_cast<int *>(&bits));
    if (static_cast<signed int>(bits) < 0) bits = 0;
    Kwave::SampleFormat::Format fmt;
    switch (af_sample_format)
    {
	case AF_SAMPFMT_TWOSCOMP:
	    fmt = Kwave::SampleFormat::Signed;
	    break;
	case AF_SAMPFMT_UNSIGNED:
	    fmt = Kwave::SampleFormat::Unsigned;
	    break;
	case AF_SAMPFMT_FLOAT:
	    fmt = Kwave::SampleFormat::Float;
	    break;
	case AF_SAMPFMT_DOUBLE:
	    fmt = Kwave::SampleFormat::Double;
	    break;
	default:
	    fmt = Kwave::SampleFormat::Unknown;
	    break;
    }

    int af_compression = afGetCompression(fh, AF_DEFAULT_TRACK);
    const Kwave::Compression compression(
	Kwave::Compression::fromAudiofile(af_compression)
    );

    info.setRate(rate);
    info.setBits(bits);
    info.setTracks(tracks);
    info.setLength(length);
    info.set(Kwave::INF_SAMPLE_FORMAT, Kwave::SampleFormat(fmt).toInt());
    info.set(Kwave::INF_COMPRESSION, compression.toInt());

    // read in all info from the LIST (INFO) chunk
    Kwave::RIFFChunk *info_chunk = parser.findChunk("/RIFF:WAVE/LIST:INFO");
    if (info_chunk) {
	// found info chunk !
	Kwave::RIFFChunkList &list = info_chunk->subChunks();
	foreach (Kwave::RIFFChunk *chunk, list) {
	    if (!chunk) continue;
	    if (!m_property_map.containsChunk(chunk->name())) continue;

	    // read the content into a QString
	    Kwave::FileProperty prop = m_property_map.property(chunk->name());
	    unsigned int ofs = chunk->dataStart();
	    unsigned int len = chunk->dataLength();
	    QByteArray buffer(len + 1, 0x00);
	    src.seek(ofs);
	    src.read(buffer.data(), len);
	    buffer[len] = 0;
	    QString value;
	    value = QString::fromUtf8(buffer);
	    info.set(prop, value);
	}
    }

    // read in the Labels (cue list)
    Kwave::RIFFChunk *cue_chunk = parser.findChunk("/RIFF:WAVE/cue ");
    if (cue_chunk) {
	// found a cue list chunk !
	quint32 count;

	src.seek(cue_chunk->dataStart());
	src.read(reinterpret_cast<char *>(&count), 4);
	count = qFromLittleEndian<quint32>(count);
// 	unsigned int length = cue_chunk->dataLength();
// 	qDebug("cue list found: %u entries, %u bytes (should be: %u)",
// 	    count, length, count * (6 * 4) + 4);

	for (unsigned int i = 0; i < count; i++) {
	    quint32 data, index, position;
	    src.seek(cue_chunk->dataStart() + 4 + ((6 * 4) * i));
	    /*
	     * typedef struct {
	     *     quint32 dwIdentifier; <- index
	     *     quint32 dwPosition;   <- 0
	     *     quint32 fccChunk;     <- 'data'
	     *     quint32 dwChunkStart; <- 0
	     *     quint32 dwBlockStart; <- 0
	     *     quint32 dwSampleOffset; <- label.pos()
	     * } cue_list_entry_t;
	     */

	    /* dwIdentifier */
	    src.read(reinterpret_cast<char *>(&data), 4);
	    index = qFromLittleEndian<quint32>(data);

	    /* dwPosition (ignored) */
	    src.read(reinterpret_cast<char *>(&data), 4);

	    /* fccChunk */
	    src.read(reinterpret_cast<char *>(&data), 4);
	    /* we currently support only 'data' */
	    if (qstrncmp(reinterpret_cast<const char *>(&data), "data", 4)) {
		qWarning("cue list entry %d refers to '%s', "
                         "which is not supported -> skipped",
		         index, QByteArray(
		         reinterpret_cast<const char *>(&data), 4).data());
		continue;
	    }
	    src.read(reinterpret_cast<char *>(&data), 4);
	    /* dwChunkStart (must be 0) */
	    if (data != 0) {
		qWarning("cue list entry %d has dwChunkStart != 0 -> skipped",
		         index);
		continue;
	    }
	    src.read(reinterpret_cast<char *>(&data), 4);
	    /* dwBlockStart (must be 0) */
	    if (data != 0) {
		qWarning("cue list entry %d has dwBlockStart != 0 -> skipped",
		         index);
		continue;
	    }

	    src.read(reinterpret_cast<char *>(&data), 4); /* dwSampleOffset */
	    position = qFromLittleEndian<quint32>(data);

	    // as we now have index and position, find out the name
	    QByteArray name = "";
	    Kwave::RIFFChunk *adtl_chunk =
		parser.findChunk("/RIFF:WAVE/LIST:adtl");
	    if (adtl_chunk) {
                Kwave::RIFFChunk *labl_chunk = Q_NULLPTR;
		bool found = false;
		QListIterator<Kwave::RIFFChunk *> it(adtl_chunk->subChunks());
		while (it.hasNext()) {
		    quint32 labl_index;
		    labl_chunk = it.next();
		    /*
		     * typedef struct {
		     *     quint32 dwChunkID;    <- 'labl'
		     *     quint32 dwChunkSize;  (without padding !)
		     *     quint32 dwIdentifier; <- index
		     *     char    dwText[];       <- label->name()
		     * } label_list_entry_t;
		     */
		    if (labl_chunk->name() != "labl") continue;

		    data = 0;
		    src.seek(labl_chunk->dataStart());
		    src.read(reinterpret_cast<char *>(&data), 4); /* dwIdentifier */
		    labl_index = qFromLittleEndian<quint32>(data);
		    if (labl_index == index) {
			found = true;
			break; /* found it! */
		    }
		}
		if (found) {
		    Q_ASSERT(labl_chunk);
		    unsigned int len = labl_chunk->length();
		    if (len > 4) {
			len -= 4;
			name.resize(len);
			src.seek(labl_chunk->dataStart() + 4);
			src.read(static_cast<char *>(name.data()), len);
			if (name[name.count() - 1] != '\0')
			    name += '\0';
		    }
		}
	    }

	    if (!name.length()) {
// 		qDebug("cue list entry %d has no name", index);
		name = "";
	    }

	    // put a new label into the list
	    QString str = QString::fromUtf8(name);
	    labels.append(Kwave::Label(position, str));
	}
    }
    labels.sort();
    metaData().replace(Kwave::MetaDataList(info));
    metaData().replace(labels.toMetaDataList());

    // set up libaudiofile to produce Kwave's internal sample format
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
    afSetVirtualByteOrder(fh, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
    afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK,
	AF_SAMPFMT_TWOSCOMP, SAMPLE_STORAGE_BITS);

    return true;
}

//***************************************************************************
bool Kwave::WavDecoder::decode(QWidget */*widget*/, Kwave::MultiWriter &dst)
{
    Q_ASSERT(m_src_adapter);
    Q_ASSERT(m_source);
    if (!m_source) return false;
    if (!m_src_adapter) return false;

    AFfilehandle fh = m_src_adapter->handle();
    Q_ASSERT(fh);
    if (!fh) return false;

//     info().dump();
    const unsigned int tracks = dst.tracks();

    // allocate an array of Writers, for speeding up
    QVector<Kwave::Writer *>writers(tracks);
    Q_ASSERT(writers.count() == Kwave::toInt(dst.tracks()));
    if (writers.count() != Kwave::toInt(dst.tracks())) return false;
    for (unsigned int t = 0; t < tracks; t++)
	writers[t] = dst[t];
    Kwave::Writer **writer_fast = writers.data();

    unsigned int frame_size = Kwave::toUint(
	afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 1));

    // allocate a buffer for input data
    const unsigned int buffer_frames = (8 * 1024);
    sample_storage_t *buffer = static_cast<sample_storage_t *>(
	malloc(buffer_frames * frame_size));
    Q_ASSERT(buffer);
    if (!buffer) return false;

    // read in from the audiofile source
    Q_ASSERT(tracks == Kwave::FileInfo(metaData()).tracks());
    sample_index_t rest = Kwave::FileInfo(metaData()).length();
    while (rest) {
	unsigned int frames = buffer_frames;
	if (frames > rest) frames = Kwave::toUint(rest);
	unsigned int buffer_used = afReadFrames(fh,
	    AF_DEFAULT_TRACK, reinterpret_cast<char *>(buffer), frames);

	// break if eof reached
	if (!buffer_used) break;
	rest -= buffer_used;

	// split into the tracks
	sample_storage_t *p = buffer;
	for (unsigned int count = buffer_used; count; count--) {
	    for (unsigned int track = 0; track < tracks; track++) {
		sample_storage_t s = *p++;

		// adjust precision
		if (SAMPLE_STORAGE_BITS != SAMPLE_BITS) {
		    s /= (1 << (SAMPLE_STORAGE_BITS - SAMPLE_BITS));
		}

		// the following cast is only necessary if
		// sample_t is not equal to a sample_storage_t
		Q_ASSERT(writer_fast[track]);
		*(writer_fast[track]) << static_cast<sample_t>(s);
	    }
	}

	// abort if the user pressed cancel
	if (dst.isCanceled()) break;
    }

    // return with a valid Signal, even if the user pressed cancel !
    if (buffer) free(buffer);
    return true;
}

//***************************************************************************
bool Kwave::WavDecoder::repairChunk(
    QList<Kwave::RecoverySource *> *repair_list,
    Kwave::RIFFChunk *chunk, quint32 &offset)
{
    Q_ASSERT(chunk);
    Q_ASSERT(m_source);
    Q_ASSERT(repair_list);
    if (!chunk) return false;
    if (!m_source) return false;
    if (!repair_list) return false;

    char buffer[16];
    quint32 length;
    Kwave::RecoverySource *repair = Q_NULLPTR;

    // create buffer with header
    strncpy(buffer, chunk->name().data(), 4);
    length = (chunk->type() == Kwave::RIFFChunk::Main) ? chunk->physLength() :
                                                         chunk->dataLength();
    buffer[4] = (length      ) & 0xFF;
    buffer[5] = (length >>  8) & 0xFF;
    buffer[6] = (length >> 16) & 0xFF;
    buffer[7] = (length >> 24) & 0xFF;
    if (chunk->type() == Kwave::RIFFChunk::Main) {
	strncpy(&(buffer[8]), chunk->format().data(), 4);
	repair = new(std::nothrow) Kwave::RecoveryBuffer(offset, 12, buffer);
	qDebug("[0x%08X-0x%08X] - main header '%s' (%s), len=%u",
	      offset, offset+11, chunk->name().data(),
	      chunk->format().data(), length);
	offset += 12;
    } else {
	repair = new(std::nothrow) Kwave::RecoveryBuffer(offset, 8, buffer);
	qDebug("[0x%08X-0x%08X] - sub header '%s', len=%u",
	      offset, offset+7, chunk->name().data(), length);
	offset += 8;
    }
    Q_ASSERT(repair);
    if (!repair) return false;
    repair_list->append(repair);

    // map the chunk's data if not main or root
    if ((chunk->type() != Kwave::RIFFChunk::Root) &&
        (chunk->type() != Kwave::RIFFChunk::Main))
    {
	repair = new(std::nothrow) Kwave::RecoveryMapping(
	    offset, chunk->physLength(),
	    *m_source, chunk->dataStart()
	);
	qDebug("[0x%08X-0x%08X] - restoring from offset 0x%08X (%u)",
	      offset, offset+chunk->physLength()-1, chunk->dataStart(),
	      chunk->physLength());
	Q_ASSERT(repair);
	if (!repair) return false;
	repair_list->append(repair);

	offset += chunk->physLength();
    }

    // recursively go over all sub-chunks
    Kwave::RIFFChunkList &list = chunk->subChunks();
    foreach (Kwave::RIFFChunk *c, list) {
	if (!c) continue;
	if (!repairChunk(repair_list, c, offset))
	    return false;
    }

    return true;
}

//***************************************************************************
bool Kwave::WavDecoder::repair(QList<Kwave::RecoverySource *> *repair_list,
    Kwave::RIFFChunk *riff_chunk,
    Kwave::RIFFChunk *fmt_chunk,
    Kwave::RIFFChunk *data_chunk)
{
    Q_ASSERT(fmt_chunk);
    Q_ASSERT(data_chunk);
    if (!fmt_chunk || !data_chunk) return false;

    // --- first create a chunk tree for the structure ---

    // make a new "RIFF" chunk as root
    Kwave::RIFFChunk new_root(Q_NULLPTR, "RIFF", "WAVE", 0,0,0);
    new_root.setType(Kwave::RIFFChunk::Main);

    // create a new "fmt " chunk
    Kwave::RIFFChunk *new_fmt = new(std::nothrow)
        Kwave::RIFFChunk(&new_root, "fmt ",Q_NULLPTR, 0,
	fmt_chunk->physStart(), fmt_chunk->physLength());
    Q_ASSERT(new_fmt);
    if (!new_fmt) return false;
    new_root.subChunks().append(new_fmt);

    // create a new "data" chunk
    Kwave::RIFFChunk *new_data =
        new(std::nothrow) Kwave::RIFFChunk(&new_root, "data", Q_NULLPTR, 0,
	data_chunk->physStart(), data_chunk->physLength());
    Q_ASSERT(new_data);
    if (!new_data) return false;
    new_root.subChunks().append(new_data);

    // if we have a RIFF chunk, add it's subchunks, except "fmt " and "data"
    // we keep only pointers here, just for getting the right structure,
    // sizes and start positions.
    // NOTE: The sizes might be re-assigned and get invalid afterwards!!!
    if (riff_chunk) {
	Kwave::RIFFChunkList &list = riff_chunk->subChunks();
	foreach (Kwave::RIFFChunk *chunk, list) {
	    if (!chunk) continue;
	    if (chunk->name() == "fmt ") continue;
	    if (chunk->name() == "data") continue;
	    if (chunk->name() == "RIFF") continue;
	    if (chunk->type() == Kwave::RIFFChunk::Empty) continue;
	    if (chunk->type() == Kwave::RIFFChunk::Garbage) continue;

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
    quint32 offset = 0;
    bool repaired = repairChunk(repair_list, &new_root, offset);

    // clean up...
    new_root.subChunks().clear();
    delete new_fmt;
    delete new_data;

    return (repaired);
}

//***************************************************************************
void Kwave::WavDecoder::close()
{
    if (m_src_adapter) delete m_src_adapter;
    m_src_adapter = Q_NULLPTR;
    m_source = Q_NULLPTR;
}

//***************************************************************************
//***************************************************************************
