
//g++ flint_loop_time.cc -O2 -lpthread -lmpfr -lgmp -lflint -I/home/hansus/Dev/ForschungsPraktikum/flint2 -I/home/hansus/Dev/ForschungsPraktikum/gmp-6.1.2

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



inline void my_fmpz_set_uiui(fmpz_t f, mp_limb_t hi, mp_limb_t lo){
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

inline void my_fmpz_add_ui(fmpz_t f, const fmpz_t g, ulong x){
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

inline int my_fmpz_equal_ui(const fmpz_t f, ulong g){
    fmpz c = *f;

    return !COEFF_IS_MPZ(c) ? ((c >= 0) & (c == g)) :
                              !flint_mpz_cmp_ui(COEFF_TO_PTR(c), g);
}

//Schleifensteuerung
void intSchleifensteuerung(){
	int i_cap = 1000000000;
	std::printf("Schleifenzähler geht bis: %d ", i_cap);

	//Legt variablen zu Speicherung der Clockcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	//Schleife, die gemessen wird.
	for(int i = 0; i < i_cap; i++){
		asm volatile(""::: "memory");
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  int took " << duration << " cycles. " << std::endl;
}

void fmpzSchleifensteuerung(){
	ulong cap = 1000000000;
	std::printf("Schleifenzähler geht bis: %d ", cap);

	fmpz_t loopCount;
	fmpz_init(loopCount);
	fmpz_set_ui(loopCount, 0L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(fmpz_t loopcount; !my_fmpz_equal_ui(loopCount, cap); my_fmpz_add_ui(loopCount, loopCount, 1) ){}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles. " << std::endl;
}

void mpzSchleifensteuerung(){
	ulong cap = 1000000000;
	std::printf("Schleifenzähler geht bis: %d ", cap);

	mpz_t loopCount;
	mpz_init(loopCount);
	mpz_set_ui(loopCount, 0L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(mpz_t loopcount; mpz_cmp_ui(loopCount, cap) != 0 ; mpz_add_ui(loopCount, loopCount, 1)){}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  mpz took " << duration << " cycles. " << std::endl;
}

int main(){
	intSchleifensteuerung();
	fmpzSchleifensteuerung();
	mpzSchleifensteuerung();
	return 0;
}
