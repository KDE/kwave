    /*

    Copyright (C) 1998 Juhana Sadeharju
                       kouhia at nic.funet.fi

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "c_filter_stuff.h"
#include <math.h>


/*-- double tan(),pow(),atan2(),sqrt(),asin(); --*/

#define C_MIN16 -32768
#define C_MAX16 32767

#define SR 44100
#define PI M_PI

/*
 * Presence and Shelve filters as given in
 *   James A. Moorer
 *   The manifold joys of conformal mapping:
 *   applications to digital filtering in the studio
 *   JAES, Vol. 31, No. 11, 1983 November
 */

/*#define SPN MINDOUBLE*/
#define SPN 0.00001

double bw2angle(a,bw)
double a,bw;
{
  double T,d,sn,cs,mag,delta,theta,tmp,a2,a4,asnd;

  T = tan(2.0*PI*bw);
  a2 = a*a;
  a4 = a2*a2;
  d = 2.0*a2*T;
  sn = (1.0 + a4)*T;
  cs = (1.0 - a4);
  mag = sqrt(sn*sn + cs*cs);
  d /= mag;
  delta = atan2(sn,cs);
  asnd = asin(d);
  theta = 0.5*(PI - asnd - delta);
  tmp = 0.5*(asnd-delta);
  if ((tmp > 0.0) && (tmp < theta)) theta = tmp;
  return(theta/(2.0*PI));
}

void presence(cf,boost,bw,a0,a1,a2,b1,b2)
double cf,boost,bw,*a0,*a1,*a2,*b1,*b2;
{
  double a,A,F,xfmbw,C,tmp,alphan,alphad,b0,recipb0,asq,F2,a2plus1,ma2plus1;

  a = tan(PI*(cf-0.25));
  asq = a*a;
  A = pow(10.0,boost/20.0);
  if ((boost < 6.0) && (boost > -6.0)) F = sqrt(A);
  else if (A > 1.0) F = A/sqrt(2.0);
  else F = A*sqrt(2.0);
  xfmbw = bw2angle(a,bw);

  C = 1.0/tan(2.0*PI*xfmbw);
  F2 = F*F;
  tmp = A*A - F2;
  if (fabs(tmp) <= SPN) alphad = C;
  else alphad = sqrt(C*C*(F2-1.0)/tmp);
  alphan = A*alphad;

  a2plus1 = 1.0 + asq;
  ma2plus1 = 1.0 - asq;
  *a0 = a2plus1 + alphan*ma2plus1;
  *a1 = 4.0*a;
  *a2 = a2plus1 - alphan*ma2plus1;

  b0 = a2plus1 + alphad*ma2plus1;
  *b2 = a2plus1 - alphad*ma2plus1;

  recipb0 = 1.0/b0;
  *a0 *= recipb0;
  *a1 *= recipb0;
  *a2 *= recipb0;
  *b1 = *a1;
  *b2 *= recipb0;
}

void shelve(cf,boost,a0,a1,a2,b1,b2)
double cf,boost,*a0,*a1,*a2,*b1,*b2;
{
  double a,A,F,tmp,b0,recipb0,asq,F2,gamma2,siggam2,gam2p1;
  double gamman,gammad,ta0,ta1,ta2,tb0,tb1,tb2,aa1,ab1;

  a = tan(PI*(cf-0.25));
  asq = a*a;
  A = pow(10.0,boost/20.0);
  if ((boost < 6.0) && (boost > -6.0)) F = sqrt(A);
  else if (A > 1.0) F = A/sqrt(2.0);
  else F = A*sqrt(2.0);

  F2 = F*F;
  tmp = A*A - F2;
  if (fabs(tmp) <= SPN) gammad = 1.0;
  else gammad = pow((F2-1.0)/tmp,0.25);
  gamman = sqrt(A)*gammad;

  gamma2 = gamman*gamman;
  gam2p1 = 1.0 + gamma2;
  siggam2 = 2.0*sqrt(2.0)/2.0*gamman;
  ta0 = gam2p1 + siggam2;
  ta1 = -2.0*(1.0 - gamma2);
  ta2 = gam2p1 - siggam2;

  gamma2 = gammad*gammad;
  gam2p1 = 1.0 + gamma2;
  siggam2 = 2.0*sqrt(2.0)/2.0*gammad;
  tb0 = gam2p1 + siggam2;
  tb1 = -2.0*(1.0 - gamma2);
  tb2 = gam2p1 - siggam2;

  aa1 = a*ta1;
  *a0 = ta0 + aa1 + asq*ta2;
  *a1 = 2.0*a*(ta0+ta2)+(1.0+asq)*ta1;
  *a2 = asq*ta0 + aa1 + ta2;

  ab1 = a*tb1;
  b0 = tb0 + ab1 + asq*tb2;
  *b1 = 2.0*a*(tb0+tb2)+(1.0+asq)*tb1;
  *b2 = asq*tb0 + ab1 + tb2;

  recipb0 = 1.0/b0;
  *a0 *= recipb0;
  *a1 *= recipb0;
  *a2 *= recipb0;
  *b1 *= recipb0;
  *b2 *= recipb0;
}

void initfilter(filter *f)
{
  f->x1 = 0.0;
  f->x2 = 0.0;
  f->y1 = 0.0;
  f->y2 = 0.0;
  f->y = 0.0;
}

void setfilter_presence(f,freq,boost,bw)
filter *f;
double freq,boost,bw;
{
  presence(freq/(double)SR,boost,bw/(double)SR,
           &f->cx,&f->cx1,&f->cx2,&f->cy1,&f->cy2);
  f->cy1 = -f->cy1;
  f->cy2 = -f->cy2;
}

void setfilter_shelve(filter *f, double freq, double boost)
{
  shelve(freq/(double)SR,boost,
	 &f->cx,&f->cx1,&f->cx2,&f->cy1,&f->cy2);
  f->cy1 = -f->cy1;
  f->cy2 = -f->cy2;
}

void setfilter_shelvelowpass(filter *f, double freq, double boost)
{
  double gain;

  gain = pow(10.0,boost/20.0);
  shelve(freq/(double)SR,boost,
	 &f->cx,&f->cx1,&f->cx2,&f->cy1,&f->cy2);
  f->cx /= gain; 
  f->cx1 /= gain; 
  f->cx2 /= gain; 
  f->cy1 = -f->cy1;
  f->cy2 = -f->cy2;
}

/*
 * As in ''An introduction to digital filter theory'' by Julius O. Smith
 * and in Moore's book; I use the normalized version in Moore's book.
 */
void setfilter_2polebp(f,freq,R)
filter *f;
double freq,R;
{
  double theta;

  theta = 2.0*PI*freq/(double)SR;
  f->cx = 1.0-R;
  f->cx1 = 0.0;
  f->cx2 = -(1.0-R)*R;
  f->cy1 = 2.0*R*cos(theta);
  f->cy2 = -R*R;
}

/*
 * As in
 *   Stanley A. White
 *   Design of a digital biquadratic peaking or notch filter
 *   for digital audio equalization
 *   JAES, Vol. 34, No. 6, 1986 June
 */
void setfilter_peaknotch(f,freq,M,bw)
filter *f;
double freq,M,bw;
{
  double w0,om,ta,d,  p=0.0 /* prevents compiler warning */;

  w0 = 2.0*PI*freq;
  if ((1.0/sqrt(2.0) < M) && (M < sqrt(2.0))) {
    fprintf(stderr,"peaknotch filter: 1/sqrt(2) < M < sqrt(2)\n");
    exit(-1);
  }
  if (M <= 1.0/sqrt(2.0)) p = sqrt(1.0-2.0*M*M);
  if (sqrt(2.0) <= M) p = sqrt(M*M-2.0);
  om = 2.0*PI*bw;
  ta = tan(om/((double)SR*2.0));
  d = p+ta;
  f->cx = (p+M*ta)/d;
  f->cx1 = -2.0*p*cos(w0/(double)SR)/d;
  f->cx2 = (p-M*ta)/d;
  f->cy1 = 2.0*p*cos(w0/(double)SR)/d;
  f->cy2 = -(p-ta)/d;
}

/*
 * Some JAES's article on ladder filter.
 * freq (Hz), gdb (dB), bw (Hz)
 */
void setfilter_peaknotch2(f,freq,gdb,bw)
filter *f;
double freq,gdb,bw;
{
  double k,w,bwr,abw,gain;

  k = pow(10.0,gdb/20.0);
  w = 2.0*PI*freq/(double)SR;
  bwr = 2.0*PI*bw/(double)SR;
  abw = (1.0-tan(bwr/2.0))/(1.0+tan(bwr/2.0));
  gain = 0.5*(1.0+k+abw-k*abw);
  f->cx = 1.0*gain;
  f->cx1 = gain*(-2.0*cos(w)*(1.0+abw))/(1.0+k+abw-k*abw);
  f->cx2 = gain*(abw+k*abw+1.0-k)/(abw-k*abw+1.0+k);
  f->cy1 = 2.0*cos(w)/(1.0+tan(bwr/2.0));
  f->cy2 = -abw;
}

double applyfilter(f,x)
filter *f;
double x;
{
  f->x = x;
  f->y = f->cx * f->x + f->cx1 * f->x1 + f->cx2 * f->x2
    + f->cy1 * f->y1 + f->cy2 * f->y2;
  f->x2 = f->x1;
  f->x1 = f->x;
  f->y2 = f->y1;
  f->y1 = f->y;
  return(f->y);
}

/*
 * aRts doesn't need the functions below this line
 */

#if 0
int saturate16(x)
double x;
{
  if (x > 32765.0) {
    return(32765);
  } else if (x < -32765.0) {
    return(-32765);
  } else return((int)x);
}

void initdelay(d,n)
delay *d;
int n;
{
  int i;

  d->len = n;
  d->wloc = n-1;
  d->rloc = 0;
  d->buf = (double *)malloc(n*sizeof(double));
  for(i = 0; i < n; i++) d->buf[i] = 0.0;
}

double readdelay(d)
delay *d;
{
  double y;

  y = d->buf[d->rloc];
  d->rloc++;
  if (d->rloc == d->len) d->rloc = 0;
  return(y);
}

void writedelay(d,x)
delay *d;
double x;
{
  d->buf[d->wloc] = x;
  d->wloc++;
  if (d->wloc == d->len) d->wloc = 0;
}

void initringbufferd(rb,n)
ringbufferd *rb;
int n;
{
  int i;

  rb->len = n;
  rb->wloc = n-1;
  rb->buf = (double *)malloc(n*sizeof(double));
  for(i = 0; i < n; i++) rb->buf[i] = 0.0;
}

double readringbufferd(rb,n)
ringbufferd *rb;
int n;
{
  int i;

  if (n >= rb->len) return(0.0);
  i = rb->wloc - n;
  if (i < 0) i += rb->len;
  return(rb->buf[i]);
}

void writeringbufferd(rb,x)
ringbufferd *rb;
double x;
{
  rb->buf[rb->wloc] = x;
  rb->wloc++;
  if (rb->wloc == rb->len) rb->wloc = 0;
}

void initringbufferi(rb,n)
ringbufferi *rb;
int n;
{
  int i;

  rb->len = n;
  rb->wloc = n-1;
  rb->buf = (int *)malloc(n*sizeof(int));
  for(i = 0; i < n; i++) rb->buf[i] = 0;
}

int readringbufferi(rb,n)
ringbufferi *rb;
int n;
{
  int i;

  if (n >= rb->len) return(0);
  i = rb->wloc - n;
  if (i < 0) i += rb->len;
  return(rb->buf[i]);
}

void writeringbufferi(rb,x)
ringbufferi *rb;
int x;
{
  rb->buf[rb->wloc] = x;
  rb->wloc++;
  if (rb->wloc == rb->len) rb->wloc = 0;
}

unsigned char buffc[BUFFSIZE];
int buffi[BUFFSIZE];
/* int buffs[C_MAXCHANNELS][BUFFSIZE]; */
int **buffs;


int makenodes(n)
int n;
{
  int *p;
  int i;

  p = (int *)malloc(n*sizeof(int *));
  for(i = 0; i < n; i++) p[i] = (int)(int *)0;
  return((int)p);
}

int makeints(n)
int n;
{
  int *p;
  int i;

  p = (int *)malloc(n*sizeof(int));
  for(i = 0; i < n; i++) p[i] = 0;
  return((int)p);
}

/*

constant memory size:
 (i) one big malloc
 (ii) many mallocs, upper limit in doing mallocs 



 */



/* new routines:
 *
 * readbufb(n) -- read n bytes (8 bits) from stream
 * readbufs(n) -- read n shorts (16 bits) from stream
 * readbufi(n) -- read n ints (32 bits) from stream
 * readbuff(n) -- read n floats (32 bits) from stream
 *
 * bufb2bufs() -- convert byte buffer to short buffer
 * bufb2bufi() -- convert byte buffer to int buffer
 * bufb2buff() -- convert byte buffer to float buffer
 * bufs2bufb() -- convert short buffer to byte buffer
 * bufi2bufb() -- convert int buffer to byte buffer
 * buff2bufb() -- convert float buffer to byte buffer
 *
 * copychannelb() -- copy one channel from buffer to buffer 
 * copychannels() -- copy one channel from buffer to buffer 
 * copychanneli() -- copy one channel from buffer to buffer 
 * copychannelf() -- copy one channel from buffer to buffer 
 *
 * multichannel buffers:
 * buf[sample][channel]
 * buf[channel][sample]
 *
 * multi to uni buffer
 *
 * reading and writing:
 * uni buffer to sample[channel]
 * multi buffer to sample[channel]
 *
 */
/*
int newfreadbufs(buf,n,p)
short **buf;
int n;
ty_audiofile *p;
{
  if (n*p->afsc > BUFFSIZE) {
    fprintf(stderr,"freadbufi: reading too many samples\n");
    exit(-1);
  }
  l = readbufs(tmpbufs,n*p->afsc);
  m = uni2multis(tmpbufs,l,p->afsc,buf);
  return(m);
}
   
int newfreadbufi(buf,n,p)
int **buf;
int n;
ty_audiofile *p;
{
  if (n*p->afsc > BUFFSIZE) {
    fprintf(stderr,"freadbufi: reading too many samples\n");
    exit(-1);
  }
  l = readbufi(tmpbufi,n*p->afsc);
  m = uni2multii(tmpbufi,l,p->afsc,buf);
  return(m);
}
   
int newfreadbuff(buf,n,p)
float **buf;
int n;
ty_audiofile *p;
{
  if (n*p->afsc > BUFFSIZE) {
    fprintf(stderr,"freadbufi: reading too many samples\n");
    exit(-1);
  }
  l = readbuf(tmpbuff,n*p->afsc);
  m = uni2multif(tmpbuff,l,p->afsc,buf);
  return(m);
}


int newfreadbuf(buf,p)
ty_buffer *buf;
ty_audiofile *p;
{
  
}

*/

/*
 * freadbuf() reads next n samples from the file; one sample may have
 * several channels.
 * Return value is the number of the samples read.
 */

int freadbuf(buf,n,p)
int **buf;
int n;
ty_audiofile *p;
{
  int h,i,j,k,l,s;
  unsigned int us;

  if (n > BUFFSIZE) {
    fprintf(stderr,"freadbuf reading too many samples\n");
    exit(-1);
  }
  if (p->afstype == C_INTTYPE) {
    h = 0;
    for(j = 0; j < p->afsc; j++) {
      l = fread(buffi,sizeof(int),n,p->affp);
      for(i = 0; i < l; i += p->afsc) {
	for(k = 0; k < p->afsc; k++) buf[k][h] = buffi[i+k];
	h++;
      }
    }
  } else if (p->afstype == C_FLOATTYPE) {
    h = 0;
    for(j = 0; j < p->afsc; j++) {
      l = fread((float *)buffi,sizeof(float),n,p->affp);
      for(i = 0; i < l; i += p->afsc) {
	for(k = 0; k < p->afsc; k++) buf[k][h] = buffi[i+k];
	h++;
      }
    }
  } else {
    h = 0;
    for(j = 0; j < 2*p->afsc; j++) {
      l = fread(buffc,sizeof(unsigned char),n,p->affp);
      for(i = 0; i < l; i += 2*p->afsc) {
	for(k = 0; k < p->afsc; k++) {
	  if (p->afstype == C_CDASBTYPE)
	    us = buffc[i+1+2*k] + (buffc[i+2*k]<<8);
	  else
	    us = buffc[i+2*k] + (buffc[i+1+2*k]<<8);
	  us = us<<16;
	  s = ((signed int)us)>>16;
	  buf[k][h] = s;
	}
	h++;
      }
    }
  }
  return(h);
}


int fwritebuf(buf,n,p)
int **buf;
int n;
ty_audiofile *p;
{
  int h,i,j,k,l,s;
  unsigned int us1,us2;

  if (p->afstype == C_INTTYPE) {
    h = 0;
    for(i = 0; i < n; i++) {
      for(k = 0; k < p->afsc; k++) {
	buffi[h] = buf[k][i];
	h++;
      }
      if (h == BUFFSIZE) {
	l = fwrite(buffi,sizeof(int),h,p->affp);
	if (l != h) {
	  fprintf(stderr,"fwritebuf() error\n");
	  exit(-1);
	}
	h = 0;
      }
    }
    l = fwrite(buffi,sizeof(int),h,p->affp);
    if (l != h) {
      fprintf(stderr,"fwritebuf() error\n");
      exit(-1);
    }
  } else {
    h = 0;
    for(i = 0; i < n; i++) {
      for(k = 0; k < p->afsc; k++) {
	s = buf[k][i];
	if (s > C_MAX16) s = C_MAX16;
	else if (s < C_MIN16) s = C_MIN16;
	us1 = ((unsigned int)s)&0x000000ff;
	us2 = (((unsigned int)s)&0x0000ff00)>>8;
	if (p->afstype == C_CDASBTYPE) {
	  buffc[h] = (unsigned char)us2;
	  h++;
	  buffc[h] = (unsigned char)us1;
	  h++;
	} else {
	  buffc[h] = (unsigned char)us1;
	  h++;
	  buffc[h] = (unsigned char)us2;
	  h++;
	}
      }
      if (h == BUFFSIZE) {
	l = fwrite(buffc,sizeof(unsigned char),h,p->affp);
	if (l != h) {
	  fprintf(stderr,"fwritebuf() error\n");
	  exit(-1);
	}
	h = 0;
      }
    }
    l = fwrite(buffc,sizeof(unsigned char),h,p->affp);
    if (l != h) {
      fprintf(stderr,"fwritebuf() error\n");
      exit(-1);
    }
  }
  return(n);
}


ty_audiofile *initaf(afm,afn,aft)
ty_afmethod *afm;
ty_afname *afn;
ty_aftype *aft;
{
  ty_audiofile *p;
  int i,j,k,n,s;
  unsigned int us;
  FILE *fp;

  p = (ty_audiofile *)malloc(sizeof(ty_audiofile));
  p->afmethod = afm->method;
  p->afname = afn->filename;
  p->affd = afn->fd;
  p->afsr = aft->sr;
  p->afsc = aft->sc;
  p->afstype = aft->stype;
  p->buflen = afm->buflen;

  switch(p->afmethod) {
  case C_FLOWOUTMETHOD:
    if (p->affd == STDOUT_FILENO) {
      fp = stdout;
      p->afname = "stdout";
    } else {
      if ((fp = fopen(p->afname,"w")) == (FILE *)NULL) {
	fprintf(stderr,"could not open file %s\n",p->afname);
	exit(-1);
      }
    }
    p->affp = fp;
    p->buflen = BUFFSIZE;
    p->buf = (int **)malloc(p->afsc*sizeof(int *));
    for(i = 0; i < p->afsc; i++)
      p->buf[i] = (int *)malloc(p->buflen*sizeof(int));
    p->bloc = 0;
    break;
  case C_RBMETHOD:
    if (p->affd == STDIN_FILENO) {
      fp = stdin;
      p->afname = "stdin";
    } else {
      if ((fp = fopen(p->afname,"r")) == (FILE *)NULL) {
	fprintf(stderr,"could not open file %s\n",p->afname);
	exit(-1);
      }
    }
    p->affp = fp;
    p->buf = (int **)malloc(p->afsc*sizeof(int *));
    for(i = 0; i < p->afsc; i++)
      p->buf[i] = (int *)malloc(p->buflen*sizeof(int));
    n = freadbuf(p->buf,MINBUFFSIZE,p);
    if (n != MINBUFFSIZE) {
      fprintf(stderr,"could not read file %s\n",p->afname);
      fprintf(stderr,"%i\n",n);
      exit(-1);
    }
    p->bloc = 0;
    p->eloc = n-1;
    p->rbbtime = 0;
    p->rbetime = n-1;
    break;
  case C_AIMROMETHOD:
    p->buf = (int **)malloc(p->afsc*sizeof(int *));
    if ((fp = fopen(p->afname,"r")) == (FILE *)NULL) {
      fprintf(stderr,"could not open file %s\n",p->afname);
      exit(-1);
    }
    (void)fseek(fp,(long)0,SEEK_END);
    p->buflen = ftell(fp)/p->afsc;
    fclose(fp);
    switch(p->afstype) {
    case C_CDATYPE:
      p->buflen /= 2;
      break;
    case C_CDASBTYPE:
      p->buflen /= 2;
      break;
    case C_INTTYPE:
      p->buflen /= sizeof(int);
      break;
    }
    for(i = 0; i < p->afsc; i++)
      p->buf[i] = (int *)malloc(p->buflen*sizeof(int));

    if ((fp = fopen(p->afname,"r")) == (FILE *)NULL) {
      fprintf(stderr,"could not open file %s\n",p->afname);
      exit(-1);
    }
    p->affp = fp;
    j = 0;
    while ((n = freadbuf(buffs,BUFFSIZE,p)) != 0) {
      for(i = 0; i < n; i++,j++) {
	for(k = 0; k < p->afsc; k++) p->buf[k][j] = buffs[k][i];
      }
    }
    fclose(fp);
    break;
  }
  return(p);
}


void bye()
{
  ty_audiofile *p;
  int i,l;

  for(i = 0; i < C_MAXAUDIOFILES; i++) {
    p = gaf[i];
    if (p != (ty_audiofile *)0) {
      switch(p->afmethod) {
      case C_FLOWOUTMETHOD:
	l = fwritebuf(p->buf,p->bloc,p);
	if (l != p->bloc) {
	  fprintf(stderr,"could not write to %s\n",p->afname);
	  exit(-1);
	}
	fclose(p->affp);
	break;
      case C_RBMETHOD:
	fclose(p->affp);
	break;
      }
    }
  }
}


ty_sample *makesample(sc)
int sc;
{
  ty_sample *p;

  p = (ty_sample *)malloc(sizeof(ty_sample));
  p->sc = sc;
  return(p);
}


int readsample(p,n,s)
ty_audiofile *p;
int n;
ty_sample *s;
{
  int i,j,k,dt,l;
  FILE *fp;
  ty_sample *out;

  /*
  out = makesample(p->afsc);
  / * out->time = n; * /
  */

  out = s;

  switch(p->afmethod) {
  case C_RBMETHOD:
    for(;;) {
      if ((p->rbbtime <= n) && (n <= p->rbetime)) {
	dt = n - p->rbbtime;
	l = p->bloc + dt;
	if (l >= p->buflen) l -= p->buflen;
	for(i = 0; i < p->afsc; i++) out->buf[i] = p->buf[i][l]; 
	return(TRUE);
      } else {
	if (n < p->rbbtime) {
	  fprintf(stderr,"n = %i\n",n);
	  fprintf(stderr,"ring buffer has dropped this sample already\n");
	  exit(-1);
	}
	l = freadbuf(buffs,BUFFSIZE,p);
	if (l == 0) return(FALSE);
	for(i = 0; i < l; i++) {
	  p->eloc++;
	  if (p->eloc >= p->buflen) p->eloc -= p->buflen;
	  p->rbetime++;
	  if (p->eloc == p->bloc) {
	    p->bloc++;
	    if (p->bloc >= p->buflen) p->bloc -= p->buflen;
	    p->rbbtime++;
	  }
	  for(j = 0; j < p->afsc; j++) {
	    p->buf[j][p->eloc] = buffs[j][i];
	  }
	}
      }
    }
    break;
  case C_AIMROMETHOD:
    if ((n < 0) || (n >= p->buflen)) return(FALSE);
    for(i = 0; i < p->afsc; i++) out->buf[i] = p->buf[i][n]; 
    return(TRUE);
    break;
  }

}


int writesample(p,n,s)
ty_audiofile *p;
int n;
ty_sample *s;
{
  int i,j,k,dt,l;
  FILE *fp;
  ty_sample *out;

  switch(p->afmethod) {
  case C_FLOWOUTMETHOD:
    for(i = 0; i < p->afsc; i++) p->buf[i][p->bloc] = s->buf[i];
    p->bloc++;
    if (p->bloc == p->buflen) {
      p->bloc = 0;
      l = fwritebuf(p->buf,p->buflen,p);
      if (l != p->buflen) {
	fprintf(stderr,"could not write to %s\n",p->afname);
	exit(-1);
      }
    }
    break;
  case C_AIMRWMETHOD:
    if ((n < 0) || (n >= p->buflen)) return(FALSE);
    for(i = 0; i < p->afsc; i++) p->buf[i][n] = s->buf[i];
    break;
  }
  return(TRUE);
}

ty_afmethod *afmethod_flowout()
{
  ty_afmethod *p;

  p = (ty_afmethod *)malloc(sizeof(ty_afmethod));
  p->method = C_FLOWOUTMETHOD;
  return(p);
}

ty_afmethod *afmethod_rb(n)
int n;
{
  ty_afmethod *p;

  if (n <= BUFFSIZE) {
    fprintf(stderr,"RB buffer size should be greater than BUFFSIZE\n");
    exit(-1);
  }
  p = (ty_afmethod *)malloc(sizeof(ty_afmethod));
  p->method = C_RBMETHOD;
  p->buflen = n;
  return(p);
}

ty_afmethod *afmethod_aimro()
{
  ty_afmethod *p;

  p = (ty_afmethod *)malloc(sizeof(ty_afmethod));
  p->method = C_AIMROMETHOD;
  return(p);
}

ty_afname *afname(s)
char *s;
{
  ty_afname *p;

  p = (ty_afname *)malloc(sizeof(ty_afname));
  p->filename = strdup(s);
  p->fd = -1;
  return(p);
}

/* stdin and stdout could have their own read and write routines
 * but this could be a second solution
 */
ty_afname *afname_stdin()
{
  ty_afname *p;

  p = (ty_afname *)malloc(sizeof(ty_afname));
  p->filename = (char *)0;
  p->fd = STDIN_FILENO;
  return(p);
}

ty_afname *afname_stdout()
{
  ty_afname *p;

  p = (ty_afname *)malloc(sizeof(ty_afname));
  p->filename = (char *)0;
  p->fd = STDOUT_FILENO;
  return(p);
}

ty_aftype *aftype(sr,sc,stype)
int sr,sc,stype;
{
  ty_aftype *p;

  p = (ty_aftype *)malloc(sizeof(ty_aftype));
  p->sr = sr;
  p->sc = sc;
  p->stype = stype;
  return(p);
}

ty_aftype *aftype_defstereo()
{
  return(aftype(44100,2,C_CDATYPE));
}


ty_audiofile *initaf_aimdefstereo(filename)
char *filename;
{
  return(initaf(afmethod_aimro(),afname(filename),aftype_defstereo()));
}


ty_audiofile *initaf_stdin()
{
  return(initaf(afmethod_rb(C_RBBUFSIZE),afname_stdin(),aftype_defstereo()));
}

void init()
{
  int i;

  for(i = 0; i < C_MAXAUDIOFILES; i++) {
    gaf[i] = (ty_audiofile *)0;
  }

  buffs = (int **)malloc(C_MAXCHANNELS*sizeof(int *));
  for(i = 0; i < C_MAXCHANNELS; i++)
    buffs[i] = (int *)malloc(BUFFSIZE*sizeof(int));

}


#endif
