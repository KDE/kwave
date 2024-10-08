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

#ifndef SAVE_BLOCKS_WIDGET_H
#define SAVE_BLOCKS_WIDGET_H

#include <QWidget>

#include "SaveBlocksPlugin.h"
#include "ui_SaveBlocksWidgetBase.h"

namespace Kwave
{
    class SaveBlocksWidget: public QWidget,
                            public Ui::SaveBlocksWidgetBase
    {
        Q_OBJECT
    public:

        /**
         * Constructor
         * @param widget pointer to the parent widget
         * @param filename_pattern pattern used for generating the file names
         * @param numbering_mode the way the numbers are given
         * @param selection_only if true, save only the selection
         * @param have_selection if true, there is a selection
         */
        SaveBlocksWidget(QWidget *widget,
            QString filename_pattern,
            Kwave::SaveBlocksPlugin::numbering_mode_t numbering_mode,
            bool selection_only,
            bool have_selection
        );

        /** Destructor */
        ~SaveBlocksWidget() override;

        /** @see KPreviewWidgetBase::showPreview() */
        virtual void showPreview(const QUrl &url)
        {
            Q_UNUSED(url)
        }

        /** @see KPreviewWidgetBase::clearPreview */
        virtual void clearPreview()
        {
        }

        /** returns the file name pattern */
        QString pattern();

        /** returns the numbering mode */
        Kwave::SaveBlocksPlugin::numbering_mode_t numberingMode();

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

    };
}

#endif /* SAVE_BLOCKS_WIDGET_H */

//***************************************************************************
//***************************************************************************
