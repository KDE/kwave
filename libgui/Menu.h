#ifndef _KWAVE_MENU_H_
#define _KWAVE_MENU_H_ 1

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
#include <qkeycode.h>
#include <kmenubar.h>
#include <qpopmenu.h>
#include <libkwave/MenuItem.h>

//maximum number of menuitem in numberedmenus, e.g. presets, channels
#define MENUMAX 100
//maximum number of top level menus...
#define TOPMENUMAX 16
class MenuCommand;
//*****************************************************************************
class Menu: public QPopupMenu
//class used to keep track of allocated menus
{
  Q_OBJECT
  public:
  Menu                             (const char *name,int id);
  ~Menu                            ();
  inline const char   *getName     () const {return name;};
  inline int           getId       () const {return id;};
  inline int           getMemberId () const {return memberId;};
  inline void          setMemberId (int id) {memberId=id;};
  static int           getUniqueId (); 
  static int           getIdRange  (int); 
         void          insertMenu  (Menu *);
	 void          setCommand  (const char *);
         int           insertEntry (const char *name,const char *com, int key);
         void          removeMenu  (const char *name);
         Menu         *findMenu    (const char *name);
         void          check       ();
         void          checkable   ();
         void          numberable  ();

 signals:

 void command (const char *);

 public slots:

 void          check       (int);
 void          selected    (int);

 private:
  QList<Menu>         children;          //list of pointers to children menus
  QList<MenuCommand>  commands;          //list of pointers to children menus
  int                 id;
  int                 memberId;          //id of members in the case items of
                                         //a numberedMenu are used in this
  char*               name;              //name of this menu as used internally
  char*               com;               //command template used if numbered
  int                 comcnt;            
  int                 checked;           //currently checked Item
  bool                numberItems;       //flag if command has to include
                                         //the menuitem id

  bool                checkItems;        //flag if menuitems may be checked, if
                                         //this is the case, they should also
                                         //be exclusiv
};
//*****************************************************************************
#endif
