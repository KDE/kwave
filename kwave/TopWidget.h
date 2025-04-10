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

#ifndef TOP_WIDGET_H
#define TOP_WIDGET_H

#include "config.h"

#include <QtGlobal>
#include <QMap>
#include <QMdiArea>
#include <QPointer>
#include <QString>
#include <QUrl>

#include <KMainWindow>

#include "libkwave/Sample.h"
#include "libkwave/String.h"

#include "App.h"
#include "FileContext.h"

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QMdiSubWindow;

namespace Kwave
{

    class FileContext;
    class MenuManager;
    class PlayerToolBar;
    class FileContext;
    class ZoomToolBar;

    /**
     * Toplevel widget of the Kwave application. Holds a main widget, a menu
     * bar, a status bar and a toolbar.
     */
    class Q_DECL_EXPORT TopWidget: public KMainWindow
    {
        Q_OBJECT

    public:

        /**
         * Constructor. Creates a new toplevel widget including menu bar,
         * buttons, working are an s on.
         * @param app reference to the Kwave application instance
         */
        explicit TopWidget(Kwave::App &app);

        /**
         * Does some initialization at startup of the instance
         *
         * @retval true if this instance was successfully initialized
         * @retval false if something went wrong during initialization
         */
        bool init();

        /**
         * Destructor.
         */
        ~TopWidget() override;

        /**
         * Returns a list of currently opened files
         * and their instance number
         */
        QList<Kwave::App::FileAndInstance> openFiles() const;

        /**
         * Returns the currently active file context (corresponds to a MDI
         * sub window or tab in MDI / TAB mode). In SDI mode, m_context_map
         * contains only one element, the current context (reachable per index
         * null).
         * @return pointer to a active FileContext within m_context_map
         */
        Kwave::FileContext *currentContext() const;

        /**
         * Detaches all file contexts from this instance
         * @return a list of Kwave::FileContext pointers (non-null)
         */
        QList<Kwave::FileContext *> detachAllContexts();

        /**
         * Insert a new file context into this instance
         * @param context the new file context
         */
        void insertContext(Kwave::FileContext *context);

        /**
         * Loads a new file and updates the widget's title, menu, status bar
         * and so on.
         * @param url URL of the file to be loaded
         * @return 0 if successful
         */
        int loadFile(const QUrl &url);

    public slots:

        /**
         * Updates the list of recent files in the menu, maybe some other
         * window has changed it. The list of recent files is static and
         * global in KwaveApp.
         */
        void updateRecentFiles();

        /**
         * Execute a Kwave text command
         * @param command a text command
         * @retval 0 if succeeded
         * @retval ENOSYS if the command is unknown in this component
         * @retval EBUSY if the command closes the widget (quit)
         * @retval negative error code if failed
         */
        int executeCommand(const QString &command);

        /**
         * forward a Kwave text command coming from an upper layer to
         * the currently active context below us (which is the main
         * entry point for all text commands)
         * @param command a text command
         * @retval 0 if succeeded
         * @retval negative error code if failed
         * @retval EAGAIN if there is no "current" context (yet)
         * @retval ENOSYS if the command is unknown in this component
         */
        int forwardCommand(const QString &command);

    protected slots:

        /** @see QWidget::closeEvent() */
        void closeEvent(QCloseEvent *e) override;

        /** @see Qt XDND documentation */
        void dragEnterEvent(QDragEnterEvent *event) override;

        /**
         * For dropping data into an empty signal
         * @see Qt XDND documentation
         */
        void dropEvent(QDropEvent *event) override;

    private slots:

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
         * Sets the descriptions of the last undo and redo action. If the
         * name is zero or zero-length, the undo / redo is currently not
         * available.
         */
        void setUndoRedoInfo(const QString &undo, const QString &redo);

        /** updates the menus when the clipboard has become empty/full */
        void clipboardChanged(bool data_available);

        /** Updates the menu by enabling/disabling some entries */
        void updateMenu();

        /** resets the toolbar layout to default settings */
        void resetToolbarToDefaults();

        /** updates all elements in the toolbar */
        void updateToolbar();

        void toolbarRecord()     { forwardCommand(_("plugin(record)")); }

        /** toolbar: "file/new" */
        void toolbarFileNew()    { forwardCommand(_("plugin(newsignal)")); }

        /** toolbar: "file/open" */
        void toolbarFileOpen()   { forwardCommand(_("open () ")); }

        /** toolbar: "file/save" */
        void toolbarFileSave()   { forwardCommand(_("save () ")); }

        /** toolbar: "file/save" */
        void toolbarFileSaveAs() { forwardCommand(_("saveas () ")); }

        /** toolbar: "file/save" */
        void toolbarFileClose()  { forwardCommand(_("close () ")); }

        /** toolbar: "edit/undo" */
        void toolbarEditUndo()   { forwardCommand(_("undo () ")); }

        /** toolbar: "edit/redo" */
        void toolbarEditRedo()   { forwardCommand(_("redo () ")); }

        /** toolbar: "edit/cut" */
        void toolbarEditCut()    { forwardCommand(_("cut () ")); }

        /** toolbar: "edit/copy" */
        void toolbarEditCopy()   { forwardCommand(_("copy () ")); }

        /** toolbar: "edit/paste" */
        void toolbarEditPaste()  { forwardCommand(_("paste () ")); }

        /** toolbar: "edit/erase" */
        void toolbarEditErase()  { forwardCommand(_("plugin(zero)")); }

        /** toolbar: "edit/delete" */
        void toolbarEditDelete() { forwardCommand(_("delete () ")); }

        /**
         * called if the signal now or no longer is modified
         */
        void modifiedChanged();

        /** shows a message/progress in the splash screen */
        void showInSplashSreen(const QString &message);

        /**
         * Show a message in the status bar
         * @param msg the status bar message, already localized
         * @param ms the time in milliseconds to show the message
         */
        void showStatusBarMessage(const QString &msg, unsigned int ms);

        /**
         * called when a MDI sub window or TAB has been activated
         * @param sub the sub window that has been activated
         */
        void subWindowActivated(QMdiSubWindow *sub);

        /**
         * called when a MDI sub window or TAB is about to be deleted
         * @param obj the sub window (not yet deleted)
         */
        void subWindowDeleted(QObject *obj);

    signals:

        /**
         * Emitted by us when the current file context has switched
         * @param context the new file context
         */
        void sigFileContextSwitched(Kwave::FileContext *context);

    private:

        /**
         * Opens a new empty window.
         * @param context reference to the pointer to the current context,
         *                can be modified in case that a new context has
         *                to be created for the new window!
         *                Must not be a null pointer
         * @param url URL of the file to be loaded (optional), used for
         *            opening a new SDI window
         * @retval -1 or negative in case of an error
         * @retval  0 if succeeded and done (SDI mode)
         * @retval  1 if succeeded but window is still empty (MDI or TAB mode)
         */
        int newWindow(Kwave::FileContext *&context, const QUrl &url);

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
         * Shows an "open file" dialog and opens the .wav file the
         * user has selected.
         * @return zero if succeeded or error code
         */
        int openFile();

        /**
         * Close all the currently opened sub windows
         * @return true if closing is allowed
         */
         bool closeAllSubWindows();

        /**
         * Opens a file contained in the list of recent files.
         * @param str the entry contained in the list
         * @return zero if succeeded, non-zero if failed
         */
        int openRecent(const QString &str);

        /** Updates the window caption */
        void updateCaption();

        /**
         * Establishes all signal/slot connections between a context and
         * this instance, including toolbars etc.
         */
        void connectContext(Kwave::FileContext *context);

        /**
         * Creates a new file context and initializes it.
         * @return the new file context or null pointer if
         *         creation or initialization failed
         */
        Kwave::FileContext *newFileContext();

    private:

        /** each TopWidget has exactly one corresponding Kwave::App instance */
        Kwave::App &m_application;

        /**
         * map for retrieving the file context that corresponds to
         * a MDI sub window or TAB. In SDI mode it contains only one
         * entry, corresponding to a null pointer as index.
         */
        QMap<QMdiSubWindow *, Kwave::FileContext *> m_context_map;

        /** toolbar with playback/record and seek controls */
        Kwave::PlayerToolBar *m_toolbar_record_playback = nullptr;

        /** toolbar with zoom controls */
        Kwave::ZoomToolBar *m_toolbar_zoom = nullptr;

        /** menu manager for this window */
        Kwave::MenuManager *m_menu_manager = nullptr;

        /**
         * MDI area, parent of all MDI child windows (only used in MDI and
         * TAB gui mode, null for SDI)
         */
        QMdiArea *m_mdi_area = nullptr;

        /** action of the "file save" toolbar button */
        QAction *m_action_save = nullptr;

        /** action of the "file save as..." toolbar button */
        QAction *m_action_save_as = nullptr;

        /** action of the "file close" toolbar button */
        QAction *m_action_close = nullptr;

        /** action of the "edit undo" toolbar button */
        QAction *m_action_undo = nullptr;

        /** action of the "edit redo" toolbar button */
        QAction *m_action_redo = nullptr;

        /** action of the "edit cut" toolbar button */
        QAction *m_action_cut = nullptr;

        /** action of the "edit copy" toolbar button */
        QAction *m_action_copy = nullptr;

        /** action of the "erase" toolbar button */
        QAction *m_action_erase = nullptr;

        /** action of the "edit delete" toolbar button */
        QAction *m_action_delete = nullptr;

        /** status bar label for length of the signal */
        QLabel *m_lbl_status_size;

        /** status bar label for mode information */
        QLabel *m_lbl_status_mode = nullptr;

        /** status bar label for cursor / playback position */
        QLabel *m_lbl_status_cursor = nullptr;
    };
}

#endif /* TOP_WIDGET_H */

//***************************************************************************
//***************************************************************************
