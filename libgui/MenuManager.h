#ifndef _KWAVE_MENU_MANAGER_H_
#define _KWAVE_MENU_MANAGER_H_ 1

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
#include <qkeycode.h>
//#include <qpopmenu.h>
#include "../libgui/Menu.h"
#include <libkwave/MenuItem.h>

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

 void appendMenus               (MenuItem *);               //adds menus
 void deleteMenus               (MenuItem *);               //delete menus
 NumberedMenu *findNumberedMenu (const char *);                  //return id
 NumberedMenu *addNumberedMenu  (const char *);           
 void clearNumberedMenu         (const char *);
 //deletes all entries of a numbered Menu
 void addNumberedMenuEntry      (const char *name,const char *entry);
 //add Entrys to numbered Window
 void checkMenuEntry		(const char *name,bool check);

 signals:

 void command(const char *);

 public slots:

   // void deliverCommand (const char *);

 protected:

 Menu                *findMenu (const char *name);
 int                  getIdRange (int);

 private:

 KMenuBar            *bar;          //visible bar to which all toplevel menus
                                    //get attached
 QList<Menu>         toplevelmenus; //list of all top-level menus
 QList<NumberedMenu> numberedMenus; //list of special menus, that allow
                                    //dynamical appending (for presets,
                                    //file list,etc)
};
#endif







