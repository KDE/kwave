/*************************************************************************
         RecordPlugin.h  -  plugin for recording audio data
                             -------------------
    begin                : Wed Jul 09 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _RECORD_PLUGIN_H_
#define _RECORD_PLUGIN_H_

#include "config.h"
#include "libkwave/KwavePlugin.h"
#include "RecordParams.h"

class QStringList;
class RecordDevice;
class RecordDialog;

class RecordPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    RecordPlugin(PluginContext &c);

    /** Destructor */
    virtual ~RecordPlugin();

    /** @see KwavePlugin::setup() */
    virtual QStringList *setup(QStringList &previous_params);

protected slots:

    /** select a new record device */
    void changeDevice(const QString &dev);

    /** select a new sample rate [samples/second] */
    void changeSampleRate(double new_rate);

private:

    /** record control: pre-record enabled */
    bool m_prerecord_enabled;

    /** all parameters of the record plugin */
    RecordParams m_params;

    /** device used for recording */
    RecordDevice *m_device;

    /** setup dialog */
    RecordDialog *m_dialog;

};


#endif /* _RECORD_PLUGIN_H_ */
