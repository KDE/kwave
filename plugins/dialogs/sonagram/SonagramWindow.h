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

#ifndef _SONAGRAM_WINDOW_H_
#define _SONAGRAM_WINDOW_H_ 1

#include <ktmainwindow.h>

#include <libkwave/gsl_fft.h>

class ImageView;
class KStatusBar;
class OverViewWidget;
class QImage;
class QTimer;
class ScaleWidget;

//***********************************************************************
class SonagramWindow : public KTMainWindow
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param name reference to the initial name of the signal (used for
     *        setting the window title, might be an empty string)
     */
    SonagramWindow(const QString &name);

    /**
     * Destructor.
     */
    virtual ~SonagramWindow();

    /**
     * Sets a new sonagram image to display.
     * @param image the bitmap with the sonagram
     */
    void setImage(QImage *image);

    /**
     * Inserts a stripe into the current image. If the stripe contains more
     * data than fits into the image, the remaining rest will be ignored,
     * if less data is present, it will be filled with 0xFF. The previous
     * content of the image stripe will be cleared or updated in all cases.
     * @param stripe_nr index of the stripe (horizontal position) [0..n-1]
     * @param stripe array with the byte data
     */
    void insertStripe(const unsigned int stripe_nr, const QByteArray &stripe);

public slots:

    void close();
    void save();
    void load();
    void toSignal();
    void setName(const QString &name);
    void setInfo(double, double);
    void setRange(int, int, int);

private slots:
    /** refreshes the image, connected to m_refresh_timer */
    void refresh_view();

signals:

protected:
    /** removes old data and the current image */
    void clear();

private:
    KStatusBar *m_status;
    QImage *m_image;
    ImageView *m_view;
    OverViewWidget *m_overview;
    ScaleWidget *m_xscale;
    ScaleWidget *m_yscale;

    /** timer used for refreshing the view from time to time */
    QTimer m_refresh_timer;
};

#endif // _SONOGRAM_WINDOW_H_
