/****************************************************************************
** MovingFilterDialog meta object code from reading C++ file 'filterwidget.h'
**
** Created: Fri Apr 23 21:54:30 1999
**      by: The Qt Meta Object Compiler ($Revision: 2.25 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "filterwidget.h"
#include <qmetaobject.h>


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
