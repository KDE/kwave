/***************************************************************************
    libkwave/FileContext.cpp  -  Context of a Loaded File
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

#include "libkwave/FileContext.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "kwave/MainWidget.h"
#include "kwave/TopWidget.h"

//***************************************************************************
Kwave::App &Kwave::FileContext::application() const
{
    return m_application;
}

//***************************************************************************
Kwave::TopWidget *Kwave::FileContext::topWidget() const
{
    Q_ASSERT(m_top_widget);
    return m_top_widget;
}

//***************************************************************************
Kwave::MainWidget *Kwave::FileContext::mainWidget() const
{
    Q_ASSERT(m_main_widget);
    return m_main_widget;
}

//***************************************************************************
Kwave::SignalManager *Kwave::FileContext::signalManager() const
{
    Q_ASSERT(m_signal_manager);
    return m_signal_manager;
}

//***************************************************************************
Kwave::PluginManager *Kwave::FileContext::pluginManager() const
{
    return m_plugin_manager;
}

//***************************************************************************
#include "FileContext.moc"
//***************************************************************************
//***************************************************************************
