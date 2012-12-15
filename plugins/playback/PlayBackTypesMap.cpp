/***************************************************************************
    PlayBackTypesMap.cpp  -  map for playback methods
			     -------------------
    begin                : Sat Feb 05 2005
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

#include <QtCore/QString>

#include <klocale.h>

#include "libkwave/String.h"

#include "PlayBackTypesMap.h"

//***************************************************************************
void Kwave::PlayBackTypesMap::fill()
{
    unsigned int index = 0;
    QString name = _("");

#ifdef HAVE_ALSA_SUPPORT
    name = _(I18N_NOOP("ALSA (Advanced Linux Sound Architecture)"));
    append(index++, Kwave::PLAYBACK_ALSA,
           _("alsa"), name);
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
    name = _(I18N_NOOP("OSS (Open Sound System)"));
    append(index++, Kwave::PLAYBACK_OSS,
           _("oss"), name);
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_PHONON_SUPPORT
    name = _(I18N_NOOP("Phonon (KDE)"));
    append(index++, Kwave::PLAYBACK_PHONON,
           _("phonon"), name);
#endif /* HAVE_PHONON_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
    name = _(I18N_NOOP("Pulse Audio"));
    append(index++, Kwave::PLAYBACK_PULSEAUDIO,
           _("pulseaudio"), name);
#endif /* HAVE_PULSEAUDIO_SUPPORT */

   if (!index) qWarning("no playback method defined!");
}

//***************************************************************************
//***************************************************************************
