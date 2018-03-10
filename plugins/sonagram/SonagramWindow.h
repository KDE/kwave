/***************************************************************************
              SonagramWindow.h  -  window for showing a sonagram
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef SONAGRAM_WINDOW_H
#define SONAGRAM_WINDOW_H

#include "config.h"

#include <QTimer>

#include <KMainWindow>

class QBitmap;
class QImage;

/** height of the overview widget in a sonagram window [pixels] */
#define SONAGRAM_OVERVIEW_HEIGHT 30

namespace Kwave
{

    class ImageView;
    class ScaleWidget;

    /**
     * Window for displaying a sonagram with scale, status bar and
     * a small menu.
     */
    class SonagramWindow: public KMainWindow
    {
	Q_OBJECT

    public:

	/**
	 * Constructor.
	 * @param parent the parent widget
	 * @param name reference to the initial name of the signal (used for
	 *        setting the window title, might be an empty string)
	 */
	SonagramWindow(QWidget *parent, const QString &name);

	/**
	 * Destructor.
	 */
	virtual ~SonagramWindow();

	/**
	 * Sets a new sonagram image to display.
	 * @param image the bitmap with the sonagram
	 */
	void setImage(QImage image);

	/**
	 * Sets a new overview bitmap for the signal space
	 */
	void setOverView(const QImage &image);

	/**
	 * Inserts a slice into the current image. If the slice contains more
	 * data than fits into the image, the remaining rest will be ignored,
	 * if less data is present, it will be filled with 0xFF. The previous
	 * content of the image slice will be cleared or updated in all cases.
	 * @param slice_nr index of the slice (horizontal position) [0..n-1]
	 * @param slice array with the byte data
	 */
	void insertSlice(const unsigned int slice_nr, const QByteArray &slice);

    public slots:

	/** closes the sonagram window */
	void close();

	/** not implemented yet */
	void save();

	/** not implemented yet */
	void load();

	/** not implemented yet */
	void toSignal();

	/**
	 * Sets the name of the signal / title of the window
	 * @param name the name of the signal
	 */
	void setName(const QString &name);

	/**
	 * Sets a new color mode. If the mode is different from the current
	 * one, the image will be automatically refreshed.
	 */
	void setColorMode(int mode);

	/**
	 * Used to update the display of the current position of the cursor.
	 * Position is given in coordinates of the QImage.
	 * @param pos current cursor position
	 */
	void cursorPosChanged(const QPoint pos);

	/**
	 * sets information about the number of fft points (needed
	 * for translating cursor coordinates into time)
	 * @param points the number of fft points [1...]
	 */
	void setPoints(unsigned int points);

	/**
	 * sets information about the sample rate (needed for
	 * translating cursor coordinates into time
	 * @param rate sample rate in samples per second
	 */
	void setRate(double rate);

    private slots:

	/** refreshes the image, connected to m_refresh_timer */
	void refresh_view();

    protected:

	/** updates the scale widgets */
	void updateScaleWidgets();

	/**
	 * adjust the brightness so that the color space is optimally
	 * used and the user doesn't just see a white image
	 */
	void adjustBrightness();

	/**
	 * Translates pixel coordinates relative to the lower left corner
	 * of the QImage into time and frequency coordinates of the signal.
	 * This requires a valid sample rate to be set, otherwise the
	 * time coordinate will be returned as zero.
	 * @param p a QPoint with the pixel position, upper left is 0/0
	 * @param ms pointer to a double that receives the time coordinate
	 *        in milliseconds (can be 0 to ignore)
	 * @param f pointer to a double that receives the frequency
	 *        coordinate (can be 0 to ignore)
	 */
	void translatePixels2TF(const QPoint p, double *ms, double *f);

    private:

	/** status bar label for time */
	QLabel *m_status_time;

	/** status bar label for frequency */
	QLabel *m_status_freq;

	/** status bar label for amplitude */
	QLabel *m_status_ampl;

	/** the QImage to be displayed */
	QImage m_image;

	/**
	 * the color mode to be used. Currently only 0 (black/white)
	 * and 1 (rainbow colors) are used.
	 */
	int m_color_mode;

	/** an ImageView to display the m_image and fit it into our window */
	Kwave::ImageView *m_view;

	/** short overview over the signal */
	Kwave::ImageView *m_overview;

	/** number of fft points */
	unsigned int m_points;

	/** sample rate, needed for translating pixel coordinates */
	double m_rate;

	/** widget for the scale on the time (x) axis */
	Kwave::ScaleWidget *m_xscale;

	/** widget for the scale on the frequency (y) axis */
	Kwave::ScaleWidget *m_yscale;

	/** timer used for refreshing the view from time to time */
	QTimer m_refresh_timer;

	/** histogram of color indices, used for auto-contrast */
	unsigned int m_histogram[256];

    };
}

#endif // _SONOGRAM_WINDOW_H_

//***************************************************************************
//***************************************************************************
