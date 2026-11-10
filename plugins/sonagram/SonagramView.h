/***************************************************************************
         SonagramView.h  -  signal view for a sonagram
                             -------------------
    begin                : Mon Oct 12 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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
#ifndef SONAGRAM_VIEW_H
#define SONAGRAM_VIEW_H

#include "config.h"

#include <array>

#include <QColor>
#include <QImage>
#include <QTimer>
#include <QWidget>

#include "libgui/SignalView.h"
#include "SonagramImage.h"
#include "SonagramPlugin.h"

class QResizeEvent;

namespace Kwave
{
    class ImageView;

    class SonagramView: public Kwave::SignalView
    {
        Q_OBJECT
    public:
        /**
         * Constructor
         * @param parent pointer to the parent widget
         * @param signal_manager the signal manager
         */
        explicit SonagramView(QWidget *parent,
                    QPointer<Kwave::SignalManager> &signal_manager);

        /** Destructor */
        ~SonagramView() override;

        /**
         * Returns the stable name used to find and reuse this dock view.
         * @return the view name
         */
        QString name() const override { return QStringLiteral("sonagram"); }

        /** Notify the current plugin owner that this view is being reused. */
        void prepareForReuse();

        /**
         * Set the color mode used to render new slices.
         * @param mode the color mode to use
         */
        void setColorMode(Kwave::SonagramPlugin::ColorMode mode);

        /**
         * Set the bitmap to display.
         * @param image the bitmap to display
         */
        void setImage(const QImage &image);

        /**
         * Set the number of FFT points.
         * @param fft_points FFT points [samples/pixel]
         */
        void setFftPoints(unsigned int fft_points);

        /**
         * Update the visible image crop with the signal viewport range.
         * @param zoom zoom factor [samples/pixel]
         * @param offset index of the first visible sample
         */
        void setZoomAndOffset(double zoom, sample_index_t offset) override;

        /**
         * Insert one calculated sonagram slice.
         * @param slice_nr horizontal slice index
         * @param slice the calculated FFT data for the slice
         */
        void insertSlice(
            unsigned int slice_nr,
            const std::array<QColor, MAX_FFT_POINTS> &slice);

    signals:

        /** Emitted before another plugin instance takes ownership. */
        void sigAboutToBeReused();

    protected:

        /** Recalculate the visible image crop after a resize. */
        void resizeEvent(QResizeEvent *event) override;

    private:

        /** Refresh the image view after the update timer expires. */
        void refreshView();

    private:

        /** bitmap and palette state */
        Kwave::SonagramImage m_image;

        /** FFT points [samples/pixel] */
        unsigned int m_fft_points;

        /** widget that paints the bitmap without window controls */
        Kwave::ImageView *m_view;

        /** throttle repaint requests while slices are inserted */
        QTimer m_refresh_timer;

    };

}

#endif /* SONAGRAM_VIEW_H */

//***************************************************************************
//***************************************************************************
