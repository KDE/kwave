/***************************************************************************
    TSS_Object.h  - base class for usage of TSS (supports asynchronous exits)
			     -------------------
    begin                : Sun Oct 01 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#include "config.h"
#include <errno.h>
#include <error.h>        // for strerror
#include <qapplication.h> // for debug() and warning()
#include <pthread.h>
#include <limits.h>       // for PTHREAD_KEYS_MAX

#include "mt/TSS_Object.h"

#ifdef HAVE_DEMANGLE_H
extern "C" {
#include <demangle.h>
}
#endif // HAVE_DEMANGLE_H

#ifdef HAVE_TYPEINFO
#include <typeinfo>
#endif // HAVE_TYPEINFO

//***************************************************************************

/**
 * Cleanup handler function for the TSS_Object class.
 * @param ptr pointer to a TSS_Object instance
 */
extern "C" void TSS_Object_cleanup_func(void *ptr)
{
    ASSERT(ptr);
    if (!ptr) {
	warning("cleanup handler for NULL pointer ?  => bailing out!");
	return;
    }

#ifdef HAVE_TYPEINFO
    try {
	TSS_Object *tssobj = (TSS_Object *)ptr;
	const char *obj_name = typeid(*tssobj).name();

	if (obj_name) {
	    /* try to demangle the symbol name */
#ifdef HAVE_DEMANGLE_H	
            char *res = cplus_demangle(obj_name,
	                               DMGL_ANSI | DMGL_PARAMS | DMGL_AUTO);
	    if (res) {
		/* use the damangeled name instead */
		warning("cleanup handler for class %s", res);
		free(res);
	    } else {
#endif // HAVE_DEMANGLE_H	
		warning("cleanup handler for class `%s'", obj_name);
#ifdef HAVE_DEMANGLE_H	
	    }
#endif // HAVE_DEMANGLE_H	
	} else {
	    warning("cleanup handler for unknown class `%s'", obj_name);
	}

	if (tssobj) delete tssobj;
    }
    catch (...) {
	warning("cleanup handler for %p failed", ptr);
    }
#else // HAVE_TYPEINFO
//    warning("cleanup handler for %p", ptr);

    warning("cleanup handler for %s", ((QObject*)ptr)->className());

#endif // HAVE_TYPEINFO

}

//***************************************************************************
unsigned int TSS_Object::m_count(0); // initializer for number of instances

//***************************************************************************
TSS_Object::TSS_Object()
   :m_key(0)
{
    m_count++;

    int res = pthread_key_create(&m_key, TSS_Object_cleanup_func);
    if (res == EAGAIN) {
	// number of keys exceeded
	warning("TSS_Object: keycreate failed: "
	    "number of keys exceeded limit: %d (limit=%d)",
	    m_count, PTHREAD_KEYS_MAX);
	debug("[maybe too many unfreed objects or memory leak?]");
    } else if (res) {
	// some other error
	warning("TSS_Object: keycreate failed: %s", strerror(res));
    } else {
	// key allocated, associate this object's instance with it
	res = pthread_setspecific(m_key, (void *)this);
	if (res) warning("TSS_Object::setspecific failed: %s",
	    strerror(res));
    }
}

//***************************************************************************
TSS_Object::~TSS_Object()
{
    int res = pthread_key_delete(m_key);
    if (res) warning(
	"TSS_Object::~TSS_Object: key deletion failed: %s",
	strerror(res));
    m_count--;
}

//***************************************************************************
//***************************************************************************

/* end of TSS_Object.cpp */
