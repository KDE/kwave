#include <qlist.h>
#include <string>
#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "globals.h"
#include "dynamicloader.h"

#define NODMODULES

extern Global globals;
extern KwavePlugin **timeplugins;
//**********************************************************************
KwavePlugin::KwavePlugin ()
{
  name=0;
  filename=0;
}
//**********************************************************************
KwavePlugin::KwavePlugin (const char *name,const char *filename)
{
  this->name=strdup (name);
  this->filename=strdup (filename);
}
//**********************************************************************
KwavePlugin::~KwavePlugin ()
{
  if (name) free (name); 
  if (filename) free (filename);
}
//**********************************************************************
DynamicLoader::DynamicLoader  ()
{
}
//**********************************************************************
DynamicLoader::~DynamicLoader ()
{
}
//**********************************************************************
KwavePlugin** DynamicLoader::getPlugins (const char *dirname)
  //scans the given directory for modules and returns description
{
  cout <<dirname<<endl;
  QList <KwavePlugin>plugins;
  DIR* dir = opendir(dirname);
  if (dir)
    {
      dirent* entry;

      /* read the directory's contents*/
      entry = readdir(dir);
      while (entry)
	{
	  string filename (dirname);
	  filename=(filename+"/");
	  filename+=entry->d_name;

	  //check for regular file -> filter directories (e.g. ./ ../)
	  struct stat fileinfo;
	  stat (filename.c_str(),&fileinfo);
#ifndef NOMODULES
	  if (S_ISREG(fileinfo.st_mode))
	    {

	      void *handle=dlopen (filename.c_str(),RTLD_LAZY);
	      if (handle)
		{
		  cout <<entry->d_name<<" : ";

		  const char **name=(const char **)dlsym (handle,"name");
		  const char **version=(const char **)dlsym (handle,"version");
		  const char **author=(const char **)dlsym (handle,"author");
		
		  cout <<*name<<" Version "<<*version<<" written by "<<*author<<endl;
		  plugins.append (new KwavePlugin (*name,filename.c_str()));
		  dlclose (handle);

		}
	      else
		cerr <<dlerror ()<<endl;
#endif
	    }
	  entry = readdir(dir);
	}
      closedir(dir);
    }
  else
    cerr <<"could not open module dir!"<<endl;

  if (!plugins.isEmpty())
    {
      KwavePlugin **tmp=new KwavePlugin *[plugins.count()+1];
      if (tmp)
	{
	  int cnt=0;
	  for (tmp[cnt]=plugins.first();tmp[cnt];tmp[++cnt]=plugins.next());
	}
      return tmp;
    }

  return 0;
}
//**********************************************************************
KwaveDialog* DynamicLoader::getDialog
(const char *name,DialogOperation *operation)
{
#ifndef NOMODULES
  if (globals.dialogplugins)
    {
      int cnt=0;
      bool done=false;

      if (globals.dialogplugins)
      while ((globals.dialogplugins[cnt])&&(!done))
        {
          if (strncmp(name,globals.dialogplugins[cnt]->getName(),5)==0)
	    {

	      void *handle=dlopen(globals.dialogplugins[cnt]->getFileName(),RTLD_NOW);
	      if (handle)
		{
		  KwaveDialog* (*modfunction)(DialogOperation *)=0;
		  const char *sym="getDialog__FP15DialogOperation"; 
		  //		  const char *sym=cplus_mangle_opname("getDialog(DialogOperation *)",0);
		  if (sym)
		    {
		      modfunction=(KwaveDialog *(*)(DialogOperation *))
			dlsym(handle,sym);

		      if (modfunction) return (*modfunction)(operation);
		      else printf ("%s\n",dlerror());
		    }
		  //dlclose (handle); must be handled, when dialog is done
		}
	      else printf ("%s\n",dlerror());

	      done=true;      
	    }
	  cnt++;
	}
    }
#endif
  return 0;
}
//**********************************************************************
void DynamicLoader::freePlugins (KwavePlugin *plugins)
{
  if (plugins) delete [] plugins;
}





