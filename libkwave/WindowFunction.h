/***************************************************************************
       WindowFunction.h  -  Windows functions for signal processing
			     -------------------
    begin                : Feb 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _WINDOW_FUNCTION_H_
#define _WINDOW_FUNCTION_H_

#include "config.h"

#include <QtCore/QVector>

#include <kdemacros.h>

#include "libkwave/TypesMap.h"

class QString;

namespace Kwave
{

    /** enumeration of window functions */
    typedef enum {
	WINDOW_FUNC_NONE = 0,
	WINDOW_FUNC_HAMMING = 1,
	WINDOW_FUNC_HANNING = 2,
	WINDOW_FUNC_BLACKMAN = 3,
	WINDOW_FUNC_TRIANGULAR = 4
    } window_function_t;

    /**
     * @class WindowFunction
     * Handles window functions for signal processing. Also holds a static
     * map of known window functions.
     */
    class KDE_EXPORT WindowFunction
    {
    public:
	/**
	 * Constructor
	 * @param type initial window function type.
	 */
	explicit WindowFunction(window_function_t type);

	/** Destructor */
	virtual ~WindowFunction();

	/**
	 * Returns the coefficients of a window function with
	 * the given number of points. This is similar to Kwave's
	 * Interpolation class.
	 */
	QVector<double> points(unsigned int len) const;

	/**
	 * Returns the window function id through it's numeric index. If
	 * the index is out of range, the return value will be "WINDOW_FUNC_NONE".
	 * @param index numeric index to be searched [0...count-1]
	 */
	static window_function_t findFromIndex(unsigned int index) {
	    return m_types_map.findFromData(index);
	}

	/**
	 * Returns the window function id through it's name. If
	 * the name is unknown the return value will be "WINDOW_FUNC_NONE".
	 * @param name the name to be searched
	 */
	static window_function_t findFromName(const QString &name) {
	    return m_types_map.findFromName(name);
	}

	/**
	 * Returns the numeric index of a window function [0...count-1].
	 * @param type the type of the window function
	 */
	static unsigned int index(window_function_t type)
	{
	    return m_types_map.data(type);
	}

	/**
	 * Returns the name of a window function.
	 * @param type the type of the window function
	 */
	static const QString name(window_function_t type)
	{
	    return m_types_map.name(type);
	}

	/**
	 * Returns the description of a window function.
	 * @param type the type of the window function
	 * @param localized if true, the localized description will be returned
	 *        instead of the non-localized one
	 */
	static QString description(window_function_t type,
	    bool localized)
	{
	    return m_types_map.description(type, localized);
	}

	/** returns the number of available window functions */
	static unsigned int count() {
	    return m_types_map.count();
	}

	/**
	 * This map will be initialized with all known window functions.
	 */
	class InitializedTypesMap:
	    public Kwave::TypesMap<window_function_t,unsigned int>
	{
	public:
	    /** Constructor */
	    explicit InitializedTypesMap()
		:Kwave::TypesMap<window_function_t, unsigned int>()
	    {
		fill();
	    }

	    /** fill function for the map */
	    virtual void fill();
	};

    private:

	/** id of the window function */
	window_function_t m_type;

	/** Static map of window function types. */
	static InitializedTypesMap m_types_map;
    };

    //***************************************************************************
    static inline window_function_t &operator++(window_function_t &f)
    {
	return (f = (f == WINDOW_FUNC_TRIANGULAR) ?
	    WINDOW_FUNC_NONE : window_function_t(f+1) );
    }

}

#endif /* _WINDOW_FUNCTION_H_ */

//***************************************************************************
//***************************************************************************
