/****************************************************************************
** PitchDialog meta object code from reading C++ file 'module.h'
**
** Created: Wed Apr 14 12:50:52 1999
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


const char *PitchDialog::className() const
{
    return "PitchDialog";
}

QMetaObject *PitchDialog::metaObj = 0;

void PitchDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KwaveDialog::className(), "KwaveDialog") != 0 )
	badSuperclassWarning("PitchDialog","KwaveDialog");
    if ( !KwaveDialog::metaObject() )
	KwaveDialog::initMetaObject();
    typedef void(PitchDialog::*m2_t0)(const char*);
    m2_t0 v2_0 = &PitchDialog::command;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "command(const char*)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "PitchDialog", "KwaveDialog",
	0, 0,
	signal_tbl, 1 );
}

// SIGNAL command
void PitchDialog::command( const char* t0 )
{
    activate_signal( "command(const char*)", t0 );
}
