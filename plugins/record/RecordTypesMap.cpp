/***************************************************************************
     RecordTypesMap.cpp  -  map for record methods
                             -------------------
    begin                : Sun Jul 31 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
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

#include <KLocalizedString>
#include <QString>

#include "libkwave/String.h"

#include "RecordTypesMap.h"

//***************************************************************************
void Kwave::RecordTypesMap::fill()
{
    unsigned int index = 0;

#ifdef HAVE_QT_AUDIO_SUPPORT
    append(index++, Kwave::RECORD_QT,
        _("qt"),
        kli18n("Qt Multimedia Audio"));
#endif /* HAVE_QT_AUDIO_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
    append(index++, Kwave::RECORD_PULSEAUDIO,
        _("pulseaudio"),
        kli18n("Pulse Audio"));
#endif /* HAVE_PULSEAUDIO_SUPPORT */

#ifdef HAVE_ALSA_SUPPORT
    append(index++, Kwave::RECORD_ALSA,
        _("alsa"),
        kli18n("ALSA (Advanced Linux Sound Architecture)"));
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
    append(index++, Kwave::RECORD_OSS,
        _("oss"),
        kli18n("OSS (Open Sound System)"));
#endif /* HAVE_OSS_SUPPORT */

   Q_ASSERT(index);
   if (!index) qWarning("no recording method defined!");
}

//***************************************************************************
//***************************************************************************
