/* --AMR-- Daemon
 *
 *  Daemon.cpp -- wrapper class for making methods asynchronously callable
 *
 * Thanks to Carlos O'Ryan (coryan@cs.wustl.edu) for elucidating
 * the Daemon_Adapter pattern on which this is closely based.
 *
 * \author  Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de>
 * \author  Thomas Eschenbacher
 * \date    12.08.1998, Oct. 1998
 *
 * \par NOTE:
 *      this code is taken from the following news article 
 *      (found by www.dejanews.com):
 *
 * \verbatim
 * ----------------------------------------------------------------------------
 * Author: Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de>
 * Date:   1998/08/12
 * Forums: comp.soft-sys.ace
 * ----------------------------------------------------------------------------
 * Subject: Re: [ace-users] Handing non-static methods into ACE_Thread::spawn()
 * From: ace-users@cs.wustl.edu
 * Date: 1998/08/12
 * Newsgroups: comp.soft-sys.ace
 * ----------------------------------------------------------------------------
 * \endverbatim
 *
 *
 * \bug     C++ exception handling does not work across additionally spawned
 *          client threads of this daemon.
 *
 * \bug     C signal handling is not done, a setlongjmp environment has to
 *          be setup.
 *
 * \bug     This class should be extended to use a TSS cleanup handler that
 *          calls the object's destructor.
 * \bug     The up-time and the number of crashes of a respawnable thread
 *          is not checked. There are no timeouts or limited numer of runs.
 *          This might lead to a infinite loop in some circumstances!
 *
 * \bug     Respawning on signals (like SIGSEGV) must stay disabled until
 *          some code is written to respawn a daemon by a message. See the
 *          comment in the daemon_adapter function for more information.
 */

// #define DEBUG_TRACE

#include <kapp.h> // for debug()

//#include <new>
//#include <typeinfo>
//#include <exception>
//#include <strstream>

//#include "ModuleList.h"
#include "Thread.h"
//#include "sys/Debug.h"

#define DBG_TRACE debug
#define DBG_DEBUG debug

//static inline int get_id() { return 0; }
//
///*  C wrapper function for Daemon::thread_adapter.
//    \internal
//*/
extern "C" void* C_thread_adapter(void* arg)
{
    Thread *thread = (Thread *)arg;
    if (!thread) return 0;

    /* call the daemon's function */
    void* result = thread->thread_adapter(arg);
    DBG_TRACE("back from daemon");

//    /* delete the thread object if it was started in standalone mode */
//    if (daemon->standalone()) {
//	DBG_TRACE("deleting standalone daemon");
//	delete daemon;
//
//	DBG_TRACE("exiting thread");
//	ACE_Thread::exit();
//    }

    DBG_TRACE("return %p", arg);
    return result;
}

//// initialization of the daemon list
//map<pthread_t, Thread *> Thread::ThreadList;
//Thread_Mutex Thread::lock_thread_list_;
//

//***************************************************************************
Thread::Thread(bool respawn, bool standalone)
:/* AMR_TSS_Object(),*/ tid_(0), respawn_(respawn), standalone_(standalone)/*,
    fail_count_(0), last_failed_(0), name_("") */
{
    pthread_attr_init(&attr_);
    pthread_attr_setdetachstate(&attr_, PTHREAD_CREATE_DETACHED);
}

//***************************************************************************
Thread::~Thread()
{
    pthread_attr_destroy(&attr_);
    DBG_TRACE("Thread::~Thread()");
//    if (tid_ != 0) {
//	DBG_TRACE("locking daemon list");
//	ACE_TSS_Guard<ACE_Thread_Mutex> mon(Daemon::lock_daemon_list_, 1);
//	DBG_TRACE("locked");
//
//	/* unregister the buffer of longjmp from the list */
//	DBG_TRACE("unregistering longjmp buffer of thread %d", tid_);
//	Daemon::DaemonList.erase(tid_);
//    }
    DBG_TRACE("Thread::~Thread(): destructor done");
}

//***************************************************************************
void *Thread::thread_adapter (void *arg)
{
//    /* magic error handling matrix, no documentation needed... */
//    const char bfc[55] = {
//	0x0A, 0x09, 0x5C, 0x7C, 0x2F, 0x20, 0x5F, 0x5F,
//        0x5F, 0x5F, 0x20, 0x5C, 0x7C, 0x2F, 0x0A, 0x09,
//	0x22, 0x40, 0x27, 0x2F, 0x20, 0x2C, 0x2E, 0x20,
//	0x5C, 0x60, 0x40, 0x22, 0x0A, 0x09, 0x2F, 0x5F,
//	0x7C, 0x20, 0x5C, 0x5F, 0x5F, 0x2F, 0x20, 0x7C,
//	0x5F, 0x5C, 0x0A, 0x09, 0x20, 0x20, 0x20, 0x5C,
//	0x5F, 0x5F, 0x55, 0x5F, 0x2F, 0x0A, 0x00
//    };

    Thread *object = (Thread *) arg;
//    bool do_respawn = object->respawn_;

    // (this looks silly, but is needed in order to compile with -O2)
    bool normal_ex = false;
    bool *normal_exit = &normal_ex;

//    {
//	DBG_TRACE("locking daemon list");
//	ACE_TSS_Guard<ACE_Thread_Mutex> mon(Daemon::lock_daemon_list_, 1);
//	DBG_TRACE("locked");
//
//	/* register the daemon with its buffer for longjmp */
//	DBG_TRACE("registering longjmp buffer of thread %d", tid_);
//	DaemonList.insert(pair<ACE_thread_t, Daemon *>(tid_, this));
//    }
//
//    do {
//	try {
//	    /* exit the loop if we were killed due to an error */
//	    if (setjmp(jmp_buffer)) {
//	        DBG_TRACE("got here from longjump, continuing...");
//
//		/* we reached this point through a longjump from
//		   ModuleList_handle_signal(), so the daemon list
//		   is still locked :-(
//		   Let's unlock it manually now... */
//		DBG_TRACE("unlocking daemon list");
//		Daemon::lock_daemon_list_.release();
//
//		DBG_FATAL("%s\n UNHANDLED SIGNAL - "
//		          "THREAD MUST BE TERMINATED !!!\n\n",
//		          (char *)&bfc);
//		if (do_respawn) {
//		    /* do something to respawn this thread... */
//		
//                    /* ###
//		    please write some code to inform the main thread
//		    that this daemon was killed and has to be respawned.
//		    This could be done by adding a function "respwan"
//		    to the Asynchronous_Object template and defining a
//		    special Message template "MessageDaemonRespawn" that
//		    has an additional function "do_respawn()" in order
//		    to generate a new Asynchronous_Object instance and
//		    run it as a daemon (now derived from the main thread,
//		    but this shouldn't matter...)
//		    ### */
//		
//		    DBG_FATAL("thread cannot be respawned\n");
//		}
//
//		/* This daemon instance MUST be terminated in order to
//		   call all necessary cleanup handlers !
//		   => NO WAY TO WORK AROUND <=
//		*/
//		break;
//	    }

	    /* execute the thread function */
	    *normal_exit = false;
//	    DBG_TRACE("executing thread %d", ACE_Thread::self());
	    object->execute();
	    *normal_exit = true;
//	}
//	catch(String &strReason)
//	{
//	    DBG_FATAL("%s\n UNHANDLED EXCEPTION: %s\n\n",
//		      (char *)&bfc, (const char *)strReason);
//	}
//	catch(exception &e)
//	{
// 	    DBG_FATAL("%s\n UNHANDLED C-EXCEPTION: `%s'\n\n", (char *)&bfc,
// 		e.what());
//	}
//	/* catch all other exceptions */
//	catch(...)
//	{
//	    DBG_FATAL("%s\n UNHANDLED C++-EXCEPTION\n\n", (char *)&bfc);
//	}
//
//	if (!normal_exit && do_respawn) {
//	    DBG_WARNING("respawning crashed thread.\n");
//	}
//    } while (!*normal_exit && do_respawn);
//
//    {
//	DBG_TRACE("locking daemon list");
//	ACE_TSS_Guard<ACE_Thread_Mutex> mon(Daemon::lock_daemon_list_, 1);
//	DBG_TRACE("locked");
//
//	/* unregister the buffer of longjmp from the list */
//	DBG_TRACE("unregistering longjmp buffer of thread %d", tid_);
//	Daemon::DaemonList.erase(tid_);
//    }
//
//    DBG_TRACE("return %d", arg);
    return arg;
}

//***************************************************************************
pthread_t Thread::activate(Thread_Manager &thr_mgr, int &grpid,
                           const char *name, long flags)
{
    int gid = -1;

//    /* set the "name" field, will be corrected later if empty */
//    name_ = (name) ? name : "";

    debug("Thread::activate(): start");

    int result = pthread_create(&tid_, &attr_,
	C_thread_adapter, this);

//    gid = thr_mgr.spawn(C_thread_adapter, (void *)this,
//			flags, &tid_, 0,
//			ACE_DEFAULT_THREAD_PRIORITY, grpid);
//
//    /* if we started a new group, return the actual group id */
//    if (grpid < 0) {
//	grpid = gid;
//    }
//
//    /* search the module list for the thread group id */
//    if (!name_.length()) {
//	int modulenr = ModuleList::find_module_by_thread_gid(gid);
//	const String *modname = ModuleList::get_name(modulenr);
//	if (modname) {
//	    ostrstream n;
//	    n << *modname << "-" << tid_;
//	    name_ = n.str();
//	}
//    }
//
//    /* if still no name is found */
//    if (!name_.length()) {
//	ostrstream gidtid;
//	gidtid << "(gid=" << gid << ",tid=" << tid_ << ")" << ends;
//	name_ = gidtid.str();
//    }

    debug("Thread::activate(): done.");
    return tid_;
}

/* end of Daemon.cpp */
