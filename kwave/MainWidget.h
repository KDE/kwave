/***************************************************************************
            MainWidget.h  -  main widget of the Kwave TopWidget
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KMAIN_WDIGET_H_
#define _KMAIN_WIDGET_H_ 1

#include <qlist.h>
#include <qwidget.h>

class QAccel;
class QComboBox;
class QFrame;
class QScrollBar;
class KButtonBox;
class KStatusBar;
class MenuManager;
class MultiStateWidget;
class SignalManager;
class OverViewWidget;
class PlaybackController;
class SignalWidget;

//***********************************************************
class MainWidget : public QWidget
{
    Q_OBJECT
public:

    MainWidget (QWidget *parent, MenuManager &manage, KStatusBar &status);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    ~MainWidget ();
    void setSignal (const char *filename, int type = 0);
    void setSignal (SignalManager *);
    void saveSignal (const char *filename, int bits, int type, bool selection);

    /**
     * Closes the current signal.
     */
    void closeSignal();

    /**
     * Returns the current number of channels of the signal or 0 if
     * no signal is loaded.
     */
    unsigned int getChannelCount();

    int getBitsPerSample();

    /** Returns the signal manager of the current signal */
    SignalManager *getSignalManager();

    /** Returns the playback controller */
    PlaybackController *playbackController();

protected:

    /**
     * Called if the main widget has been resized and resizes/moves
     * the signal widget and the channel controls
     */
    virtual void resizeEvent(QResizeEvent *);

    void refreshChannelControls();

public slots:

    bool executeCommand(const char *command);

    void forwardCommand(const char *command);

    void resetChannels();
    void setRateInfo (int);

    void setLengthInfo(int);

    void setTimeInfo(double ms);

    void parseKey(int key);

    void setSelectedTimeInfo(double ms);

    /** returns the current zoom factor */
    double zoom();

    /** calls setZoom() of the signal widget */
    void setZoom(double new_zoom);

    /** calls zoomRange() of the signal widget */
    void zoomRange();

    /** calls zoomIn() of the signal widget */
    void zoomIn();

    /** calls zoomOut() of the signal widget */
    void zoomOut();

    /** calls zoomAll() of the signal widget */
    void zoomAll();

    /** calls zoomNormal() of the signal widget */
    void zoomNormal();

    /** Connected to the signal widget's signalChanged() signal */
    void slot_SignalChanged(int left,int right);

private slots:

    /**
     * Forwards the zoomChanged signal of the iternal view window
     * by emitting sigZoomChanged.
     */
    void forwardZoomChanged(double zoom);

    /**
     * Called if a channel has been added. Updates the display by
     * resizing/re-positioning the channel controls and the signal
     * display.
     * @see #channelDeleted()
     * @param channel index of the added channel [0..n]
     */
    void channelAdded(unsigned int channel);

    /**
     * Called if a channel has been deleted. Updates the display by
     * resizing/re-positioning the channel controls and the signal
     * display.
     * @param #channelAdded()
     * @param channel index of the added channel [0..n]
     */
    void channelDeleted(unsigned int channel);

    /**
     * Connected to the vertical scrollbar and called if the value
     * has changed so that the signal display and the channel
     * controls have to be moved.
     */
    void scrollbarMoved(int newval);

signals:

    /**
     * Will be emitted if the zoom factor of the
     * view has changed.
     */
    void sigZoomChanged(double zoom);

    void sigCommand(const char *command);

    void setOperation (int);
    void channelInfo (unsigned int);

protected:

    /** Updates the menu and buttons. */
    void refreshControls();

    /** Updates the overview in the horizontal slider */
    void refreshOverView();

private:

    QAccel *keys;
    OverViewWidget *m_slider;
    SignalWidget *signalview;
    KStatusBar &status;
    MenuManager &menu;
    QFrame *frmChannelControls;
    QFrame *frmSignal;

    /** vertical scrollbar, only visible if channels do not fit vertically */
    QScrollBar *scrollbar;

    /** array of lamps, one for each channel */
    QList<MultiStateWidget> lamps;

    /** array of speaker icons, one for each channel */
    QList<MultiStateWidget> speakers;

    /** the last number of channels (for detecting changes) */
    unsigned int lastChannels;

    int bsize;
};
#endif // _KMAIN_WDIGET_H_
