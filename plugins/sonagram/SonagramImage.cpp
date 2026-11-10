/***************************************************************************
        SonagramImage.cpp  -  shared sonagram image operations
                             -------------------
    begin                : Mon Oct 26 2026
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

#include "libkwave/Utils.h"

#include "SonagramImage.h"

//***************************************************************************
Kwave::SonagramImage::SonagramImage()
    :QImage(),
     m_color_mode(Kwave::SonagramPlugin::COLOR_MODE_NONE),
     m_histogram{0}
{
}

//***************************************************************************
void Kwave::SonagramImage::setColorMode(
    Kwave::SonagramPlugin::ColorMode color_mode)
{
    m_color_mode = color_mode;
    updateHistogram();
}

//***************************************************************************
void Kwave::SonagramImage::setImage(const QImage &image)
{
    QImage::operator=(image);
    updateHistogram();
}

//***************************************************************************
void Kwave::SonagramImage::resizeWidth(unsigned int new_width)
{
    if (isNull() || !new_width || (new_width ==
            static_cast<unsigned int>(width()))) return;

    QImage resized(static_cast<int>(new_width), height(), format());
    if (resized.isNull()) return;

    const unsigned int old_width = static_cast<unsigned int>(width());
    const unsigned int old_height = static_cast<unsigned int>(height());
    const unsigned int copy_width = qMin(old_width, new_width);

    if (format() == QImage::Format_Indexed8) {
        resized.setColorTable(colorTable());
        resized.fill(0xFF);
        for (unsigned int x = 0; x < copy_width; ++x)
            for (unsigned int y = 0; y < old_height; ++y)
                resized.setPixel(x, y, pixelIndex(x, y));
    } else {
        resized.fill(Qt::transparent);
        for (unsigned int x = 0; x < copy_width; ++x)
            for (unsigned int y = 0; y < old_height; ++y)
                resized.setPixelColor(x, y, pixelColor(x, y));
    }

    setImage(resized);
}

//***************************************************************************
void Kwave::SonagramImage::updateHistogram()
{
    for (unsigned int i = 0; i < 256; ++i)
        m_histogram[i] = 0;

    if (isNull() || (format() != QImage::Format_Indexed8) ||
        (m_color_mode == Kwave::SonagramPlugin::COLOR_MODE_HSV)) return;

    for (int x = 0; x < width(); ++x) {
        for (int y = 0; y < height(); ++y) {
            const unsigned int index =
                static_cast<unsigned int>(pixelIndex(x, y));
            m_histogram[index]++;
        }
    }
}

//***************************************************************************
void Kwave::SonagramImage::insertSlice(
        unsigned int slice_nr, unsigned int slice_height,
        const std::array<QColor, MAX_FFT_POINTS> &slice)
{
    if (isNull() || (slice_nr >= static_cast<unsigned int>(width()))) return;

    const unsigned int image_height = static_cast<unsigned int>(height());
    unsigned int y = 0;
    const unsigned int height = qMin(slice_height, image_height);
    for (; y < height; ++y) {
        const QColor &pixel = slice[(slice_height - 1) - y];
        if (m_color_mode == Kwave::SonagramPlugin::COLOR_MODE_HSV) {
            setPixelColor(slice_nr, y, pixel);
        } else {
            const unsigned int old_index =
                static_cast<unsigned int>(pixelIndex(slice_nr, y));
            m_histogram[old_index]--;

            const unsigned int index =
                static_cast<unsigned int>(pixel.red());
            setPixel(slice_nr, y, static_cast<int>(index));
            m_histogram[index]++;
        }
    }

    while (y < image_height) {
        setPixel(slice_nr, y++, 0xFE);
        m_histogram[0xFE]++;
    }
}

//***************************************************************************
void Kwave::SonagramImage::createPalette()
{
    if (isNull() ||
        (m_color_mode == Kwave::SonagramPlugin::COLOR_MODE_HSV)) return;

    unsigned long int sum = 0;
    for (unsigned int i = 1; i <= 254; ++i)
        sum += m_histogram[i];

    const unsigned int cutoff = Kwave::toUint(
        static_cast<double>(sum) * (0.1 / 100.0));

    int last = 254;
    while ((last >= 0) && (m_histogram[last] <= cutoff))
        --last;

    QColor color;
    for (int i = 0; i < 255; ++i) {
        const int value = (i >= last) ? 254 : ((last - i) * 254) / last;
        if (m_color_mode == Kwave::SonagramPlugin::COLOR_MODE_8BIT)
            color.setHsv(value, 255, 255, 255);
        else
            color.setRgb(value, value, value, 255);

        setColor(i, color.rgba());
    }

    setColor(0xFF, QColor(0, 0, 0, 0).rgba());
}

//***************************************************************************
//***************************************************************************