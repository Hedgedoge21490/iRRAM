#include "iRRAM.h"

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
	RATIONAL f = RATIONAL(largestuff);
	largestuff += 1;
	RATIONAL x = RATIONAL(0.5);
	RATIONAL emx;;
	INTEGER xint;

	//TIME
	uint64_t start, end, duration;
	start = rdtsc();
	
	//LOOP
	for(INTEGER loopcount = 1; loopcount != loopcap; loopcount=loopcount + 1){
		
		//cout << "xn ist : \n" << swrite(x) << "\n\n";

		emx = RATIONAL(1,1) - x ;
		//cout << "emx ist : \n" << swrite(emx) << "\n\n";
		
		x = f * c * x * emx;
		//cout << "xn+1 ist : \n" << swrite(x) << "\n\n";
		

		xint = denominator(x) / numerator(x);
		
		//cout << "xint ist:        " << swrite(xint) << "\n\n";

		x = RATIONAL(xint);

		x = x/f;
	}
	
	//TIME
	end = rdtsc();
	duration = end - start;

	cout << "Dieser Test dauerte " << duration << " Zyklen!\n";
}
