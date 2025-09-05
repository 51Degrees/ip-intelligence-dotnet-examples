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
#include "../IpAddress.hpp"
#include "../fiftyone.h"


static bool CheckResult(const byte *result, const byte *expected, uint16_t const size) {
	bool match = true;
	for (uint16_t i = 0; i < size; i++) {
		match = match && (*result == *expected);
		result++;
		expected++;
	}
	return match;
}

TEST(ParseAddress, ParseAddress_Ipv6_AbbreviatedStart)
{
	const char * const ipv6AbbreviatedStart = "::FFFF:FFFF:FFFF:FFFF";
	const byte expected[IPV6_LENGTH] =
		{ 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255 };

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(ipv6AbbreviatedStart);

	EXPECT_TRUE(ipAddress.getType() == IP_TYPE_IPV6) <<
		"IP address version was not identified correctly where the " <<
		"IP address is " << ipv6AbbreviatedStart << ".";

	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), expected, IPV6_LENGTH)) <<
		"The value of the abbreviated start IPv6 address is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Invalid_ipv4OutOfRange)
{
	const char * const ipv4OutOfRange = "256.256.256.256";
	constexpr byte expected[IPV4_LENGTH] =
		{255, 255, 255, 255};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(ipv4OutOfRange);

	EXPECT_TRUE(ipAddress.getType() == IP_TYPE_IPV4) <<
		"IP address version was not identified correctly where the " <<
		"IP address is " << ipv4OutOfRange << ".";

	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), expected, IPV4_LENGTH)) <<
		"The value of the out of range IPv4 address is not correctly restricted "
		"at 255.";
}

TEST(ParseAddress, ParseAddress_Ipv6_BasicIpv6Address)
{
	// Basic IPv6 address
	const char * const rawAddress = "2001:0db8:85a3::8a2e:370:7334";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 133, 163, 0, 0, 0, 0, 138, 46, 3, 112, 115, 52
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Basic IPv6 address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_CompressedZeros)
{
	// Compressed zeros
	const char * const rawAddress = "2001:db8::1";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Compressed zeros) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_LeadingZeros)
{
	// Leading zeros in each segment
	const char * const rawAddress = "2001:0db8:0000:0042:0000:8a2e:0370:7334";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 0, 0, 0, 66, 0, 0, 138, 46, 3, 112, 115, 52
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Leading zeros in each segment) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_MixedIpv6Ipv4)
{
	// IPv4-mapped IPv6 address
	const char * const rawAddress = "::ffff:192.168.1.1";
	const unsigned char addressBytes[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 192, 168, 1, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (IPv4-mapped IPv6 address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_LoopbackAddress)
{
	// Loopback address
	const char * const rawAddress = "::1";
	const unsigned char addressBytes[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Loopback address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_LinkLocalAddress)
{
	// Link-local address
	const char * const rawAddress = "fe80::1";
	const unsigned char addressBytes[] = {
		254, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Link-local address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_MulticastAddress)
{
	// Multicast address
	const char * const rawAddress = "ff02::1";
	const unsigned char addressBytes[] = {
		255, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Multicast address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_GlobalUnicast)
{
	// Global unicast address
	const char * const rawAddress = "2001:0db8:1234:5678:9abc:def0:1234:5678";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 18, 52, 86, 120, 154, 188, 222, 240, 18, 52, 86, 120
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Global unicast address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_UniqueLocal)
{
	// Unique local address
	const char * const rawAddress = "fd00::1";
	const unsigned char addressBytes[] = {
		253, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Unique local address) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_Ipv6InterfaceId)
{
	// Interface identifier (last 64 bits)
	const char * const rawAddress = "2001:0db8:85a3::8a2e:0370:7334";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 133, 163, 0, 0, 0, 0, 138, 46, 3, 112, 115, 52
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Interface identifier (last 64 bits)) is not correctly parsed.";
}

TEST(ParseAddress, ParseAddress_Ipv6_ZeroCompressedBlocks)
{
	// Zero-compressed blocks
	const char * const rawAddress = "2001:db8::85a3:0:0:370:7334";
	const unsigned char addressBytes[] = {
		32, 1, 13, 184, 0, 0, 133, 163, 0, 0, 0, 0, 3, 112, 115, 52
	};

	auto const ipAddress = FiftyoneDegrees::IpIntelligence::IpAddress(rawAddress);
	EXPECT_TRUE(
		CheckResult(ipAddress.getIpAddress(), addressBytes, IPV6_LENGTH)) <<
		"The value of the IPv6 address (Zero-compressed blocks) is not correctly parsed.";
}
