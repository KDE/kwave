#ifndef _MENU_MANAGER_H_
#define _MENU_MANAGER_H_ 1

#include <qwidget.h>

class QWidget;
class KMenuBar;
class MenuRoot;

//*****************************************************************************
class MenuManager: public QObject
{
    Q_OBJECT

 public:
    /**
     * Constructor.
     * @param pointer to the node's parent (might be 0)
     * @param name the non-localized name of the node
     * @param command the command to be sent when the node is
     *                selected (optional, default=0)
     * @param key bitmask of the keyboard shortcut (see "qkeycode.h"),
     *            (optional, default=0)
     * @param uid unique id string (optional, default=0)
     */
    MenuManager(QWidget *parent,KMenuBar &bar);
    ~MenuManager();

    void setCommand(const char *);

    //deletes all entries of a numbered Menu
    void clearNumberedMenu         (const char *uid);
    //add Entrys to numbered Window
    void addNumberedMenuEntry      (const char *uid, char *entry);

    void selectItem(const char *group, const char *uid);
    void setItemChecked(const char *uid, bool check);
    void setItemEnabled(const char *uid, bool enable);

 signals:

 void command(const char *);

   // public slots:
   // void deliverCommand (const char *);

 private:
    int parseToKeyCode(const char *key_name);

    /** root node of the menu structure */
    MenuRoot *menu_root;
};

#endif // _MENU_MANAGER_H_
