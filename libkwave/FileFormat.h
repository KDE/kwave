// /***************************************************************************
//            FileFormat.h  -  information about a file format
// 			     -------------------
//     begin                : Mar 05 2002
//     copyright            : (C) 2002 by Thomas Eschenbacher
//     email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
//  ***************************************************************************/
//
// /***************************************************************************
//  *                                                                         *
//  *   This program is free software; you can redistribute it and/or modify  *
//  *   it under the terms of the GNU General Public License as published by  *
//  *   the Free Software Foundation; either version 2 of the License, or     *
//  *   (at your option) any later version.                                   *
//  *                                                                         *
//  ***************************************************************************/
//
// #ifndef _FILE_FORMAT_H_
// #define _FILE_FORMAT_H_
//
// #include "config.h"
// #include <QString>
// #include <QStringList>
//
// namespace Kwave
// {
//     /**
//     * Encapsulates all information for identifying a file format.
//     */
//     class FileFormat
//     {
//     public:
// 	/** Constructor */
// 	FileFormat(const QString &mime_type, const QStringList &sub_types,
// 	           const QString &description,
// 	           const QStringList &extensions);
//
// 	/** Destructor */
// 	virtual ~FileFormat();
//
// 	/** returns the mime type as String */
// 	QString mimeType();
//
// 	/** returns the list of subtypes (optional, might be empty) */
// 	QStringList subTypes();
//
// 	/** return the description of the format (no i18n) */
// 	QString description();
//
// 	/** returns the list of file extensions (optional, might be empty) */
// 	QStringList extensions();
//
//     private:
//
// 	/** name of the mime type */
// 	QString m_mime_type;
//
// 	/** list of subtypes, optional/can be empty */
// 	QStringList m_sub_types;
//
// 	/** user-readable description */
// 	QString m_description;
//
// 	/** list of associated file extensions */
// 	QStringList m_extensions;
//
//     };
// }
//
// #endif /* _FILE_FORMAT_H_ */
//
// //***************************************************************************
// //***************************************************************************
