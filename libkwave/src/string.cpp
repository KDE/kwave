#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
  if (str) delete [] str;
}
//**********************************************************
char *mstotimec (int ms)
{
  static char buf[32];

  if (ms<10000)	sprintf (buf,"%d.%d ms",ms/10,ms%10);
  else
    {
      ms/=10;
      if (ms<60*1000)	sprintf (buf,"%d.%03d s",ms/1000,ms%1000);
      else if (ms<60*60*1000)	sprintf (buf,"%d:%02d.%02d m",
					 ms/(60*1000),			//minutes
					 (ms%(60*1000))/1000,		//seconds
					 (ms%1000)/10);			//ms
      else if (ms<24*60*60*1000)	sprintf (buf,"%d h %d:%d.%d m",
       					 ms/(60*60*1000),		//hours
	       				 ms%(60*60*1000)/(60*1000),	//minutes
		       			 (ms%(60*1000))/1000,		//seconds
			       		 (ms%1000)/10);			//ms
    }
  return (buf);
}
//*****************************************************************************
const char *catString (
const char *s1,
const char *s2,
const char *s3,
const char *s4,
const char *s5,
const char *s6,
const char *s7,
const char *s8
)
{
  int len=0;
  char *buf;

  if (s1) len+=strlen(s1);
  if (s2) len+=strlen(s2);
  if (s3) len+=strlen(s3);
  if (s4) len+=strlen(s4);
  if (s5) len+=strlen(s5);
  if (s6) len+=strlen(s6);
  if (s7) len+=strlen(s7);
  if (s8) len+=strlen(s8);

  buf=(char *)malloc (len+1);
  if (buf)
    {
      int pos=0;
      buf[len]=0;

      if (s1) memcpy (&buf[pos],s1,strlen(s1)),pos+=strlen(s1);
      if (s2) memcpy (&buf[pos],s2,strlen(s2)),pos+=strlen(s2);
      if (s3) memcpy (&buf[pos],s3,strlen(s3)),pos+=strlen(s3);
      if (s4) memcpy (&buf[pos],s4,strlen(s4)),pos+=strlen(s4);
      if (s5) memcpy (&buf[pos],s5,strlen(s5)),pos+=strlen(s5);
      if (s6) memcpy (&buf[pos],s6,strlen(s6)),pos+=strlen(s6);
      if (s7) memcpy (&buf[pos],s7,strlen(s7)),pos+=strlen(s7);
      if (s8) memcpy (&buf[pos],s8,strlen(s8)),pos+=strlen(s8);

      return buf;
    }
  return 0;
}
