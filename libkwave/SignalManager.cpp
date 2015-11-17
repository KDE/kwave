/***************************************************************************
       SignalManager.cpp -  manager class for multi channel signals
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
#include <new>

#include <QApplication>
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QMutableListIterator>
#include <QMutexLocker>
#include <QUrl>

#include <KAboutData>
#include <KLocalizedString>
#include <kxmlgui_version.h>

#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/Decoder.h"
#include "libkwave/Encoder.h"
#include "libkwave/FileProgress.h"
#include "libkwave/InsertMode.h"
#include "libkwave/LabelList.h"
#include "libkwave/MemoryManager.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/MultiTrackWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/Signal.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Track.h"
#include "libkwave/Utils.h"
#include "libkwave/Writer.h"
#include "libkwave/undo/UndoAction.h"
#include "libkwave/undo/UndoAddMetaDataAction.h"
#include "libkwave/undo/UndoDeleteAction.h"
#include "libkwave/undo/UndoDeleteMetaDataAction.h"
#include "libkwave/undo/UndoDeleteTrack.h"
#include "libkwave/undo/UndoInsertAction.h"
#include "libkwave/undo/UndoInsertTrack.h"
#include "libkwave/undo/UndoModifyAction.h"
#include "libkwave/undo/UndoModifyMetaDataAction.h"
#include "libkwave/undo/UndoSelection.h"
#include "libkwave/undo/UndoTransaction.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#define CASE_COMMAND(x) } else if (parser.command() == _(x)) {

//***************************************************************************
Kwave::SignalManager::SignalManager(QWidget *parent)
    :QObject(),
    m_parent_widget(parent),
    m_closed(true),
    m_empty(true),
    m_modified(false),
    m_modified_enabled(true),
    m_signal(),
    m_selection(0,0),
    m_last_selection(0,0),
    m_last_track_selection(),
    m_last_length(0),
    m_playback_controller(*this),
    m_undo_enabled(false),
    m_undo_buffer(),
    m_redo_buffer(),
    m_undo_transaction(0),
    m_undo_transaction_level(0),
    m_undo_transaction_lock(QMutex::Recursive),
    m_meta_data()
{
    // connect to the track's signals
    Kwave::Signal *sig = &m_signal;
    connect(sig, SIGNAL(sigTrackInserted(uint,Kwave::Track*)),
            this, SLOT(slotTrackInserted(uint,Kwave::Track*)));
    connect(sig, SIGNAL(sigTrackDeleted(uint,Kwave::Track*)),
            this, SLOT(slotTrackDeleted(uint,Kwave::Track*)));
    connect(sig, SIGNAL(sigTrackSelectionChanged(bool)),
            this,SIGNAL(sigTrackSelectionChanged(bool)));
    connect(sig, SIGNAL(sigSamplesDeleted(unsigned int, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesDeleted(unsigned int, sample_index_t,
            sample_index_t)));
    connect(sig, SIGNAL(sigSamplesInserted(unsigned int, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesInserted(unsigned int, sample_index_t,
            sample_index_t)));
    connect(sig, SIGNAL(sigSamplesModified(unsigned int, sample_index_t,
            sample_index_t)),
            this, SLOT(slotSamplesModified(unsigned int, sample_index_t,
            sample_index_t)));
}

//***************************************************************************
Kwave::SignalManager::~SignalManager()
{
    close();
}

//***************************************************************************
int Kwave::SignalManager::loadFile(const QUrl &url)
{
    int res = 0;
    Kwave::FileProgress *dialog = 0;

    // take over the new file name, so that we have a valid signal
    // name during loading
    QString filename = url.path();
    QFile src(filename);
    QFileInfo fi(src);
    {
	Kwave::FileInfo info(m_meta_data);
	info.set(Kwave::INF_FILENAME, fi.absoluteFilePath());
	m_meta_data.replace(Kwave::MetaDataList(info));
    }

    // work with a copy of meta data, to avoid flicker effects
    Kwave::MetaDataList meta_data(m_meta_data);

    // enter and stay in not modified state
    enableModifiedChange(true);
    setModified(false);
    enableModifiedChange(false);

    // disable undo (discards all undo/redo data)
    disableUndo();

    QString mimetype = Kwave::CodecManager::whatContains(url);
    qDebug("SignalManager::loadFile(%s) - [%s]",
           DBG(url.toDisplayString()), DBG(mimetype));
    Kwave::Decoder *decoder = Kwave::CodecManager::decoder(mimetype);
    while (decoder) {
	// be sure that the current signal is really closed
	m_signal.close();

	// open the source file
	if (!(res = decoder->open(m_parent_widget, src))) {
	    qWarning("unable to open source: '%s'", DBG(url.toDisplayString()));
	    res = -EIO;
	    break;
	}

	// get the initial meta data from the decoder
	meta_data = decoder->metaData();
	Kwave::FileInfo info(meta_data);

	// take the preliminary meta data, needed for estimated length
	m_meta_data = meta_data;

	// detect stream mode. if so, use one sample as display
	bool streaming = (!info.length());

	// we must change to open state to see the file while
	// it is loaded
	m_closed = false;
	m_empty = false;

	// create all tracks (empty)
	unsigned int track;
	const unsigned int tracks   = info.tracks();
	const sample_index_t length = info.length();
	Q_ASSERT(tracks);
	if (!tracks) break;

	for (track = 0; track < tracks; ++track) {
	    Kwave::Track *t = m_signal.appendTrack(length, 0);
	    Q_ASSERT(t);
	    if (!t || (t->length() != length)) {
		qWarning("SignalManager::loadFile: out of memory");
		res = -ENOMEM;
		break;
	    }
	}
	if (track < tracks) break;

	// create the multitrack writer as destination
	// if length was zero -> append mode / decode a stream ?
	Kwave::InsertMode mode = (streaming) ? Kwave::Append : Kwave::Overwrite;
	Kwave::MultiTrackWriter writers(*this, allTracks(), mode, 0,
	    (length) ? length-1 : 0);

	// try to calculate the resulting length, but if this is
	// not possible, we try to use the source length instead
	quint64 resulting_size = info.tracks() * info.length() *
	                              (info.bits() >> 3);
	bool use_src_size = (!resulting_size);
	if (use_src_size) resulting_size = src.size();

	//prepare and show the progress dialog
	dialog = new Kwave::FileProgress(m_parent_widget,
	    QUrl(filename), resulting_size,
	    info.length(), info.rate(), info.bits(), info.tracks());
	Q_ASSERT(dialog);

	if (dialog && use_src_size) {
	    // use source size for progress / stream mode
	    QObject::connect(decoder, SIGNAL(sourceProcessed(quint64)),
	                     dialog,  SLOT(setBytePosition(quint64)));
	    QObject::connect(&writers, SIGNAL(written(quint64)),
	                     dialog,   SLOT(setLength(quint64)));
	} else {
	    // use resulting size percentage for progress
	    QObject::connect(&writers, SIGNAL(progress(qreal)),
	                     dialog,   SLOT(setValue(qreal)));
	}
	QObject::connect(dialog,   SIGNAL(canceled()),
	                 &writers, SLOT(cancel()));

	// now decode
	res = 0;
	if (!decoder->decode(m_parent_widget, writers)) {
	    qWarning("decoding failed.");
	    res = -EIO;
	} else {
	    // read information back from the decoder, some settings
	    // might have become available during the decoding process
	    meta_data = decoder->metaData();
	    info = Kwave::FileInfo(meta_data);
	}

	decoder->close();

	// check for length info in stream mode
	if (!res && streaming) {
	    // source was opened in stream mode -> now we have the length
	    writers.flush();
	    sample_index_t new_length = writers.last();
	    if (new_length) new_length++;
	    info.setLength(new_length);
	} else {
	    info.setLength(this->length());
	    info.setTracks(tracks);
	}

	// enter the filename/mimetype and size into the file info
	info.set(Kwave::INF_FILENAME, fi.absoluteFilePath());
	info.set(Kwave::INF_FILESIZE, src.size());
	if (!info.contains(Kwave::INF_MIMETYPE))
	    info.set(Kwave::INF_MIMETYPE, mimetype);

	// remove the estimated length again, it is no longer needed
	info.set(Kwave::INF_ESTIMATED_LENGTH, QVariant());

	// take over the decoded and updated file info
	meta_data.replace(Kwave::MetaDataList(info));
	m_meta_data = meta_data;

	// update the length info in the progress dialog if needed
	if (dialog && use_src_size) {
	    dialog->setLength(
		quint64(info.length()) *
		quint64(info.tracks()));
	    dialog->setBytePosition(src.size());
	}

	break;
    }

    if (!decoder) {
	qWarning("unknown file type");
	res = -EINVAL;
    } else {
	delete decoder;
    }

    // process any queued events of the writers, like "sigSamplesInserted"
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    // remember the last length and selection
    m_last_length = length();
    rememberCurrentSelection();

    // from now on, undo is enabled
    enableUndo();

    // modified can change from now on
    enableModifiedChange(true);

    if (dialog) delete dialog;
    if (res) close();

    m_meta_data.dump();

    // we now have new meta data
    emit sigMetaDataChanged(m_meta_data);

    return res;
}

//***************************************************************************
int Kwave::SignalManager::save(const QUrl &url, bool selection)
{
    int res = 0;
    sample_index_t ofs  = 0;
    sample_index_t len  = length();
    unsigned int tracks = this->tracks();
    unsigned int bits   = this->bits();

    if (selection) {
	// zero-length -> nothing to do
	ofs = m_selection.offset();
	len = m_selection.length();
	tracks = selectedTracks().count();
    }

    if (!tracks || !len) {
	Kwave::MessageBox::error(m_parent_widget,
	    i18n("Signal is empty, nothing to save."));
	return 0;
    }

    QString mimetype_name;
    mimetype_name = Kwave::CodecManager::whatContains(url);
    qDebug("SignalManager::save(%s) - [%s] (%d bit, selection=%d)",
	DBG(url.toDisplayString()), DBG(mimetype_name), bits, selection);

    Kwave::Encoder *encoder = Kwave::CodecManager::encoder(mimetype_name);
    Kwave::FileInfo file_info(m_meta_data);
    if (encoder) {

	// maybe we now have a new mime type
	file_info.set(Kwave::INF_MIMETYPE, mimetype_name);

	// check if we lose information and ask the user if this would
	// be acceptable if so
	QList<Kwave::FileProperty> supported = encoder->supportedProperties();
	QMap<Kwave::FileProperty, QVariant> properties(file_info.properties());
	bool all_supported = true;
	QMap<Kwave::FileProperty, QVariant>::Iterator it;
	QString lost_properties;
	for (it = properties.begin(); it != properties.end(); ++it) {
	    if ( (! supported.contains(it.key())) &&
                 (file_info.canLoadSave(it.key())) )
	    {
		qWarning("SignalManager::save(): unsupported property '%s'",
		    DBG(file_info.name(it.key())));
		all_supported = false;
		lost_properties += i18n(UTF8(file_info.name(it.key()))) + _("\n");
	    }
	}
	if (!all_supported) {
	    // show a warning to the user and ask him if he wants to continue
	    if (Kwave::MessageBox::warningContinueCancel(m_parent_widget,
		i18n("Saving in this format will lose the following "
		     "additional file attribute(s):\n"
		     "%1\n"
		     "Do you still want to continue?",
		     lost_properties),
		QString(),
		QString(),
		QString(),
		_("accept_lose_attributes_on_export")
		) != KMessageBox::Continue)
	    {
		delete encoder;
		return -1;
	    }
	}

	// open the destination file
	QString filename = url.path();
	QFile dst(filename);

	Kwave::MultiTrackReader src(Kwave::SinglePassForward, *this,
	    (selection) ? selectedTracks() : allTracks(),
	    ofs, ofs + len - 1);

	// update the file information
	file_info.setLength(len);
	file_info.setRate(rate());
	file_info.setBits(bits);
	file_info.setTracks(tracks);

	if (!file_info.contains(Kwave::INF_SOFTWARE) &&
	    encoder->supportedProperties().contains(Kwave::INF_SOFTWARE))
	{
	    // add our Kwave Software tag
	    const KAboutData about_data = KAboutData::applicationData();
	    QString software = about_data.displayName() + _("-") +
	                       about_data.version() +
	                       i18n(" for KDE ") +
			       _(KXMLGUI_VERSION_STRING);
	    qDebug("adding software tag: '%s'", DBG(software));
	    file_info.set(Kwave::INF_SOFTWARE, software);
	}

	if (!file_info.contains(Kwave::INF_CREATION_DATE) &&
	    encoder->supportedProperties().contains(Kwave::INF_CREATION_DATE))
	{
	    // add a date tag
	    QDate now(QDate::currentDate());
	    QString date;
	    date = date.sprintf("%04d-%02d-%02d",
		now.year(), now.month(), now.day());
	    qDebug("adding date tag: '%s'", DBG(date));
	    file_info.set(Kwave::INF_CREATION_DATE, date);
	}

	// prepare and show the progress dialog
	Kwave::FileProgress *dialog = new Kwave::FileProgress(m_parent_widget,
	    QUrl(filename), file_info.length() * file_info.tracks() *
	    (file_info.bits() >> 3),
	    file_info.length(), file_info.rate(), file_info.bits(),
	    file_info.tracks());
	Q_ASSERT(dialog);
	QObject::connect(&src,   SIGNAL(progress(qreal)),
	                 dialog, SLOT(setValue(qreal)),
	                 Qt::QueuedConnection);
	QObject::connect(dialog, SIGNAL(canceled()),
	                 &src,   SLOT(cancel()));

	// invoke the encoder...
	bool encoded = false;
	m_meta_data.replace(Kwave::MetaDataList(file_info));

	if (selection) {
	    // use a copy, don't touch the original !
	    Kwave::MetaDataList meta = m_meta_data;

	    // we have to adjust all position aware meta data
	    meta.cropByRange(ofs, ofs + len - 1);

	    // filter out all the track bound meta data that is not selected
	    meta.cropByTracks(selectedTracks());

	    // set the filename in the copy of the fileinfo, the original
	    // file which is currently open keeps it's name
	    Kwave::FileInfo info(meta);
	    info.set(Kwave::INF_FILENAME, filename);
	    meta.replace(Kwave::MetaDataList(info));

	    encoded = encoder->encode(m_parent_widget, src, dst, meta);
	} else {
	    // in case of a "save as" -> modify the current filename
	    file_info.set(Kwave::INF_FILENAME, filename);
	    m_meta_data.replace(Kwave::MetaDataList(file_info));
	    encoded = encoder->encode(m_parent_widget, src, dst, m_meta_data);
	}
	if (!encoded) {
	    Kwave::MessageBox::error(m_parent_widget,
	        i18n("An error occurred while saving the file."));
	    res = -1;
	}

	delete encoder;
	encoder = 0;

	if (dialog) {
	    qApp->processEvents();
	    if (dialog->isCanceled()) {
		// user really pressed cancel !
		Kwave::MessageBox::error(m_parent_widget,
		    i18n("The file has been truncated and "
		          "might be corrupted."));
		res = -EINTR;
	    }
	    delete dialog;
	    dialog = 0;
	}
    } else {
	Kwave::MessageBox::error(m_parent_widget,
	    i18n("Sorry, the file type is not supported."));
	res = -EINVAL;
    }

    if (!res) {
	// saved without error -> no longer modified
	flushUndoBuffers();
	enableModifiedChange(true);
	setModified(false);
    }

    emit sigMetaDataChanged(m_meta_data);
    qDebug("SignalManager::save(): res=%d",res);
    return res;
}

//***************************************************************************
void Kwave::SignalManager::newSignal(sample_index_t samples, double rate,
                                     unsigned int bits, unsigned int tracks)
{
    // enter and stay in modified state
    enableModifiedChange(true);
    setModified(true);
    enableModifiedChange(false);

    // disable undo (discards all undo/redo data)
    disableUndo();

    m_meta_data.clear();
    Kwave::FileInfo file_info(m_meta_data);
    file_info.setRate(rate);
    file_info.setBits(bits);
    file_info.setTracks(tracks);
    m_meta_data.replace(Kwave::MetaDataList(file_info));

    // now the signal is considered not to be empty
    m_closed = false;
    m_empty = false;

    // add all empty tracks
    while (tracks) {
	m_signal.appendTrack(samples, 0);
	tracks--;
    }

    // remember the last length
    m_last_length = samples;
    file_info.setLength(length());
    m_meta_data.replace(Kwave::MetaDataList(file_info));
    rememberCurrentSelection();

    // from now on, undo is enabled
    enableUndo();

    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
void Kwave::SignalManager::close()
{
    // stop the playback
    m_playback_controller.playbackStop();
    m_playback_controller.reset();

    // fix the modified flag to false
    enableModifiedChange(true);
    setModified(false);
    enableModifiedChange(false);

    // reset the last length of the signal
    m_last_length = 0;

    // disable undo and discard all undo buffers
    // undo will be re-enabled when a signal is loaded or created
    disableUndo();

    // for safety: flush all undo/redo buffers
    flushUndoBuffers();
    flushRedoBuffer();

    // reset the selection
    m_selection.clear();

    m_empty = true;
    while (tracks()) deleteTrack(tracks()-1);
    m_signal.close();

    // clear all meta data
    m_meta_data.clear();

    m_closed = true;
    rememberCurrentSelection();

    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
QString Kwave::SignalManager::signalName()
{
    // if a file is loaded -> path of the URL if it has one
    QUrl url;
    url = QUrl(Kwave::FileInfo(m_meta_data).get(Kwave::INF_FILENAME).toString());
    if (url.isValid()) return url.path();

    // we have something, but no name yet
    if (!isClosed()) return QString(NEW_FILENAME);

    // otherwise: closed, nothing loaded
    return _("");
}

//***************************************************************************
const QList<unsigned int> Kwave::SignalManager::selectedTracks()
{
    QList<unsigned int> list;
    const unsigned int tracks = this->tracks();

    for (unsigned int track = 0; track < tracks; track++) {
	if (!m_signal.trackSelected(track)) continue;
	list.append(track);
    }

    return list;
}

//***************************************************************************
const QList<unsigned int> Kwave::SignalManager::allTracks()
{
    return m_signal.allTracks();
}

//***************************************************************************
int Kwave::SignalManager::executeCommand(const QString &command)
{
    sample_index_t offset = m_selection.offset();
    sample_index_t length = m_selection.length();

    if (!command.length()) return -EINVAL;
    Kwave::Parser parser(command);

    if (false) {
    // --- undo / redo ---
    CASE_COMMAND("undo")
	undo();
    CASE_COMMAND("redo")
	redo();
    CASE_COMMAND("undo_all")
	while (m_undo_enabled && !m_undo_buffer.isEmpty())
	    undo();
    CASE_COMMAND("redo_all")
	while (!m_redo_buffer.isEmpty())
	    redo();

    // --- copy & paste + clipboard ---
    CASE_COMMAND("copy")
	if (length) {
	    Kwave::ClipBoard &clip = Kwave::ClipBoard::instance();
	    clip.copy(
		m_parent_widget,
		*this,
		selectedTracks(),
		offset, length
	    );
	    // remember the last selection
	    rememberCurrentSelection();
	}
    CASE_COMMAND("insert_at")
	Kwave::ClipBoard &clip = Kwave::ClipBoard::instance();
	if (clip.isEmpty()) return 0;
	if (!selectedTracks().size()) return 0;
	sample_index_t ofs = parser.toSampleIndex();

	Kwave::UndoTransactionGuard undo(*this,
	                                 i18n("Insert Clipboard at position"));

	selectRange(ofs, 0);
	clip.paste(m_parent_widget, *this, ofs, 0);

    CASE_COMMAND("paste")
	Kwave::ClipBoard &clip = Kwave::ClipBoard::instance();
	if (clip.isEmpty()) return 0;
	if (!selectedTracks().size()) return 0;

	Kwave::UndoTransactionGuard undo(*this, i18n("Paste"));
	clip.paste(m_parent_widget, *this, offset, length);
    CASE_COMMAND("cut")
	if (length) {
	    // remember the last selection
	    rememberCurrentSelection();

	    Kwave::ClipBoard &clip = Kwave::ClipBoard::instance();
	    clip.copy(
		m_parent_widget,
		*this,
		selectedTracks(),
		offset, length
	    );
	    Kwave::UndoTransactionGuard undo(*this, i18n("Cut"));
	    deleteRange(offset, length);
	    selectRange(m_selection.offset(), 0);
	}
    CASE_COMMAND("clipboard_flush")
	Kwave::ClipBoard::instance().clear();
    CASE_COMMAND("crop")
	if (length) {
	    Kwave::UndoTransactionGuard undo(*this, i18n("Crop"));
	    sample_index_t rest = this->length() - offset;
	    rest = (rest > length) ? (rest-length) : 0;
	    QList<unsigned int> tracks = selectedTracks();
	    if (saveUndoDelete(tracks, offset+length, rest) &&
		saveUndoDelete(tracks, 0, offset))
	    {
		// remember the last selection
		rememberCurrentSelection();

		unsigned int count = tracks.count();
		while (count--) {
		    m_signal.deleteRange(count, offset+length, rest);
		    m_signal.deleteRange(count, 0, offset);
		}
		selectRange(0, length);
	    }
	}
    CASE_COMMAND("delete")
	Kwave::UndoTransactionGuard undo(*this, i18n("Delete"));
	deleteRange(offset, length);
	selectRange(m_selection.offset(), 0);

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

    CASE_COMMAND("delete_label")
	int index = parser.toInt();
	deleteLabel(index, true);

    CASE_COMMAND("expandtolabel")
	Kwave::UndoTransactionGuard undo(*this,
	                                 i18n("Expand Selection to Label"));
	sample_index_t selection_left  = m_selection.first();
	sample_index_t selection_right = m_selection.last();
	Kwave::LabelList labels(m_meta_data);
	if (labels.isEmpty()) return false; // we need labels for this
	Kwave::Label label_left  = Kwave::Label();
	Kwave::Label label_right = Kwave::Label();

	// the last label <= selection start -> label_left
	// the first label >= selection end  -> label_right
	foreach (const Kwave::Label &label, labels) {
	    sample_index_t lp = label.pos();
	    if (lp <= selection_left)
		label_left = label;
	    if ((lp >= selection_right) && (label_right.isNull())) {
		label_right = label;
		break; // done
	    }
	}
	// default left label = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default right label = end of file
	selection_right = (label_right.isNull()) ?
	    (this->length() - 1) : label_right.pos();
	sample_index_t len = selection_right - selection_left + 1;
	selectRange(selection_left, len);

    CASE_COMMAND("selectnextlabels")
	Kwave::UndoTransactionGuard undo(*this, i18n("Select Next Labels"));
	sample_index_t selection_left;
	sample_index_t selection_right = m_selection.last();
	Kwave::Label label_left  = Kwave::Label();
	Kwave::Label label_right = Kwave::Label();
	Kwave::LabelList labels(m_meta_data);
	if (labels.isEmpty()) return false; // we need labels for this

	// special case: nothing selected -> select up to the first label
	if (selection_right == 0) {
	    label_right = labels.first();
	    selection_left = 0;
	} else {
	    // find the first label starting after the current selection
	    LabelListIterator it(labels);
	    while (it.hasNext()) {
		Kwave::Label label = it.next();
		if (label.pos() >= selection_right) {
		    // take it as selection start
		    label_left  = label;
		    // and it's next one as selection end (might be null)
		    label_right = it.hasNext() ? it.next() : Kwave::Label();
		    break;
		}
	    }
	    // default selection start = last label
	    if (label_left.isNull()) label_left = labels.last();
	    if (label_left.isNull()) return false; // no labels at all !?
	    selection_left = label_left.pos();
	}
	// default selection end = end of the file
	selection_right = (label_right.isNull()) ?
	    (this->length() - 1) : label_right.pos();
	sample_index_t len = (selection_right > selection_left) ?
	    (selection_right - selection_left + 1) : 1;
	selectRange(selection_left, len);

    CASE_COMMAND("selectprevlabels")
	Kwave::UndoTransactionGuard undo(*this, i18n("Select Previous Labels"));
	sample_index_t selection_left  = selection().first();
	sample_index_t selection_right = selection().last();
	Kwave::Label label_left  = Kwave::Label();
	Kwave::Label label_right = Kwave::Label();
	Kwave::LabelList labels(m_meta_data);
	if (labels.isEmpty()) return false; // we need labels for this

	// find the last label before the start of the selection
	foreach (const Kwave::Label &label, labels) {
	    if (label.pos() > selection_left)
		break; // done
	    label_left  = label_right;
	    label_right = label;
	}
	// default selection start = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default selection end = first label
	if (label_right.isNull()) label_right = labels.first();
	if (label_right.isNull()) return false; // no labels at all !?
	selection_right = label_right.pos();
	sample_index_t len = selection_right - selection_left + 1;
	selectRange(selection_left, len);

    // --- track related functions ---
    CASE_COMMAND("add_track")
	appendTrack();
    CASE_COMMAND("delete_track")
	Kwave::Parser p(command);
	unsigned int track = p.toUInt();
	if (track >= tracks()) return -EINVAL;
	deleteTrack(track);
    CASE_COMMAND("insert_track")
	Kwave::Parser p(command);
	unsigned int track = p.toUInt();
	insertTrack(track);

    // track selection
    CASE_COMMAND("select_track:all")
	Kwave::UndoTransactionGuard undo(*this, i18n("Select All Tracks"));
	foreach (unsigned int track, allTracks())
	    selectTrack(track, true);
    CASE_COMMAND("select_track:none")
	Kwave::UndoTransactionGuard undo(*this, i18n("Deselect all tracks"));
	foreach (unsigned int track, allTracks())
	    selectTrack(track, false);
    CASE_COMMAND("select_track:invert")
	Kwave::UndoTransactionGuard undo(*this, i18n("Invert Track Selection"));
	foreach (unsigned int track, allTracks())
	    selectTrack(track, !trackSelected(track));
    CASE_COMMAND("select_track:on")
	unsigned int track = parser.toUInt();
	if (track >= tracks()) return -EINVAL;
	Kwave::UndoTransactionGuard undo(*this, i18n("Select Track"));
	selectTrack(track, true);
    CASE_COMMAND("select_track:off")
	unsigned int track = parser.toUInt();
	if (track >= tracks()) return -EINVAL;
	Kwave::UndoTransactionGuard undo(*this, i18n("Deselect Track"));
	selectTrack(track, false);
    CASE_COMMAND("select_track:toggle")
	unsigned int track = parser.toUInt();
	if (track >= tracks()) return -EINVAL;
	Kwave::UndoTransactionGuard undo(*this, i18n("Toggle Track Selection"));
	selectTrack(track, !(trackSelected(track)));

    // playback control
    CASE_COMMAND("playback_start")
	m_playback_controller.playbackStart();

    CASE_COMMAND("fileinfo")
	QString property = parser.firstParam();
	QString value    = parser.nextParam();
	Kwave::FileInfo info(m_meta_data);
	bool found = false;
	foreach (Kwave::FileProperty p, info.allKnownProperties()) {
	    if (info.name(p) == property) {
		if (value.length())
		    info.set(p, QVariant(value)); // add/modify
		else
		    info.set(p, QVariant());      // delete
		found = true;
		break;
	    }
	}

	if (found) {
	    m_meta_data.replace(Kwave::MetaDataList(info));
	    // we now have new meta data
	    emit sigMetaDataChanged(m_meta_data);
	} else
	    return -EINVAL;
    CASE_COMMAND("dump_metadata")
	qDebug("DUMP OF META DATA => %s", DBG(parser.firstParam()));
	m_meta_data.dump();
    } else {
	return -ENOSYS;
    }

    return 0;
}

//***************************************************************************
void Kwave::SignalManager::appendTrack()
{
    Kwave::UndoTransactionGuard undo(*this, i18n("Append Track"));
    insertTrack(tracks());
}

//***************************************************************************
void Kwave::SignalManager::insertTrack(unsigned int index)
{
    Kwave::UndoTransactionGuard undo(*this, i18n("Insert Track"));

    const unsigned int count = tracks();
    Q_ASSERT(index <= count);
    if (index > count) index = count;

    // undo action for the track insert
    if (m_undo_enabled && !registerUndoAction(
	new(std::nothrow) Kwave::UndoInsertTrack(m_signal, index))) return;

    // if the signal is currently empty, use the last
    // known length instead of the current one
    sample_index_t len = (count) ? length() : m_last_length;

    if (index >= count) {
	// do an "append"
	m_signal.appendTrack(len, 0);
    } else {
	if (m_undo_enabled) {
	    // undo action for the corresponding meta data change
	    QList<unsigned int> tracks;
	    for (unsigned int t = index; t < count; t++) tracks.append(t);
	    Kwave::MetaDataList list = m_meta_data.selectByTracks(tracks);
	    if (!list.isEmpty() && !registerUndoAction(
		new(std::nothrow) Kwave::UndoModifyMetaDataAction(list)))
		    return;
	}

	// adjust the track bound meta data
	m_meta_data.insertTrack(index);

	// insert into the list
	m_signal.insertTrack(index, len, 0);
    }

    // remember the last length
    m_last_length = length();
}

//***************************************************************************
void Kwave::SignalManager::deleteTrack(unsigned int index)
{
    Kwave::UndoTransactionGuard undo(*this, i18n("Delete Track"));

    const unsigned int count = tracks();
    Q_ASSERT(index <= count);
    if (index > count) return;

    if (m_undo_enabled) {
	// undo action for the track deletion
	if (!registerUndoAction(new(std::nothrow)
	    UndoDeleteTrack(m_signal, index)))
		return;

	// undo action for the corresponding meta data change
	QList<unsigned int> tracks;
	for (unsigned int t = index; t < count; t++) tracks.append(t);
	Kwave::MetaDataList list = m_meta_data.selectByTracks(tracks);
	if (!list.isEmpty() && !registerUndoAction(
	    new(std::nothrow) Kwave::UndoModifyMetaDataAction(list)))
		return;
    }

    // adjust the track bound meta data
    m_meta_data.deleteTrack(index);

    setModified(true);
    m_signal.deleteTrack(index);
}

//***************************************************************************
void Kwave::SignalManager::slotTrackInserted(unsigned int index,
                                             Kwave::Track *track)
{
    setModified(true);

    Kwave::FileInfo file_info(m_meta_data);
    file_info.setTracks(tracks());
    m_meta_data.replace(Kwave::MetaDataList(file_info));

    emit sigTrackInserted(index, track);
    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
void Kwave::SignalManager::slotTrackDeleted(unsigned int index,
                                             Kwave::Track *track)
{
    setModified(true);

    Kwave::FileInfo file_info(m_meta_data);
    file_info.setTracks(tracks());
    m_meta_data.replace(Kwave::MetaDataList(file_info));

    emit sigTrackDeleted(index, track);
    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
void Kwave::SignalManager::slotSamplesInserted(unsigned int track,
	sample_index_t offset, sample_index_t length)
{
    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);

    // only adjust the meta data once per operation
    QList<unsigned int> tracks = selectedTracks();
    if (track == tracks[0]) {
	m_meta_data.shiftRight(offset, length, tracks);
    }

    emit sigSamplesInserted(track, offset, length);

    Kwave::FileInfo info(m_meta_data);
    info.setLength(m_last_length);
    m_meta_data.replace(Kwave::MetaDataList(info));
    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
void Kwave::SignalManager::slotSamplesDeleted(unsigned int track,
	sample_index_t offset, sample_index_t length)
{
    // remember the last known length
    m_last_length = m_signal.length();

    setModified(true);

    // only adjust the meta data once per operation
    QList<unsigned int> tracks = selectedTracks();
    if (track == tracks[0]) {
	m_meta_data.shiftLeft(offset, length, tracks);
    }

    emit sigSamplesDeleted(track, offset, length);

    Kwave::FileInfo info(m_meta_data);
    info.setLength(m_last_length);
    m_meta_data.replace(Kwave::MetaDataList(info));
    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
void Kwave::SignalManager::slotSamplesModified(unsigned int track,
	sample_index_t offset, sample_index_t length)
{
    setModified(true);
    emit sigSamplesModified(track, offset, length);
}

//***************************************************************************
bool Kwave::SignalManager::deleteRange(sample_index_t offset,
    sample_index_t length, const QList<unsigned int> &track_list)
{
    if (!length || track_list.isEmpty()) return true; // nothing to do

    // put the selected meta data into a undo action
    if (m_undo_enabled) {
	if (!registerUndoAction(new(std::nothrow) UndoDeleteMetaDataAction(
	    m_meta_data.copy(offset, length, track_list))))
	{
	    abortUndoTransaction();
	    return false;
	}
	m_meta_data.deleteRange(offset, length, track_list);

	// store undo data for all audio data (without meta data)
	if (!registerUndoAction(new(std::nothrow) UndoDeleteAction(
	    m_parent_widget, track_list, offset, length)))
	{
	    abortUndoTransaction();
	    return false;
	}
    } else {
	// delete without undo
	m_meta_data.deleteRange(offset, length, track_list);
    }

    // delete the ranges in all tracks
    // (this makes all metadata positions after the selected range invalid)
    foreach (unsigned int track, track_list) {
	m_signal.deleteRange(track, offset, length);
    }

    emit sigMetaDataChanged(m_meta_data);

    return true;
}

//***************************************************************************
bool Kwave::SignalManager::deleteRange(sample_index_t offset,
                                       sample_index_t length)
{
    return deleteRange(offset, length, selectedTracks());
}

//***************************************************************************
bool Kwave::SignalManager::insertSpace(sample_index_t offset,
                                       sample_index_t length,
                                       const QList<unsigned int> &track_list)
{
    if (!length) return true; // nothing to do
    Kwave::UndoTransactionGuard undo(*this, i18n("Insert Space"));

    unsigned int count = track_list.count();
    if (!count) return true; // nothing to do

    // first store undo data for all tracks
    unsigned int track;
    if (m_undo_enabled) {
	if (!registerUndoAction(new(std::nothrow) Kwave::UndoInsertAction(
	    m_parent_widget, track_list, offset, length))) return false;
    }

    // then insert space into all tracks
    foreach (track, track_list) {
	m_signal.insertSpace(track, offset, length);
    }

    return true;
}

//***************************************************************************
void Kwave::SignalManager::selectRange(sample_index_t offset,
                                       sample_index_t length)
{
    // first do some range checking
    sample_index_t len = this->length();

    if (offset >= len) offset = len ? (len - 1) : 0;
    if ((offset + length) > len) length = len - offset;

    m_selection.select(offset, length);
}

//***************************************************************************
void Kwave::SignalManager::selectTracks(QList<unsigned int> &track_list)
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
void Kwave::SignalManager::selectTrack(unsigned int track, bool select)
{
    bool old_select = m_signal.trackSelected(track);
    if (select != old_select) {
	m_signal.selectTrack(track, select);
    }
}

//***************************************************************************
QList<Kwave::Stripe::List> Kwave::SignalManager::stripes(
    const QList<unsigned int> &track_list,
    sample_index_t left, sample_index_t right)
{
    QList<Kwave::Stripe::List> stripes;

    foreach (unsigned int track, track_list) {
	Kwave::Stripe::List s = m_signal.stripes(track, left, right);
	if (s.isEmpty()) {
	    stripes.clear(); // something went wrong -> abort
	    break;
	}
	stripes.append(s);
    }

    return stripes;
}

//***************************************************************************
bool Kwave::SignalManager::mergeStripes(
    const QList<Kwave::Stripe::List> &stripes,
    const QList<unsigned int> &track_list)
{
    Q_ASSERT(stripes.count() == track_list.count());
    if (stripes.count() != track_list.count())
	return false;

    QListIterator<Kwave::Stripe::List> it_s(stripes);
    QListIterator<unsigned int>        it_t(track_list);
    while (it_s.hasNext() && it_t.hasNext()) {
	Kwave::Stripe::List s = it_s.next();
	unsigned int        t = it_t.next();
	if (!m_signal.mergeStripes(s, t))
	    return false; // operation failed
    }

    return true;
}

//***************************************************************************
Kwave::PlaybackController &Kwave::SignalManager::playbackController()
{
    return m_playback_controller;
}

//***************************************************************************
void Kwave::SignalManager::startUndoTransaction(const QString &name)
{
    if (!m_undo_enabled) return; // undo is currently not enabled

    QMutexLocker lock(&m_undo_transaction_lock);

    // check for modified selection
    checkSelectionChange();

    // increase recursion level
    m_undo_transaction_level++;

    // start/create a new transaction if none existing
    if (!m_undo_transaction) {

	// if a new action starts, discard all redo actions !
	flushRedoBuffer();

	m_undo_transaction = new(std::nothrow) Kwave::UndoTransaction(name);
	Q_ASSERT(m_undo_transaction);
	if (!m_undo_transaction) return;

	// give all registered undo handlers a chance to register their own
	// undo actions
	if (!m_undo_manager.startUndoTransaction(m_undo_transaction)) {
	    delete m_undo_transaction;
	    m_undo_transaction = 0;
	}

	// if it is the start of the transaction, also create one
	// for the selection
	UndoAction *selection = new(std::nothrow) Kwave::UndoSelection(*this);
	Q_ASSERT(selection);
	if (selection && selection->store(*this)) {
	    m_undo_transaction->append(selection);
	} else {
	    // out of memory
	    delete selection;
	    delete m_undo_transaction;
	    m_undo_transaction = 0;
	}
    }
}

//***************************************************************************
void Kwave::SignalManager::closeUndoTransaction()
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
		// if the transaction has been aborted, undo all actions
		// that have currently been queued but do not
		// use the transaction any more, instead delete it.
		if (m_undo_transaction->isAborted()) {
		    qDebug("SignalManager::closeUndoTransaction(): aborted");
		    while (!m_undo_transaction->isEmpty()) {
			UndoAction *undo_action;
			UndoAction *redo_action;

			// unqueue the undo action
			undo_action = m_undo_transaction->takeLast();
			Q_ASSERT(undo_action);
			if (!undo_action) continue;

			// execute the undo operation
			redo_action = undo_action->undo(*this, false);

			// remove the old undo action if no longer used
			if (redo_action && (redo_action != undo_action))
			    delete redo_action;
			delete undo_action;
		    }
		    delete m_undo_transaction;
		    m_undo_transaction = 0;
		} else {
		    m_undo_buffer.append(m_undo_transaction);
		}
	    } else {
		qDebug("SignalManager::closeUndoTransaction(): empty");
		delete m_undo_transaction;
		m_undo_transaction = 0;
	    }
	}

	// dump, for debugging
// 	if (m_undo_transaction)
// 	    m_undo_transaction->dump("closed undo transaction: ");

	// declare the current transaction as "closed"
	rememberCurrentSelection();
	m_undo_transaction = 0;
	emitUndoRedoInfo();
    }
}

//***************************************************************************
void Kwave::SignalManager::enableUndo()
{
    m_undo_enabled = true;
    emitUndoRedoInfo();
}

//***************************************************************************
void Kwave::SignalManager::disableUndo()
{
    Q_ASSERT(m_undo_transaction_level == 0);

    flushUndoBuffers();
    m_undo_enabled = false;
}

//***************************************************************************
void Kwave::SignalManager::flushUndoBuffers()
{
    QMutexLocker lock(&m_undo_transaction_lock);

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
void Kwave::SignalManager::abortUndoTransaction()
{
    // abort the current transaction
    if (m_undo_transaction) m_undo_transaction->abort();
}

//***************************************************************************
void Kwave::SignalManager::flushRedoBuffer()
{
    qDeleteAll(m_redo_buffer);
    m_redo_buffer.clear();
    emitUndoRedoInfo();
}

//***************************************************************************
bool Kwave::SignalManager::continueWithoutUndo()
{
    // undo has not been enabled before?
    if (!m_undo_transaction) return true;

    // transaction has been aborted before
    if (m_undo_transaction->isAborted()) return false;

    // transaction is empty -> must have been flushed before, otherwise
    // it would contain at least a undo action for the selection
    // => user already has pressed "continue"
    if (m_undo_transaction->isEmpty()) return true;

    if (Kwave::MessageBox::warningContinueCancel(m_parent_widget,
	_("<html>") +
	i18n("Not enough memory for saving undo information.") +
	_("<br><br><b>") +
	i18n("Do you want to continue without the possibility to undo?") +
	_("</b><br><br><i>") +
	i18n("<b>Hint</b>: you can configure the amount of memory<br>"
	     "available for undo under '%1'/'%2'.",
	     i18n("Settings"),
	     i18n("Memory") +
	_("</i></html>"))) == KMessageBox::Continue)
    {
	// the signal was modified, it will stay in this state, it is
	// not possible to change to "non-modified" state through undo
	// from now on...
	setModified(true);
	enableModifiedChange(false);

	// flush the current undo transaction
	while (!m_undo_transaction->isEmpty()) {
	    Kwave::UndoAction *undo_action = m_undo_transaction->takeLast();
	    if (undo_action) delete undo_action;
	}

	// flush all undo/redo buffers
	flushUndoBuffers();
	return true;
    }

    // Set the undo transaction into "aborted" state. The final
    // closeUndoTransaction() will take care of the rest when
    // detecting that state and clean up...
    abortUndoTransaction();
    return false;
}

//***************************************************************************
bool Kwave::SignalManager::registerUndoAction(Kwave::UndoAction *action)
{
    QMutexLocker lock(&m_undo_transaction_lock);

    if (!action) return continueWithoutUndo();

    // if undo is not enabled, this will fail -> discard the action
    if (!m_undo_enabled) {
	delete action;
	return true;
    }

    // check if the undo action is too large
    qint64 limit_mb     = Kwave::MemoryManager::instance().undoLimit();
    qint64 needed_size  = action->undoSize();
    qint64 needed_mb    = needed_size  >> 20;
    if (needed_mb > limit_mb) {
	delete action;
	return continueWithoutUndo();
    }

    // undo has been aborted before ?
    if (!m_undo_transaction) return true;

    // transaction has been aborted before
    if (m_undo_transaction->isAborted()) {
	delete action;
	return true;
    }

    // make room...
    freeUndoMemory(needed_size);

    // now we might have enough place to append the undo action
    // and store all undo info
    if (!action->store(*this)) {
	delete action;
	return continueWithoutUndo();
    }

    // everything went ok, register internally
    m_undo_transaction->append(action);

    return true;
}

//***************************************************************************
bool Kwave::SignalManager::saveUndoDelete(QList<unsigned int> &track_list,
                                          sample_index_t offset,
                                          sample_index_t length)
{
    if (!m_undo_enabled) return true;
    if (track_list.isEmpty()) return true;

    // create a undo action for deletion
    UndoDeleteAction *action = new(std::nothrow)
	UndoDeleteAction(m_parent_widget, track_list, offset, length);
    if (!registerUndoAction(action)) return false;

    return true;
}

//***************************************************************************
qint64 Kwave::SignalManager::usedUndoRedoMemory()
{
    qint64 size = 0;

    foreach (Kwave::UndoTransaction *undo, m_undo_buffer)
	if (undo) size += undo->undoSize();

    foreach (Kwave::UndoTransaction *redo, m_redo_buffer)
	if (redo) size += redo->undoSize();

    return size;
}

//***************************************************************************
void Kwave::SignalManager::freeUndoMemory(qint64 needed)
{
    qint64 size = usedUndoRedoMemory() + needed;
    qint64 undo_limit = Kwave::MemoryManager::instance().undoLimit() << 20;

    // remove old undo actions if not enough free memory
    while (!m_undo_buffer.isEmpty() && (size > undo_limit)) {
	Kwave::UndoTransaction *undo = m_undo_buffer.takeFirst();
	if (!undo) continue;
	qint64 s = undo->undoSize();
	size = (size >= s) ? (size - s) : 0;
	delete undo;

	// if the signal was modified, it will stay in this state, it is
	// not possible to change to "non-modified" state through undo
	if (m_modified) enableModifiedChange(false);
    }

    // remove old redo actions if still not enough memory
    while (!m_redo_buffer.isEmpty() && (size > undo_limit)) {
	Kwave::UndoTransaction *redo = m_redo_buffer.takeLast();
	if (!redo) continue;
	qint64 s = redo->undoSize();
	size = (size >= s) ? (size - s) : 0;
	delete redo;
    }
}

//***************************************************************************
void Kwave::SignalManager::emitUndoRedoInfo()
{
    QString undo_name = QString();
    QString redo_name = QString();

    if (m_undo_enabled) {
	Kwave::UndoTransaction *transaction;

	// get the description of the last undo action
	if (!m_undo_buffer.isEmpty()) {
	    transaction = m_undo_buffer.last();
	    if (transaction) undo_name = transaction->description();
	    if (!undo_name.length()) undo_name = i18n("Last Action");
	}

	// get the description of the last redo action
	if (!m_redo_buffer.isEmpty()) {
	    transaction = m_redo_buffer.first();
	    if (transaction) redo_name = transaction->description();
	    if (!redo_name.length()) redo_name = i18n("Last Action");
	}
    }

    // now emit the undo/redo transaction names
    emit sigUndoRedoInfo(undo_name, redo_name);
}

//***************************************************************************
void Kwave::SignalManager::undo()
{
    QMutexLocker lock(&m_undo_transaction_lock);

    // check for modified selection
    checkSelectionChange();

    // remember the last selection
    rememberCurrentSelection();

    // get the last undo transaction and abort if none present
    if (m_undo_buffer.isEmpty()) return;
    Kwave::UndoTransaction *undo_transaction = m_undo_buffer.takeLast();
    if (!undo_transaction) return;

    // dump, for debugging
//     undo_transaction->dump("before undo: ");

    // temporarily disable undo while undo itself is running
    bool old_undo_enabled = m_undo_enabled;
    m_undo_enabled = false;

    // get free memory for redo
    qint64 undo_limit = Kwave::MemoryManager::instance().undoLimit() << 20;
    qint64 redo_size = undo_transaction->redoSize();
    qint64 undo_size = undo_transaction->undoSize();
    Kwave::UndoTransaction *redo_transaction = 0;
    if ((redo_size > undo_size) && (redo_size - undo_size > undo_limit)) {
	// not enough memory for redo
	qWarning("SignalManager::undo(): not enough memory for redo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(redo_size);

	// create a new redo transaction
	QString name = undo_transaction->description();
	redo_transaction = new(std::nothrow) Kwave::UndoTransaction(name);
	Q_ASSERT(redo_transaction);
    }

    // if *one* redo fails, all following redoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!redo_transaction) {
	flushRedoBuffer();
	qDebug("SignalManager::undo(): redo buffer flushed!");
    }

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // execute all undo actions and store the resulting redo
    // actions into the redo transaction
    while (!undo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;

	// unqueue the undo action
	undo_action = undo_transaction->takeLast();
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

    if (redo_transaction && (redo_transaction->count() < 1)) {
	// if there is no redo action -> no redo possible
	qWarning("SignalManager::undo(): no redo possible");
	delete redo_transaction;
	redo_transaction = 0;
    }

    // check whether the selection has changed, if yes: put a undo action
    // for this selection change at the end of the redo transaction
    if (redo_transaction) {
	bool range_modified = !(m_selection == m_last_selection);
	QList<unsigned int> tracks = selectedTracks();
	bool tracks_modified = !(tracks == m_last_track_selection);
	if (range_modified || tracks_modified) {
	    UndoAction *redo_action = new(std::nothrow) Kwave::UndoSelection(
		*this,
		m_last_track_selection,
		m_last_selection.offset(),
		m_last_selection.length());
	    Q_ASSERT(redo_action);
	    if (redo_action) redo_transaction->append(redo_action);
	}
    }

    // find out if there is still an action in the undo buffer
    // that has to do with modification of the signal
    if (m_modified) {
	bool stay_modified = false;
	foreach (Kwave::UndoTransaction *transaction, m_undo_buffer) {
	    if (!transaction) continue;
	    if (transaction->containsModification()) {
		stay_modified = true;
		break;
	    }
	}
	if (!stay_modified) {
	    // try to return to non-modified mode (might be a nop if
	    // not enabled)
	    setModified(false);
	}
    }

    // save "redo" information if possible
    if (redo_transaction)
	m_redo_buffer.prepend(redo_transaction);

    // remember the last selection
    rememberCurrentSelection();

    // re-enable undo
    m_undo_enabled = old_undo_enabled;

    // finished / buffers have changed, emit new undo/redo info
    emitUndoRedoInfo();

    // maybe the meta data has changed
    emit sigMetaDataChanged(m_meta_data);

    // remove hourglass
    QApplication::restoreOverrideCursor();
}

//***************************************************************************
void Kwave::SignalManager::redo()
{
    QMutexLocker lock(&m_undo_transaction_lock);

    // get the last redo transaction and abort if none present
    if (m_redo_buffer.isEmpty()) return;
    Kwave::UndoTransaction *redo_transaction = m_redo_buffer.takeFirst();
    if (!redo_transaction) return;

    // check for modified selection
    checkSelectionChange();

    // temporarily disable undo while redo is running
    bool old_undo_enabled = m_undo_enabled;
    m_undo_enabled = false;

    // get free memory for undo
    qint64 undo_limit = Kwave::MemoryManager::instance().undoLimit() << 20;
    qint64 undo_size = redo_transaction->undoSize();
    qint64 redo_size = redo_transaction->redoSize();
    Kwave::UndoTransaction *undo_transaction = 0;
    if ((undo_size > redo_size) && (undo_size - redo_size > undo_limit)) {
	// not enough memory for undo
	qWarning("SignalManager::redo(): not enough memory for undo !");
    } else {
	// only free the memory if it will be used
	freeUndoMemory(undo_size);

	// create a new undo transaction
	QString name = redo_transaction->description();
	undo_transaction = new(std::nothrow) Kwave::UndoTransaction(name);
	Q_ASSERT(undo_transaction);
    }

    // if *one* undo fails, all following undoes will also fail or
    // produce inconsistent data -> remove all of them !
    if (!undo_transaction) {
	qDeleteAll(m_undo_buffer);
	m_undo_buffer.clear();
    } else {
	m_undo_buffer.append(undo_transaction);
    }

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // execute all redo actions and store the resulting undo
    // actions into the undo transaction
    bool modified = false;
    while (!redo_transaction->isEmpty()) {
	UndoAction *undo_action;
	UndoAction *redo_action;

	// unqueue the undo action
	redo_action = redo_transaction->takeFirst();

	// execute the redo operation
	Q_ASSERT(redo_action);
	if (!redo_action) continue;
	modified |= redo_transaction->containsModification();
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

    if (undo_transaction && (undo_transaction->count() < 1)) {
	// if there is no undo action -> no undo possible
	qWarning("SignalManager::redo(): no undo possible");
	m_undo_buffer.removeAll(undo_transaction);
	delete undo_transaction;
    }

    // if the redo operation modified something,
    // we have to update the "modified" flag of the current signal
    if (modified) setModified(true);

    // remember the last selection
    rememberCurrentSelection();

    // re-enable undo
    m_undo_enabled = old_undo_enabled;

    // finished / buffers have changed, emit new undo/redo info
    emitUndoRedoInfo();

    // maybe the meta data has changed
    emit sigMetaDataChanged(m_meta_data);

    // remove hourglass
    QApplication::restoreOverrideCursor();
}

//***************************************************************************
void Kwave::SignalManager::setModified(bool mod)
{
    if (!m_modified_enabled) return;

    if (m_modified != mod) {
	m_modified = mod;
// 	qDebug("SignalManager::setModified(%d)",mod);
	emit sigModified(m_modified);
    }
}

//***************************************************************************
void Kwave::SignalManager::enableModifiedChange(bool en)
{
    m_modified_enabled = en;
}

//***************************************************************************
void Kwave::SignalManager::setFileInfo(Kwave::FileInfo &new_info,
                                       bool with_undo)
{
    if (m_undo_enabled && with_undo) {
	/* save data for undo */
	Kwave::UndoTransactionGuard undo_transaction(*this,
	                                             i18n("Modify File Info"));
	Kwave::FileInfo old_inf(m_meta_data);
	if (!registerUndoAction(
	    new(std::nothrow) Kwave::UndoModifyMetaDataAction(
		Kwave::MetaDataList(old_inf))))
	    return;
    }

    m_meta_data.replace(Kwave::MetaDataList(new_info));
    setModified(true);
    emitUndoRedoInfo();
    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
Kwave::Label Kwave::SignalManager::findLabel(sample_index_t pos)
{
    Kwave::LabelList labels(m_meta_data);
    foreach (const Kwave::Label &label, labels) {
	if (label.pos() == pos) return label; // found it
    }
    return Kwave::Label(); // nothing found
}

//***************************************************************************
int Kwave::SignalManager::labelIndex(const Kwave::Label &label) const
{
    int index = 0;
    Kwave::LabelList labels(m_meta_data);
    foreach (const Kwave::Label &l, labels) {
	if (l == label) return index; // found it
	index++;
    }
    return -1; // nothing found*/
}

//***************************************************************************
Kwave::Label Kwave::SignalManager::addLabel(sample_index_t pos,
                                            const QString &name)
{
    // if there already is a label at the given position, do nothing
    if (!findLabel(pos).isNull()) return Kwave::Label();

    // create a new label
    Kwave::Label label(pos, name);

    // register the undo action
    if (m_undo_enabled) {
	Kwave::UndoTransactionGuard undo(*this, i18n("Add Label"));
	if (!registerUndoAction(new(std::nothrow) UndoAddMetaDataAction(
	        Kwave::MetaDataList(label))))
	    return Kwave::Label();
    }

    // put the label into the list
    m_meta_data.add(label);

    // register this as a modification
    setModified(true);

    emit sigMetaDataChanged(m_meta_data);

    return label;
}

//***************************************************************************
void Kwave::SignalManager::deleteLabel(int index, bool with_undo)
{
    Kwave::LabelList labels(m_meta_data);

    if ((index < 0) || (index >= Kwave::toInt(labels.count()))) return;

    Kwave::MetaData label(labels.at(index));

    // register the undo action
    if (with_undo) {
	Kwave::UndoTransactionGuard undo(*this, i18n("Delete Label"));
	if (!registerUndoAction(new(std::nothrow)
	    UndoDeleteMetaDataAction(Kwave::MetaDataList(label))))
	    return;
    }

    m_meta_data.remove(label);

    // register this as a modification
    setModified(true);

    emit sigMetaDataChanged(m_meta_data);
}

//***************************************************************************
bool Kwave::SignalManager::modifyLabel(int index, sample_index_t pos,
                                       const QString &name)
{
    Kwave::LabelList labels(m_meta_data);
    if ((index < 0) || (index >= Kwave::toInt(labels.count())))
	return false;

    Kwave::Label label = labels.at(index);

    // check: if the label should be moved and there already is a label
    // at the new position -> fail
    if ((pos != label.pos()) && !findLabel(pos).isNull())
	return false;

    // add a undo action
    if (m_undo_enabled) {
	Kwave::UndoModifyMetaDataAction *undo_modify =
	    new(std::nothrow) Kwave::UndoModifyMetaDataAction(
	            Kwave::MetaDataList(label));
	if (!registerUndoAction(undo_modify))
	    return false;
    }

    // now modify the label
    label.moveTo(pos);
    label.rename(name);
    m_meta_data.add(label);

    // register this as a modification
    setModified(true);

    emit sigMetaDataChanged(m_meta_data);
    return true;
}

//***************************************************************************
void Kwave::SignalManager::rememberCurrentSelection()
{
    m_last_selection       = m_selection;
    m_last_track_selection = selectedTracks();
}

//***************************************************************************
void Kwave::SignalManager::checkSelectionChange()
{
    if (m_undo_transaction_level) return;

    // detect sample selection change
    bool range_modified = !(m_selection == m_last_selection);

    // detect track selection change
    QList<unsigned int> tracks = selectedTracks();
    bool tracks_modified = !(tracks == m_last_track_selection);

    if (range_modified || tracks_modified) {
	// selection has changed since last undo/redo operation
// 	qDebug("SignalManager::checkSelectionChange() => manually modified");
// 	qDebug("    before: [%8u...%8u]", m_last_selection.first(), m_last_selection.last());
// 	qDebug("    after : [%8u...%8u]", m_selection.first(),      m_selection.last());

	// temporarily activate the previous selection (last stored)
	Kwave::Selection new_selection(m_selection);
	m_selection = m_last_selection;
	selectTracks(m_last_track_selection);

	// save the last selection into a undo action
	if (tracks_modified && !range_modified)
	    Kwave::UndoTransactionGuard undo(*this,
	                                     i18n("Manual Track Selection"));
	else
	    Kwave::UndoTransactionGuard undo(*this,
	                                     i18n("Manual Selection"));

	// restore the current selection again
	m_selection = new_selection;
	selectTracks(tracks);
    }

}

//***************************************************************************
//***************************************************************************
