#include "fileloader.h"
#include <stdio.h>

FileLoader::FileLoader (const char *name)
{
  FILE *in=fopen (name,"r");
  if (in)
    {
      fseek (in,0,SEEK_END);
      int size=ftell (in);
      fseek (in,0,SEEK_SET);
      buf=new char [size+1];

      if (buf)
	{
	  buf[size]=0;
	  fread (buf,size,1,in);
	}
      else printf ("FileLoader:could not allocate memory for reading file !\n");      
    }
  else printf ("FileLoader:could not open file !\n");
}
//****************************************************************************
FileLoader::~FileLoader ()
{
  if (buf) delete [] buf;
}
