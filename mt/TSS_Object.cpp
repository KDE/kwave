/***************************************************************************
 TSS_Object.cpp  - base class for usage of TSS (supports asynchronous exits)
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
#include <qapplication.h> // for qDebug() and qWarning()
#include <pthread.h>
#include <stdio.h>
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
    fprintf(stderr, "cleanup handler for %p\n", ptr);
    Q_ASSERT(ptr);
    if (!ptr) {
	qWarning("cleanup handler for NULL pointer ?  => bailing out!");
	return;
    }

#ifdef HAVE_TYPEINFO
//    try {
	TSS_Object *tssobj = (TSS_Object *)ptr;
	const char *obj_name = typeid(*tssobj).name();

	if (obj_name) {
	    /* try to demangle the symbol name */
#ifdef HAVE_DEMANGLE_H	
            char *res = cplus_demangle(obj_name,
	                               DMGL_ANSI | DMGL_PARAMS | DMGL_AUTO);
	    if (res) {
		/* use the damangeled name instead */
		qWarning("cleanup handler for class %s", res);
		free(res);
	    } else {
#endif // HAVE_DEMANGLE_H	
		qWarning("cleanup handler for class `%s'", obj_name);
#ifdef HAVE_DEMANGLE_H	
	    }
#endif // HAVE_DEMANGLE_H	
	} else {
	    qWarning("cleanup handler for unknown class `%s'", obj_name);
	}

	if (tssobj) delete tssobj;
//    }
//    catch (...) {
//	qWarning("cleanup handler for %p failed", ptr);
//    }
#else // HAVE_TYPEINFO
//    qWarning("cleanup handler for %p", ptr);

    qWarning("cleanup handler for %s", ((QObject*)ptr)->className());

#endif // HAVE_TYPEINFO

}

//***************************************************************************
// static initializers
unsigned int TSS_Object::m_count(0); // for number of instances

//static Mutex _lock;

//***************************************************************************
TSS_Object::TSS_Object()
   :m_key(0), m_thread(pthread_self())
{
//    _lock.lock(); // ###
//    m_count++;
//
//    int res = pthread_key_create(&m_key, TSS_Object_cleanup_func);
//    if (res == EAGAIN) {
//	// number of keys exceeded
//	qWarning("TSS_Object: keycreate failed: "
//	    "number of keys exceeded limit: %d (limit=%d)",
//	    m_count, PTHREAD_KEYS_MAX);
//	qDebug("[maybe too many unfreed objects or memory leak?]");
//    } else if (res) {
//	// some other error
//	qWarning("TSS_Object: keycreate failed: %s", strerror(res));
//    } else {
//	// key allocated, associate this object's instance with it
//	res = pthread_setspecific(m_key, (void *)this);
//	if (res) qWarning("TSS_Object::setspecific failed: %s",
//	    strerror(res));
//    }
//
//    qDebug("TSS_Object::TSS_Object():  this=%p, tid=%d, key=%p, count=%d",
//	this, pthread_self(), m_key, m_count);
//
//    _lock.unlock(); // ###
}

//***************************************************************************
TSS_Object::~TSS_Object()
{
//    _lock.lock(); // ###
//
//    Q_ASSERT(pthread_getspecific(m_key) != 0);
//    Q_ASSERT(pthread_getspecific(m_key) == (void *)this);
//    Q_ASSERT(m_thread == pthread_self());
//
//    qDebug("TSS_Object::~TSS_Object(): this=%p, tid=%d, key=%p, count=%d",
//	this, pthread_self(), m_key, m_count);
//
////    pthread_setspecific(m_key, 0);
//
//    int res = pthread_key_delete(m_key);
//    if (res) qWarning(
//	"TSS_Object::~TSS_Object: key deletion failed: %s",
//	strerror(res));
//
//    Q_ASSERT(m_count != 0);
//    m_count--;
//
//    _lock.unlock(); // ###
}

//***************************************************************************
//***************************************************************************
/* end of TSS_Object.cpp */
