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
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QScreen>
#include <QString>
#include <QThread>

#include <KAboutData>
#include <KLocalizedString>

#include <QStandardPaths>

#include "libkwave/String.h"

#include "Splash.h"

// static pointer to the current instance
QPointer<Kwave::Splash> Kwave::Splash::m_splash = Q_NULLPTR;

//***************************************************************************
Kwave::Splash::Splash(const QString &PNGFile)
    :QFrame(Q_NULLPTR, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint),
     m_font(),
     m_pixmap(QStandardPaths::locate(QStandardPaths::AppDataLocation, PNGFile)),
     m_message(_("   "))
{
    const int w = m_pixmap.width();
    const int h = m_pixmap.height();
    setFixedSize(w, h);

    QPainter p;
    p.begin(&m_pixmap);

    // get all the strings that we should display
    const KAboutData about_data = KAboutData::applicationData();
    QString version = i18nc("%1=Version number", "v%1", about_data.version());

    m_font.setBold(true);
    m_font.setStyleHint(QFont::Decorative);
    m_font.setWeight(QFont::Black);
    p.setFont(m_font);

    QFontMetrics fm(m_font);
    QRect rect = fm.boundingRect(version);

    // version
    const int r  = 5;
    const int th = rect.height();
    const int tw = rect.width();
    const int x  = w - 10 - tw;
    const int y  = h - 10 - th;

    const QBrush brush(palette().window().color());
    p.setBrush(brush);
    p.setOpacity(0.50);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(
        x - r, y - r,
        tw + 2 * r, th + (2 * r),
        (200 * r) / th, (200 * r) / th,
        Qt::RelativeSize
    );

    p.setOpacity(1.0);
    p.setPen(palette().buttonText().color());
    p.drawText(x, y, tw, th, Qt::AlignCenter, version);

    p.end();

    // center on the screen
    rect = QRect(QPoint(), m_pixmap.size());
    resize(rect.size());

    QScreen *screen = qApp->primaryScreen();
    Q_ASSERT(screen);
    QRect screen_rect;
    screen_rect.setSize(screen->availableSize());
    move(screen_rect.center() - rect.center());

    m_splash = this;
}

//***************************************************************************
void Kwave::Splash::done()
{
    // check: start() must be called from the GUI thread only!
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(this->thread() == qApp->thread());
    m_splash = Q_NULLPTR;
    hide();
}

//***************************************************************************
Kwave::Splash::~Splash()
{
    m_splash = Q_NULLPTR;
}

//***************************************************************************
void Kwave::Splash::showMessage(const QString &message)
{
    if (!m_splash) return;
    m_splash->m_message = message;
    m_splash->repaint();
    QApplication::processEvents(QEventLoop::AllEvents);
}

//***************************************************************************
void Kwave::Splash::paintEvent(QPaintEvent *)
{
    // special handling: a null message tells us to hide
    if (!m_message.length()) {
        m_splash = Q_NULLPTR;
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
    p.setFont(m_font);
    p.setPen(palette().buttonText().color());
    p.drawText(rect, Qt::AlignLeft | Qt::AlignTop, m_message);
}

//***************************************************************************
void Kwave::Splash::mousePressEvent(QMouseEvent *)
{
    done();
}

//***************************************************************************
//***************************************************************************

#include "moc_Splash.cpp"
