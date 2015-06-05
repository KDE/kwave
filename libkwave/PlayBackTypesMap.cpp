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

#include <QString>

#include <KLocalizedString>

#include "libkwave/PlayBackTypesMap.h"
#include "libkwave/String.h"

//***************************************************************************
void Kwave::PlayBackTypesMap::fill()
{
    unsigned int index = 0;

#ifdef HAVE_ALSA_SUPPORT
    append(index++, Kwave::PLAYBACK_ALSA,       _("alsa"),
	   _(I18N_NOOP("ALSA (Advanced Linux Sound Architecture)")) );
#endif /* HAVE_ALSA_SUPPORT */

#ifdef HAVE_OSS_SUPPORT
    append(index++, Kwave::PLAYBACK_OSS,        _("oss"),
	   _(I18N_NOOP("OSS (Open Sound System)")) );
#endif /* HAVE_OSS_SUPPORT */

#ifdef HAVE_PHONON_SUPPORT
    append(index++, Kwave::PLAYBACK_PHONON,     _("phonon"),
	   _(I18N_NOOP("Phonon (KDE)")) );
#endif /* HAVE_PHONON_SUPPORT */

#ifdef HAVE_PULSEAUDIO_SUPPORT
    append(index++, Kwave::PLAYBACK_PULSEAUDIO, _("pulseaudio"),
	   _(I18N_NOOP("Pulse Audio")) );
#endif /* HAVE_PULSEAUDIO_SUPPORT */

    if (!index) qWarning("no playback method defined!");
}

//***************************************************************************
//***************************************************************************
