/***************************************************************************
         SonagramDialog.h  -  dialog for setting up the sonagram window
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

#ifndef _SONAGRAM_DIALOG_H_
#define _SONAGRAM_DIALOG_H_

#include <qdialog.h>

class Slider;
class QComboBox;
class QLabel;
class QPushButton;
class KwavePlugin;

//*****************************************************************************
class SonagramDialog : public QDialog 
{
    Q_OBJECT

public:
    SonagramDialog(KwavePlugin &p);
    virtual ~SonagramDialog ();

    const char * getCommand ();

public slots:
    void setPoints (int);
    void setBoxPoints (int);

private:

    Slider *pointslider;
    QLabel *pointlabel;
    QLabel *windowlabel;
    QLabel *bitmaplabel;
    QLabel *windowtypelabel;
    QComboBox *pointbox;
    QComboBox *windowtypebox;
    QPushButton *ok, *cancel;
    int length, rate;
};

#endif /* _SONAGRAM_DIALOG_H_ */

/* end of SonagramDialog.h */
