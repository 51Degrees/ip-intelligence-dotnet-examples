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
#include "../threading.h"
#include "Base.hpp"

/**
 * Macro used to ensure that local variables are aligned to memory boundaries
 * to support interlocked operations that require double width data structures
 * and pointers to be aligned.
 */
#if ((defined(_MSC_VER) && defined(_WIN64)) \
    || ((defined(__GNUC__) || defined(__clang__)) \
        && (defined(__x86_64__) || defined(__aarch64__))))
#define ALIGN_SIZE 16
typedef struct testDoubleWidth_t {
	void* ptr;
	int32_t count;
	int32_t padding;
} testDoubleWidth;
#else
#define ALIGN_SIZE 8
typedef struct testDoubleWidth_t {
	void* ptr;
	int32_t count;
} testDoubleWidth;
#endif

typedef union doubleWidth_u {
    fiftyoneDegreesInterlockDoubleWidth fodDW;
    testDoubleWidth testDW;
} doubleWidth;

#ifdef _MSC_VER
#define INTERLOCK_DOUBLE_WIDTH \
    __declspec(align(ALIGN_SIZE)) doubleWidth
#else
typedef doubleWidth alignedDoubleWidth __attribute__ ((aligned (ALIGN_SIZE)));
#define INTERLOCK_DOUBLE_WIDTH alignedDoubleWidth
#endif

class Threading : public Base
{
public:
    INTERLOCK_DOUBLE_WIDTH item;

protected:

	void SetUp() {
		Base::SetUp();
	}

	void TearDown() {
		Base::TearDown();
	}
};
TEST_F(Threading, DoubleWidthExchange_Matching) {

    int val1 = 122;
    int val2 = 123;

    memset(&item, 0, sizeof(item));
    item.testDW.ptr = (void*)&val1;

    INTERLOCK_DOUBLE_WIDTH newItem;
    newItem = item;
    INTERLOCK_DOUBLE_WIDTH compare;
    compare = newItem;
    newItem.testDW.ptr = (void*)&val2;
    newItem.testDW.count++;

    bool changed = 
        FIFTYONE_DEGREES_INTERLOCK_EXCHANGE_DW(
            item.fodDW, 
            newItem.fodDW, 
            compare.fodDW);
    
    ASSERT_TRUE(changed);
    ASSERT_EQ(newItem.testDW.count, item.testDW.count);
    ASSERT_EQ(newItem.testDW.ptr, item.testDW.ptr);
    ASSERT_EQ(
        *(int*)newItem.testDW.ptr, 
        *(int*)item.testDW.ptr);
}
TEST_F(Threading, DoubleWidthExchange_NonMatching) {

    int val1 = 122;
    int val2 = 123;

    memset(&item, 0, sizeof(item));
    item.testDW.ptr = (void*)&val1;

    INTERLOCK_DOUBLE_WIDTH newItem;
    newItem = item;
    INTERLOCK_DOUBLE_WIDTH compare;
    compare = newItem;
    newItem.testDW.ptr = (void*)&val2;
    newItem.testDW.count++;

    item.testDW.count++;

    bool changed = 
        FIFTYONE_DEGREES_INTERLOCK_EXCHANGE_DW(
            item.fodDW, 
            newItem.fodDW, 
            compare.fodDW);
    ASSERT_FALSE(changed);
    ASSERT_NE(newItem.testDW.ptr, item.testDW.ptr);
    ASSERT_NE(
        *(int*)newItem.testDW.ptr, 
        *(int*)item.testDW.ptr);
}