/*
 *
 * Copyright (C) unknown, probably 1996 Paul Mackerras ?
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * 2004-12-06
 *   Copied this source into the Kwave project and adapted it to compile
 *   cleanly within this new environment
 *   by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 */
#include <stdlib.h>
void *ppcasm_cacheable_memcpy(void *, const void *, size_t);
void *ppcasm_memcpy(void *, const void *, size_t);
