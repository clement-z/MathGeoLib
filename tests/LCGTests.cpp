#include "../src/Math/myassert.h"
#include "../src/MathGeoLib.h"
#include "../tests/TestRunner.h"

MATH_IGNORE_UNUSED_VARS_WARNING

RANDOMIZED_TEST(LCG_IntFast)
{
	LCG lcg;
	u32 prev = lcg.IntFast();
	for(int i = 0; i < 1000; ++i)
	{
		u32 next = lcg.IntFast();
		assert(next != prev);
		prev = next;
	}
}

RANDOMIZED_TEST(LCG_Int)
{
	LCG lcg;
	assert4((lcg.lastNumber != 0 || lcg.increment != 0) && "Default-seeded LCG should be functional!", lcg.lastNumber, lcg.increment, lcg.multiplier, lcg.modulus);
	bool allEqual = true;
	for(int i = 0; i < 1000; ++i)
	{
		int prev = lcg.Int();
		int next = lcg.Int();
		assert4(prev != 0 || next != 0, lcg.multiplier, lcg.increment, lcg.modulus, lcg.lastNumber);
		if (prev != next)
			allEqual = false;
	}
	assert(!allEqual);
}

RANDOMIZED_TEST(LCG_Int_A_B)
{
	LCG lcg;
	for(int i = 0; i < 1000; ++i)
	{
		int a = lcg.Int();
		int b = lcg.Int();
		if (b < a)
			Swap(a, b);
		int val = lcg.Int(a, b);
		assert(a <= val);
		assert(val <= b);
	}
}

RANDOMIZED_TEST(LCG_Float)
{
	LCG lcg;
	bool allEqual = true;
	for(int i = 0; i < 1000; ++i)
	{
		float f = lcg.Float();
		float f2 = lcg.Float();
		assert1(f < 1.f, f);
		assert1(f >= 0.f, f);
		assert4(f != 0.f || f2 != 0.f, lcg.multiplier, lcg.increment, lcg.modulus, lcg.lastNumber);
		if (f != f2)
			allEqual = false;
	}
	assert(!allEqual);
}

RANDOMIZED_TEST(LCG_Float01Incl)
{
	LCG lcg;
	bool allEqual = true;
	for(int i = 0; i < 1000; ++i)
	{
		float f = lcg.Float01Incl();
		float f2 = lcg.Float01Incl();
		assert(f <= 1.f);
		assert(f >= 0.f);
		assert4(f != 0.f || f2 != 0.f, lcg.multiplier, lcg.increment, lcg.modulus, lcg.lastNumber);
		if (f != f2)
			allEqual = false;
	}
	assert(!allEqual);
}

RANDOMIZED_TEST(LCG_FloatNeg1_1)
{
	LCG lcg;
	bool allEqual = true;
	for(int i = 0; i < 1000; ++i)
	{
		float f = lcg.FloatNeg1_1();
		float f2 = lcg.FloatNeg1_1();
		assert1(f < 1.f, f);
		assert1(f > -1.f, f);
		assert4(f != 0.f || f2 != 0.f, lcg.multiplier, lcg.increment, lcg.modulus, lcg.lastNumber);
		if (f != f2)
			allEqual = false;
	}
	assert(!allEqual);
}

RANDOMIZED_TEST(LCG_Float_A_B)
{
	LCG lcg;
	for(int i = 0; i < 1000; ++i)
	{
		float a = lcg.Float();
		float b = lcg.Float();
		if (b < a)
			Swap(a, b);

		float f = lcg.Float(a, b);
		assert2(a <= f, a, b);
		assert2(f < b, f, b);
	}
}
