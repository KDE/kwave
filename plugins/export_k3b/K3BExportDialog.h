/***************************************************************************
 *      K3BExportDialog.h  -  Extended KwaveFileDialog for exporting to K3b
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

#ifndef K3B_EXPORT_DIALOG_H
#define K3B_EXPORT_DIALOG_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QWidget>

#include "K3BExportPlugin.h"
#include "libgui/FileDialog.h"

class QUrl;

namespace Kwave
{

    class K3BExportWidget;

    class K3BExportDialog: public Kwave::FileDialog
    {
	Q_OBJECT
    public:

	/**
	 * Constructor.
	 * @see KFileFialog
	 * @param startDir the start directory
	 * @param filter string with a file type filter
	 * @param parent the parent widget
	 * @param last_url the last used URL
	 * @param last_ext the last used extension (preset only)
	 * @param pattern the pattern used for detecting title and artist
	 * @param selection_only if true, save only the selection
	 * @param have_selection if true, there is a selection
	 * @param export_location where to export files with tracks
	 * @param overwrite_policy overwrite existing files or use a new name
	 */
	K3BExportDialog(
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
	);

	/** Destructor */
	virtual ~K3BExportDialog();

	/** returns the title/artist detection pattern (as is, not escaped) */
	QString pattern() const;

	/** returns true if only the selection should be saved */
	bool selectionOnly() const;

	/** returns export location of the files of the tracks */
	Kwave::K3BExportPlugin::export_location_t exportLocation() const;

	/** returns the file overwrite policy */
	Kwave::K3BExportPlugin::overwrite_policy_t overwritePolicy() const;

    private:

	/** the widget with extra settings for K3B export */
	Kwave::K3BExportWidget *m_widget;

    };
}

#endif /* K3B_EXPORT_DIALOG_H */

//***************************************************************************
//***************************************************************************
