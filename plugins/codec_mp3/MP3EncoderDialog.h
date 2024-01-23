/***************************************************************************
     MP3EncoderDialog.h  -  dialog for configuring the MP3 encoer
                            -------------------
    begin                : Sun Jun 03 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef MP3_ENCODER_DIALOG_H
#define MP3_ENCODER_DIALOG_H

#include "config.h"

#include <QDialog>
#include <QMap>
#include <QString>

#include "MP3EncoderSettings.h"

#include "ui_MP3EncoderDialogBase.h"

class QAbstractButton;
class QWidget;

namespace Kwave
{

    class MP3EncoderDialog :public QDialog,
                            public Ui::MP3EncoderDialogBase
    {
        Q_OBJECT

    public:
        /** Constructor */
        explicit MP3EncoderDialog(QWidget *parent);

        /** Destructor */
        virtual ~MP3EncoderDialog();

    public slots:

        /** load all settings from the config file */
        void load();

        /** store all settings to a config file */
        void save();

    private slots:

        /** called when the selection of the program has changed */
        void selectProgram(int index);

        /** called whenever an edit field has changed its value */
        void switchToUserDefined();

        /** evaluates a click to one of the standard buttons */
        void buttonClicked(QAbstractButton *button);

        /** auto-detect settings */
        void autoDetect();

        /** locate the full path of the file */
        void locatePath();

        /** search for the program, file browser */
        void browseFile();

        /** test the settings */
        void testSettings();

        /** show help about the encoder */
        void encoderHelp();

        /** show the help page the manual */
        void invokeHelp();

    private:

        /**
         * Calls the program with one parameter and returns the output
         * of stderr/stdout (combined) as a string
         * @param path the path of the program to call
         * @param param a command line parameter
         * @return string with combined stdout/stderr
         */
        QString callWithParam(const QString &path, const QString &param);

        /**
         * Retrieves the title/name of an encoder by calling it once
         * @param path the path of the encoder program
         * @param param the command line parameter to get the version
         * @return title of the encoder
         */
        QString encoderVersion(const QString &path, const QString &param);

        /**
         * Search for an encoder in the system PATH
         * @param program name of the program
         * @return full path where it can be found,
         *         or the value of the parameter "program" if not found
         */
        QString searchPath(const QString &program);

        /** updates the version info string of the current encoder */
        void updateEncoderInfo();

    private:

        /** current encoder settings, complete struct */
        Kwave::MP3EncoderSettings m_settings;
    };
}

#endif /* MP3_ENCODER_DIALOG_H */

//***************************************************************************
//***************************************************************************
