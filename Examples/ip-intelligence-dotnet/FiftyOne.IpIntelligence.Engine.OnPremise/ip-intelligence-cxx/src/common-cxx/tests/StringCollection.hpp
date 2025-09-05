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

#ifndef FIFTYONE_DEGREES_STRINGS_TESTS_INCLUDED
#define FIFTYONE_DEGREES_STRINGS_TESTS_INCLUDED

#include "../collection.h"
#include "../string.h"

typedef struct string_collection_state_t {
	fiftyoneDegreesCollection *collection;
	uint32_t *offsets;
	void *data;
	uint32_t count;
} stringCollectionState;

// Function used to return string names when the collections code 
// requests them
const fiftyoneDegreesString* getStringValue(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item);

long getHeaderUniqueId(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item);

/**
 * String collection helper class, used by test classes to fetch.
 */
class StringCollection {
public:
	StringCollection(const char **values, int count);
	~StringCollection();
	stringCollectionState* getState();

private:
	stringCollectionState state;
};

#endif