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

void fmpzHarmonic(int i_cap){
	
	fmpz_t loopCount;
	fmpz_init_set_ui(loopCount, 1);
	double result = 0;
	
	uint64_t start, end, duration;
	start = rdtsc();

	for(loopCount; !fmpz_equal_ui(loopCount, i_cap); fmpz_add_ui(loopCount, loopCount, 1) ){
		ulong div = fmpz_get_ui(loopCount);
		result += (1.0 / div);
	}
	std::cout << result << std::endl;

	end = rdtsc();
	duration = end - start;
	std::cout << "  fmpz took " << duration << " cycles. " << std::endl;
}

void mpzHarmonic(int i_cap){
	
	mpz_t loopCount;
	mpz_init(loopCount);
	mpz_set_ui(loopCount, 1L);
	double result;

	uint64_t start, end, duration;
	start = rdtsc();

	for(loopCount; mpz_cmp_ui(loopCount, i_cap) != 0; mpz_add_ui(loopCount, loopCount, 1) ){
		ulong div = mpz_get_ui(loopCount);
		result += (1.0 / div);
	}
	std::cout << result << std::endl;

	end = rdtsc();
	duration = end - start;
	std::cout << "   mpz took " << duration << " cycles. " << std::endl;

}

void ulongHarmonic(int i_cap){
	ulong u_cap = (ulong) i_cap;
	double result = 0;

	uint64_t start, end, duration;
	start = rdtsc();

	for(ulong loopCount = 1; loopCount != u_cap; loopCount++){
		result += (1.0 / loopCount);
	}
	std::cout << result << std::endl;

	end = rdtsc();
	duration = end - start;
	std::cout << " ulong took " << duration << " cycles. " << std::endl;
}

void intHarmonic(int i_cap){
	
	double result = 0;

	uint64_t start, end, duration;
	start = rdtsc();

	for(int loopCount = 1; loopCount != i_cap; loopCount++){
		result += (1.0 / loopCount);
	}
	std::cout << result << std::endl;

	end = rdtsc();
	duration = end - start;
	std::cout << "   int took " << duration << " cycles. " << std::endl;
}

int main(){
	int cap = 10000000;
	fmpzHarmonic(cap);
	mpzHarmonic(cap);	
	ulongHarmonic(cap);
	intHarmonic(cap);

	return 0;
}
