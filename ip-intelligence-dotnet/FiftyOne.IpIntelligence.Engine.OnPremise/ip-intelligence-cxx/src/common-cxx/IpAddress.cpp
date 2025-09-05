/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#include <cstring>
#include <string>
#include <stdexcept>
#include "memory.h"
#include "IpAddress.hpp"
#include "Exceptions.hpp"
#include "fiftyone.h"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

FiftyoneDegrees::IpIntelligence::IpAddress::IpAddress() {
    memset(this->ipAddress, 0, IPV6_LENGTH);
    this->type = IP_TYPE_INVALID;
}

FiftyoneDegrees::IpIntelligence::IpAddress::IpAddress(
    const unsigned char ipAddressData[],
    IpType addressType) {
    init(ipAddressData, addressType);
}

FiftyoneDegrees::IpIntelligence::IpAddress::IpAddress(
    const char * const ipAddressString) {
    ::IpAddress eIpAddress;
    const bool parsed =
		IpAddressParse(
			ipAddressString,
			ipAddressString + strlen(ipAddressString),
			&eIpAddress);
    // Make sure the ip address has been parsed successfully
	if (!parsed) {
		throw Common::StatusCodeException(INCORRECT_IP_ADDRESS_FORMAT);
	}
    // Initialize the IP address object
    init(eIpAddress.value, static_cast<IpType>(eIpAddress.type));
}

void FiftyoneDegrees::IpIntelligence::IpAddress::init(
    const unsigned char * const ipAddressData,
    const IpType addressType) {
    switch (addressType) {
    case IP_TYPE_IPV4:
        memcpy(ipAddress, ipAddressData, IPV4_LENGTH);
        break;
    case IP_TYPE_IPV6:
        memcpy(ipAddress, ipAddressData, IPV6_LENGTH);
        break;
    default:
        memset(ipAddress, 0, IPV6_LENGTH);
        break;
    }
    type = addressType;
}

void FiftyoneDegrees::IpIntelligence::IpAddress::getCopyOfIpAddress(
    unsigned char copy[], const uint32_t size) const {
    const uint32_t dataSize = ((type == IP_TYPE_IPV4)
        ? IPV4_LENGTH
        : IPV6_LENGTH);
	const uint32_t copySize = (size < dataSize) ? size : dataSize;
	memcpy(copy, ipAddress, copySize);
}
