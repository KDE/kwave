#ifndef _KWAVE_MENU_MANAGER_H_
#define _KWAVE_MENU_MANAGER_H_ 1

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
#include <qdict.h>
#include <qkeycode.h>
#include <libkwave/MenuItem.h>
#include "../libgui/Menu.h"

class KMenuBar;
class NumberedMenu;

//*****************************************************************************
class MenuManager:public QObject
{
 Q_OBJECT
 public:

 MenuManager	                (QWidget *parent,KMenuBar *);
 ~MenuManager	                ();
 
 void setCommand                (const char *);

 NumberedMenu *findNumberedMenu (const char *);                  //return id
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

 public slots:

   // void deliverCommand (const char *);

 protected:

 Menu                *findTopLevelMenu  (const char *name);
 void                registerID         (const char *id, Menu *menu);

 private:

 KMenuBar            *bar;          //visible bar to which all toplevel menus
                                    //get attached
 QList<Menu>         toplevelmenus; //list of all top-level menus
 QList<NumberedMenu> numberedMenus; //list of special menus, that allow
                                    //dynamical appending (for presets,
                                    //file list,etc)
 QDict<Menu> menuIDs;               //for mapping string ids of menues
                                    //to references of menues

};
#endif







