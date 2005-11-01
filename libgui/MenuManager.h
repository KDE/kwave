#ifndef _MENU_MANAGER_H_
#define _MENU_MANAGER_H_

#include "config.h"
#include <qwidget.h>

#include "mt/SignalProxy.h"

class QWidget;
class KMenuBar;
class MenuRoot;

//*****************************************************************************
/**
 * @class MenuManager
 * @brief Manager class for access to Kwave's menu subsystem.
 *
 * @note All commands must be emitted synchronously during X11 event
 *       processing instead of  immediately through normal signal
 *       handling. This avoids trouble when a signal handler within
 *       the MenuNode class causes an action that deletes that menu
 *       node. <em>It took me one week to find that bug!</em>
 */
class MenuManager: public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param parent the menu's parent widget
     * @param bar reference to the menu bar
     */
    MenuManager(QWidget *parent, KMenuBar &bar);

    /** Destructor. */
    virtual ~MenuManager();

    /**
     * Executes menu commands.
     * @param command string with the command
     */
    void executeCommand(const QString &command);

    /**
     * Deletes all entries of a numbered menu
     * @param uid unique id string of the numbered menu
     */
    void clearNumberedMenu(const QString &uid);

    /**
     * Add an entry to a numbered menu
     * @param uid unique id string of the numbered menu
     * @param entry name of the new entry (non-localized)
     */
    void addNumberedMenuEntry(const QString &uid, const QString &entry);

    /**
     * Selects an menu item within a group of menu items. All other
     * items will be deselected and the new one will become the only
     * selected one. (exclusive one-of-n-selection)
     * @param group name of the menu group
     * @param uid unique id string specifying the new selection
     */
    void selectItem(const QString &group, const QString &uid);

    /**
     * Checks/unchecks a menu node.
     * @param uid unique id string of the menu node
     * @param check true to set a checkmark, false to remove
     */
    void setItemChecked(const QString &uid, bool check);

    /**
     * Enables/disables a menu node.
     * @param uid unique id string of the menu node
     * @param enable true to enable, false to disable
     */
    void setItemEnabled(const QString &uid, bool enable);

    /**
     * Sets the text of a menu entry to a new value.
     * @param uid unique id string of the menu node
     * @param text the new text of the item
     */
    void setItemText(const QString &uid, const QString &text);

signals:

    /**
     * Will be emitted if the command of a menu node
     * should be executed.
     * @see #slotMenuCommand()
     * @see MenuNode.sigCommand()
     */
    void sigMenuCommand(const QString &command);

protected slots:

    /**
     * Enqueues a command from a menu entry into the internal
     * SignalProxy.
     * @see m_spx_command()
     * @see SignalProxy1
     */
    void slotEnqueueCommand(const QString &command);

    /**
     * Will be indirectly connected to the sigCommand() signal of the menu
     * structure's root node and gets called if a menu node's command
     * should be executed. Internally the commands are queued through
     * a SignalProxy to be executed during normal X11 event processing
     * after finishing the internal menu subsystem work.
     * @see MenuNode.sigCommand()
     * @see SignalProxy1
     */
    void slotMenuCommand();

private:
    /**
     * Translates a verbose key name into an integer representation.
     * @param key_name name of the key (upper case)
     * @return integer value
     * @see KAccel
     * @see kckey.h
     */
    int parseToKeyCode(const QString &key_name);

    /** root node of the menu structure */
    MenuRoot *m_menu_root;

    /** threadsafe message queue for emitted commands */
    SignalProxy1<QString> m_spx_command;

};

#endif // _MENU_MANAGER_H_
