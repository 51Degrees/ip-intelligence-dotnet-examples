/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "pch.h"
#include <cmath>
#include "../float.h"

using namespace std;

/*
 * fiftyoneDegreesFloat to Native conversion tests
 */
TEST(Float, Float_FloatToNative_Postive)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 133;
	floatU.parts.mantissa = 7749370;

	float expected = 123.123f;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Postive_Low)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 123;
	floatU.parts.mantissa = 8120173;

	float expected = 0.123f;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Negative)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 1;
	floatU.parts.exponent = 133;
	floatU.parts.mantissa = 7749370;

	float expected = -123.123f;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Negative_High)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 1;
	floatU.parts.exponent = 123;
	floatU.parts.mantissa = 8120173;

	float expected = -0.123f;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Zero)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 0;
	floatU.parts.mantissa = 0;

	float expected = 0;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Max)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 254;
	floatU.parts.mantissa = 8388607;

	float expected = FIFTYONE_DEGREES_FLOAT_MAX;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Min)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 1;
	floatU.parts.mantissa = 0;

	float expected = FIFTYONE_DEGREES_FLOAT_MIN;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Min_Neg)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 1;
	floatU.parts.exponent = 254;
	floatU.parts.mantissa = 8388607;

	float expected = FIFTYONE_DEGREES_FLOAT_MIN_NEG;
	EXPECT_FLOAT_EQ(expected, fiftyoneDegreesFloatToNative(floatU.fValue));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	EXPECT_EQ(0, memcmp(&expected, floatU.fValue.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_FloatToNative_Nan)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = FIFTYONE_DEGREES_FLOAT_EXP_MAX;
	floatU.parts.mantissa = 1;

	EXPECT_EQ(FP_NAN, fpclassify(fiftyoneDegreesFloatToNative(floatU.fValue)));
}

TEST(Float, Float_FloatToNative_Inf)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = FIFTYONE_DEGREES_FLOAT_EXP_MAX;
	floatU.parts.mantissa = FIFTYONE_DEGREES_FLOAT_MANT_MAX;

	EXPECT_EQ(FP_INFINITE, fpclassify(fiftyoneDegreesFloatToNative(floatU.fValue)));
}

TEST(Float, Float_FloatToNative_Subnormal)
{
	fiftyoneDegreesFloatU floatU;
	floatU.parts.sign = 0;
	floatU.parts.exponent = 0;
	floatU.parts.mantissa = 1;

	EXPECT_EQ(FP_SUBNORMAL, fpclassify(fiftyoneDegreesFloatToNative(floatU.fValue)));
}

/*
 * Native to fiftyoneDegreesFloat conversion tests
 */
TEST(Float, Float_NativeToFloat_Postive)
{
	float testFloat = 123.123f;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = 133;
	expected.parts.mantissa = 7749370;
	
	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Postive_Low)
{
	float testFloat = 0.123f;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = 123;
	expected.parts.mantissa = 8120173;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Negative)
{
	float testFloat = -123.123f;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 1;
	expected.parts.exponent = 133;
	expected.parts.mantissa = 7749370;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Negative_High)
{
	float testFloat = -0.123f;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 1;
	expected.parts.exponent = 123;
	expected.parts.mantissa = 8120173;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Zero)
{
	float testFloat = 0;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = 0;
	expected.parts.mantissa = 0;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Max)
{
	float testFloat = FIFTYONE_DEGREES_FLOAT_MAX;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = 254;
	expected.parts.mantissa = 8388607;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Min)
{
	float testFloat = FIFTYONE_DEGREES_FLOAT_MIN;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = 1;
	expected.parts.mantissa = 0;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Min_Neg)
{
	float testFloat = FIFTYONE_DEGREES_FLOAT_MIN_NEG;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 1;
	expected.parts.exponent = 254;
	expected.parts.mantissa = 8388607;

	fiftyoneDegreesFloatInternal actual = fiftyoneDegreesNativeToFloat(testFloat);
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, actual));
#if (FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128 && FLT_MIN_EXP == -125)
	/*
	 * On machine where the standard is supported. 51Degrees implementation should
	 * have the exact match as native float.
	 */
	ASSERT_EQ(0, memcmp(&testFloat, actual.value, FIFTYONE_DEGREES_FLOAT_SIZE));
#endif
}

TEST(Float, Float_NativeToFloat_Nan)
{
	float testFloat = NAN;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = FIFTYONE_DEGREES_FLOAT_EXP_MAX;
	expected.parts.mantissa = 1;
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, fiftyoneDegreesNativeToFloat(testFloat)));
}

TEST(Float, Float_NativeToFloat_Inf)
{
	float testFloat = INFINITY;

	fiftyoneDegreesFloatU expected;
	expected.parts.sign = 0;
	expected.parts.exponent = FIFTYONE_DEGREES_FLOAT_EXP_MAX;
	expected.parts.mantissa = FIFTYONE_DEGREES_FLOAT_MANT_MAX;
	ASSERT_EQ(0, fiftyoneDegreesFloatIsEqual(expected.fValue, fiftyoneDegreesNativeToFloat(testFloat)));
}