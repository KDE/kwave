
#ifndef _KWAVE_APP_H_
#define _KWAVE_APP_H_

#include <qlist.h>
#include <qstrlist.h>

#include <kuniqueapp.h>
#include <kapp.h>

#include "SignalManager.h" // for struct playback_param_t

class ClipBoard;
class Plugin;
class QCloseEvent;
class QString;
class TopWidget;

class KwaveApp :public KApplication
{
    Q_OBJECT
public:

    KwaveApp();

    /**
     * Returns true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    virtual bool isOK();

    /** Destructor. */
    ~KwaveApp();

    /**
     * Returns the name of the application
     */
    QString appName() { return name(); };

    /**
     * Adds a file to the top of the list of recent files. If it was
     * already contained in the list the previous occourence is removed.
     * @param filename path to the file
     */
    void addRecentFile(const QString &filename);

    /**
     * Overwritten for unique application to open a new window.
     */
    virtual int newInstance();

    /**
     * Opens a new toplevel window. If a filename is specified the will
     * will be opened (should be a .wav-file).
     * @param filename name of the .wav file, optional
     * @return true if succeeded
     * @see #closeWindow()
     * @see TopWidget
     */
    bool newWindow(const QString *filename = 0);

    /**
     * Closes a previously opened toplevel window.
     * @param todel the toplevel window that closes down
     * @return true if it was the last toplevel window
     * @see #newWindow()
     * @see TopWidget
     */
    bool closeWindow(TopWidget *todel);

    /** Returns a reference to the list of recent files */
    inline QStrList &getRecentFiles() {
	return recentFiles;
    };

    /** Returns a reference to the current playback parameters */
    static inline playback_param_t &getPlaybackParams() {
	return playback_params;
    };

    /** Returns a reference to Kwave's clipboard */
    static ClipBoard &clipboard();

signals:
    /**
     * Will be emitted if the list of recent files has changed. Can
     * be used by toplevel widgets to update their menus.
     */
    void recentFilesChanged();

protected:
    friend class TopWidget;

    bool executeCommand(const QString &command);

protected:
    /**
     * Reads the configuration settings and the list of recent files,
     * opposite of saveConfig().
     * @see #saveConfig()
     */
    void readConfig();

    /**
     * Saves the list of recent files to the kwave configuration file
     * @see KConfig
     */
    void saveRecentFiles();

    /**
     * Saves the current configuration of kwave to the configuration file,
     * opposite of readConfig(). Also saves the list of recent files.
     * @see #saveRecentFiles()
     */
    void saveConfig();

private:

    /**
     * Local list of recent files. This list will be synchronized
     * with the global list of recent files stored in the libkwave
     * library whenever there is a change.
     */
    QStrList recentFiles;

    /** list of toplevel widgets */
    QList<TopWidget> topwidgetlist;

    /** parameters for audio playback */
    static playback_param_t playback_params;

    /** Kwave's clipboard */
    static ClipBoard &m_clipboard;
};

#endif // _KWAVE_APP_H_
