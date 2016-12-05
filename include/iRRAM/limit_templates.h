/*

iRRAM_limit_templates.h -- template definitions file for the limit operators of the iRRAM library
 
Copyright (C) 2005 Norbert Mueller
Copyright     2016 Franz Brausse
 
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
#ifndef iRRAM_LIMIT_TEMPLATES_H
#define iRRAM_LIMIT_TEMPLATES_H

#include <iRRAM/core.h>

/* Limit operations
 * ~~~~~~~~~~~~~~~~
 * These functions compute the mathematical limit y of sequences f_p of
 * continuous objects, optionally depending on discrete or continuous arguments
 * x satisfying
 *
 *     forall integers p : |f_p(x)-y| < 2**p
 *
 * in the iRRAM limit for x, f_p and y. In the first case, x is discrete and
 * therefore exact, the errors introduced stem from imprecision of the
 * computation f_p itself, which is unbounded. Since f_p is executed within
 * iRRAM, its mathematical operations are exact within 2**q, where q is the
 * current working precision, however the objects operated upon are imprecise
 * intervals and therefore f_p introduces additional errors. To reduce their
 * effect, the current working precision q is successively increased, starting
 * at the initial P when limit was invoked (env.saved_prec()), whenever f_q
 * could not be computed. If at some point the computation f_q succeeds, it
 * has computed y up to 2**q < 2**p. The error 2**p is added to y's error to
 * ensure correctness.
 *
 * If x is not exact, |f_p(x)-y| generally is larger than 2**p. This additional
 * error cannot get arbitrarily small when increasing p, therefore a different
 * scheme is employed: when computing f_P(x) failed and also f_{p_0}(x) failed
 * (where p_0 is the lowest precision available in iRRAM), the limit cannot be
 * computed with the current precision of x and therefore a reiteration is done.
 * If f_P(x) succeeded but the resulting error + 2**P is larger than 2**P[-1]
 * and is larger than x's error - 2**P[-1], and p is set to p_0 for the
 * potential next iteration. If y's error already is smaller than 2**P, the
 * iteration succeeded. Otherwise the result is deemed too imprecise and the
 * computation is repeated with p increased by 4 precision steps.
 *
 * Note
 * ~~~~
 * There fundamentally are two different variants of limit(), one taking *only*
 * parameters representing discrete values and one for the mixed/only-continuous
 * case. Continuity of a type is determined through specializations of the type
 * trait
 *
 *     template <typename T> struct is_continuous;
 *
 * providing a static `bool value` as defined in <iRRAM/lib.h>.
 *
 * Which one is called when invoking `limit(f,args...)` is not determined by the
 * actual parameter types of f but by the types of args...
 * This is deliberate and consistent with how iRRAM works - i.e. allowing
 * implicit conversions from the types of smaller domain to those with larger
 * domain. Usually there is no need to explicitely specify the template types.
 *
 * Example:
 *
 *     REAL f(int prec, const REAL &x);
 *     REAL y = limit(f, 42)
 *
 * will *not* invoke the continuous variant since a REAL will be constructed
 * from the given int using REAL's implicit converting constructor while limit()
 * is being executed. That REAL's precision therefore is controlled by limit()
 * and being adjusted as to f's and limit's requirements to yield a precise
 * enough result.
 *
 * This behaviour is desirable as the non-continuous parameter version of
 * limit() is more efficient.
 *
 * Regarding the same example but calling
 *
 *     REAL x = 42; // or a complicated computation
 *     REAL y = limit(f, x);
 *
 * explains the necessity for the second version. It will use the continuous
 * version as the precision of the result of f(p,x) typically will depend on the
 * precision of x and not just on p. Therefore the continuous variant of limit()
 * provisions for reiterations outside of the limit() call to enable increased
 * precision of x itself.
 *
 * For a continuous parameter pack T (as determined by `any_continuous<T...>`) a
 * function
 *
 *     int geterror_exp(const T &...)
 *
 * needs to be provided which defaults to returning the maximum error exponent
 * as determined by
 *
 *     int geterror_exp(const Ti &t)
 *
 * the default implementation of which is
 *
 *     return geterror(t).exponent;
 *
 * but may be specialized according to the type's characteristics. In the future
 * it is possible that the continuous variant of limit() will use more
 * fine-grained error information than just the exponent, so it may be helpful
 * to additionally provide
 *
 *     sizetype geterror(const Ti &)
 *
 * for all continuous types Ti used as limit() parameters. Additionally,
 * declarations of
 *
 *     void     seterror(R &, sizetype)
 *     sizetype geterror(const R &)
 *
 * for result type R must be provided for both of the limit() variants. For
 * iRRAM's types REAL, *REALMATRIX and COMPLEX implementations are available.
 */

namespace iRRAM {

inline void limit_debug(const char* c){
  if ( iRRAM_unlikely(state.debug > 0) ){
    if (state.debug >=state.ACTUAL_STACK.inlimit + 2 )
		cerr << c <<"...\n";
    if (state.max_prec <= state.ACTUAL_STACK.prec_step) 
		state.max_prec  = state.ACTUAL_STACK.prec_step;
  }
}

inline void limit_debug2(const char* c){
      if ( iRRAM_unlikely(state.debug > 0) ) {
	    if (state.debug >=state.ACTUAL_STACK.inlimit + 2 )
		cerr << c <<", increasing precision locally to "
			<<state.ACTUAL_STACK.actual_prec<<"\n";
	    if (state.max_prec <= state.ACTUAL_STACK.prec_step) 
		state.max_prec  = state.ACTUAL_STACK.prec_step;
      }
}

template <typename C,typename... ContArgs>
int      geterror_exp(const C &, const ContArgs &...);

template <typename S>
int geterror_exp(const S &x)
{
	return geterror(x).exponent;
}

template <typename S,typename T,typename... ContArgs>
typename std::enable_if<is_continuous<S>::value &&
                        any_continuous<T,ContArgs...>::value,int>::type
geterror_exp(const S &x, const T &y, const ContArgs &... z)
{
	return max(geterror_exp(x), geterror_exp(y, z...));
}

template <typename S,typename T,typename... ContArgs>
typename std::enable_if<is_continuous<S>::value &&
                        !any_continuous<T,ContArgs...>::value,int>::type
geterror_exp(const S &x, const T &, const ContArgs &...)
{
	return geterror_exp(x);
}

template <typename S,typename T,typename... ContArgs>
typename std::enable_if<!is_continuous<S>::value,int>::type
geterror_exp(const S &, const T &y, const ContArgs &... z)
{
	static_assert(any_continuous<T,ContArgs...>::value,
	              "geterror_exp() is only applicable to continuous types");
	return geterror_exp(y, z...);
}

inline sizetype geterror(const REAL &r) { return r.geterror(); }
inline void     seterror(REAL &r, const sizetype &err) { r.seterror(err); }

inline sizetype geterror(const COMPLEX &r) { return r.geterror(); }
inline void     seterror(COMPLEX &r, const sizetype &err) { r.seterror(err); }

inline sizetype geterror(const REALMATRIX &r) { return r.geterror(); }
inline void     seterror(REALMATRIX &r, const sizetype &err) { r.seterror(err); }

inline sizetype geterror(const SPARSEREALMATRIX &r) { return r.geterror(); }
inline void     seterror(SPARSEREALMATRIX &r, const sizetype &err) { r.seterror(err); }

/* F must be the signature
 *    Result (int prec, const C &, const ContArgs &...)
 * Requires for Result and ContArgs functions
 *  - int geterror_exp(const C &, const ContArgs &...)
 *    (usually it's enough to specialize the single-argument versions of
 *     geterror_exp(const C &) and geterror_exp(const ContArgs &)... or to
 *     overload geterror(const C &) and geterror(const ContArgs &)...;
 *     then the maximum-norm is used on the vector of all continuous parameters)
 *  - sizetype geterror(const Result &)
 *  - void seterror(Result &, const sizetype &)
 */
template <typename F,typename... ContArgs>
auto limit(F f, const ContArgs &... cont_args)
-> typename std::enable_if<any_continuous<ContArgs...>::value,
                           decltype(f(0,cont_args...))>::type
{
	using Result = decltype(f(0,cont_args...));

	limit_computation env;

	Result lim, limnew;
	sizetype limnew_error, lim_error;
	int args_error_exp;

	int element      = env.saved_prec();
	int element_step = env.saved_step();
	int firsttime    = 2;

	args_error_exp = geterror_exp(cont_args...);

	limit_debug("starting general limit_gen1");

	while (1) {
		try {
			iRRAM_DEBUG2(2,"trying to compute general limit_gen1 "
			               "with precicion 2^(%d)...\n", element);
			limnew = f(element,cont_args...);
			limnew_error = sizetype_add_power2(geterror(limnew), element);
			if (firsttime == 2)
				if (limnew_error.exponent > env.saved_prec(-1)
				    && limnew_error.exponent > args_error_exp - env.saved_prec(-1)) {
					iRRAM_DEBUG2(2,"computation not precise enough (%d*2^%d), "
						"trying normal p-sequence\n",
						limnew_error.mantissa,
						limnew_error.exponent);
					element_step = 1;
					element = 4+iRRAM_prec_array[element_step];
					firsttime = 1;
				}
			if (firsttime != 0 ||
			    sizetype_less(limnew_error,lim_error)) {
				lim = limnew;
				lim_error = limnew_error;
				iRRAM_DEBUG2(2,"getting result with error %d*2^(%d)\n",
				               lim_error.mantissa, lim_error.exponent);
			} else {
				iRRAM_DEBUG1(2,"computation successful, but no improvement\n");
			}
			firsttime = 0;
			if (element <= env.saved_prec())
				break;
			element_step += 4;
			element = iRRAM_prec_array[element_step];
		} catch (const Iteration &it) {
			if (firsttime == 0) {
				iRRAM_DEBUG1(2,"computation failed, using best success\n");
				break;
			} else if (firsttime == 2) {
				iRRAM_DEBUG1(2,"computation failed, trying normal p-sequence\n");
				element_step = 1;
				element = 4+iRRAM_prec_array[element_step];
				firsttime = 1;
			} else {
				iRRAM_DEBUG1(1,"computation of general limit_gen1 failed totally\n");
				REITERATE(0);
			}
		}
	}
	seterror(lim, lim_error);
	iRRAM_DEBUG2(2,"end of general limit_gen1 with error %d*2^(%d)\n",
	             lim_error.mantissa, lim_error.exponent);
	return lim;
}

template <typename F,typename... DiscArgs>
auto limit(F f, const DiscArgs &... disc_args)
-> typename std::enable_if<!any_continuous<DiscArgs...>::value,
                           decltype(f(0,disc_args...))>::type
{
	using Result = decltype(f(0,disc_args...));

	limit_computation env;

	Result lim;
	sizetype lim_error;

	limit_debug("starting general limit_0");

	while (1) {
		try {
			iRRAM_DEBUG2(2,"trying to compute general limit_0 with precision %d...\n",state.ACTUAL_STACK.actual_prec);
			lim = f(env.saved_prec(), disc_args...);
			lim_error = geterror(lim);
			iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n", lim_error.mantissa, lim_error.exponent);
			break;
		} catch (const Iteration &it) {
			env.inc_step(2);
			limit_debug2("general limit_0 failed");
		}
	}
	lim_error = sizetype_add_power2(lim_error, env.saved_prec());
	seterror(lim, lim_error);
	iRRAM_DEBUG2(2,"end of limit_0 with error %d*2^(%d)\n",
	               lim_error.mantissa,lim_error.exponent);
	return lim;
}

template <class ARGUMENT, class RESULT>
RESULT limit_mv (RESULT (*f)(int prec,
                             int* choice,
                             const ARGUMENT&),
                 const ARGUMENT& x)
{
  bool inlimit = state.ACTUAL_STACK.inlimit != 0;

  limit_computation env;

  RESULT lim,limnew;
  sizetype limnew_error,element_error;
  int choice=0;
  sizetype lim_error,x_error;

  int element=env.saved_prec();
  int element_step=env.saved_step();
  int firsttime=2;

  if (!inlimit && !state.thread_data_address->cache_i.get(choice))
    state.thread_data_address->cache_i.put(choice);

  x.geterror(x_error);

  limit_debug("starting limit_mv");

  while (1) {
   try {
    iRRAM_DEBUG2(2,"trying to compute limit_mv with precicion 2^(%d)...\n",element);
    limnew=f(element,&choice,x);
    if (!inlimit)
      state.thread_data_address->cache_i.modify(choice);
    element_error = sizetype_power2(element);
    limnew.geterror(limnew_error);
    limnew_error += element_error;
    if (firsttime ==2 ) if ( limnew_error.exponent > env.saved_prec(-1)
    	&&  limnew_error.exponent > x_error.exponent -env.saved_prec(-1)) {
    iRRAM_DEBUG0(2,{fprintf(stderr,"computation not precise enough (%d*2^%d), trying normal p-sequence\n",
                   limnew_error.mantissa,limnew_error.exponent);});
       element_step=1;
       element=4+iRRAM_prec_array[element_step];
       firsttime=1;
    }}
     catch ( Iteration it) {
      if ( firsttime==0) {
      iRRAM_DEBUG1(2,"computation failed, using best success\n");
      break;
      } else
      if ( firsttime==2) {
      iRRAM_DEBUG1(2,"computation failed, trying normal p-sequence\n");
      element_step=1;
      element=4+iRRAM_prec_array[element_step];
      firsttime=1;
      } else {
      iRRAM_DEBUG1(1,"computation of limit_gen1 failed totally\n");
      REITERATE(0);
      }}
   if ( firsttime != 0 || sizetype_less(limnew_error,lim_error) ) {
      lim=limnew;
      lim_error=limnew_error;
      iRRAM_DEBUG2(2,"getting result with error %d*2^(%d)\n",
               lim_error.mantissa, lim_error.exponent);
      } else {
      iRRAM_DEBUG1(2,"computation successful, but no improvement\n");
      }
    firsttime=0;
    if (element<=env.saved_prec())break;
    element_step+=4;
    element=iRRAM_prec_array[element_step];
    }
  lim.seterror(lim_error);
  iRRAM_DEBUG0(2,{fprintf(stderr,"end of limit_mv with error %d*2^(%d)\n",
                   lim_error.mantissa,lim_error.exponent);});
  return lim;
}

template <class ARGUMENT, class DISCRETE, class RESULT>
RESULT  limit_lip (RESULT  (*f)(int,const ARGUMENT&,DISCRETE param),
            int lip_value,
	    bool (*on_domain)(const ARGUMENT&),
            const ARGUMENT& x,DISCRETE param)
{
  if ( on_domain(x) != true ) REITERATE(0);

  limit_computation env;

  ARGUMENT x_new;
  RESULT lim;
  sizetype lim_error,x_error;

  x_new=x;
  x_new.geterror(x_error);
  sizetype_exact(lim_error);
  x_new.seterror(lim_error);

  limit_debug("starting limit_lip");

  while (1) {
     try{
      iRRAM_DEBUG2(2,"trying to compute limit_lip with precision %d...\n",state.ACTUAL_STACK.actual_prec);
    lim=f(env.saved_prec(),x_new,param);
    lim.geterror(lim_error);
    if (lim_error.exponent > env.saved_prec()) {
      env.inc_step(2);
      limit_debug2("limit_lip too imprecise");

   } else {
      iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lim_error.mantissa, lim_error.exponent);
      break;
    }}
    catch ( Iteration it){
      env.inc_step(2);
      limit_debug2("limit_lip failed");
    } }
  lim_error = sizetype_power2(env.saved_prec());
  lim.adderror(lim_error);
  lim_error = x_error << lip_value;
  lim.adderror(lim_error);
  iRRAM_DEBUG0(2,{lim.geterror(lim_error);
            fprintf(stderr,"end of limit_lip with error %d*2^(%d)\n",
              lim_error.mantissa,lim_error.exponent);
            fprintf(stderr,"  error of argument: %d*2^(%d)\n",
              x_error.mantissa,x_error.exponent);});
  return lim;
}




// template <class ARGUMENT, class PARAM, class RESULT>
// RESULT lipschitz (RESULT f(const PARAM& param,const ARGUMENT& ),
//             REAL lip_f(const PARAM& param, const ARGUMENT&),
// 	    bool on_domain(const PARAM& param, const ARGUMENT&),
//             const PARAM& param, const ARGUMENT& x)
// {
//   if ( on_domain(param,x) != true ) REITERATE(0);
// 
//   ITERATION_STACK SAVED_STACK;
// 
//   single_valued code;
//   iRRAM_DEBUG1(2,"starting lipschitz1b ...\n");
// 
// // for the computation of the Lipschitz bound, we work with
// // reduced precision:
//   ACTUAL_STACK.prec_step=(ACTUAL_STACK.prec_step+1)/2;
//   ACTUAL_STACK.actual_prec=iRRAM_prec_array[ACTUAL_STACK.prec_step];
//   iRRAM_highlevel = (ACTUAL_STACK.prec_step > 1);
// 
//   lip_bound=lip_f(param,x);
// 
//   ACTUAL_STACK.prec_step=SAVED_STACK.prec_step;
//   ACTUAL_STACK.actual_prec=iRRAM_prec_array[ACTUAL_STACK.prec_step];
//   iRRAM_highlevel = (ACTUAL_STACK.prec_step > 1);
// 
//   ARGUMENT x_new;
//   RESULT lip_result,lip_bound;
//   sizetype lip_error,lip_size,tmp_size,x_error;
// 
//   bool try_it=true;
//   x_new=x;
//   x_new.geterror(x_error);
//   sizetype_exact(x_new.error);
// 
// 
//   while (try_it) {
//   try { try_it=false;
//         lip_result=f(param,x_new); }
//   catch ( Iteration it)  { try_it=true;
//       ACTUAL_STACK.prec_step+=2;
//       ACTUAL_STACK.actual_prec=iRRAM_prec_array[ACTUAL_STACK.prec_step];
//       iRRAM_highlevel = (ACTUAL_STACK.prec_step > 1);
//       iRRAM_DEBUG2(2,"limit_lip2 failed, increasing precision locally to %d...\n",ACTUAL_STACK.actual_prec);
//     }
//   }
// 
//   iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
//              lip_result.error.mantissa, lip_result.error.exponent);
//   lip_size=lip_bound.vsize;
//   lip_bound.geterror(tmp_size);
//   lip_size += tmp_size;
//   lip_error = lip_size * x_error;
//   lip_result.adderror(lip_error);
//   iRRAM_DEBUG0(2,{lip_result.geterror(lip_error);
//             fprintf(stderr,"end of lipschitz_1b with error %d*2^(%d)\n",
//               lip_error.mantissa,lip_error.exponent);
//             fprintf(stderr,"  for argument with error %d*2^(%d)\n",
//               x_error.mantissa,x_error.exponent);});
//   return lip_result;
// }

template <class CONT_ARGUMENT, class DISCRETE_ARGUMENT, class RESULT, class PARAM>
RESULT lipschitz_1p_1a (RESULT (*f)(const DISCRETE_ARGUMENT&, const PARAM& param),
            int lip,
            bool (*on_domain)(const CONT_ARGUMENT&, const PARAM& param),
            const CONT_ARGUMENT& x, const PARAM& param )
{
  if ( on_domain(x,param) != true ) REITERATE(0);

  limit_computation env(0);
  iRRAM_DEBUG1(2,"starting lipschitz_1p_1a ...\n");

  DISCRETE_ARGUMENT x_center;
  RESULT lip_result;
  sizetype lip_error,x_error;

  bool try_it=true;
  x.to_formal_ball(x_center,x_error);

  while (try_it) {
    try { try_it=false;
        lip_result=f(x_center,param); }
    catch ( Iteration it)  { try_it=true;
      env.inc_step(2);
      iRRAM_DEBUG2(2,"lipschitz_1p_1a failed, increasing precision locally to step %d...\n",state.ACTUAL_STACK.prec_step);
    }
  }

  iRRAM_DEBUG2(2,"getting result with local error %d*2^(%d)\n",
             lip_result.error.mantissa, lip_result.error.exponent);
  lip_error = x_error << lip;
  lip_result.adderror(lip_error);
  iRRAM_DEBUG0(2,{lip_result.geterror(lip_error);
            fprintf(stderr,"end of lipschitz_1p_1a with error %d*2^(%d)\n",
              lip_error.mantissa,lip_error.exponent);
            fprintf(stderr,"  for argument with error %d*2^(%d)\n",
              x_error.mantissa,x_error.exponent);});
  return lip_result;
}


} /* ! namespace iRRAM */


#endif /* ! iRRAM_LIMIT_TEMPLATES_H */
