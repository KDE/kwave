//code taken from gnu gsl project
//this code is part of the gnu gsl library. The functions being useful
//for kwave were concatenated, since gsl library is not far spread
//so users won't have to search for this one.
//Later on, when both gsl and kwave leave alpha state, kwave will leave
//this file out and use the shared library (that contains many more functions)

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h> // for memcpy

#include <time.h>

#include "gsl_fft.h"


int gsl_fft_complex_bitreverse_order (complex data[],
				      const unsigned int n,
				      const unsigned int logn) {
    unsigned int i;

    for (i = 0; i < n; i++) {
	unsigned int j = 0;
	unsigned int i_tmp = i;
	unsigned int bit;

	for (bit = 0; bit < logn; bit++) {
	    j <<= 1;                /* reverse shift i into j */
	    j |= i_tmp & 1;
	    i_tmp >>= 1;
	}

	if (i < j) {
	    const complex data_tmp = data[i];
	    data[i] = data[j];
	    data[j] = data_tmp;
	}
    }
    return 0;
}

int
gsl_fft_complex_forward (complex data[],
			 const unsigned int n,
			 const gsl_fft_complex_wavetable * wavetable) {
    gsl_fft_direction sign = forward;
    int status = gsl_fft_complex (data, n, wavetable, sign);
    return status;
}

int
gsl_fft_complex_backward (complex data[],
			  const unsigned int n,
			  const gsl_fft_complex_wavetable * wavetable) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex (data, n, wavetable, sign);
    return status;
}

int
gsl_fft_complex_inverse (complex data[],
			 const unsigned int n,
			 const gsl_fft_complex_wavetable * wavetable) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex (data, n, wavetable, sign);

    if (status) {
	return status;
    }

    /* normalize inverse fft with 1/n */

    {
	const double norm = 1.0 / n;
	unsigned int i;
	for (i = 0; i < n; i++) {
	    data[i].real *= norm;
	    data[i].imag *= norm;
	}
    }
    return status;
}

int
gsl_fft_complex_init (unsigned int n,
		      gsl_fft_complex_wavetable * wavetable) {
    int status;
    unsigned int n_factors;

    wavetable->n = n;

    status = gsl_fft_complex_factorize (n, &n_factors, wavetable->factor);

    wavetable->nf = n_factors;

    status = gsl_fft_complex_generate_wavetable (n, wavetable);

    return 0;
}

int
gsl_fft_complex_generate_wavetable (unsigned int n,
				    gsl_fft_complex_wavetable * wavetable) {
    unsigned int i;
    double d_theta;
    unsigned int t, product, product_1, q;

    d_theta = -2.0 * M_PI / ((double) n);

    t = 0;
    product = 1;
    for (i = 0; i < wavetable->nf; i++) {
	unsigned int j;
	const unsigned int factor = wavetable->factor[i];
	wavetable->twiddle[i] = wavetable->trig + t;
	product_1 = product;        /* product_1 = p_(i-1) */
	product *= factor;
	q = n / product;

	for (j = 1; j < factor; j++) {
	    unsigned int k;
	    unsigned int m = 0;
	    for (k = 1; k <= q; k++) {
		double theta;
		m = m + j * product_1;
		m = m % n;
		theta = d_theta * m;        /*  d_theta*j*k*p_(i-1) */
		wavetable->trig[t].real = cos (theta);
		wavetable->trig[t].imag = sin (theta);

		t++;
	    }
	}
    }

    return 0;
}

int
gsl_fft_complex_wavetable_alloc (unsigned int n,
				 gsl_fft_complex_wavetable * wavetable) {

    wavetable->scratch = (complex *) malloc (n * sizeof (complex));

    wavetable->trig = (complex *)malloc (n * sizeof (complex));


    return 0;
}

int
gsl_fft_complex_wavetable_free (gsl_fft_complex_wavetable * wavetable) {

    /* release scratch space and trigonometric lookup tables */

    free (wavetable->scratch);
    wavetable->scratch = NULL;

    free (wavetable->trig);
    wavetable->trig = NULL;

    return 0;
}

int
gsl_fft_complex_pass_2 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle[]) {
    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 2;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    for (k = 0; k < q; k++) {
	double w_real, w_imag;

	if (k == 0) {
	    w_real = 1.0;
	    w_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w_real = twiddle[k - 1].real;
		w_imag = twiddle[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w_real = twiddle[k - 1].real;
		w_imag = -twiddle[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {
	    complex z0, z1;
	    double x0_real, x0_imag, x1_real, x1_imag; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		z0 = from[from0];
		z1 = from[from1];
	    }

	    /* compute x = W(2) z */

	    /* x0 = z0 + z1 */
	    x0_real = z0.real + z1.real;
	    x0_imag = z0.imag + z1.imag;

	    /* x1 = z0 - z1 */
	    x1_real = z0.real - z1.real;
	    x1_imag = z0.imag - z1.imag;

	    /* apply twiddle factors */
	    {
		const unsigned int to0 = j;
		const unsigned int to1 = product_1 + j;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w * x1 */
		to[to1].real = w_real * x1_real - w_imag * x1_imag;
		to[to1].imag = w_real * x1_imag + w_imag * x1_real;
	    }

	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}

int
gsl_fft_complex_pass_3 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex * twiddle1,
			const complex * twiddle2) {
    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 3;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    const double tau = sqrt (3.0) / 2.0;

    for (k = 0; k < q; k++) {
	double w1_real, w1_imag, w2_real, w2_imag;

	if (k == 0) {
	    w1_real = 1.0;
	    w1_imag = 0.0;
	    w2_real = 1.0;
	    w2_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w1_real = twiddle1[k - 1].real;
		w1_imag = twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = twiddle2[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w1_real = twiddle1[k - 1].real;
		w1_imag = -twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = -twiddle2[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {

	    complex z0, z1, z2;
	    double x0_real, x0_imag, x1_real, x1_imag, x2_real, x2_imag; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		const unsigned int from2 = from1 + m;

		z0 = from[from0];
		z1 = from[from1];
		z2 = from[from2];
	    }

	    /* compute x = W(3) z */
	    {
		/* t1 = z1 + z2 */
		const double t1_real = z1.real + z2.real;
		const double t1_imag = z1.imag + z2.imag;

		/* t2 = z0 - t1/2 */
		const double t2_real = z0.real - t1_real / 2.0;
		const double t2_imag = z0.imag - t1_imag / 2.0;

		/* t3 = (+/-) sin(pi/3)*(z1 - z2) */
		const double t3_real = ((int) sign) * tau * (z1.real - z2.real);
		const double t3_imag = ((int) sign) * tau * (z1.imag - z2.imag);

		/* x0 = z0 + t1 */
		x0_real = z0.real + t1_real;
		x0_imag = z0.imag + t1_imag;

		/* x1 = t2 + i t3 */
		x1_real = t2_real - t3_imag;
		x1_imag = t2_imag + t3_real;

		/* x2 = t2 - i t3 */
		x2_real = t2_real + t3_imag;
		x2_imag = t2_imag - t3_real;
	    }

	    /* apply twiddle factors */
	    {
		const unsigned int to0 = j;
		const unsigned int to1 = to0 + product_1;
		const unsigned int to2 = to1 + product_1;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w1 * x1 */
		to[to1].real = w1_real * x1_real - w1_imag * x1_imag;
		to[to1].imag = w1_real * x1_imag + w1_imag * x1_real;

		/* to2 = w2 * x2 */
		to[to2].real = w2_real * x2_real - w2_imag * x2_imag;
		to[to2].imag = w2_real * x2_imag + w2_imag * x2_real;
	    }

	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}

int
gsl_fft_complex_pass_4 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle1[],
			const complex twiddle2[],
			const complex twiddle3[]) {
    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 4;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    for (k = 0; k < q; k++) {
	double w1_real, w1_imag, w2_real, w2_imag, w3_real, w3_imag;

	if (k == 0) {
	    w1_real = 1.0;
	    w1_imag = 0.0;
	    w2_real = 1.0;
	    w2_imag = 0.0;
	    w3_real = 1.0;
	    w3_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w1_real = twiddle1[k - 1].real;
		w1_imag = twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = twiddle3[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w1_real = twiddle1[k - 1].real;
		w1_imag = -twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = -twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = -twiddle3[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {
	    complex z0, z1, z2, z3;
	    double x0_real, x0_imag, x1_real, x1_imag, x2_real, x2_imag,
	    x3_real, x3_imag; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		const unsigned int from2 = from1 + m;
		const unsigned int from3 = from2 + m;

		z0 = from[from0];
		z1 = from[from1];
		z2 = from[from2];
		z3 = from[from3];
	    }

	    /* compute x = W(4) z */
	    {
		/* t1 = z0 + z2 */
		const double t1_real = z0.real + z2.real;
		const double t1_imag = z0.imag + z2.imag;

		/* t2 = z1 + z3 */
		const double t2_real = z1.real + z3.real;
		const double t2_imag = z1.imag + z3.imag;

		/* t3 = z0 - z2 */
		const double t3_real = z0.real - z2.real;
		const double t3_imag = z0.imag - z2.imag;

		/* t4 = (+/-) (z1 - z3) */
		const double t4_real = ((int) sign) * (z1.real - z3.real);
		const double t4_imag = ((int) sign) * (z1.imag - z3.imag);

		/* x0 = t1 + t2 */
		x0_real = t1_real + t2_real;
		x0_imag = t1_imag + t2_imag;

		/* x1 = t3 + i t4 */
		x1_real = t3_real - t4_imag;
		x1_imag = t3_imag + t4_real;

		/* x2 = t1 - t2 */
		x2_real = t1_real - t2_real;
		x2_imag = t1_imag - t2_imag;

		/* x3 = t3 - i t4 */
		x3_real = t3_real + t4_imag;
		x3_imag = t3_imag - t4_real;
	    }

	    /* apply twiddle factors */
	    {
		const unsigned int to0 = j;
		const unsigned int to1 = product_1 + to0;
		const unsigned int to2 = product_1 + to1;
		const unsigned int to3 = product_1 + to2;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w1 * x1 */
		to[to1].real = w1_real * x1_real - w1_imag * x1_imag;
		to[to1].imag = w1_real * x1_imag + w1_imag * x1_real;

		/* to2 = w2 * x2 */
		to[to2].real = w2_real * x2_real - w2_imag * x2_imag;
		to[to2].imag = w2_real * x2_imag + w2_imag * x2_real;

		/* to3 = w3 * x3 */
		to[to3].real = w3_real * x3_real - w3_imag * x3_imag;
		to[to3].imag = w3_real * x3_imag + w3_imag * x3_real;
	    }

	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}


int
gsl_fft_complex_pass_5 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle1[],
			const complex twiddle2[],
			const complex twiddle3[],
			const complex twiddle4[]) {
    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 5;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    const double sin_2pi_by_5 = sin (2.0 * M_PI / 5.0);
    const double sin_2pi_by_10 = sin (2.0 * M_PI / 10.0);

    for (k = 0; k < q; k++) {

	double w1_real, w1_imag, w2_real, w2_imag, w3_real, w3_imag, w4_real,
	w4_imag;

	if (k == 0) {
	    w1_real = 1.0;
	    w1_imag = 0.0;
	    w2_real = 1.0;
	    w2_imag = 0.0;
	    w3_real = 1.0;
	    w3_imag = 0.0;
	    w4_real = 1.0;
	    w4_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w1_real = twiddle1[k - 1].real;
		w1_imag = twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = twiddle4[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w1_real = twiddle1[k - 1].real;
		w1_imag = -twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = -twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = -twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = -twiddle4[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {

	    complex z0, z1, z2, z3, z4;
	    double x0_real, x0_imag, x1_real, x1_imag, x2_real, x2_imag,
	    x3_real, x3_imag, x4_real, x4_imag; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		const unsigned int from2 = from1 + m;
		const unsigned int from3 = from2 + m;
		const unsigned int from4 = from3 + m;

		z0 = from[from0];
		z1 = from[from1];
		z2 = from[from2];
		z3 = from[from3];
		z4 = from[from4];
	    }

	    /* compute x = W(5) z */
	    {
		/* t1 = z1 + z4 */
		const double t1_real = z1.real + z4.real;
		const double t1_imag = z1.imag + z4.imag;

		/* t2 = z2 + z3 */
		const double t2_real = z2.real + z3.real;
		const double t2_imag = z2.imag + z3.imag;

		/* t3 = z1 - z4 */
		const double t3_real = z1.real - z4.real;
		const double t3_imag = z1.imag - z4.imag;

		/* t4 = z2 - z3 */
		const double t4_real = z2.real - z3.real;
		const double t4_imag = z2.imag - z3.imag;

		/* t5 = t1 + t2 */
		const double t5_real = t1_real + t2_real;
		const double t5_imag = t1_imag + t2_imag;

		/* t6 = (sqrt(5)/4)(t1 - t2) */
		const double t6_real = (sqrt (5.0) / 4.0) * (t1_real - t2_real);
		const double t6_imag = (sqrt (5.0) / 4.0) * (t1_imag - t2_imag);

		/* t7 = z0 - ((t5)/4) */
		const double t7_real = z0.real - t5_real / 4.0;
		const double t7_imag = z0.imag - t5_imag / 4.0;

		/* t8 = t7 + t6 */
		const double t8_real = t7_real + t6_real;
		const double t8_imag = t7_imag + t6_imag;

		/* t9 = t7 - t6 */
		const double t9_real = t7_real - t6_real;
		const double t9_imag = t7_imag - t6_imag;

		/* t10 = sin(2 pi/5) t3 + sin(2 pi/10) t4 */
		const double t10_real = ((int) sign) * (sin_2pi_by_5 * t3_real +
							sin_2pi_by_10 * t4_real);
		const double t10_imag = ((int) sign) * (sin_2pi_by_5 * t3_imag +
							sin_2pi_by_10 * t4_imag);

		/* t11 = sin(2 pi/10) t3 - sin(2 pi/5) t4 */
		const double t11_real = ((int) sign) * (sin_2pi_by_10 * t3_real -
							sin_2pi_by_5 * t4_real);
		const double t11_imag = ((int) sign) * (sin_2pi_by_10 * t3_imag -
							sin_2pi_by_5 * t4_imag);

		/* x0 = z0 + t5 */
		x0_real = z0.real + t5_real;
		x0_imag = z0.imag + t5_imag;

		/* x1 = t8 + i t10 */
		x1_real = t8_real - t10_imag;
		x1_imag = t8_imag + t10_real;

		/* x2 = t9 + i t11 */
		x2_real = t9_real - t11_imag;
		x2_imag = t9_imag + t11_real;

		/* x3 = t9 - i t11 */
		x3_real = t9_real + t11_imag;
		x3_imag = t9_imag - t11_real;

		/* x4 = t8 - i t10 */
		x4_real = t8_real + t10_imag;
		x4_imag = t8_imag - t10_real;
	    }

	    /* apply twiddle factors */
	    {
		const unsigned int to0 = j;
		const unsigned int to1 = to0 + product_1;
		const unsigned int to2 = to1 + product_1;
		const unsigned int to3 = to2 + product_1;
		const unsigned int to4 = to3 + product_1;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w1 * x1 */
		to[to1].real = w1_real * x1_real - w1_imag * x1_imag;
		to[to1].imag = w1_real * x1_imag + w1_imag * x1_real;

		/* to2 = w2 * x2 */
		to[to2].real = w2_real * x2_real - w2_imag * x2_imag;
		to[to2].imag = w2_real * x2_imag + w2_imag * x2_real;

		/* to3 = w3 * x3 */
		to[to3].real = w3_real * x3_real - w3_imag * x3_imag;
		to[to3].imag = w3_real * x3_imag + w3_imag * x3_real;

		/* to4 = w4 * x4 */
		to[to4].real = w4_real * x4_real - w4_imag * x4_imag;
		to[to4].imag = w4_real * x4_imag + w4_imag * x4_real;
	    }

	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}

int
gsl_fft_complex_pass_6 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle1[],
			const complex twiddle2[],
			const complex twiddle3[],
			const complex twiddle4[],
			const complex twiddle5[]) {

    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 6;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    const double tau = sqrt (3.0) / 2.0;

    for (k = 0; k < q; k++) {
	double w1_real, w1_imag, w2_real, w2_imag, w3_real, w3_imag, w4_real,
	w4_imag, w5_real, w5_imag;

	if (k == 0) {
	    w1_real = 1.0;
	    w1_imag = 0.0;
	    w2_real = 1.0;
	    w2_imag = 0.0;
	    w3_real = 1.0;
	    w3_imag = 0.0;
	    w4_real = 1.0;
	    w4_imag = 0.0;
	    w5_real = 1.0;
	    w5_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w1_real = twiddle1[k - 1].real;
		w1_imag = twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = twiddle4[k - 1].imag;
		w5_real = twiddle5[k - 1].real;
		w5_imag = twiddle5[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w1_real = twiddle1[k - 1].real;
		w1_imag = -twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = -twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = -twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = -twiddle4[k - 1].imag;
		w5_real = twiddle5[k - 1].real;
		w5_imag = -twiddle5[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {
	    complex z0, z1, z2, z3, z4, z5;
	    double x0_real, x0_imag, x1_real, x1_imag, x2_real, x2_imag,
	    x3_real, x3_imag, x4_real, x4_imag, x5_real, x5_imag; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		const unsigned int from2 = from1 + m;
		const unsigned int from3 = from2 + m;
		const unsigned int from4 = from3 + m;
		const unsigned int from5 = from4 + m;

		z0 = from[from0];
		z1 = from[from1];
		z2 = from[from2];
		z3 = from[from3];
		z4 = from[from4];
		z5 = from[from5];
	    }

	    /* compute x = W(6) z */

	    /* W(6) is a combination of sums and differences of W(3) acting
	       on the even and odd elements of z */
	    {
		/* ta1 = z2 + z4 */
		const double ta1_real = z2.real + z4.real;
		const double ta1_imag = z2.imag + z4.imag;

		/* ta2 = z0 - ta1/2 */
		const double ta2_real = z0.real - ta1_real / 2;
		const double ta2_imag = z0.imag - ta1_imag / 2;

		/* ta3 = (+/-) sin(pi/3)*(z2 - z4) */
		const double ta3_real = ((int) sign) * tau * (z2.real - z4.real);
		const double ta3_imag = ((int) sign) * tau * (z2.imag - z4.imag);

		/* a0 = z0 + ta1 */
		const double a0_real = z0.real + ta1_real;
		const double a0_imag = z0.imag + ta1_imag;

		/* a1 = ta2 + i ta3 */
		const double a1_real = ta2_real - ta3_imag;
		const double a1_imag = ta2_imag + ta3_real;

		/* a2 = ta2 - i ta3 */
		const double a2_real = ta2_real + ta3_imag;
		const double a2_imag = ta2_imag - ta3_real;

		/* tb1 = z5 + z1 */
		const double tb1_real = z5.real + z1.real;
		const double tb1_imag = z5.imag + z1.imag;

		/* tb2 = z3 - tb1/2 */
		const double tb2_real = z3.real - tb1_real / 2;
		const double tb2_imag = z3.imag - tb1_imag / 2;

		/* tb3 = (+/-) sin(pi/3)*(z5 - z1) */
		const double tb3_real = ((int) sign) * tau * (z5.real - z1.real);
		const double tb3_imag = ((int) sign) * tau * (z5.imag - z1.imag);

		/* b0 = z3 + tb1 */
		const double b0_real = z3.real + tb1_real;
		const double b0_imag = z3.imag + tb1_imag;

		/* b1 = tb2 + i tb3 */
		const double b1_real = tb2_real - tb3_imag;
		const double b1_imag = tb2_imag + tb3_real;

		/* b2 = tb2 - i tb3 */
		const double b2_real = tb2_real + tb3_imag;
		const double b2_imag = tb2_imag - tb3_real;

		/* x0 = a0 + b0 */
		x0_real = a0_real + b0_real;
		x0_imag = a0_imag + b0_imag;

		/* x4 = a1 + b1 */
		x4_real = a1_real + b1_real;
		x4_imag = a1_imag + b1_imag;

		/* x2 = a2 + b2 */
		x2_real = a2_real + b2_real;
		x2_imag = a2_imag + b2_imag;

		/* x3 = a0 - b0 */
		x3_real = a0_real - b0_real;
		x3_imag = a0_imag - b0_imag;

		/* x1 = a1 - b1 */
		x1_real = a1_real - b1_real;
		x1_imag = a1_imag - b1_imag;

		/* x5 = a2 - b2 */
		x5_real = a2_real - b2_real;
		x5_imag = a2_imag - b2_imag;
	    }

	    /* apply twiddle factors */
	    {
		const unsigned int to0 = j;
		const unsigned int to1 = to0 + product_1;
		const unsigned int to2 = to1 + product_1;
		const unsigned int to3 = to2 + product_1;
		const unsigned int to4 = to3 + product_1;
		const unsigned int to5 = to4 + product_1;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w1 * x1 */
		to[to1].real = w1_real * x1_real - w1_imag * x1_imag;
		to[to1].imag = w1_real * x1_imag + w1_imag * x1_real;

		/* to2 = w2 * x2 */
		to[to2].real = w2_real * x2_real - w2_imag * x2_imag;
		to[to2].imag = w2_real * x2_imag + w2_imag * x2_real;

		/* to3 = w3 * x3 */
		to[to3].real = w3_real * x3_real - w3_imag * x3_imag;
		to[to3].imag = w3_real * x3_imag + w3_imag * x3_real;

		/* to4 = w4 * x4 */
		to[to4].real = w4_real * x4_real - w4_imag * x4_imag;
		to[to4].imag = w4_real * x4_imag + w4_imag * x4_real;

		/* to5 = w5 * x5 */
		to[to5].real = w5_real * x5_real - w5_imag * x5_imag;
		to[to5].imag = w5_real * x5_imag + w5_imag * x5_real;
	    }
	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}

int
gsl_fft_complex_pass_7 (const complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle1[],
			const complex twiddle2[],
			const complex twiddle3[],
			const complex twiddle4[],
			const complex twiddle5[],
			const complex twiddle6[]) {

    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int factor = 7;
    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    const double c1 = cos(1.0 * 2.0 * M_PI / 7.0) ;
    const double c2 = cos(2.0 * 2.0 * M_PI / 7.0) ;
    const double c3 = cos(3.0 * 2.0 * M_PI / 7.0) ;

    const double s1 = sin(1.0 * 2.0 * M_PI / 7.0) ;
    const double s2 = sin(2.0 * 2.0 * M_PI / 7.0) ;
    const double s3 = sin(3.0 * 2.0 * M_PI / 7.0) ;

    for (k = 0; k < q; k++) {
	double w1_real, w1_imag, w2_real, w2_imag, w3_real, w3_imag, w4_real,
	w4_imag, w5_real, w5_imag, w6_real, w6_imag;

	if (k == 0) {
	    w1_real = 1.0;
	    w1_imag = 0.0;
	    w2_real = 1.0;
	    w2_imag = 0.0;
	    w3_real = 1.0;
	    w3_imag = 0.0;
	    w4_real = 1.0;
	    w4_imag = 0.0;
	    w5_real = 1.0;
	    w5_imag = 0.0;
	    w6_real = 1.0;
	    w6_imag = 0.0;
	} else {
	    if (sign == forward) {
		/* forward tranform */
		w1_real = twiddle1[k - 1].real;
		w1_imag = twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = twiddle4[k - 1].imag;
		w5_real = twiddle5[k - 1].real;
		w5_imag = twiddle5[k - 1].imag;
		w6_real = twiddle6[k - 1].real;
		w6_imag = twiddle6[k - 1].imag;
	    } else {
		/* backward tranform: w -> conjugate(w) */
		w1_real = twiddle1[k - 1].real;
		w1_imag = -twiddle1[k - 1].imag;
		w2_real = twiddle2[k - 1].real;
		w2_imag = -twiddle2[k - 1].imag;
		w3_real = twiddle3[k - 1].real;
		w3_imag = -twiddle3[k - 1].imag;
		w4_real = twiddle4[k - 1].real;
		w4_imag = -twiddle4[k - 1].imag;
		w5_real = twiddle5[k - 1].real;
		w5_imag = -twiddle5[k - 1].imag;
		w6_real = twiddle6[k - 1].real;
		w6_imag = -twiddle6[k - 1].imag;
	    }
	}

	for (k1 = 0; k1 < product_1; k1++) {
	    complex z0, z1, z2, z3, z4, z5, z6; {
		const unsigned int from0 = i;
		const unsigned int from1 = from0 + m;
		const unsigned int from2 = from1 + m;
		const unsigned int from3 = from2 + m;
		const unsigned int from4 = from3 + m;
		const unsigned int from5 = from4 + m;
		const unsigned int from6 = from5 + m;

		z0 = from[from0];
		z1 = from[from1];
		z2 = from[from2];
		z3 = from[from3];
		z4 = from[from4];
		z5 = from[from5];
		z6 = from[from6];
	    }

	    /* compute x = W(7) z */

	    {
		/* t0 = z1 + z6 */
		const double t0_real = z1.real + z6.real ;
		const double t0_imag = z1.imag + z6.imag ;

		/* t1 = z1 - z6 */
		const double t1_real = z1.real - z6.real ;
		const double t1_imag = z1.imag - z6.imag ;

		/* t2 = z2 + z5 */
		const double t2_real = z2.real + z5.real ;
		const double t2_imag = z2.imag + z5.imag ;

		/* t3 = z2 - z5 */
		const double t3_real = z2.real - z5.real ;
		const double t3_imag = z2.imag - z5.imag ;

		/* t4 = z4 + z3 */
		const double t4_real = z4.real + z3.real ;
		const double t4_imag = z4.imag + z3.imag ;

		/* t5 = z4 - z3 */
		const double t5_real = z4.real - z3.real ;
		const double t5_imag = z4.imag - z3.imag ;

		/* t6 = t2 + t0 */
		const double t6_real = t2_real + t0_real ;
		const double t6_imag = t2_imag + t0_imag ;

		/* t7 = t5 + t3 */
		const double t7_real = t5_real + t3_real ;
		const double t7_imag = t5_imag + t3_imag ;

		/* b0 = z0 + t6 + t4 */
		const double b0_real = z0.real + t6_real + t4_real ;
		const double b0_imag = z0.imag + t6_imag + t4_imag ;

		/* b1 = ((cos(2pi/7) + cos(4pi/7) + cos(6pi/7))/3-1) (t6 + t4) */
		const double b1_real = (((c1 + c2 + c3) / 3.0 - 1.0) * (t6_real + t4_real));
		const double b1_imag = (((c1 + c2 + c3) / 3.0 - 1.0) * (t6_imag + t4_imag));

		/* b2 = ((2*cos(2pi/7) - cos(4pi/7) - cos(6pi/7))/3) (t0 - t4) */
		const double b2_real = (((2.0 * c1 - c2 - c3) / 3.0) * (t0_real - t4_real));
		const double b2_imag = (((2.0 * c1 - c2 - c3) / 3.0) * (t0_imag - t4_imag));

		/* b3 = ((cos(2pi/7) - 2*cos(4pi/7) + cos(6pi/7))/3) (t4 - t2) */
		const double b3_real = (((c1 - 2.0 * c2 + c3) / 3.0) * (t4_real - t2_real));
		const double b3_imag = (((c1 - 2.0 * c2 + c3) / 3.0) * (t4_imag - t2_imag));

		/* b4 = ((cos(2pi/7) + cos(4pi/7) - 2*cos(6pi/7))/3) (t2 - t0) */
		const double b4_real = (((c1 + c2 - 2.0 * c3) / 3.0) * (t2_real - t0_real));
		const double b4_imag = (((c1 + c2 - 2.0 * c3) / 3.0) * (t2_imag - t0_imag));

		/* b5 = sign * ((sin(2pi/7) + sin(4pi/7) - sin(6pi/7))/3) (t7 + t1) */
		const double b5_real = ( -(int)sign) * ((s1 + s2 - s3) / 3.0) * (t7_real + t1_real) ;
		const double b5_imag = ( -(int)sign) * ((s1 + s2 - s3) / 3.0) * (t7_imag + t1_imag) ;

		/* b6 = sign * ((2sin(2pi/7) - sin(4pi/7) + sin(6pi/7))/3) (t1 - t5) */
		const double b6_real = ( -(int)sign) * ((2.0 * s1 - s2 + s3) / 3.0) * (t1_real - t5_real) ;
		const double b6_imag = ( -(int)sign) * ((2.0 * s1 - s2 + s3) / 3.0) * (t1_imag - t5_imag) ;

		/* b7 = sign * ((sin(2pi/7) - 2sin(4pi/7) - sin(6pi/7))/3) (t5 - t3) */
		const double b7_real = ( -(int)sign) * ((s1 - 2.0 * s2 - s3) / 3.0) * (t5_real - t3_real) ;
		const double b7_imag = ( -(int)sign) * ((s1 - 2.0 * s2 - s3) / 3.0) * (t5_imag - t3_imag) ;

		/* b8 = sign * ((sin(2pi/7) + sin(4pi/7) + 2sin(6pi/7))/3) (t3 - t1) */
		const double b8_real = ( -(int)sign) * ((s1 + s2 + 2.0 * s3) / 3.0) * (t3_real - t1_real) ;
		const double b8_imag = ( -(int)sign) * ((s1 + s2 + 2.0 * s3) / 3.0) * (t3_imag - t1_imag) ;


		/* T0 = b0 + b1 */
		const double T0_real = b0_real + b1_real ;
		const double T0_imag = b0_imag + b1_imag ;

		/* T1 = b2 + b3 */
		const double T1_real = b2_real + b3_real ;
		const double T1_imag = b2_imag + b3_imag ;

		/* T2 = b4 - b3 */
		const double T2_real = b4_real - b3_real ;
		const double T2_imag = b4_imag - b3_imag ;

		/* T3 = -b2 - b4 */
		const double T3_real = -b2_real - b4_real ;
		const double T3_imag = -b2_imag - b4_imag ;

		/* T4 = b6 + b7 */
		const double T4_real = b6_real + b7_real ;
		const double T4_imag = b6_imag + b7_imag ;

		/* T5 = b8 - b7 */
		const double T5_real = b8_real - b7_real ;
		const double T5_imag = b8_imag - b7_imag ;

		/* T6 = -b8 - b6 */
		const double T6_real = -b8_real - b6_real ;
		const double T6_imag = -b8_imag - b6_imag ;

		/* T7 = T0 + T1 */
		const double T7_real = T0_real + T1_real ;
		const double T7_imag = T0_imag + T1_imag ;

		/* T8 = T0 + T2 */
		const double T8_real = T0_real + T2_real ;
		const double T8_imag = T0_imag + T2_imag ;

		/* T9 = T0 + T3 */
		const double T9_real = T0_real + T3_real ;
		const double T9_imag = T0_imag + T3_imag ;

		/* T10 = T4 + b5 */
		const double T10_real = T4_real + b5_real ;
		const double T10_imag = T4_imag + b5_imag ;

		/* T11 = T5 + b5 */
		const double T11_real = T5_real + b5_real ;
		const double T11_imag = T5_imag + b5_imag ;

		/* T12 = T6 + b5 */
		const double T12_real = T6_real + b5_real ;
		const double T12_imag = T6_imag + b5_imag ;


		/* x0 = b0 */
		const double x0_real = b0_real ;
		const double x0_imag = b0_imag ;

		/* x1 = T7 - i T10 */
		const double x1_real = T7_real + T10_imag ;
		const double x1_imag = T7_imag - T10_real ;

		/* x2 = T9 - i T12 */
		const double x2_real = T9_real + T12_imag ;
		const double x2_imag = T9_imag - T12_real ;

		/* x3 = T8 + i T11 */
		const double x3_real = T8_real - T11_imag ;
		const double x3_imag = T8_imag + T11_real ;

		/* x4 = T8 - i T11 */
		const double x4_real = T8_real + T11_imag ;
		const double x4_imag = T8_imag - T11_real ;

		/* x5 = T9 + i T12 */
		const double x5_real = T9_real - T12_imag ;
		const double x5_imag = T9_imag + T12_real ;

		/* x6 = T7 + i T10 */
		const double x6_real = T7_real - T10_imag ;
		const double x6_imag = T7_imag + T10_real ;

		/* apply twiddle factors */

		const unsigned int to0 = j;
		const unsigned int to1 = to0 + product_1;
		const unsigned int to2 = to1 + product_1;
		const unsigned int to3 = to2 + product_1;
		const unsigned int to4 = to3 + product_1;
		const unsigned int to5 = to4 + product_1;
		const unsigned int to6 = to5 + product_1;

		/* to0 = 1 * x0 */
		to[to0].real = x0_real;
		to[to0].imag = x0_imag;

		/* to1 = w1 * x1 */
		to[to1].real = w1_real * x1_real - w1_imag * x1_imag;
		to[to1].imag = w1_real * x1_imag + w1_imag * x1_real;

		/* to2 = w2 * x2 */
		to[to2].real = w2_real * x2_real - w2_imag * x2_imag;
		to[to2].imag = w2_real * x2_imag + w2_imag * x2_real;

		/* to3 = w3 * x3 */
		to[to3].real = w3_real * x3_real - w3_imag * x3_imag;
		to[to3].imag = w3_real * x3_imag + w3_imag * x3_real;

		/* to4 = w4 * x4 */
		to[to4].real = w4_real * x4_real - w4_imag * x4_imag;
		to[to4].imag = w4_real * x4_imag + w4_imag * x4_real;

		/* to5 = w5 * x5 */
		to[to5].real = w5_real * x5_real - w5_imag * x5_imag;
		to[to5].imag = w5_real * x5_imag + w5_imag * x5_real;

		/* to6 = w6 * x6 */
		to[to6].real = w6_real * x6_real - w6_imag * x6_imag;
		to[to6].imag = w6_real * x6_imag + w6_imag * x6_real;

	    }
	    i++;
	    j++;
	}
	j += jump;
    }
    return 0;
}

int
gsl_fft_complex_pass_n (complex from[],
			complex to[],
			const gsl_fft_direction sign,
			const unsigned int factor,
			const unsigned int product,
			const unsigned int n,
			const complex twiddle[]) {
    unsigned int i = 0, j = 0;
    unsigned int k, k1;

    const unsigned int m = n / factor;
    const unsigned int q = n / product;
    const unsigned int product_1 = product / factor;
    const unsigned int jump = (factor - 1) * product_1;

    unsigned int e, e1;

    for (i = 0; i < m; i++) {
	to[i] = from[i];
    }

    for (e = 1; e < (factor - 1) / 2 + 1; e++) {
	for (i = 0; i < m; i++) {
	    const unsigned int idx = i + e * m;
	    const unsigned int idxc = i + (factor - e) * m;
	    to[idx].real = from[idx].real + from[idxc].real;
	    to[idx].imag = from[idx].imag + from[idxc].imag;
	    to[idxc].real = from[idx].real - from[idxc].real;
	    to[idxc].imag = from[idx].imag - from[idxc].imag;
	}
    }

    /* e = 0 */

    for (i = 0 ; i < m; i++) {
	from[i] = to[i] ;
    }

    for (e1 = 1; e1 < (factor - 1) / 2 + 1; e1++) {
	for (i = 0; i < m; i++) {
	    from[i].real += to[i + e1 * m].real ;
	    from[i].imag += to[i + e1 * m].imag ;
	}
    }

    for (e = 1; e < (factor - 1) / 2 + 1; e++) {
	unsigned int idx = e * q ;
	const unsigned int idx_step = e * q ;
	double w_real, w_imag ;

	const unsigned int em = e * m ;
	const unsigned int ecm = (factor - e) * m ;

	for (i = 0; i < m; i++) {
	    from[i + em] = to[i];
	    from[i + ecm] = to[i];
	}

	for (e1 = 1; e1 < (factor - 1) / 2 + 1; e1++) {
	    if (idx == 0) {
		w_real = 1 ;
		w_imag = 0 ;
	    } else {
		if (sign == forward) {
		    w_real = twiddle[idx - 1].real ;
		    w_imag = twiddle[idx - 1].imag ;
		} else {
		    w_real = twiddle[idx - 1].real ;
		    w_imag = -twiddle[idx - 1].imag ;
		}
	    }

	    for (i = 0; i < m; i++) {
		complex xp = to[i + e1 * m];
		complex xm = to[i + (factor - e1) * m];

		const double ap = w_real * xp.real ;
		const double am = w_imag * xm.imag ;

		double sum_real = ap - am;
		double sumc_real = ap + am;

		const double bp = w_real * xp.imag ;
		const double bm = w_imag * xm.real ;

		double sum_imag = bp + bm;
		double sumc_imag = bp - bm;

		from[i + em].real += sum_real;
		from[i + em].imag += sum_imag;
		from[i + ecm].real += sumc_real;
		from[i + ecm].imag += sumc_imag;
	    }
	    idx += idx_step ;
	    idx %= factor * q ;
	}
    }

    i = 0;
    j = 0;

    /* k = 0 */
    for (k1 = 0; k1 < product_1; k1++) {
	to[k1] = from[k1];
    }

    for (e1 = 1; e1 < factor; e1++) {
	for (k1 = 0; k1 < product_1; k1++) {
	    to[k1 + e1 * product_1] = from[k1 + e1 * m] ;
	}
    }

    i = product_1 ;
    j = product ;

    for (k = 1; k < q; k++) {
	for (k1 = 0; k1 < product_1; k1++) {

	    to[j].real = from[i].real;
	    to[j].imag = from[i].imag;

	    i++;
	    j++;
	}
	j += jump;
    }

    i = product_1 ;
    j = product ;

    for (k = 1; k < q; k++) {
	for (k1 = 0; k1 < product_1; k1++) {
	    for (e1 = 1; e1 < factor; e1++) {
		double x_real = from[i + e1 * m].real;
		double x_imag = from[i + e1 * m].imag;

		double w_real, w_imag ;
		if (sign == forward) {
		    w_real = twiddle[(e1 - 1) * q + k - 1].real ;
		    w_imag = twiddle[(e1 - 1) * q + k - 1].imag ;
		} else {
		    w_real = twiddle[(e1 - 1) * q + k - 1].real ;
		    w_imag = -twiddle[(e1 - 1) * q + k - 1].imag ;
		}

		to[j + e1 * product_1].real = w_real * x_real - w_imag * x_imag;
		to[j + e1 * product_1].imag = w_real * x_imag + w_imag * x_real;
	    }
	    i++;
	    j++;
	}
	j += jump;
    }

    return 0;
}


int
gsl_fft_complex_radix2_forward (complex data[],
				const unsigned int n) {
    gsl_fft_direction sign = forward;
    int status = gsl_fft_complex_radix2 (data, n, sign);
    return status;
}

int
gsl_fft_complex_radix2_backward (complex data[],
				 const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex_radix2 (data, n, sign);
    return status;
}

int
gsl_fft_complex_radix2_inverse (complex data[],
				const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex_radix2 (data, n, sign);

    if (status) {
	return status;
    }

    /* normalize inverse fft with 1/n */

    {
	const double norm = 1.0 / n;
	unsigned int i;
	for (i = 0; i < n; i++) {
	    data[i].real *= norm;
	    data[i].imag *= norm;
	}
    }
    return status;
}


int
gsl_fft_complex_radix2_dif_forward (complex data[],
				    const unsigned int n) {
    gsl_fft_direction sign = forward;
    int status = gsl_fft_complex_radix2_dif (data, n, sign);
    return status;
}

int
gsl_fft_complex_radix2_dif_backward (complex data[],
				     const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex_radix2_dif (data, n, sign);
    return status;
}

int
gsl_fft_complex_radix2_dif_inverse (complex data[],
				    const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_fft_complex_radix2_dif (data, n, sign);

    if (status) {
	return status;
    }

    /* normalize inverse fft with 1/n */

    {
	const double norm = 1.0 / n;
	unsigned int i;
	for (i = 0; i < n; i++) {
	    data[i].real *= norm;
	    data[i].imag *= norm;
	}
    }
    return status;
}


int
gsl_fft_complex_radix2 (complex data[],
			const unsigned int n,
			const gsl_fft_direction sign) {

    int result ;
    unsigned int dual;
    unsigned int bit;
    unsigned int logn = 0;
    int status;

    if (n == 1) /* identity operation */
    {
	return 0 ;
    }

    /* make sure that n is a power of 2 */

    result = gsl_fft_binary_logn(n) ;

    if (result == -1) {}
    else {
	logn = result ;
    }

    /* bit reverse the ordering of input data for decimation in time algorithm */

    status = gsl_fft_complex_bitreverse_order(data, n, logn) ;

    /* apply fft recursion */

    dual = 1;

    for (bit = 0; bit < logn; bit++) {
	double w_real = 1.0;
	double w_imag = 0.0;

	const double theta = 2.0 * ((int) sign) * M_PI / (2.0 * (double) dual);

	const double s = sin (theta);
	const double t = sin (theta / 2.0);
	const double s2 = 2.0 * t * t;

	unsigned int a, b;

	/* a = 0 */

	for (b = 0; b < n; b += 2 * dual) {
	    const unsigned int i = b ;
	    const unsigned int j = b + dual;

	    const double z1_real = data[j].real ;
	    const double z1_imag = data[j].imag ;

	    const double wd_real = z1_real ;
	    const double wd_imag = z1_imag ;

	    data[j].real = data[i].real - wd_real;
	    data[j].imag = data[i].imag - wd_imag;
	    data[i].real += wd_real;
	    data[i].imag += wd_imag;
	}

	/* a = 1 .. (dual-1) */

	for (a = 1; a < dual; a++) {

	    /* trignometric recurrence for w-> exp(i theta) w */

	    {
		const double tmp_real = w_real - s * w_imag - s2 * w_real;
		const double tmp_imag = w_imag + s * w_real - s2 * w_imag;
		w_real = tmp_real;
		w_imag = tmp_imag;
	    }

	    for (b = 0; b < n; b += 2 * dual) {
		const unsigned int i = b + a;
		const unsigned int j = b + a + dual;

		const double z1_real = data[j].real ;
		const double z1_imag = data[j].imag ;

		const double wd_real = w_real * z1_real - w_imag * z1_imag;
		const double wd_imag = w_real * z1_imag + w_imag * z1_real;

		data[j].real = data[i].real - wd_real;
		data[j].imag = data[i].imag - wd_imag;
		data[i].real += wd_real;
		data[i].imag += wd_imag;
	    }
	}
	dual *= 2;
    }

    return 0;

}



int
gsl_fft_complex_radix2_dif (complex data[],
			    const unsigned int n,
			    const gsl_fft_direction sign) {
    int result ;
    unsigned int dual;
    unsigned int bit;
    unsigned int logn = 0;
    int status;

    if (n == 1) /* identity operation */
    {
	return 0 ;
    }

    /* make sure that n is a power of 2 */

    result = gsl_fft_binary_logn(n) ;

    if (result == -1) {}
    else {
	logn = result ;
    }

    /* apply fft recursion */

    dual = n / 2;

    for (bit = 0; bit < logn; bit++) {
	double w_real = 1.0;
	double w_imag = 0.0;

	const double theta = 2.0 * ((int) sign) * M_PI / ((double) (2 * dual));

	const double s = sin (theta);
	const double t = sin (theta / 2.0);
	const double s2 = 2.0 * t * t;

	unsigned int a, b;

	for (b = 0; b < dual; b++) {
	    for (a = 0; a < n; a += 2 * dual) {
		const unsigned int i = b + a;
		const unsigned int j = b + a + dual;

		const double t1_real = data[i].real + data[j].real;
		const double t1_imag = data[i].imag + data[j].imag;
		const double t2_real = data[i].real - data[j].real;
		const double t2_imag = data[i].imag - data[j].imag;

		data[i].real = t1_real;
		data[i].imag = t1_imag;
		data[j].real = w_real * t2_real - w_imag * t2_imag;
		data[j].imag = w_real * t2_imag + w_imag * t2_real;
	    }

	    /* trignometric recurrence for w-> exp(i theta) w */

	    {
		const double tmp_real = w_real - s * w_imag - s2 * w_real;
		const double tmp_imag = w_imag + s * w_real - s2 * w_imag;
		w_real = tmp_real;
		w_imag = tmp_imag;
	    }
	}
	dual /= 2;
    }

    /* bit reverse the ordering of output data for decimation in
       frequency algorithm */

    status = gsl_fft_complex_bitreverse_order(data, n, logn) ;

    return 0;

}

int
gsl_dft_complex (const complex data[],
		 complex result[],
		 const unsigned int n,
		 const gsl_fft_direction sign) {

    unsigned int i, j, exponent;
    const double d_theta = 2.0 * ((int) sign) * M_PI / (double) n;

    for (i = 0; i < n; i++) {
	double sum_real = 0;
	double sum_imag = 0;

	exponent = 0;

	for (j = 0; j < n; j++) {
	    double theta = d_theta * (double) exponent;
	    /* sum = exp(i theta) * data[j] */

	    double w_real = cos (theta);
	    double w_imag = sin (theta);

	    double data_real = data[j].real;
	    double data_imag = data[j].imag;

	    sum_real += w_real * data_real - w_imag * data_imag;
	    sum_imag += w_real * data_imag + w_imag * data_real;

	    exponent = (exponent + i) % n;
	}
	result[i].real = sum_real;
	result[i].imag = sum_imag;
    }
    return 0;
}

int
gsl_dft_complex_forward (const complex data[],
			 complex result[],
			 const unsigned int n) {
    gsl_fft_direction sign = forward;
    int status = gsl_dft_complex (data, result, n, sign);
    return status;
}

int
gsl_dft_complex_backward (const complex data[],
			  complex result[],
			  const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_dft_complex (data, result, n, sign);
    return status;
}


int
gsl_dft_complex_inverse (const complex data[],
			 complex result[],
			 const unsigned int n) {
    gsl_fft_direction sign = backward;
    int status = gsl_dft_complex (data, result, n, sign);

    /* normalize inverse fft with 1/n */

    {
	const double norm = 1.0 / n;
	unsigned int i;
	for (i = 0; i < n; i++) {
	    result[i].real *= norm;
	    result[i].imag *= norm;
	}
    }
    return status;
}



int verbose = 0;

unsigned int tests = 0;
unsigned int passed = 0;
unsigned int failed = 0;

int
gsl_fft_complex_factorize (const unsigned int n,
			   unsigned int *nf,
			   unsigned int factors[]) {
    const unsigned int complex_subtransforms[] = {
	7, 6, 5, 4, 3, 2, 0
    };

    /* other factors can be added here if their transform modules are
       implemented. The end of the list is marked by 0. */

    int status = gsl_fft_factorize (n, complex_subtransforms, nf, factors);
    return status;
}

int
gsl_fft_halfcomplex_factorize (const unsigned int n,
			       unsigned int *nf,
			       unsigned int factors[]) {
    const unsigned int halfcomplex_subtransforms[] = {
	5, 4, 3, 2, 0
    };

    int status = gsl_fft_factorize (n, halfcomplex_subtransforms, nf, factors);
    return status;
}

int
gsl_fft_real_factorize (const unsigned int n,
			unsigned int *nf,
			unsigned int factors[]) {
    const unsigned int real_subtransforms[] = {
	5, 4, 3, 2, 0
    };

    int status = gsl_fft_factorize (n, real_subtransforms, nf, factors);
    return status;
}


int
gsl_fft_factorize (const unsigned int n,
		   const unsigned int implemented_subtransforms[],
		   unsigned int *n_factors,
		   unsigned int factors[]) {
    unsigned int nf = 0;
    unsigned int ntest = n;
    unsigned int factor_sum = 0;
    unsigned int factor;
    unsigned int i = 0;

    if (n == 1) {
	factors[0] = 1;
	*n_factors = 1;
	return 1;
    }

    /* deal with the implemented factors first */

    while (implemented_subtransforms[i] && ntest != 1) {
	factor = implemented_subtransforms[i];
	while ((ntest % factor) == 0) {
	    ntest = ntest / factor;
	    factors[nf] = factor;
	    factor_sum += factor;
	    nf++;
	}
	i++;
    }

    /* deal with any other even prime factors (there is only one) */

    factor = 2;

    while ((ntest % factor) == 0 && (ntest != 1)) {
	ntest = ntest / factor;
	factors[nf] = factor;
	factor_sum += factor;
	nf++;
    }

    /* deal with any other odd prime factors */

    factor = 3;

    while (ntest != 1) {
	while ((ntest % factor) != 0) {
	    factor += 2;
	}
	ntest = ntest / factor;
	factors[nf] = factor;
	factor_sum += factor;
	nf++;
    }

    /* check that the factorization is correct */
    {
	unsigned int product = 1;

	for (i = 0; i < nf; i++) {
	    product *= factors[i];
	}

    }

    *n_factors = nf;

    /* the sum of the factors gives the scaling of the algorithm

       T ~ O(n factor_sum)

       a well factorized length has a factor sum which is much less than n */

    return factor_sum;
}


int gsl_fft_binary_logn (const unsigned int n) {
    unsigned int ntest ;
    unsigned int binary_logn = 0 ;
    unsigned int k = 1;

    while (k < n) {
	k *= 2;
	binary_logn++;
    }

    ntest = (1 << binary_logn) ;

    if (n != ntest ) {
	/* n is not a power of 2 */
	return -1 ;
    } else {
	return binary_logn;
    }

}


int
gsl_fft_signal_complex_pulse (const unsigned int k,
			      const unsigned int n,
			      const double z_real,
			      const double z_imag,
			      complex data[],
			      complex fft[]) {
    unsigned int j;

    /* complex pulse at position k,  data[j] = z * delta_{jk} */

    for (j = 0; j < n; j++) {
	data[j].real = 0.0;
	data[j].imag = 0.0;
    }

    data[k % n].real = z_real;
    data[k % n].imag = z_imag;

    /* fourier transform, fft[j] = z * exp(-2 pi i j k / n) */

    for (j = 0; j < n; j++) {
	const double arg = -2 * M_PI * ((double) ((j * k) % n)) / ((double) n);
	const double w_real = cos (arg);
	const double w_imag = sin (arg);
	fft[j].real = w_real * z_real - w_imag * z_imag;
	fft[j].imag = w_real * z_imag + w_imag * z_real;
    }

    return 0;

}


int
gsl_fft_signal_complex_constant (const unsigned int n,
				 const double z_real,
				 const double z_imag,
				 complex data[],
				 complex fft[]) {
    unsigned int j;

    /* constant, data[j] = z */

    for (j = 0; j < n; j++) {
	data[j].real = z_real;
	data[j].imag = z_imag;
    }

    /* fourier transform, fft[j] = n z delta_{j0} */

    for (j = 0; j < n; j++) {
	fft[j].real = 0.0;
	fft[j].imag = 0.0;
    }

    fft[0].real = ((double) n) * z_real;
    fft[0].imag = ((double) n) * z_imag;

    return 0;

}


int
gsl_fft_signal_complex_exp (const int k,
			    const unsigned int n,
			    const double z_real,
			    const double z_imag,
			    complex data[],
			    complex fft[]) {
    unsigned int j;

    /* exponential,  data[j] = z * exp(2 pi i j k) */

    for (j = 0; j < n; j++) {
	const double arg = 2 * M_PI * ((double) ((j * k) % n)) / ((double) n);
	const double w_real = cos (arg);
	const double w_imag = sin (arg);
	data[j].real = w_real * z_real - w_imag * z_imag;
	data[j].imag = w_real * z_imag + w_imag * z_real;
    }

    /* fourier transform, fft[j] = z * delta{(j - k),0} */

    for (j = 0; j < n; j++) {
	fft[j].real = 0.0;
	fft[j].imag = 0.0;
    }

    {
	int freq;

	if (k <= 0) {
	    freq = (n - k) % n ;
	} else {
	    freq = (k % n);
	};

	fft[freq].real = ((double) n) * z_real;
	fft[freq].imag = ((double) n) * z_imag;
    }

    return 0;

}


int
gsl_fft_signal_complex_exppair (const int k1,
				const int k2,
				const unsigned int n,
				const double z1_real,
				const double z1_imag,
				const double z2_real,
				const double z2_imag,
				complex data[],
				complex fft[]) {
    unsigned int j;

    /* exponential,  data[j] = z1 * exp(2 pi i j k1) + z2 * exp(2 pi i j k2) */

    for (j = 0; j < n; j++) {
	const double arg1 = 2 * M_PI * ((double) ((j * k1) % n)) / ((double) n);
	const double w1_real = cos (arg1);
	const double w1_imag = sin (arg1);
	const double arg2 = 2 * M_PI * ((double) ((j * k2) % n)) / ((double) n);
	const double w2_real = cos (arg2);
	const double w2_imag = sin (arg2);
	data[j].real = w1_real * z1_real - w1_imag * z1_imag;
	data[j].imag = w1_real * z1_imag + w1_imag * z1_real;
	data[j].real += w2_real * z2_real - w2_imag * z2_imag;
	data[j].imag += w2_real * z2_imag + w2_imag * z2_real;
    }

    /* fourier transform, fft[j] = z1 * delta{(j - k1),0} + z2 *
       delta{(j - k2,0)} */

    for (j = 0; j < n; j++) {
	fft[j].real = 0.0;
	fft[j].imag = 0.0;
    }

    {
	int freq1, freq2;

	if (k1 <= 0) {
	    freq1 = (n - k1) % n;
	} else {
	    freq1 = (k1 % n);
	};

	if (k2 <= 0) {
	    freq2 = (n - k2) % n;
	} else {
	    freq2 = (k2 % n);
	};

	fft[freq1].real += ((double) n) * z1_real;
	fft[freq1].imag += ((double) n) * z1_imag;
	fft[freq2].real += ((double) n) * z2_real;
	fft[freq2].imag += ((double) n) * z2_imag;
    }

    return 0;

}


int
gsl_fft_signal_complex_noise (const unsigned int n,
			      complex data[],
			      complex fft[]) {
    unsigned int i;
    int status;

    for (i = 0; i < n; i++) {
	data[i].real = ((double) rand ()) / RAND_MAX;
	data[i].imag = ((double) rand ()) / RAND_MAX;
    }

    /* compute the dft */
    status = gsl_dft_complex_forward (data, fft, n);

    return status;
}


int
gsl_fft_signal_real_noise (const unsigned int n,
			   complex data[],
			   complex fft[]) {
    unsigned int i;
    int status;

    for (i = 0; i < n; i++) {
	data[i].real = ((double) rand ()) / RAND_MAX;
	data[i].imag = 0.0;
    }

    /* compute the dft */
    status = gsl_dft_complex_forward (data, fft, n);

    return status;
}

int
gsl_fft_complex (complex data[],
		 const unsigned int n,
		 const gsl_fft_complex_wavetable * wavetable,
		 const gsl_fft_direction sign) {

    const unsigned int nf = wavetable->nf;

    unsigned int i;

    unsigned int q, product = 1;

    complex *scratch = wavetable->scratch;
    complex *twiddle1, *twiddle2, *twiddle3, *twiddle4, *twiddle5, *twiddle6;

    unsigned int state = 0;
    complex *from = data;
    complex *to = scratch;

    if (n == 1) {                           /* FFT of 1 data point is the identity */
	return 0;
    }

    for (i = 0; i < nf; i++) {
	const unsigned int factor = wavetable->factor[i];
	product *= factor;
	q = n / product;

	if (state == 0) {
	    from = data;
	    to = scratch;
	    state = 1;
	} else {
	    from = scratch;
	    to = data;
	    state = 0;
	}

	if (factor == 2) {
	    twiddle1 = wavetable->twiddle[i];
	    gsl_fft_complex_pass_2 (from, to, sign, product, n, twiddle1);
	} else if (factor == 3) {
	    twiddle1 = wavetable->twiddle[i];
	    twiddle2 = twiddle1 + q;
	    gsl_fft_complex_pass_3 (from, to, sign, product, n, twiddle1,
				    twiddle2);
	} else if (factor == 4) {
	    twiddle1 = wavetable->twiddle[i];
	    twiddle2 = twiddle1 + q;
	    twiddle3 = twiddle2 + q;
	    gsl_fft_complex_pass_4 (from, to, sign, product, n, twiddle1,
				    twiddle2, twiddle3);
	} else if (factor == 5) {
	    twiddle1 = wavetable->twiddle[i];
	    twiddle2 = twiddle1 + q;
	    twiddle3 = twiddle2 + q;
	    twiddle4 = twiddle3 + q;
	    gsl_fft_complex_pass_5 (from, to, sign, product, n, twiddle1,
				    twiddle2, twiddle3, twiddle4);
	} else if (factor == 6) {
	    twiddle1 = wavetable->twiddle[i];
	    twiddle2 = twiddle1 + q;
	    twiddle3 = twiddle2 + q;
	    twiddle4 = twiddle3 + q;
	    twiddle5 = twiddle4 + q;
	    gsl_fft_complex_pass_6 (from, to, sign, product, n, twiddle1,
				    twiddle2, twiddle3, twiddle4, twiddle5);
	} else if (factor == 7) {
	    twiddle1 = wavetable->twiddle[i];
	    twiddle2 = twiddle1 + q;
	    twiddle3 = twiddle2 + q;
	    twiddle4 = twiddle3 + q;
	    twiddle5 = twiddle4 + q;
	    twiddle6 = twiddle5 + q;
	    gsl_fft_complex_pass_7 (from, to, sign, product, n, twiddle1,
				    twiddle2, twiddle3, twiddle4, twiddle5, twiddle6);
	} else {
	    twiddle1 = wavetable->twiddle[i];
	    gsl_fft_complex_pass_n (from, to, sign, factor, product, n,
				    twiddle1);
	}
    }

    if (state == 1)               /* copy results back from scratch to data */
    {
	memcpy (data, scratch, n * sizeof (complex));
    }

    return 0;

}


