/***************************************************************************
    LabelerPlugin.h      - Plugin for enhanced signal labeling
                             -------------------
    begin                : Jan 02 2010
    copyright            : (C) 2010 by Daniel Tihelka
    email                : dtihelka@kky.zcu.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _LABELER_PLUGIN_H_
#define _LABELER_PLUGIN_H_

#include "config.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include "libkwave/KwavePlugin.h"



/**
 * TODO: finish it!!!
 *
 */
class LabelerPlugin: public Kwave::Plugin
{
    Q_OBJECT

public:

    /** Constructor */
    LabelerPlugin(const PluginContext &context);

    /** Destructor */
    virtual ~LabelerPlugin();

    /**
     * Reimplementation of Plugin::isUnique() - the plugin must be unique because
     * we need to connect signals/slots from the currently opened signal to that
     * plugin in order to get informed about new tracks, deleted or inserted samples,
     * or just to know when we need some repaint after data changed...
     *
     * @see Kwave::Plugin::isUnique()
     */
    virtual bool isUnique() { return true; };

    /**
     * Shows a dialog for chhosing the label file to read.
     * @see Kwave::Plugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

//    /**
//     * @see Kwave::Plugin::start(),
//     * overloaded to get the action name from the parameters
//     */
//    virtual int start(QStringList &params);

//    /**
//     * Returns a text for the progress dialog if enabled.
//     * (already be localized)
//     */
//    virtual QString progressText();

    /**
     * @see Kwave::Plugin::run(QStringList),
     */
    virtual void run(QStringList params);


protected slots:

    /**
     * Connected to the signal's sigTrackInserted.
     * @param index numeric index of the inserted track
     * @param track reference to the track that has been inserted
     * @see Signal::sigTrackInserted
     * @internal
     */
    void slotTrackInserted(unsigned int index, Track *track);



private:

};

#endif /* _LABELER_PLUGIN_H_ */
