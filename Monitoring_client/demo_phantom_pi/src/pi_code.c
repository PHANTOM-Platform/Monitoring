#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <gmp.h>
#include "pi_code.h"

unsigned long roundDiv(unsigned long q, unsigned long r) {
	unsigned long ret, doubleMod;
	if (q<r)
		return 0;
	ret = q / r;
	doubleMod = 2*(q % r);
	if (doubleMod >= r)
		return (ret+1);
	else
		return ret;
}

inline void mpz_fact(mpz_t ret, mpz_t n) {
	int cmp;
	mpz_t result, m;
	mpz_init(result);
	mpz_set_ui(result, 1);
	if (mpz_cmp_ui(n, 2u) < 0) {
		mpz_set(ret, result);
		mpz_clear(result);
		return;
	}
	mpz_init_set(m, n);
	while (mpz_sgn(m)) {
		mpz_mul(result, result, m);
		mpz_sub_ui(m, m, 1);
	}
	mpz_set(ret, result);
	mpz_clear(result);
	mpz_clear(m);
}

void chudnovsky(mpf_t rop, unsigned long precision, unsigned long iterStart, unsigned long iterFinish) {
	unsigned long k1;
	mpz_t k2;
	mpf_t sum, fraction;
	mpz_t numerator, denominator_part1;
	mpf_t numerator_f, denominator_part1_f;
	mpz_t tokenNeg1_powK, token1359, token5451, token6403_pow3, token6403;
	mpf_t token6403_NegSqrt;
	k1 = iterStart;
	mpz_init_set_ui(k2, iterStart);
	mpf_init2(sum, precision);
	mpf_init2(fraction, precision);
	mpz_init(numerator);
	mpf_init2(numerator_f, precision);
	mpz_init(denominator_part1);
	mpf_init2(denominator_part1_f, precision);
	mpz_init_set_ui(tokenNeg1_powK, 1u);
	mpz_init_set_ui(token1359, 13591409u);
	mpz_init_set_ui(token5451, 545140134u);
	mpz_init_set_str(token6403_pow3, "262537412640768000", 10);
	mpz_init_set_ui(token6403, 640320u);
	mpf_init2(token6403_NegSqrt, precision);
		mpf_set_ui(token6403_NegSqrt, 640320u);
		mpf_sqrt(token6403_NegSqrt, token6403_NegSqrt);
		mpf_ui_div(token6403_NegSqrt, 1u, token6403_NegSqrt);
	mpz_t k_mul6_fact, k_mul3_fact, k_fact_cube;
	mpz_init(k_mul6_fact);
	mpz_init(k_mul3_fact);
	mpz_init(k_fact_cube);
	while (k1<iterFinish) {
		mpz_mul_ui(k_mul6_fact, k2, 6u);
		mpz_fact(k_mul6_fact, k_mul6_fact);
		mpz_mul_ui(k_mul3_fact, k2, 3u);
		mpz_fact(k_mul3_fact, k_mul3_fact);
		mpz_fact(k_fact_cube, k2);
		mpz_pow_ui(k_fact_cube, k_fact_cube, 3u);
		// numerador
		mpz_mul(numerator, token5451, k2);
		mpz_add(numerator, numerator, token1359);
		mpz_mul(numerator, numerator, k_mul6_fact);
		mpz_mul(numerator, numerator, tokenNeg1_powK);
		// denominador
		mpz_pow_ui(denominator_part1, token6403_pow3, k1);
		mpz_mul(denominator_part1, denominator_part1, token6403);
		mpz_mul(denominator_part1, denominator_part1, k_fact_cube);
		mpz_mul(denominator_part1, denominator_part1, k_mul3_fact);
		// fraction
		mpf_set_z(numerator_f, numerator);
		mpf_set_z(denominator_part1_f, denominator_part1);
		mpf_div(fraction, numerator_f, denominator_part1_f);
		mpf_mul(fraction, fraction, token6403_NegSqrt);
		// addition
		mpf_add(sum, sum, fraction);
		k1++;
		mpz_add_ui(k2, k2, 1u);
		mpz_neg(tokenNeg1_powK, tokenNeg1_powK);
	}
	// multiplicar por 12
	mpf_mul_ui(sum, sum, 12u);
	mpf_set(rop, sum);
	mpz_clear(k_mul6_fact);
	mpz_clear(k_mul3_fact);
	mpz_clear(k_fact_cube);
	mpz_clear(numerator);
	mpz_clear(denominator_part1);
	mpz_clear(tokenNeg1_powK);
	mpz_clear(token1359);
	mpz_clear(token5451);
	mpz_clear(token6403_pow3);
	mpz_clear(token6403);
	mpz_clear(k2);
	mpf_clear(sum);
	mpf_clear(fraction);
	mpf_clear(numerator_f);
	mpf_clear(denominator_part1_f);
	mpf_clear(token6403_NegSqrt);
}
