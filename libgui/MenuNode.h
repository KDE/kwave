/***************************************************************************
                          MenuNode.h  -  description
                             -------------------
    begin                : Mon Jan 10 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MENUNODE_H
#define MENUNODE_H

#include <qobject.h>
#include <qwidget.h>
#include <qlist.h>
// #include <qkeycode.h>
// #include <kmenubar.h>
// #include <qpopmenu.h>

//maximum number of menuitem in numberedmenus, e.g. presets, channels
#define MENUMAX 100
//maximum number of top level menus...
#define TOPMENUMAX 16
class MenuCommand;

/**
 * This is the base class for the MenuRoot, MenuEntry, SubMenu and
 * the ToplevelMenu class.
 * @author Thomas Eschenbacher
 */
class MenuNode: public QObject
{
    Q_OBJECT

public:
    MenuNode(const char *command, const char *name);
    virtual ~MenuNode();

    inline const char   *getName() const {return name;};

    inline int           getId()       {return this->id;};
    inline void          setId(int id) {this->id=id;};

    virtual int getIndex() {return -1;};
    virtual int getChildIndex(const int id);

    virtual bool isBranch() {return false;};

/* ###
  inline bool          isTopLevel  () { return toplevel;};
  inline void          setTopLevel (bool toplevel) { this->toplevel=toplevel;};

         void          setTopLevelEnabled(bool enable);

  static int           getUniqueId ();
  static int           getIdRange  (int);

         int           insertMenu  (MenuNode *);
         int           insertEntry (const char *name,const char *com,
                                    int key, int id);
         void          removeMenu  (const char *name);
         MenuNode         *findMenu    (const char *name);

	 void          setCommand  (const char *);

         void          setEnabled  (bool enable);

         void          checkEntry  (int id);
         void          checkEntry  (int id, const bool check);
         int           getCheckedId() { return checked; };
         void          setCheckedId(int id) { this->checked = id;};
         void          setCheckable ();

         void          setNumberable();
### */

    /** removes all entries from the menu */
    virtual void clear();

    /** sets a new pointer to the node's parent */
    virtual void setParent(MenuNode *newParent);

    /** returns a pointer to the menu's parent node */
    virtual MenuNode * getParentNode();

    /** Returns the number of ids a menu needs */
    virtual int getNeededIDs();

    /** Enables or disables an item of the current menu */
    virtual bool setItemEnabled(const int item, const bool enable);

    /** Tries to find a child node by it's name */
    MenuNode *findChild(const char *name);


    MenuNode *findChild(const int id);

    virtual void removeChild(const int id);

    virtual MenuNode *insertBranch(char *name, const int key,
                                   const char *uid, const int index=-1);

    virtual MenuNode *insertLeaf(const char *command, char *name,
                                 const int key, const char *uid,
                                 const int index=-1);

    /** Inserts a new child node into the current menu node. */
    virtual int registerChild(MenuNode *node);

    /** Inserts a new child node into the structure (also children) */
    virtual int insertNode(const char *command, char *name, char *position,
	                   const int key, const char *uid);

    virtual bool specialCommand(const char *command);

    virtual void actionSelected();

 signals: // Signals

 void sigCommand(const char *);

private:

  QList<MenuNode>     children;          //list of pointers to children menus

/* ###
  QList<MenuCommand>  commands;
  bool                toplevel;          //true for toplevel menu
  bool                toplevelEnabled;   //toplevel menu is enabled
*/

  int   id;
  char *name;    // name of this menu node non-localized
  char *command; // command, might be null

/* ###
  char*               com;               //command template used if numbered
  int                 comcnt;
  bool                enabled;           //entry enabled, independend from toplevel
  int                 checked;           //currently checked Item
  bool                numberItems;       //flag if command has to include
                                         //the menuitem id

  bool                checkItems;        //flag if menuitems may be checked, if
                                         //this is the case, they should also
                                         //be exclusive
*/

    /** parent of this entry */
    MenuNode* parentNode;

};

#endif
