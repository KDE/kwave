/***************************************************************************
                     KwavePlugin.h  -  New Interface for Kwave plugins
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef _KWAVE_PLUGIN_H_
#define _KWAVE_PLUGIN_H_

#include <qobject.h>
#include "mt/Mutex.h"
#include "mt/Asynchronous_Object.h"

class PluginManager;
class PluginContext;
class KwavePlugin;
class TopWidget;
class QStrList;

#define KWAVE_PLUGIN(class_name,plugin_name,author_name) \
    const char *version = "2.0"; \
    const char *name = plugin_name; \
    const char *author = author_name; \
    KwavePlugin *load(PluginContext &c) { \
	class_name *np = new class_name(c); \
	return np; \
    }

/**
 * Generic class that should be used for all types of Kwave plugins.
 * This interface is the only one that should be used, it provides
 * all necessary functions to access the functionality of the main
 * Kwave program.
 */
class KwavePlugin: public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    KwavePlugin(PluginContext &c);

    /**
     * Destructor.
     */
    virtual ~KwavePlugin();

    /**
     * Sets up all necessary parameters for executing the plugin. Could
     * be overwritten and show a dialog box. This only will be called if
     * currently no parameters are given from the function that loads
     * the plugin (the normal case) but not in the context of replaying
     * a previously recorded macro.
     * @param previous_params the parameters of a previous call, could
     *        be used to initialize the controls of a setup dialog
     * @return a string list with all parameters, an empty list (if nothing
     *         has to be set up) or null if the setup (dialog) has been
     *         aborted and the plugin should not get executed
     */
    virtual QStrList *setup(QStrList *previous_params = 0);

    /**
     * Is called from the main program before the run() function and can
     * be overwritten to show a window or initialize some things before
     * the run() function gets called.
     * @param list of strings with parameters
     * @return an error code if the execution failed or zero if everything
     *         was ok.
     */
    virtual int start(QStrList &params);

    /**
     * Stops any threads and is called from the close() function and
     * the plugin's destructor.
     */
    virtual int stop();

    /**
     * Gets called from the plugin's execute function and should be overwritten to
     * perform some action. This function runs in a separate thread!
     * @param list of strings with parameters
     */
    virtual void run(QStrList params);

    /**
     * Returns a reference to the manager of this plugin.
     */
    PluginManager &manager();

    /**
     * Returns the parent widget of the plugin. This normally should be
     * a TopWidget of the Kwave main program.
     */
    QWidget* parentWidget();

    /**
     * Returns the name of the current signal. This can be used to set the
     * caption of a plugin's main window. If no signal is currently loaded
     * the returned string is empty.
     */
    const QString &signalName();

    /**
     * Returns the length of the current signal in samples. If no signal is
     * present the return value will be zero.
     */
    virtual unsigned int signalLength();

    /**
     * Returns the sample rate of the current signal. If no signal is
     * present the return value will be zero.
     */
    virtual unsigned int signalRate();

    /**
     * Returns the left and right sample of the current selection
     * in samples from 0 to (size-1). The left and right samples
     * are included in the selection and might be equal. The left
     * is always less or equal than the right. Note that there is
     * always at least one sample selected!
     * @param left receives the first selected sample (optional or null-pointer)
     * @param right receives the last seleted sample (optional or null-pointer)
     * @return the number of selected samples (right-left+1) [1..length]
     */
    virtual unsigned int selection(unsigned int *left=0,unsigned int *right=0);

    /**
     * Returns the value of one single sample of a specified channel.
     * If the channel does not exist or the index of the sample is
     * out of range the return value will be zero.
     */
    virtual int singleSample(unsigned int channel, unsigned int offset);

    /**
     * Gives the control to the next thread. This can be called from
     * within the run() function.
     */
    virtual void yield();

    /**
     * Converts a zoom factor into a string. The number of decimals
     * is automatically adjusted in order to give a nice formated
     * percent value. If the zoom factor gets too high for a reasonable
     * display in percent, the factor is displayed as a numeric
     * multiplier.
     * examples: "0.1 %", "12.3 %", "468 %", "11x"
     * @param buf string buffer to receive the formated string,
     *            should have room for about 16 characters (char[16])
     * @param bufsize number of characters that can be stored in the
     *                buffer, including the trailing zero
     * @param percent the zoom factor to be formated, a value of "100.0"
     *             means "100%", "0.1" means "0.1%" and so on.
     */
    static void zoom2string(char *buf,
	unsigned int bufsize, double percent);

    /**
     * Converts a time in milliseconds into a string. Times below one
     * millisecond are formated with an automatically adjusted number
     * of decimals. Times below one second are formated like "9.9 ms".
     * Times above one second and below one minute are rounded up
     * to full seconds and shown as "12.3 s". From one full minute
     * upwards time is shown as "12:34" (like most CD players do).
     */
    static void ms2string(char *buf, unsigned int bufsize, double ms);

protected:

    friend class PluginManager;

    /**
     * Gets called to execute the plugin's run function in a separate thread.
     * @param list of strings with parameters
     */
    int execute(QStrList &params);

    /**
     * Returns the handle to the dynamically loaded shared object.
     * For internal usage only!
     */
    void *handle();
        	
signals:
    void sigParentWidgetDestroyed();

    void sigDataChanged(int channel, unsigned int first, unsigned int last);

    void sigCommand(const char *command);

    /** will be emitted in the plugin's destructor */
    void sigClosed(KwavePlugin *p, bool remove);

public slots:

    /**
     * Called to close the plugin. This will be called from the plugin
     * manager and can as well be used from inside the plugin if it
     * wishes to close itself.
     */
    virtual void close();

private:
    /**
     * context of the plugin, includes references to some objects of
     * the main program.
     * @deprecated, should be eliminated soon!
     */
    PluginContext &m_context;

    /**
     * Thread that executes the run() member function.
     */
    Asynchronous_Object_with_1_arg<KwavePlugin, QStrList> *m_thread;

    /** Mutex for control over the thread */
    Mutex m_thread_lock;
};

#endif _KWAVE_PLUGIN_H_

// end of libgui/KwavePlugin.h
