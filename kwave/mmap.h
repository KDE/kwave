
/* mmapalloc   Copyright 1998 Juhana Kouhia, <kouhia at nic.funet.fi>
 *             GNU licences apply, http://www.gnu.org
 */

#ifndef MMAPALLOC
#define MMAPALLOC

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct mmapallocentry mmapallocnode;

struct mmapallocentry
 {
  mmapallocnode *next;
  char *filename;
  void *mmapaddr;
  int mmapsize;
};

#endif

