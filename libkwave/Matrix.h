/***************************************************************************
	       Matrix.h  -  template class describing a 2D grid structure
			     -------------------
    begin                : Fri May 18 2001
    copyright            : (C) 2001 by Martin Hinsch
    email                : vidas@sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "config.h"

template <class T> class Matrix
{

protected:
    /** Dimensions of the matrix. */
    unsigned mX, mY;

    /**
     * The raw array. It is allocated once and afterwards only changed
     * WRT values.
     */
    T *cell;

    /**
     * The distance between two lines. This enables sub-matrizes to be
     * created.
     */
    unsigned offset;

    /** Whether cell is our's. */
    bool ownData;

public:
    /** A constructor.*/
    Matrix(unsigned mx, unsigned my);
    Matrix(T * data, unsigned mx, unsigned my, unsigned offset = 0);
    Matrix(const Matrix < T > & other,
	   unsigned sx, unsigned sy, unsigned mx, unsigned my);

    /** The destructor. */
    ~Matrix();

    unsigned x() const {
	return mX;
    }

    unsigned y() const {
	return mY;
    }

    /** Get the value of the cell at (x, y). */
    T & at(int x, int y) const {
	return cell[x*(mY + offset) + y];
    }

    /** Get the xth column. Enables expressions like myMatrix[x][y]. */
    T * operator[](int x) const {
	return cell + x*(mY + offset);
    }

    /** Set the value at (x, y).*/
    void set(int x, int y, const T & to) {
	at(x, y) = to;
    }

    /** Inserts a new value at (x, y) (necessary for LSC)*/
    void insert(int x, int y, const T &to) {
	set(x, y, to);
    }

    void fill(const T & value) {
	for (unsigned x = 0; x < mX; x++)
	    for (unsigned y = 0; y < mY; y++)
		set(x, y, value);
    }

    /** Copy the value at (x1, y1) to (x2, y2). */
    void copy(int x1, int y1, int x2, int y2) {
	set(x2, y2, at(x1, y1));
    };

    /**
     * Apply a function to each cell of a rectangular area of the matrix.
     * This can be done vertically or horizontally.
     * @param aFunction The function to apply.
     * @param xFirst In which direction to work. If set to true
     *               The matrix will be iterated in x direction.
     * @param x1, y1, x2, y2 Limits of the region to apply
     *        the function to. x2 and y2 are set to the right respective lower
     *        border if they are smaller then x1 and y1.
     * @return A reference to the function object after the
     * last call.
     */
    template < class F >
    const F & apply(F & aFunction, bool colMajor = true,
		    unsigned x1 = 0, unsigned y1 = 0,
		    unsigned x2 = 0, unsigned y2 = 0) {
	x2 <= x1 ? (x2 = mX) : 0;
	y2 <= y1 ? (y2 = mY) : 0;
	unsigned x, y;
	if (colMajor)
	    for(x = x1; x < x2; x++)
		for(y = y1; y < y2; y++)
		    aFunction(at(x, y));
	else
	    for(y = y1; y < y2; y++)
		for(x = x1; x < x2; x++)
		    aFunction(at(x, y));

	return aFunction;
    }

    /**
     * Apply a function to each cell of a rectangular area of the matrix.
     * This can be done vertically or horizontally.
     * @param aFunction The function to apply.
     * @param xFirst In which direction to work. If set to true
     * The matrix will be iterated in x direction.
     * @param x1, y1, x2, y2 Limits of the region to apply
     * the function to. x2 and y2 are set to the right respective lower
     * border if they are smaller then x1 and y1.
     * @return A reference to the function object after the
     * last call.
     */
    template < class F >
    const F & scan(F & aConstFunction, bool xFirst = true,
		   unsigned x1 = 0, unsigned y1 = 0,
		   unsigned x2 = 0, unsigned y2 = 0) const {
	x2 <= x1 ? (x2 = mX) : 0;
	y2 <= y1 ? (y2 = mY) : 0;
	unsigned x, y;
	if (xFirst)
	    for(x = x1; x < x2; x++)
		for(y = y1; y < y2; y++)
		    aConstFunction(at(x, y));
	else
	    for(y = y1; y < y2; y++)
		for(x = x1; x < x2; x++)
		    aConstFunction(at(x, y));

	return aConstFunction;
    }

    /**
     * Create a sub-matrix. The resulting matrix operates on the same data
     * as the original one.
     */
    Matrix < T > sub(unsigned x, unsigned y, unsigned w, unsigned h) {
	return Matrix < T > (&(at(x, y)), w, h, mY - h + offset);
    }
};

template < class T >
Matrix < T > ::Matrix(unsigned mx, unsigned my) {
    mX = mx;
    mY = my;
    offset = 0;
    cell = new T[mX * mY];
    ownData = true;
}

template < class T >
Matrix < T > ::Matrix(T * data, unsigned mx, unsigned my, unsigned offs) {
    mX = mx;
    mY = my;
    offset = offs;
    cell = data;
    ownData = false;
}

template < class T >
Matrix < T > ::Matrix(const Matrix < T > & other,
		      unsigned sx, unsigned sy, unsigned mx, unsigned my) {
    cell = &(other.at(sx, sy));
    mX = mx;
    mY = my;
    offset = other.mY - mY + other.offset;
    ownData = false;
}

template < class T >
Matrix < T > ::~Matrix() {
    if (!cell || !ownData) return ;

    delete[] cell;
}

#endif /* _MATRIX_H_ */
