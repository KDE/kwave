/****************************************************************************
** FilterDialog meta object code from reading C++ file 'filter.h'
**
** Created: Sun Aug 23 00:12:30 1998
**      by: The Qt Meta Object Compiler ($Revision: 2.25 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "filter.h"
#include <qmetaobject.h>


const char *FilterDialog::className() const
{
    return "FilterDialog";
}

QMetaObject *FilterDialog::metaObj = 0;

void FilterDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("FilterDialog","QDialog");
    if ( !QDialog::metaObject() )
	QDialog::initMetaObject();
    typedef void(FilterDialog::*m1_t0)(const char*);
    typedef void(FilterDialog::*m1_t1)(const char*);
    typedef void(FilterDialog::*m1_t2)(int);
    typedef void(FilterDialog::*m1_t3)();
    typedef void(FilterDialog::*m1_t4)();
    typedef void(FilterDialog::*m1_t5)();
    typedef struct Filter * (FilterDialog::*m1_t6)();
    m1_t0 v1_0 = &FilterDialog::setTaps;
    m1_t1 v1_1 = &FilterDialog::setOffset;
    m1_t2 v1_2 = &FilterDialog::setMult;
    m1_t3 v1_3 = &FilterDialog::refresh;
    m1_t4 v1_4 = &FilterDialog::loadFilter;
    m1_t5 v1_5 = &FilterDialog::saveFilter;
    m1_t6 v1_6 = &FilterDialog::getFilter;
    QMetaData *slot_tbl = new QMetaData[7];
    slot_tbl[0].name = "setTaps(const char*)";
    slot_tbl[1].name = "setOffset(const char*)";
    slot_tbl[2].name = "setMult(int)";
    slot_tbl[3].name = "refresh()";
    slot_tbl[4].name = "loadFilter()";
    slot_tbl[5].name = "saveFilter()";
    slot_tbl[6].name = "getFilter()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    slot_tbl[6].ptr = *((QMember*)&v1_6);
    metaObj = new QMetaObject( "FilterDialog", "QDialog",
	slot_tbl, 7,
	0, 0 );
}


const char *MovingFilterDialog::className() const
{
    return "MovingFilterDialog";
}

QMetaObject *MovingFilterDialog::metaObj = 0;

void MovingFilterDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QDialog::className(), "QDialog") != 0 )
	badSuperclassWarning("MovingFilterDialog","QDialog");
    if ( !QDialog::metaObject() )
	QDialog::initMetaObject();
    typedef void(MovingFilterDialog::*m1_t0)();
    typedef void(MovingFilterDialog::*m1_t1)(const char*);
    m1_t0 v1_0 = &MovingFilterDialog::toggleState;
    m1_t1 v1_1 = &MovingFilterDialog::checkTap;
    QMetaData *slot_tbl = new QMetaData[2];
    slot_tbl[0].name = "toggleState()";
    slot_tbl[1].name = "checkTap(const char*)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    metaObj = new QMetaObject( "MovingFilterDialog", "QDialog",
	slot_tbl, 2,
	0, 0 );
}
