/*************************************************************************
               Splash.h  -  splash screen for Kwave
                             -------------------
    begin                : Tue Jun 24 2003
    copyright            : Copyright (C) 2003 Gilles CAULIER
    email                : Gilles CAULIER <caulier.gilles@free.fr>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWAVE_SPLASH_H
#define KWAVE_SPLASH_H

#include <QFrame>
#include <QFont>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QString>
#include <QTimer>

class QMouseEvent;
class QPaintEvent;

namespace Kwave
{
    class Splash: public QFrame
    {
    Q_OBJECT

    public:
	/**
	 * Constructor
	 * @param PNGFile name of a file to be shown as splashscreen,
	 *        should be found in one of the "appdata" directories.
	 */
	explicit Splash(const QString &PNGFile);

	/** Destructor */
        virtual ~Splash() Q_DECL_OVERRIDE;

	/** wrapper for QSplashScreen::showMessage with only one parameter */
	static void showMessage(const QString &message);

	/** handles the painting of this splash screen */
        virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	/** hides the splash screen on mouse click */
        virtual void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;

	/** should be called when the splashscreen is no longer needed */
	void done();

    private:

	/** font to use for the status text and version number */
	QFont m_font;

	/** pixmap with the Kwave logo */
	QPixmap m_pixmap;

	/** last status message */
	QString  m_message;

	/** static instance */
	static QPointer<Kwave::Splash> m_splash;
    };
}

#endif /* KWAVE_SPLASH_H */

//***************************************************************************
//***************************************************************************
