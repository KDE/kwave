#ifndef _KWAVE_MENU_MANAGER_H_
#define _KWAVE_MENU_MANAGER_H_ 1

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
#include <qkeycode.h>
#include <kmenubar.h>
#include <qpopmenu.h>
#include "../libgui/kwavemenu.h"
#include "../lib/menuitem.h"

class NumberedMenu
{
 public:
  NumberedMenu          (const char *name);
  ~NumberedMenu         ();
  void       clear      ();
  void       notifyMenu (KwavePopMenu *menu);
  void       addEntry   (const char *name);
  void       refresh    ();
  const char *name      ();

 private:
  const char       *  objname;
  QList<char> entries;
  QList<KwavePopMenu> notifymenus;
};
//*****************************************************************************
class MenuManager:public QObject
{
 Q_OBJECT
 public:

 MenuManager	           (QWidget *parent,KMenuBar *);
 ~MenuManager	           ();
 
 void setCommand           (const char *);

 void appendMenus          (KwaveMenuItem *);            //adds menus
 void deleteMenus          (KwaveMenuItem *);            //deletes menus
 NumberedMenu *findNumberedMenu     (char *);            //return id
 bool addNumberedMenu      (char *);           
 void clearNumberedMenu    (char *);                     //delete all entries of a numbered Menu
 void addNumberedMenuEntry (char *name,char *entry);     //add Entrys to numbered Window

 signals:

 void command(const char *);

 public slots:

 void deliverCommand (const char *);

 protected:

 KwavePopMenu              *findMenu (const char *name);
 int  getIdRange           (int);

 private:

 KMenuBar            *bar;          //visible bar to which all toplevel menus
                                    //get attached
 QList<KwavePopMenu> toplevelmenus; //list of all top-level menus
 QList<NumberedMenu> numberedMenus; //list of special menus, that allow
                                    //dynamical appending (for presets,
                                    //file list,etc)
};
#endif






