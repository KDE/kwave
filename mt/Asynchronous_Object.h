/**
 * \file  mt/Asynchronous_Object.h
 * \brief Template classes for making methods asynchronously callable 
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
 
 \verbatim
  ----------------------------------------------------------------------------
  Author: Oliver M. Kellogg <Oliver.Kellogg@vs.dasa.de>
  Date:   1998/08/12
  Forums: comp.soft-sys.ace
  ----------------------------------------------------------------------------
  Subject: Re: [ace-users] Handing non-static methods into ACE_Thread::spawn()
  From: ace-users@cs.wustl.edu
  Date: 1998/08/12
  Newsgroups: comp.soft-sys.ace

 \endverbatim

 */

/**
 * \class    Asynchronous_Object
 * \brief    Template classes for making methods asynchronously callable
 */

#ifndef _ASYNCHRONOUS_OBJECT_H_
#define _ASYNCHRONOUS_OBJECT_H_

#include "config.h"
#include "mt/Thread.h"

template <class T>class Asynchronous_Object: public Thread
{
    Q_OBJECT
public:
    typedef void (T::*PTMF) (void);

    /**
     * Constructor.
     * @param object pointer to the object that should be made
     *               asynchronously called
     * @param method pointer to a method within the object, will be made
     *               the thread's active function
     */
    Asynchronous_Object (T* object, PTMF method)
	:Thread(), o_(object), m_(method)
    {};

    /*! Wrapper function to execute the object's method as a thread */
    void run() { (o_->*m_) (); }
    
private:
    T* o_;          /*!< pointer to the object */
    PTMF m_;        /*!< pointer to the member function within the object */
};

template <class T, class Arg1>
class Asynchronous_Object_with_1_arg : public Thread
{
public:
    typedef void (T::*PTMF) (Arg1);
    Asynchronous_Object_with_1_arg (T* o, PTMF m, Arg1 a1)
    : Thread(), o_ (o), m_ (m), a1_(a1)
    {};
    void run() { (o_->*m_) (a1_); }
private:
    T* o_;
    PTMF m_;
    Arg1 a1_;
};

//template <class T, class Arg1, class Arg2>
//class Asynchronous_Object_with_2_args : public Daemon {
//public:
//    typedef void (T::*PTMF) (Arg1, Arg2);
//    Asynchronous_Object_with_2_args (T* o, PTMF m, Arg1 a1, Arg2 a2,
//                                           bool respawn = false,
//					   bool standalone = false)
//    : Daemon(respawn, standalone), o_ (o), m_ (m), a1_(a1), a2_(a2)
//    {};
//    void execute () { (o_->*m_) (a1_, a2_); }
//private:
//    T* o_;
//    PTMF m_;
//    Arg1 a1_;
//    Arg2 a2_;
//};
//
//template <class T, class Arg1, class Arg2, class Arg3>
//class Asynchronous_Object_with_3_args : public Daemon {
//public:
//    typedef void (T::*PTMF) (Arg1, Arg2, Arg3);
//    Asynchronous_Object_with_3_args (T* o, PTMF m, Arg1 a1, Arg2 a2, Arg3 a3,
//                                           bool respawn = false,
//					   bool standalone = false)
//    : Daemon(respawn, standalone), o_ (o), m_ (m), a1_(a1), a2_(a2), a3_(a3)
//    { };
//    void execute () { (o_->*m_) (a1_, a2_, a3_); }
//private:
//    T* o_;
//    PTMF m_;
//    Arg1 a1_;
//    Arg2 a2_;
//    Arg3 a3_;
//};

#endif /* _ASYNCHRONOUS_OBJECT_H_ */

/* end of mt/Asynchronous_Object.h */
