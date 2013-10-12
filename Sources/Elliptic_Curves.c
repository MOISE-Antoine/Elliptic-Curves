/** @file Elliptic_Curves.c
 * Basic operations for Weierstrass elliptic curves.
 */
#include <stdio.h>
#include <gmp.h>
#include "Elliptic_Curves.h"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
/** Add two points.
 * @param Pointer_Curve Elliptic curve.
 * @param Pointer_Point_P First point to add.
 * @param Pointer_Point_Q Second point to add.
 * @param Lambda Precalculated lambda.
 * @param Pointer_Output_Point Result.
 */
static inline void ECAdd(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, TPoint *Pointer_Point_Q, mpz_t Lambda, TPoint *Pointer_Output_Point)
{
	mpz_t Temp;
	
	mpz_init(Temp);
	
	// Compute xr
	mpz_mul(Pointer_Output_Point->X, Lambda, Lambda); // lambda^2
	mpz_sub(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Point_P->X); // lambda^2 - xp
	mpz_sub(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Point_Q->X); // lambda^2 - xp - xq
	mpz_mod(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Curve->p);
	
	// Compute yr
	mpz_neg(Temp, Lambda); // -lambda
	mpz_mul(Temp, Temp, Pointer_Output_Point->X); // -lambda * xr
	mpz_mul(Pointer_Output_Point->Y, Lambda, Pointer_Point_P->X); // lambda * xp
	mpz_add(Pointer_Output_Point->Y, Temp, Pointer_Output_Point->Y); // (-lambda * xr) + (lambda * xp)
	mpz_sub(Pointer_Output_Point->Y, Pointer_Output_Point->Y, Pointer_Point_P->Y); // (-lambda * xr) + (lambda * xp) - yp
	mpz_mod(Pointer_Output_Point->Y, Pointer_Output_Point->Y, Pointer_Curve->p);
	
	mpz_clear(Temp);
}

/** Compute the double of a point.
 * @param Pointer_Curve The elliptic curve.
 * @param Pointer_Point_P The point to double.
 * @param Pointer_Output_Point Result.
 */
static void ECDoubling(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, TPoint *Pointer_Output_Point)
{
	mpz_t Lambda, Temp;
	
	mpz_init(Lambda);
	mpz_init(Temp);
	
	// Compute lambda
	// Compute numerator
	mpz_mul(Lambda, Pointer_Point_P->X, Pointer_Point_P->X); // xp^2
	mpz_mul_ui(Lambda, Lambda, 3); // 3*xp^2
	mpz_add(Lambda, Lambda, Pointer_Curve->a4); // 3*xp^2 + a4
	
	// Compute denominator
	mpz_mul_ui(Temp, Pointer_Point_P->Y, 2);
	
	mpz_fdiv_q(Lambda, Lambda, Temp);
	
	// Compute remainder to stay on Fp
	mpz_mod(Lambda, Lambda, Pointer_Curve->p);
	
	// Double the point
	ECAdd(Pointer_Curve, Pointer_Point_P, Pointer_Point_P, Lambda, Pointer_Output_Point);
	
	#ifdef DEBUG
		gmp_printf("[ECDoubling] Lambda = %Zd\n", Lambda);
		gmp_printf("[ECDoubling] Xr = %Zd\n", Pointer_Output_Point->X);
		gmp_printf("[ECDoubling] Yr = %Zd\n", Pointer_Output_Point->Y);
	#endif
	
	mpz_clear(Lambda);
	mpz_clear(Temp);
}

/** Add two different points.
 * @param Pointer_Curve The ellipitic curve.
 * @param Pointer_Point_P First point.
 * @param Pointer_Point_Q Second point.
 * @param Pointer_Output_Point Result.
 */
static void ECAddDifferentPoints(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, TPoint *Pointer_Point_Q, TPoint *Pointer_Output_Point)
{
	mpz_t Lambda, Temp;
	
	mpz_init(Lambda);
	mpz_init(Temp);
	
	// Compute lambda
	mpz_sub(Lambda, Pointer_Point_P->Y, Pointer_Point_Q->Y); // yp - yq
	mpz_sub(Temp, Pointer_Point_P->X, Pointer_Point_Q->X); // xp - xq
	mpz_fdiv_q(Lambda, Lambda, Temp);
	
	// Compute remainder to stay on Fp
	mpz_mod(Lambda, Lambda, Pointer_Curve->p);
	
	// Add the two points
	ECAdd(Pointer_Curve, Pointer_Point_P, Pointer_Point_Q, Lambda, Pointer_Output_Point);
	
	#ifdef DEBUG
		gmp_printf("[ECAddDifferentPoints] Lambda = %Zd\n", Lambda);
		gmp_printf("[ECAddDifferentPoints] Xr = %Zd\n", Pointer_Output_Point->X);
		gmp_printf("[ECAddDifferentPoints] Yr = %Zd\n", Pointer_Output_Point->Y);
	#endif
	
	mpz_clear(Lambda);
	mpz_clear(Temp);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
void ECLoadFromFile(char *String_Path, TEllipticCurve *Pointer_Curve)
{
	FILE *File;
	
	File = fopen(String_Path, "r");
	assert(File != NULL);
	
	// Initialize curve members
	mpz_init(Pointer_Curve->p);
	mpz_init(Pointer_Curve->n);
	mpz_init(Pointer_Curve->a4);
	mpz_init(Pointer_Curve->a6);
	
	// Load values
	gmp_fscanf(File, "p=%Zd", &Pointer_Curve->p);
	gmp_fscanf(File, "n=%Zd", &Pointer_Curve->n);
	gmp_fscanf(File, "a4=%Zd", &Pointer_Curve->a4);
	gmp_fscanf(File, "a6=%Zd", &Pointer_Curve->a6);
	
	fclose(File);
}

void ECOpposite(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Input_Point, TPoint *Pointer_Output_Point)
{
	mpz_set(Pointer_Output_Point->X, Pointer_Input_Point->X);
	mpz_neg(Pointer_Output_Point->Y, Pointer_Input_Point->Y);
	mpz_mod(Pointer_Output_Point->Y, Pointer_Output_Point->Y, Pointer_Curve->p);
	Pointer_Output_Point->Is_Infinite = Pointer_Input_Point->Is_Infinite;
}

void ECAddition(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, TPoint *Pointer_Point_Q, TPoint *Pointer_Output_Point)
{
	TPoint Point_Temp;
	
	// Is P infinite ?
	if (Pointer_Point_P->Is_Infinite)
	{
		// Result is Q
		PointCopy(Pointer_Output_Point, Pointer_Point_Q);
		#ifdef DEBUG
			printf("[ECAddition] Point P is infinite\n");
		#endif
		return;
	}
	
	// Is Q infinite ?
	if (Pointer_Point_Q->Is_Infinite)
	{
		// Result is P
		PointCopy(Pointer_Output_Point, Pointer_Point_P);
		#ifdef DEBUG
			printf("[ECAddition] Point Q is infinite\n");
		#endif
		return;
	}
	
	// Are P and Q opposite ?
	PointCreate(0, 0, &Point_Temp);
	ECOpposite(Pointer_Curve, Pointer_Point_Q, &Point_Temp);
	if (PointIsEqual(Pointer_Point_P, &Point_Temp))
	{
		// Result is infinite
		Pointer_Output_Point->Is_Infinite = 1;
		PointDelete(&Point_Temp);
		#ifdef DEBUG
			printf("[ECAddition] P = -Q\n");
		#endif
		return;
	}
	
	// Is P equal to Q ?
	if (PointIsEqual(Pointer_Point_P, Pointer_Point_Q)) ECDoubling(Pointer_Curve, Pointer_Point_P, Pointer_Output_Point); // Double the point
	else ECAddDifferentPoints(Pointer_Curve, Pointer_Point_P, Pointer_Point_Q, Pointer_Output_Point); // Add the two different points
	
	Pointer_Output_Point->Is_Infinite = 0;
}

void ECMultiply(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, mpz_t Factor, TPoint *Pointer_Output_Point)
{
	// V 1.0
	
	// Get absolute value of Factor (it is possible because we don't need to keep track of a negative number as we are doing modulus calculations)
	mpz_abs(Factor, Factor);
	
	while (mpz_cmp_ui(Factor, 0) > 0)
	{
		ECAddition(Pointer_Curve, Pointer_Point_P, Pointer_Point_P, Pointer_Output_Point);
		mpz_sub_ui(Factor, Factor, 1);
	}
}

/*void ECAddition(TEllipticCurve *Pointer_Curve, TPoint *Pointer_Point_P, TPoint *Pointer_Point_Q, TPoint *Pointer_Output_Point)
{
	TPoint Output_Point;
	mpz_t Lambda, Temp;
	Point Point_Temp;
	
	// Is P infinite ?
	if (Pointer_Point_P->Is_Infinite)
	{
		// Result is Q
		PointCopy(Pointer_Output_Point, Pointer_Point_Q);
		return;
	}
	
	// Is Q infinite ?
	if (Pointer_Point_Q->Is_Infinite)
	{
		// Result is P
		PointCopy(Pointer_Output_Point, Pointer_Point_P);
		return;
	}
	
	// Are P and Q opposite ?
	PointCreate(&Point_Temp);
	ECOpposite(Point_Temp, Pointer_Point_Q);
	if (PointIsEqual(Pointer_Point_P, Point_Temp))
	{
		Pointer_Output_Point->Is_Infinite = 1;
		// TODO PointFree
		return;
	}
	
	// Compute lambda
	mpz_init(Lambda);
	if (PointIsEqual(Pointer_Point_P, Pointer_Point_Q))
	{
		// Compute numerator
		mpz_mul(Lambda, Pointer_Point_P->X, Pointer_Point_P->X); // xp^2
		mpz_mul_ui(Lambda, Lambda, 3); // 3*xp^2
		mpz_add(Lambda, Lambda, Pointer_Curve->a4); // 3*xp^2 + a4
		
		// Compute denominator
		mpz_mul_ui(Temp, Pointer_Point_P->Y, 2);
		
		mpz_fdiv_q(Lambda, Lambda, Temp);
	}
	else
	{
		mpz_sub(Lambda, Pointer_Point_P->Y, Pointer_Point_Q->Y); // yp - yq
		mpz_sub(Temp, Pointer_Point_P->X, Pointer_Point_Q->X); // xp - xq
		mpz_fdiv_q(Lambda, Lambda, Temp);
	}
	// Compute remainder to stay on Fp
	mpz_mod(Lambda, Lambda, Pointer_Curve->p);

	// Compute xr
	mpz_mul(Pointer_Output_Point->X, Lambda, Lambda); // lambda^2
	mpz_sub(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Point_P->X); // lambda^2 - xp
	mpz_sub(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Point_Q->X); // lambda^2 - xp - xq
	mpz_mod(Pointer_Output_Point->X, Pointer_Output_Point->X, Pointer_Curve->p);
	
	// Compute yr
	mpz_neg(Temp, Lambda); // -lambda
	mpz_mul(Temp, Temp, Pointer_Output_Point->X); // -lambda * xr
	mpz_mul(Pointer_Output_Point->Y, Lambda, Pointer_Point_P->X); // lambda * xp
	mpz_add(Pointer_Output_Point->Y, Temp, Pointer_Output_Point->Y); // (-lambda * xr) + (lambda * xp)
	mpz_sub(Pointer_Output_Point->Y, Pointer_Output_Point->Y, Pointer_Point_P->Y); // (-lambda * xr) + (lambda * xp) - yp
	mpz_mod(Pointer_Output_Point->Y, Pointer_Output_Point->Y, Pointer_Curve->p);
	
	#ifdef DEBUG
		gmp_printf("[ECAddition] Lambda = %Zd\n", Lambda);
		gmp_printf("[ECAddition] Xr = %Zd\n", Pointer_Output_Point->X);
		gmp_printf("[ECAddition] Yr = %Zd\n", Pointer_Output_Point->Y);
	#endif
	
	Pointer_Output_Point->Is_Infinite = 0;
	
	mpz_clear(Lambda);
	mpz_clear(Temp);
}*/