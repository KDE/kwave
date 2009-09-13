/***************************************************************************
     SaveBlocksWidget.h  -  widget for extra options in the file open dialog
                             -------------------
    begin                : Fri Mar 02 2007
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

#ifndef _SAVE_BLOCKS_WIDGET_H_
#define _SAVE_BLOCKS_WIDGET_H_

#include <QWidget>

#include "SaveBlocksPlugin.h"
#include "ui_SaveBlocksWidgetBase.h"

class SaveBlocksWidget: public QWidget,
                        public Ui::SaveBlocksWidgetBase
{
    Q_OBJECT
public:

    /**
     * Constructor
     * @param widget pointer to the parent widget
     * @param filename_pattern the pattern used for generating the file names
     * @param numbering_mode the way the numbers are given
     * @param selection_only if true, save only the selection
     * @param have_selection if true, there is a selection
     */
    SaveBlocksWidget(QWidget *widget,
	QString filename_pattern,
	SaveBlocksPlugin::numbering_mode_t numbering_mode,
	bool selection_only,
	bool have_selection
    );

    /** Destructor */
    virtual ~SaveBlocksWidget();

    /** @see KPreviewWidgetBase::showPreview() */
    virtual void showPreview(const KUrl &url)
    {
	Q_UNUSED(url);
    };

    /** @see KPreviewWidgetBase::clearPreview */
    virtual void clearPreview()
    {
    };

    /** returns the file name pattern */
    QString pattern();

    /** returns the numbering mode */
    SaveBlocksPlugin::numbering_mode_t numberingMode();

    /** returns true if only the selection should be saved */
    bool selectionOnly();

signals:

    /** emitted whenever one of the input controls has changed */
    void somethingChanged();

public slots:

    /**
     * update the filename preview
     * @param example the example filename
     */
    void setNewExample(const QString &example);

private slots:

    /** calls somethingChanged() and ignores it's parameter */
    void textChanged(const QString &);

    /** calls somethingChanged() and ignores it's parameter */
    void indexChanged(int);

};

#endif /* _SAVE_BLOCKS_WIDGET_H_ */
