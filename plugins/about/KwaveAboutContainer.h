/***************************************************************************
    kwaveaboutcontainer.h  -  Base class for the authors and thanks field in
            the kwave about dialog
                             -------------------
    begin                : Sat Mar 9 2002
    copyright            : (C) 2002 by Ralf Waspe
    email                : rwaspe@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KWAVEABOUTCONTAINER_H
#define KWAVEABOUTCONTAINER_H

#include <kaboutdata.h>
#include <kaboutdialog.h>
#include <qwidget.h>

/**
  *@author Ralf Waspe
  */

class KwaveAboutContainer : public KAboutContainer
{
    Q_OBJECT

public:
    /** Constructor */
    	KwaveAboutContainer(QWidget* parent = 0, const char* name = "");
    /** destructor */
    ~KwaveAboutContainer();

public slots:
    /** open webpage if url is clicked
      * connect to :
      * void  urlClick(const QString &url)
      */
    void openURL(const QString &url);
    /** send email if email address is clicked
      * connect to :
      * void  mailClick(const QString &name,const QString &address)
      */
    void sendMail(const QString &name,const QString &address);
};

#endif
