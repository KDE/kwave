/***************************************************************************
    TrackView.h  -  signal views that shows the track in time space
                             -------------------
    begin                : Sat Jan 30 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "config.h"
#include "libkwavegui_export.h"

#include <QtGlobal>
#include <QImage>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QWidget>

#include "libkwave/Sample.h"

#include "libgui/SignalView.h"
#include "libgui/TrackPixmap.h"

class QPaintEvent;
class QResizeEvent;

namespace Kwave
{

    class SignalManager; // forward declaration
    class Track;

    class LIBKWAVEGUI_EXPORT TrackView: public SignalView
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param parent pointer to the parent widget
         * @param controls container widget for associated controls
         * @param signal_manager the signal manager
         * @param track the track object this view is bound to
         */
        TrackView(QWidget *parent, QWidget *controls,
                  Kwave::SignalManager *signal_manager,
                  Kwave::Track *track);

        /** Destructor */
        ~TrackView() override;

        /** refreshes the bitmap that displays the signal */
        void refresh() override;

        /**
         * sets new zoom factor and offset
         * @param zoom the new zoom factor in pixels/sample
         * @param offset the index of the first visible sample
         */
        virtual void setZoomAndOffset(double zoom, sample_index_t offset)
            override;

        /**
         * sets new vertical zoom factor
         * @param zoom vertical zoom factor
         */
        void setVerticalZoom(double zoom) override;

        /** called when the widget has been resized */
        void resizeEvent(QResizeEvent *event) override;

        /** slot for repainting the widget or portions of it */
        void paintEvent(QPaintEvent *) override;

        /**
         * Should be overwritten by subclasses that can display the currently
         * selected range and allow the user to change the selection by mouse.
         * @return true if mouse selection is handled
         */
        bool canHandleSelection() const override { return true; }

        /**
         * Tries to find the nearest item that is visible in this view
         * at a given position
         *
         * @param pos position to look at, relative to view [pixels]
         * @return the nearest ViewObject in range
         *         or a null pointer if nothing found
         */
        virtual QSharedPointer<Kwave::ViewItem> findItem(const QPoint &pos)
            override;

        /**
         * Called when the context menu has been activated over this view
         * @param pos a position in pixel within this widget
         * @param menu pointer to the context menu
         */
        virtual void handleContextMenu(const QPoint &pos, QMenu *menu)
            override;

    public slots:

        /**
         * requests a repaint, as soon as the repaint timer elapsed
         * @param pos current position of the cursor
         */
        virtual void showCursor(sample_index_t pos = SAMPLE_INDEX_MAX)
            override;

    private slots:

        /** requests a refresh of the signal layer */
        void refreshSignalLayer();

        /** requests a refresh of the selection layer */
        void refreshSelectionLayer();

        /** requests a refresh of the markers layer */
        void refreshMarkersLayer();

        /** requests a refresh of all layers */
        void refreshAllLayers();

        /** context menu: "label / new" */
        void contextMenuLabelNew();

    private:

        /** the track pixmap */
        Kwave::TrackPixmap m_pixmap;

        /** last/previous width of the widget, for detecting size changes */
        int m_last_width;

        /** last/previous height of the widget, for detecting size changes */
        int m_last_height;

        /** QImage used for composition */
        QImage m_image;

        /** QImage used for the signal layer */
        QImage m_img_signal;

        /** QImage used for the selection layer */
        QImage m_img_selection;

        /** QImage used for the markers/labels layer */
        QImage m_img_markers;

        /** if true, the signal layer needs to be refreshed */
        bool m_img_signal_needs_refresh;

        /** if true, the selection layer needs to be refreshed */
        bool m_img_selection_needs_refresh;

        /** if true, the markers layer needs to be refreshed */
        bool m_img_markers_needs_refresh;

        /** position of the last mouse click in samples */
        sample_index_t m_mouse_click_position;

        /** last known position of the playback pointer */
        sample_index_t m_cursor_pos;
    };
}

#endif /* TRACK_VIEW_H */

//***************************************************************************
//***************************************************************************
