
#include "klocale.h"

#include "Curve.h"
#include "Interpolation.h"

//***************************************************************************
//***************************************************************************

interpolation_t& operator++(interpolation_t i)
{
    return (i = (i == INTPOL_SAH) ? INTPOL_LINEAR : interpolation_t(i+1) );
}

//***************************************************************************
//***************************************************************************
Interpolation::InterpolationMap::InterpolationMap()
{
    insert(INTPOL_LINEAR,      "linear");    i18n("linear");
    insert(INTPOL_SPLINE,      "spline");    i18n("spline");
    insert(INTPOL_NPOLYNOMIAL, "n-polynom"); i18n("n-polynom");
    insert(INTPOL_POLYNOMIAL3, "3-polynom"); i18n("3-polynom");
    insert(INTPOL_POLYNOMIAL5, "5-polynom"); i18n("5-polynom");
    insert(INTPOL_POLYNOMIAL7, "5-polynom"); i18n("7-polynom");
    insert(INTPOL_SAH,   "sample and hold"); i18n("sample and hold");
}

//***************************************************************************
interpolation_t Interpolation::InterpolationMap::find(const QString &name,
	bool localized)
{
    InterpolationMap::Iterator it;
    for (it = begin(); it != end(); ++it ) {
	if (name == (localized ? QString(i18n(it.data())) : it.data() ) )
	    return it.key();
    }
    ASSERT(false && "unknown interpolation type");
    return INTPOL_LINEAR;
}
	
//***************************************************************************
QStringList Interpolation::InterpolationMap::names(bool localized)
{
    QStringList list;
    InterpolationMap::Iterator it;
    for (it = begin(); it != end(); ++it ) {
	list.append(localized ? QString(i18n(it.data())) : it.data() );
    }
    list.sort();
    return list;
}

//***************************************************************************
//***************************************************************************

// static initializer
Interpolation::InterpolationMap Interpolation::m_interpolation_map;

//***************************************************************************
Interpolation::Interpolation(const QString &name)
{
    x = 0;
    y = 0;
    der = 0;
    m_type = m_interpolation_map.find(name, false);
}

//***************************************************************************
Interpolation::Interpolation(interpolation_t type)
{
    x = 0;
    y = 0;
    der = 0;
    m_type = type;
}

//***************************************************************************
Interpolation::~Interpolation()
{
    if (x) delete x;
    if (y) delete y;
    if (der) delete[] der;
}

//***************************************************************************
interpolation_t Interpolation::find(const QString &name, bool localized)
{
    return m_interpolation_map.find(name, localized);
}

//***************************************************************************
QStringList Interpolation::names(bool localized)
{
    return m_interpolation_map.names(localized);
}

//***************************************************************************
QString Interpolation::name(interpolation_t type)
{
    return m_interpolation_map[type];
}

//***************************************************************************
unsigned int Interpolation::count()
{
    return (m_curve ? m_curve->count() : 0);
}

//***************************************************************************
double Interpolation::singleInterpolation(double input)
{
    ASSERT(count());
    if (!count()) return 0; // no data ?

    unsigned int degree = 0;

    switch (m_type) {
	case INTPOL_LINEAR:
	    {
		unsigned int i = 1;
		// ### range checking ???
		while (x[i] < input) i++;
		double dif1 = x[i] - x[i - 1];  //!=0 per definition
		double dif2 = input - x[i - 1];

		return (y[i -1] + ((y[i] - y[i - 1])*dif2 / dif1));
	    }
	case INTPOL_SPLINE:
	    {
		double a, b, diff;
		unsigned int j = 1;
		// ### range checking ???
		while (x[j] < input) j++;
		diff = x[j] - x[j - 1];

		a = (x[j] - input) / diff;    //div should not be 0
		b = (input - x[j - 1]) / diff;

		return (a*y[j - 1] + b*y[j] + ((a*a*a - a)*der[j - 1] +
		       (b*b*b - b)*der[j])*(diff*diff) / 6);
	    }
	case INTPOL_NPOLYNOMIAL:
	    {
		double ny = y[0];
		for (unsigned int j = 1; j < count(); j++)
		    ny = ny * (input - x[j]) + y[j];
		return ny;
	    }
	case INTPOL_SAH:     //Sample and hold
	    {
		int i = 1;
		// ### range checking ???
		while (x[i] < input) i++;
		return y[i -1];
	    }
	case INTPOL_POLYNOMIAL3:
	    degree = 3;
	    break;
	case INTPOL_POLYNOMIAL5:
	    degree = 5;
	    break;
	case INTPOL_POLYNOMIAL7:
	    degree = 7;
	    break;		
    }
	
    if (degree && (degree <= 7)) {
	// use polynom
	double ny, ax[7], ay[7];

	int i = 1;
	while (x[i] < input) i++;

	createPolynom (m_curve, ax, ay, i - 1 - degree / 2, degree);

	ny = ay[0];
	for (unsigned int j = 1; j < degree; j++)
	    ny = ny * (input - ax[j]) + ay[j];
	return ny;
    }

    return 0;
}

//***************************************************************************
bool Interpolation::prepareInterpolation(Curve *points)
{
    m_curve = points;

    ASSERT(count());
    if (!count()) return false; // no data ?

    if (x)   delete [] x;
    if (y)   delete [] y;
    if (der) delete [] der;
    der = 0;

    x = new double[count() + 1];
    ASSERT(x);
    if (!x) return false;

    y = new double[count() + 1];
    ASSERT(y);
    if (!y) return false;

    int c = 0;
    for (Point *p = points->first(); p; p = points->next(p)) {
	x[c] = p->x;
	y[c] = p->y;
	c++;
    }
    x[c] = y[c] = 0.0;

    switch (m_type) {
	case INTPOL_NPOLYNOMIAL:
	    createFullPolynom (points, x, y);
	    break;
	case INTPOL_SPLINE:
	    if (der) delete[] der;
	    der = new double[count() + 1];
	    ASSERT(der);
	    if (!der) return false;

	    get2Derivate(x, y, der, count());
	    break;
	default:
	    ;
    }
    return true;
}

//***************************************************************************
QArray<double> *Interpolation::limitedInterpolation(Curve *points,
	unsigned int len)
{
    ASSERT(points);
    ASSERT(len);
    if (!points) return false;
    if (!len) return false;

    QArray<double> *y = interpolation(points, len);
    ASSERT(y);
    if (!y) return 0;

    for (unsigned int i = 0; i < len; i++) {
	if (*y[i] > 1) y[i] = 1;
	if (*y[i] < 0) y[i] = 0;
    }
    return y;
}

//***************************************************************************
QArray<double> *Interpolation::interpolation(Curve *points, unsigned int len)
{
    ASSERT(points);
    ASSERT(len);
    if (!points) return 0;
    if (!len) return 0;

    Point *tmp;
    unsigned int degree = 0;

    double *y_out = new double[len];
    ASSERT(y_out);
    if (!y_out) return 0;

    for (unsigned int i=0; i<len; i++) {
	y_out[i] = 0.0;
    }

    switch (m_type) {
	case INTPOL_LINEAR:
	    {
		double x, y, lx, ly;
		tmp = points->first();
		if (tmp) {
		    lx = tmp->x;
		    ly = tmp->y;

		    for (tmp = points->next(tmp); tmp; tmp = points->next(tmp)) {
			x = tmp->x;
			y = tmp->y;

			double dify = (y - ly);
			int difx = (int) ((x - lx) * len);
			int min = (int)(lx * len);
			double h;

			for (int i = (int)(lx * len); i < (int)(x*len); i++) {
			    h = (double(i - min)) / difx;
			    y_out[i] = ly + (h * dify);
			}
			lx = x;
			ly = y;
		    }
		}
		break;
	    }
	case INTPOL_SPLINE:
	    {
		int t = 1;
		unsigned int count = points->count();

		double ny = 0;
		double der[count + 1];
		double x[count + 1];
		double y[count + 1];

		for (tmp = points->first(); tmp; tmp = points->next(tmp)) {
		    x[t] = tmp->x;
		    y[t] = tmp->y;
		    t++;
		}

		get2Derivate (x, y, der, count);

		int ent;
		int start = (int) (x[1] * len);

		for (unsigned int j = 2; j <= count; j++) {
		    ent = (int) (x[j] * len);
		    for (int i = start; i < ent; i++) {
			double xin = ((double) i) / len;
			double h, b, a;

			h = x[j] - x[j - 1];

			if (h != 0) {
			    a = (x[j] - xin) / h;
			    b = (xin - x[j - 1]) / h;

			    ny = (a * y[j - 1] + b * y[j] +
			         ((a * a * a - a) * der[j - 1] +
			         (b * b * b - b) * der[j]) * (h * h) / 6.0);
			}

			y_out[i] = ny;
			start = ent;
		    }
		}
		break;
	    }
	case INTPOL_POLYNOMIAL3:
	    if (!degree) degree = 3;
	    // ### ??? and now ???
	case INTPOL_POLYNOMIAL5:
	    if (!degree) degree = 5;
	    // ### ??? and now ???
	case INTPOL_POLYNOMIAL7:
	    {
		if (!degree) degree = 7;
		unsigned int count = points->count();
		double ny, x[7], y[7];
		double ent, start;

		tmp = points->first();
		if (tmp) {
		    for (unsigned int px = 0; px < count - 1; px++) {
			createPolynom (points, x, y, px - degree / 2, degree);
			start = points->at(px)->x;

			if (px >= count - degree / 2 + 1)
			    ent = 1;
			else
			    ent = points->at(px + 1)->x;

			for (int i=(int)(start*len); i<(int)(ent*len); i++) {
			    ny = y[0];
			    for (unsigned int j = 1; j < degree; j++)
				ny = ny * (((double)i) / len - x[j]) + y[j];

			    y_out[i] = ny;
			}
		    }
		}
		break;
	    }
	case INTPOL_NPOLYNOMIAL:
	    {
		double ny;
		int count = points->count();

		tmp = points->first();
		if (tmp != 0) {
		    double x[count + 1];
		    double y[count + 1];
		    double px;

		    createFullPolynom (points, x, y);

		    for (unsigned int i = 1; i < len; i++) {
			px = (double)(i) / len;

			ny = y[0];
			for (int j = 1; j < count; j++)
			    ny = ny * (px - x[j]) + y[j];

			y_out[i] = ny;

		    }
		}
		break;
	    }
	case INTPOL_SAH:
	    {
		double lx, ly, x, y;

		tmp = points->first();
		if (tmp != 0) {
		    lx = tmp->x;
		    ly = tmp->y;

		    for (tmp = points->next(tmp); tmp; tmp=points->next(tmp)) {
			x = tmp->x;
			y = tmp->y;

			for (int i = (int)(lx * len); i < (int)(x*len); i++)
			    y_out[i] = ly;

			lx = x;
			ly = y;
		    }
		}
	    }
    }

    QArray<double> *yo = new QArray<double>;
    ASSERT(yo);
    if (yo) yo->assign(y_out, len);
    return yo;
}

//***************************************************************************
void Interpolation::createFullPolynom (Curve *points, double *x, double *y)
{
    unsigned int count = 0;
    Point *tmp;

    ASSERT(points);
    ASSERT(m_curve);
    ASSERT(x);
    ASSERT(y);
    if (!points) return;
    if (!m_curve) return;
    if (!x) return;
    if (!y) return;

    ASSERT(points->count() == m_curve->count());
    if (points->count() == m_curve->count()) return;

    for (tmp = points->first(); tmp; tmp = points->next(tmp)) {
	x[count] = tmp->x;
	y[count] = tmp->y;
	count++;
    }

    for (unsigned int k = 0; k < count; k++)
	for (unsigned int j = k; j != 0; )
	{
	    j--;
	    y[j] = (y[j] - y[j + 1]) / (x[j] - x[k]);
	}
}

//***************************************************************************
void Interpolation::get2Derivate(const double *x, const double *y,
	double *ab, int n)
{
    ASSERT(x);
    ASSERT(y);
    ASSERT(ab);
    ASSERT(n);
    if (!x) return;
    if (!y) return;
    if (!ab) return;
    if (!n) return;

    int i, k;
    double p, qn, sig, un;

    double *u = new double[n];
    ASSERT(u);
    if (!u) return;

    ab[0] = ab[1] = 0;
    u[0] = u[1] = 0;

    for (i = 2; i < n; i++) {
	sig = (x[i] - x[i - 1]) / (x[i + 1]-x[i - 1]);
	p = sig * ab[i - 1] + 2;
	ab[i] = (sig - 1) / p;
	u[i] = (y[i + 1]-y[i]) / (x[i + 1]-x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
	u[i] = (6 * u[i] / (x[i + 1]-x[i - 1]) - sig * u[i - 1]) / p;
    }

    qn = 0;
    un = 0;
    ab[n] = (un - qn * u[n - 1]) / (qn * ab[n - 1] + 1);

    for (k = n - 1; k > 0; k--)
	ab[k] = ab[k] * ab[k + 1] + u[k];

    delete [] u;
}

//***************************************************************************
void Interpolation::createPolynom (Curve *points, double x[], double y[],
	int pos, int degree)
{
    Point *tmp;
    int count = 0;

    if (pos < 0) {
	switch (pos) {
	   case - 3:
		x[count] = -1.5;
		y[count++] = points->first()->y;
		pos++;
	   case - 2:
		x[count] = -1;
		y[count++] = points->first()->y;
		pos++;
	   case - 1:
		x[count] = -.5;
		y[count++] = points->first()->y;
		pos = 0;
	}
    }

    tmp = points->first();
    for (int i = 0; i < pos; i++) tmp = points->next(tmp);

    for (; (count < degree) && (tmp); tmp = points->next(tmp)) {
	x[count] = tmp->x;
	y[count++] = tmp->y;
    }

    int i = 1;
    for (; count < degree; count++) {
	x[count] = 1 + .5 * (i++);
	y[count] = points->last()->y;
    }

    for (int k = 0; k < degree; k++)  //create coefficients in y[i] and x[i];
	for (int j = k - 1; j >= 0; j--)
	    y[j] = (y[j] - y[j + 1]) / (x[j] - x[k]);
}

//***************************************************************************
//***************************************************************************
