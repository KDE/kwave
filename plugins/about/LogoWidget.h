/***************************************************************************
         LogoWidget.h  -  widget with the animated Kwave logo
                             -------------------
    begin                : Sun Oct 29 2000
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

#ifndef _LOGO_WIDGET_H_
#define _LOGO_WIDGET_H_

#include <qpainter.h>
#include <qwidget.h>

class QPaintEvent;
class QPixmap;
class QTimer;

#define MAXSIN 5

//**********************************************************
class LogoWidget : public QWidget
{
    Q_OBJECT

public:
    /** Constructor */
    LogoWidget(QWidget *parent, const char *name);

    /** Destructor */
    virtual ~LogoWidget();

public slots:
    /** animates the next step of the logo */
    void doAnim();

protected:
    /** repaints */
    void paintEvent(QPaintEvent *);

private:
    /** width of the widget */
    int m_width;

    /** height of the widget */
    int m_height;

    /** set to true for repaint */
    bool m_repaint;

    /** phase of sinus for animation */
    double m_deg[MAXSIN];

    /** pixmap for output */
    QPixmap *m_pixmap;

    /** additional pixmap to avoid flicker */
    QPixmap *m_buffer;

    /** image with the logo */
    QPixmap *m_img;

    /** timer for refresh */
    QTimer *m_timer;
};

#endif  /* _LOGO_WIDGET_H_ */

