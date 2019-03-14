#include "iRRAM.h"
#include <csignal>
using namespace iRRAM;
using std::setw;

//Gibt momentanen Taktzyklus an.
uint64_t rdtsc()
{
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

void compute(){	
	//INIT
	INTEGER loopcap 	= 1000000;

	INTEGER largestuff = INTEGER(10);
	largestuff = power(largestuff,100);
	RATIONAL c = RATIONAL(3.75);
	largestuff = largestuff + 1;
	INTEGER f = INTEGER(99991);
	RATIONAL x = RATIONAL(0.5);
	
	RATIONAL emx;
	INTEGER numtmp;
	INTEGER dentmp;
	INTEGER xint;

	INTEGER numx;
	INTEGER denx;
	INTEGER numemx;
	INTEGER denemx;
	INTEGER numc = INTEGER(numerator(c));
	INTEGER denc = INTEGER(denominator(c));

	//TIME
	uint64_t start, end, duration;
	start = rdtsc();

	//LOOP
	for(INTEGER loopcount = 1; loopcount != loopcap; loopcount=loopcount + 1){
		//Berechnet (1-x)
		emx = RATIONAL(1,1) - x ;
		numx = INTEGER(numerator(x));
		denx = INTEGER(denominator(x));
		numemx = INTEGER(numerator(emx));
		denemx = INTEGER(denominator(emx));

		/*
		cout << loopcount << ") num x       : " << swrite(numx) << "\n";
		cout << loopcount << ") den x       : " << swrite(denx) << "\n_______________\n";
		cout << loopcount << ") num emx     : " << swrite(numemx) << "\n";
		cout << loopcount << ") den emx     : " << swrite(denemx) << "\n_______________\n";
		*/		

		//Zähler aufblähen
		numtmp = f * numc * numx * numemx;
		//cout << "1) Aufgeblähter Zähler    : " << swrite(numtmp) << "\n";

		//Nenner berechnen
		dentmp = denc * denemx * denx;
		//cout << "2) Normaler Nenner        : " << swrite(dentmp) << "\n";
			
		//Hier findet die Division mit Rundung statt.
		xint = numtmp / dentmp;
		//cout << "3) Ergebnis mit Präzision : " << swrite(xint) << "\n";

		//Zuletzt die Präzision rausrechnen (truncate) und als RATIONAL speichern.
		x = RATIONAL(xint, f);
		//cout << "4) Ergebnis als Rational  : " << swrite(x) << "\n__________________________\n";
				
	}
	
	//TIME
	end = rdtsc();
	duration = end - start;

	cout << "Dieser Test dauerte " << duration << " Zyklen!\n";

	cout << "Endergebnis : " << swrite(xint) << "\n";
}
