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
#include "errno.h"
#include <qstringlist.h>
#include <klocale.h>
#include <kmdcodec.h>

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
        (mode != START_AT_ZERO) &&
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
//***************************************************************************
