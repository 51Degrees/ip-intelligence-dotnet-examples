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

#ifndef FixedSizeCollection_hpp
#define FixedSizeCollection_hpp

#include "../collection.h"
#include "../string.h"
#include "../fileOffset.h"
#include <vector>

typedef struct fixed_size_collection_state_t {
    fiftyoneDegreesCollection *collection;
    uint32_t *offsets;
    void *data;
    uint32_t count;
} fixedSizeCollectionState;

/**
 * Fixed size object collection helper class, used by test classes to fetch.
 * T must be a fixed size value type (s.a. struct) the colleciton will hold instance of size sizeof(T)
 */
template<typename T>
class FixedSizeCollection {
public:
    FixedSizeCollection(const std::vector<T>& values);
    ~FixedSizeCollection();
    fixedSizeCollectionState* getState();

private:
    fixedSizeCollectionState state;
};

template<typename T>
FixedSizeCollection<T>::~FixedSizeCollection() {
    fiftyoneDegreesFree(state.offsets);
    fiftyoneDegreesFree(state.data);
    state.collection->freeCollection(state.collection);
}

template<typename T>
FixedSizeCollection<T>::FixedSizeCollection(const std::vector<T> &values) {
    uint32_t currentOffsetIndex = 0;
    fiftyoneDegreesMemoryReader reader;
    size_t dataLength = values.size() * sizeof(T);
    state.count = (uint32_t) values.size();
    
    reader.length = (fiftyoneDegreesFileOffset)(dataLength + sizeof(uint32_t));
    state.data = fiftyoneDegreesMalloc(reader.length);
    *(int32_t*)state.data = (int32_t)dataLength;
    state.offsets = (uint32_t*)fiftyoneDegreesMalloc(values.size() * sizeof(uint32_t));
    reader.startByte = ((byte*)state.data);
    reader.lastByte = reader.startByte + reader.length;
    reader.current = reader.startByte + sizeof(uint32_t);
    
    for (size_t i = 0; i < values.size(); i++) {
        T *element = (T*)reader.current;
        memcpy((void *) element, (void *) &values[i], sizeof(T));
        state.offsets[currentOffsetIndex] =
            (uint32_t)(reader.current - (reader.startByte + sizeof(uint32_t)));
        reader.current += sizeof(T);
        currentOffsetIndex++;
    }
    assert(currentOffsetIndex == state.count);
    assert(reader.lastByte == reader.current);
    assert((byte*)state.data + sizeof(int32_t) + dataLength == reader.lastByte);
    assert((byte*)state.data + reader.length == reader.lastByte);
    reader.current = reader.startByte;
    state.collection = fiftyoneDegreesCollectionCreateFromMemory(
        &reader,
        fiftyoneDegreesCollectionHeaderFromMemory(&reader, sizeof(T), false));
    assert(state.collection->size == dataLength);
}

template<typename T>
fixedSizeCollectionState* FixedSizeCollection<T>::getState() {
    return &state;
}

#endif /* FixedSizeCollection_hpp */
