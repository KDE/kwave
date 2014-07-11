/***************************************************************************
    kwave/FileContext.cpp  -  Context of a Loaded File
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

#include <new>

#include "libkwave/FileContext.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/PluginManager.h"
#include "libkwave/SignalManager.h"

#include "App.h"
#include "MainWidget.h"
#include "TopWidget.h"

//***************************************************************************
Kwave::FileContext::FileContext(Kwave::App &app)
    :QObject(),
     m_application(app),
     m_top_widget(0),
     m_main_widget(0),
     m_signal_manager(0),
     m_plugin_manager(0)
{
}

//***************************************************************************
Kwave::FileContext::~FileContext()
{
    m_top_widget     = 0;
    m_main_widget    = 0;
    m_signal_manager = 0;
    m_plugin_manager = 0;
}

//***************************************************************************
bool Kwave::FileContext::init(Kwave::TopWidget *top_widget)
{
    m_top_widget = top_widget;
    Q_ASSERT(m_top_widget);
    if (!m_top_widget) return false;

    m_signal_manager = new Kwave::SignalManager(m_top_widget);
    Q_ASSERT(m_signal_manager);
    if (!m_signal_manager) return false;

    m_plugin_manager = new Kwave::PluginManager(m_top_widget, *m_signal_manager);
    Q_ASSERT(m_plugin_manager);
    if (!m_plugin_manager) return false;

    m_main_widget = new Kwave::MainWidget(top_widget, *this);
    Q_ASSERT(m_main_widget);
    if (!m_main_widget) return false;
    if (!(m_main_widget->isOK())) {
	delete m_main_widget;
	m_main_widget = 0;
	return false;
    }

    connect(&(m_signal_manager->playbackController()),
            SIGNAL(sigSeekDone(sample_index_t)),
            m_main_widget, SLOT(scrollTo(sample_index_t)));

    return true;
}

//***************************************************************************
void Kwave::FileContext::close()
{
    if (m_main_widget) delete m_main_widget;
    m_main_widget = 0;

    m_application.closeWindow(m_top_widget);
    m_top_widget = 0;

    if (m_plugin_manager) delete m_plugin_manager;
    m_plugin_manager = 0;

    if (m_signal_manager) delete m_signal_manager;
    m_signal_manager = 0;
}

//***************************************************************************
//***************************************************************************
