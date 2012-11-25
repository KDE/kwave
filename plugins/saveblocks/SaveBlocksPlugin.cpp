/***************************************************************************
   SaveBlocksPlugin.cpp  -  Plugin for saving blocks between labels
                             -------------------
    begin                : Thu Mar 01 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
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

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QRegExp>

#include <klocale.h>

#include "libkwave/CodecManager.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/SignalManager.h"

#include "SaveBlocksDialog.h"
#include "SaveBlocksPlugin.h"

KWAVE_PLUGIN(Kwave::SaveBlocksPlugin, "saveblocks", "2.1",
             I18N_NOOP("Save Blocks"), "Thomas Eschenbacher");

//***************************************************************************
Kwave::SaveBlocksPlugin::SaveBlocksPlugin(const Kwave::PluginContext &c)
    :Kwave::Plugin(c), m_url(), m_pattern(), m_numbering_mode(CONTINUE),
     m_selection_only(true)
{
}

//***************************************************************************
Kwave::SaveBlocksPlugin::~SaveBlocksPlugin()
{
}

//***************************************************************************
QStringList *Kwave::SaveBlocksPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    KUrl url(signalName());
    sample_index_t selection_left  = 0;
    sample_index_t selection_right = 0;
    selection(0, &selection_left, &selection_right, false);

    // enable the "selection only" checkbox only if there is something
    // selected but not everything
    bool selected_something = (selection_left != selection_right);
    bool selected_all = ((selection_left == 0) &&
                         (selection_right + 1 >= signalLength()));
    bool enable_selection_only = selected_something && !selected_all;

    Kwave::SaveBlocksDialog *dialog = new Kwave::SaveBlocksDialog(
	":<kwave_save_blocks>", Kwave::CodecManager::encodingFilter(),
	parentWidget(), true,
	url.prettyUrl(), "*.wav",
	m_pattern,
	m_numbering_mode,
	m_selection_only,
	enable_selection_only
    );
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // connect the signals/slots from the plugin and the dialog
    connect(dialog, SIGNAL(sigSelectionChanged(const QString &,
	const QString &, Kwave::SaveBlocksPlugin::numbering_mode_t, bool)),
	this, SLOT(updateExample(const QString &, const QString &,
	Kwave::SaveBlocksPlugin::numbering_mode_t, bool)));
    connect(this, SIGNAL(sigNewExample(const QString &)),
	dialog, SLOT(setNewExample(const QString &)));

    dialog->setOperationMode(KFileDialog::Saving);
    dialog->setCaption(i18n("Save Blocks"));
    dialog->emitUpdate();
    if (dialog->exec() != QDialog::Accepted) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list) {
	// user has pressed "OK"
	QString pattern, name;

	name = QByteArray(
	    dialog->selectedUrl().prettyUrl().toUtf8()).toBase64();
	pattern = QByteArray(
	    dialog->pattern().toUtf8()).toBase64();
	int mode = static_cast<int>(dialog->numberingMode());
	bool selection_only = (enable_selection_only) ?
	    dialog->selectionOnly() : m_selection_only;

	*list << name;
	*list << pattern;
	*list << QString::number(mode);
	*list << QString::number(selection_only);

	emitCommand("plugin:execute(saveblocks,"+
	    name+","+
	    pattern+","+
	    QString::number(mode)+","+
	    QString::number(selection_only)+
	    ")"
	);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
int Kwave::SaveBlocksPlugin::start(QStringList &params)
{
    qDebug("SaveBlocksPlugin::start()");

    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    QString filename = m_url.prettyUrl();
    QFileInfo file(filename);
    QString path = file.absolutePath();
    QString name = file.fileName();
    QString ext  = file.suffix();
    QString base = findBase(filename, m_pattern);

    // determine the selection settings
    sample_index_t selection_left  = 0;
    sample_index_t selection_right = 0;
    selection(0, &selection_left, &selection_right, false);

    bool selected_something = (selection_left != selection_right);
    bool selected_all = ((selection_left == 0) &&
                         (selection_right+1 >= signalLength()));
    bool enable_selection_only = selected_something && !selected_all;
    bool selection_only = enable_selection_only && m_selection_only;

    if (selection_only) {
	selection(0, &selection_left, &selection_right, true);
    } else {
	selection_left  = 0;
	selection_right = signalLength() - 1;
    }

    // get the index range
    unsigned int count = blocksToSave(selection_only);
    unsigned int first = firstIndex(path, base, ext, m_pattern,
                                    m_numbering_mode, count);

//     qDebug("m_url            = '%s'", m_url.prettyURL().local8Bit().data());
//     qDebug("m_pattern        = '%s'", m_pattern.local8Bit().data());
//     qDebug("m_numbering_mode = %d", (int)m_numbering_mode);
//     qDebug("selection_only   = %d", selection_only);
//     qDebug("indices          = %u...%u (count=%u)", first, first+count-1,count);

    // check for filenames that might be overwritten
    const int max_overwrite_list_length = 7;
    QDir dir(path, "*");
    QStringList files;
    files = dir.entryList();
    QStringList overwritten;
    for (unsigned int i = first; i < first+count; i++) {
	QString name = createFileName(base, ext, m_pattern, i, count,
	                              first + count - 1);
	QRegExp rx("^(" + QRegExp::escape(name) + ")$", Qt::CaseInsensitive);
	QStringList matches = files.filter(rx);
	if (matches.count() > 0) {
	    overwritten += name;
	    if (overwritten.count() > max_overwrite_list_length)
		break; // we have collected enough names...
	}
    }
    if (overwritten.count()) {
	// ask the user for confirmation if he really wants to overwrite...

	QString list = "<br><br>";
	int cnt = 0;
	for (QStringList::Iterator it = overwritten.begin();
	     it != overwritten.end() && (cnt <= max_overwrite_list_length);
	     ++it, ++cnt)
	{
	    list += (*it);
	    list += "<br>";
	}
	if (overwritten.count() > max_overwrite_list_length)
	    list += i18n("...");
	list += "<br>";

	if (Kwave::MessageBox::warningYesNo(parentWidget(),
	    "<html>" +
	    i18n("This would overwrite the following file(s): %1" \
	    "Do you really want to continue?",
	    list) + "</html>") != KMessageBox::Yes)
	{
	    return -1;
	}
    }

    // save the current selection, we have to restore it afterwards!
    sample_index_t saved_selection_left  = 0;
    sample_index_t saved_selection_right = 0;
    selection(0, &saved_selection_left, &saved_selection_right, false);

    // now we can loop over all blocks and save them
    sample_index_t block_start;
    sample_index_t block_end = 0;
    Kwave::LabelList labels(signalManager().metaData());
    Kwave::LabelListIterator it(labels);
    Kwave::Label label = it.hasNext() ? it.next() : Kwave::Label();

    for (unsigned int index = first;;) {
	block_start = block_end;
	block_end   = (label.isNull()) ? signalLength() : label.pos();

	if ((selection_left < block_end) && (selection_right > block_start)) {
	    // found a block to save...
	    Q_ASSERT(index < first + count);

	    sample_index_t left  = block_start;
	    sample_index_t right = block_end - 1;
	    if (left  < selection_left)  left  = selection_left;
	    if (right > selection_right) right = selection_right;
	    Q_ASSERT(right > left);
	    if (right <= left) break; // zero-length ?

	    // select the range of samples
	    selectRange(left, right - left + 1);

	    // determine the filename
	    QString name = createFileName(base, ext, m_pattern, index, count,
                                          first + count - 1);
	    KUrl url = m_url;
	    url.setFileName(name);
	    filename = url.prettyUrl();

	    qDebug("saving %9lu...%9lu -> '%s'",
		   static_cast<unsigned long int>(left),
		   static_cast<unsigned long int>(right),
		   filename.toLocal8Bit().data());
	    if (signalManager().save(url, true) < 0)
		break;

	    // increment the index for the next filename
	    index++;
	}
	if (label.isNull()) break;
	label = (it.hasNext()) ? it.next() : Kwave::Label();
    }

    // restore the previous selection
    selectRange(saved_selection_left,
	(saved_selection_left != saved_selection_right) ?
	(saved_selection_right - saved_selection_left + 1) : 0);

    return result;
}

//***************************************************************************
int Kwave::SaveBlocksPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 4) {
	return -EINVAL;
    }

    // the selected URL
    m_url = QString::fromUtf8(QByteArray::fromBase64(params[0].toAscii()));
    if (!m_url.isValid()) return -EINVAL;

    // filename pattern
    m_pattern =
	QString::fromUtf8(QByteArray::fromBase64(params[1].toAscii()));
    if (!m_pattern.length()) return -EINVAL;

    // numbering mode
    param = params[2];
    int mode = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    if ((mode != CONTINUE) &&
        (mode != START_AT_ONE)) return -EINVAL;
    m_numbering_mode = static_cast<numbering_mode_t>(mode);

    // flag: save only the selection
    param = params[3];
    m_selection_only = (param.toUInt(&ok) != 0);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
unsigned int Kwave::SaveBlocksPlugin::blocksToSave(bool selection_only)
{
    unsigned int count = 0;
    sample_index_t selection_left, selection_right;

    sample_index_t block_start;
    sample_index_t block_end = 0;
    Kwave::LabelList labels(signalManager().metaData());
    Kwave::LabelListIterator it(labels);
    Kwave::Label label = (it.hasNext()) ? it.next() : Kwave::Label();

    if (selection_only) {
	selection(0, &selection_left, &selection_right, true);
    } else {
	selection_left = 0;
	selection_right = signalLength() - 1;
    }
    for (;;) {
	block_start = block_end;
	block_end   = (label.isNull()) ? signalLength() : label.pos();
	if ((selection_left < block_end) && (selection_right > block_start))
	    count++;
	if (label.isNull()) break;
	label = (it.hasNext()) ? it.next() : Kwave::Label();
    }

    return count;
}

//***************************************************************************
QString Kwave::SaveBlocksPlugin::createFileName(const QString &base,
    const QString &ext, const QString &pattern,
    unsigned int index, int count, int total)
{
    QString p = QRegExp::escape(pattern);
    QString nr;

    // format the "index" parameter
    QRegExp rx_nr("(\\\\\\[%\\d*nr\\\\\\])", Qt::CaseInsensitive);
    while (rx_nr.indexIn(p) >= 0) {
	QString format = rx_nr.cap(1);
	format = format.mid(2, format.length() - 6) + "u";
	p.replace(rx_nr, nr.sprintf(format.toAscii(), index));
    }

    // format the "count" parameter
    QRegExp rx_count("(\\\\\\[%\\d*count\\\\\\])", Qt::CaseInsensitive);
    while (rx_count.indexIn(p) >= 0) {
	if (count >= 0) {
	    QString format = rx_count.cap(1);
	    format = format.mid(2, format.length() - 9) + "u";
	    p.replace(rx_count, nr.sprintf(format.toAscii(), count));
	} else {
	    p.replace(rx_count, "(\\d+)");
	}
    }

    // format the "total" parameter
    QRegExp rx_total("(\\\\\\[%\\d*total\\\\\\])", Qt::CaseInsensitive);
    while (rx_total.indexIn(p) >= 0) {
	if (total >= 0) {
	    QString format = rx_total.cap(1);
	    format = format.mid(2, format.length() - 9) + "u";
	    p.replace(rx_total, nr.sprintf(format.toAscii(), total));
	} else {
	    p.replace(rx_total, "(\\d+)");
	}
    }

    // format the "filename" parameter
    QRegExp rx_filename("\\\\\\[%filename\\\\\\]", Qt::CaseInsensitive);
    if (rx_filename.indexIn(p) >= 0) {
	p.replace(rx_filename, QRegExp::escape(base));
    }

    if (ext.length()) p += "." + ext;
    return p;
}

//***************************************************************************
unsigned int Kwave::SaveBlocksPlugin::firstIndex(const QString &path,
    const QString &base, const QString &ext, const QString &pattern,
    Kwave::SaveBlocksPlugin::numbering_mode_t mode, unsigned int count)
{
    unsigned int first = 1;
    switch (mode) {
	case START_AT_ONE:
	    first = 1;
	    break;
	case CONTINUE: {
	    QDir dir(path, "*");
	    QStringList files;
	    files = dir.entryList();
	    for (unsigned int i = first; i < first+count; i++) {
		QString name = createFileName(base, ext, pattern, i, -1, -1);
		QRegExp rx("^(" + name + ")$", Qt::CaseInsensitive);
		QStringList matches = files.filter(rx);
		if (matches.count() > 0) first = i + 1;
	    }
	    break;
	}
    }

    return first;
}

//***************************************************************************
QString Kwave::SaveBlocksPlugin::findBase(const QString &filename,
                                          const QString &pattern)
{
    QFileInfo file(filename);
    QString path = file.absolutePath();
    QString name = file.fileName();
    QString base = file.completeBaseName();
    QString ext  = file.suffix();

    // convert the pattern into a regular expression in order to check if
    // the current name already is produced by the current pattern
    // \[%[0-9]?nr\]      -> \d+
    // \[%[0-9]?count\]   -> \d+
    // \[%[0-9]?total\]   -> \d+
    // \[%filename\]      -> base
    QRegExp rx_nr("\\\\\\[%\\d*nr\\\\\\]", Qt::CaseInsensitive);
    QRegExp rx_count("\\\\\\[%\\d*count\\\\\\]", Qt::CaseInsensitive);
    QRegExp rx_total("\\\\\\[%\\d*total\\\\\\]", Qt::CaseInsensitive);
    QRegExp rx_filename("\\\\\\[%filename\\\\\\]", Qt::CaseInsensitive);

    QString p = QRegExp::escape(pattern);
    int idx_nr = rx_nr.indexIn(p);
    int idx_count = rx_count.indexIn(p);
    int idx_total = rx_total.indexIn(p);
    int idx_filename = rx_filename.indexIn(p);
    p.replace(rx_nr, "(\\d+)");
    p.replace(rx_count, "(\\d+)");
    p.replace(rx_total, "(\\d+)");
    p.replace(rx_filename, "(.+)");

    int max = 0;
    for (int i=0; i < pattern.length(); i++) {
	if (idx_nr       == max) max++;
	if (idx_count    == max) max++;
	if (idx_total    == max) max++;
	if (idx_filename == max) max++;
	if (idx_nr       > max) idx_nr--;
	if (idx_count    > max) idx_count--;
	if (idx_total    > max) idx_total--;
	if (idx_filename > max) idx_filename--;
    }

    if (ext.length()) p += "." + ext;
    QRegExp rx_current(p, Qt::CaseInsensitive);
    if (rx_current.indexIn(name) >= 0) {
	// filename already produced by this pattern
	base = rx_current.cap(idx_filename + 1);
    }

    return base;
}

//***************************************************************************
QString Kwave::SaveBlocksPlugin::firstFileName(const QString &filename,
    const QString &pattern, Kwave::SaveBlocksPlugin::numbering_mode_t mode,
    bool selection_only)
{
    QFileInfo file(filename);
    QString path = file.absolutePath();
    QString name = file.fileName();
    QString ext  = file.suffix();
    QString base = findBase(filename, pattern);

    // now we have a new name, base and extension
    // -> find out the numbering, min/max etc...
    unsigned int count = blocksToSave(selection_only);
    unsigned int first = firstIndex(path, base, ext, pattern, mode, count);
    unsigned int total = first + count - 1;

    // create the complete filename, including extension but without path
    return createFileName(base, ext, pattern, first, count, total);
}

//***************************************************************************
static QString unescape(const QString &s)
{
    QRegExp rx_single("\\\\(?!\\\\)");
    QString str = s;
    str.replace(rx_single, "");
    return str;
}

//***************************************************************************
void Kwave::SaveBlocksPlugin::updateExample(const QString &filename,
    const QString &pattern, Kwave::SaveBlocksPlugin::numbering_mode_t mode,
    bool selection_only)
{
    QString example = firstFileName(filename, pattern, mode, selection_only);
    emit sigNewExample(unescape(example));
}

//***************************************************************************
#include "SaveBlocksPlugin.moc"
//***************************************************************************
//***************************************************************************
