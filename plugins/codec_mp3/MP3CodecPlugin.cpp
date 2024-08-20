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

#include <KLazyLocalizedString>
#include <QPointer>

#include "libkwave/String.h"

#include "MP3CodecPlugin.h"
#include "MP3Decoder.h"
#include "MP3Encoder.h"
#include "MP3EncoderDialog.h"

KWAVE_PLUGIN(codec_mp3, MP3CodecPlugin)

// static instance of the codec container
Kwave::CodecPlugin::Codec Kwave::MP3CodecPlugin::m_codec = EMPTY_CODEC;

/***************************************************************************/
Kwave::MP3CodecPlugin::MP3CodecPlugin(QObject *parent,
                                      const QVariantList &args)
    :Kwave::CodecPlugin(parent, args, m_codec)
{
}

/***************************************************************************/
Kwave::MP3CodecPlugin::~MP3CodecPlugin()
{
}

/***************************************************************************/
void Kwave::MP3CodecPlugin::load(QStringList &params)
{
    emitCommand(_("menu (plugin:setup(codec_mp3), Settings/%1)").arg(
        _(kli18nc("menu: /Settings/MP3 Encoder Setup",
                                  "MP3 Encoder Setup").untranslatedText())));
    Kwave::CodecPlugin::load(params);
}

//***************************************************************************
QStringList *Kwave::MP3CodecPlugin::setup(QStringList &previous_params)
{
    Q_UNUSED(previous_params);

    // create the setup dialog
    QPointer<MP3EncoderDialog> dialog =
        new(std::nothrow) MP3EncoderDialog(parentWidget());
    Q_ASSERT(dialog);
    if (!dialog) return nullptr;

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec() && dialog) {
        // user has pressed "OK"
        dialog->save();
    } else {
        // user pressed "Cancel"
        if (list) delete list;
        list = nullptr;
    }

    if (dialog) delete dialog;
    return list;

}

/***************************************************************************/
QList<Kwave::Decoder *> Kwave::MP3CodecPlugin::createDecoder()
{
    return singleDecoder<Kwave::MP3Decoder>();
}

/***************************************************************************/
QList<Kwave::Encoder *> Kwave::MP3CodecPlugin::createEncoder()
{
    return singleEncoder<Kwave::MP3Encoder>();
}

//***************************************************************************
#include "MP3CodecPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_MP3CodecPlugin.cpp"
