/****************************************************************************
** MenuDialog meta object code from reading C++ file 'module.h'
**
** Created: Sun Apr 25 13:53:42 1999
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


const char *MenuDialog::className() const
{
    return "MenuDialog";
}

QMetaObject *MenuDialog::metaObj = 0;

void MenuDialog::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KwaveDialog::className(), "KwaveDialog") != 0 )
	badSuperclassWarning("MenuDialog","KwaveDialog");
    if ( !KwaveDialog::metaObject() )
	KwaveDialog::initMetaObject();
    metaObj = new QMetaObject( "MenuDialog", "KwaveDialog",
	0, 0,
	0, 0 );
}
