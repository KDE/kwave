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
class QStrList;
class KwavePlugin;

//*****************************************************************************
class SonagramDialog : public QDialog 
{
    Q_OBJECT

public:
    SonagramDialog(KwavePlugin &p);
    ~SonagramDialog();

    /**
     * Fills the current parameters into a parameter list.
     * The list always is cleared before it gets filled.
     * The first parameter will contain the number of fft points [1...n]
     * The second parameter will contain the id of a window function
     * or zero if no window function was selected ("<none>").
     */
    void getParameters(QStrList &list);

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
