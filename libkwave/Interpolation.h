

#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_

typedef enum {
    INTPOL_LINEAR = 0,
    INTPOL_SPLINE,
    INTPOL_NPOLYNOMIAL,
    INTPOL_POLYNOMIAL3,
    INTPOL_POLYNOMIAL5,
    INTPOL_POLYNOMIAL7,
    INTPOL_SAH
} interpolation_t;

interpolation_t &operator ++(interpolation_t &i);

#include <qarray.h>
#include <qstringlist.h>

#include "libkwave/TypesMap.h"

class Curve;

class Interpolation
{
public:
    /** Constructor, initializes type by enum type */
    Interpolation(interpolation_t type = INTPOL_LINEAR);

    /** Destructor. */
    virtual ~Interpolation ();

    bool prepareInterpolation(Curve *points);

    QArray<double> interpolation(Curve *points, unsigned int len);

    QArray<double> limitedInterpolation(Curve *points, unsigned int len);

    /**
     * Returns a single point of the interpolation.
     */
    double singleInterpolation(double pos);

    /**
     * Same as getSingleInterpolation, but return value
     * will be limited to be [0...1]
     * @see #singleInterpolation
     * @param pos ???
     * @return interpolated value [0...1]
     */
    double singleLimitedInterpolation(double pos);

    /**
     * Returns the if of a type through it's name.
     * @param name the short name of the interpolation, like used in a command
     * @return the interpolation
     */
    static interpolation_t find(const QString &name)
    {
	debug("Interpolation::find("+name+")");
	return m_interpolation_map.findFromName(name);
    };

    /**
     * Returns the name of an interpolation (non-localized).
     * @param t type to get the name of
     */
    static QString name(interpolation_t type);

    /**
     * Returns an alphabetically sorted list of verbose
     * interpolation type names, useful for providing a list
     * of available types in the gui.
     * @param localized if true, the list will contain localized
     *        names (useful for filling combo boxes)
     */
    static QStringList descriptions(bool localized = false);

    /** Sets a new interpolation tpye */
    inline void setType (interpolation_t t) {
	m_type = t;
    };

    /** Returns the currently interpolation selected type */
    inline interpolation_t type() {
	return m_type;
    };

    /** Translates an index in an interpolation type */
    static inline interpolation_t findByIndex(int index) {
	return m_interpolation_map.findFromData(index);
    }

    /**
     * Little private class for initialized map. Used
     * to translate interpolation_t into verbose name
     * and vice-versa.
     */
    class InterpolationMap: public TypesMap<interpolation_t, int >
    {
    public:
	/** filling function for the map. */
	void fill();
    };

private:

    /** Returns the number of points **/
    unsigned int count();

    /**
     * ???
     * @param points curve with points for interpolation
     * @param x receives all x coordinates ???
     * @param y receives all y coordinates ???
     */
    void createFullPolynom(Curve *points, const QArray<double> &x,
	const QArray<double> &y);

    /**
     * ???
     * @param x array of x coordinates
     * @param y array of y coordinates
     * @param ab array for return values
     * @param n ???
     */
    void get2Derivate(const QArray<double> &x, const QArray<double> &y,
                      QArray<double> &ab, unsigned int n);

    /**
     * ???
     * @param points curve with points for interpolation
     * @param x array of x coordinates
     * @param y array of y coordinates
     * @param pos ???
     * @param degree ???
     */
    void createPolynom (Curve *points, QArray<double> &x, QArray<double> &y,
	int pos, unsigned int degree);

private:

    /**  List of points to be interpolated. */
    Curve *m_curve;

    /** ??? used for temporary purposes */
    QArray<double> m_x;

    /** ??? used for temporary purposes */
    QArray<double> m_y;

    /** ??? used for temporary purposes */
    QArray<double> m_der;

    /** Map with type and name of interpolations */
    static InterpolationMap m_interpolation_map;

    /** Type of the interpolation. */
    interpolation_t m_type;

};

#endif /* _INTERPOLATION_H_ */
