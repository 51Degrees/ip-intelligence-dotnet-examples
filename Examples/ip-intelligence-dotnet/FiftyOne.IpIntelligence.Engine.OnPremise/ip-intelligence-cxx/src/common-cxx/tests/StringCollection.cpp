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
#include "StringCollection.hpp"

#include "../collectionKeyTypes.h"
#include "../fiftyone.h"
#include "../Exceptions.hpp"

// Function used to return string names when the collections code 
// requests them
const fiftyoneDegreesString* getStringValue(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE
	fiftyoneDegreesString *value = nullptr;
	stringCollectionState *strings = (stringCollectionState*)state;
	if (index < strings->count) {
		const fiftyoneDegreesCollectionKey key {
			strings->offsets[index],
			CollectionKeyType_String,
		};
		value = (String*)strings->collection->get(
			strings->collection,
			&key,
			item,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
		item->collection = strings->collection;
	}
	return value;
}

// Function used to get the string name of a header, and return the unique id
// when the collections code requests them
long getHeaderUniqueId(
	void *state,
	uint32_t index,
	fiftyoneDegreesCollectionItem *item) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE
	long uniqueId = -1;
	auto const strings = (const stringCollectionState*)state;
	if (index >= 0 && index < strings->count) {
		const fiftyoneDegreesCollectionKey key {
			strings->offsets[index],
			CollectionKeyType_String,
		};
		strings->collection->get(
			strings->collection,
			&key,
			item,
			exception);
		FIFTYONE_DEGREES_EXCEPTION_THROW
		item->collection = strings->collection;
		uniqueId = strings->offsets[index];
	}
	return uniqueId;
}

// Class that sets up the properties structure. This stops us having to 
// do it multiple times.
StringCollection::~StringCollection() {
	free(state.offsets);
	free(state.data);
	state.collection->freeCollection(state.collection);
}

StringCollection::StringCollection(
	const char **values,
	int valuesCount) {
	int i, j;
	uint32_t currentOffsetIndex = 0;
	fiftyoneDegreesMemoryReader reader;
	size_t dataLength = 0;
	state.count = 0;
	for (i = 0; i < valuesCount; i++) {
		if (values[i] != NULL) {
			dataLength += 2 + strlen(values[i]) + 1;
			state.count++;
		}
	}
	reader.length = (FileOffset)(dataLength + sizeof(uint32_t));
	state.data = malloc(reader.length);
	*(int32_t*)state.data = (int32_t)dataLength;
	state.offsets = (uint32_t*)malloc(valuesCount * sizeof(uint32_t));
	reader.startByte = ((byte*)state.data);
	reader.lastByte = reader.startByte + reader.length;
	reader.current = reader.startByte + sizeof(uint32_t);
	for (i = 0; i < valuesCount; i++) {
		if (values[i] != NULL) {
			fiftyoneDegreesString *string =
				(fiftyoneDegreesString*)reader.current;
			string->size = (int16_t)strlen(values[i]) + 1;
			for (j = 0; j < (int)strlen(values[i]); j++) {
				(&string->value)[j] = values[i][j];
			}
			(&string->value)[strlen(values[i])] = '\0';
			state.offsets[currentOffsetIndex] =
				(uint32_t)(reader.current - (reader.startByte + sizeof(uint32_t)));
			reader.current += 2 + string->size;
			currentOffsetIndex++;
		}
	}
	assert(currentOffsetIndex == state.count);
	assert(reader.lastByte == reader.current);
	assert((byte*)state.data + sizeof(int32_t) + dataLength == reader.lastByte);
	assert((byte*)state.data + reader.length == reader.lastByte);
	reader.current = reader.startByte;
	state.collection = fiftyoneDegreesCollectionCreateFromMemory(
		&reader,
		fiftyoneDegreesCollectionHeaderFromMemory(&reader, 0, false));
	assert(state.collection->size == dataLength);
}

stringCollectionState* StringCollection::getState() {
	return &state;
}