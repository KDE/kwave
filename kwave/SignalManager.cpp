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

#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>

#include <kaboutdata.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kprogressdialog.h>
#include <kurl.h>

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
#include "libgui/MessageBox.h"
#include "libgui/OverViewCache.h"

#include "KwaveApp.h"
#include "ClipBoard.h"
#include "CodecManager.h"
#include "SignalManager.h"
#include "SignalWidget.h"
#include "UndoAction.h"
#include "UndoAddLabelAction.h"
#include "UndoDeleteAction.h"
#include "UndoDeleteLabelAction.h"
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
    m_undo_limit(64*1024*1024), // 64 MB (for testing) ###
    /** @todo the undo memory limit should be user-configurable. */
    m_file_info()
{
    // connect to the track's signals
    Signal *sig = &m_signal;
    connect(sig, SIGNAL(sigTrackInserted(unsigned int, Track *)),
            this, SLOT(slotTrackInserted(unsigned int, Track *)));
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
int SignalManager::loadFile(const KUrl &url)
{
    int res = 0;
    FileProgress *dialog = 0;

    // enter and stay in not modified state
    enableModifiedChange(true);
    setModified(false);
    enableModifiedChange(false);

    // disable undo (discards all undo/redo data)
    disableUndo();

    QString mimetype = CodecManager::whatContains(url);
    qDebug("SignalManager::loadFile(%s) - [%s]",
           url.prettyUrl().toLocal8Bit().data(),
           mimetype.toLocal8Bit().data());
    Decoder *decoder = CodecManager::decoder(mimetype);
    while (decoder) {
	// be sure that the current signal is really closed
	m_signal.close();

	// open the source file
	QString filename = url.path();
	QFile src(filename);
	if (!(res = decoder->open(m_parent_widget, src))) {
	    qWarning("unable to open source: '%s'",
	             url.prettyUrl().toLocal8Bit().data());
	    res = -EIO;
	    break;
	}

	// enter the filename/mimetype and size into the decoder
	QFileInfo fi(src);
	decoder->info().set(INF_FILENAME, fi.absoluteFilePath());
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
	// if length was zero -> append mode / decode a stream ?
	InsertMode mode = (streaming) ? Append : Overwrite;
	MultiTrackWriter writers(*this, allTracks(), mode, 0,
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
	QObject::connect(dialog, SIGNAL(canceled()),
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
	res = -EINVAL;
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
int SignalManager::save(const KUrl &url, bool selection)
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
	Kwave::MessageBox::error(m_parent_widget,
	    i18n("Signal is empty, nothing to save !"));
	return 0;
    }

    QString mimetype_name;
    mimetype_name = CodecManager::whatContains(url);
    qDebug("SignalManager::save(%s) - [%s] (%d bit, selection=%d)",
	url.prettyUrl().toLocal8Bit().data(),
	mimetype_name.toLocal8Bit().data(),
	bits, selection);

    Encoder *encoder = CodecManager::encoder(mimetype_name);
    if (encoder) {
	// maybe we now have a new mime type
	m_file_info.set(INF_MIMETYPE, mimetype_name);

	// check if we lose information and ask the user if this would
	// be acceptable if so
	QList<FileProperty> supported = encoder->supportedProperties();
	QMap<FileProperty, QVariant> properties(m_file_info.properties());
	bool all_supported = true;
	QMap<FileProperty, QVariant>::Iterator it;
	QString lost_properties;
	for (it=properties.begin(); it!=properties.end(); ++it) {
	    if ( (! supported.contains(it.key())) &&
                 (m_file_info.canLoadSave(it.key())) )
	    {
		qWarning("SignalManager::save(): unsupported property '%s'",
		    m_file_info.name(it.key()).toLocal8Bit().data());
		all_supported = false;
		lost_properties += i18n("%1", m_file_info.name(it.key()))
		    + "\n";
	    }
	}
	if (!all_supported) {
	    // show a warning to the user and ask him if he wants to continue
	    if (Kwave::MessageBox::warningContinueCancel(m_parent_widget,
		i18n("Saving in this format will lose the following "
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

	MultiTrackReader src(*this,
	    (selection) ? selectedTracks() : allTracks(),
	    ofs, ofs+len-1);

	// update the file information
	m_file_info.setLength(len);
	m_file_info.setRate(rate());
	m_file_info.setBits(bits);
	m_file_info.setTracks(tracks);

	if (!m_file_info.contains(INF_SOFTWARE) &&
	    encoder->supportedProperties().contains(INF_SOFTWARE))
	{
	    // add our Kwave Software tag
	    const KAboutData *about_data =
		KGlobal::mainComponent().aboutData();
	    QString software = about_data->programName() + "-" +
	                       about_data->version() +
	                       i18n(" for KDE ") +
			       i18n(KDE_VERSION_STRING);
	    qDebug("adding software tag: '%s'",
	           software.toLocal8Bit().data());
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
	    QVariant value = date.toUtf8();
	    qDebug("adding date tag: '%s'",
	           date.toLocal8Bit().data());
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
	QObject::connect(dialog, SIGNAL(canceled()),
	                 &src,   SLOT(cancel()));

	// invoke the encoder...
	bool encoded = false;

	if (selection) {
	    // we have to adjust all labels in the file info
	    // use a copy, don't touch the original !
	    FileInfo info = m_file_info;

	    LabelList &labels = info.labels();
	    foreach (Label *label, labels) {
		if (!label) continue;
		unsigned int pos = label->pos();
		if ((pos < ofs) || (pos >= ofs + len)) {
		    // out of the selected area -> remove
		    labels.removeAll(label);
		} else {
		    // move label left
		    label->moveTo(pos - ofs);
		}
	    }

	    // set the filename in the copy of the fileinfo, the original
	    // file which is currently open keeps it's name
	    info.set(INF_FILENAME, filename);
	    encoded = encoder->encode(m_parent_widget, src, dst, info);
	} else {
	    // in case of a "save as" -> modify the current filename
	    m_file_info.set(INF_FILENAME, filename);
	    encoded = encoder->encode(m_parent_widget, src, dst, m_file_info);
	}
	if (!encoded) {
	    Kwave::MessageBox::error(m_parent_widget,
	        i18n("An error occurred while saving the file!"));
	    res = -1;
	}

	delete encoder;
	if (dialog) {
	    if (dialog->isCanceled()) {
		// user really pressed cancel !
		Kwave::MessageBox::error(m_parent_widget,
		    i18n("The file has been truncated and "\
		         "might be corrupted!"));
	    }
	    delete dialog;
	}
    } else {
	Kwave::MessageBox::error(m_parent_widget,
	    i18n("Sorry, the file type is not supported!"));
	res = -EINVAL;
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
    while (tracks()) deleteTrack(tracks()-1);
    m_signal.close();
    m_file_info.clear();

    m_closed = true;
    m_selection.select(0,0);
    emitStatusInfo();
}

//***************************************************************************
const QList<unsigned int> SignalManager::selectedTracks()
{
    unsigned int track;
    QList<unsigned int> list;
    const unsigned int tracks = this->tracks();

    for (track=0; track < tracks; track++) {
	if (!m_signal.trackSelected(track)) continue;
	list.append(track);
    }

    return list;
}

//***************************************************************************
const QList<unsigned int> SignalManager::allTracks()
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
    UndoAction *undo = 0;
    switch (mode) {
	case Append:
	    qDebug("SignalManager::openSampleWriter(): "\
	           "NO UNDO FOR APPEND YET !");
	    abortUndoTransaction();
	    break;
	case Insert:
	    undo = new UndoInsertAction(track, left, right-left+1);
	    if (!registerUndoAction(undo)) undo = 0;
	    if (undo) {
		QObject::connect(
		    writer,
		    SIGNAL(sigSamplesWritten(unsigned int)),
		    reinterpret_cast<UndoInsertAction *>(undo),
		    SLOT(setLength(unsigned int)));
	    }
	    break;
	case Overwrite:
	    undo = new UndoModifyAction(track, left, right-left+1);
	    break;
    }

    if (!registerUndoAction(undo)) {
	// aborted, do not continue without undo
	delete writer;
	return 0;
    }

    // Everything was ok, the action now is owned by the current undo
    // transaction. The transaction is owned by the SignalManager and
    // will be closed when the writer gets closed.
    return writer;
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
	QList<unsigned int> tracks = selectedTracks();
	if (saveUndoDelete(tracks, offset+length, rest) &&
	    saveUndoDelete(tracks, 0, offset))
	{
	    unsigned int count = tracks.count();
	    while (count--) {
		m_signal.deleteRange(count, offset+length, rest);
		m_signal.deleteRange(count, 0, offset);
	    }
	    selectRange(0, length);
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
    QList<unsigned int> selected_tracks = selectedTracks();
    if (clipboard.isEmpty()) return;
    if (!selected_tracks.size()) return;

    UndoTransactionGuard u(*this, i18n("paste"));

    // delete the current selection (with undo)
    if (length <= 1) length = 0; // do not paste single samples !
    if (length && !deleteRange(offset, length))
	return;

    // if the signal has no tracks, create new ones
    if (!tracks()) {
	unsigned int missing = clipboard.tracks();
	qDebug("SignalManager::paste(): appending %u tracks", missing);
	while (missing--) appendTrack();
    }

    // open a stream into the signal
    MultiTrackWriter dst(*this, selectedTracks(), Insert,
                         offset, offset+clipboard.length()-1);
    if (static_cast<int>(dst.tracks()) != selectedTracks().count())
	return;

    // transfer the content
    clipboard.paste(dst);

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

    if (m_undo_enabled && !registerUndoAction(
	new UndoInsertTrack(m_signal, index))) return;

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

    if (m_undo_enabled && !registerUndoAction(
	new UndoDeleteTrack(m_signal, index))) return;

    setModified(true);
    m_signal.deleteTrack(index);
}

//***************************************************************************
void SignalManager::slotTrackInserted(unsigned int index,
	Track *track)
{
    setModified(true);
    emit sigTrackInserted(index, track);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotTrackDeleted(unsigned int index)
{
    setModified(true);
    emit sigTrackDeleted(index);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesInserted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);

    // only adjust the labels once per operation
    if (track == selectedTracks().at(0)) {
	foreach (Label *label, labels()) {
	    if (!label) continue;
	    unsigned int pos = label->pos();
	    if (pos >= offset) {
		label->moveTo(pos + length);
	    }
	}
    }

    emit sigSamplesInserted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesDeleted(unsigned int track,
	unsigned int offset, unsigned int length)
{
    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);

    // only adjust the labels once per operation
    if (track == selectedTracks().at(0)) {
	foreach (Label *label, labels()) {
	    if (!label) continue;
	    unsigned int pos = label->pos();
	    if (pos >= offset + length) {
		// move label left
		label->moveTo(pos - length);
	    } else if ((pos >= offset) && (pos < offset+length)) {
		// delete the label
		deleteLabel(labelIndex(label), true);
	    }
	}
    }

    emit sigSamplesDeleted(track, offset, length);
    emitStatusInfo();
}

//***************************************************************************
void SignalManager::slotSamplesModified(unsigned int track,
	unsigned int offset, unsigned int length)
{
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
                                const QList<unsigned int> &track_list)
{
    if (!length) return true; // nothing to do
    UndoTransactionGuard undo(*this, i18n("delete"));

    unsigned int count = track_list.count();
    if (!count) return true; // nothing to do

    // first store undo data for all tracks
    unsigned int track;
    if (m_undo_enabled) {
	foreach (track, track_list) {
	    if (!registerUndoAction(new UndoDeleteAction(
		track, offset, length))) return false;
	}
    }

    // then delete the ranges in all tracks
    foreach (track, track_list) {
	m_signal.deleteRange(track, offset, length);
    }

    // finally set the current selection to zero-length
    selectRange(m_selection.offset(), 0);

    return true;
}

//***************************************************************************
bool SignalManager::deleteRange(unsigned int offset, unsigned int length)
{
    return deleteRange(offset, length, selectedTracks());
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
void SignalManager::selectTracks(QList<unsigned int> &track_list)
{
    unsigned int track;
    unsigned int n_tracks = tracks();
    for (track = 0; track < n_tracks; track++) {
	bool old_select = m_signal.trackSelected(track);
	bool new_select = track_list.contains(track);
	if (new_select != old_select) {
	    m_signal.selectTrack(track, new_select);
	}
    }
}

//***************************************************************************
void SignalManager::selectTrack(unsigned int track, bool select)
{
    bool old_select = m_signal.trackSelected(track);
    if (select != old_select) {
	m_signal.selectTrack(track, select);

	// if selection changed during playback, reload with pause/continue
	if (m_playback_controller.running()) {
	    m_playback_controller.reload();
	}
    }
}

//***************************************************************************
void SignalManager::emitStatusInfo()
{
    emit sigStatusInfo(length(), tracks(), rate(), bits());
}

//***************************************************************************
PlaybackController &SignalManager::playbackController()
{
    return m_playback_controller;
}

//***************************************************************************
void SignalManager::startUndoTransaction(const QString &name)
{
    if (!m_undo_enabled) return; // undo is currently not enabled

    QMutexLocker lock(&m_undo_transaction_lock);

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
	    if (selection->store(*this)) {
		m_undo_transaction->append(selection);
	    } else {
		// out of memory
		delete selection;
		delete m_undo_transaction;
		m_undo_transaction = 0;
	    }
	}
    }
}

//***************************************************************************
void SignalManager::closeUndoTransaction()
{
    QMutexLocker lock(&m_undo_transaction_lock);

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
	emitUndoRedoInfo();
    }
}

//***************************************************************************
void SignalManager::enableUndo()
{
    m_undo_enabled = true;
    emitUndoRedoInfo();
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
    QMutexLocker lock(&m_undo_transaction_lock);

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
    qDeleteAll(m_undo_buffer);
    qDeleteAll(m_redo_buffer);
    m_undo_buffer.clear();
    m_redo_buffer.clear();

    emitUndoRedoInfo();
}

//***************************************************************************
void SignalManager::abortUndoTransaction()
{
    // close the current transaction
    if (m_undo_transaction) delete m_undo_transaction;
    m_undo_transaction = 0;
    m_undo_transaction_level = 0;
}

//***************************************************************************
void SignalManager::flushRedoBuffer()
{
    qDeleteAll(m_redo_buffer);
    m_redo_buffer.clear();
    emitUndoRedoInfo();
}

//***************************************************************************
bool SignalManager::continueWithoutUndo()
{
    if (!m_parent_widget) return false;

    if (Kwave::MessageBox::warningContinueCancel(m_parent_widget,
	i18n("Unable to save undo information. "
	     "Do you want to continue without undo?")) ==
	KMessageBox::Continue)
    {
	return true;
    }
    return false;
}

//***************************************************************************
bool SignalManager::registerUndoAction(UndoAction *action)
{
    QMutexLocker lock(&m_undo_transaction_lock);

    Q_ASSERT(action);
    if (!action) {
	abortUndoTransaction();
	return continueWithoutUndo();
    }

    // if undo is not enabled, this will fail -> no memory leak!
    Q_ASSERT(m_undo_enabled);
    if (!m_undo_enabled) {
	delete action;
	abortUndoTransaction();
	return continueWithoutUndo();
    }

    // check if the undo action is too large
    unsigned int needed_size  = action->undoSize();
    unsigned int needed_mb = needed_size  >> 20;
    unsigned int limit_mb  = m_undo_limit >> 20;
    if (needed_mb > limit_mb) {
	delete action;
	abortUndoTransaction();
	return continueWithoutUndo();
    }

    // undo has been aborted before ?
    if (!m_undo_transaction) return true;

    // make room...
    freeUndoMemory(needed_size);

    // now we might have enough place to append the undo action
    // and store all undo info
    if (!action->store(*this)) {
	delete action;
	abortUndoTransaction();
	return continueWithoutUndo();
    }

    // everything went ok, register internally
    m_undo_transaction->append(action);

    return true;
}

//***************************************************************************
bool SignalManager::saveUndoDelete(QList<unsigned int> &track_list,
                                   unsigned int offset, unsigned int length)
{
    if (!m_undo_enabled) return true;
    if (track_list.isEmpty()) return true;

    // loop over all tracks
    QListIterator<unsigned int> it(track_list);
    it.toBack();
    while (m_undo_enabled && it.hasPrevious()) {
	unsigned int t = it.previous();
	UndoDeleteAction *action = new UndoDeleteAction(t, offset, length);
	if (!registerUndoAction(action)) return false;
    }

    return true;
}

//***************************************************************************
unsigned int SignalManager::usedUndoRedoMemory()
{
    unsigned int size = 0;

    foreach (UndoTransaction *undo, m_undo_buffer)
	if (undo) size += undo->undoSize();

    foreach (UndoTransaction *redo, m_redo_buffer)
	if (redo) size += redo->undoSize();

    return size;
}

//***************************************************************************
void SignalManager::freeUndoMemory(unsigned int needed)
{
    unsigned int size = usedUndoRedoMemory() + needed;

    // remove old undo actions if not enough free memory
    while (!m_undo_buffer.isEmpty() && (size > m_undo_limit)) {
	UndoTransaction *undo = m_undo_buffer.takeFirst();
	if (!undo) continue;
	unsigned int s = undo->undoSize();
	size = (size >= s) ? (size - s) : 0;
	delete undo;

	// if the signal was modified, it will stay in this state, it is
	// not possible to change to "non-modified" state through undo
	if ((!m_undo_buffer.isEmpty()) && (m_modified)) {
	    enableModifiedChange(false);
	}
    }

    // remove old redo actions if still not enough memory
    while (!m_redo_buffer.isEmpty() && (size > m_undo_limit)) {
	UndoTransaction *redo = m_redo_buffer.takeLast();
	if (!redo) continue;
	unsigned int s = redo->undoSize();
	size = (size >= s) ? (size - s) : 0;
	delete redo;
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
    QMutexLocker lock(&m_undo_transaction_lock);

    // get the last undo transaction and abort if none present
    if (m_undo_buffer.isEmpty()) return;
    UndoTransaction *undo_transaction = m_undo_buffer.last();
    Q_ASSERT(undo_transaction);
    if (!undo_transaction) return;

    // remove the undo transaction from the list without deleting it
    m_undo_buffer.takeLast();

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
	qDebug("SignalManager::undo(): redo buffer flushed!");
    } else {
	m_redo_buffer.prepend(redo_transaction);
    }

    // execute all undo actions and store the resulting redo
    // actions into the redo transaction
    while (!undo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;

	// unqueue the undo action
	undo_action = undo_transaction->nextUndo();
	undo_transaction->removeAll(undo_action);
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
	m_redo_buffer.removeAll(redo_transaction);
	delete redo_transaction;
    }

    if (m_undo_buffer.isEmpty() && m_modified) {
	// try to return to non-modified mode (might be a nop if
	// not enabled)
	setModified(false);
    }

    // finished / buffers have changed, emit new undo/redo info
    emitUndoRedoInfo();
}

//***************************************************************************
void SignalManager::redo()
{
    QMutexLocker lock(&m_undo_transaction_lock);

    // get the last redo transaction and abort if none present
    if (m_redo_buffer.isEmpty()) return;
    UndoTransaction *redo_transaction = m_redo_buffer.first();
    if (!redo_transaction) return;

    // remove the redo transaction from the list without deleting it
    m_redo_buffer.takeFirst();

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
	qDeleteAll(m_undo_buffer);
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
	redo_action = redo_transaction->nextRedo();
	redo_transaction->removeAll(redo_action);

	// execute the redo operation
	Q_ASSERT(redo_action);
	if (!redo_action) continue;
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
	m_undo_buffer.removeAll(undo_transaction);
	delete undo_transaction;
    }

    // finished / buffers have changed, emit new undo/redo info
    emitUndoRedoInfo();
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
    if (m_undo_enabled && with_undo) {
	/* save data for undo */
	UndoTransactionGuard undo_transaction(*this, i18n("modify file info"));
	if (!registerUndoAction(new UndoFileInfo(*this))) return;
    }

    m_file_info = new_info;
    setModified(true);
    emitStatusInfo();
    emitUndoRedoInfo();
}

//***************************************************************************
Label *SignalManager::findLabel(unsigned int pos) const
{
    foreach (Label *label, labels()) {
	if (label && (label->pos() == pos)) return label; // found it
    }

    return 0; // nothing found
}

//***************************************************************************
int SignalManager::labelIndex(const Label *label) const
{
    int index = 0;
    foreach (Label *l, labels()) {
	if (l == label) return index; // found it
	index++;
    }
    return -1; // nothing found*/
}

//***************************************************************************
Label *SignalManager::labelAtIndex(int index)
{
    return labels().at(index);
}

//***************************************************************************
bool SignalManager::addLabel(unsigned int pos)
{
    // if there already is a label at the given position, do nothing
    if (findLabel(pos)) return false;

    // create a new label
    Label *label = new Label(pos, "");
    Q_ASSERT(label);
    if (!label) return false;

    // put the label into the list
    labels().append(label);
    labels().sort();
    emit sigLabelCountChanged();

    // register the undo action
    UndoTransactionGuard undo(*this, i18n("add label"));
    if (!registerUndoAction(new UndoAddLabelAction(labelIndex(label)))) {
	labels().removeAll(label);
	delete label;
	emit sigLabelCountChanged();
	return false;
    }
    return true;
}

//***************************************************************************
Label *SignalManager::addLabel(unsigned int pos, const QString &name)
{
    // if there already is a label at the given position, do nothing
    if (findLabel(pos)) return 0;

    // create a new label
    Label *label = new Label(pos, name);
    Q_ASSERT(label);
    if (!label) return 0;

    // put the label into the list
    labels().append(label);
    labels().sort();
    emit sigLabelCountChanged();

    return label;
}

//***************************************************************************
void SignalManager::deleteLabel(int index, bool with_undo)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < (int)labels().count());
    if ((index < 0) || (index >= (int)labels().count())) return;

    Label *label = labels().at(index);
    Q_ASSERT(label);
    if (!label) return;

    // register the undo action
    if (with_undo) {
	UndoTransactionGuard undo(*this, i18n("delete label"));
	if (!registerUndoAction(new UndoDeleteLabelAction(*label)))
	    return;
    }

    labels().removeAll(label);
    emit sigLabelCountChanged();
}

//***************************************************************************
#include "SignalManager.moc"
//***************************************************************************
//***************************************************************************
