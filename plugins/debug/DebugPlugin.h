/*************************************************************************
          DebugPlugin.h  -  various debug aids
                             -------------------
    begin                : Mon Feb 02 2009
    copyright            : (C) 2009 by Thomas Eschenbacher
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

#ifndef DEBUG_PLUGIN_H
#define DEBUG_PLUGIN_H

#include "config.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

#include "libkwave/Plugin.h"
#include "libkwave/Sample.h"
#include "libkwave/SampleArray.h"

namespace Kwave
{
    /**
     * This plugin is intended to be used internally for debuggin and
     * verification purposes.
     */
    class DebugPlugin: public Kwave::Plugin
    {
        Q_OBJECT

    public:

        /**
         * Constructor
         * @param parent reference to our plugin manager
         * @param args argument list [unused]
         */
        DebugPlugin(QObject *parent, const QVariantList &args);

        /** Destructor */
        ~DebugPlugin() override;

        /** @see Kwave::Plugin::load() */
        void load(QStringList &params) override;

        /**
         * Normally this method is used to set up all necessary parameters
         * for executing the plugin. This plugin uses it for performing
         * actions in the context of the GUI thread.
         *
         * @param params some parameters
         * @return always a null pointer
         */
        QStringList *setup(QStringList &params) override;

        /**
         * performs the special function
         * @param params list of strings with parameters
         */
        void run(QStringList params) override;

    private slots:

        /**
         * makes a screenshot, using the information from m_screenshot
         * @param class_name class name of the widget to capture
         * @param filename path to the file to save the screenshot
         */
        void screenshot(const QByteArray &class_name,
                        const QString &filename);

    private:

        /**
         * Dump a tree with all child objects (for debugging)
         * @param obj parent object to start the dump
         * @param indent string for indenting the console output
         */
        void dump_children(const QObject *obj, const QString &indent) const;

        /**
         * Find a widget with a given class name
         * @param class_name name of the class to search
         * @return pointer to the QWidget if found or null
         *         if not found or no QWidget
         */
        QWidget *findWidget(const char *class_name) const;

        /**
         * Find a (child) object with a given class name
         * @param obj object to start the search at
         * @param class_name name of the class to search
         * @return pointer to the QObject if found or null if not found
         */
        QObject *findObject(QObject *obj,
                            const char *class_name) const;

    private:

        /** use an intermediate buffer for faster filling */
        Kwave::SampleArray m_buffer;

    };
}

#endif /* DEBUG_PLUGIN_H */

//***************************************************************************
//***************************************************************************
