/*************************************************************************
     MP3CodecPlugin.cpp  -  import and export of MP3 data
                             -------------------
    begin                : Mon May 28 2012
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

#include "config.h"

#include "libkwave/CodecManager.h"
#include "libkwave/String.h"

#include "MP3CodecPlugin.h"
#include "MP3Decoder.h"
#include "MP3Encoder.h"
#include "MP3EncoderDialog.h"

KWAVE_PLUGIN(Kwave::MP3CodecPlugin, "codec_mp3", "2.3",
             I18N_NOOP("MP3 Codec"), "Thomas Eschenbacher");

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::MP3CodecPlugin::m_codec = {0, 0, 0};

/***************************************************************************/
Kwave::MP3CodecPlugin::MP3CodecPlugin(Kwave::PluginManager &plugin_manager)
    :Kwave::CodecPlugin(plugin_manager, m_codec)
{
}

/***************************************************************************/
Kwave::MP3CodecPlugin::~MP3CodecPlugin()
{
}

/***************************************************************************/
void Kwave::MP3CodecPlugin::load(QStringList &params)
{
    emitCommand(_("menu (plugin:setup(codec_mp3), &Options/%1)").arg(
        i18n("MP3 Encoder Setup")));
    Kwave::CodecPlugin::load(params);
}

//***************************************************************************
QStringList *Kwave::MP3CodecPlugin::setup(QStringList &previous_params)
{
    Q_UNUSED(previous_params);

    // create the setup dialog
    MP3EncoderDialog *dialog = new MP3EncoderDialog(parentWidget());
    Q_ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	dialog->save();
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;

}

/***************************************************************************/
Kwave::Decoder *Kwave::MP3CodecPlugin::createDecoder()
{
    return new Kwave::MP3Decoder();
}

/***************************************************************************/
Kwave::Encoder *Kwave::MP3CodecPlugin::createEncoder()
{
    return new Kwave::MP3Encoder();
}

//***************************************************************************
#include "MP3CodecPlugin.moc"
//***************************************************************************
//***************************************************************************
