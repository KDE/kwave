/***************************************************************************
            TopWidget.h  -  Toplevel widget of Kwave
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

#ifndef _TOP_WIDGET_H_
#define _TOP_WIDGET_H_

#include "config.h"

#include <QPair>
#include <QPointer>
#include <QString>

#include <kdemacros.h>
#include <kmainwindow.h>
#include <kurl.h>

#include "libkwave/Sample.h"

#include "libgui/MouseMark.h"

class QCloseEvent;
class QLabel;
class QTextStream;
class QTimer;

class KCombo;
class KComboBox;
class KDNDDropZone;
class KStatusBar;

class KwaveApp;
class MenuManager;
class MainWidget;
class SignalManager;

namespace Kwave { class PlayerToolBar; }
namespace Kwave { class PluginManager; }
namespace Kwave { class ApplicationContext; }

/**
 * Toplevel widget of the Kwave application. Holds a main widget, a menu
 * bar, a status bar and a toolbar.
 */
class TopWidget : public KMainWindow
{
    Q_OBJECT

public:

    /**
     * Constructor. Creates a new toplevel widget including menu bar,
     * buttons, working are an s on.
     * @param context reference to the context of this instance
     */
    TopWidget(Kwave::ApplicationContext &context);

    /**
     * Does some initialization at startup of the instance
     * @return true if this instance was successfully initialized, or
     * false if something went wrong during initialization.
     */
    bool init();

    /**
     * Destructor.
     */
    virtual ~TopWidget();

    /**
     * Loads a new file and updates the widget's title, menu, status bar
     * and so on.
     * @param url URL of the file to be loaded
     * @return 0 if successful
     */
    int loadFile(const KUrl &url);

    /**
     * Loads a batch file into memory, parses and executes
     * all commands in it.
     * @param url URL of the macro (batch file) to be loaded
     */
    int loadBatch(const KUrl &url);

public slots:

    /**
     * Updates the list of recent files in the menu, maybe some other
     * window has changed it. The list of recent files is static and
     * global in KwaveApp.
     */
    void updateRecentFiles();

protected slots:

    /** @see QWidget::closeEvent() */
    virtual void closeEvent(QCloseEvent *e);

private slots:

    /**
     * Execute a Kwave text command
     * @param command a text command
     * @return zero if succeeded or negative error code if failed
     * @retval -ENOSYS is returned if the command is unknown in this component
     */
    int executeCommand(const QString &command);

    /** called on changes in the zoom selection combo box */
    void selectZoom(int index);

    /**
     * Called if a new zoom factor has been set in order to update
     * the status display and the content of the zomm selection
     * combo box.
     * @note This method can not be called to *set* a new zoom factor.
     */
    void setZoomInfo(double zoom);

    /**
     * Called when the meta data of the current signal has changed
     * @param meta_data the new meta data, after the change
     */
    void metaDataChanged(Kwave::MetaDataList meta_data);

    /**
     * Updates the number of selected samples in the status bar.
     * @param offset index of the first selected sample
     * @param length number of selected samples
     */
    void selectionChanged(sample_index_t offset, sample_index_t length);

    /**
     * updates the playback position in the status bar
     * @param offset the current playback position [samples]
     */
    void updatePlaybackPos(sample_index_t offset);

    /**
     * Sets the descriptions of the last undo and redo action. If the
     * name is zero or zero-length, the undo / redo is currently not
     * available.
     */
    void setUndoRedoInfo(const QString &undo, const QString &redo);

    /**
     * Updates the status bar's content depending on the current status
     * or position of the mouse cursor.
     */
    void mouseChanged(Kwave::MouseMark::Mode mode);

    /** updates the menus when the clipboard has become empty/full */
    void clipboardChanged(bool data_available);

    /** Updates the menu by enabling/disabling some entries */
    void updateMenu();

    /** resets the toolbar layout to default settings */
    void resetToolbarToDefaults();

    /** updates all elements in the toolbar */
    void updateToolbar();

    void toolbarRecord()    { executeCommand("plugin(record)"); }

    /** toolbar: "file/new" */
    void toolbarFileNew()    { executeCommand("plugin(newsignal)"); }

    /** toolbar: "file/open" */
    void toolbarFileOpen()   { executeCommand("open () "); }

    /** toolbar: "file/save" */
    void toolbarFileSave()   { executeCommand("save () "); }

    /** toolbar: "edit/undo" */
    void toolbarEditUndo()   { executeCommand("undo () "); }

    /** toolbar: "edit/redo" */
    void toolbarEditRedo()   { executeCommand("redo () "); }

    /** toolbar: "edit/cut" */
    void toolbarEditCut()    { executeCommand("cut () "); }

    /** toolbar: "edit/copy" */
    void toolbarEditCopy()   { executeCommand("copy () "); }

    /** toolbar: "edit/paste" */
    void toolbarEditPaste()  { executeCommand("paste () "); }

    /** toolbar: "edit/erase" */
    void toolbarEditErase()  { executeCommand("plugin(zero)"); }

    /** toolbar: "edit/delete" */
    void toolbarEditDelete() { executeCommand("delete () "); }

    /** called if the signal now or no longer is modified */
    void modifiedChanged(bool);

    /** shows a message/progress in the splash screen */
    void showInSplashSreen(const QString &message);

signals:
    /**
     * Tells this TopWidget's parent to execute a command
     */
    void sigCommand(const QString &command);

    /**
     * Emitted it the name of the signal has changed.
     */
    void sigSignalNameChanged(const QString &name);

private:

    /**
     * Closes the current file and creates a new empty signal.
     * @param samples number of samples per track
     * @param rate sample rate
     * @param bits number of bits per sample
     * @param tracks number of tracks
     * @return zero if successful, -1 if failed or canceled
     */
    int newSignal(sample_index_t samples, double rate,
                  unsigned int bits, unsigned int tracks);

    /**
     * Discards all changes to the current file and loads
     * it again.
     */
    int revert();

    /**
     * Shows an "open file" dialog and opens the .wav file the
     * user has selected.
     */
    int openFile();

    /**
     * Closes the current file and updates the menu and other controls.
     * If the file has been modified and the user wanted to break
     * the close operation, the file will not get closed and the
     * function returns with false.
     * @return true if closing is allowed
     */
    bool closeFile();

    /**
     * Saves the current file.
     * @return zero if succeeded, non-zero if failed
     */
    int saveFile();

    /**
     * Opens a dialog for saving the current .wav file.
     * @param selection if set to true, only the current selection
     *        will be saved
     * @return zero if succeeded, non-zero if failed
     */
    int saveFileAs(bool selection = false);

    /**
     * Opens a file contained in the list of recent files.
     * @param str the entry contained in the list
     * @return zero if succeeded, non-zero if failed
     */
    int openRecent(const QString &str);

    /** Updates the caption with the filename */
    void updateCaption();

    /**
     * Parses a text stream line by line and executes each line
     * as a command until all commands are done or the first one fails.
     * @param stream a QTextStream to read from
     * @return zero if successful, non-zero error code if a command failed
     */
    int parseCommands(QTextStream &stream);

    /** returns true if we have a non-empty signal */
    bool haveSignal();

    /** returns the name of the signal */
    QString signalName() const;

private:

    /** reference to the application context of this instance */
    Kwave::ApplicationContext &m_context;

    /**
     * the main widget with all views and controls (except menu and
     * toolbar)
     */
    QPointer<MainWidget> m_main_widget;

    /** toolbar with playback/record and seek controls */
    Kwave::PlayerToolBar *m_toolbar_record_playback;

    /** combo box for selection of the zoom factor */
    KComboBox *m_zoomselect;

    /** menu manager for this window */
    MenuManager *m_menu_manager;

    /** action of the "edit undo" toolbar button */
    QAction *m_action_undo;

    /** action of the "edit redo" toolbar button */
    QAction *m_action_redo;

    /** action of the "zoom to selection" toolbar button */
    QAction *m_action_zoomselection;

    /** action of the "zoom in" toolbar button */
    QAction *m_action_zoomin;

    /** action of the "zoom out" toolbar button */
    QAction *m_action_zoomout;

    /** action of the "zoom to 100%" toolbar button */
    QAction *m_action_zoomnormal;

    /** action of the "zoom to all" toolbar button */
    QAction *m_action_zoomall;

    /** action of the "zoom factor" combobox in the toolbar */
    QAction *m_action_zoomselect;

    /** status bar label for length of the signal */
    QLabel *m_lbl_status_size;

    /** status bar label for mode information */
    QLabel *m_lbl_status_mode;

    /** status bar label for cursor / playback position */
    QLabel *m_lbl_status_cursor;

};

#endif /* _TOP_WIDGET_H_ */
