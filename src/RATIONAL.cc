/*

RATIONAL.cc -- routines for RATIONAL data type
 
Copyright (C) 2001-2003 Norbert Mueller, Tom van Diessen
Copyright     2016      Franz Brausse
 
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


/*
Changelog: (initial version by Tom, all modifications by Norbert and Franz)

  2016-11-28  Remove explicit casts from INTEGER (use constructor instead)
              Allow for copy-elision / moves
              Optimize operator+=(RATIONAL,int) and operator-(RATIONAL)

  2003-08-29  Addition of functions x+i, x-i, x*i, x/i and i+x etc
              for i of simple type "int"

  2001-07-19  Initial version from Tom's diploma thesis

*/

#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <iRRAM/RATIONAL.h>
#include <iRRAM/INTEGER.h>

#include <flint/fmpz.h>
#include <flint/fmpq.h>

namespace iRRAM {

//****************************************************************************************
// CONSTRUCTORS
// IMPORTANT: Objects MUST be initialized before being used.
//****************************************************************************************

//****************************************************************************************
// Constructing RATIONAL from scratch
//****************************************************************************************

RATIONAL::RATIONAL(const INTEGER& i,const INTEGER& j){			//Ich gehe davon aus, dass hier i der Zähler ist.
  fmpq_init(value);
  fmpq_set_fmpz_frac(fmpq_t value , i.value, j.value)			//value = i / j
}

RATIONAL::RATIONAL(int i, int j){
  fmpq_init(value);
  if ( j >= 0) {
	mpq_set_si(value, i, (ulong)(j))							//cast war vorher mit (unsigned int)
	} else {
	INTEGER ii(i);
	INTEGER jj(j);
	fmpq_set_fmpz_frac(fmpq_t value , ii.value, jj.value)
	}
}

RATIONAL::RATIONAL(const INTEGER& i){							//ich vermute mal, dass hier einfach Zähler i und Nenner 1 sind.
  fmpq_init(value);
  fmpz_t one;
  fmpz_init(one);
  fmpz_one(one);
  fmpq_set_fmpz_frac(fmpq_t value , i.value, one)
}

RATIONAL::RATIONAL(int i) {
  fmpq_init(value);
  fmpq_set_si(value, i, 1);										//erzeugt fmpq mit Wert i
}

RATIONAL::RATIONAL(const RATIONAL& y){
  fmpq_init(value);
  fmpq_set(value, y.value);										//erzeugt Kopie
}


//****************************************************************************************
// Constructing RATIONAL from double, the result is NOT rounded
//****************************************************************************************

RATIONAL::RATIONAL(double d){			//finde ich nicht
  MP_rat_init(value);
  MP_double_to_RATIONAL(d,value);
}

//****************************************************************************************
// Constructing RATIONAL from string, the string must be in decimal base
//****************************************************************************************

RATIONAL::RATIONAL(const char* s){		//finde ich nicht
  MP_rat_init(value);
  MP_string_to_RATIONAL(s,value);
}

//******************************************************************************
// Copy assignment
// left side (this) is already initialized
//******************************************************************************

RATIONAL & RATIONAL::operator=(RATIONAL y)			//kein Plan, was std::swap macht aber ändert wohl nichts
{
	using std::swap;
	swap(value, y.value);
	return *this;
}

RATIONAL& RATIONAL::operator = (int y){				//setzt value auf fmpq mit wert i
  fmpq_set_si(value, i, 1);
  return (*this);
}

//**************************************************************/
// Destructor
// Destroying the object and freeing space occupied by it
//******************************************************************/

RATIONAL::~RATIONAL(){
  if (value) fmpq_clear(value);						//fmpq_clear
}

//****************************************************************************************
// Returns the sign of RATIONAL objects (-1/0/1): 
// sign(i)= -1, iff i<0,   sign(i)=1 iff i>0,  and sign(0)=0
//****************************************************************************************
/*! \ingroup maths */
int sign(const RATIONAL& x)
{
  return fmpq_sgn(x.value);							//fmpq_sgn
}

//****************************************************************************************
// Shift
// defines shifting function if none exists in the backend
//****************************************************************************************

RATIONAL scale(RATIONAL x, int n)
{
	if(n >= 0){
		fmpq_mul_2exp(x.value,x.value,n);
	}else{
		n = -n;
		fmpq_div_2exp(x.value,x.value,n);
	}
}


//*********************************************************************/
// Addition
//**********************************************************************/

RATIONAL& operator += (RATIONAL& x, const RATIONAL& y)
{
  fmpq_add(x.value,x.value,y.value);					//ganz normal x = x + y
  return x;
}

RATIONAL & operator+=(RATIONAL &x, int y)
{
	fmpq_add_si(x.value, x.value, y);					//was war das inplace?
	return x;
}

//******************************************************************************
// Subtraction
//******************************************************************************

RATIONAL operator-(int x, RATIONAL y)
{
	fmpq_neg(y.value, y.value);
	y += x;
	return y;
}

RATIONAL& operator -= (RATIONAL& x, const RATIONAL& y){
  fmpq_sub(x.value,x.value,y.value);								//x = x - y
  return x;
}

//******************************************************************************
// Negation
//******************************************************************************

RATIONAL operator-(RATIONAL x)
{
	fmpq_neg(x.value, x.value);										//fmpq neg
	return x;
}

//******************************************************************************
// Multiplication
//******************************************************************************

RATIONAL& operator *= (RATIONAL& x, const RATIONAL& y){
  fmpq_mul(x.value,x.value,y.value);							//x = x * y
  return x;
}

RATIONAL & operator*=(RATIONAL &x, const int n)					//es gibt kein mul_si also workaround mit mul_fmpz
{
	fmpz_t fac;
	fmpz_init(fac);													
	fmpz_set_si(fac, n);	
	fmpq_mul_fmpz(x.value, x.value, fac);
	return x;
}

//******************************************************************************
// Division
//******************************************************************************

RATIONAL operator / (int x, RATIONAL y)							//Erstellen eines fmpq aus x wie im Konstruktor
{
  fmpq_init(denominator);
  fmpq_set_si(denominator, x, 1);
  fmpq_div(y.value,denominator,y.value);
  return y;
}

RATIONAL& operator /= (RATIONAL& x, const RATIONAL& y){
  fmpq_div(x.value,x.value,y.value);
  return x;
}

RATIONAL& operator /= (RATIONAL& x, const int n)				//es gibt kein div_si also workaround mit div_fmpz
{
  fmpz_t q;
  fmpz_init(q);													
  fmpz_set_si(q, y);
  fmpq_div_fmpz(y.value,y.value,q);
  return y;
}


//****************************************************************************************
// Absolute value: |x|
//****************************************************************************************

/*! \ingroup maths */
RATIONAL abs(RATIONAL x)
{
	fmpq_abs(x.value,x.value);									//check
	return x;
}

INTEGER denominator (const RATIONAL& x){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  zvalue = x.value.num;											//fmpq ist struct aus fmpz_t namens num und den
  return { zvalue, INTEGER::move_t{} };
}

INTEGER numerator (const RATIONAL& x){
  fmpz_t zvalue;
  fmpz_init(zvalue);
  zvalue = x.value.den;											//fmpq ist struct aus fmpz_t namens num und den
  return { zvalue, INTEGER::move_t{} };
}

RATIONAL power(RATIONAL x, slong n)								// nie ist etwas unsigned, außer wenn flint signed benutzt ._."
{
	fmpq_pow_si(x.value, x.value, n);
	return x;
}

//****************************************************************************************
// Comparators
// returns boolean value 1 if x<y, 0 otherwise
//****************************************************************************************

bool operator < (const RATIONAL& x, const RATIONAL& y){
  return (fmpq_cmp(x.value,y.value) < 0 );						//fein
}

//****************************************************************************************
// Equal to: x == y
// returns boolean value 1 if x==y, 0 otherwise
//****************************************************************************************

bool operator == (const RATIONAL& x, const RATIONAL& y){
  return (fmpq_equal(x.value,y.value));							//fein
}


//*****************************************************************************
// Function swrite
// string function for RATIONAL
// writes rational into string
//*****************************************************************************
/*
std::string swrite(const RATIONAL& x, const int w){
     char* erg= MP_rat_swritee(x.value,w);
     std::string result=erg;
     free(erg);
     return result;
}
*/
//****************************************************************************************
// swrite: writes RATIONAL to string
// 1. argument: RATIONAL
//****************************************************************************************
/*
std::string swrite(const RATIONAL& x){
     char* erg= MP_rat_sprintf(x.value);
     std::string result=erg;
     free(erg);
     return result;  
}
*/
} // namespace iRRAM
