/****************************************************************************
** PercentDialog meta object code from reading C++ file 'module.h'
**
** Created: Fri Apr 16 00:02:12 1999
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


const char *PercentDialog::className() const
{
    return "PercentDialog";
}

QMetaObject *PercentDialog::metaObj = 0;

void PercentDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KwaveDialog::className(), "KwaveDialog") != 0 )
	badSuperclassWarning("PercentDialog","KwaveDialog");
    if ( !KwaveDialog::metaObject() )
	KwaveDialog::initMetaObject();
    typedef void(PercentDialog::*m1_t0)(int);
    m1_t0 v1_0 = &PercentDialog::setValue;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "setValue(int)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(PercentDialog::*m2_t0)(const char*);
    m2_t0 v2_0 = &PercentDialog::command;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "command(const char*)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "PercentDialog", "KwaveDialog",
	slot_tbl, 1,
	signal_tbl, 1 );
}

// SIGNAL command
void PercentDialog::command( const char* t0 )
{
    activate_signal( "command(const char*)", t0 );
}
