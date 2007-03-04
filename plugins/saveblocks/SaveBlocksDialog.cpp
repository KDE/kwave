/***************************************************************************
   SaveBlocksDialog.cpp  -  Extended KwaveFileDialog for saving blocks
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
#include "SaveBlocksDialog.h"
#include "SaveBlocksWidget.h"

//***************************************************************************
SaveBlocksDialog::SaveBlocksDialog(const QString &startDir,
    const QString &filter,
    QWidget *parent,
    const char *name,
    bool modal,
    const QString last_url,
    const QString last_ext,
    QString filename_pattern,
    SaveBlocksPlugin::numbering_mode_t numbering_mode,
    bool selection_only,
    bool have_selection
)
    :KwaveFileDialog(startDir, filter, parent, name, modal, last_url,
	last_ext, m_widget = new SaveBlocksWidget(parent, filename_pattern,
	numbering_mode, selection_only, have_selection))
{
}

//***************************************************************************
QString SaveBlocksDialog::pattern()
{
    return (m_widget) ? m_widget->pattern() : "";
}

//***************************************************************************
SaveBlocksPlugin::numbering_mode_t SaveBlocksDialog::numberingMode()
{
    return (m_widget) ? m_widget->numberingMode() :
                        SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
bool SaveBlocksDialog::selectionOnly()
{
    return (m_widget) ? m_widget->selectionOnly() : false;
}

//***************************************************************************
//***************************************************************************
