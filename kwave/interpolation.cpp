#include "interpolation.h"
#include "curvewidget.h"

#define INT_COUNT 7
const char *NAMES[]={"Linear","Spline","N-Polynom","3-Polynom","5-Polynom","7-Polynom","Sample and Hold",NULL};

void get2Derivate (double *x, double *y, double *ab,int n)
{
  int 	i, k;
  double p, qn, sig, un;

  double u[n];

  ab[1]=0;
  u[1]=0;

  for (i=2; i<n; i++)
    {
      sig = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
      p   = sig*ab[i-1] + 2;
      ab[i] = (sig-1)/p;
      u[i] = (y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
      u[i] = (6*u[i]/(x[i+1]-x[i-1]) - sig*u[i-1])/p;
    }

  qn=0;
  un=0;
  ab[n] = (un-qn*u[n-1])/(qn*ab[n-1]+1);

  for (k=n-1; k>0; k--)  ab[k] = ab[k]*ab[k+1]+u[k];
}
//****************************************************************************
void createPolynom  (QList<CPoint> *points,double x[],double y[],int pos,int degree)
{
  CPoint *tmp;
  int count=0;

  if (pos<0)
    {
      switch (pos)
	{
	case -3:
	  x[count]=-1.5;
	  y[count++]=points->first()->y;
	  pos++;
	case -2:
	  x[count]=-1;
	  y[count++]=points->first()->y;
	  pos++;
	case -1:
	  x[count]=-.5;
	  y[count++]=points->first()->y;
	  pos=0;
	}
    }
  for (tmp=points->at(pos);(count<degree)&&(tmp!=0);tmp=points->next())
    {
      x[count]=tmp->x;
      y[count++]=tmp->y;
    }

  int i=1;
  for (;count<degree;count++)
    {
      x[count]=1+.5*(i++);
      y[count]=points->last()->y;
    }
  
  for (int k=0;k<degree;k++)  //create coefficients in y[i] and x[i];
    for (int j=k-1;j>=0;j--)
      y[j]=(y[j]-y[j+1])/(x[j]-x[k]);
}
//****************************************************************************
void createFullPolynom  (QList<CPoint> *points,double *x,double *y)
{
  int count=0;
  CPoint *tmp;

  for (tmp=points->first();tmp!=NULL;tmp=points->next())
    {
      x[count]=tmp->x;
      y[count++]=tmp->y;
    }
  
  for (int k=0;k<count;k++)
      for (int j=k-1;j>=0;j--)
	  y[j]=(y[j]-y[j+1])/(x[j]-x[k]);
}
//****************************************************************************
Interpolation::Interpolation (int type)
{
  y_out=0;
  x=0;
  y=0;
  der=0;
  count=0;
  this->type=type;
}
//****************************************************************************
Interpolation::~Interpolation ()
{
  if (x)     delete x;
  if (y)     delete y;
  if (der)   delete der;
}
//****************************************************************************
const char** Interpolation::getTypes ()
{
  return NAMES;
}
//****************************************************************************
int Interpolation::getCount ()
{
  return INT_COUNT;
}
//****************************************************************************
double Interpolation::getSingleInterpolation (double input)
{
  int degree=0;
  switch (type)
    {
    case LINEAR:
      {
	int i=1;      
	while (x[i]<input) i++;
	double dif1=x[i]-x[i-1];           //!=0 per definition
	double dif2=input-x[i-1];      

	return (y[i-1]+((y[i]-y[i-1])*dif2/dif1));
      }
    case SPLINE:
      {
	double a,b,diff;
	int j=1;
	while (x[j]<input) j++;
	diff = x[j] - x[j-1];

	a = (x[j]-input)/diff;  //div should not be 0
	b = (input-x[j-1])/diff;

	return (a*y[j-1] + b*y[j] + ((a*a*a-a)*der[j-1]+(b*b*b-b)*der[j])*(diff*diff)/6);
      }
    case NPOLYNOMIAL:
      {
	static double ny=y[0];
	for (int j=1;j<count;j++) ny=ny*(input-x[j])+y[j];
	return ny;
      }
    case SAH:   //Sample and hold
      {
	int i=1; 
	while (x[i]<input) i++;
      
	return y[i-1];
      }
    case POLYNOMIAL3:
      if (!degree) degree=3;
    case POLYNOMIAL5:
      if (!degree) degree=5;
    case POLYNOMIAL7:
      {
	if (!degree) degree=7;

	double ny,ax[7],ay[7];

	int i=1;      
	while (x[i]<input) i++;

	createPolynom (points,ax,ay,i-1-degree/2,degree);

	ny=ay[0];
	for (int j=1;j<degree;j++)
	  ny=ny*(input-ax[j])+ay[j];

	return ny;
	break;
      }
    }
  return 0;
}
//****************************************************************************
int Interpolation::prepareInterpolation (QList<CPoint> *points)
{
  this->count=points->count();
  this->points=points;
  if (x)     delete x;
  if (y)     delete y;
  if (der)   delete der;

  x=new double [count+1];
  y=new double [count+1];

  if (x&&y)
    {
      int c=0;
      CPoint *tmp;

      for (tmp=points->first();tmp!=NULL;tmp=points->next())
	{
	  x[c]=tmp->x;
	  y[c++]=tmp->y;
	}

      switch (type)
	{
	case NPOLYNOMIAL:
	  createFullPolynom (points,x,y);
	  break;
	case SPLINE:
	  if (der) delete der;
	  der=new double [count+1];

	  get2Derivate (x,y,der,count);
	}  
    }
  else return false;
 return true;
}
//****************************************************************************
double *Interpolation::getInterpolation (QList<CPoint> *points,int len)
{
  CPoint *tmp;

  if (y_out) delete y_out;
  y_out=new double[len];
  int degree=0;

  switch (type)
    {
    case LINEAR:
      {
	double x,y,lx,ly;
	tmp=points->first();
	if (tmp!=0)
	  {
	    lx=tmp->x;
	    ly=tmp->y;

	    for ( tmp=points->next(); tmp != 0; tmp=points->next() )
	      {
		x=tmp->x;
		y=tmp->y;

		double dify= (y-ly);
		int difx=(int) ((x-lx)*len);
		int min= (int)(lx*len);
		double h;

		for (int i=(int)(lx*len);i<(int)(x*len);i++)
		  {
		    h=(double(i-min))/difx;
		    y_out[i]=ly+(h*dify);
		  }
		lx=x;
		ly=y;
	      }
	  }
	break;
      }
    case SPLINE:
      {
	int t=1;
	int count=points->count();

	double ny=0;
	double x[count+1];
	double y[count+1];
	double der[count+1];

	for (tmp=points->first();tmp!=NULL;tmp=points->next())
	  {
	    x[t]=tmp->x;
	    y[t++]=tmp->y;
	  }

	get2Derivate (x,y,der,count);

	int ent;
	int start=(int) (x[1]*len);

	for (int j=2;j<=count;j++)
	  {
	    ent=(int) (x[j]*len);
	    for (int i=start;i<ent;i++)
	      {
		double xin=((double) i)/len;
		double h, b, a;

		h = x[j] - x[j-1];

		if (h!=0)
		  {
		    a = (x[j]-xin)/h;
		    b = (xin-x[j-1])/h;

		    ny=(a*y[j-1] + b*y[j] + ((a*a*a-a)*der[j-1]+(b*b*b-b)*der[j])*(h*h)/6.0);
		  }

		y_out[i]=ny;
		start=ent;
	      }
	  }
	break;
      }
    case POLYNOMIAL3:
      if (!degree) degree=3;
    case POLYNOMIAL5:
      if (!degree) degree=5;
    case POLYNOMIAL7:
      {
	if (!degree) degree=7;
	int count=points->count();
	double ny,x[7],y[7];
	double ent,start;

	tmp=points->first();
	if (tmp!=0)
	  {
	    for (int px=0;px<count-1;px++)
	      {
		createPolynom (points,x,y,px-degree/2,degree);
		start=points->at(px)->x;

		if (px>=count-degree/2+1) ent=1;
		else ent=points->at(px+1)->x;

		for (int i=(int)(start*len);i<(int)(ent*len);i++)
		  {
		    ny=y[0];
		    for (int j=1;j<degree;j++)
		      ny=ny*(((double)i)/len-x[j])+y[j];

		    y_out[i]=ny;
		  }
	      }
	  }
	break;
      }
    case NPOLYNOMIAL:
      {
	double ny;
	int count=points->count();

	tmp=points->first();
	if (tmp!=0)
	  {
	    double x[count+1];
	    double y[count+1];
	    double px;

	    createFullPolynom (points,x,y);

	    for (int i=1;i<len;i++)
	      {
		px=(double)(i)/len;

		ny=y[0];
		for (int j=1;j<count;j++) ny=ny*(px-x[j])+y[j];

		y_out[i]=ny;

	      }
	  }
	break;
      }
      //**********************************************************************
    case SAH:
      {
	double lx,ly,x,y;

	tmp=points->first();
	if (tmp!=0)
	  {
	    lx=tmp->x;
	    ly=tmp->y;

	    for ( tmp=points->next(); tmp != 0; tmp=points->next() )
	      {
		x=tmp->x;
		y=tmp->y;

		for (int i=(int)(lx*len);i<(int)(x*len);i++) y_out[i]=ly;

		lx=x;
		ly=y;
	      }
	  }
      }
    }
  return y_out;
}










