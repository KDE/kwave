#ifndef _MENU_MANAGER_H_
#define _MENU_MANAGER_H_ 1

#include <qwidget.h>

class QWidget;
class KMenuBar;
class MenuRoot;

//*****************************************************************************
class MenuManager: public QObject {
    Q_OBJECT

public:
    /**
     * Constructor.
     * @param parent the menu's parent widget
     * @param bar reference to the menu bar
     */
    MenuManager(QWidget *parent, KMenuBar &bar);

    /**
     * Destructor.
     */
    ~MenuManager();

    /**
     * Executes menu commands.
     * @param command string with the command
     */
    void executeCommand(const char *command);

    /**
     * Deletes all entries of a numbered menu
     * @param uid unique id string of the numbered menu
     */
    void clearNumberedMenu(const char *uid);

    /**
     * Add an entry to a numbered menu
     * @param uid unique id string of the numbered menu
     * @param entry name of the new entry (non-localized)
     */
    void addNumberedMenuEntry(const char *uid, char *entry);

    /**
     * Selects an menu item within a group of menu items. All other
     * items will be deselected and the new one will become the only
     * selected one. (exclusive one-of-n-selection)
     * @param group name of the menu group
     * @param uid unique id string specifying the new selection
     */
    void selectItem(const char *group, const char *uid);

    /**
     * Checks/unchecks a menu node.
     * @param uid unique id string of the menu node
     * @param check true to set a checkmark, false to remove
     */
    void setItemChecked(const char *uid, bool check);

    /**
     * Enables/disables a menu node.
     * @param uid unique id string of the menu node
     * @param enable true to enable, false to disable
     */
    void setItemEnabled(const char *uid, bool enable);

signals:

    /**
     * Will be emitted if the command of a menu node
     * should be executed.
     * @see #slotMenuCommand()
     * @see MenuNode.sigCommand()
     */
    void sigMenuCommand(const char *command);

private slots:

    /**
     * Will be connected to the sigCommand() signal of the menu
     * structure's root node and gets called if a menu node's command
     * should be executed.
     * @see MenuNode.sigCommand()
     */
    void slotMenuCommand(const char *command);

private:
    /**
     * Translates a verbose key name into an integer representation.
     * @param key_name name of the key (upper case)
     * @return integer value
     * @see KAccel
     * @see kckey.h
     */
    int parseToKeyCode(const char *key_name);

    /** root node of the menu structure */
    MenuRoot *menu_root;
};

#endif // _MENU_MANAGER_H_
