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
  Menu                             (const char *name,int id, bool toplevel=false);
  ~Menu                            ();
  inline Menu         *getParent   () { return parent_menu;};
  inline void          setParent   (Menu *parent) { this->parent_menu = parent;};
  inline const char   *getName     () const {return name;};
  inline bool          isTopLevel  () { return toplevel;};
  inline void          setTopLevel (bool toplevel) { this->toplevel=toplevel;};
  inline int           getId       () const {return id;};
  inline void          setId       (int id) {this->id=id;};
  static int           getUniqueId ();
  static int           getIdRange  (int); 
         int           insertMenu  (Menu *);
         int           insertEntry (const char *name,const char *com,
                                    int key, int id);
	 void          setCommand  (const char *);
         void          removeMenu  (const char *name);
         Menu         *findMenu    (const char *name);
         void          setEnabled  (const bool enable);
         void          setTopLevelEnabled(const bool enable);
         void          checkEntry  (const int id);
         void          checkEntry  (const int id, const bool check);
         int           getCheckedId() { return checked; };
         void          setCheckedId(const int id) { this->checked = id;};
         void          checkable   ();
         void          numberable  ();

 signals:

 void command (const char *);

 public slots:

 void          check       (int);
 void          selected    (int);
 void          hilight     (int);

 private:
  Menu                *parent_menu;
  QList<Menu>         children;          //list of pointers to children menus
  QList<MenuCommand>  commands;          //list of pointers to children menus
  bool                toplevel;          //true for toplevel menu
  bool                toplevelEnabled;   //toplevel menu is enabled
  int                 id;
  char*               name;              //name of this menu as used internally
  char*               com;               //command template used if numbered
  int                 comcnt;
  bool                enabled;           //entry enabled, independend from toplevel
  int                 checked;           //currently checked Item
  bool                numberItems;       //flag if command has to include
                                         //the menuitem id

  bool                checkItems;        //flag if menuitems may be checked, if
                                         //this is the case, they should also
                                         //be exclusive
};
//*****************************************************************************
#endif
