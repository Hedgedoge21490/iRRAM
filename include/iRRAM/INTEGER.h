/*

iRRAM_INTEGER.h -- header file for the INTEGER part of the iRRAM library

Copyright (C) 2001-2003 Norbert Mueller

This file is part of the iRRAM Library.
The iRRAM Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The iRRAM Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. 
*/



#ifndef iRRAM_INTEGER_H
#define iRRAM_INTEGER_H

#include <string>

#include <iRRAM/core.h>
#include <flint/fmpz.h>

#include <mpfr.h>

namespace iRRAM {

/*! \ingroup types */

class INTEGER 
{
	fmpz_t value; 						//changed MPINT to FLINT

	INTEGER(fmpz_t y);					//changed MPINT to FLINT

	friend INTEGER numerator(const RATIONAL &);
	friend INTEGER denominator(const RATIONAL &);

	friend class RATIONAL;
	friend class REAL;
	friend class DYADIC;

	/* internal tag type to indicate the MP_int_type passed to INTEGER's
	 * constructor is supposed to be moved and not copied (in case MP_int_type
	 * matches one of the other conversion sources like e.g. mpz_srcptr). */
	struct move_t {};

	INTEGER(fmpz_t y, move_t);

public:

/*
	#if iRRAM_HAVE_GMP_C
		explicit INTEGER(mpz_srcptr)
	#endif
*/


/****** Constructors ******/


INTEGER(int i = 0);
INTEGER(const std::string &s);
INTEGER(const char* s);
INTEGER(const INTEGER& y);
//INTEGER(INTEGER &&o) : value(o.value) { o.value = nullptr; }
INTEGER(double d);

/****** Copy/move assignment ******/

INTEGER& operator = (const int y);
INTEGER& operator = (const INTEGER& y);
INTEGER & operator=(INTEGER &&o) { using std::swap; swap(value, o.value); return *this; }

	/****** Destructor ******/

~INTEGER();

	/****** Standard arithmetic ******/

friend INTEGER 	operator +  (const INTEGER& x, const INTEGER& y);
friend INTEGER 	operator +  (const INTEGER& x, const int      y);
friend INTEGER 	operator +  (const int      x, const INTEGER& y);

friend INTEGER 	operator -  (const INTEGER& x, const INTEGER& y);
friend INTEGER 	operator -  (const INTEGER& x, const int      y);
friend INTEGER 	operator -  (const int      x, const INTEGER& y);

friend INTEGER 	operator -  (const INTEGER& x);

friend INTEGER 	operator *  (const INTEGER& x, const INTEGER& y);
friend INTEGER	operator *  (const INTEGER& x, const int      y);
friend INTEGER	operator *  (const int      x, const INTEGER& y);

friend INTEGER 	operator /  (const INTEGER& x, const INTEGER& y);
friend INTEGER 	operator /  (const INTEGER& x, const int      y);
friend INTEGER 	operator /  (const int      x, const INTEGER& y);

friend INTEGER 	operator << (const INTEGER& x, const int y);
friend INTEGER 	operator >> (const INTEGER& x, const int y);
friend INTEGER	operator %  (const INTEGER& x, const INTEGER& y);

friend INTEGER&	operator += (	   INTEGER& x, const INTEGER& y);
friend INTEGER& operator += (      INTEGER& x,       int      y);

friend INTEGER&	operator -= (	   INTEGER& x, const INTEGER& y);
friend INTEGER& operator -= (      INTEGER& x,       int      y);

friend INTEGER&	operator *= (	   INTEGER& x, const INTEGER& y);
friend INTEGER& operator *= (      INTEGER& x,       int      y);

friend INTEGER&	operator /= (	   INTEGER& x, const INTEGER& y);
friend INTEGER& operator /= (      INTEGER& x,       int      y);

friend INTEGER & operator%=(INTEGER &x, const INTEGER &y);


friend INTEGER  power(const INTEGER& x,       unsigned y);
friend INTEGER 	sqrt	(const INTEGER& x);
friend INTEGER		scale	(const INTEGER& x, const int k);
friend INTEGER		abs	(const INTEGER& x);

/****** Comparisons ******/

friend bool operator <  (const INTEGER& x, const INTEGER& y);
friend bool operator <  (const INTEGER& x, const int      y);
friend bool operator <  (const int      x, const INTEGER& y);

friend bool operator <= (const INTEGER& x, const INTEGER& y);
friend bool operator <= (const INTEGER& x, const int      y);
friend bool operator <= (const int      x, const INTEGER& y);

friend bool operator >  (const INTEGER& x, const INTEGER& y);
friend bool operator >  (const INTEGER& x, const int      y);
friend bool operator >  (const int      x, const INTEGER& y);

friend bool operator >= (const INTEGER& x, const INTEGER& y);
friend bool operator >= (const INTEGER& x, const int      y);
friend bool operator >= (const int      x, const INTEGER& y);

friend bool operator == (const INTEGER& x, const INTEGER& y);
friend bool operator == (const INTEGER& x, const int      y);
friend bool operator == (const int      x, const INTEGER& y);

friend bool operator != (const INTEGER& x, const INTEGER& y);
friend bool operator != (const INTEGER& x, const int      y);
friend bool operator != (const int      x, const INTEGER& y);

friend int size(const INTEGER& x);
friend int sign(const INTEGER& x);

/* conversion */

friend std::string    swrite  (const INTEGER& x, const int w);
friend std::string    swrite  (const INTEGER& x);

explicit operator int()  const ;

};

//Hier endet die Klassendefinition!  #######################################################################
inline INTEGER::~INTEGER() { if (value) fmpz_clear(value); }			//fmpz_clear nun zur Speicherfreigabe.

inline INTEGER::INTEGER(fmpz_t y, INTEGER::move_t) : value{*y} {}

#if iRRAM_HAVE_GMP_C
//inline INTEGER::INTEGER(mpz_srcptr p);
/*
{
	MP_int_duplicate_w_init(p, value);
}
*/
#endif

inline INTEGER::INTEGER(int i){
	fmpz_init(value);													//fmpz_init
	fmpz_set_si(value, i);												//set_si
}

inline INTEGER::INTEGER(double d){
  fmpz_init(value);														//fmpz_init
  fmpz_set_d(value, d);													//set_d
}

inline INTEGER::INTEGER(const INTEGER& i){
  fmpz_init_set(value, i.value);										//setzt value auf i.value, geht das so? init set?
}


//****************************************************************************************
// Constructing INTEGER from character string
// The base is set to 10 (decimal)
//****************************************************************************************

inline INTEGER::INTEGER(const char* s){
  fmpz_init(value);													//fmpz wird iniziiert und auf s_(decimal) gesetzt
  fmpz_set_str(value, s, 10);
}

inline INTEGER::INTEGER(const std::string &s) : INTEGER(s.c_str()) {}

// mpfr to INTEGER

inline void mpfr_to_INTEGER(MP_type x, fmpz_t f){
	//fmpz_set_mpfr(x, f, MPFR_RNDZ); existiert leider nicht.

	MP_int_type r;
	MP_int_init(r);
	MP_mp_to_INTEGER(x,r);
	fmpz_set_mpz(f,r);
	//freigeben von r
}

inline void INTEGER_to_mpfr(const fmpz_t input, MP_type output){
	fmpz_get_mpfr(output, input, MPFR_RNDZ);
}

// operator zwischen INTEGER und INTEGER Objekt zum Gleichsetzen.

inline INTEGER& INTEGER::operator = (const INTEGER& y){
  fmpz_set(value, y.value);										//man geht davon aus, dass dies auf ein bereit initialisiertes fmpz angewandt wird.
  return (*this);
}

inline INTEGER& INTEGER::operator = (const int y){
  fmpz_set_si(value, y);										//value wird auf y si gesetzt
  return (*this);
}


inline INTEGER scale(const INTEGER& x, const int n)

{
	fmpz_t zvalue;
	fmpz_init(zvalue);

	if(n>=0){							//Fallunterscheidung, je nachdem ob n größer oder kleiner 0. Cast implizit?
	fmpz_mul_2exp(zvalue,x.value ,n); 	//Funktion nimmt nur ulongs für n. Ist der Cast implizit?
	}	
	else{
	int m = -n;							//!2er-Komplement.  unsigned int hier.
	fmpz_tdiv_q_2exp(zvalue, x.value, m);
	}
	return { zvalue, INTEGER::move_t{} };
}

//****************************************************************************************
// Returns the sign of INTEGER objects (-1/0/1): 
// sign(i)= -1, iff i<0,   sign(i)=1 iff i>0,  and sign(0)=0
//****************************************************************************************

inline int sign(const INTEGER& x){  return fmpz_sgn(x.value); }			//fmpz_sgn macht genau, was gefordert ist, -1,0,1

//****************************************************************************************
// Returns the size of INTEGER objects: The smallest i>0 such that |x| < 2^i,
// e.g. size(0)=size(1)=1;size(2)=size(3)=2,size(4)=...size(7)=3
//****************************************************************************************

inline int size(const INTEGER& x){ return fmpz_size(x.value);}			//fmpz_size returns number of bits used to store the value

//****************************************************************************************
// Power: return = x**n
//****************************************************************************************

inline INTEGER power(const INTEGER& x, unsigned n)
{
	if (n == 0 || x == 1) return 1;
	if (n == 1)           return x;
	fmpz_t zvalue;
	fmpz_init(zvalue);
	fmpz_pow_ui(zvalue,x.value,n);										//initiiert nun fmpz und benutzt _pow_ui
	return { zvalue, INTEGER::move_t{} };
}

//****************************************************************************************
// Shifter: returns x << n
// shifts n bits to the left
//****************************************************************************************

inline INTEGER operator << (const INTEGER& x, const int n){
  return scale(x,n);													//nutzt das scale von oben
}

//****************************************************************************************
// Shifter: returns x >> n
// shifts n bits to the right
//****************************************************************************************

inline INTEGER operator >> (const INTEGER& x, const int n){
  return scale(x,-n);													//scale von oben. Implizites Casting?!
}




//****************************************************************************************
// Addition: returns x + y
//****************************************************************************************

inline INTEGER operator + (const INTEGER& x, const INTEGER& y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_add(zvalue,x.value,y.value);									//fmpz_add benutzt
return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator + (const INTEGER& x, const int y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  if (y<0) fmpz_sub_ui(zvalue,x.value,y);							//fmpz_sub_ui
  else     fmpz_add_ui(zvalue,x.value,y);							//fmpz_add_ui
  return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator + (const int     x, const INTEGER& y) {return y+x;}		//Kommutativität

//****************************************************************************************
// Addition: returns x += y
//****************************************************************************************

inline INTEGER& operator += (INTEGER& x, const INTEGER& y){
  fmpz_add(x.value,y.value,x.value);								//x = y+x
  return x;
}

inline INTEGER& operator += (INTEGER& x, int n)
{
	fmpz_t zvalue;
    fmpz_init(zvalue);
	if (n<0) fmpz_sub_ui(zvalue,x.value,n);							//sub_ui
	else     fmpz_add_ui(zvalue,x.value,n);							//add_ui
	return x;
}

//****************************************************************************************
// Subtraction: returns x - y
//****************************************************************************************

inline INTEGER operator - (const INTEGER& x, const INTEGER& y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_sub(zvalue,x.value,y.value);									//fmpz_sub benutzt
  return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator - (const INTEGER& x, const int y){
	return x + (-y);												//benutzt INTEGER operator +
}



inline INTEGER operator - (const int x, const INTEGER& y){
  return -(y-x);													//benutzt INTEGER operator -
}

//****************************************************************************************
// Subtraction: returns x -= y
//****************************************************************************************

inline INTEGER& operator -= (INTEGER& x, const INTEGER& y){
  fmpz_sub(x.value,x.value,y.value);								//x=x-y
  return x;
}

inline INTEGER& operator -= (INTEGER& x, int n) { return x += -n; }		//benutzt += -n

//****************************************************************************************
// Negative: returns -x
//****************************************************************************************

inline INTEGER operator - (const INTEGER& x){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_neg(zvalue,x.value);												//negiert den value von x
  return { zvalue, INTEGER::move_t{} };
}

//****************************************************************************************
// Multiplication: returns x * y
//****************************************************************************************

inline INTEGER operator * (const INTEGER& x, const INTEGER& y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_mul(zvalue,x.value,y.value);										//fmpz_mul
  return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator * (const INTEGER& x, const int y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_mul_si(zvalue,x.value,y);										//fmpz_mul_si
  return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator * (const int x, const INTEGER& y)
{
	return y * x;
}

//****************************************************************************************

// Multiplication: returns x *= y

//****************************************************************************************

inline INTEGER& operator *= (INTEGER& x, const INTEGER& y){
  fmpz_mul(x.value,x.value,y.value);									//fmpz_mul , merke Output ist erstes arg
  return x;
}

inline INTEGER& operator *= (INTEGER& x, int n) {
  fmpz_mul_si(x.value,x.value,n);										//fmpz_mul_si benutzt
  return x;
}

//****************************************************************************************
// Division: returns x / y
//****************************************************************************************

inline INTEGER operator / (const INTEGER& x, const INTEGER& y){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_tdiv_q(zvalue, x.value, y.value);								//rundet zur 0
  return { zvalue, INTEGER::move_t{} };
}

inline INTEGER operator / (const INTEGER& x, const int n){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  fmpz_tdiv_q_si(zvalue, x.value, n);									//fmpz kann vorzeichen bei Division beachten, daher hier kürzer.			
  return { zvalue, INTEGER::move_t{} };
 
}

inline INTEGER operator / (const int      x, const INTEGER& y) {return INTEGER(x)/y;}		//Benutzt kein MPINT

//****************************************************************************************
// Division: returns x /= y
//****************************************************************************************

inline INTEGER& operator /= (INTEGER& x, const INTEGER& y){
  fmpz_tdiv_q(x.value,x.value,y.value);
  return x;
}

inline INTEGER& operator /= (INTEGER& x, int n)
{
	fmpz_tdiv_q_si(x.value, x.value, n);									//fmpz kann vorzeichen bei Division beachten, daher hier kürzer.
	return x;
}

//****************************************************************************************
// Modulo: returns x % y
//****************************************************************************************

inline INTEGER operator % (const INTEGER& x, const INTEGER& y){
	fmpz_t zvalue;
	fmpz_init(zvalue);
	fmpz_mod(zvalue,x.value,y.value);											//mit fmpz_mod
	return { zvalue, INTEGER::move_t{} };
}

inline INTEGER & operator%=(INTEGER &x, const INTEGER &y)
{
	fmpz_mod(x.value,x.value,y.value);											//mit fmpz_mod
	return x;
}

//****************************************************************************************
// Square-root: returns SQRT(x)
// Returns the truncated square-root of the given argument
//****************************************************************************************

/*! \ingroup maths */

inline INTEGER sqrt (const INTEGER& x)
{
	fmpz_t zvalue;
	fmpz_init(zvalue);
	fmpz_sqrt(zvalue, x.value);													// mit fmpz_sqrt (integer part)
	return { zvalue, INTEGER::move_t{} };
}

//****************************************************************************************
// Absolute value: |x|
// returns the positive part of the object
// 1. argument: INTEGER
// return value: INTEGER
//****************************************************************************************

/*! \ingroup maths */

inline INTEGER abs (const INTEGER& x)
{
	fmpz_t zvalue;
	fmpz_init(zvalue);
	fmpz_abs(zvalue,x.value);													//mit fmpz_abs
	return { zvalue, INTEGER::move_t{} };
}

//****************************************************************************************
// boolean operators
// Last change: 2004-12-20
//****************************************************************************************

//****************************************************************************************
// Less than: x < y
//****************************************************************************************

inline bool operator < (const INTEGER& x, const INTEGER& y){
  return (fmpz_cmp(x.value,y.value) < 0 );										//fmpz_cmp nun. funktioniert genauso.
}
inline bool operator <  (const INTEGER& x, const int      y){ return x<INTEGER(y) ;}
inline bool operator <  (const int      x, const INTEGER& y){ return INTEGER(x)<y ;}

inline bool operator <= (const INTEGER& x, const INTEGER& y){ return !(y<x) ;}
inline bool operator <= (const INTEGER& x, const int      y){ return !(y<x) ;}
inline bool operator <= (const int      x, const INTEGER& y){ return !(y<x) ;}

inline bool operator >  (const INTEGER& x, const INTEGER& y){ return y<x ;}
inline bool operator >  (const INTEGER& x, const int      y){ return y<x ;}
inline bool operator >  (const int      x, const INTEGER& y){ return y<x ;}

inline bool operator >= (const INTEGER& x, const INTEGER& y){ return !(x<y) ;}
inline bool operator >= (const INTEGER& x, const int      y){ return !(x<y) ;}
inline bool operator >= (const int      x, const INTEGER& y){ return !(x<y) ;}


inline bool operator == (const INTEGER& x, const INTEGER& y){
  return (fmpz_cmp(x.value,y.value)==0);										// ebenso genauso mit fmpz_cmp
}
inline bool operator == (const INTEGER& x, const int  i){ return x==INTEGER(i);}
inline bool operator == (const int      x, const INTEGER& y){ return (y==x) ;}

inline bool operator != (const INTEGER& x, const INTEGER& y){ return !(x==y) ;} 
inline bool operator != (const INTEGER& x, const int      y){ return !(x==y) ;}
inline bool operator != (const int      x, const INTEGER& y){ return !(x==y) ;}

//*****************************************************************************
// Function swrite
// output function for integer 
// 1. argument: INTEGER
// 2. argument: int integer : width
//*****************************************************************************

inline std::string swrite(const INTEGER& x, const int w){

	 /*		ALT
	 char* erg= MP_int_swritee(x.value,w);
     std::string result=erg;
     free(erg);
     return reeult;
	 */

	//WIDTH kann man bei fmpz nicht angeben. Dafür aber Basis.
	//NULL ist Ausgabetring, speichert ja in erg (kann man sich aussuchen), w ist die Basis!, x.value ist auszugeben.
	char* erg= fmpz_get_str(NULL, w, x.value);
	std::string result=erg;
    free(erg);
    return result;
}

//*****************************************************************************
// Function swrite
// output function for integer 
// 1. argument: INTEGER
//*****************************************************************************

inline std::string swrite(const INTEGER& x){
	char* erg= fmpz_get_str(NULL, 10, x.value);			//funktioniert ohne width ohne probleme
	std::string result=erg;
    free(erg);
    return result;
}

// Conversion from INTEGER to smaller types
inline INTEGER::operator int() const { return fmpz_get_si(value); }		//macht aus dem value ein slong glaube ich. Geht das? Aber undefined wenn es nicht passt.
} /* ! namespace iRRAM */

#endif /* ! iRRAM_INTEGER_H */