/***************************************************************************
               Matrix.h  -  simple template class for a NxM matrix
                             -------------------
    begin                : Sat May 12 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de

    based on Matrix.h from Martin Hinsch
    copyright (C) 2001 by Martin Hinsch <vidas@sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include "config.h"

#include <string.h>
#include <new>

#include <QtGlobal>

namespace Kwave
{

    template <class T> class Matrix
    {
    public:

        /**
         * constructor
         * @param cols number of columns
         * @param rows number of rows
         */
        Matrix(unsigned int cols, unsigned int rows)
            :m_rows(rows), m_cols(cols), m_data(nullptr)
        {
            m_data = new(std::nothrow) T[m_rows * m_cols];
            Q_ASSERT(m_data);
        }

        /** copy constructor */
        Matrix(const Matrix &other)
            :m_rows(other.m_rows), m_cols(other.m_cols), m_data(nullptr)
        {
            m_data = new(std::nothrow) T[m_rows * m_cols];
            Q_ASSERT(m_data);
            memcpy(m_data, other.m_data, m_rows * m_cols * sizeof(T));
        }

        /** destructor */
        virtual ~Matrix()
        {
            delete[] m_data;
            m_data = nullptr;
        }

        /** Get the xth column. Enables expressions like myMatrix[x][y]. */
        inline T * operator[] (unsigned int col) const {
            return m_data + (col * m_rows);
        }

    private:

        /** number of rows */
        unsigned int m_rows;

        /** number of columns */
        unsigned int m_cols;

        /** raw data */
        T *m_data;
    };
}

#endif /* MATRIX_H */

//***************************************************************************
//***************************************************************************
