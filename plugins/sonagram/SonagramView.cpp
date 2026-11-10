/***************************************************************************
       SonagramView.cpp  -  signal view for a sonagram
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

#include "config.h"

#include <new>

#include <QPointer>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "libkwave/SignalManager.h"
#include "libgui/ImageView.h"

#include "SonagramImage.h"
#include "SonagramView.h"

//***************************************************************************
Kwave::SonagramView::SonagramView(QWidget *parent,
        QPointer<Kwave::SignalManager> &signal_manager)
    :Kwave::SignalView(parent, nullptr, signal_manager,
                       Kwave::SignalView::LowerDockTop, -1),
     m_image(),
     m_fft_points(0),
     m_view(new(std::nothrow) Kwave::ImageView(this)),
     m_refresh_timer()
{
    Q_ASSERT(m_view);
    if (!m_view) return;

    setMinimumSize(100, 50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_view->setCursor(Qt::ArrowCursor);
    m_view->setMouseTracking(false);

    QVBoxLayout *layout = new(std::nothrow) QVBoxLayout(this);
    Q_ASSERT(layout);
    if (!layout) return;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_view);

    connect(&m_refresh_timer, &QTimer::timeout,
            this, [this]() { refreshView(); });
}

//***************************************************************************
Kwave::SonagramView::~SonagramView() = default;

//***************************************************************************
void Kwave::SonagramView::prepareForReuse()
{
    emit sigAboutToBeReused();
}

//***************************************************************************
void Kwave::SonagramView::setColorMode(
    Kwave::SonagramPlugin::ColorMode mode)
{
    m_image.setColorMode(mode);
    refreshView();
}

//***************************************************************************
void Kwave::SonagramView::setImage(const QImage &image)
{
    m_image.setImage(image);
    refreshView();
}

//***************************************************************************
void Kwave::SonagramView::setFftPoints(unsigned int fft_points)
{
    m_fft_points = fft_points;
    refreshView();
}

//***************************************************************************
void Kwave::SonagramView::setZoomAndOffset(double zoom,
                                           sample_index_t offset)
{
    Kwave::SignalView::setZoomAndOffset(zoom, offset);
    refreshView();
}

//***************************************************************************
void Kwave::SonagramView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshView();
}

//***************************************************************************
void Kwave::SonagramView::insertSlice(
    unsigned int slice_nr,
    const std::array<QColor, MAX_FFT_POINTS> &slice)
{
    m_image.insertSlice(
        slice_nr, static_cast<unsigned int>(m_image.height()), slice);

    if (!m_refresh_timer.isActive()) {
        m_refresh_timer.setSingleShot(true);
        m_refresh_timer.start(100);
    }
}

//***************************************************************************
void Kwave::SonagramView::refreshView()
{
    if (!m_view) return;
    m_image.createPalette();

    QPointer<Kwave::SignalManager> manager = signalManager();
    const unsigned int image_width =
        static_cast<unsigned int>(m_image.width());
    if (!manager || m_image.isNull() || !image_width || !m_fft_points ||
        !width()) {
        m_view->setImage(QImage());
        return;
    }

    const sample_index_t total_samples = manager->length();
    if (!total_samples) {
        m_view->setImage(QImage());
        return;
    }

    const sample_index_t first_sample = qMin(offset(), total_samples - 1);
    const sample_index_t remaining_samples = total_samples - first_sample;
    const sample_index_t visible_samples = pixels2samples(width());
    const sample_index_t sample_count =
        qMin(visible_samples, remaining_samples);
    const sample_index_t end_sample = first_sample +
        ((sample_count) ? sample_count : 1);

    const unsigned int first_slice = qMin(
        static_cast<unsigned int>(first_sample / m_fft_points),
        image_width - 1);
    unsigned int end_slice = static_cast<unsigned int>(
        end_sample / m_fft_points);
    if (end_sample % m_fft_points) ++end_slice;
    end_slice = qMin(end_slice, image_width);
    if (end_slice <= first_slice) end_slice = first_slice + 1;

    m_view->setImage(m_image.copy(
        static_cast<int>(first_slice), 0,
        static_cast<int>(end_slice - first_slice), m_image.height()));
}

//***************************************************************************
//***************************************************************************
