/*************************************************************************
             Splash.cpp  -  splash screen for Kwave
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
#include <QThread>

#include <KAboutData>
#include <KLocalizedString>
#include <kapplication.h>
#include <kglobal.h>

#include <QStandardPaths>

#include "Splash.h"

// static pointer to the current instance
QPointer<Kwave::Splash> Kwave::Splash::m_splash = 0;

//***************************************************************************
Kwave::Splash::Splash(const QString &PNGImageFileName)
    :QFrame(0, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
     m_pixmap(QStandardPaths::locate(QStandardPaths::DataLocation, PNGImageFileName))
{
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

    m_splash = this;
}

//***************************************************************************
void Kwave::Splash::done()
{
    // check: start() must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());
    m_splash = 0;
    hide();
}

//***************************************************************************
Kwave::Splash::~Splash()
{
    m_splash = 0;
}

//***************************************************************************
void Kwave::Splash::showMessage(const QString &message)
{
    if (!m_splash) return;
    m_splash->m_message = message;
    m_splash->repaint();
    QApplication::processEvents();
}

//***************************************************************************
void Kwave::Splash::paintEvent(QPaintEvent *)
{
    // special handling: a null message tells us to hide
    if (!m_message.length()) {
	m_splash = 0;
	hide();
	return;
    }

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
void Kwave::Splash::mousePressEvent(QMouseEvent *)
{
    done();
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
