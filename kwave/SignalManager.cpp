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
#include <limits.h>
#include <math.h>

#include <qbitmap.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qptrlist.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtimer.h>

#include <kaboutdata.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprogress.h>
#include <kurl.h>

#include "mt/ThreadsafeX11Guard.h"

#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/InsertMode.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SampleWriter.h"
#include "libkwave/Signal.h"
#include "libkwave/Track.h"

#include "libgui/FileProgress.h"
#include "libgui/OverViewCache.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "CodecManager.h"
#include "SignalManager.h"
#include "SignalWidget.h"
#include "UndoAction.h"
#include "UndoDeleteAction.h"
#include "UndoDeleteTrack.h"
#include "UndoFileInfo.h"
#include "UndoInsertAction.h"
#include "UndoInsertTrack.h"
#include "UndoModifyAction.h"
#include "UndoSelection.h"
#include "UndoTransaction.h"
#include "UndoTransactionGuard.h"

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
    m_modified(false),
    m_modified_enabled(true),
    m_signal(),
    m_selection(0,0),
    m_last_length(0),
    m_playback_controller(),
    m_undo_enabled(false),
    m_undo_buffer(),
    m_redo_buffer(),
    m_undo_transaction(0),
    m_undo_transaction_level(0),
    m_undo_transaction_lock(),
    m_spx_undo_redo(this, SLOT(emitUndoRedoInfo())),
    m_undo_limit(64*1024*1024), // 64 MB (for testing) ###
    /** @todo the undo memory limit should be user-configurable. */
    m_file_info()
{
    // per default we use auto-delete for the und/redo buffers
    m_undo_buffer.setAutoDelete(true);
    m_redo_buffer.setAutoDelete(true);

    // connect to the track's signals
    Signal *sig = &m_signal;
    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track &)),
            this, SLOT(slotTrackInserted(unsigned int, Track &)));
    connect(sig, SIGNAL(sigTrackDeleted(unsigned int)),
            this, SLOT(slotTrackDeleted(unsigned int)));
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
int SignalManager::loadFile(const KURL &url)
{
    int res = 0;
    FileProgress *dialog = 0;
    m_name = url.filename();

    // enter and stay in not modified state
    enableModifiedChange(true);
    setModified(false);
    enableModifiedChange(false);

    // disable undo (discards all undo/redo data)
    disableUndo();

    QString mimetype = CodecManager::whatContains(url);
    qDebug("SignalManager::loadFile(%s) - [%s]",
           url.prettyURL().local8Bit().data(),
           mimetype.local8Bit().data());
    Decoder *decoder = CodecManager::decoder(mimetype);
    while (decoder) {
	// be sure that the current signal is really closed
	m_signal.close();

	// open the source file
	QString filename = url.path();
	QFile src(filename);
	if (!(res = decoder->open(m_parent_widget, src))) {
	    qWarning("unable to open source: '%s'",
	             url.prettyURL().local8Bit().data());
	    res = -EIO;
	    break;
	}

	// enter the filename/mimetype and size into the decoder
	QFileInfo fi(src);
	decoder->info().set(INF_FILENAME, fi.absFilePath());
	decoder->info().set(INF_FILESIZE, (unsigned int)src.size());
	decoder->info().set(INF_MIMETYPE, mimetype);

	// get the file info from the decoder
	m_file_info = decoder->info();

	// detect stream mode. if so, use one sample as display
	bool streaming = (!m_file_info.length());

	// we must change to open state to see the file while
	// it is loaded
	m_closed = false;
	m_empty = false;

	// create all tracks (empty)
	unsigned int track;
	const unsigned int tracks = decoder->info().tracks();
	const unsigned int length = decoder->info().length();
	Q_ASSERT(tracks);
	if (!tracks) break;

	for (track=0; track < tracks; ++track) {
	    Track *t = m_signal.appendTrack(length);
	    Q_ASSERT(t);
	    if (!t || (t->length() != length)) {
		qWarning("out of memory");
		res = -ENOMEM;
		break;
	    }
	}
	if (track < tracks) break;

	// create the multitrack writer as destination
	MultiTrackWriter writers;

	// if length was zero -> append mode / decode a stream ?
	InsertMode mode = (streaming) ? Append : Overwrite;
	openMultiTrackWriter(writers, allTracks(), mode, 0,
	    (length) ? length-1 : 0);

	// try to calculate the resulting length, but if this is
	// not possible, we try to use the source length instead
	FileInfo &info = decoder->info();
	unsigned int resulting_size = info.tracks() * info.length() *
	                              (info.bits() >> 3);
	bool use_src_size = (!resulting_size);
	if (use_src_size) resulting_size = src.size();

	//prepare and show the progress dialog
	dialog = new FileProgress(m_parent_widget,
	    filename, resulting_size,
	    info.length(), info.rate(), info.bits(), info.tracks());
	Q_ASSERT(dialog);
	if (use_src_size) {
	    // use source size for progress / stream mode
	    QObject::connect(decoder, SIGNAL(sourceProcessed(unsigned int)),
	                     dialog, SLOT(setBytePosition(unsigned int)));
	    QObject::connect(&writers, SIGNAL(progress(unsigned int)),
	                     dialog, SLOT(setLength(unsigned int)));
	} else {
	    // use resulting size for progress
	    QObject::connect(&writers, SIGNAL(progress(unsigned int)),
	                     dialog, SLOT(setValue(unsigned int)));
	}
	QObject::connect(dialog, SIGNAL(cancelled()),
	                 &writers, SLOT(cancel()));

	// now decode
	res = 0;
	if (!decoder->decode(m_parent_widget, writers)) {
	    qWarning("decoding failed.");
	    res = -EIO;
	} else {
	    // read information back from the decoder, some settings
	    // might have become available during the decoding process
	    m_file_info = decoder->info();
	    m_file_info.dump();
	}

	decoder->close();

	// check for length info in stream mode
	if (!res && streaming) {
	    // source was opened in stream mode -> now we have the length
	    writers.flush();
	    unsigned int new_length = writers.last();
	    if (new_length) new_length++;
	    m_file_info.setLength(new_length);
	} else {
	    m_file_info.setLength(this->length());
	    m_file_info.setTracks(tracks);
	}

	// update the length info in the progress dialog if needed
	if (dialog && use_src_size) {
	    dialog->setLength(m_file_info.length() * m_file_info.tracks());
	    dialog->setBytePosition(src.size());
	}

	emitStatusInfo();
	break;
    }
    if (!decoder) {
	qWarning("unknown file type");
	res = -EMEDIUMTYPE;
    } else {
	delete decoder;
    }

    // remember the last length
    m_last_length = length();

    // from now on, undo is enabled
    enableUndo();

    // modified can change from now on
    enableModifiedChange(true);

    if (dialog) delete dialog;
    if (res) close();

    return res;
}

//***************************************************************************
int SignalManager::save(const KURL &url, bool selection)
{
    int res = 0;
    unsigned int ofs = 0;
    unsigned int len = length();
    unsigned int tracks = this->tracks();
    unsigned int bits = this->bits();

    if (selection) {
	// zero-length -> nothing to do
	ofs = m_selection.offset();
	len = m_selection.length();
	tracks = selectedTracks().count();
    }

    if (!tracks || !len) {
	KMessageBox::error(m_parent_widget,
	    i18n("Signal is empty, nothing to save !"));
	return 0;
    }

    QString mimetype_name;
    mimetype_name = CodecManager::whatContains(url);
    qDebug("SignalManager::save(%s) - [%s] (%d bit, selection=%d)",
	url.prettyURL().local8Bit().data(), mimetype_name.data(),
	bits, selection);

    Encoder *encoder = CodecManager::encoder(mimetype_name);
    if (encoder) {
	// maybe we now have a new mime type
	m_file_info.set(INF_MIMETYPE, mimetype_name);

	// check if we loose information and as the user if this would
	// be acceptable if so
	QValueList<FileProperty> supported = encoder->supportedProperties();
	QMap<FileProperty, QVariant> properties(m_file_info.properties());
	bool all_supported = true;
	QMap<FileProperty, QVariant>::Iterator it;
	QString lost_properties;
	for (it=properties.begin(); it!=properties.end(); ++it) {
	    if ( (! supported.contains(it.key())) &&
                 (m_file_info.canLoadSave(it.key())) )
	    {
		qWarning("SignalManager::save(): unsupported property '%s'",
		    m_file_info.name(it.key()).data());
		all_supported = false;
		lost_properties += m_file_info.name(it.key()) + "\n";
	    }
	}
	if (!all_supported) {
	    // show a warning to the user and ask him if he wants to continue
	    if (KMessageBox::warningContinueCancel(m_parent_widget,
		i18n("Saving in this format will loose the following "
		     "additional file attribute(s):\n"
		     "%1\n"
		     "Do you still want to continue?").arg(
		     lost_properties)
		) != KMessageBox::Continue)
	    {
		delete encoder;
		return -1;
	    }
	}

	// open the destination file
	QString filename = url.path();
	QFile dst(filename);

	MultiTrackReader src;
	if (selection) {
	    openMultiTrackReader(src, selectedTracks(), ofs, ofs+len-1);
	} else {
	    openMultiTrackReader(src, allTracks(), ofs, ofs+len-1);
	}

	// update the file information
	m_file_info.setLength(len);
	m_file_info.setRate(rate());
	m_file_info.setBits(bits);
	m_file_info.setTracks(tracks);
	m_file_info.set(INF_FILENAME, filename);

	if (!m_file_info.contains(INF_SOFTWARE) &&
	    encoder->supportedProperties().contains(INF_SOFTWARE))
	{
	    // add our Kwave Software tag
	    const KAboutData *about_data = KGlobal::instance()->aboutData();
	    QString software = about_data->programName() + "-" +
	                       about_data->version() +
	                       i18n(" for KDE ") +
			       i18n(QString::fromLatin1(KDE_VERSION_STRING));
	    qDebug("adding software tag: '%s'",
	           software.local8Bit().data());
	    m_file_info.set(INF_SOFTWARE, software);
	}

	if (!m_file_info.contains(INF_CREATION_DATE) &&
	    encoder->supportedProperties().contains(INF_CREATION_DATE))
	{
	    // add a date tag
	    QDate now(QDate::currentDate());
	    QString date;
	    date = date.sprintf("%04d-%02d-%02d",
	    now.year(), now.month(), now.day());
	    QVariant value = date.utf8();
	    qDebug("adding date tag: '%s'",
	           date.local8Bit().data());
	    m_file_info.set(INF_CREATION_DATE, value);
	}

	//prepare and show the progress dialog
	FileProgress *dialog = new FileProgress(m_parent_widget,
	    filename, m_file_info.tracks()*m_file_info.length()*
	    (m_file_info.bits() >> 3),
	    m_file_info.length(), m_file_info.rate(), m_file_info.bits(),
	    m_file_info.tracks());
	Q_ASSERT(dialog);
	QObject::connect(&src,   SIGNAL(progress(unsigned int)),
	                 dialog, SLOT(setValue(unsigned int)));
	QObject::connect(dialog, SIGNAL(cancelled()),
	                 &src,   SLOT(cancel()));

	// invoke the encoder...
	if (!encoder->encode(m_parent_widget, src, dst, m_file_info)) {
	    KMessageBox::error(m_parent_widget,
	        i18n("An error occurred while saving the file!"));
	    res = -1;
	}

	delete encoder;
	if (dialog) {
	    if (dialog->isCancelled()) {
		// user really pressed cancel !
		KMessageBox::error(m_parent_widget,
		    i18n("The file has been truncated and "\
		         "might be corrupted!"));
	    }
	    delete dialog;
	}
    } else {
	KMessageBox::error(m_parent_widget,
	    i18n("Sorry, the file type is not supported!"));
	res = -EMEDIUMTYPE;
    }

    if (!res) {
	// saved without error -> no longer modified
	flushUndoBuffers();
	enableModifiedChange(true);
	setModified(false);
    }
    qDebug("SignalManager::save(): res=%d",res);
    return res;
}

//***************************************************************************
void SignalManager::newSignal(unsigned int samples, double rate,
                              unsigned int bits, unsigned int tracks)
{
    // enter and stay in not modified state
    enableModifiedChange(true);
    setModified(true);
    enableModifiedChange(false);

    // disable undo (discards all undo/redo data)
    disableUndo();

    m_file_info.clear();
    m_file_info.setRate(rate);
    m_file_info.setBits(bits);

    // now the signal is considered not to be empty
    m_closed = false;
    m_empty = false;

    // add all empty tracks
    m_file_info.setTracks(tracks);
    while (tracks--) m_signal.appendTrack(samples);

    // remember the last length
    m_last_length = samples;
    m_file_info.setLength(length());

    // from now on, undo is enabled
    enableUndo();
}

//***************************************************************************
void SignalManager::close()
{
    // fix the modified flag to false
    enableModifiedChange(true);
    setModified(false);
    enableModifiedChange(false);

    // reset the last length of the signal
    m_last_length = 0;

    // disable undo and discard all undo buffers
    // undo will be re-enabled when a signal is loaded or created
    disableUndo();

    // for safety: flush all undo/redo buffer
    flushUndoBuffers();
    flushRedoBuffer();

    m_empty = true;
    m_name = "";
    while (tracks()) deleteTrack(tracks()-1);
    m_signal.close();

    m_closed = true;
    m_selection.select(0,0);
    emitStatusInfo();
}

//***************************************************************************
const QMemArray<unsigned int> SignalManager::selectedTracks()
{
    unsigned int track;
    unsigned int count = 0;
    QMemArray<unsigned int> list(tracks());

    for (track=0; track < list.count(); track++) {
	if (!m_signal.trackSelected(track)) continue;
	list[count++] = track;
    }

    list.resize(count);
    return list;
}

//***************************************************************************
const QMemArray<unsigned int> SignalManager::allTracks()
{
    return m_signal.allTracks();
}

//***************************************************************************
SampleWriter *SignalManager::openSampleWriter(unsigned int track,
	InsertMode mode, unsigned int left, unsigned int right,
	bool with_undo)
{
    SampleWriter *writer = m_signal.openSampleWriter(
	track, mode, left, right);

    // skip all that undo stuff below if undo is not enabled
    // or the writer creation has failed
    if (!undoEnabled() || !writer || !with_undo) return writer;

    // get the real/effective left and right sample
    left  = writer->first();
    right = writer->last();

    // enter a new undo transaction and let it close when the writer closes
    UndoTransactionGuard guard(*this, 0);
    startUndoTransaction();
    QObject::connect(writer, SIGNAL(destroyed()),
                     this,   SLOT(closeUndoTransaction()));

    // create an undo action for the modification of the samples
    UndoAction *action = 0;
    switch (mode) {
	case Append:
	    qDebug("SignalManager::openSampleWriter(): "\
	           "NO UNDO FOR APPEND YET !");
	    break;
	case Insert:
	    action = new UndoInsertAction(track, left, right-left+1);
	    if (action) {
		QObject::connect(
		    writer, SIGNAL(sigSamplesWritten(unsigned int)),
		    (UndoInsertAction*)action, SLOT(setLength(unsigned int)));
	    }
	    break;
	case Overwrite:
	    action = new UndoModifyAction(track, left, right-left+1);
	    break;
    }
    Q_ASSERT(action);

    if (m_undo_enabled) {
	ThreadsafeX11Guard x11_guard;
	if (!registerUndoAction(action)) {
	    // creating/starting the action failed, so fail now.
	    // close the writer and return 0 -> abort the operation
	    qDebug("SignalManager::openSampleWriter(): register failed"); // ###
	    if (action) delete action;
	    delete writer;
	    return 0;
	} else {
	    action->store(*this);
	}
    }

    // Everything was ok, the action now is owned by the current undo
    // transaction. The transaction is owned by the SignalManager and
    // will be closed when the writer gets closed.
    return writer;
}

//***************************************************************************
void SignalManager::openMultiTrackReader(MultiTrackReader &readers,
    const QMemArray<unsigned int> &track_list,
    unsigned int first, unsigned int last)
{
    m_signal.openMultiTrackReader(readers, track_list, first, last);
}

//***************************************************************************
void SignalManager::openMultiTrackWriter(MultiTrackWriter &writers,
    const QMemArray<unsigned int> &track_list, InsertMode mode,
    unsigned int left, unsigned int right)
{
    UndoTransactionGuard guard(*this, 0);
    unsigned int count = track_list.count();
    unsigned int track;
    writers.clear();
    writers.resize(count);

    for (unsigned int i=0; i < count; i++) {
	track = track_list[i];
	// NOTE: this function is *nearly* identical to the one in the
	//       Signal class, except for undo support
	SampleWriter *s = openSampleWriter(track, mode, left, right, true);
	if (s) {
	    writers.insert(i, s);
	} else {
	    // out of memory or aborted
	    qDebug("Signal::openMultiTrackWriter: "\
	          "out of memory or aborted");
	    writers.clear();
	    return;
	}
    }
}

//***************************************************************************
bool SignalManager::executeCommand(const QString &command)
{
    unsigned int offset = m_selection.offset();
    unsigned int length = m_selection.length();
    double rate = m_file_info.rate();

    if (!command.length()) return true;
    Parser parser(command);

    if (false) {
    CASE_COMMAND("undo")
	undo();
    CASE_COMMAND("redo")
	redo();
    CASE_COMMAND("copy")
	ClipBoard &clip = KwaveApp::clipboard();
	clip.copy(m_signal, selectedTracks(), offset, length, rate);
    CASE_COMMAND("paste")
	paste(KwaveApp::clipboard(), offset, length);
    CASE_COMMAND("cut")
	ClipBoard &clip = KwaveApp::clipboard();
	clip.copy(m_signal, selectedTracks(), offset, length, rate);
	UndoTransactionGuard undo(*this, i18n("cut"));
	deleteRange(offset, length);
    CASE_COMMAND("crop")
	UndoTransactionGuard undo(*this, i18n("crop"));
	unsigned int rest = this->length() - offset;
	rest = (rest > length) ? (rest-length) : 0;
	QMemArray<unsigned int> tracks = selectedTracks();
	if (saveUndoDelete(tracks, offset+length, rest) &&
	    saveUndoDelete(tracks, 0, offset))
	{
	    unsigned int count = tracks.count();
	    while (count--) {
		m_signal.deleteRange(count, offset+length, rest);
		m_signal.deleteRange(count, 0, offset);
	    }
	    selectRange(0, length);
	} else {
	    abortUndoTransaction();
	}
    CASE_COMMAND("delete")
	deleteRange(offset, length);
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
//		    Q_ASSERT(signal.at(i));
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
    CASE_COMMAND("add_track")
	appendTrack();
    CASE_COMMAND("delete_track")
	Parser parser(command);
	unsigned int track = parser.toUInt();
	deleteTrack(track);
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
void SignalManager::paste(ClipBoard &clipboard, unsigned int offset,
                          unsigned int length)
{
    QMemArray<unsigned int> selected_tracks = selectedTracks();
    if (clipboard.isEmpty()) return;
    if (!selected_tracks.size()) return;

    UndoTransactionGuard u(*this, i18n("paste"));

    // delete the current selection (with undo)
    if (length <= 1) length = 0; // do not paste single samples !
    if (length && !deleteRange(offset, length)) {
	abortUndoTransaction();
	return;
    }

    // if the signal has no tracks, create new ones
    if (!tracks()) {
	unsigned int missing = clipboard.tracks();
	qDebug("SignalManager::paste(): appending %u tracks", missing);
	while (missing--) appendTrack();
    }

    // open the clipboard as source
    MultiTrackReader src;
    clipboard.openMultiTrackReader(src);
    if (src.isEmpty()) {
	abortUndoTransaction();
	return;
    }

    // open a stream into the signal
    MultiTrackWriter dst;
    openMultiTrackWriter(dst, selectedTracks(), Insert,
                         offset, offset+clipboard.length()-1);
    if (dst.isEmpty()) {
	abortUndoTransaction();
	return;
    }

    // transfer the content
    dst << src;

    // set the selection to the inserted range
    selectRange(offset, clipboard.length());
}

//***************************************************************************
void SignalManager::appendTrack()
{
    UndoTransactionGuard u(*this, i18n("append track"));
    insertTrack(tracks());
}

//***************************************************************************
void SignalManager::insertTrack(unsigned int index)
{
    UndoTransactionGuard u(*this, i18n("insert track"));

    if (m_undo_enabled) {
	UndoAction *undo = new UndoInsertTrack(m_signal, index);
	if (!registerUndoAction(undo)) {
	    if (undo) delete undo;
	    abortUndoTransaction();
	    return;
	}
    }

    unsigned int count = tracks();
    Q_ASSERT(index <= count);
    if (index > count) index = count;

    // if the signal is currently empty, use the last
    // known length instead of the current one
    unsigned int len = (count) ? length() : m_last_length;

    if (index >= count) {
	// do an "append"
//	qDebug("SignalManager::insertTrack(): appending");
	m_signal.appendTrack(len);
    } else {
	// insert into the list
	qWarning("m_signal.insertTrack(index, len) - NOT IMPLEMENTED !");
	// ### TODO ### m_signal.insertTrack(index, len);
    }

    // remember the last length
    m_last_length = length();
}

//***************************************************************************
void SignalManager::deleteTrack(unsigned int index)
{
    UndoTransactionGuard u(*this, i18n("delete track"));

    if (m_undo_enabled) {
	UndoAction *undo = new UndoDeleteTrack(m_signal, index);
	if (!registerUndoAction(undo)) {
	    if (undo) delete undo;
	    abortUndoTransaction();
	    return;
	}
    }

    setModified(true);
    m_signal.deleteTrack(index);
}

//***************************************************************************
void SignalManager::slotTrackInserted(unsigned int index,
	Track &track)
{
    ThreadsafeX11Guard x11_guard;

    setModified(true);
    emit sigTrackInserted(index, track);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotTrackDeleted(unsigned int index)
{
    ThreadsafeX11Guard x11_guard;

    setModified(true);
    emit sigTrackDeleted(index);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesInserted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    ThreadsafeX11Guard x11_guard;

    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);
    emit sigSamplesInserted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesDeleted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    ThreadsafeX11Guard x11_guard;

    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);
    emit sigSamplesDeleted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesModified(unsigned int track,
	unsigned int offset, unsigned int length)
{
    ThreadsafeX11Guard x11_guard;

    setModified(true);
    emit sigSamplesModified(track, offset, length);
}

//***************************************************************************
SignalManager::~SignalManager()
{
    close();
}

//***************************************************************************
bool SignalManager::deleteRange(unsigned int offset, unsigned int length,
                                const QMemArray<unsigned int> &track_list)
{
    if (!length) return true; // nothing to do
    UndoTransactionGuard undo(*this, i18n("delete"));

    unsigned int count = track_list.count();
    if (!count) return true; // nothing to do

    // first store undo data for all tracks
    unsigned int track;
    unsigned int i;

    if (m_undo_enabled) {
	for (i=0; i < count; i++) {
	    track = track_list[i];
	    UndoAction *undo = new UndoDeleteAction(track, offset, length);
	    if (!registerUndoAction(undo)) {
		// abort
		if (undo) delete undo;
		abortUndoTransaction();
		return false;
	    }
	}
    }

    // then delete the ranges in all tracks
    for (i=0; i < count; i++) {
	track = track_list[i];
	m_signal.deleteRange(track, offset, length);
    }

    // finally set the current selection to zero-length
    selectRange(m_selection.offset(), 0);

    return true;
}

//***************************************************************************
bool SignalManager::deleteRange(unsigned int offset, unsigned int length)
{
    return deleteRange(offset, length, allTracks());
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
void SignalManager::selectTracks(QMemArray<unsigned int> &track_list)
{
    unsigned int track;
    unsigned int n_tracks = tracks();
    for (track = 0; track < n_tracks; track++) {
	bool old_select = m_signal.trackSelected(track);
	bool new_select = track_list.contains(track);
	if (new_select != old_select) {
	    m_signal.selectTrack(track, new_select);
	    emit sigTrackSelected(track, new_select);
	}
    }
}

//***************************************************************************
void SignalManager::selectTrack(unsigned int track, bool select)
{
    bool old_select = m_signal.trackSelected(track);
    if (select != old_select) {
	m_signal.selectTrack(track, select);
	emit sigTrackSelected(track, select);

	// if selection changed during playback, reload with pause/continue
	if (m_playback_controller.running()) {
	    m_playback_controller.reload();
	}
    }
}

//***************************************************************************
/* ### -> should go to a "codec_ascii" plugin...

int SignalManager::loadAscii()
{
    float value;
    int cnt = 0;
    float max = 0;
    float amp;
    int *sample = 0;

    FILE *sigin = fopen(m_name.local8Bit(), "r");
    if (!sigin) {
	KMessageBox::error(0, i18n("File does not exist !"), i18n("Info"), 2);
	return -ENOENT;
    }

    // scan in the first line for the sample rate
    // ### to be done ###
    rate = 44100;        //will be asked in requester

    // scan for number of channels
    m_channels = 1;

    // loop over all samples in file to get maximum value
    while (!feof(sigin)) {
	if (fscanf (sigin, "%e\n", &value) == 1) {
	    if ( value > max) max = value;
	    if ( -value > max) max = -value;
	    cnt++;
	}
    }
    qDebug("SignalManager::loadAscii(): reading ascii file with %d samples",
	  cnt);    // ###

    // get the maximum and the scale
    amp = (float)((1 << 23)-1) / max;
    Signal *new_signal = new Signal(cnt, rate);
    Q_ASSERT(new_signal);

    if (new_signal) {
	signal.append(new_signal);
	new_signal->setBits(24);
	sample = new_signal->getSample();
    }

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
### */

//***************************************************************************
void SignalManager::emitStatusInfo()
{
    emit sigStatusInfo(length(), tracks(), rate(), bits());
}

/* ### -> should go to a "codec_ascii" plugin...

int SignalManager::exportAscii(const char *name)
{
    Q_ASSERT(name);
    if (!name) return ;

    unsigned int length = getLength();
    Q_ASSERT(length);
    if (!length) return;

    Q_ASSERT(m_channels);
    if (!m_channels) return;

    FILE *sigout = fopen(name, "w");
    Q_ASSERT(sigout);
    if (!sigout) return;

    //prepare and show the progress dialog
    char progress_title[256];
    char str_channels[128];
    if (m_channels == 1)
	strncpy(str_channels, i18n("Mono"), sizeof(str_channels));
    else if (m_channels == 2)
	strncpy(str_channels, i18n("Stereo"), sizeof(str_channels));
    else
	snprintf(str_channels, sizeof(str_channels), "%d-channel", m_channels);
    snprintf(progress_title, sizeof(progress_title),
	i18n("Exporting %s ASCII file :"),
	str_channels);

    QString title = i18n(progress_title);
    ProgressDialog *dialog = new ProgressDialog(100, title);
    delete[] title;
    if (dialog) dialog->show();

    // loop for writing data
    int *sample;
    int percent_count = 0;
    const double scale_y = (1 << 23)-1;
    for (unsigned int pos = 0; pos < length ; pos++) {
	// loop over all channels
	for (unsigned int channel=0; channel < m_channels; channel++) {
	    sample = signal.at(channel)->getSample();
	    if (!sample) continue;

	    if (channel != 0) fprintf(sigout, ",");
	    fprintf(sigout, "%0.8e", (double)sample[pos]/scale_y);
	}
	fprintf(sigout,"\n");

	// update the progress bar
	percent_count--;
	if (dialog && (percent_count <= 0)) {
	    percent_count = length / 200;
	    float percent = (float)pos;
	    percent /= (float)length;
	    percent *= 100.0;
	    dialog->setProgress (percent);
	}
    }

    if (dialog) delete dialog;
    fclose(sigout);
    return -1;
}
### */

//***************************************************************************
PlaybackController &SignalManager::playbackController()
{
    return m_playback_controller;
}

//***************************************************************************
void SignalManager::startUndoTransaction(const QString &name)
{
    if (!m_undo_enabled) return; // undo is currently not enabled

    MutexGuard lock(m_undo_transaction_lock);

    // increase recursion level
    m_undo_transaction_level++;

    // start/create a new transaction if none existing
    if (!m_undo_transaction) {
	// if a new action starts, discard all redo actions !
	flushRedoBuffer();

	m_undo_transaction = new UndoTransaction(name);
	Q_ASSERT(m_undo_transaction);
	if (!m_undo_transaction) return;

	// if it is the start of the transaction, also create one
	// for the selection
	UndoAction *selection = new UndoSelection(*this);
	Q_ASSERT(selection);
	if (selection) {
	    selection->store(*this);
	    m_undo_transaction->append(selection);
	}
    }
}

//***************************************************************************
void SignalManager::closeUndoTransaction()
{
    MutexGuard lock(m_undo_transaction_lock);

    // decrease recursion level
    if (!m_undo_transaction_level) return; // undo was not enabled ?
    m_undo_transaction_level--;

    if (!m_undo_transaction_level) {
	// append the current transaction to the undo buffer if
	// not empty
	if (m_undo_transaction) {
	    if (!m_undo_transaction->isEmpty()) {
		m_undo_buffer.append(m_undo_transaction);
	    } else {
		qDebug("SignalManager::closeUndoTransaction(): empty");
		delete m_undo_transaction;
	    }
	}

	// declare the current transaction as "closed"
	m_undo_transaction = 0;
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
    Q_ASSERT(m_undo_transaction_level == 0);

    flushUndoBuffers();
    m_undo_enabled = false;
}

//***************************************************************************
void SignalManager::flushUndoBuffers()
{
    MutexGuard lock(m_undo_transaction_lock);

    Q_ASSERT(m_undo_transaction_level == 0);

    // close the current transaction
    if (m_undo_transaction) delete m_undo_transaction;
    m_undo_transaction = 0;
    m_undo_transaction_level = 0;

    // if the signal was modified, it will stay in this state, it is
    // not possible to change to "non-modified" state through undo
    if ((!m_undo_buffer.isEmpty()) && (m_modified)) {
	enableModifiedChange(false);
    }

    // clear all buffers
    m_undo_buffer.clear();
    m_redo_buffer.clear();

    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::abortUndoTransaction()
{
    MutexGuard lock(m_undo_transaction_lock);

    if (!m_undo_transaction) return;
    delete m_undo_transaction;
    m_undo_transaction = 0;
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
    Q_ASSERT(action);
    if (!action) return false;

    // if undo is not enabled, this will fail -> no memory leak!
    Q_ASSERT(m_undo_enabled);
    if (!m_undo_enabled) return false;

    unsigned int needed_size  = action->undoSize();
    unsigned int needed_mb = needed_size  >> 20;
    unsigned int limit_mb  = m_undo_limit >> 20;

    if (needed_mb > limit_mb) {
	// Allow: discard buffers and omit undo
	m_undo_buffer.clear();
	m_redo_buffer.clear();

	// close the current transaction
	if (m_undo_transaction) delete m_undo_transaction;
	m_undo_transaction = 0;

	// if the signal was modified, it will stay in this state, it is
	// not possible to change to "non-modified" state through undo
	if ((!m_undo_buffer.isEmpty()) && (m_modified)) {
	    enableModifiedChange(false);
	}

	m_spx_undo_redo.AsyncHandler();
	return true;
    }

    // undo has been aborted before ?
    if (!m_undo_transaction) return true;

    // make room...
    freeUndoMemory(needed_size);

    // now we have enough place to append the undo action
    // and store all undo info
    m_undo_transaction->append(action);
    action->store(*this);

    return true;
}

//***************************************************************************
bool SignalManager::saveUndoDelete(QMemArray<unsigned int> &track_list,
                                   unsigned int offset, unsigned int length)
{
    if (!m_undo_enabled) return true;
    if (track_list.isEmpty()) return true;

    unsigned int count = track_list.count();
    QPtrList<UndoDeleteAction> undo_list;
    undo_list.setAutoDelete(true);

    // loop over all tracks
    while (m_undo_enabled && count--) {
	unsigned int t = track_list[count];
	UndoDeleteAction *action = new UndoDeleteAction(t, offset, length);
	if (!registerUndoAction(action)) {
	    // registration or creation failed
	    undo_list.clear();
	    return false;
	}
    }

    // do not delete the actions from the list, so it's important
    // to disable the auto-delete feature now!
    undo_list.setAutoDelete(false);
    return true;
}

//***************************************************************************
unsigned int SignalManager::usedUndoRedoMemory()
{
    unsigned int size = 0;

    QPtrListIterator<UndoTransaction> undo_it(m_undo_buffer);
    for ( ; undo_it.current(); ++undo_it ) {
	size += undo_it.current()->undoSize();
    }
    QPtrListIterator<UndoTransaction> redo_it(m_redo_buffer);
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
    while (!m_undo_buffer.isEmpty() && (size > m_undo_limit)) {
	unsigned int s = m_undo_buffer.first()->undoSize();
	size = (size >= s) ? (size - s) : 0;
	m_undo_buffer.removeFirst();

	// if the signal was modified, it will stay in this state, it is
	// not possible to change to "non-modified" state through undo
	if ((!m_undo_buffer.isEmpty()) && (m_modified)) {
	    enableModifiedChange(false);
	}
    }

    // remove old redo actions if still not enough memory
    while (!m_redo_buffer.isEmpty() && (size > m_undo_limit)) {
	unsigned int s = m_redo_buffer.last()->undoSize();
	size = (size >= s) ? (size - s) : 0;
	m_redo_buffer.removeLast();
    }
}

//***************************************************************************
void SignalManager::emitUndoRedoInfo()
{
    ThreadsafeX11Guard x11_guard;

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
    if (m_undo_buffer.isEmpty()) return;
    UndoTransaction *undo_transaction = m_undo_buffer.last();
    Q_ASSERT(undo_transaction);
    if (!undo_transaction) return;

    // remove the undo transaction from the list without deleting it
    m_undo_buffer.setAutoDelete(false);
    m_undo_buffer.removeLast();
    m_undo_buffer.setAutoDelete(true);

    // get free memory for redo
    // also bear in mind that the undo transaction we removed is still
    // allocated and has to be subtracted from the undo limit. As we
    // don't want to modify the limit, we increase the needed size.
    unsigned int redo_size = undo_transaction->redoSize();
    unsigned int undo_size = undo_transaction->undoSize();

    UndoTransaction *redo_transaction = 0;
    if (undo_size + redo_size > m_undo_limit) {
	// not enough memory for redo
	qWarning("SignalManager::undo(): not enough memory for redo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(undo_size + redo_size);

	// create a new redo transaction
	QString name = undo_transaction->description();
	redo_transaction = new UndoTransaction(name);
	Q_ASSERT(redo_transaction);
    }

    // if *one* redo fails, all following redoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!redo_transaction) {
	flushRedoBuffer();
	qDebug("SignalManager::undo(): redo buffer flushed!"); // ###
    } else {
	m_redo_buffer.prepend(redo_transaction);
    }

    // execute all undo actions and store the resulting redo
    // actions into the redo transaction
    while (!undo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;

	// unqueue the undo action
	undo_action = undo_transaction->last();
	undo_transaction->setAutoDelete(false);
	undo_transaction->removeLast();
	undo_transaction->setAutoDelete(true);
	Q_ASSERT(undo_action);
	if (!undo_action) continue;

	// execute the undo operation
	redo_action = undo_action->undo(*this, (redo_transaction != 0));

	// remove the old undo action if no longer used
	if (redo_action != undo_action) {
	    delete undo_action;
	}

	// queue the action into the redo transaction
	if (redo_action) {
	    if (redo_transaction) {
		redo_transaction->prepend(redo_action);
	    } else {
		// redo is not usable :-(
		delete redo_action;
	    }
	}
    }

    // now the undo_transaction should be empty -> get rid of it
    Q_ASSERT(undo_transaction->isEmpty());
    delete undo_transaction;

    if (redo_transaction && (redo_transaction->count() <= 1)) {
	// if there is not more than the UndoSelection action,
	// there are no real redo actions -> no redo possible
	qWarning("SignalManager::undo(): no redo possible");
	m_redo_buffer.remove(redo_transaction);
    }

    if (m_undo_buffer.isEmpty() && m_modified) {
	// try to return to non-modified mode (might be a nop if
	// not enabled)
	setModified(false);
    }

    // finished / buffers have changed, emit new undo/redo info
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::redo()
{
    MutexGuard lock(m_undo_transaction_lock);

    // get the last redo transaction and abort if none present
    if (m_redo_buffer.isEmpty()) return;
    UndoTransaction *redo_transaction = m_redo_buffer.first();
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
	qWarning("SignalManager::redo(): not enough memory for undo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(undo_size + redo_size);

	// create a new undo transaction
	QString name = redo_transaction->description();
	undo_transaction = new UndoTransaction(name);
	Q_ASSERT(undo_transaction);
    }

    // if *one* undo fails, all following undoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!undo_transaction) {
	m_undo_buffer.clear();
	qDebug("SignalManager::redo(): undo buffer flushed!");
    } else {
	m_undo_buffer.append(undo_transaction);
    }

    // execute all redo actions and store the resulting undo
    // actions into the undo transaction
    while (!redo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;

	// unqueue the undo action
	redo_action = redo_transaction->first();
	redo_transaction->setAutoDelete(false);
	redo_transaction->removeFirst();
	redo_transaction->setAutoDelete(true);
	Q_ASSERT(redo_action);
	if (!redo_action) continue;

	// execute the redo operation
	undo_action = redo_action->undo(*this, (undo_transaction != 0));

	// remove the old redo action if no longer used
	if (redo_action != undo_action) {
	    delete redo_action;
	}

	// queue the action into the undo transaction
	if (undo_action) {
	    if (undo_transaction) {
		undo_transaction->append(undo_action);
	    } else {
		// undo is not usable :-(
		delete undo_action;
	    }
	}
    }

    // now the redo_transaction should be empty -> get rid of it
    Q_ASSERT(redo_transaction->isEmpty());
    delete redo_transaction;

    if (undo_transaction && (undo_transaction->count() <= 1)) {
	// if there is not more than the UndoSelection action,
	// there are no real undo actions -> no undo possible
	qWarning("SignalManager::redo(): no undo possible");
	m_undo_buffer.remove(undo_transaction);
    }

    // finished / buffers have changed, emit new undo/redo info
    m_spx_undo_redo.AsyncHandler();
}

//***************************************************************************
void SignalManager::setModified(bool mod)
{
    if (!m_modified_enabled) return;

    if (m_modified != mod) {
	m_modified = mod;
	qDebug("SignalManager::setModified(%d)",mod);
	emit sigModified(m_modified);
    }
}

//***************************************************************************
void SignalManager::enableModifiedChange(bool en)
{
    m_modified_enabled = en;
}

//***************************************************************************
void SignalManager::setFileInfo(FileInfo &new_info, bool with_undo)
{
    ThreadsafeX11Guard x11_guard;

    if (m_undo_enabled && with_undo) {
	/* save data for undo */
	UndoTransactionGuard undo_transaction(*this, i18n("modify file info"));
	UndoFileInfo *undo = new UndoFileInfo(*this);
	Q_ASSERT(undo);
	if (!undo) return;
	if (!registerUndoAction(undo)) return;
    }

    m_file_info = new_info;
    setModified(true);
    emitStatusInfo();
    emitUndoRedoInfo();
}

//***************************************************************************
//***************************************************************************
