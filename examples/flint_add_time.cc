
//g++ flint_add_time.cc -O2 -lpthread -lmpfr -lgmp -lflint -I/home/hansus/Dev/ForschungsPraktikum/flint2 -I/home/hansus/Dev/ForschungsPraktikum/gmp-6.1.2 -o flint-add-time

#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include <flint.h>
#include "fmpz.h"

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

#define my_COEFF_IS_MPZ(x) (((x) >> (FLINT_BITS - 2)) == WORD(1))

inline void my_fmpz_demote(fmpz_t f)
{
    /*
       warning, if fmpz_demote changes, fmpz_zero must
       also be changed to match
    */
    if (COEFF_IS_MPZ(*f))
    {
        _fmpz_clear_mpz(*f);
        (*f) = WORD(0);
    }
}

inline void my_fmpz_set_si(fmpz_t f, slong val)
{
    if (val < COEFF_MIN || val > COEFF_MAX) /* val is large */
    {
        __mpz_struct *mpz_coeff = _fmpz_promote(f);
        flint_mpz_set_si(mpz_coeff, val);
    }
    else
    {
        my_fmpz_demote(f);
        *f = val;               /* val is small */
    }
}

inline void my_fmpz_add(fmpz_t f, const fmpz_t g, const fmpz_t h)
{
    fmpz c1 = *g;
    fmpz c2 = *h;

    if (!my_COEFF_IS_MPZ(c1))  /* g is small */
    {
        if (!my_COEFF_IS_MPZ(c2))  /* both inputs are small */
        {
            my_fmpz_set_si(f, c1 + c2);
        } else  /* g is small, h is large */
        {
            __mpz_struct * mpz3 = _fmpz_promote(f);  /* g is saved and h is large */
            __mpz_struct * mpz2 = COEFF_TO_PTR(c2);
            if (c1 < WORD(0)) flint_mpz_sub_ui(mpz3, mpz2, -c1);
            else flint_mpz_add_ui(mpz3, mpz2, c1);
            _fmpz_demote_val(f);  /* may have cancelled */
        }
    }
    else
    {
        if (!my_COEFF_IS_MPZ(c2))  /* g is large, h is small */
        {
            __mpz_struct * mpz3 = _fmpz_promote(f);  /* h is saved and g is large */
            __mpz_struct * mpz1 = COEFF_TO_PTR(c1);
            if (c2 < WORD(0)) flint_mpz_sub_ui(mpz3, mpz1, -c2);
            else flint_mpz_add_ui(mpz3, mpz1, c2);
            _fmpz_demote_val(f);  /* may have cancelled */
        }
        else  /* g and h are large */
        {
            __mpz_struct * mpz3 = _fmpz_promote(f);  /* aliasing means f is already large */
            __mpz_struct * mpz1 = COEFF_TO_PTR(c1);
            __mpz_struct * mpz2 = COEFF_TO_PTR(c2);
            mpz_add(mpz3, mpz1, mpz2);
            _fmpz_demote_val(f);  /* may have cancelled */
        }
    }
}

inline void my_fmpz_set_uiui(fmpz_t f, mp_limb_t hi, mp_limb_t lo)
{
    if (hi == 0)
    {
        fmpz_set_ui(f, lo);
    }
    else
    {
        __mpz_struct *z = _fmpz_promote(f);
        if (z->_mp_alloc < 2)
            mpz_realloc2(z, 2 * FLINT_BITS);
        z->_mp_d[0] = lo;
        z->_mp_d[1] = hi;
        z->_mp_size = 2;
    }
}

inline void my_fmpz_add_ui(fmpz_t f, const fmpz_t g, ulong x)
{
	fmpz c = *g;

	if (!COEFF_IS_MPZ(c))  /* g is small */
	{
        mp_limb_t sum[2];
		if (c >= WORD(0))  /* both operands non-negative */
		{
			add_ssaaaa(sum[1], sum[0], 0, c, 0, x);
            my_fmpz_set_uiui(f, sum[1], sum[0]);
		}
        else  /* coeff is negative, x positive */
		{
			if (-c > x)
                fmpz_set_si(f, x + c); /* can't overflow as g is small and x smaller */
			else
                fmpz_set_ui(f, x + c);  /* won't be negative and has to be less than x */
		}
	}
    else
	{
		__mpz_struct * mpz_ptr2 = _fmpz_promote(f);  /* g is already large */
		__mpz_struct * mpz_ptr = COEFF_TO_PTR(c);
		flint_mpz_add_ui(mpz_ptr2, mpz_ptr, x);
		_fmpz_demote_val(f);  /* cancellation may have occurred */
	}
}

inline int my_fmpz_equal_ui(const fmpz_t f, ulong g)
{
    fmpz c = *f;

    return !COEFF_IS_MPZ(c) ? ((c >= 0) & (c == g)) :
                              !flint_mpz_cmp_ui(COEFF_TO_PTR(c), g);
}

/*
################################################################
TEST-Funktionen					########
################################################################
*/

inline void fmpzIncrementTest(){
	int i_cap = 1000000000;

	std::printf("Incrementiere fmpz um: %d ", i_cap);

	fmpz_t increment;
	fmpz_init(increment);
	fmpz_set_ui(increment, 0L);

	fmpz_t eins;
	fmpz_init(eins);
	fmpz_set_ui(eins, 1L);

	//Legt variablen zu Speicherung der Clockcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		fmpz_add(increment, increment, eins);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles. " << std::endl;

	//Ausgabefunktion für fmpz.
	//fmpz_print(increment);
}

inline void fmpzIncrementTestMyAdd(){

	int i_cap = 1000000000;

	std::printf("Incrementiere fmpz um: %d ", i_cap);

	fmpz_t increment;
	fmpz_init(increment);
	fmpz_set_ui(increment, 0L);

	fmpz_t eins;
	fmpz_init(eins);
	fmpz_set_ui(eins, 1L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		my_fmpz_add(increment, increment, eins);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles with local addition. " << std::endl;

	//Ausgabefunktion für fmpz.
	//fmpz_print(increment);
}

inline void fmpzIncrementTestAddUI(){

	int i_cap = 1000000000;

	std::printf("Incrementiere fmpz um: %d ", i_cap);

	fmpz_t increment;
	fmpz_init(increment);
	fmpz_set_ui(increment, 0L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		my_fmpz_add_ui(increment, increment, 1);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles with add_ui method. " << std::endl;

}

inline void mpzIncrementTest(){
	int i_cap = 1000000000;
	std::printf("Incrementiere  mpz um: %d ", i_cap);

	mpz_t increment;
	mpz_init(increment);
	mpz_set_ui(increment, 0L);

	mpz_t eins;
	mpz_init(eins);
	mpz_set_ui(eins, 1L);

	//Legt variablen zu Speicherung der Clockcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		mpz_add(increment, increment, eins);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  mpz took " << duration << " cycles. " << std::endl;

	//Ausgabefunktion für mpz.
	//mpz_out_str(stdout, 10 ,increment);
}

void intIncrementTestVolatile(){
	int i_cap = 1000000000;
	std::printf("Incrementiere  int um: %d ", i_cap);

	int increment = 0;

	//Legt variablen zu Speicherung der Clockcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		increment++;
		asm volatile(""::: "memory");
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
		end = rdtsc();
		duration = end - start;
		std::cout << "  int took " << duration << " cycles. " << std::endl;
}

int main(){

	fmpzIncrementTest();
	fmpzIncrementTestMyAdd();
	fmpzIncrementTestAddUI();
	mpzIncrementTest();
	intIncrementTestVolatile();
	//intIncrementTest();

	return 0;
}
