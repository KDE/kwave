#include <qlist.h>

class MenuNode;
class NumberedMenu
{
 public:
  NumberedMenu          (const char *name);
  ~NumberedMenu         ();
  void       clear      ();
  void       notifyMenu (MenuNode *menu);
  void       addEntry   (const char *name);
  void       refresh    ();
  const char *name      ();

 private:
  const char *objname;
  QList<char> entries;
  QList<MenuNode> notifymenus;
};
