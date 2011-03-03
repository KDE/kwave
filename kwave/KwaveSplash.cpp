/*************************************************************************
        KwaveSplash.cpp  -  splash screen for Kwave
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

#include "config.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTimer>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "KwaveSplash.h"

// static pointer to the current instance
QPointer<KwaveSplash> KwaveSplash::m_splash = 0;
 
//***************************************************************************
KwaveSplash::KwaveSplash(const QString &PNGImageFileName)
    :QFrame(0, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
     m_pixmap(KStandardDirs::locate("appdata", PNGImageFileName))
{
    m_splash = this;

    const int w = m_pixmap.width();
    const int h = m_pixmap.height();
    setFixedSize(w, h);

    QPainter p;
    p.begin(&m_pixmap);

    // get all the strings that we should display
    const KAboutData *about_data = KGlobal::mainComponent().aboutData();
    QString version = i18nc("%1=Version number", "v%1", about_data->version());

    QFont font;
    font.setBold(true);
    font.setStyleHint(QFont::Decorative);
    font.setWeight(QFont::Black);
    p.setFont(font);

    QFontMetrics fm(font);
    QRect rect = fm.boundingRect(version);

    // version
    const int r  = 5;
    const int th = rect.height();
    const int tw = rect.width();
    int x = w - 10 - tw;
    int y = h - 10 - th;
    const QColor textcolor = palette().buttonText().color();

    const QBrush brush(palette().background().color());
    p.setBrush(brush);
    p.setOpacity(0.50);
    p.setPen(Qt::NoPen);
    p.drawRoundRect(
	x - r, y - r,
	tw + 2 * r, th + (2 * r),
	(200 * r) / th, (200 * r) / th
    );

    p.setOpacity(1.0);
    p.setPen(textcolor);
    p.drawText(x, y, tw, th, Qt::AlignCenter, version);

    p.end();

    // center on the screen
    rect = QRect(QPoint(), m_pixmap.size());
    resize(rect.size());
    move(QApplication::desktop()->screenGeometry().center() - rect.center());

    // auto-close in 4 seconds...
    QTimer::singleShot(4000, this, SLOT(done()));
}

//***************************************************************************
void KwaveSplash::done()
{
    m_splash = 0;
    close();
}

//***************************************************************************
KwaveSplash::~KwaveSplash()
{
    m_splash = 0;
}

//***************************************************************************
void KwaveSplash::showMessage(const QString &message)
{
    if (!m_splash) return;
    m_splash->m_message = message;
    m_splash->repaint();
    QApplication::processEvents();
}

//***************************************************************************
void KwaveSplash::paintEvent(QPaintEvent *)
{
    QRect rect(this->rect());
    const int border = 5;
    rect.setRect(
	rect.x() + border, 
	rect.y() + border, 
	rect.width()  - (2 * border), 
	rect.height() - (2 * border)
    );
 
    QPainter p(this);
    p.drawPixmap(this->rect(), m_pixmap);
    p.setPen(Qt::black);
    p.drawText(rect, Qt::AlignLeft, m_message);
}

//***************************************************************************
void KwaveSplash::mousePressEvent(QMouseEvent *)
{
    hide();
}

//***************************************************************************
#include "KwaveSplash.moc"
//***************************************************************************
//***************************************************************************
