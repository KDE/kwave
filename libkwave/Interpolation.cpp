
#include "klocale.h"

#include "Curve.h"
#include "Interpolation.h"

//***************************************************************************
//***************************************************************************

interpolation_t &operator ++(interpolation_t &i)
{
    return (i = (i == INTPOL_SAH) ? INTPOL_LINEAR : interpolation_t(i+1) );
}

//***************************************************************************
//***************************************************************************
void Interpolation::InterpolationMap::fill()
{
    debug("--- InterpolationMap::fill() ---");
    append(INTPOL_LINEAR,      0, "linear",      "linear");
    append(INTPOL_SPLINE,      1, "spline",      "spline");
    append(INTPOL_NPOLYNOMIAL, 2, "n-polynom",   "polynom, nth degree");
    append(INTPOL_POLYNOMIAL3, 3, "3-polynom",   "polynom, 3rd degree");
    append(INTPOL_POLYNOMIAL5, 4, "5-polynom",   "polynom, 5th degree");
    append(INTPOL_POLYNOMIAL7, 5, "5-polynom",   "polynom, 7th degree");
    append(INTPOL_SAH,         6, "sample_hold", "sample and hold");

#undef NEVER_COMPILE_THIS
#ifdef NEVER_COMPILE_THIS
#error "this could produce problems in plugins and/or libs when \
        loaded before the main application is up."
    i18n("linear");
    i18n("spline");
    i18n("n-polynom");
    i18n("3-polynom");
    i18n("5-polynom");
    i18n("7-polynom");
    i18n("sample and hold");
#endif
}

//***************************************************************************
//***************************************************************************

// static initializer
Interpolation::InterpolationMap Interpolation::m_interpolation_map;

//***************************************************************************
Interpolation::Interpolation(interpolation_t type)
  :m_curve(), m_x(), m_y(), m_der(), m_type(type)
{
}

//***************************************************************************
Interpolation::~Interpolation()
{
}

//***************************************************************************
QStringList Interpolation::descriptions(bool localized)
{
    QStringList list;
    unsigned int count = m_interpolation_map.count();
    unsigned int i;
    for (i=0; i < count; i++) {
	interpolation_t index = m_interpolation_map.findFromData(i);
	list.append(m_interpolation_map.description(index, localized));
    }
    return list;
}

//***************************************************************************
QString Interpolation::name(interpolation_t type)
{
    return m_interpolation_map.name(type);
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
    if (!count()) return 0.0; // no data ?

    unsigned int degree = 0;
    unsigned int count = this->count();

    switch (m_type) {
	case INTPOL_LINEAR:
	    {
		unsigned int i = 1;
		while ((m_x[i] < input) && (i < count))
		    i++;
		
		double dif1 = m_x[i] - m_x[i-1];  //!=0 per definition
		double dif2 = input - m_x[i-1];
		
		return (m_y[i-1] + ((m_y[i] - m_y[i-1])*dif2 / dif1));
	    }
	case INTPOL_SPLINE:
	    {
		double a, b, diff;
		unsigned int j = 1;
		
		while ((m_x[j] < input) && (j < count))
		    j++;
		
		diff = m_x[j] - m_x[j-1];

		a = (m_x[j] - input) / diff;    //div should not be 0
		b = (input - m_x[j-1]) / diff;

		return (a*m_y[j-1] + b*m_y[j] + ((a*a*a - a)*m_der[j - 1] +
		       (b*b*b - b)*m_der[j])*(diff*diff) / 6);
	    }
	case INTPOL_NPOLYNOMIAL:
	    {
		double ny = m_y[0];
		for (unsigned int j = 1; j < count; j++)
		    ny = ny * (input - m_x[j]) + m_y[j];
		return ny;
	    }
	case INTPOL_SAH:     //Sample and hold
	    {
		int i = 1;
		// ### range checking ???
		while (m_x[i] < input) i++;
		return m_y[i-1];
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
	double ny;
	QArray<double> ax(7);
	QArray<double> ay(7);
	
	int i = 1;
	while (m_x[i] < input) i++;
	
	createPolynom(m_curve, ax, ay, i - 1 - degree/2, degree);
	
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

    m_x.resize(count()+1);
    m_y.resize(count()+1);
    m_der.resize(0);

    int c = 0;
    for (Point *p = points->first(); p; p = points->next(p)) {
	m_x[c] = p->x;
	m_y[c] = p->y;
	c++;
    }
    m_x[c] = m_y[c] = 0.0;

    switch (m_type) {
	case INTPOL_NPOLYNOMIAL:
	    createFullPolynom(m_curve, m_x, m_y);
	    break;
	case INTPOL_SPLINE:
	    m_der.resize(count() + 1);
	    get2Derivate(m_x, m_y, m_der, count());
	    break;
	default:
	    ;
    }
    return true;
}

//***************************************************************************
QArray<double> Interpolation::limitedInterpolation(Curve *points,
	unsigned int len)
{
    QArray<double> y = interpolation(points, len);
    for (unsigned int i = 0; i < len; i++) {
	if (y[i] > 1) y[i] = 1;
	if (y[i] < 0) y[i] = 0;
    }
    return y;
}

//***************************************************************************
QArray<double> Interpolation::interpolation(Curve *points, unsigned int len)
{
    ASSERT(points);
    ASSERT(len);
    if (!points) return 0;
    if (!len) return 0;

    Point *tmp;
    unsigned int degree = 0;

    QArray<double> y_out(len);
    for (unsigned int i=0; i < len; i++) y_out[i] = 0.0;

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
		QArray<double> der(count + 1);
		QArray<double> x(count + 1);
		QArray<double> y(count + 1);

		for (tmp = points->first(); tmp; tmp = points->next(tmp)) {
		    x[t] = tmp->x;
		    y[t] = tmp->y;
		    t++;
		}

		get2Derivate(x, y, der, count);

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
		double ny;
		QArray<double> x(7);
		QArray<double> y(7);
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
		    QArray<double> x(count+1);
		    QArray<double> y(count+1);
		    double px;
		
		    createFullPolynom(points, x, y);
		
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
		if (tmp) {
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

    return y_out;
}

//***************************************************************************
void Interpolation::createFullPolynom(Curve *points,
	const QArray<double> &x, const QArray<double> &y)
{
    ASSERT(points);
    ASSERT(m_curve);
    if (!points) return;
    if (!m_curve) return;

    ASSERT(points->count() == m_curve->count());
    if (points->count() != m_curve->count()) return;

    unsigned int count = 0;
    Point *tmp;
    for (tmp = points->first(); (tmp); tmp = points->next(tmp)) {
	x[count] = tmp->x;
	y[count] = tmp->y;
	count++;
    }

    for (unsigned int k = 0; k < count; k++)
	for (unsigned int j = k; j; ) {
	    j--;
	    y[j] = (y[j] - y[j+1]) / (x[j] - x[k]);
	}
}

//***************************************************************************
void Interpolation::get2Derivate(const QArray<double> &x,
	const QArray<double> &y, QArray<double> &ab, unsigned int n)
{
    ASSERT(n);
    if (!n) return;

    unsigned int i, k;
    double p, qn, sig, un;

    QArray<double> u(n);

    ab[0] = ab[1] = 0;
    u[0] = u[1] = 0;

    for (i = 2; i < n; i++) {
	sig = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
	p = sig * ab[i-1] + 2;
	ab[i] = (sig-1) / p;
	u[i] = (y[i+1] - y[i])   / (x[i+1] - x[i])
	     - (y[i]   - y[i-1]) / (x[i]   - x[i-1]);
	u[i] = (6 * u[i] / (x[i+1] - x[i-1]) - sig * u[i-1]) / p;
    }

    qn = 0;
    un = 0;
    ab[n] = (un - qn * u[n - 1]) / (qn * ab[n - 1] + 1);

    for (k = n - 1; k > 0; k--)
	ab[k] = ab[k] * ab[k + 1] + u[k];

}

//***************************************************************************
void Interpolation::createPolynom(Curve *points, QArray<double> &x,
	QArray<double> &y, int pos, unsigned int degree)
{
    unsigned int count = 0;

    if (pos < 0) {
	switch (pos) {
	   case -3:
		x[count] = -1.5;
		y[count++] = points->first()->y;
		pos++;
	   case -2:
		x[count] = -1;
		y[count++] = points->first()->y;
		pos++;
	   case -1:
		x[count] = -.5;
		y[count++] = points->first()->y;
		pos++;
	}
    }

    Point *tmp = points->first();;
    for (int i = 0; i < pos; i++) tmp = points->next(tmp);

    for (; (count < degree) && (tmp); tmp = points->next(tmp)) {
	x[count] = tmp->x;
	y[count++] = tmp->y;
    }

    int i = 1;
    while (count < degree) {
	x[count] = 1.0 + 0.5 * (i++);
	y[count++] = points->last()->y;
    }

    // create coefficients in y[i] and x[i];
    for (unsigned int k = 0; k < degree; k++)
	for (int j = k - 1; j >= 0; j--)
	    y[j] = (y[j] - y[j + 1]) / (x[j] - x[k]);

}

//***************************************************************************
//***************************************************************************
