/***************************************************************************
    KwaveFilterPlugin.h  -  generic class for filter plugins with setup
                             -------------------
    begin                : Sat Jun 07 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#ifndef _KWAVE_FILTER_PLUGIN_H_
#define _KWAVE_FILTER_PLUGIN_H_

#include "config.h"
#include <qobject.h>
#include <qstring.h>
#include "mt/SignalProxy.h"
#include "libkwave/KwavePlugin.h"
#include "libkwave/KwavePluginSetupDialog.h"

class ArtsMultiTrackFilter;
class ConfirmCancelProxy;
class QProgressDialog;
class QStringList;
class QWidget;

class KwaveFilterPlugin: public KwavePlugin
{
    Q_OBJECT
public:

    /** Constructor */
    KwaveFilterPlugin(PluginContext &context);

    /** Destructor */
    virtual ~KwaveFilterPlugin();

    /** Reads values from the parameter list */
    virtual int interpreteParameters(QStringList & /* params */) = 0;

    /**
     * Creates a setup dialog an returns a pointer to it.
     */
    virtual KwavePluginSetupDialog *createDialog(QWidget * /*parent*/) = 0;

    /**
     * Creates a multi-track filter with the given number of tracks
     * @param tracks number of tracks that the filter should have
     * @return pointer to the filter or null if failed
     */
    virtual ArtsMultiTrackFilter *createFilter(unsigned int tracks) = 0;

    /**
     * Shows a dialog for setting up the filter plugin
     * @see KwavePlugin::setup
     */
    virtual QStringList *setup(QStringList &previous_params);

    /** Does the filter operation and/or pre-listen */
    virtual void run(QStringList);

    /** Aborts the process (if running). */
    virtual int stop();

    /**
     * Returns true if the parameters have changed during pre-listen.
     * @note this default implementation always returns false.
     */
    virtual bool paramsChanged();

    /**
     * Update the filter with new parameters if it has changed
     * changed during the pre-listen.
     * @param filter the ArtsMultiTrackFilter to be updated, should be the
     *               same one as created with createFilter()
     * @param force if true, even update if no settings have changed
     */
    virtual void updateFilter(ArtsMultiTrackFilter *filter, bool force=0);

    /**
     * Returns a verbose name of the performed action. Used for giving
     * the undo action a readable name.
     * The name must already be localized !
     */
    virtual QString actionName() = 0;

signals:

    /**
     * emitted when the user pressed the cancel button
     * of the progress dialog
     * @internal
     */
    void sigCancelPressed();

protected slots:

    /** Start the pre-listening */
    void startPreListen();

    /** Stop the pre-listening */
    void stopPreListen();

    /** forward changes of the progress dialog to a signal proxy */
    void forwardProgress(unsigned int progress);

    /** forward press of the cancel button int the progress dialog */
    void forwardCancel();

    /** update the progress dialog */
    void updateProgress();

    /**
     * called when the user has pressed "Cancel" in the progress
     * dialog and also has confirmed the cancel confirmation
     * message box.
     */
    void cancel();
    
private:    
    /** List of parameters */
    QStringList m_params;

    /** flag for stopping the process */
    bool m_stop;

    /** flag for indicating pre-listen mode */
    bool m_listen;

    /** a progress dialog, if the audio processing takes longer... */
    QProgressDialog *m_progress;

    /** signal proxy for the progress / dialog */
    SignalProxy1< unsigned int > *m_spx_progress;

    /**
     * proxy dialog that asks for configmation if the user
     * pressed cancel in the progress dialog
     */
    ConfirmCancelProxy *m_confirm_cancel;

    /** flag for pausing the process */
    bool m_pause;
    
};

#endif /* _KWAVE_FILTER_PLUGIN_H_ */
