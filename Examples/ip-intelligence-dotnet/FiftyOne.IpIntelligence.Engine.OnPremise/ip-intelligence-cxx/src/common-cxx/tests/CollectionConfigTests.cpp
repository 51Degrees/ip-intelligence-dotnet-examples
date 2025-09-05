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
 
#include "Base.hpp"
#include "../CollectionConfig.hpp"

static fiftyoneDegreesCollectionConfig testValues = {
	true, /* Loaded */
	1, /* Capacity */
	2 /* Concurrency */
};

static fiftyoneDegreesCollectionConfig otherTestValues = {
	true, /* Loaded */
	4, /* Capacity */
	5 /* Concurrency */
};

TEST_CLASS(CollectionConfig, &testValues)

class CollectionConfigTestSet : public CollectionConfigTest {
public:
	void SetUp() {
		CollectionConfigTest::SetUp();
		instance->setCapacity(otherTestValues.capacity);
		instance->setConcurrency(otherTestValues.concurrency);
		instance->setLoaded(otherTestValues.loaded);
	};
};

TEST_PROPERTY_EQUAL(CollectionConfigTest, Capacity, , testValues.capacity)
TEST_PROPERTY_EQUAL(CollectionConfigTest, Concurrency, , testValues.concurrency)
TEST_PROPERTY_EQUAL(CollectionConfigTest, Loaded, , testValues.loaded)
TEST_PROPERTY_EQUAL(CollectionConfigTestSet, Capacity, , otherTestValues.capacity)
TEST_PROPERTY_EQUAL(CollectionConfigTestSet, Concurrency, , otherTestValues.concurrency)
TEST_PROPERTY_EQUAL(CollectionConfigTestSet, Loaded, , otherTestValues.loaded)