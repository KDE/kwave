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

#include <QComboBox>
#include <QString>

#include <kabstractfilewidget.h>
#include <kfiledialog.h>
#include <kurlcombobox.h>

#include "SaveBlocksDialog.h"
#include "SaveBlocksWidget.h"

//***************************************************************************
SaveBlocksDialog::SaveBlocksDialog(const QString &startDir,
    const QString &filter,
    QWidget *parent,
    bool modal,
    const QString last_url,
    const QString last_ext,
    QString filename_pattern,
    SaveBlocksPlugin::numbering_mode_t numbering_mode,
    bool selection_only,
    bool have_selection
)
    :KwaveFileDialog(startDir, filter, parent, modal, last_url, last_ext),
     m_widget(new SaveBlocksWidget(this, filename_pattern,
	numbering_mode, selection_only, have_selection))
{
    Q_ASSERT(m_widget);
    fileWidget()->setCustomWidget(m_widget);
    connect(m_widget, SIGNAL(somethingChanged()),
            this, SLOT(emitUpdate()));

    // if something in the file selection changes
    connect(this, SIGNAL(filterChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
    connect(locationEdit(), SIGNAL(textChanged(const QString &)),
            this, SLOT(textChanged(const QString &)));
}

//***************************************************************************
SaveBlocksDialog::~SaveBlocksDialog()
{
    if (m_widget) delete m_widget;
    m_widget = 0;
}

//***************************************************************************
QString SaveBlocksDialog::pattern()
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->pattern() : "";
}

//***************************************************************************
SaveBlocksPlugin::numbering_mode_t SaveBlocksDialog::numberingMode()
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->numberingMode() :
                        SaveBlocksPlugin::CONTINUE;
}

//***************************************************************************
bool SaveBlocksDialog::selectionOnly()
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->selectionOnly() : false;
}

//***************************************************************************
void SaveBlocksDialog::setNewExample(const QString &example)
{
    Q_ASSERT(m_widget);
    if (m_widget) m_widget->setNewExample(example);
}

//***************************************************************************
void SaveBlocksDialog::emitUpdate()
{
    QString path = baseUrl().path(KUrl::AddTrailingSlash);
    QString filename = path + locationEdit()->currentText();
    QFileInfo file(filename);
    if (!file.suffix().length()) {
	// append the currently selected extension if it's missing
	QString extension = selectedExtension().section(" ", 0, 0);
	filename += extension.remove(0, 1);
    }
    emit sigSelectionChanged(filename, pattern(),
	numberingMode(), selectionOnly());
}

//***************************************************************************
void SaveBlocksDialog::textChanged(const QString &)
{
    emitUpdate();
}

//***************************************************************************
#include "SaveBlocksDialog.moc"
//***************************************************************************
//***************************************************************************
