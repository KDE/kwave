#include <stdlib.h>
#include <qstring.h>
#include <qfile.h>
#include "filter.h"
#include "parser.h"
#include "kwavestring.h"

//**********************************************************
Filter::Filter (const char *command)
{
  KwaveParser parse (command);
  comstr=0;

  this->rate=parse.toInt();
  this->fir=parse.toBool("fir");
  num=0;
  offset=0;
  mult=0;
  resize (parse.toInt());

  for (int i=0;i<num;i++)
    {
      offset[i]=parse.toInt();
      mult[i]=parse.toDouble();
    }
}
//**********************************************************
Filter::Filter (int rate)
{
  comstr=0;
  this->rate=rate;
  num=0;
  offset=0;
  mult=0;
}
//**********************************************************
Filter::~Filter ()
{
  if (comstr) delete comstr;
  if (offset) delete offset;
  if (mult) delete mult;
}
//**********************************************************
const char *Filter::getCommand ()
{
  char *tmpstr;
  if (comstr) delete comstr;
  char ratestr[128];
  char numstr[128];
  char buf[512];

  sprintf (ratestr,"%d",rate);
  sprintf (numstr,"%d",num);

  comstr=catString ("filter (",
		    ratestr,
		    ",",
		    fir?"fir":"iir",
		    ",",
		    numstr);

  tmpstr=comstr;
  for (int i=0;i<num;i++)
    {
      sprintf (buf,",%d,%f",offset[i],mult[i]);
      comstr=catString (tmpstr,buf);
      if (tmpstr) free (tmpstr);
      tmpstr=comstr;
    }

  tmpstr=comstr;
  comstr=catString (tmpstr,")");
  if (tmpstr) free (tmpstr);

  return comstr;
}
//**********************************************************
int  Filter::resize (int newnum)
{
  if (newnum!=num) //no need to resize arrays ....
    {
      int    *boffset=new int[newnum];
      double *bmult=new double[newnum];

      if (boffset&&bmult)
	{
	  for (int i=0;i<newnum;i++)  //initialize new arrays
	    {
	      bmult[i]=0;
	      boffset[i]=i;
	    }
	  bmult[0]=1;

	  int less=num;
	  if (newnum<num) less=newnum;

	  if (offset) //copy old values....
	    {
	      for (int i=0;i<less;i++) boffset[i]=offset[i];
	      delete offset;
	    }
	  if (mult)
	    {
	      for (int i=0;i<less;i++) bmult[i]=mult[i];
	      delete mult;
	    }

	  offset=boffset;
	  mult=bmult;
	  num=newnum;
	  return num;
	}
      else return false;
    }
  return newnum;
}
//**********************************************************
void Filter::save (const char *name1)
{
  char buf[80];
  QString name(name1);
  printf ("%s\n",name.data());
  if (name.find (".filter")==-1) name.append (".filter");
  QFile out(name.data());
  out.open (IO_WriteOnly);

  if (fir) sprintf (buf,"FIR %d\n",num);
  else sprintf (buf,"IIR %d\n",num);
  out.writeBlock (&buf[0],strlen(buf));

  for (int i=0;i<num;i++)
    {
      sprintf (buf,"%d %e\n",offset[i],((double)mult[i]));
      out.writeBlock (&buf[0],strlen(buf));
    }                                         
}
//**********************************************************
void Filter::load (const char *name1)
{
  char buf[120];
  QString name(name1);

  QFile *in=new QFile(name.data());
  if ((in)&&(in->open (IO_ReadOnly)))  
    {
      int res;
      res=in->readLine(buf,120);
      while ((res>0)&&((strncmp (buf,"FIR",3)!=0)&&(strncmp(buf,"IIR",3)!=0)))
	res=in->readLine(buf,120);

      if (res>0)
	{
	  if (strncmp (buf,"FIR",3)==0)
	      fir=true;
	  else
	      fir=false;

	  int newnum=strtol (&buf[4],0,0);
	  int x;
	  int cnt=0;
	  float y;

	  if (resize (newnum))
	    {

	      while ((in->readLine(buf,120)>0)&&(cnt<newnum))
		{
		  if ((buf[0]!='/')||(buf[0]!='#'))
		    {
		      sscanf (buf,"%d %f\n",&x,&y);
		      offset[cnt]=x;
		      mult[cnt++]=(int) (y);
		    }
		}
	    }
	}
    }
}
//**********************************************************
