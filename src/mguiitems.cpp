/****************************************************************************
** FloatLine meta object code from reading C++ file 'guiitems.h'
**
** Created: Wed Feb 17 16:47:51 1999
**      by: The Qt Meta Object Compiler ($Revision: 2.25 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "guiitems.h"
#include <qmetaobject.h>


const char *FloatLine::className() const
{
    return "FloatLine";
}

QMetaObject *FloatLine::metaObj = 0;

void FloatLine::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KRestrictedLine::className(), "KRestrictedLine") != 0 )
	badSuperclassWarning("FloatLine","KRestrictedLine");
    if ( !KRestrictedLine::metaObject() )
	KRestrictedLine::initMetaObject();
    metaObj = new QMetaObject( "FloatLine", "KRestrictedLine",
	0, 0,
	0, 0 );
}


const char *TimeLine::className() const
{
    return "TimeLine";
}

QMetaObject *TimeLine::metaObj = 0;

void TimeLine::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KRestrictedLine::className(), "KRestrictedLine") != 0 )
	badSuperclassWarning("TimeLine","KRestrictedLine");
    if ( !KRestrictedLine::metaObject() )
	KRestrictedLine::initMetaObject();
    typedef void(TimeLine::*m1_t0)();
    typedef void(TimeLine::*m1_t1)();
    typedef void(TimeLine::*m1_t2)();
    typedef void(TimeLine::*m1_t3)();
    typedef void(TimeLine::*m1_t4)(const char*);
    typedef void(TimeLine::*m1_t5)(int);
    m1_t0 v1_0 = &TimeLine::setSampleMode;
    m1_t1 v1_1 = &TimeLine::setMsMode;
    m1_t2 v1_2 = &TimeLine::setKbMode;
    m1_t3 v1_3 = &TimeLine::setSMode;
    m1_t4 v1_4 = &TimeLine::setValue;
    m1_t5 v1_5 = &TimeLine::setRate;
    QMetaData *slot_tbl = new QMetaData[6];
    slot_tbl[0].name = "setSampleMode()";
    slot_tbl[1].name = "setMsMode()";
    slot_tbl[2].name = "setKbMode()";
    slot_tbl[3].name = "setSMode()";
    slot_tbl[4].name = "setValue(const char*)";
    slot_tbl[5].name = "setRate(int)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    metaObj = new QMetaObject( "TimeLine", "KRestrictedLine",
	slot_tbl, 6,
	0, 0 );
}
