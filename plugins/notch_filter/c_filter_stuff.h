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

#ifndef C_FILTER_STUFF_H
#define C_FILTER_STUFF_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  double cx,cx1,cx2,cy1,cy2;
  double x,x1,x2,y,y1,y2;
} filter;

void presence();
void shelve(double cf,double boost,double *a0,double *a1,double *a2,double *b1,double *b2);
void initfilter(filter *f);
void setfilter_presence();
void setfilter_shelve(filter *f, double freq, double boost);
void setfilter_shelvelowpass(filter *f, double freq, double boost);
void setfilter_2polebp();
void setfilter_peaknotch(filter *f, double freq, double M, double bw);
void setfilter_peaknotch2(filter *f, double freq, double gdm, double bw);
double applyfilter();

#ifdef __cplusplus
}
#endif

/*
 * aRts doesn't need the flow stuff that's in music_orig.c - just the filters
 */
#if 0

#define STRBUFSIZE 200
#define TRUE            1
#define FALSE           0

/* must be divisible by 6 and 8
 * max 2 items (ints or bytes) per sample value, 3 or 4 channels */
#define MINBUFFSIZE 2*3*4
#define BUFFSIZE 512*MINBUFFSIZE

#define C_RBBUFSIZE 10*44100
#define C_MAXCHANNELS 4

/*
 * afmethod = 0, ring buffer
 *            1, swap ro bufs
 *            2, swap rw bufs
 *            3, all in memory ro
 *            4, all in memory rw
 * afname = filename for the audio file;
 *          in case of multipart file, the filenames are <filename>.aa, etc.
 * affd = file descriptor number, if it is preset to be STDIN_FILENO or
 *        STDOUT_FILENO, then the filename has no effect, otherwise affd
 *        is set at the init time if afmethod == 0
 * afsr = samplerate
 * afsc = samplechannels
 * afstype = 0, 16 bit (standard CDA format)
 *           1, direct copy of int variable
 * noofbufs = number of swap buffers
 * buflen = length of swap buffers
 * realbuflen = length of swap buffers with respect to the data;
 *              different from buflen only if content is load from
 *              the end of audiofile
 * btime = time of the first sample in buffers
 * etime = time of the last sample in buffers
 *
 * **buf and ***bufs since one array is for one channel
 */

typedef struct {
  int afmethod;
  char *afname;
  FILE *affp;
  int affd;
  int afsr;
  int afsc;
  int afstype;
  int buflen;
  /* ring buffer
   *   int buflen;
   */
  int **buf;
  int bloc;
  int eloc;
  int rbbtime;
  int rbetime;
  /* swap buffers
   *   int buflen;
   */
  int ***bufs;
  int noofbufs;
  int *realbuflen;
  int *btime;
  int *etime;
  int bufupdatemethod;
  /* all in memory
   *   int buflen;
   *   int *buf;
   */
  /* buffer updating method info */
  int *modifiedbuf;
  int *bufpri;
  int npri;
  int cpri;
} ty_audiofile;

/*
 * Priority entries are numbered 0,1,2,... no two same number
 * in two buffers. The buffer which will be swapped is the buffer
 * with highest priority (i.e. nobufs-1). When a buffer is swapped,
 * the priority is set to 1 and priorities of all other buffers are
 * lowered down by one.
 * When a sample is read, the priorities are set for each Nth read.
 */

typedef struct {
  int method;
  int noofbufs;
  int buflen;
} ty_afmethod;

#define C_FLOWOUTMETHOD 0
#define C_RBMETHOD 1
#define C_SWAPROMETHOD 2
#define C_SWAPRWMETHOD 3
#define C_AIMROMETHOD 4
#define C_AIMRWMETHOD 5

typedef struct {
  char *filename;
  int fd;
} ty_afname;

typedef struct {
  int sr;
  int sc;
  int stype;
} ty_aftype;

#define C_CDATYPE 0
#define C_CDASBTYPE 1 /* swap bytes */
#define C_INTTYPE 2
#define C_FLOATTYPE 3

typedef struct {
  int sc;
  int time;
  int buf[C_MAXCHANNELS];
} ty_sample;

#define C_MAXAUDIOFILES 20

typedef struct {
  int len;
  int rloc,wloc;
  double *buf;
} delay;

typedef struct {
  int len;
  int wloc;
  double *buf;
} ringbufferd;

typedef struct {
  int len;
  int wloc;
  int *buf;
} ringbufferi;

typedef struct {
  int n;
  double gain;
  filter f;
} rbreaddev;


ty_audiofile *gaf[C_MAXAUDIOFILES];

int makenodes();
int makeints();
/*
int freadbuf();
int fwritebuf();
*/
ty_audiofile *initaf();
void bye();
ty_sample *makesample();
int readsample();
int writesample();
ty_afmethod *afmethod_flowout();
ty_afmethod *afmethod_rb();
ty_afmethod *afmethod_aimro();
ty_afname *afname();
ty_afname *afname_stdin();
ty_afname *afname_stdout();
ty_aftype *aftype();
ty_aftype *aftype_defstereo();
ty_audiofile *initaf_aimdefstereo();
ty_audiofile *initaf_stdin();
void init();
int saturate16();
void initdelay();
double readdelay();
void writedelay();
void initringbufferd();
double readringbufferd();
void writeringbufferd();
void initringbufferi();
int readringbufferi();
void writeringbufferi();
#endif

#endif /* C_FILTER_STUFF_H */

