#ifndef _TOP_WIDGET_H_
#define _TOP_WIDGET_H_ 1

#include <qlist.h>
#include <ktmainwindow.h>

class QCloseEvent;
class QStrList;
class QTimer;
class KCombo;
class KDNDDropZone;
class KToolBar;
class KStatusBar;
class MenuManager;
class MainWidget;
class PluginManager;
class SignalManager;
class KwaveApp;

class TopWidget : public KTMainWindow
{
    Q_OBJECT

public:

    /**
     * Constructor. Creates a new toplevel widget including menu bar,
     * buttons, working are an s on.
     * @param main_app reference to the main kwave aplication object
     * @param recent_files reference to the global list of recent files
     */
    TopWidget(KwaveApp &main_app, QStrList &recent_files);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /**
     * Destructor.
     */
    ~TopWidget();

    /**
     * Returns the reference to the Kwave application
     */
    inline KwaveApp &getKwaveApp() { return app; };

    void setSignal(const char *name);

    /**
     * Returns a reference to the current name of the signal. If no signal is
     * loaded the string is zero-length.
     */
    const QString &getSignalName();

    void setSignal(SignalManager *);
    void parseCommands(const char *);
    void loadBatch(const char *);

    /**
     * Returns a pointer to the current signal manager or zero if
     * no signal is loaded.
     */
    SignalManager *getSignalManager();

public slots:

    void executeCommand(const char *command);

    void dropEvent (KDNDDropZone *);

    void updateRecentFiles();

private slots:

    /** called on changes in the zoom selection combo box */
    void selectZoom(int index);

    /** called to set a new zoom factor */
    void setZoom(double zoom);

    /** updates all elements in the toolbar */
    void updateToolbar();

    /** called if the playback has started */
    void playbackStarted();

    /** called if the playback has been paused */
    void playbackPaused();

    /** called if the playback has been stopped or is done */
    void playbackStopped();

    /** toolbar: "file/new" */
    inline void toolbarFileNew()    { executeCommand("dialog(newsignal)"); };

    /** toolbar: "file/open" */
    inline void toolbarFileOpen()   { executeCommand("open () "); };

    /** toolbar: "file/save" */
    inline void toolbarFileSave()   { executeCommand("save () "); };

    /** toolbar: "file/save as.." */
    inline void toolbarFileSaveAs() { executeCommand("saveas () "); };

    /** toolbar: "edit/cut" */
    inline void toolbarEditCut()    { executeCommand("cut () "); };

    /** toolbar: "edit/copy" */
    inline void toolbarEditCopy()   { executeCommand("copy () "); };

    /** toolbar: "edit/paste" */
    inline void toolbarEditPaste()  { executeCommand("paste () "); };

    /** toolbar: "edit/erase" */
    inline void toolbarEditErase()  { executeCommand("zero () "); };

    /** toolbar: "edit/delete" */
    inline void toolbarEditDelete() { executeCommand("delete () "); };

signals:
    /**
     * Tells this TopWidget's parent to execute a command
     */
    void sigCommand(const char *command);

    /**
     * Emitted it the name of the signal has changed.
     */
    void sigSignalNameChanged(const QString &name);

protected:

    void updateMenu();

    /**
     * Sets a new caption of the this toplevel widget's window.
     * If a file is loaded, the caption is set to the
     * application's name + " - " + the filename. If no file is
     * loaded the caption consists only of the application's name.
     * @param filename path of the loaded file or 0 if no file loaded
     */
    virtual void setCaption(char *filename);

    /**
     * Loads a new file and updates the widget's title, menu, status bar
     * and so on.
     * @param filename path to the file to be loaded
     * @param type format of the file (WAV or ASCII)
     * @return 0 if successful
     */
    int loadFile(const char *filename, int type);

    void revert();
    void openFile();

    /**
     * Closes the current file and updates the menu and other controls.
     * If the file has been modified and the user wanted to break
     * the close operation, the file will not get closed and the
     * function returns with false.
     * @return true if closing is allowed
     */
    bool closeFile();

    void importAsciiFile();
    void exportAsciiFile();
    void openRecent (const char *str);
    void saveFile();
    void saveFileAs(bool selection = false);
    void resolution (const char *str);

private:
    /** reference to the main kwave application */
    KwaveApp &app;

    /** reference to the application's list of recent files */
    QStrList &recentFiles;

    /** our internal plugin manager */
    PluginManager *plugin_manager;

    /** caption of the main window */
    char *caption;

    QDir *saveDir;
    QDir *loadDir;

    /**
     * the main widget with all views and controls (except menu and
     * toolbar)
     */
    MainWidget *mainwidget;

    /** combo box for selection of the zoom factor */
    KCombo *m_zoomselect;

    /**
     * toolbar for controlling file operations, copy&paste,
     * playback and zoom
     */
    KToolBar *m_toolbar;

    /** reference to the main window's status bar */
    KStatusBar *status_bar;

    KDNDDropZone *dropZone;

    /** Name of the current signal or file. Zero-Length if nothing loaded */
    QString signalName;

    /** the window's menu bar */
    KMenuBar *menu_bar;

    /** menu manager for this window */
    MenuManager *menu;

    /** bits per sample to save with */
    int bits;

//    /**
//     * Set to "true" to indicate that the playback has only been
//     * paused and not stopped, so that the next press on the "play"
//     * button calls "continue" instead of "play from start".
//     */
//    bool m_playback_paused;
//
//    /**
//     * Timer used during playback to let some buttons blink..
//     */
//    QTimer *m_playback_timer;

    /** member id of the "start playback" toolbar button */
    int m_id_play;

    /** member id of the "start playback and loop" toolbar button */
    int m_id_loop;

    /** member id of the "pause playback" toolbar button */
    int m_id_pause;

    /** member id of the "stop playback" toolbar button */
    int m_id_stop;

    /** member id of the "zoom to selection" toolbar button */
    int m_id_zoomrange;

    /** member id of the "zoom in" toolbar button */
    int m_id_zoomin;

    /** member id of the "zoom out" toolbar button */
    int m_id_zoomout;

    /** member id of the "zoom to 100%" toolbar button */
    int m_id_zoomnormal;

    /** member id of the "zoom to all" toolbar button */
    int m_id_zoomall;

    /** member id of the "zoom factor" combobox in the toolbar */
    int m_id_zoomselect;

}
;

#endif // _TOP_WIDGET_H_
