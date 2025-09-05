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

#ifndef FIFTYONE_DEGREES_EVIDENCE_TESTS_BASE_INCLUDED
#define FIFTYONE_DEGREES_EVIDENCE_TESTS_BASE_INCLUDED

#include "pch.h"
#include "Base.hpp"
#include "StringCollection.hpp"
#include "../evidence.h"
#include "HeadersContainer.hpp"

/**
 * Evidence test class used to test the functionality in evidence.c.
 */
class Evidence : public Base {
protected:
	fiftyoneDegreesEvidenceKeyValuePairArray *evidence = nullptr;
    HeadersContainer headersContainer;
    size_t bufferSize = 512;
    char *buffer = nullptr;

	/**
	 * Calls the base setup method to enable memory leak checking and memory
	 * allocation checking.
	 */
	void SetUp() {
		Base::SetUp();
        buffer = (char *) fiftyoneDegreesMalloc(bufferSize);
	}

	/**
	 * Frees the evidence structure if one was created. Then calls the base
	 * teardown method to check for memory leaks and compare expected and
	 * actual memory allocations.
	 */
	void TearDown() {
		if (evidence != nullptr) {
			fiftyoneDegreesEvidenceFree(evidence);
		}
        headersContainer.Dealloc();
        if (buffer != nullptr) {
            fiftyoneDegreesFree(buffer);
            buffer = nullptr;
        }
        Base::TearDown();
	}

	/**
	 * Create an evidence structure with the specified capacity using the
	 * create method in evidence.c. The expected memory allocation is
	 * calculated, and the actual memory allocation is tracked. The structure
	 * is freed automatically after each test, at which point the expected and
	 * actual memory allocation is checked for equality.
	 */
	void CreateEvidence(uint32_t capacity) {
		evidence = fiftyoneDegreesEvidenceCreate(capacity);
	}
};

#endif
