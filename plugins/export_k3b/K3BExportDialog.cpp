/***************************************************************************
 *    K3BExportDialog.cpp  -  Extended KwaveFileDialog for exporting to K3b
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

#include <QString>
#include <QUrl>

#include <KComboBox>
#include <KUrlComboBox>

#include "libkwave/String.h"

#include "K3BExportDialog.h"
#include "K3BExportWidget.h"

//***************************************************************************
Kwave::K3BExportDialog::K3BExportDialog(
    const QString &startDir,
    const QString &filter,
    QWidget *parent,
    const QUrl &last_url,
    const QString &last_ext,
    QString &pattern,
    bool selection_only,
    bool have_selection,
    Kwave::K3BExportPlugin::export_location_t export_location,
    Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy
)
    :Kwave::FileDialog(startDir, Kwave::FileDialog::SaveFile, filter, parent,
                       last_url, last_ext),
     m_widget(new(std::nothrow) Kwave::K3BExportWidget(
	 this, pattern, selection_only, have_selection,
	 export_location, overwrite_policy
     ))
{
    Q_ASSERT(m_widget);
    setCustomWidget(m_widget);
}

//***************************************************************************
Kwave::K3BExportDialog::~K3BExportDialog()
{
    if (m_widget) delete m_widget;
    m_widget = Q_NULLPTR;
}

// //***************************************************************************
QString Kwave::K3BExportDialog::pattern() const
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->pattern() : _("");
}

//***************************************************************************
bool Kwave::K3BExportDialog::selectionOnly() const
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->selectionOnly() : false;
}

//***************************************************************************
Kwave::K3BExportPlugin::export_location_t
    Kwave::K3BExportDialog::exportLocation() const
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->exportLocation() :
                        Kwave::K3BExportPlugin::EXPORT_TO_SUB_DIR;
}

//***************************************************************************
Kwave::K3BExportPlugin::overwrite_policy_t
    Kwave::K3BExportDialog::overwritePolicy() const
{
    Q_ASSERT(m_widget);
    return (m_widget) ? m_widget->overwritePolicy() :
                        Kwave::K3BExportPlugin::USE_NEW_FILE_NAMES;
}

//***************************************************************************
//***************************************************************************
