#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kwavestring.h"
#include "parser.h"

//**********************************************************
bool matchCommand (const char *com,const char *comp)
{
  KwaveParser parser(com);
  //printf ("comparing %s to %s\n",parser.getCommand(),comp);
  return (strcmp (parser.getCommand(),comp)==0);
}
//**********************************************************
KwaveParser::KwaveParser (const char *init)
{
  done=false;
  len=strlen (init);
  str=duplicateString (init);
  ptr=0;
  ret=0;
}
//**********************************************************
KwaveParser::~KwaveParser ()
{
  if (ret) deleteString (ret);  
  if (str) deleteString (str);
}
//**********************************************************
const char *KwaveParser::getCommand ()
{
  done=false;

  ptr=0;
  while ((str[ptr]==9) //skip white spaces
	 ||(str[ptr]==32)
	 ||(str[ptr]==10)
	 ||(str[ptr]==13)) ptr++;
  int  cnt=ptr;

  while (str[cnt]!='(') cnt++;
  cnt--;

  while ((str[cnt]==9) //skip white spaces
	 ||(str[cnt]==32)
	 ||(str[cnt]==10)
	 ||(str[cnt]==13)) cnt--;

  if (ret) deleteString (ret);  
  ret=new char[2+cnt-ptr];
  if (ret)
    {
      strncpy (ret,&str[ptr],1+cnt-ptr);
      ret[cnt-ptr+1]=0;   //terminate string
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
  bool foundend=false;

  while ((str[cnt])&&(!foundend))
    {
      switch (str[cnt])
	{
	case ',':
	  if (bracket==0) foundend=true;
	  break;
	case '(':
	  bracket++;
	  break;
	case ')':
	  bracket--;
	  if (bracket<0) foundend=true;
	  break;      
	}
      cnt++;
    }

  if (ret) deleteString (ret);
  ret=0;

  int len=cnt-ptr;
  if ((done==false)&&(len>0))
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
  int  numbr=0;
  int  paramcnt=0;
  while (str[cnt])
    {
      switch (str[cnt])
	{
	case '(':
	  numbr++;
	  if (str[cnt+1]!=')') paramcnt++;
	  break;
	case ')':
	  numbr--;
	  break;
	case ';':
	  if (numbr==1) return 0;
	  if (numbr!=1) printf ("parse error wrong number of brackets!\n");
	  break;
	case ',':
	  if (numbr==1) paramcnt++; //increase number of parameters if there
                                    //not subcommand
	default: 
	  break;
	}
      cnt++;
    }
  return paramcnt;
}
//**********************************************************
LineParser::LineParser (const char *init)
{
  done=false;
  len=strlen (init);
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

      while ((str[ptr]==9) //skip white spaces
	     ||(str[ptr]==32)
	     ||(str[ptr]==10)
	     ||(str[ptr]==13)) ptr++;
      int  cnt=ptr;

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
