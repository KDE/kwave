#ifndef _KWAVE_MENU_MANAGER_H_
#define _KWAVE_MENU_MANAGER_H_ 1

// #include <qobject.h>
#include <qwidget.h>
// #include <qlist.h>
// #include <qdict.h>

// #include "MenuNode.h"

class QWidget;
class KMenuBar;
class NumberedMenu;
class MenuRoot;

//*****************************************************************************
class MenuManager: public QObject
{
    Q_OBJECT

 public:
    MenuManager(QWidget *parent,KMenuBar &bar);
    ~MenuManager();

    void setCommand(const char *);

    NumberedMenu *findNumberedMenu (const char *);
    NumberedMenu *addNumberedMenu  (const char *);
    //deletes all entries of a numbered Menu
    void clearNumberedMenu         (const char *);
    //add Entrys to numbered Window
    void addNumberedMenuEntry      (const char *name,const char *entry);

    void selectItemChecked		(const char *id);
    void setItemChecked		(const char *id, bool check);
    void setItemEnabled		(const char *id, bool enable);

 signals:

 void command(const char *);

   // public slots:
   // void deliverCommand (const char *);

 private:
    int parseToKeyCode(const char *key_name);

    /** root node of the menu structure */
    MenuRoot *menu_root;
};

#endif
