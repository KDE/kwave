
#include <stdio.h>

// #include <stdlib.h>
// #include <string.h>

// #include <qstack.h>
#include <qkeycode.h>

#include <kapp.h>
#include <klocale.h>

#include <libkwave/Parser.h>
#include <libkwave/String.h>

// #include "NumberedMenu.h"
#include "MenuNode.h"
#include "MenuRoot.h"
#include "MenuSub.h" // ###
#include "MenuManager.h"

//*****************************************************************************
MenuManager::MenuManager (QWidget *parent, KMenuBar &bar)
  :QObject(parent) , menu_root(*(new MenuRoot(bar)))
{
    debug("MenuManager::MenuManager(%p,%p)", parent, &bar);
}

//*****************************************************************************
int parseToKeyCode (char *key)
  //parse the key string into integer qt keycode
{
  int keycode=0;
  int len=strlen(key);
  int cnt=0;
  if (len==1) keycode=Key_A+key[0]-'A';
  else
    while (cnt<len)
      {
	int pos=cnt;
	while ((key[cnt])&&(key[cnt]!='+')) cnt++;		    
	if (cnt<len) key[cnt]=0;
	if (strlen(&key[pos])==1)
	  {
	    if ((key[pos]>='A')&&(key[pos]<='Z'))	keycode+=Key_A+key[pos]-'A';
	  }			  
	if (!strcmp (&key[pos],"PLUS")) keycode+=Key_Plus;
	if (!strcmp (&key[pos],"MINUS")) keycode+=Key_Minus;
	if (!strcmp (&key[pos],"SPACE")) keycode+=Key_Space;
	if (!strcmp (&key[pos],"CTRL")) keycode+=CTRL;
	if (!strcmp (&key[pos],"PAGEUP")) keycode+=Key_PageUp;
	if (!strcmp (&key[pos],"PAGEDOWN")) keycode+=Key_PageDown;
	if (!strcmp (&key[pos],"UP")) keycode+=Key_Up;
	if (!strcmp (&key[pos],"DEL")) keycode+=Key_Delete;
	if (!strcmp (&key[pos],"DOWN")) keycode+=Key_Down;
	if (!strcmp (&key[pos],"LEFT")) keycode+=Key_Left;
	if (!strcmp (&key[pos],"RIGHT")) keycode+=Key_Right;
	if (!strcmp (&key[pos],"SHIFT")) keycode+=SHIFT;

	cnt++;
      }
  return keycode;
}

void MenuManager::setCommand (const char *command)
{
    debug("MenuManager::setCommand(%s)",command); // ###

    Parser parser(command);

    const char *tmp;
    char *com=0;
    char *pos=0;
    char *key=0; // keyboard shortcut (optional)
    char *id=0;  // string id (optional)

    tmp=parser.getFirstParam ();
    if (tmp) com=duplicateString (tmp);

    tmp=parser.getNextParam ();
    if (tmp) pos=duplicateString (tmp);

    // bail out if no menu position is found
    if (!pos) {
	debug ("no position field !\n");
	if (com) delete com;
	return;
    }

    tmp=parser.getNextParam();
    if (tmp) key=duplicateString(tmp);

    tmp=parser.getNextParam();
    if (tmp) id=duplicateString(tmp);

    ((MenuNode*)(&menu_root))->insertNode((const char*)com, (char*)0, pos,
	(const char *)key, (const char *)id); // ###
/* ###

    debug("com='%s', pos='%s', key='%s', id='%s'", com, pos, key, id); // ###

    MenuNode *parentmenu=0;
    MenuNode *newmenu=0;
    int len=strlen(pos);
    int cnt;

  //parse the string pos into menu structure and insert accordingly
  for (cnt=0; cnt<len; cnt++)
    {
      int begin=cnt;
      while ((pos[cnt]) && (pos[cnt] != '/')) cnt++;
      if (cnt<len) pos[cnt]=0;

      if (parentmenu)
	{
	  if ((strncmp (&pos[begin],"listmenu",8)==0))
	    {
	      //create a numbered Menu and connect it to the parent
	      Parser getname(&pos[begin]);
	      NumberedMenu *newmenu=
	        addNumberedMenu(getname.getFirstParam());
	      if (newmenu)
	        {
// ###	          newmenu->notifyMenu (parentmenu);
// ###	          parentmenu->setCommand (com);
	        }
	      else debug ("creation of listmenu failed\n");

	      // reserve IDs for the entries
// ###	      MenuNode::getIdRange(MENUMAX);
	    }
	  else
/ * ###	  if ((strcmp (&pos[begin],"exclusive")==0))
	    parentmenu->setCheckable ();
	  else
	  if ((strcmp (&pos[begin],"number")==0))
	    parentmenu->setNumberable ();
	  else
* /
	  if (strcmp (&pos[begin],"separator")==0)
	    {
// ###	      parentmenu->insertSeparator();
	      cnt=len;
	    }
	  else
	    {
	      if (cnt+1>=len) // (end of tree / leaf)
		{
		  MenuNode *foundmenu=0; // ### parentmenu->findMenu(&pos[begin]);
		  if (foundmenu)
		    {
		      // menu already exists, this is only
		      // relevant for registering it's ID

		      // register the <string_id:menuItem> pair
// ###		      registerID(id, foundmenu);
		    }
                  else
		    {
		      // insert a new entry into the current parent menu
		      int keycode=0;
		      if (key) keycode=parseToKeyCode(key);

/ * ###
//		      debug("new entry='%s'", pos+begin); // ###
		      int numeric_id = parentmenu->insertEntry(
		        klocale->translate(&pos[begin]),
			com, keycode, MenuNode::getUniqueId());

		      // register the <string_id:menuItem> pair
		      MenuNode *entry = new MenuNode((const char *)0, numeric_id);
		      entry->setParent(parentmenu);
		      registerID(id, entry);
* /		    }
		}
	      else
		newmenu=0; // ### parentmenu->findMenu(&pos[begin]);
	    }
	}
      else //is a top-level menu ?
	newmenu=0; // ### this->findTopLevelMenu (&pos[begin]);

      if (!newmenu) // if menu is not already known, create a new menu
	{
// ###	  newmenu = new MenuSub(klocale->translate(&pos[begin]));
// ###	  debug("new menu='%s', id=%d", pos+begin, newmenu->getId()); // ###

// ###	  if (newmenu)
// ###	    {
//	      debug("new menu='%s' (parent=%p, id=%d)", pos+begin,parentmenu,newmenu->getId()); // ###

	      if (parentmenu)
		{
// ###		  int numeric_id = parentmenu->insertMenu(newmenu);
//		  debug("numeric_id (submenu) returned as %d",numeric_id);
// ###		  newmenu->setId(numeric_id);

		  // register the <string_id:menu> pair
// ###		  registerID(id, newmenu);
		}
	      else
		{
// ###		  newmenu = new MenuToplevel(klocale->translate(&pos[begin]));
		
		  //append to the list of top-level menus
// ###		  int numeric_id = bar->insertItem(
// ###		    klocale->translate(&pos[begin]),newmenu);
//		  debug("numeric_id (toplevel) returned as %d",numeric_id);
// ###		  newmenu->setId(numeric_id);
// ###		  newmenu->setTopLevel(true);
// ###		  toplevelmenus.append (newmenu);

		  // register the <string_id:menu> pair
// ###		  registerID(id, newmenu);
		}

//		connect (newmenu,SIGNAL(command(const char *)),
//		  this,SLOT(deliverCommand(const char *)));
// ###	    }
// ###	  else debug ("creation of menu failed !\n");
	}

	parentmenu=newmenu;
    }
*/

  if (com) deleteString(com);
  if (pos) deleteString(pos);
  if (key) deleteString(key);
  if (id)  deleteString(id);
}


//*****************************************************************************
void MenuManager::clearNumberedMenu (const char *name)
{
/* ###
  NumberedMenu *menu=findNumberedMenu (name);
  if (menu) menu->clear();
*/
}
//*****************************************************************************
NumberedMenu* MenuManager::addNumberedMenu (const char *name)
{/* ###
  debug("MenuManager::addNumberedMenu(%s)", name); // ###
  NumberedMenu *newmenu=findNumberedMenu (name);

  if (newmenu) return newmenu;

  newmenu=new NumberedMenu (name);
  numberedMenus.append (newmenu);
  return newmenu;  */
  return 0;
}
//*****************************************************************************
void MenuManager::addNumberedMenuEntry (const char *name,const char *entry)
{
/* ###
  NumberedMenu *menu=findNumberedMenu (name);
  if (menu) menu->addEntry (entry);
  else debug ("could not find numbered Menu %s\n",name);
*/
}
//*****************************************************************************
void MenuManager::selectItemChecked(const char *id)
{
/* ###
  debug("MenuManager::selectItemChecked('%s')", id);
  MenuNode *menu = menuIDs.find(id);
  if (menu)
    {
      MenuNode *parent = menu->getParent();
      if (parent) parent->checkEntry(menu->getId());
    }
*/
}
//*****************************************************************************
void MenuManager::setItemChecked(const char *id, bool check)
{
/* ###
  debug("MenuManager::setItemChecked('%s', %d)", id, check);
  MenuNode *menu = menuIDs.find(id);
  if (menu)
    {
      MenuNode *parent = menu->getParent();
      if (parent) parent->checkEntry(menu->getId(), check);
    }
*/
}
//*****************************************************************************
void MenuManager::setItemEnabled(const char *id, bool enable)
{
/* ###
  debug("MenuManager::setItemEnabled('%s', %d)", id, enable);
  MenuNode *menu = menuIDs.find(id);
  if (menu) menu->setEnabled(enable);
*/
}
//*****************************************************************************
NumberedMenu *MenuManager::findNumberedMenu (const char *name)
{
  //straight forward linear search
/* ###
  NumberedMenu *tmp=numberedMenus.first();
  while (tmp)
    {
      if (strcmp(tmp->name(),name)==0) return tmp;
      tmp=numberedMenus.next();
    }
### */
    return 0;
}
//*****************************************************************************
//void MenuManager::deliverCommand (const char *c)
//{
//  emit command(c);
//}
//*****************************************************************************
MenuManager::~MenuManager ()
{
// ###  toplevelmenus.clear();
// ###  numberedMenus.clear();
// ###  menuIDs.clear();
    menu_root.clear();
}

/* end of MenuManager.cpp */
