#ifndef _GSL_FFT_H
#define _GSL_FFT_H


typedef enum
  {
    forward = -1, backward = +1
  }
gsl_fft_direction;

/* this give the sign in the formula

   h(f) = \sum x(t) exp(+/- 2 pi i f t) 
       
   where - is the forward transform direction and + the inverse direction */


typedef struct
  {
    double real;
    double imag;
  }
complex;

typedef struct
  {
    unsigned int n;
    unsigned int nf;
    unsigned int factor[64];
    complex *twiddle[64];
    complex *trig;
    complex *scratch;
  }
gsl_fft_complex_wavetable;

int
  gsl_fft_complex (complex data[],
                   const unsigned int n,
                   const gsl_fft_complex_wavetable * wavetable,
                   const gsl_fft_direction sign);
 int
  gsl_fft_complex_factorize (const unsigned int n,
			     unsigned int *nf,
			     unsigned int factors[]);

int
  gsl_fft_halfcomplex_factorize (const unsigned int n,
				 unsigned int *nf,
				 unsigned int factors[]);

int
  gsl_fft_real_factorize (const unsigned int n,
			  unsigned int *nf,
			  unsigned int factors[]);

int
  gsl_fft_factorize (const unsigned int n,
		     const unsigned int implemented_subtransforms[],
		     unsigned int *n_factors,
		     unsigned int factors[]);

int gsl_fft_binary_logn (const unsigned int n) ;

int gsl_fft_complex_bitreverse_order (complex data[], 
				      const unsigned int n,
				      const unsigned int logn) ;

int gsl_fft_real_bitreverse_order (double data[], 
				   const unsigned int n,
				   const unsigned int logn) ;


int gsl_fft_complex_goldrader_bitreverse_order (complex data[], 
						const unsigned int n) ;

int gsl_fft_real_goldrader_bitreverse_order (double data[], 
					     const unsigned int n) ;

int gsl_fft_complex_rodriguez_bitreverse_order (complex data[], 
						const unsigned int n,
						const unsigned int logn) ;

int gsl_fft_real_rodriguez_bitreverse_order (double data[], 
					     const unsigned int n,
					     const unsigned int logn) ;


  /*  Power of 2 routines  */

int
gsl_fft_complex_radix2_forward (complex data[],
				const unsigned int n);

int
gsl_fft_complex_radix2_backward (complex data[],
				 const unsigned int n);

int
gsl_fft_complex_radix2_inverse (complex data[],
				const unsigned int n);

int
gsl_fft_complex_radix2 (complex data[],
			const unsigned int n,
			const gsl_fft_direction sign);


int
gsl_fft_complex_radix2_dif_forward (complex data[],
				    const unsigned int n);

int
gsl_fft_complex_radix2_dif_backward (complex data[],
				     const unsigned int n);

int
gsl_fft_complex_radix2_dif_inverse (complex data[],
				    const unsigned int n);

int
gsl_fft_complex_radix2_dif (complex data[],
			    const unsigned int n,
			    const gsl_fft_direction sign);

int gsl_fft_binary_logn (const unsigned int n);

int gsl_fft_complex_bitreverse_order (complex data[], 
				      const unsigned int n,
				      const unsigned int n_bits);

  /*  Mixed Radix general-N routines  */

int gsl_fft_complex_forward (complex data[],
			     const unsigned int n,
			     const gsl_fft_complex_wavetable * wavetable);

int gsl_fft_complex_backward (complex data[],
			      const unsigned int n,
			      const gsl_fft_complex_wavetable * wavetable);

int gsl_fft_complex_inverse (complex data[],
			     const unsigned int n,
			     const gsl_fft_complex_wavetable * wavetable);

int
  gsl_fft_complex (complex data[],
		   const unsigned int n,
		   const gsl_fft_complex_wavetable * wavetable,
		   const gsl_fft_direction sign);

int
  gsl_fft_complex_init (const unsigned int n,
			gsl_fft_complex_wavetable * wavetable);

int
  gsl_fft_complex_generate_wavetable (unsigned int n,
				      gsl_fft_complex_wavetable * wavetable);

int
  gsl_fft_complex_wavetable_alloc (unsigned int n,
				   gsl_fft_complex_wavetable * wavetable);

int
  gsl_fft_complex_wavetable_free (gsl_fft_complex_wavetable * wavetable);



#endif /* _GSL_FFT_H */
