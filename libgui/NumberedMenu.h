#include <qlist.h>

class Menu;
class NumberedMenu
{
 public:
  NumberedMenu          (const char *name);
  ~NumberedMenu         ();
  void       clear      ();
  void       notifyMenu (Menu *menu);
  void       addEntry   (const char *name);
  void       refresh    ();
  const char *name      ();

 private:
  const char *objname;
  QList<char> entries;
  QList<Menu> notifymenus;
};
