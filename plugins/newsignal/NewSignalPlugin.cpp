/***************************************************************************
    NewSignalPlugin.cpp  -  plugin for creating a new signal
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

#include "config.h"

#include <errno.h>
#include <new>

#include <QPointer>
#include <QStringList>

#include <KLocalizedString>

#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "NewSignalDialog.h"
#include "NewSignalPlugin.h"

KWAVE_PLUGIN(newsignal, NewSignalPlugin)

//***************************************************************************
Kwave::NewSignalPlugin::NewSignalPlugin(QObject *parent,
                                        const QVariantList &args)
    :Kwave::Plugin(parent, args), m_samples(2646000), m_rate(44100),
    m_bits(16), m_tracks(2), m_bytime(true)
{
}

//***************************************************************************
Kwave::NewSignalPlugin::~NewSignalPlugin()
{
}

//***************************************************************************
int Kwave::NewSignalPlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    if (params.count() != 5) return -EINVAL;

    param = params[0];
    m_samples = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[1];
    m_rate = Kwave::toUint(param.toDouble(&ok));
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[2];
    m_bits = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[3];
    m_tracks = param.toUInt(&ok);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    param = params[4];
    m_bytime = (param.toUInt(&ok) != 0);
    Q_ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
QStringList *Kwave::NewSignalPlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    QPointer<Kwave::NewSignalDialog> dialog = new(std::nothrow)
        Kwave::NewSignalDialog(
            parentWidget(), m_samples, m_rate, m_bits, m_tracks, m_bytime);
    Q_ASSERT(dialog);
    if (!dialog) return nullptr;

    QStringList *list = new(std::nothrow) QStringList();
    Q_ASSERT(list);
    if (list && dialog->exec() && dialog) {
        // user has pressed "OK"
        *list << QString::number(dialog->samples());
        *list << QString::number(dialog->rate());
        *list << QString::number(dialog->bitsPerSample());
        *list << QString::number(dialog->tracks());
        *list << _(dialog->byTime() ? "1" : "0");

        emitCommand(_("newsignal(") +
            QString::number(dialog->samples()) + _(",") +
            QString::number(dialog->rate()) + _(",") +
            QString::number(dialog->bitsPerSample()) + _(",") +
            QString::number(dialog->tracks()) + _(")")
        );
    } else {
        // user pressed "Cancel"
        delete list;
        list = nullptr;
    }

    delete dialog;
    return list;
}

//***************************************************************************
#include "NewSignalPlugin.moc"
//***************************************************************************
//***************************************************************************

#include "moc_NewSignalPlugin.cpp"
