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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

#include <KLocalizedString>

#include "libkwave/CodecManager.h"
#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Parser.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"

#include "SaveBlocksDialog.h"
#include "SaveBlocksPlugin.h"

KWAVE_PLUGIN(Kwave::SaveBlocksPlugin, "saveblocks", "2.4",
             I18N_NOOP("Save Blocks"), "Thomas Eschenbacher");

//***************************************************************************
Kwave::SaveBlocksPlugin::SaveBlocksPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::Plugin(plugin_manager),
     m_url(), m_pattern(), m_numbering_mode(CONTINUE),
     m_selection_only(true), m_block_info()
{
}

//***************************************************************************
Kwave::SaveBlocksPlugin::~SaveBlocksPlugin()
{
}

//***************************************************************************
QStringList *Kwave::SaveBlocksPlugin::setup(QStringList &previous_params)
{
    // try to interpret the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    sample_index_t selection_left  = 0;
    sample_index_t selection_right = 0;
    selection(0, &selection_left, &selection_right, false);

    // enable the "selection only" checkbox only if there is something
    // selected but not everything
    bool selected_something = (selection_left != selection_right);
    bool selected_all = ((selection_left == 0) &&
                         (selection_right + 1 >= signalLength()));
    bool enable_selection_only = selected_something && !selected_all;

    QString filename = m_url.path();
    QString base = findBase(filename, m_pattern);
    scanBlocksToSave(base, m_selection_only && enable_selection_only);

    QPointer<Kwave::SaveBlocksDialog> dialog =
	new(std::nothrow) Kwave::SaveBlocksDialog(
	    _("kfiledialog:///kwave_save_blocks"),
	    Kwave::CodecManager::encodingFilter(),
	    parentWidget(),
	    QUrl::fromUserInput(signalName()),
	    _("*.wav"),
	    m_pattern,
	    m_numbering_mode,
	    m_selection_only,
	    enable_selection_only
	);
    if (!dialog) return 0;

    // connect the signals/slots from the plugin and the dialog
    connect(dialog, SIGNAL(sigSelectionChanged(QString,
	QString,Kwave::SaveBlocksPlugin::numbering_mode_t,bool)),
	this, SLOT(updateExample(QString,QString,
	Kwave::SaveBlocksPlugin::numbering_mode_t,bool)));
    connect(this, SIGNAL(sigNewExample(QString)),
	dialog, SLOT(setNewExample(QString)));

    dialog->setWindowTitle(i18n("Save Blocks"));
    dialog->emitUpdate();
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return 0;
    }

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list) {
	// user has pressed "OK"
	QString pattern;

	QUrl url = dialog->selectedUrl();
	if (url.isEmpty()) {
	    delete dialog;
	    return 0;
	}
	QString name = url.path();
	QFileInfo path(name);

	// add the correct extension if necessary
	if (!path.suffix().length()) {
	    QString ext = dialog->selectedExtension();
	    QStringList extensions = ext.split(_(" "));
	    ext = extensions.first();
	    name += ext.mid(1);
	    path = name;
	    url.setPath(name);
	}

	name     = Kwave::Parser::escape(name);
	pattern  = Kwave::Parser::escape(dialog->pattern());
	int mode = static_cast<int>(dialog->numberingMode());
	bool selection_only = (enable_selection_only) ?
	    dialog->selectionOnly() : m_selection_only;

	*list << name;
	*list << pattern;
	*list << QString::number(mode);
	*list << QString::number(selection_only);

	emitCommand(_("plugin:execute(saveblocks,") +
	    name + _(",") + pattern + _(",") +
	    QString::number(mode) + _(",") +
	    QString::number(selection_only) + _(")")
	);
    } else {
	// user pressed "Cancel"
	delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
QString Kwave::SaveBlocksPlugin::createDisplayList(
    const QStringList &list,
    unsigned int max_entries) const
{
    if (!max_entries || list.isEmpty()) return QString();

    QString retval;
    unsigned int count = 0;

    foreach (const QString &entry, list) {
	if (count == 0) // first entry
	    retval = _("<br><br>");
	if (count < max_entries)
	    retval += entry + _("<br>");
	else if (count == max_entries)
	    retval += i18n("...") + _("<br>");

	if (++count > max_entries)
	    break;
    }

    return retval;
}

//***************************************************************************
int Kwave::SaveBlocksPlugin::start(QStringList &params)
{
    qDebug("SaveBlocksPlugin::start()");

    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    QString filename = m_url.path();
    QFileInfo file(filename);
    QString path = file.absolutePath();
    QString ext  = file.suffix();
    QString base = findBase(filename, m_pattern);
    QByteArray sep("/");

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
    scanBlocksToSave(base, selection_only);
    unsigned int count = m_block_info.count();
    unsigned int first = firstIndex(path, base, ext, m_pattern,
                                    m_numbering_mode, count);

//     qDebug("m_url            = '%s'", m_url.prettyURL().local8Bit().data());
//     qDebug("m_pattern        = '%s'", m_pattern.local8Bit().data());
//     qDebug("m_numbering_mode = %d", (int)m_numbering_mode);
//     qDebug("selection_only   = %d", selection_only);
//     qDebug("indices          = %u...%u (count=%u)", first, first+count-1,count);

    // iterate over all blocks to check for overwritten files and missing dirs
    QStringList  overwritten_files;
    QStringList  missing_dirs;
    for (unsigned int i = first; i < (first + count); i++) {
	QString name = createFileName(base, ext, m_pattern, i, count,
	                              first + count - 1);
	QString display_name = Kwave::Parser::unescape(name);

	// split the name into directory and file name
	name = QString::fromLatin1(QUrl::toPercentEncoding(display_name, sep));
	QUrl url = m_url.adjusted(QUrl::RemoveFilename);
	url.setPath(url.path(QUrl::FullyEncoded) + name, QUrl::StrictMode);

	QString filename = url.path();
	QFileInfo file_info(filename);

	// check for potentially overwritten file
	if (file_info.exists())
	    overwritten_files += Kwave::Parser::unescape(display_name);

	// check for missing subdirectory
	if (!file_info.dir().exists()) {
	    QFileInfo inf(display_name);
	    QString missing_dir = inf.path();
	    if (!missing_dirs.contains(missing_dir))
		missing_dirs += missing_dir;
	}
    }

    // inform about overwritten files
    if (!overwritten_files.isEmpty()) {
	// ask the user for confirmation if he really wants to overwrite...
	if (Kwave::MessageBox::warningYesNo(parentWidget(),
	    _("<html>") +
	    i18n("This would overwrite the following file(s): %1" \
	    "Do you really want to continue?",
	    createDisplayList(overwritten_files, 5)) +
	    _("</html>") ) != KMessageBox::Yes)
	{
	    return -1;
	}
    }

    // handle missing directories
    if (!missing_dirs.isEmpty()) {
	// ask the user if he wants to continue and create the directory
	if (Kwave::MessageBox::warningContinueCancel(parentWidget(),
	    i18n("The following directories do not exist: %1"
	         "Do you want to create them and continue?",
	         createDisplayList(missing_dirs, 5)),
	    QString(),
	    QString(),
	    QString(),
	    _("saveblocks_create_missing_dirs")
	    ) != KMessageBox::Continue)
	{
	    return -1;
	}

	// create all missing directories
	QUrl base_url = m_url.adjusted(QUrl::RemoveFilename);
	foreach (const QString &missing, missing_dirs) {
	    QUrl url(base_url);
	    url.setPath(
		base_url.path(QUrl::FullyEncoded) +
		QString::fromLatin1(QUrl::toPercentEncoding(missing)),
		QUrl::StrictMode
	    );
	    QString path = url.path();
	    QDir dir;
	    if (!dir.mkpath(path))
		qWarning("creating path '%s' failed", DBG(path));
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
	    name = Kwave::Parser::unescape(name);
	    // use URL encoding for the filename
	    name = QString::fromLatin1(QUrl::toPercentEncoding(name, sep));
	    QUrl url = m_url.adjusted(QUrl::RemoveFilename);
	    url.setPath(url.path(QUrl::FullyEncoded) + name, QUrl::StrictMode);

	    qDebug("saving %9lu...%9lu -> '%s'",
		   static_cast<unsigned long int>(left),
		   static_cast<unsigned long int>(right),
		   DBG(url.toDisplayString()));
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
    m_url = QUrl::fromUserInput(Kwave::Parser::unescape(params[0]));
    if (!m_url.isValid()) return -EINVAL;

    // filename pattern
    m_pattern = Kwave::Parser::unescape(params[1]);
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
void Kwave::SaveBlocksPlugin::scanBlocksToSave(const QString &base,
                                               bool selection_only)
{
    sample_index_t selection_left, selection_right;

    sample_index_t block_start;
    sample_index_t block_end = 0;
    QString        block_title;
    Kwave::LabelList labels(signalManager().metaData());
    Kwave::LabelListIterator it(labels);
    Kwave::Label label = (it.hasNext()) ? it.next() : Kwave::Label();

    if (selection_only) {
	selection(0, &selection_left, &selection_right, true);
    } else {
	selection_left  = 0;
	selection_right = signalLength() - 1;
    }

    // get the title of the whole file, in case that a block does not have
    // an own title
    FileInfo info(signalManager().metaData());
    QString file_title = info.get(INF_NAME).toString();

    // fallback: if there is no INF_NAME either, fall back to the file
    //           name as last resort
    if (!file_title.length()) file_title = base;

    m_block_info.clear();
    QString prev_title;
    for (;;) {
	block_start = block_end;
	block_end   = (label.isNull()) ? signalLength() : label.pos();
	block_title = prev_title;
	prev_title  = (label.isNull()) ? file_title     : label.name();

	if ((block_end > selection_left) && (block_start <= selection_right)) {
	    BlockInfo block;
	    block.m_start  = block_start;
	    block.m_length = block_end - block_start;
	    block.m_title  = block_title;
	    if (!block.m_title.length()) block.m_title = file_title;
	    m_block_info.append(block);
	}

	if (label.isNull()) break;
	label = (it.hasNext()) ? it.next() : Kwave::Label();
    }
}

//***************************************************************************
QString Kwave::SaveBlocksPlugin::createFileName(const QString &base,
    const QString &ext, const QString &pattern,
    unsigned int index, int count, int total)
{
    QString p = QRegExp::escape(pattern);
    QString nr;

    // format the "index" parameter
    QRegExp rx_nr(_("(\\\\\\[%(\\d*)nr\\\\\\])"), Qt::CaseInsensitive);
    while (rx_nr.indexIn(p) >= 0) {
	QString format = rx_nr.cap(1);
	format = format.mid(2, format.length() - 6);
	QString ex = _("(\\\\\\[") + format + _("nr\\\\\\])");
	QRegExp rx(ex, Qt::CaseInsensitive);
	format += _("u");
	p.replace(rx, nr.sprintf(format.toLatin1(), index));
    }

    // format the "count" parameter
    QRegExp rx_count(_("(\\\\\\[%\\d*count\\\\\\])"), Qt::CaseInsensitive);
    while (rx_count.indexIn(p) >= 0) {
	if (count >= 0) {
	    QString format = rx_count.cap(1);
	    format = format.mid(2, format.length() - 9);
	    QString ex = _("(\\\\\\[") + format + _("count\\\\\\])");
	    QRegExp rx(ex, Qt::CaseInsensitive);
	    format += _("u");
	    p.replace(rx, nr.sprintf(format.toLatin1(), count));
	} else {
	    p.replace(rx_count, _("(\\d+)"));
	}
    }

    // format the "total" parameter
    QRegExp rx_total(_("(\\\\\\[%\\d*total\\\\\\])"), Qt::CaseInsensitive);
    while (rx_total.indexIn(p) >= 0) {
	if (total >= 0) {
	    QString format = rx_total.cap(1);
	    format = format.mid(2, format.length() - 9);
	    QString ex = _("(\\\\\\[") + format + _("total\\\\\\])");
	    QRegExp rx(ex, Qt::CaseInsensitive);
	    format += _("u");
	    p.replace(rx, nr.sprintf(format.toLatin1(), total));
	} else {
	    p.replace(rx_total, _("(\\d+)"));
	}
    }

    // format the "filename" parameter
    QRegExp rx_filename(_("\\\\\\[%filename\\\\\\]"), Qt::CaseInsensitive);
    if (rx_filename.indexIn(p) >= 0) {
	p.replace(rx_filename, QRegExp::escape(base));
    }

    // support for file info
    QRegExp rx_fileinfo(
	_("\\\\\\[%(\\d*)fileinfo\\\\\\{([\\w\\s]+)\\\\\\}\\\\\\]"),
	Qt::CaseInsensitive
    );
    Kwave::FileInfo info(signalManager().metaData());
    while (rx_fileinfo.indexIn(p) >= 0) {
	const QString format = rx_fileinfo.cap(1);
	const QString id     = rx_fileinfo.cap(2);
	QString value;
	FileProperty property = info.fromName(id);
	if (property != Kwave::INF_UNKNOWN) {
	    QVariant val = info.get(property);
	    if (!val.isNull()) {
		// we have a property value
		value = val.toString();

		// check for format (desired minimum string length)
		bool ok  = false;
		int  len = format.toUInt(&ok);
		if ((len > 0) && ok) {
		    Kwave::FileInfo::Flags flags = info.flags(property);
		    if (flags & Kwave::FileInfo::FP_FORMAT_NUMERIC) {
			// numeric format, pad with leading zeros or spaces
			QString pad = (format.startsWith(QLatin1Char('0'))) ?
			    _("0") : _(" ");
			while (value.length() < len)
			    value = pad + value;
		    } else {
			// string format, pad with trailing spaces
			while (value.length() < len)
			    value = value + _(" ");
		    }
		}
		value = Kwave::Parser::escape(value);
	    }
	}

	QString ex(_("(\\\\\\[%") + format + _("fileinfo\\\\\\{") + id +
	           _("\\\\\\}\\\\\\])"));
	QRegExp rx(ex, Qt::CaseInsensitive);
	p.replace(rx, value);
    }

    // format the "title" parameter
    QRegExp rx_title(_("\\\\\\[%title\\\\\\]"), Qt::CaseInsensitive);
    if (rx_title.indexIn(p) >= 0) {
	QString title;
	int idx = (index - 1) - (total - count);
	if ((idx >= 0) && (idx < m_block_info.count()))
	    title = m_block_info[idx].m_title;
	if (title.length())
	    p.replace(rx_title, QRegExp::escape(title));
    }

    if (ext.length()) p += _(".") + ext;

    // sanitize the filename/path, make sure that there are no spaces
    // before and after all path separators
    QString sep = _("/");
    QRegExp rx_sep(_("\\s*") + sep + _("\\s*"));
    p.replace(rx_sep, sep);

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
	    QDir dir(path, _("*"));
	    QStringList files;
	    files = dir.entryList();
	    for (unsigned int i = first; i < (first + count); i++) {
		QString name = createFileName(base, ext, pattern, i, -1, -1);
		QRegExp rx(_("^(") + name + _(")$"),
		           Qt::CaseInsensitive);
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
    QString name = file.fileName();
    QString base = file.completeBaseName();
    QString ext  = file.suffix();

    // convert the pattern into a regular expression in order to check if
    // the current name already is produced by the current pattern
    // \[%[0-9]?nr\]      -> \d+
    // \[%[0-9]?count\]   -> \d+
    // \[%[0-9]?total\]   -> \d+
    // \[%filename\]      -> base
    // \[%fileinfo\]      -> .
    // \[%title\]         -> .
    QRegExp rx_nr(_("\\\\\\[%\\d*nr\\\\\\]"), Qt::CaseInsensitive);
    QRegExp rx_count(_("\\\\\\[%\\d*count\\\\\\]"), Qt::CaseInsensitive);
    QRegExp rx_total(_("\\\\\\[%\\d*total\\\\\\]"), Qt::CaseInsensitive);
    QRegExp rx_filename(_("\\\\\\[%filename\\\\\\]"), Qt::CaseInsensitive);
    QRegExp rx_fileinfo(_("\\\\\\[%fileinfo\\\\\\]"), Qt::CaseInsensitive);
    QRegExp rx_title(_("\\\\\\[%title\\\\\\]"), Qt::CaseInsensitive);

    QString p = QRegExp::escape(pattern);
    int idx_nr       = rx_nr.indexIn(p);
    int idx_count    = rx_count.indexIn(p);
    int idx_total    = rx_total.indexIn(p);
    int idx_filename = rx_filename.indexIn(p);
    int idx_fileinfo = rx_fileinfo.indexIn(p);
    int idx_title    = rx_fileinfo.indexIn(p);
    p.replace(rx_nr,       _("(\\d+)"));
    p.replace(rx_count,    _("(\\d+)"));
    p.replace(rx_total,    _("(\\d+)"));
    p.replace(rx_filename, _("(.+)"));
    p.replace(rx_fileinfo, _("(.+)"));
    p.replace(rx_title,    _("(.+)"));

    int max = 0;
    for (int i = 0; i < pattern.length(); i++) {
	if (idx_nr       == max) max++;
	if (idx_count    == max) max++;
	if (idx_total    == max) max++;
	if (idx_filename == max) max++;
	if (idx_fileinfo == max) max++;
	if (idx_title    == max) max++;

	if (idx_nr       > max) idx_nr--;
	if (idx_count    > max) idx_count--;
	if (idx_total    > max) idx_total--;
	if (idx_filename > max) idx_filename--;
	if (idx_fileinfo > max) idx_fileinfo--;
	if (idx_title    > max) idx_title--;
    }

    if (ext.length()) p += _(".") + ext;
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
    QString ext  = file.suffix();
    QString base = findBase(filename, pattern);

    // now we have a new name, base and extension
    // -> find out the numbering, min/max etc...
    scanBlocksToSave(base, selection_only);
    unsigned int count = m_block_info.count();
    unsigned int first = firstIndex(path, base, ext, pattern, mode, count);
    unsigned int total = first + count - 1;

    // create the complete filename, including extension but without path
    return createFileName(base, ext, pattern, first, count, total);
}

//***************************************************************************
void Kwave::SaveBlocksPlugin::updateExample(const QString &filename,
    const QString &pattern, Kwave::SaveBlocksPlugin::numbering_mode_t mode,
    bool selection_only)
{
    QString example = firstFileName(filename, pattern, mode, selection_only);
    emit sigNewExample(Kwave::Parser::unescape(example));
}

//***************************************************************************
//***************************************************************************
