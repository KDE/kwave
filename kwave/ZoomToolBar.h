/***************************************************************************
          ZoomToolBar.h  -  Toolbar for zoom control
			     -------------------
    begin                : 2014-08-12
    copyright            : (C) 2014 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ZOOM_TOOL_BAR_H
#define ZOOM_TOOL_BAR_H

#include "config.h"

#include <QObject>
#include <QTimer>

#include <KToolBar>

#include "libkwave/LabelList.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/Sample.h"

class QAction;
class KComboBox;
class KMainWindow;

namespace Kwave
{

    class FileContext;

    class ZoomToolBar: public KToolBar
    {
	Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param parent a KMainWidget
	 * @param name the name of the toolbar (for config)
	 */
	ZoomToolBar(KMainWindow *parent, const QString &name);

	/** Destructor */
	virtual ~ZoomToolBar();

    signals:

	/** tells this widget's parent to execute a command */
	void sigCommand(const QString &command);

    public slots:

	/** called when the file context has been (updates the toolbar) */
	void contextSwitched(Kwave::FileContext *context);

	/** called when a file context has been deleted */
	void contextDestroyed(Kwave::FileContext *context);

	/**
	 * updates all the toolbar, after change of context and
	 * after meta data change (e.g. signal empty, closed, new, etc...)
	 */
	void updateToolbar();

	/**
	 * Called if a new zoom factor has been set in order to update
	 * the status display and the content of the zoom selection
	 * combo box.
	 * @note This method can not be called to *set* a new zoom factor.
	 * @param context the file context that caused the zoom change
	 * @param zoom the new zoom factor
	 */
	void setZoomInfo(Kwave::FileContext *context, double zoom);

    private slots:

	/** called on changes in the zoom selection combo box */
	void selectZoom(int index);

	/** toolbar: "zoom selection" */
	void zoomSelection() {
	    emit sigCommand(_("view:zoom_selection()"));
	}

	/** toolbar: "zoom in" */
	void zoomIn() {
	    emit sigCommand(_("view:zoom_in()"));
	}

	/** toolbar: "zoom out" */
	void zoomOut() {
	    emit sigCommand(_("view:zoom_out()"));
	}

	/** toolbar: "zoom 1:1" */
	void zoomNormal() {
	    emit sigCommand(_("view:zoom_normal()"));
	}

	/** toolbar: "zoom all" */
	void zoomAll() {
	    emit sigCommand(_("view:zoom_all()"));
	}

    private:

	/** the current file context (could be null) */
	Kwave::FileContext *m_context;

	/** action of the "zoom to selection" toolbar button */
	QAction *m_action_zoomselection;

	/** action of the "zoom in" toolbar button */
	QAction *m_action_zoomin;

	/** action of the "zoom out" toolbar button */
	QAction *m_action_zoomout;

	/** action of the "zoom to 100%" toolbar button */
	QAction *m_action_zoomnormal;

	/** action of the "zoom to all" toolbar button */
	QAction *m_action_zoomall;

	/** action of the "zoom factor" combo box in the toolbar */
	QAction *m_action_zoomselect;

	/** combo box for selection of the zoom factor */
	KComboBox *m_zoomselect;

    };

}

#endif /* ZOOM_TOOL_BAR_H */

//***************************************************************************
//***************************************************************************
