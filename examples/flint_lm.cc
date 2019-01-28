#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include <flint.h>
#include "fmpz.h"
#include "fmpq.h"

//Gibt momentanen Taktzyklus an.
uint64_t rdtsc()
{
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

void fmpqLM(int cap, double c, long f){
	fmpq_t result;
	fmpq_init(result);
	fmpq_set_si(result, 1, 2);

	fmpq_t emr;
	fmpq_init(emr);

	fmpq_t eins;
	fmpq_init(eins);
	fmpq_one(eins);

	fmpq_t fmpqc;
	fmpq_init(fmpqc);
	mpq_t mpqc;
	mpq_init(mpqc);
	mpq_set_d(mpqc, c);
	fmpq_set_mpq(fmpqc, mpqc);
	mpq_clear(mpqc);

	fmpz_t fmpzf;
	fmpz_init_set_si(fmpzf, f);


	uint64_t start, end, duration;
	start = rdtsc();
	
	for(int i = 1; i <= cap; i++){
		//fmpq_sub(emr, eins, result);
		fmpq_mul(result, result, fmpqc);
		fmpq_mul_fmpz(result, result, fmpzf);
		fmpq_sub(emr, eins, result);
		fmpq_mul(result, result, emr);
		fmpq_div_fmpz(result, result, fmpzf);

		//std::cout << "result: " << fmpq_get_str(NULL, 10, result) << std::endl;
	}

	end = rdtsc();
	duration = end - start;
	std::cout << "Parameter: f = " << f << " ; c = " << c << std::endl;
	std::cout << "Dauer    : " << duration << " cycles. " << std::endl;
}

int main(){
	int cap = 20;
	double c = 3.6;
	int f = 1000;
	
	fmpqLM(cap, c, f);	
	fmpqLM(cap, c, 1024);	
	fmpqLM(cap, c, 999983);	
	fmpqLM(cap, c, 10000000000);	

	return 0;
}
