/***************************************************************************
              SonagramPlugin.h  -  plugin that shows a sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
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

#ifndef _SONAGRAM_PLUGIN_H_
#define _SONAGRAM_PLUGIN_H_

#include <qarray.h>
#include <qstring.h>
#include <mt/SignalProxy.h>
#include <libgui/KwavePlugin.h>

class QImage;
class QStrList;
class PluginContext;
class SonagramWindow;
class StripeInfoPrivate;

class SonagramPlugin: public KwavePlugin
{
    Q_OBJECT
public:
    /** Constructor */
    SonagramPlugin(PluginContext &c);

    /** Destructor */
    virtual ~SonagramPlugin();

    /** @see KwavePlugin::setup() */
    virtual QStrList *setup(QStrList *previous_params);

    /** @see KwavePlugin::start() */
    virtual int start(QStrList &params);

    /** @see KwavePlugin::stop() */
    virtual int stop();

    /**
     * Runns once until all stripes of the sonagram are
     * calculated.
     * @see KwavePlugin::run()
     */
    virtual void run(QStrList params);

private slots:

    /**
     * Connected to the SonagramWindow's "destroyed()" signal.
     * @see #sonagram_window
     */
    void windowClosed();

    /**
     * Internally used to synchronously insert the data of one
     * sonagram stripe int the current image and refresh the
     * display.
     * DO NOT CALL DIRECTLY!
     */
    void insertStripe();

protected:

    /**
     * interpretes a given parameter list and sets up internal
     * coordinates accordingly
     * @param params reference to a QStrList with parameters
     * @return 0 if ok, or an error code if failed
     */
    int interpreteParameters(QStrList &params);

private:

    /**
     * Creates a new image for the current processing.
     * If an old image exists, it will be deleted first, a new image
     * will not be created if either width or height is zero.
     * The image will get 8 bits depth and use a color or a greyscale
     * palette.
     * @param width number of horizontal pixels (stripes = signal
     *        length / fft points, rounded up) [1..32767]
     * @param height number of vertical pixels (= fft points / 2) [1..32767]
     * @param color if true, use rainbow colors, if false use greyscale
     */
    void createNewImage(const unsigned int width,
	const unsigned int height, const bool color);

    /**
     * Calculates the fft for one stripe. If the input data range runns
     * over the end of the signal or selection, zeroes will be used instead.
     * @param start sample to start from [0...length-1]
     * @param points number of fft points
     * @param output reference to an array to receive the output
     */
    void calculateStripe(const unsigned int start, const int points,
	QByteArray &output);

    /** the main view of the plugin, a SonagramWindow */
    SonagramWindow *m_sonagram_window;

    /** list of selected channels */
    QArray<unsigned int> m_selected_channels;

    /** first sample of the selection, inclusive */
    unsigned int m_first_sample;

    /** last sample of the selection, inclusive */
    unsigned int m_last_sample;

    /** number of stripes (= width of the image in pixels) */
    unsigned int m_stripes;

    /** number of fft points */
    unsigned int m_fft_points;

    /** index of the window function */
    int m_window_type;

    /** if true, use color display, else use greyscale */
    bool m_color;

    /** if true, update the sonagram if the signal changes */
    bool m_track_changes;

    /** if true, update the sonagram if the selection changed */
    bool m_follow_selection;

    /** stores the image that is currently in process */
    QImage *m_image;

    /**
     * signal proxy for synchronously inserting stripes of
     * sonagram data into the current image and updating the
     * display.
     */
    SignalProxy1<StripeInfoPrivate> *m_spx_insert_stripe;

    /** will be set to true in order to stop the run() loop */
    bool m_cmd_shutdown;

};

#endif /* _SONAGRAM_PLUGIN_H_ */

/* end of module.h */
