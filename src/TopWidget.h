#ifndef _TOP_WIDGET_H_
#define _TOP_WIDGET_H_ 1

#include <qlist.h>
#include <ktmainwindow.h>

class QCloseEvent;
class QStrList;
class MenuManager;
class MainWidget;
class PluginManager;
class SignalManager;
class KDNDDropZone;
class KStatusBar;
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

public slots:

    void executeCommand(const char *command);

    void dropEvent (KDNDDropZone *);

    void updateRecentFiles();

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
    MainWidget *mainwidget;

    /** reference to the main window's status bar */
    KStatusBar *status_bar;

    KDNDDropZone *dropZone;

    /** Name of the current signal or file. Zero-Length if nothing loaded */
    QString signalName;

    /** the window's menu bar */
    KMenuBar *menu_bar;

    /** menu manager for this window */
    MenuManager *menu;

    int bits;            //bit resolution to save with
}
;

#endif // _TOP_WIDGET_H_
