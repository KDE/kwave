#ifndef _KWAVE_DYNAMIC_LOADER_H_
#define _KWAVE_DYNAMIC_LOADER_H_ 1      

class DialogOperation;
class KwaveDialog;
//**********************************************************************
struct KwavePlugin
{
  public:

  KwavePlugin::KwavePlugin ();
  KwavePlugin::KwavePlugin (const char *name,const char *path);
  KwavePlugin::~KwavePlugin ();

  inline const char *getFileName () {return (filename);};
  inline const char *getName     () {return (name);};

  private:
  char* name;
  char* filename;
};
//**********************************************************************
class DynamicLoader 
{
 public:
  DynamicLoader::DynamicLoader  ();
  DynamicLoader::~DynamicLoader ();

  static  struct KwaveMenu* getMenufromPlugin (KwavePlugin *plugin);
  static  KwavePlugin**     getPlugins        (const char *dir);
  static  void              freePlugins       (KwavePlugin *plugins);
  static  KwaveDialog*      getDialog         (const char *,DialogOperation *);

 private:
};


#endif
