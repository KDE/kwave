/***************************************************************************
    AmplifyFreePlugin.h  -  Plugin for free amplification curves
                             -------------------
    begin                : Sun Sep 02 2001
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

#ifndef AMPLIFY_FREE_PLUGIN_H
#define AMPLIFY_FREE_PLUGIN_H

#include "config.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/Curve.h"
#include "libkwave/Plugin.h"

namespace Kwave
{
    /** @todo add support for logarithmic scale */
    class AmplifyFreePlugin: public Kwave::Plugin
    {
	Q_OBJECT

    public:

	/**
	 * Constructor
	 * @param parent reference to our plugin manager
	 * @param args argument list [unused]
	 */
	AmplifyFreePlugin(QObject *parent, const QVariantList &args);

	/** Destructor */
	virtual ~AmplifyFreePlugin();

	/**
	 * Shows a dialog for editing the amplification curve and emits a command
	 * for applying the curve if OK has been pressed.
	 * @see Kwave::Plugin::setup
	 */
	QStringList *setup(QStringList &previous_params) Q_DECL_OVERRIDE;

	/**
	 * Does the fade operation
	 * @param params list of strings with parameters
	 */
	void run(QStringList params) Q_DECL_OVERRIDE;

	/**
	 * @see Kwave::Plugin::start(),
	 * overloaded to get the action name from the parameters
	 */
	int start(QStringList &params) Q_DECL_OVERRIDE;

	/**
	 * Returns a text for the progress dialog if enabled.
	 * (already be localized)
	 */
	QString progressText() Q_DECL_OVERRIDE;

    protected:

	/** Reads values from the parameter list */
	int interpreteParameters(QStringList &params);

    private:

	/** name of the action (untranslated) */
	QString m_action_name;

	/** List of parameters */
	QStringList m_params;

	/** curve used for interpolation */
	Kwave::Curve m_curve;

	/** map for command name <-> description */
	QMap<QString, QString> m_cmd_map;
    };
}

#endif /* AMPLIFY_FREE_PLUGIN_H */

//***************************************************************************
//***************************************************************************
