
/* mmapalloc   Copyright 1998 Juhana Kouhia, <kouhia at nic.funet.fi>
 *             GNU licenses apply, http://www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include "mmap.h"

mmapallocnode *mmapallocbase = NULL;
char *mmapallocdir= "/tmp";

 /* I used mcopy.c in Stevens' book "Advanced Programming in the Unix
 * Environment" in making this stuff.
 */

#define FILE_MODE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int is_file(char *path)
{
  struct stat file_stats;

  /* lstat is used for a purpose */
  if (lstat(path,&file_stats) == -1) return(0);
  return(1);
}

void *mmapalloc(int n)
{
  char *name;
  int fd,i=1;
  mmapallocnode *m,*pm;

  /* generate a new filename */
  if ((name = (char *)malloc(512*sizeof(char))) == NULL)
    {
      fprintf(stderr,"mmapalloc: malloc failed\n");
      return(NULL);
    }
  do
    {
      sprintf(name,"%s/kwave_%i.mmap",mmapallocdir,i);
      i++;
    }
  while(is_file(name) != 0);

  /* if first time? */
  if (mmapallocbase == NULL) {
    mmapallocbase = (mmapallocnode *)malloc(sizeof(mmapallocnode));
    if (mmapallocbase == NULL) {
      fprintf(stderr,"mmapalloc: malloc failed\n");
      free(name);
      return(NULL);
    }
    m = mmapallocbase;
    pm = NULL;
  } else {
    m = mmapallocbase;
    while (m->next != NULL) m = m->next;
    m->next = (mmapallocnode *)malloc(sizeof(mmapallocnode));
    if (m->next == NULL) {
      fprintf(stderr,"mmapalloc: malloc failed\n");
      free(name);
      return(NULL);
    }
    pm = m;
    m = m->next;
  }

  if ((fd = open(name, O_RDWR|O_CREAT|O_TRUNC, FILE_MODE)) == -1) {
    fprintf(stderr,"mmapalloc: cannot open the file %s\n",name);
    free(name);
    free(m);
    if (pm == NULL) mmapallocbase = NULL;
    else pm->next = NULL;
    return(NULL);
  }

  if (lseek(fd,n-1,SEEK_SET) == -1) {
    fprintf(stderr,"mmapalloc: cannot seek the file\n");
    close(fd);
    if (unlink(name) == -1)
      fprintf(stderr,"mmapalloc: cannot unlink the file\n");
    free(name);
    free(m);
    if (pm == NULL) mmapallocbase = NULL;
    else pm->next = NULL;
    return(NULL);
  }

  if (write(fd, "", 1) != 1) {
    fprintf(stderr,"mmapalloc: cannot set the size of the file\n");
    close(fd);
    if (unlink(name) == -1)
      fprintf(stderr,"mmapalloc: cannot unlink the file\n");
    free(name);
    free(m);
    if (pm == NULL) mmapallocbase = NULL;
    else pm->next = NULL;
    return(NULL);
  }

  if ((m->mmapaddr = mmap(0, n, PROT_READ | PROT_WRITE,
			  MAP_FILE | MAP_SHARED, fd, 0)) ==
      (caddr_t) -1) {
    fprintf(stderr,"mmapalloc: cannot mmap the file\n");
    close(fd);
    if (unlink(name) == -1)
      fprintf(stderr,"mmapalloc: cannot unlink the file\n");
    free(name);
    free(m);
    if (pm == NULL) mmapallocbase = NULL;
    else pm->next = NULL;
    return(NULL);
  }

  close(fd);
  m->next = NULL;
  m->filename = name;
  m->mmapsize = n;
  return(m->mmapaddr);
}

void mmapfree(void *mmapaddr)
{
  mmapallocnode *m,*pm;

  if (mmapaddr == NULL) return;
  if (mmapallocbase == NULL) return;

  m = mmapallocbase;
  pm = NULL;
  while ((m->mmapaddr != mmapaddr) && (m->next != NULL))
    {
      pm = m;
      m = m->next;
    }
  if (m->mmapaddr == mmapaddr)
    {
      (void) munmap(m->mmapaddr,m->mmapsize);
      if (unlink(m->filename) == -1)
	fprintf(stderr,"mmapfree: cannot unlink the file\n");
      free(m->filename);
      if (pm == NULL) mmapallocbase = m->next;
      else pm->next = m->next;
      free(m);
    }
}



