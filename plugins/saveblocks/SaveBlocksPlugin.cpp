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
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <klocale.h>
#include <kmdcodec.h>

#include "libkwave/FileInfo.h"
#include "libkwave/Label.h"
#include "kwave/CodecManager.h"
#include "SaveBlocksDialog.h"
#include "SaveBlocksPlugin.h"

KWAVE_PLUGIN(SaveBlocksPlugin,"saveblocks","Thomas Eschenbacher");

//***************************************************************************
SaveBlocksPlugin::SaveBlocksPlugin(const PluginContext &c)
    :KwavePlugin(c), m_pattern(), m_numbering_mode(CONTINUE),
     m_selection_only(true)
{
    i18n("saveblocks");
}

//***************************************************************************
SaveBlocksPlugin::~SaveBlocksPlugin()
{
}

//***************************************************************************
QStringList *SaveBlocksPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    KURL url;
    url = signalName();
    unsigned int selection_left  = 0;
    unsigned int selection_right = 0;
    selection(&selection_left, &selection_right, false);

    // enable the "selection only" checkbox only if there is something
    // selected but not everything
    bool enable_selection_only = (selection_left != selection_right) &&
	!((selection_left == 0) || (selection_right+1 >= signalLength()));

    SaveBlocksDialog *dialog = new SaveBlocksDialog(
	":<kwave_save_blocks>", CodecManager::encodingFilter(),
	parentWidget(), "Kwave save blocks", true,
	url.prettyURL(), "*.wav",
	m_pattern,
	m_numbering_mode,
	m_selection_only,
	enable_selection_only
    );
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    // connect the signals/slots from the plugin and the dialog
    connect(dialog, SIGNAL(sigSelectionChanged(const QString &,
	const QString &, SaveBlocksPlugin::numbering_mode_t, bool)),
	this, SLOT(updateExample(const QString &, const QString &,
	SaveBlocksPlugin::numbering_mode_t, bool)));
    connect(this, SIGNAL(sigNewExample(const QString &)),
	dialog, SLOT(setNewExample(const QString &)));

    dialog->setOperationMode(KFileDialog::Saving);
    dialog->setCaption(i18n("Save Blocks"));
    if (dialog->exec() != QDialog::Accepted) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list) {
	// user has pressed "OK"
	QString pattern;
	pattern = KCodecs::base64Encode(QCString(dialog->pattern()), false);
	int mode = static_cast<int>(dialog->numberingMode());
	bool selection_only = (enable_selection_only) ?
	    dialog->selectionOnly() : m_selection_only;

	*list << pattern;
	*list << QString::number(mode);
	*list << QString::number(selection_only);

	emitCommand("plugin:execute(saveblocks,"+
	    pattern+","+
	    QString::number(mode)+","+
	    QString::number(selection_only)+","+
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
int SaveBlocksPlugin::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // ...
    qDebug("SaveBlocksPlugin::start"); // ###

    return result;
}

//***************************************************************************
int SaveBlocksPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 3) {
	return -EINVAL;
    }

    // filename pattern
    m_pattern = KCodecs::base64Decode(QCString(params[0]));
    if (!m_pattern.length()) return -EINVAL;

    // numbering mode
    param = params[1];
    int mode = param.toInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;
    if ((mode != CONTINUE) &&
        (mode != START_AT_ONE)) return -EINVAL;
    m_numbering_mode = static_cast<numbering_mode_t>(mode);

    // flag: save only the selection
    param = params[2];
    m_selection_only = (param.toUInt(&ok) != 0);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
unsigned int SaveBlocksPlugin::blocksToSave(bool selection_only)
{
    unsigned int count = 0;
    unsigned int selection_left, selection_right;

    unsigned int block_start;
    unsigned int block_end = 0;
    LabelListIterator it(fileInfo().labels());
    Label *label = it.current();

    if (selection_only) {
	selection(&selection_left, &selection_right, true);
    } else {
	selection_left = 0;
	selection_right = signalLength() - 1;
    }
    for (;;) {
	block_start = block_end;
	block_end   = (label) ? label->pos() : signalLength();
	if ((selection_left < block_end) && (selection_right > block_start))
	    count++;
	if (!label) break;
	++it;
	label = it.current();
    }

    return count;
}

//***************************************************************************
QString SaveBlocksPlugin::createFileName(const QString &base,
    const QString &ext, const QString &pattern,
    unsigned int index, unsigned int count)
{
    QString p = pattern;
    QString escaped_pattern = QRegExp::escape(p);
    QString nr;

    // format the "index" parameter
    QRegExp rx_nr("(\\[\%\\d*nr\\])", false);
    if (rx_nr.search(p) >= 0) {
	QString format = rx_nr.cap(1);
	format = format.mid(1, format.length() - 4) + "u";
	p.replace(rx_nr, nr.sprintf(format, index));
    }

    // format the "count" parameter
    QRegExp rx_count("(\\[\%\\d*count\\])", false);
    while (rx_count.search(p) >= 0) {
	QString format = rx_count.cap(1);
	format = format.mid(1, format.length() - 7) + "u";
	p.replace(rx_count, nr.sprintf(format, count));
    }

    // format the "filename" parameter
    QRegExp rx_filename("\\[\%filename\\]", false);
    if (rx_filename.search(p) >= 0) {
	p.replace(rx_filename, base);
    }

    if (ext.length()) p += "." + ext;
    return p;
}

//***************************************************************************
QString SaveBlocksPlugin::firstFileName(const QString &filename,
    const QString &pattern, SaveBlocksPlugin::numbering_mode_t mode,
    bool selection_only)
{
    QFileInfo file(filename);
    QString path = file.dirPath(true);
    QString name = file.fileName();
    QString base = file.baseName(true);
    QString ext  = file.extension(false);

    // convert the pattern into a regular expression in order to check if
    // the current name already is produced by the current pattern
    // \[%[0-9]?nr\]      -> \d+
    // \[%[0-9]?count\]   -> \d+
    // \[%filename\]       -> base
    QString escaped_pattern = QRegExp::escape(pattern);
    QRegExp rx_nr("\\[\%\\d*nr\\]", false);
    QRegExp rx_count("\\[\%\\d*count\\]", false);
    QRegExp rx_filename("\\[\%filename\\]", false);

    QString p = pattern;

    int idx_nr = rx_nr.search(p);
    int idx_count = rx_count.search(p);
    int idx_filename = rx_filename.search(p);
    p.replace(rx_nr, "(\\d+)");
    p.replace(rx_count, "(\\d+)");
    p.replace(rx_filename, "(.+)");
    int max = 0;
    for (unsigned int i=0; i < pattern.length(); i++) {
	if (idx_nr       == max) max++;
	if (idx_count    == max) max++;
	if (idx_filename == max) max++;
	if (idx_nr       > max) idx_nr--;
	if (idx_count    > max) idx_count--;
	if (idx_filename > max) idx_filename--;
    }

    p += "." + ext;
    QRegExp rx_current(p, false);
    if (rx_current.search(name) >= 0) {
	// filename already produced by this pattern
	base = rx_current.cap(idx_filename + 1);
    }

    // now we have a new name, base and extension
    // -> find out the numbering, min/max etc...
    unsigned int count = blocksToSave(selection_only);
    unsigned int first = 1;
    QDir dir(path, "*");
    QStringList files;
    files = dir.entryList();
    switch (mode) {
	case START_AT_ONE:
	    first = 1;
	    break;
	case CONTINUE:
	    for (unsigned int i = first; i <= first+count; i++) {
		QString name = createFileName(base, ext, pattern, i, count);
		QRegExp rx("^(" + QRegExp::escape(name) + ")$", false);
		QStringList matches = files.grep(rx);
		if (matches.count() > 0) first = i + 1;
	    }
	    break;
    }

    // create the complete filename, including extension but without path
    return createFileName(base, ext, pattern, first, count);
}

//***************************************************************************
void SaveBlocksPlugin::updateExample(const QString &filename,
    const QString &pattern, SaveBlocksPlugin::numbering_mode_t mode,
    bool selection_only)
{
    emit sigNewExample(firstFileName(filename, pattern, mode, selection_only));
}

//***************************************************************************
//***************************************************************************
