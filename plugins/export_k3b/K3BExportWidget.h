/***************************************************************************
 * K3BExportWidget.h  -  widget for K3b export options in the file open dlg
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

#ifndef K3B_EXPORT_WIDGET_H
#define K3B_EXPORT_WIDGET_H

#include <QWidget>

#include "K3BExportPlugin.h"
#include "ui_K3BExportWidgetBase.h"

namespace Kwave
{
    class K3BExportWidget: public QWidget, public Ui::K3BExportWidgetBase
    {
	Q_OBJECT
    public:

	/**
	 * Constructor
	 * @param widget pointer to the parent widget
	 * @param pattern the pattern used for detecting title and artist
	 * @param selection_only if true, save only the selection
	 * @param have_selection if true, there is a selection
	 * @param export_location where to export files with tracks
	 * @param overwrite_policy overwrite existing files or use a new name
	 */
	K3BExportWidget(
	    QWidget *widget,
	    QString &pattern,
	    bool selection_only,
	    bool have_selection,
	    Kwave::K3BExportPlugin::export_location_t export_location,
	    Kwave::K3BExportPlugin::overwrite_policy_t overwrite_policy
	);

	/** Destructor */
	virtual ~K3BExportWidget();

	/** @see KPreviewWidgetBase::showPreview() */
	virtual void showPreview(const QUrl &url)
	{
	    Q_UNUSED(url);
	}

	/** @see KPreviewWidgetBase::clearPreview */
	virtual void clearPreview()
	{
	}

	/** returns the title/artist detection pattern (as is, not escaped) */
	QString pattern() const;

	/** returns true if only the selection should be saved */
	bool selectionOnly() const;

	/** returns export location of the files of the tracks */
	Kwave::K3BExportPlugin::export_location_t exportLocation() const;

	/** returns the file overwrite policy */
	Kwave::K3BExportPlugin::overwrite_policy_t overwritePolicy() const;

    };
}

#endif /* K3B_EXPORT_WIDGET_H */

//***************************************************************************
//***************************************************************************
