#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

//**********************************************************
char *duplicateString (const char *str)
{
  char  *buf=0;
  if (str)
    {
      int   len=strlen(str);
      buf=new char [len+1];
      if (buf)
	{
	  for (int i=0;i<len;i++) buf[i]=str[i];
	  buf[len]=0;
	}
      else printf ("duplicateString allocation failed !\n");
    }
  return buf;
}
//**********************************************************
void deleteString (char *str)
{
  printf ("deleteString");
  if (str) delete [] str;
}
//**********************************************************
KwaveParser::KwaveParser (const char *init)
{
  printf ("parser allocated\n");
  done=false;
  len=strlen (init);
  str=duplicateString (init);
  ptr=0;
  ret=0;
}
//**********************************************************
KwaveParser::~KwaveParser ()
{
  //ret somehow gets corrupted...
  if (ret) deleteString (ret);  
  if (str) deleteString (str);
  printf ("parser deleted\n");
}
//**********************************************************
const char *KwaveParser::getCommand ()
{
  done=false;

  ptr=0;
  while (str[ptr]!='(') ptr++;

  if (ret) deleteString (ret);  
  ret=new char[ptr+1];
  if (ret)
    {
      strncpy (ret,str,ptr-1);
      ret[ptr]=0;   //terminate string
    }
  return ret;
}
//**********************************************************
const char *KwaveParser::getFirstParam ()
{
  //find first bracket, then call getNextParam....
  done=false;

  ptr=0;
  while (str[ptr]!='(') ptr++;
  ptr++;

  return getNextParam ();
}
//**********************************************************
const char *KwaveParser::getNextParam ()
{
  int  cnt=ptr;
  int  bracket=0;

  while ((str[cnt])&&(!done))
    {
      switch (str[cnt])
	{
	case ',':
	  if (bracket==0) done=true;
	  break;
	case '(':
	  bracket++;
	  break;
	case ')':
	  bracket--;
	  if (bracket<0) done=true;
	  break;      
	}
      cnt++;
    }

  //  this does crash while reading menus.config. don't know why yet
  //  this way it is a memory leak...
  if (ret) deleteString (ret);

  int len=cnt-ptr;
  if ((done)&&(len>0))
    {
      ret=new char[len];
      if (ret)
	{
	  memcpy (ret,&str[ptr],len-1);
	  ret[len-1]=0;   //terminate string
	  ptr=cnt;        //set pointer for next call to this function
	  if (ptr>=this->len-1) this->done=true;
	}
    }
  return ret;
}
//**********************************************************
void KwaveParser::skipParam ()
{
  const char *tmp;
  if (ptr) tmp=getNextParam ();
  else tmp=getFirstParam ();
}
//**********************************************************
bool KwaveParser::toBool (const char *match)
{
  const char *tmp;
  if (ptr) tmp=getNextParam ();
  else tmp=getFirstParam ();

  return (strcmp (tmp,match)==0);
}
//**********************************************************
int KwaveParser::toInt ()
{
  const char *tmp;
  if (ptr) tmp=getNextParam ();
  else tmp=getFirstParam ();

  return  atoi (tmp);
}
//**********************************************************
double KwaveParser::toDouble ()
{
  const char *tmp;

  if (ptr) tmp=getNextParam ();
  else tmp=getFirstParam ();
  return   strtod (tmp,0);
}
//**********************************************************
int KwaveParser::countParams ()
{
  int  cnt=ptr;
  int  numpar=0;
  int  bracket=0;
  while (str[cnt])
    {
      switch (str[cnt])
	{
	case '(':
	  numpar++;
	  break;
	case ')':
	  numpar--;
	  break;
	case ';':
	  if (numpar==0) return 0;
	  if (numpar<0) printf ("parse error wrong number of brackets!\n");
	  break;
	case ',':
	  if (numpar==0) bracket++; //increase number of parameters
	default: 
	  break;
	}
      cnt++;
    }
  return bracket;
}
//**********************************************************
bool KwaveParser::hasParams ()
{
  //not that sophisticated but works for a quick check...
  int  cnt=ptr;
  bool inbracket=false;
  while (str[cnt])
    {
      switch (str[cnt])
	{
	case '(':
	  inbracket=true;
	  break;
	case ' ':  //nothing to do for this ones
	case '\t': 
	case '\n': 
	  break;
	case ';': //end of command, no params found yet....
	  return false;
	  break;
	default: 
	  //something between ( ) found, must be a parameter
	  if (inbracket) return true;
	}
      cnt++;
    }
  return false;
}
//**********************************************************
LineParser::LineParser (const char *init)
{
  done=false;
  len=strlen (init);
  printf ("length is %d\n",len);
  str=duplicateString (init);
  ptr=0;
  ret=0;
}
//**********************************************************
LineParser::~LineParser ()
{
  if (ret) deleteString (ret);  
  if (str) deleteString (str);
}
//**********************************************************
const char *LineParser::getLine ()
{
  if (!done)
    {
      bool lineend=false;
      int  cnt=ptr;

      while ((str[cnt]==9) //skip white spaces
	     ||(str[cnt]==32)
	     ||(str[cnt]==10)
	     ||(str[cnt]==13)) cnt++;

      while ((str[cnt])&&(!lineend))
	{
	  switch (str[cnt])
	    {
	    case 10:
	    case 13:
	      lineend=true;
	      break;
	    }
	  cnt++;
	}

      int len=cnt-ptr;
      if ((lineend)&&(len))
	{
	  if (ret) deleteString (ret);
	  ret=new char[len];
	  if (ret)
	    {
	      memcpy (ret,&str[ptr],len-1);
	      ret[len-1]=0;   //terminate string
	      ptr=cnt;        //set pointer for next call to this function
	      if (ptr>=this->len) this->done=true;
	    }
	  return ret;
	}
    }
  return 0;   //failed to find another parameter substring...
}
