/***************************************************************************
         ZeroPlugin.cpp  -  wipes out the selected range of samples to zero
                             -------------------
    begin                : Fri Jun 01 2001
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

#include <klocale.h> // for the i18n macro

#include "libkwave/MultiTrackWriter.h"
#include "libkwave/SampleWriter.h"
#include "kwave/PluginManager.h"
#include "kwave/UndoTransactionGuard.h"

#include "ZeroPlugin.h"

KWAVE_PLUGIN(ZeroPlugin,"zero","Thomas Eschenbacher");

#define ZERO_COUNT 64*1024

//***************************************************************************
ZeroPlugin::ZeroPlugin(PluginContext &context)
    :KwavePlugin(context), m_stop(false)
{
}

//***************************************************************************
void ZeroPlugin::run(QStringList)
{
    UndoTransactionGuard undo_guard(*this, i18n("silence"));
    m_stop = false;

    MultiTrackWriter writers;
    manager().openMultiTrackWriter(writers, Overwrite);

    // break if aborted
    if (writers.isEmpty()) return;

    unsigned int first = writers[0]->first();
    unsigned int last  = writers[0]->last();
    unsigned int count = writers.count();

    // get the buffer with zeroes for faster filling
    if (m_zeroes.count() != ZERO_COUNT) {
	m_zeroes.resize(ZERO_COUNT);
	m_zeroes.fill(0);
    }

    // loop over the sample range
    while ((first <= last) && (!m_stop)) {
	unsigned int rest = last - first + 1;
	if (rest < m_zeroes.count()) m_zeroes.resize(rest);
	
	// loop over all writers
	unsigned int w;
	for (w=0; w < count; w++) {
	    *(writers[w]) << m_zeroes;
	}
	
	first += m_zeroes.count();
    }

    close();
}

//***************************************************************************
int ZeroPlugin::stop()
{
    m_stop = true;
    return KwavePlugin::stop();
}

//***************************************************************************
//***************************************************************************
