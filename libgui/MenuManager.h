#ifndef _MENU_MANAGER_H_
#define _MENU_MANAGER_H_ 1

#include <qwidget.h>

class QWidget;
class KMenuBar;
class NumberedMenu;
class MenuRoot;

//*****************************************************************************
class MenuManager: public QObject
{
    Q_OBJECT

 public:
    MenuManager(QWidget *parent,KMenuBar &bar);
    ~MenuManager();

    void setCommand(const char *);

    NumberedMenu *findNumberedMenu (const char *);
    NumberedMenu *addNumberedMenu  (const char *);
    //deletes all entries of a numbered Menu
    void clearNumberedMenu         (const char *);
    //add Entrys to numbered Window
    void addNumberedMenuEntry      (const char *name,const char *entry);

    void selectItemChecked(const char *uid);
    void setItemChecked(const char *uid, bool check);
    void setItemEnabled(const char *uid, bool enable);

 signals:

 void command(const char *);

   // public slots:
   // void deliverCommand (const char *);

 private:
    int parseToKeyCode(const char *key_name);

    /** root node of the menu structure */
    MenuRoot *menu_root;
};

#endif // _MENU_MANAGER_H_
