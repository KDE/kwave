#ifndef _KWAVE_MENU_ITEM_H_
#define _KWAVE_MENU_ITEM_H_ 1

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
//*****************************************************************************
//structure used by modules to allocate whole menus with submenus
struct MenuItem {
    int internalID;
    const char *command;
    const char *name;
    short int type;         //type of entry, either KITEM, KREF, KSEP ,KCHECK
    //or KEND
    int id;           //ITEM: id is set to -1 and will be retrieved or
    //      is fixed
    //MENU: id defines MULTIMENUTYPE
    int shortcut;     //shortcutkey for the menu
}
;
#endif








