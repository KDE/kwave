/***************************************************************************
      NewSignalDialog.h  -  dialog for the "newsignal" plugin
                             -------------------
    begin                : Wed Jul 18 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#ifndef _NEW_SIGNAL_DIALOG_H_
#define _NEW_SIGNAL_DIALOG_H_

#include <qwidget.h>
#include "NewSigDlg.uih.h"

class QString;
class QWidget;

class NewSignalDialog: public NewSigDlg
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent widget the dialog belongs to
     */
    NewSignalDialog(QWidget *parent);

    /** Returns the selected sample rate [samples/second] */
    double rate();

    /** Returns the selected number of tracks */
    unsigned int tracks();

    /** Returns the selected resolution [bits per sample] */
    unsigned int bitsPerSample();

private slots:

    /** updates rbTime if rbSamples has been toggled */
    void rbSamplesToggled(bool);

    /** updates rbSignal if rbTime has been toggled */
    void rbTimeToggled(bool);

    /** updates the number of samples if the time changed */
    void timeChanged(int);

    /** called when the sample rate has been edited or changed */
    void sampleRateChanged(const QString&);

    /** updates the file size */
    void updateFileSize();

};

#endif /* _NEW_SIGNAL_DIALOG_H_ */
