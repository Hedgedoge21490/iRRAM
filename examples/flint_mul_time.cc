//g++ flint_mul_time.cc -O2 -lpthread -lmpfr -lgmp -lflint -I/home/hansus/Dev/ForschungsPraktikum/flint2 -I/home/hansus/Dev/ForschungsPraktikum/gmp-6.1.2 -o flint_mul_time

#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include "flint.h"
#include "fmpz.h"
#include <mpfr.h>

//Gibt momentanen Taktzyklus an.
uint64_t rdtsc()
{
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

/*
################################################################
inline-Funktionen					########
################################################################
*/

inline void my_fmpz_mul_si(fmpz_t f, const fmpz_t g, slong x){
    fmpz c2 = *g;

    if (x == 0)
    {
        fmpz_zero(f);
        return;
    }
    else if (!COEFF_IS_MPZ(c2)) /* c2 is small */
    {
        mp_limb_t prod[2];
        mp_limb_t uc2 = FLINT_ABS(c2);
        mp_limb_t ux = FLINT_ABS(x);

        /* unsigned limb by limb multiply (assembly for most CPU's) */
        umul_ppmm(prod[1], prod[0], uc2, ux);
        if ((c2 ^ x) < WORD(0))
            fmpz_neg_uiui(f, prod[1], prod[0]);
        else
            fmpz_set_uiui(f, prod[1], prod[0]);
    }
    else                        /* c2 is large */
    {
        __mpz_struct *mpz_ptr = _fmpz_promote(f);   /* ok without val as if aliased both are large */
        flint_mpz_mul_si(mpz_ptr, COEFF_TO_PTR(c2), x);
    }
}

inline void my_fmpz_mul(fmpz_t f, const fmpz_t g, const fmpz_t h){
    fmpz c1, c2;
    __mpz_struct *mpz_ptr;

    c1 = *g;

    if (!COEFF_IS_MPZ(c1))      /* g is small */
    {
        my_fmpz_mul_si(f, h, c1);
        return;
    }

    c2 = *h;                    /* save h in case it is aliased with f */

    if (c2 == WORD(0))               /* special case, h = 0  */
    {
        fmpz_zero(f);
        return;
    }

    mpz_ptr = _fmpz_promote(f); /* h is saved, g is already large */

    if (!COEFF_IS_MPZ(c2))      /* g is large, h is small */
        flint_mpz_mul_si(mpz_ptr, COEFF_TO_PTR(c1), c2);
    else                        /* c1 and c2 are large */
        mpz_mul(mpz_ptr, COEFF_TO_PTR(c1), COEFF_TO_PTR(c2));
}

//Multiplikation
void intMulTest(){
	int cap = 1000000000;
	std::printf("Führe %d Multiplikationen aus ", cap);

	int fac1 = 42;
	int fac2 = 137;
	int result;

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		asm volatile(""::: "memory");
		result = fac1 * fac2;
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  int took " << duration << " cycles. " << std::endl;
}

void fmpzMulTestInline(){
	int cap = 1000000000;
	std::printf("Führe %d Multiplikationen aus ", cap);

	fmpz_t fac1;
	fmpz_init(fac1);
	fmpz_set_ui(fac1, 42L);
	fmpz_t fac2;
	fmpz_init(fac2);
	fmpz_set_ui(fac1, 1337L);
	fmpz_t result;
	fmpz_init(result);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		my_fmpz_mul(result, fac1, fac2);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles mit inlining. " << std::endl;
}

void fmpzMulTest(){
	int cap = 1000000000;
	std::printf("Führe %d Multiplikationen aus ", cap);

	fmpz_t fac1;
	fmpz_init(fac1);
	fmpz_set_ui(fac1, 42L);
	fmpz_t fac2;
	fmpz_init(fac2);
	fmpz_set_ui(fac1, 1337L);
	fmpz_t result;
	fmpz_init(result);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		fmpz_mul(result, fac1, fac2);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles. " << std::endl;
}

void mpzMulTest(){
	int cap = 1000000000;
		std::printf("Führe %d Multiplikationen aus ", cap);

		mpz_t fac1;
		mpz_init(fac1);
		mpz_set_ui(fac1, 42L);
		mpz_t fac2;
		mpz_init(fac2);
		mpz_set_ui(fac1, 1337L);
		mpz_t result;
		mpz_init(result);

		//Legt variablen zu Speicherung der Cloclcycles an.
		uint64_t start, end, duration;
		start = rdtsc();

		for(int i = 0; i < cap ; i++){
			mpz_mul(result, fac1, fac2);
		}

		//Berechnet Clockcycle-Duration und gibt diese aus.
		end = rdtsc();
		duration = end - start;
		std::cout << "  mpz took " << duration << " cycles. " << std::endl;
}

int main(){
	intMulTest();
	fmpzMulTestInline();
	fmpzMulTest();
	mpzMulTest();
	return 0;
}
