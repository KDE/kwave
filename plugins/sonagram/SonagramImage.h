/***************************************************************************
        SonagramImage.h  -  shared sonagram image operations
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
#ifndef SONAGRAM_IMAGE_H
#define SONAGRAM_IMAGE_H

#include "config.h"

#include <array>

#include <QColor>
#include <QImage>

#include "SonagramPlugin.h"

namespace Kwave
{
    /** Sonagram bitmap with its palette and histogram state. */
    class SonagramImage: public QImage
    {
    public:

        /** Default constructor. */
        SonagramImage();

        /**
         * Set the palette mode used for rendering.
         * @param color_mode the color mode for the image
         */
        void setColorMode(Kwave::SonagramPlugin::ColorMode color_mode);

        /**
         * Assign an image and rebuild the indexed-pixel histogram.
         * @param image the image to assign
         */
        void setImage(const QImage &image);

        /** Resize the image width while preserving overlapping columns. */
        void resizeWidth(unsigned int width);

        /**
         * Insert a slice and update the indexed-pixel histogram.
         * @param slice_nr horizontal slice index
         * @param slice_height number of valid rows in the slice
         * @param slice the calculated FFT data for the slice
         */
        void insertSlice(unsigned int slice_nr, unsigned int slice_height,
            const std::array<QColor, MAX_FFT_POINTS> &slice);

        /** Rebuild the palette with automatic contrast. */
        void createPalette();

    private:

        /** Rebuild the pixel histogram from the current image. */
        void updateHistogram();

        /** Palette mode used for inserted slices and indexed colors. */
        Kwave::SonagramPlugin::ColorMode m_color_mode;

        /** Histogram of indexed pixels for automatic contrast. */
        unsigned int m_histogram[256];
    };
}

#endif /* SONAGRAM_IMAGE_H */

//***************************************************************************
//***************************************************************************