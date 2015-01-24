/***************************************************************************
            MetaData.h  -  base class for associated meta data
                             -------------------
    begin                : Sat Jan 23 2010
    copyright            : (C) 2010 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _META_DATA_H_
#define _META_DATA_H_

#include <config.h>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QSharedData>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <kdemacros.h>

#include "libkwave/Sample.h"

namespace Kwave
{

    class KDE_EXPORT MetaData
    {
    public:
	/** standard property: type of the meta data object */
	static const QString STDPROP_TYPE;

	/** standard property: list of zero based track indices */
	static const QString STDPROP_TRACKS;

	/** standard property: start sample index (inclusive) */
	static const QString STDPROP_START;

	/** standard property: end sample index (inclusive) */
	static const QString STDPROP_END;

	/** standard property: position [zero based sample index] */
	static const QString STDPROP_POS;

	/** standard property: description (string) */
	static const QString STDPROP_DESCRIPTION;

	typedef enum
	{
	    /** no scope */
	    None     = 0,

	    /** whole signal */
	    Signal   = (1 << 0),

	    /**
	     * bound to a (list of) tracks, requires the property
	     * "STDPROP_TRACKS" with a QVariant::List of track indices
	     * encoded as QVariant::UInt
	     */
	    Track    = (1 << 1),

	    /**
	     * bound to a range of samples, requires the properties
	     * "STDPROP_START" and "STDPROP_END" with data type
	     * QVariant::ULongLong
	     */
	    Range    = (1 << 2),

	    /**
	     * bound to a single sample, requires the property
	     * "STDPROP_POS" with data type "QVariant::ULongLong"
	     */
	    Position = (1 << 3),

	    /** can be used for selecting all the scopes above */
	    All      = ~0
	} Scope;

	/** List of metadata properties */
	typedef QMap<QString, QVariant> PropertyList;

	/**
	 * default constructor, generates a metadata object
	 * with a new ID
	 */
	MetaData();

	/**
	 * copy constructor
	 * @param other the other meta data object to copy from
	 */
	MetaData(const MetaData &other);

	/** constructor */
	MetaData(Scope scope);

	/** destructor */
	virtual ~MetaData();

	/** removes all properties */
	virtual void clear();

	/** returns true if this is an empty record */
	virtual bool isNull() const;

	/** returns the ID of the meta data */
	QString id() const;

	/** returns the scope of the meta data */
	Scope scope() const;

	/**
	 * Sets the scope of the meta data
	 * @param scope the new scope
	 */
	void setScope(Scope scope);

	/**
	 * Sets a property to a new value. If the property already exists
	 * it will be created and if it did not exist, a new one will be
	 * created. If the value is not valid (null), the property will
	 * be deleted.
	 * @param p name of the property
	 * @param value a QVariant with the property's data
	 */
	void setProperty(const QString &p, const QVariant &value);

	/**
	 * Checks whether this metadata object contains a given property.
	 * @param p name of the property
	 * @return true if the property exists, false otherwise
	 */
	bool hasProperty(const QString &p) const;

	/**
	 * Returns a QVariant with the copy of the value of a property
	 * or an empty QVariant if the property does not exist.
	 * @param p name of the property
	 * @return value of the property or an empty QVariant
	 */
	QVariant property(const QString &p) const;

	/** Same as above, for using through the [] operator */
	inline QVariant operator [] (const QString p) const
	{
	    return property(p);
	}

	/**
	 * Returns a mutable reference to an existing property (or the
	 * reference to an empty dummy if it did not exist).
	 * @param p name of the property
	 * @return reference to the value of the property
	 */
	QVariant &property(const QString &p);

	/** Same as above, for using through the [] operator */
	inline QVariant &operator [] (const QString p)
	{
	    return property(p);
	}

	/** equal operator, compares by data (not by ID) */
	bool operator == (const MetaData &other) const;

	/** not equal operator, compares by data (not by ID) */
	inline bool operator != (const MetaData &other) const
	{
	    return !(operator == (other));
	}

	/** returns a list with all property names */
	QStringList keys() const;

	/** returns a list of position bount property names */
	static QStringList positionBoundPropertyNames();

	/**
	 * Returns the index of the first sample covered by a given
	 * meta data item
	 * @return index of the first sample
	 */
	sample_index_t firstSample() const;

	/**
	 * Returns the index of the last sample covered by a given
	 * meta data item
	 * @return index of the last sample
	 */
	sample_index_t lastSample() const;

	/**
	 * Returns a list of tracks a meta data item is bound to
	 * @return list of bound tracks or empty list if not bound
	 */
	QList<unsigned int> boundTracks() const;

	/** dump all properties to stdout, for debugging */
	virtual void dump() const;

    private:

	/** internal container class with meta data */
	class MetaDataPriv: public QSharedData
	{
	public:

	    /** constructor */
	    MetaDataPriv();

	    /** copy constructor */
	    MetaDataPriv(const MetaDataPriv &other);

	    /** destructor */
	    virtual ~MetaDataPriv();

	    /** id of the meta data */
	    QString m_id;

	    /** scope of the meta data */
	    Scope m_scope;

	    /** list of properties, user defined */
	    PropertyList m_properties;

	private:

	    /** creates a new unique ID */
	    static QString newUid();

	    /** counter for unique id generation */
	    static quint64 m_id_counter;

	    /** mutex for protecting the id generator */
	    static QMutex m_id_lock;
	};

	/** pointer to the shared meta data */
	QSharedDataPointer<MetaDataPriv> m_data;
    };
}

#endif /* _META_DATA_H_ */

//***************************************************************************
//***************************************************************************
