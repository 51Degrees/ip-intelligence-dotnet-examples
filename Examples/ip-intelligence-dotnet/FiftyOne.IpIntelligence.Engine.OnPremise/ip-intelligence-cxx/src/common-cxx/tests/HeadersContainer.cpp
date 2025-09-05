//
//  HeadersContainer.cpp
//  CommonTests
//
//  Created by Eugene Dorfman on 8/6/24.
//

#include "HeadersContainer.hpp"
#include "../fiftyone.h"

void HeadersContainer::CreateHeaders(
    const char** headersList,
    int headersCount,
    bool expectUpperPrefixedHeaders) {
    EXCEPTION_CREATE
    count = headersCount;
    strings = new StringCollection(headersList, count);
    headers = fiftyoneDegreesHeadersCreate(
        expectUpperPrefixedHeaders,
        strings->getState(),
        getHeaderUniqueId,
        exception);
    
    FIFTYONE_DEGREES_ARRAY_CREATE(fiftyoneDegreesHeaderPtr, headerPointers, headersCount);
    if (headerPointers) {
        for (int i=0;i<headersCount;++i) {
            headerPointers->items[i] = &headers->items[i];
            headerPointers->count++;
        }
    }
}

void HeadersContainer::Dealloc() {
    if (headers != nullptr) {
        fiftyoneDegreesHeadersFree(headers);
        headers = nullptr;
    }
    if (strings != nullptr) {
        delete strings;
        strings = nullptr;
    }
    if (headerPointers != nullptr) {
        fiftyoneDegreesFree(headerPointers);
        headerPointers = nullptr;
    }
}
