#ifndef _KMAIN_WDIGET_H_
#define _KMAIN_WIDGET_H_ 1

#include <qlist.h>
#include <qwidget.h>

class QAccel;
class QComboBox;
class QPushButton;
class KButtonBox;
class KStatusBar;
class MenuManager;
class MultiStateWidget;
class SignalManager;
class OverViewWidget;
class SignalWidget;

//***********************************************************
class MainWidget : public QWidget
	    //mainwidget is parent for all widgets in the main window
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

    unsigned char *getOverView (int);
    int getBitsPerSample();

protected:

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

    void play ();
    void stop ();
    void halt ();
    void loop ();
    void slot_ZoomChanged (double zoom);

private slots:

    /**
     * Informs that a zoom factor has been selected out of the list
     * of predefined zoom factors.
     * @param index the index within the list [0...N-1]
     */
    void zoomSelected(int index);

    void channelAdded(unsigned int channel);

    void channelDeleted(unsigned int channel);

signals:

    void sigCommand(const char *command);

    void setOperation (int);
    void channelInfo (unsigned int);

protected:

    /**
     * Update the menu and buttons.
     */
    void refreshControls();

    virtual void resizeEvent(QResizeEvent *);

private:

    QAccel *keys;
    KButtonBox *buttons;
    OverViewWidget *slider;
    SignalWidget *signalview;
    QPushButton *plusbutton, *minusbutton;
    QPushButton *zoombutton, *nozoombutton;
    QPushButton *zoomallbutton;
    QPushButton *playbutton, *loopbutton;
    QComboBox *zoomselect;
    KStatusBar &status;
    MenuManager &menu;

    /** array of lamps, one for each channel */
    QList<MultiStateWidget> lamps;

    /** array of speaker icons, one for each channel */
    QList<MultiStateWidget> speakers;

    /** the last number of channels (for detecting changes) */
    unsigned int lastChannels;

    int bsize;
};
#endif // _KMAIN_WDIGET_H_
