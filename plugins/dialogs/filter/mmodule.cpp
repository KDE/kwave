/****************************************************************************
** FilterDialog meta object code from reading C++ file 'module.h'
**
** Created: Tue Apr 13 16:55:31 1999
**      by: The Qt Meta Object Compiler ($Revision: 2.25 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 2
#elif Q_MOC_OUTPUT_REVISION != 2
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "module.h"
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
    if ( strcmp(KwaveDialog::className(), "KwaveDialog") != 0 )
	badSuperclassWarning("FilterDialog","KwaveDialog");
    if ( !KwaveDialog::metaObject() )
	KwaveDialog::initMetaObject();
    typedef void(FilterDialog::*m1_t0)(const char*);
    typedef void(FilterDialog::*m1_t1)(const char*);
    typedef void(FilterDialog::*m1_t2)(int);
    typedef void(FilterDialog::*m1_t3)();
    typedef void(FilterDialog::*m1_t4)();
    typedef void(FilterDialog::*m1_t5)();
    m1_t0 v1_0 = &FilterDialog::setTaps;
    m1_t1 v1_1 = &FilterDialog::setOffset;
    m1_t2 v1_2 = &FilterDialog::setMult;
    m1_t3 v1_3 = &FilterDialog::refresh;
    m1_t4 v1_4 = &FilterDialog::loadFilter;
    m1_t5 v1_5 = &FilterDialog::saveFilter;
    QMetaData *slot_tbl = new QMetaData[6];
    slot_tbl[0].name = "setTaps(const char*)";
    slot_tbl[1].name = "setOffset(const char*)";
    slot_tbl[2].name = "setMult(int)";
    slot_tbl[3].name = "refresh()";
    slot_tbl[4].name = "loadFilter()";
    slot_tbl[5].name = "saveFilter()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    slot_tbl[2].ptr = *((QMember*)&v1_2);
    slot_tbl[3].ptr = *((QMember*)&v1_3);
    slot_tbl[4].ptr = *((QMember*)&v1_4);
    slot_tbl[5].ptr = *((QMember*)&v1_5);
    typedef void(FilterDialog::*m2_t0)(const char*);
    m2_t0 v2_0 = &FilterDialog::command;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "command(const char*)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "FilterDialog", "KwaveDialog",
	slot_tbl, 6,
	signal_tbl, 1 );
}

// SIGNAL command
void FilterDialog::command( const char* t0 )
{
    activate_signal( "command(const char*)", t0 );
}
