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
#include "Base.hpp"
#include "../Value.hpp"

using std::vector;
using namespace FiftyoneDegrees::Common;

/**
 * Value test class used to test the functionality of Value.cpp.
 */
class ValueTests : public Base {
protected:

	/**
	 * Calls the base setup method to enable memory leak checking and memory
	 * allocation checking.
	 */
	void SetUp() {
		Base::SetUp();
	}

	/**
	 * Calls the base teardown method to check for memory leaks and compare
	 * expected and actual memory allocations.
	 */
	void TearDown() {
		Base::TearDown();
	}
};

/**
 * Check that the hasValue() method returns false when a value has not yet been
 * set, and true once it has.
 */
TEST_F(ValueTests, HasValue) {
	Value<string> value;

	EXPECT_FALSE(value.hasValue()) <<
		L"The value has not been set, so hasValue() should have returned false.";
	value.setValue("some value");
	EXPECT_TRUE(value.hasValue()) <<
		L"The value has been set, so hasValue() should have returned true.";
}

/**
 * Check that the getValue() method returns the value which has been set by the
 * setValue() method.
 */
TEST_F(ValueTests, GetValue) {
	int expected = 51;
	Value<int> value;

	value.setValue(expected);

	EXPECT_EQ(expected, value.getValue()) <<
		L"The value returned was not the value which was set.";
}

/**
 * Check that the * operator returns the value which has been set by the
 * setValue() method.
 */
TEST_F(ValueTests, GetValueWithOperator) {
	int expected = 51;
	Value<int> value;

	value.setValue(expected);

	EXPECT_EQ(expected, *value) << 
		L"The value returned was not that value which was set.";
}

/**
 * Check that an exception is thrown when an attempt is made to get a value
 * using the getValue() method when it has not been set.
 */
TEST_F(ValueTests, GetUnsetValue) {
	Value<int> value;
	ASSERT_THROW(value.getValue(), NoValuesAvailableException) <<
		L"No exception was thrown, even though no value was there to be returned.";
}

/**
 * Check that an exception is thrown when an attempt is made to get a value
 * using the * operator when it has not been set.
 */
TEST_F(ValueTests, GetUnsetValueWithOperator) {
	Value<int> value;
	ASSERT_THROW(*value, NoValuesAvailableException) <<
		L"No exception was thrown, even though no value was there to be returned.";
}

/**
 * Check that when the no value reason is set to "invalid property", that an
 * InvalidPropertyException is thrown when attempting to get the value.
 */
TEST_F(ValueTests, GetInvalidProperty) {
	Value<int> value;
	value.setNoValueReason(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY,
		nullptr);
	ASSERT_THROW(*value, InvalidPropertyException) <<
		L"The wrong exception was thrown for an invalid property.";
}

/**
 * Check that when the no value reason is set to "too many properties", that an
 * TooManyValuesException is thrown when attempting to get the value.
 */
TEST_F(ValueTests, GetTooManyValues) {
	Value<int> value;
	value.setNoValueReason(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
		nullptr);
	ASSERT_THROW(*value, TooManyValuesException) <<
		L"The wrong exception was thrown for too many values.";
}

/**
 * Check that when a no value message is set using the setNoValueException()
 * method, the same message is returned by the getNoValueMessage() method.
 */
TEST_F(ValueTests, GetNoValueMessage) {
	const char *expected = "some error message";
	Value<int> value;

	value.setNoValueReason(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS,
		expected);

	ASSERT_STREQ(expected, value.getNoValueMessage()) <<
		L"The no value message returned was not the one which was set.";
}

/**
 * Check that when a no value message is set using the setNoValueException()
 * method, the same message is used in the exception thrown.
 */
TEST_F(ValueTests, ExceptionMessage) {
	const char *expected = "some error message";
	Value<int> value;

	value.setNoValueReason(
		FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS,
		expected);

	try {
		value.getValue();
		FAIL() << L"This point should not be reached. See GetUnsetValue test.";
	}
	catch (NoValuesAvailableException& e) {
		ASSERT_STREQ(expected, e.what()) <<
			L"The no value message in the exception was not the one which was set.";
	}
}

/**
 * Check that behavior when using a complex type is the same.
 */
TEST_F(ValueTests, ComplexType) {
	vector<string> expected;
	Value<vector<string>> value;

	value.setValue(expected);

	ASSERT_EQ(expected, *value) <<
		L"The value returned was not the same as the one which was set.";
}

/**
 * Check that behavior when using a pointer to a type is the same.
 */
TEST_F(ValueTests, PointerType) {
	vector<string> *expected = new vector<string>();
	Value<vector<string>*> value;

	value.setValue(expected);

	ASSERT_EQ(expected, *value) <<
		L"The value returned was not the same as the one which was set.";

	delete expected;
}
