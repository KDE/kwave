#ifndef _KWAVE_APP_H
#define _KWAVE_APP_H 1

#include <kapp.h>
#include <qlist.h>

class QCloseEvent;
class TopWidget;

class KwaveApp : public KApplication
{
    Q_OBJECT
public:

    KwaveApp(int argc, char **argv);

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    ~KwaveApp();

    /**
     * Adds a file to the top of the list of recent files. If it was
     * already contained in the list the previous occourence is removed.
     * @param filename path to the file
     */
    void addRecentFile(const char *filename);

//    void updateRecentFiles();

//    void parseCommands(const char *);

    /**
     * Opens a new toplevel window. If a filename is specified the will
     * will be opened (should be a .wav-file).
     * @param filename name of the .wav file, optional
     * @return true if succeeded
     * @see #closeWindow()
     * @see TopWidget
     */
    bool newWindow(const char *filename = 0);

    /**
     * Closes a previously opened toplevel window.
     * @param todel the toplevel window that closes down
     * @return true if it was the last toplevel window
     * @see #newWindow()
     * @see TopWidget
     */
    bool closeWindow(TopWidget *todel);

    inline QStrList &getRecentFiles() { return recentFiles; };

signals:
    /**
     * Will be emitted if the list of recent files has changed. Can
     * be used by toplevel widgets to update their menus.
     */
    void recentFilesChanged();

protected:
    friend class TopWidget;

    bool executeCommand(const char *str);

protected:
    void readConfig();

    /**
     * Saves the list of recent files to the kwave configuration file
     * @see KConfig
     */
    void saveRecentFiles();

    /**
     * Saves the current configuration of kwave to the configuration file.
     * This also includes saving the list of recent files.
     * @see #saveRecentFiles()
     */
    void saveConfig();

private:

    /**
     * Finds/creates configuration/plugin directories and stores them in
     * the globals struct.
     * @return true if successful
     */
    bool findDirectories();

    /**
     * Local list of recent files. This list will be synchronized
     * with the global list of recent files stored in the libkwave
     * library whenever there is a change.
     */
    QStrList recentFiles;

    QList<TopWidget> topwidgetlist;
};

#endif // _KWAVE_APP_H
