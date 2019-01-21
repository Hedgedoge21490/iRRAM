//g++ flint_dif_time.cc -O2 -lpthread -lmpfr -lgmp -lflint -I/home/hansus/Dev/ForschungsPraktikum/flint2 -I/home/hansus/Dev/ForschungsPraktikum/gmp-6.1.2 -o flint_dif_time

#include <iostream>
#include <stdio.h>
#include <gmp.h>
#include "flint.h"
#include "fmpz.h"

//Gibt momentanen Taktzyklus an.
uint64_t rdtsc()
{
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}


//Subtraktion
void intSubtraktionTest(){
	int cap = 1000000000;
	std::printf("Führe %d Subraktionen aus ", cap);

	int startwert = cap;

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		asm volatile(""::: "memory");
		startwert--;
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  int took " << duration << " cycles. " << std::endl;
}

void fmpzSubtraktionTest(){
	int cap = 1000000000;
	std::printf("Führe %d Subraktionen aus ", cap);

	fmpz_t startwert;
	fmpz_init(startwert);
	fmpz_set_si(startwert, cap);

	fmpz_t eins;
	fmpz_init(eins);
	fmpz_set_ui(eins, 1L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		fmpz_sub(startwert, startwert, eins);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << " fmpz took " << duration << " cycles. " << std::endl;
}

void mpzSubraktionTest(){
	int cap = 1000000000;
	std::printf("Führe %d Subraktionen aus ", cap);

	mpz_t startwert;
	mpz_init(startwert);
	mpz_set_ui(startwert, cap);

	mpz_t eins;
	mpz_init(eins);
	mpz_set_ui(eins, 1L);

	//Legt variablen zu Speicherung der Cloclcycles an.
	uint64_t start, end, duration;
	start = rdtsc();

	for(int i = 0; i < cap ; i++){
		mpz_sub(startwert, startwert, eins);
	}

	//Berechnet Clockcycle-Duration und gibt diese aus.
	end = rdtsc();
	duration = end - start;
	std::cout << "  mpz took " << duration << " cycles. " << std::endl;
}

int main(){

	mpzSubraktionTest();
	fmpzSubtraktionTest();
	intSubtraktionTest();
	
	return 0;
}
