#ifndef _KWAVEMENUMANAGE_H_
#define _KWAVEMENUMANAGE_H_ 1

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
#include <qkeycode.h>
#include <kmenubar.h>

//maximum number of menuitem in numberedmenus, e.g. presets, channels
#define MENUMAX 100
//maximum number of top level menus...
#define TOPMENUMAX 16

#define KMENU  -1 //a menu
#define KITEM  -2 //a single menu entry
#define KEND   -3 //end of an menu
#define KSEP   -4 //inserts one of those separators. use only in front of
                  //a KITEM, when the surounding part of the menu should
                  //be deleted again.
#define KREF   -5 //All entries of the NumberedMenu given by the name,
                  //will be placed at this point.
#define KCHECK -6 //a menu, which entries are checkable. only one of the
                  // entries may have the checkmark at a time

#define KCHANNEL -4
#define KNAME    -5
#define KFILTER  -6
#define KMARKER  -7

#define KEXCLUSIVE -2
#define KSHARED    -1

class KWavePopMenu: public QPopupMenu
//class used to keep track of allocated menus
{
  Q_OBJECT
  public:
  KWavePopMenu             (const char *name,int id,bool checkItems=false);
  ~KWavePopMenu            ();
 const char   *getName     ();
 int           getId       ();
 int           getMemberId ();
 void          setMemberId (int);
 void          insertMenu  (KWavePopMenu *);
 void          removeMenu  (const char *name);
 KWavePopMenu *findMenu    (const char *name);
 void          check       ();
 public slots:

 void          check       (int);

 private:
  QString             *name;             //name as used internally
  QList<KWavePopMenu> children;          //list of pointers to children menus
  int                 id;
  int                 memberId;          //id of members in the case items of
                                         //a numberedMenu are used in this
  int                 checked;           //currently checked Item
  bool                checkItems;        //flag 
};

struct KWaveMenuItem
//structure used by clients to allocate whole menus with submenus
{
  int        internalID;
  const char *name;
  short int  type;       //type of entry, either KITEM, KREF, KSEP ,KCHECK
                         //or KEND
  int        id;         //ITEM: id is set to -1 and will be retrieved or
                         //      is fixed
                         //MENU: id defines MULTIMENUTYPE
  int        shortcut;   //shortcutkey for the menu
};
//*****************************************************************************
class NumberedMenu
{
 public:
  NumberedMenu (char *name);
  ~NumberedMenu();
  void clear ();
  void notifyMenu (KWavePopMenu *menu);
  void addEntry (char *name);
  void refresh  ();
  char *name    ();

 private:
  QString *objname;
  QList<QString> entries;
  QList<KWavePopMenu> notifymenus;
};
//*****************************************************************************
class MenuManager:public QObject
{
 Q_OBJECT
 public:

 MenuManager	           (QWidget *parent,KMenuBar *);
 ~MenuManager	           ();
 void appendMenus          (KWaveMenuItem *);            //adds menus
 void deleteMenus          (KWaveMenuItem *);            //deletes menus
 NumberedMenu *findNumberedMenu     (char *);            //return id
 bool addNumberedMenu      (char *);                     //special type of menu, where all Items are numbered
 void clearNumberedMenu    (char *);                     //delete all entries of a numbered Menu
 void addNumberedMenuEntry (char *name,char *entry);     //add Entrys to numbered Window
 int  translateId          (KWaveMenuItem *,int );       //returns internal id, if found

 signals:
 void id(int);

 public slots:
 void map(int);

 protected:

 KWavePopMenu              *findMenu (const char *name);
 int  getUniqueId          ();
 int  getIdRange           (int);

 private:

 KMenuBar *bar;
 QList<KWavePopMenu> menus;
 QList<NumberedMenu> numberedMenus;
};
#endif






