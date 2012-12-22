/***************************************************************************
    kwave/ApplicationContext.cpp  -  Context of one Kwave instance
			     -------------------
    begin                : 2010-01-02
    copyright            : (C) 2010 by Thomas.Eschenbacher
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
Kwave::ApplicationContext::ApplicationContext(Kwave::App &app)
    :QObject(), m_application(app), m_top_widget(0), m_signal_manager(0),
     m_plugin_manager(0)
{
}

//***************************************************************************
Kwave::ApplicationContext::~ApplicationContext()
{
    if (m_top_widget)     delete m_top_widget;
    if (m_signal_manager) delete m_signal_manager;
    if (m_plugin_manager) delete m_plugin_manager;
}

//***************************************************************************
bool Kwave::ApplicationContext::init()
{
    m_top_widget = new Kwave::TopWidget(*this);
    Q_ASSERT(m_top_widget);
    if (!m_top_widget) return false;

    m_signal_manager = new Kwave::SignalManager(m_top_widget);
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return false;

    m_plugin_manager = new Kwave::PluginManager(m_top_widget, *m_signal_manager);
    Q_ASSERT(m_plugin_manager);
    if (!m_plugin_manager) return false;

    return m_top_widget->init();
}

//***************************************************************************
//***************************************************************************
