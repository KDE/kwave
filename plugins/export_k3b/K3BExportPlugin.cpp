/*************************************************************************
 *     K3BExportPlugin.cpp -  export of K3B project files
 *                             -------------------
 *    begin                : Thu Apr 13 2017
 *    copyright            : (C) 2017 by Thomas Eschenbacher
 *    email                : Thomas.Eschenbacher@gmx.de
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

#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QFileInfo>
#include <QIODevice>
#include <QMap>
#include <QProcess>
#include <QRegExp>
#include <QTextStream>

#include <KLocalizedString> // for the i18n macro
#include <KZip>

#include "libkwave/CodecManager.h"
#include "libkwave/Encoder.h"
#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/Logger.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Parser.h"
#include "libkwave/Plugin.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "K3BExportDialog.h"
#include "K3BExportPlugin.h"

KWAVE_PLUGIN(export_k3b, K3BExportPlugin)

/** mime type of K3B project files */
#define K3B_PROJECT_MIME_TYPE "application/x-k3b"

/** file suffix of K3B project files */
#define K3B_FILE_SUFFIX _("*.k3b")

/** number of digits to use for out files */
#define OUTFILE_DIGITS 4

/** file name pattern for out files */
#define OUTFILE_PATTERN (_("[%0") + _("%1nr]").arg(OUTFILE_DIGITS))

/** file suffix for out files */
#define OUTFILE_SUFFIX  _(".wav")

//***************************************************************************
Kwave::K3BExportPlugin::K3BExportPlugin(QObject *parent,
                                        const QVariantList &args)
    :Kwave::Plugin(parent, args),
     m_url(),
     m_pattern(),
     m_selection_only(false),
     m_export_location(EXPORT_TO_SUB_DIR),
     m_overwrite_policy(USE_NEW_FILE_NAMES),
     m_block_info()
{
}

//***************************************************************************
Kwave::K3BExportPlugin::~K3BExportPlugin()
{
}

//***************************************************************************
int Kwave::K3BExportPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5)
	return -EINVAL;

    // the selected URL
    m_url = QUrl::fromUserInput(Kwave::Parser::unescape(params[0]));
    if (!m_url.isValid()) return -EINVAL;

    // label pattern
    m_pattern = Kwave::Parser::unescape(params[1]);

    // selection only
    param = params[2];
    int v = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    m_selection_only = (v != 0);

    // export location
    param = params[3];
    int where = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    if ((where != EXPORT_TO_SAME_DIR) &&
	(where != EXPORT_TO_SUB_DIR)) return -EINVAL;
    m_export_location = static_cast<export_location_t>(where);

    // overwrite policy
    param = params[4];
    int overwrite = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    if ((overwrite != OVERWRITE_EXISTING_FILES) &&
	(overwrite != USE_NEW_FILE_NAMES)) return -EINVAL;
    m_overwrite_policy = static_cast<overwrite_policy_t>(overwrite);

    return 0;
}

//***************************************************************************
void Kwave::K3BExportPlugin::scanBlocksToSave(const QString &base,
                                              sample_index_t selection_left,
                                              sample_index_t selection_right)
{
    sample_index_t block_start;
    sample_index_t block_end = 0;

    Kwave::LabelList labels(signalManager().metaData());
    Kwave::LabelListIterator it(labels);
    Kwave::Label label = (it.hasNext()) ? it.next() : Kwave::Label();

    // get the title of the whole file, in case that a block does not have
    // an own title
    FileInfo info(signalManager().metaData());
    QString file_title  = info.get(INF_NAME).toString();
    QString file_artist = info.get(INF_AUTHOR).toString();

    // fallback: if there is no INF_NAME either, fall back to the file
    //           name as last resort
    if (!file_title.length()) file_title = base;

    m_block_info.clear();
    QString prev_title = file_title;
    for (unsigned int index = 1; ; ++index) {
	block_start = block_end;
	block_end   = (label.isNull()) ? signalLength() : label.pos();

	QString block_title = (!label.isNull() && label.name().length()) ?
	    label.name() : prev_title;

	if ((block_end > selection_left) && (block_start <= selection_right)) {
	    BlockInfo block;

	    // init and set reasonable defaults
	    block.m_index    = index;
	    block.m_filename = QString();
	    block.m_start    = block_start;
	    block.m_length   = block_end - block_start;
	    block.m_title    = block_title;
	    block.m_artist   = file_artist;

	    // detect title and artist
	    detectBlockMetaData(block_title, m_pattern, block);
	    m_block_info.append(block);

	    prev_title = block.m_title;

// 	    qDebug("#%d [%llu...%llu]", index, block_start, block_end);
// 	    qDebug("    title  = '%s'", DBG(block.m_title));
// 	    qDebug("    artist = '%s'", DBG(block.m_artist));
	}

	if (label.isNull()) break;
	label = (it.hasNext()) ? it.next() : Kwave::Label();
    }
}

//***************************************************************************
QString Kwave::K3BExportPlugin::createFileName(const QString &pattern,
                                               unsigned int index)
{
    QString name = pattern;
    QString num  = _("%1").arg(index, OUTFILE_DIGITS, 10, QLatin1Char('0'));
    name.replace(OUTFILE_PATTERN, num);
    name += OUTFILE_SUFFIX;
    return name;
}

//***************************************************************************
bool Kwave::K3BExportPlugin::detectBlockMetaData(
    const QString &text,
    const QString &pattern,
    Kwave::K3BExportPlugin::BlockInfo &block
)
{
    if (!pattern.length()) {
	// auto detect -> try all known patterns
	foreach (const QString &p, knownPatterns())
	    if (detectBlockMetaData(text, p, block))
		return true;
	return false;
    }

    // list of placeholders and pointers to the resulting strings
    QMap <QString, QString *> map_patterns;
    map_patterns.insert(_("[%artist]"), &block.m_artist);
    map_patterns.insert(_("[%title]"),  &block.m_title);

    // try to find the placeholders within the pattern
    // NOTE: we use a map because it will automatically be sorted (by pos)
    QString pattern_esc = Kwave::Parser::escape(pattern);
    QMap <int, QString *>   map_result;
    foreach (const QString &placeholder, map_patterns.keys()) {
	QString placeholder_esc;
	placeholder_esc = Kwave::Parser::escape(placeholder);
	if (pattern_esc.contains(placeholder_esc)) {
	    const QString rx_string = _("(.+)");
	    int pos = pattern.indexOf(placeholder);
	    pattern_esc.replace(placeholder_esc, rx_string);
	    map_result.insert(pos, map_patterns[placeholder]);
	}
    }
    if (map_result.isEmpty())
	return false; // no placeholders found in the patterns

    // relax the pattern: turn single whitespace to one or more whitespaces
    pattern_esc.replace(QRegExp(_("(\\\\\\s)+")), _("\\s+"));

    // try to match the pattern on the given text
    QRegExp rx(pattern_esc, Qt::CaseInsensitive);
    if (!rx.exactMatch(text.trimmed()))
	return false; // does not match :-(

    // we found a match
    // -> now map the results into the corresponding result strings
    for (int index = 0; index < map_result.count(); ++index) {
	QString value = rx.cap(index + 1).trimmed();
	if (value.length()) {
	    QString *result = map_result[map_result.keys()[index]];
	    if (result) *result = value;
	}
    }

    return true;
}

//***************************************************************************
void Kwave::K3BExportPlugin::load(QStringList &params)
{
    Q_UNUSED(params);

    QString menu_path = _("File/Save/%1").arg(i18nc(
	"menu: /File/Save/Export to K3B Project...",
	"Export to K3B Project..."
    ));
    emitCommand(_("menu(plugin:setup(export_k3b),%1%2)").arg(
	menu_path).arg(_("/#group(@SIGNAL)")));
    emitCommand(_("menu(plugin:setup(export_k3b),%1%2)").arg(
	menu_path).arg(_("/#icon(application-x-k3b)")));
}

//***************************************************************************
QStringList *Kwave::K3BExportPlugin::setup(QStringList &params)
{
    // try to interpret the previous parameters
    interpreteParameters(params);

    sample_index_t selection_left  = 0;
    sample_index_t selection_right = 0;
    selection(0, &selection_left, &selection_right, false);

    // enable the "selection only" checkbox only if there is something
    // selected but not everything
    bool selected_something = (selection_left != selection_right);
    bool selected_all = ((selection_left == 0) &&
        (selection_right + 1 >= signalLength()));
    bool enable_selection_only = selected_something && !selected_all;

    // show a "File / Save As..." dialog for the *.k3b file
    QPointer<Kwave::K3BExportDialog> dialog =
	new(std::nothrow) Kwave::K3BExportDialog(
	    _("kfiledialog:///kwave_export_k3b"),
	    K3B_FILE_SUFFIX + _("|") + i18nc(
		"file type filter when exporting to K3B",
		"K3B project file (*.k3b)"
	    ),
	    parentWidget(),
	    QUrl::fromUserInput(signalName()),
	    _("*.k3b"),
	    m_pattern,
	    m_selection_only,
	    enable_selection_only,
	    m_export_location,
	    m_overwrite_policy
	);
    if (!dialog) return 0;

    dialog->setWindowTitle(description());
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return 0;
    }

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (!list) {
	delete dialog;
	return 0;
    }

    // user has pressed "OK"
    QUrl url = dialog->selectedUrl();
    if (url.isEmpty()) {
	delete dialog;
	delete list;
	return 0;
    }

    QString name = url.path();
    QFileInfo path(name);

    // add the correct extension if necessary
    if (path.suffix() != K3B_FILE_SUFFIX.mid(2))
	url.setPath(name + K3B_FILE_SUFFIX.mid(1));

    name                 = Kwave::Parser::escape(url.toString());
    QString pattern      = Kwave::Parser::escape(dialog->pattern());
    int export_location  = static_cast<int>(dialog->exportLocation());
    int overwrite_policy = static_cast<int>(dialog->overwritePolicy());
    bool selection_only  = (enable_selection_only) ?
        dialog->selectionOnly() : m_selection_only;

    *list << name;                              // url
    *list << pattern;                           // pattern
    *list << QString::number(selection_only);   // selection only
    *list << QString::number(export_location);  // export location
    *list << QString::number(overwrite_policy); // overwrite policy

    emitCommand(_("plugin:execute(export_k3b,") +
	name + _(",") + pattern + _(",")  +
	QString::number(selection_only)   + _(",") +
	QString::number(export_location)  + _(",") +
	QString::number(overwrite_policy) + _(")")
    );

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
/*
 * taken from K3B, libk3b/projects/k3bdoc.cpp
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 *
 */
void Kwave::K3BExportPlugin::saveGeneralDocumentData(QDomElement *part)
{
    QDomDocument doc = part->ownerDocument();
    QDomElement mainElem = doc.createElement(_("general"));

    QDomElement propElem = doc.createElement(_("writing_mode"));
    propElem.appendChild(doc.createTextNode(_("auto")));
    mainElem.appendChild(propElem);

    propElem = doc.createElement(_("dummy"));
    propElem.setAttribute(_("activated"), _("no"));
    mainElem.appendChild(propElem);

    propElem = doc.createElement(_("on_the_fly"));
    propElem.setAttribute(_("activated"), _("true"));
    mainElem.appendChild(propElem);

    propElem = doc.createElement(_("only_create_images"));
    propElem.setAttribute(_("activated"), _("no"));
    mainElem.appendChild(propElem);

    propElem = doc.createElement(_("remove_images"));
    propElem.setAttribute(_("activated"), _("no"));
    mainElem.appendChild(propElem);

    part->appendChild( mainElem );
}

//***************************************************************************
/*
 * taken from K3B, libk3b/projects/audiocd/k3baudiodoc.cpp
 *
 * Copyright (C) 2003-2008 Sebastian Trueg <trueg@k3b.org>
 * Copyright (C)      2009 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (C)      2010 Michal Malek <michalm@jabster.pl>
 */
void Kwave::K3BExportPlugin::saveDocumentData(QDomElement *docElem)
{
    #define GET_INF(inf) doc.createTextNode(info.get(inf).toString())

    const Kwave::FileInfo info(signalManager().metaData());

    QDomDocument doc = docElem->ownerDocument();
    saveGeneralDocumentData(docElem);

    // add normalize
    QDomElement normalizeElem = doc.createElement(_("normalize"));
    normalizeElem.appendChild(doc.createTextNode(_("no")));
    docElem->appendChild(normalizeElem);

    // add hide track
    QDomElement hideFirstTrackElem = doc.createElement(_("hide_first_track"));
    hideFirstTrackElem.appendChild(doc.createTextNode(_("no")));
    docElem->appendChild(hideFirstTrackElem);

    // save the audio cd ripping settings
    // paranoia mode, read retries, and ignore read errors
    // ------------------------------------------------------------
    QDomElement ripMain = doc.createElement(_("audio_ripping"));
    docElem->appendChild(ripMain);

    QDomElement ripElem = doc.createElement(_("paranoia_mode"));
    ripElem.appendChild(doc.createTextNode(_("0")));
    ripMain.appendChild(ripElem);

    ripElem = doc.createElement(_("read_retries"));
    ripElem.appendChild(doc.createTextNode(_("0")));
    ripMain.appendChild(ripElem);

    ripElem = doc.createElement(_("ignore_read_errors"));
    ripElem.appendChild(doc.createTextNode(_("no")));
    ripMain.appendChild(ripElem);
    // ------------------------------------------------------------

    // save disc cd-text
    // -------------------------------------------------------------
    QDomElement cdTextMain = doc.createElement(_("cd-text"));
    cdTextMain.setAttribute(_("activated"), _("yes"));

    QDomElement cdTextElem = doc.createElement(_("title"));
    cdTextElem.appendChild(GET_INF(INF_NAME));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("artist"));
    cdTextElem.appendChild(GET_INF(INF_AUTHOR));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("arranger"));
    cdTextElem.appendChild(GET_INF(INF_TECHNICAN));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("songwriter"));
    cdTextElem.appendChild(GET_INF(INF_PERFORMER));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("composer"));
    cdTextElem.appendChild(GET_INF(INF_ORGANIZATION));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("disc_id"));
    cdTextElem.appendChild(GET_INF(INF_CD));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("upc_ean"));
    cdTextElem.appendChild(GET_INF(INF_ISRC));
    cdTextMain.appendChild(cdTextElem);

    cdTextElem = doc.createElement(_("message"));
    cdTextElem.appendChild(GET_INF(INF_COMMENTS));
    cdTextMain.appendChild(cdTextElem);

    docElem->appendChild( cdTextMain );
    // -------------------------------------------------------------

    // save the tracks
    // -------------------------------------------------------------
    QDomElement contentsElem = doc.createElement(_("contents"));

    unsigned int index = 1;
    foreach (const Kwave::K3BExportPlugin::BlockInfo &block, m_block_info) {
	QString title      = block.m_title;
	QString artist     = block.m_artist;
	QString songwriter = QString();
	QString url        = block.m_filename;

	QDomElement trackElem = doc.createElement(_("track"));

	// add sources
	QDomElement sourcesParent = doc.createElement(_("sources"));
	QDomElement sourceElem = doc.createElement(_("file"));
	sourceElem.setAttribute(_("url"), url);
	sourceElem.setAttribute(_("start_offset"), _("00:00:00"));
	sourceElem.setAttribute(_("end_offset"), _("00:00:00"));
	sourcesParent.appendChild(sourceElem);
	trackElem.appendChild(sourcesParent);

	// index 0
	QDomElement index0Elem = doc.createElement(_("index0"));
	index0Elem.appendChild(doc.createTextNode(QString::number(index)));
	trackElem.appendChild(index0Elem);

	// add cd-text
	cdTextMain = doc.createElement(_("cd-text"));
	cdTextElem = doc.createElement(_("title"));
	cdTextElem.appendChild(doc.createTextNode(title));
	cdTextMain.appendChild(cdTextElem);

	cdTextElem = doc.createElement(_("artist"));
	cdTextElem.appendChild(doc.createTextNode(artist));
	cdTextMain.appendChild(cdTextElem);

	cdTextElem = doc.createElement(_("arranger"));
	cdTextElem.appendChild(GET_INF(INF_TECHNICAN));
	cdTextMain.appendChild(cdTextElem);

	cdTextElem = doc.createElement(_("songwriter"));
	cdTextElem.appendChild(doc.createTextNode(songwriter));
	cdTextMain.appendChild(cdTextElem );

	cdTextElem = doc.createElement(_("composer"));
	cdTextElem.appendChild(GET_INF(INF_ORGANIZATION));
	cdTextMain.appendChild(cdTextElem);

	cdTextElem = doc.createElement(_("isrc"));
	cdTextElem.appendChild(GET_INF(INF_ISRC));
	cdTextMain.appendChild(cdTextElem);

	cdTextElem = doc.createElement(_("message"));
	cdTextElem.appendChild(GET_INF(INF_COMMENTS));
	cdTextMain.appendChild(cdTextElem);

	trackElem.appendChild(cdTextMain);

	// add copy protection
	QDomElement copyElem = doc.createElement(_("copy_protection"));
	copyElem.appendChild(doc.createTextNode(
	    info.get(INF_COPYRIGHTED).toInt() ? _("yes") : _("no")
	));
	trackElem.appendChild(copyElem);

	// add pre emphasis
	copyElem = doc.createElement(_("pre_emphasis"));
	copyElem.appendChild(doc.createTextNode(_("no")));
	trackElem.appendChild(copyElem);

	contentsElem.appendChild(trackElem);
	index++;
    }
    // -------------------------------------------------------------

    docElem->appendChild(contentsElem);
}

//***************************************************************************
int Kwave::K3BExportPlugin::start(QStringList &params)
{
    qDebug("K3BExportPlugin::start()");

    // interpret the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // check the output file
    if (!m_url.isLocalFile())
	return -EINVAL; // sorry, KZip supports only local files

    // determine output directory and file name pattern
    QString   k3b_filename = m_url.path();
    QFileInfo fi(k3b_filename);
    QString   base = fi.completeBaseName();
    QString   out_dir;
    QString   out_pattern;
    if (m_export_location == Kwave::K3BExportPlugin::EXPORT_TO_SUB_DIR) {
	// export to a subdir with the name "<filename>.dir"
	out_dir     = fi.absolutePath() + QDir::separator() + base + _(".dir");
	out_pattern = _("track-") + OUTFILE_PATTERN;
    } else {
	// use the same directory as the *.k3b file
	out_dir     = fi.absolutePath();
	out_pattern = base + _("-track-") + OUTFILE_PATTERN;
    }
    qDebug("out_dir     = '%s'", DBG(out_dir));
    qDebug("out_pattern = '%s'", DBG(out_pattern));

    // determine the selection settings
    sample_index_t selection_left  = 0;
    sample_index_t selection_right = 0;
    QList<unsigned int> tracks;
    selection(&tracks, &selection_left, &selection_right, false);

    // check: only mono or stereo files are supported
    if ((tracks.count() != 1) && (tracks.count() != 2)) {
	qWarning("sorry, K3B can not handle %u tracks", tracks.count());
	Kwave::MessageBox::sorry(parentWidget(), i18n(
	    "Only mono and stereo files can be used for an audio CD. "
	    "You can either deselect some channels or export the file "
	    "in a different file format that supports mono and stereo "
	    "only (for example FLAC) and then try again."
	));
	return -EINVAL;
    }

    bool selected_something = (selection_left != selection_right);
    bool selected_all = ( (selection_left == 0) &&
                          ((selection_right + 1) >= signalLength()) );
    bool enable_selection_only = selected_something && !selected_all;
    bool selection_only = enable_selection_only && m_selection_only;
    if (!selection_only) {
	selection_left  = 0;
	selection_right = signalLength() - 1;
    }

    // create a list of blocks to save, but not yet the output file names
    scanBlocksToSave(base, selection_left, selection_right);
    unsigned int count = m_block_info.count();
    if (!count)
	return -EINVAL;

    // find the start index of the file numbering
    unsigned int first = 1;
    if (m_overwrite_policy == Kwave::K3BExportPlugin::USE_NEW_FILE_NAMES) {
	// use new files, find out the highest existing index
	QString pat = out_pattern;
	pat.replace(OUTFILE_PATTERN, _("*"));
	pat += OUTFILE_SUFFIX;

	QDir dir(out_dir, pat);
	QStringList files;
	files = dir.entryList();

	for (unsigned int i = first; i < (first + count); ++i) {
	    QString name = createFileName(out_pattern, i);
	    QRegExp rx(_("^(") + name + _(")$"), Qt::CaseInsensitive);
	    QStringList matches = files.filter(rx);
	    if (matches.count() > 0) first = i + 1;
	}
	qDebug("found first usable index -> %d", first);
    } else {
	// overwrite mode, always start at 1
    }

    // create the complete file names
    for (unsigned int i = 0; i < count; ++i) {
	m_block_info[i].m_filename = out_dir + QDir::separator() +
	    createFileName(out_pattern, first + i);
    }

    result = saveBlocks(selection_only, out_dir, out_pattern);
    if (result != 0)
	return result; // aborted or failed -> do not create a k3b file

    result = saveK3BFile(k3b_filename);
    if (result != 0)
	return result; // aborted or failed -> do not ask about starting k3b

    if (Kwave::MessageBox::questionYesNo(parentWidget(), i18n(
	"A K3B project file has been created and audio files have "
	"been exported.\n"
	"Should I start K3B and open the audio CD project now?"
    )) == KMessageBox::Yes) {
	// call k3b and pass the project file name (must be full path)
	QStringList args;
	args << k3b_filename;
	if (!QProcess::startDetached(_("k3b"), args)) {
	    return -EIO;
	}
    }

    return result;
}

//***************************************************************************
int Kwave::K3BExportPlugin::saveBlocks(bool selection_only,
                                       const QString &out_dir,
                                       const QString &out_pattern)
{
    QString first_filename = Kwave::Parser::escapeForFileName(
	QUrl::fromLocalFile(createFileName(out_pattern, 1)).toString());

    // remember the original file info remove all unsupported/ properties,
    // to avoid that the saveblocks plugin complains...
    const Kwave::FileInfo orig_file_info(signalManager().metaData());
    Kwave::FileInfo file_info(orig_file_info);
    QList<Kwave::FileProperty> unsupported_properties;
    {
	QString mimetype = Kwave::CodecManager::whatContains(m_url);
	Kwave::Encoder *encoder = Kwave::CodecManager::encoder(mimetype);
	if (encoder) {
	    unsupported_properties = encoder->unsupportedProperties(
		file_info.properties().keys());
	    delete encoder;
	}
	if (!unsupported_properties.isEmpty()) {
	    foreach (const Kwave::FileProperty &p, unsupported_properties) {
		file_info.set(p, QVariant());
	    }
	}
    }

    // make sure that the file uses 16 bits/sample only
    file_info.setBits(16);

    signalManager().metaData().replace(Kwave::MetaDataList(file_info));

    // call the saveblocks plugin and let it do the main work of exporting
    // the *.wav files with all the tracks...

    QStringList params;
    params << out_dir + QDir::separator() + first_filename;
    params << Kwave::Parser::escape(out_pattern);
    params << ((m_overwrite_policy == USE_NEW_FILE_NAMES) ? _("0") : _("1"));
    params << (selection_only ? _("1") : _("0"));
    int result = manager().executePlugin(_("saveblocks"), &params);

    // restore the original file info
    signalManager().metaData().replace(Kwave::MetaDataList(orig_file_info));

    return result;
}

//***************************************************************************
int Kwave::K3BExportPlugin::saveK3BFile(const QString &k3b_filename)
{
    // create the K3B file
    KZip zip(k3b_filename);

    bool ok = zip.open(QIODevice::WriteOnly);
    if (!ok) return -EIO;

    // write the mime type
    QByteArray app_type(K3B_PROJECT_MIME_TYPE);
    zip.setCompression(KZip::NoCompression);
    zip.setExtraField(KZip::NoExtraField);
    zip.writeFile(_("mimetype"), app_type);

    // export file global data
    QByteArray xml;
    QBuffer out(&xml);
    out.open(QIODevice::WriteOnly);

    // save the data in the document
    QDomDocument xmlDoc(_("k3b_audio_project"));

    xmlDoc.appendChild(xmlDoc.createProcessingInstruction(
	_("xml"), _("version=\"1.0\" encoding=\"UTF-8\"")
    ));
    QDomElement docElem = xmlDoc.createElement(_("k3b_audio_project"));
    xmlDoc.appendChild(docElem);
    saveDocumentData(&docElem);
    QTextStream xmlStream(&out);
    xmlDoc.save(xmlStream, 0);

    out.close();

    zip.setCompression(KZip::NoCompression);
    zip.setExtraField(KZip::NoExtraField);
    zip.writeFile(_("maindata.xml"), xml.data());
    zip.close();

    return 0;
}

//***************************************************************************
QStringList Kwave::K3BExportPlugin::knownPatterns()
{
    // list of all known detection patterns
    QStringList patterns;
    patterns << _("[%title] ([%artist])");
    patterns << _("[%title], [%artist]");
    patterns << _("[%artist]: [%title]");
    patterns << _("[%artist] - [%title]");
    return patterns;
}

//***************************************************************************
#include "K3BExportPlugin.moc"
//***************************************************************************
//***************************************************************************
