/***************************************************************************
               Sample.h  -  definition of the sample type
			     -------------------
    begin                : Feb 09 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SAMPLE_H_
#define _SAMPLE_H_

//***************************************************************************

#include <sys/types.h>

/** Currently a "sample" is defined as a 32 bit integer with 24 valid bits */
typedef int32_t sample_t;

/** lowest sample value */
#define SAMPLE_MIN (-(1<<23)+1)

/** highest sample value */
#define SAMPLE_MAX (+(1<<23)-1)

#endif /* _SAMPLE_H_ */
