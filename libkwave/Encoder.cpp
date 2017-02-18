/***************************************************************************
            Encoder.cpp  -  abstract base class of all encoders
			     -------------------
    begin                : May 08 2007
    copyright            : (C) 2007 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "libkwave/Encoder.h"

//***************************************************************************
Kwave::Encoder::Encoder()
    :QObject(), Kwave::CodecBase()
{
}

//***************************************************************************
QList<Kwave::FileProperty> Kwave::Encoder::unsupportedProperties(
    const QList<Kwave::FileProperty> &properties_to_check)
{
    QList<Kwave::FileProperty> unsupported;
    const QList<Kwave::FileProperty> supported(supportedProperties());
    const Kwave::FileInfo info;

    foreach (const Kwave::FileProperty &s, properties_to_check) {
	if (info.canLoadSave(s) && !supported.contains(s))
	    unsupported.append(s);
    }

    return unsupported;
}

//***************************************************************************
//***************************************************************************
