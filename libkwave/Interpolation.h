

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

interpolation_t& operator++(interpolation_t i);

#include <qarray.h>
#include <qmap.h>
#include <qstringlist.h>

class Curve;

class Interpolation
{
public:
    /** Constructor, initializes type by enum type */
    Interpolation(interpolation_t type = INTPOL_LINEAR);

    /** Constructor, initializes type by name */
    Interpolation(const QString &name);

    /** Destructor. */
    ~Interpolation ();

    bool prepareInterpolation(Curve *points);

    QArray<double> *interpolation(Curve *points, unsigned int len);

    QArray<double> *limitedInterpolation(Curve *points, unsigned int len);

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
     * @param localized if true, the name is expected to be localized
     */
    static interpolation_t find(const QString &name, bool localized);

    /**
     * Returns the name of an interpolation (non-localized).
     * @param t type to get the name of
     */
    static QString name(interpolation_t type);

    /**
     * Returns an alphabetically sorted list of
     * interpolation type names.
     * @param localized if true, the list will contain localized
     *        names (useful for filling combo boxes)
     */
    static QStringList names(bool localized = false);

    /** Sets a new interpolation tpye */
    inline void setType (interpolation_t t) {
	m_type = t;
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
    void createFullPolynom(Curve *points, double *x, double *y);

    /**
     * ???
     * @param x array of x coordinates
     * @param y array of y coordinates
     * @param ab array for return values
     * @param n ???
     */
    void get2Derivate (const double *x, const double *y, double *ab, int n);

    /**
     * ???
     * @param points curve with points for interpolation
     * @param x array of x coordinates
     * @param y array of y coordinates
     * @param pos ???
     * @param degree ???
     */
    void createPolynom (Curve *points, double x[], double y[],
	int pos, int degree);

    /**  List of points to be interpolated. */
    Curve *m_curve;

    /** ??? used for temporary purposes */
    double *x;

    /** ??? used for temporary purposes */
    double *y;

    /** ??? used for temporary purposes */
    double *der;

    /** Type of the interpolation. */
    interpolation_t m_type;

    /**
     * Little private class for initialized map. Used
     * to translate interpolation_t into verbose name
     * and vice-versa.
     */
    class InterpolationMap: public QMap<interpolation_t, QString>
    {
    public:
	/** Constructor for filling the map. */
	InterpolationMap();
	
	/** Reverse Lookup, find an interpolation by name. */
	interpolation_t find(const QString &name, bool localized);

	/** Return a list of type names */
	QStringList names(bool localized);
    };

    /** Map with type and name of interpolations */
    static InterpolationMap m_interpolation_map;

};

#endif /* _INTERPOLATION_H_ */
