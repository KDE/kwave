#ifndef _KWAVE_MENU_MANAGER_H_
#define _KWAVE_MENU_MANAGER_H_ 1

// #include <qobject.h>
#include <qwidget.h>
// #include <qlist.h>
// #include <qdict.h>
// #include <qkeycode.h>

// #include "MenuNode.h"

class QWidget;
class KMenuBar;
class NumberedMenu;
// class MenuRoot;

//*****************************************************************************
class MenuManager: public QObject
{
    Q_OBJECT

 public:
    MenuManager(QWidget *parent,KMenuBar &bar);
//    ~MenuManager();

//    void setCommand(const char *);

    NumberedMenu *findNumberedMenu (const char *) {return 0;};
    NumberedMenu *addNumberedMenu  (const char *) {return 0;};
    //deletes all entries of a numbered Menu
    void clearNumberedMenu         (const char *) {};
    //add Entrys to numbered Window
    void addNumberedMenuEntry      (const char *name,const char *entry) {};

    void selectItemChecked		(const char *id) {};
    void setItemChecked		(const char *id, bool check) {};
    void setItemEnabled		(const char *id, bool enable) {};

 signals:

 void command(const char *);

/*
// ### public slots:

   // void deliverCommand (const char *);

// protected:

// ### MenuNode                *findTopLevelMenu  (const char *name);
// ### void                registerID         (const char *id, MenuNode *menu);

// private:

 / ** root node of the menu structure * /
// MenuRoot *menu_root;

// ### KMenuBar            *bar;          //visible bar to which all toplevel menus
                                    //get attached
// ### QList<MenuNode>         toplevelmenus; //list of all top-level menus
// ### QList<NumberedMenu> numberedMenus; //list of special menus, that allow
                                    //dynamical appending (for presets,
                                    //file list,etc)
// ### QDict<MenuNode> menuIDs;               //for mapping string ids of menues
                                    //to references of menues
*/

};

#endif
