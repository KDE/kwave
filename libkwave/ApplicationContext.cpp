/***************************************************************************
    libkwave/ApplicationContext.cpp  -  Context of one Kwave instance
			     -------------------
    begin                : 2009-12-31
    copyright            : (C) 2009 by Thomas.Eschenbacher
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

#include "libkwave/ApplicationContext.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "kwave/TopWidget.h"

//***************************************************************************
KwaveApp &Kwave::ApplicationContext::application()
{
    return m_application;
}

//***************************************************************************
TopWidget *Kwave::ApplicationContext::topWidget()
{
    Q_ASSERT(m_top_widget);
    return m_top_widget;
}

//***************************************************************************
SignalManager *Kwave::ApplicationContext::signalManager()
{
    Q_ASSERT(m_signal_manager);
    return m_signal_manager;
}

//***************************************************************************
Kwave::PluginManager *Kwave::ApplicationContext::pluginManager()
{
    Q_ASSERT(m_plugin_manager);
    return m_plugin_manager;
}

//***************************************************************************
//***************************************************************************
