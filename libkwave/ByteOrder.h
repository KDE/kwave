/***************************************************************************
            ByteOrder.h  -  enum for byte order / endianness
                             -------------------
    begin                : Sat Sep 17 2005
    copyright            : (C) 2005 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _BYTE_ORDER_H_
#define _BYTE_ORDER_H_

#include "config.h"

typedef enum byte_order_t {
	UnknownEndian = -1, /**< unknown/invalid */
	CpuEndian     = 0,  /**< default of the host system's CPU */
	LittleEndian  = 1,  /**< little endian (Intel) */
	BigEndian     = 2   /**< big endian (Motorola) */
} byte_order_t;

#endif /* _BYTE_ORDER_H_ */

//***************************************************************************
//***************************************************************************
