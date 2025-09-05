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
#include "../ip.h"
#include "../fiftyone.h"
#include <memory>

static bool CheckResult(
	const byte* result,
	const byte* expected,
	const uint16_t size) {
	bool match = true;
	for (uint16_t i = 0; i < size && match; i++) {
		match = (*result == *expected);
		result++;
		expected++;
	}
	return match;
}

static bool parseIpAddressInPlace(const std::unique_ptr<IpAddress> &ipAddress, const char * const ipString, const size_t length) {
	const char * const end = ipString ? ipString + length : nullptr;
	return IpAddressParse(ipString, end, ipAddress.get());
}

static std::unique_ptr<IpAddress> parseIpAddressStringOfLength(const char * const ipString, const size_t length) {
	std::unique_ptr<IpAddress> ipPtr = std::make_unique<IpAddress>();
	for (size_t i = 0; i < std::size(ipPtr->value); i++) {
		ipPtr->value[i] = static_cast<byte>(0xA0 + i);
	}
	const bool parsed =parseIpAddressInPlace(ipPtr, ipString, length);
	return parsed ? std::move(ipPtr) : nullptr;
}

static std::unique_ptr<IpAddress> parseIpAddressString(const char * const ipString) {
	return parseIpAddressStringOfLength(ipString, ipString ? strlen(ipString) : 0);
}

 // ------------------------------------------------------------------------------
 // IPv4 tests
 // ------------------------------------------------------------------------------
TEST(ParseIp, ParseIp_Ipv4_Low)
{
	const char* ip = "0.0.0.0";
	auto const result = parseIpAddressString(ip);
	byte expected[] = { 0, 0, 0, 0 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '0.0.0.0'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_High)
{
	auto const result = parseIpAddressString("255.255.255.255");
	byte expected[] = { 255, 255, 255, 255 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '255.255.255.255'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_Endline)
{
	const char* ip = "0.0.0.0\n";
	auto const result = parseIpAddressString(ip);
	byte expected[] = { 0, 0, 0, 0 };
	EXPECT_TRUE(result) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '0.0.0.0'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_NoOverwrite12)
{
	const char ip[] = "73.79.83.89";
	constexpr byte initial[] {
		67, 13, 43, 47, 53, 29, 61, 19, 41, 31, 59, 71, 23, 37, 17, 11,
	}, expected[] {
		73, 79, 83, 89, 53, 29, 61, 19, 41, 31, 59, 71, 23, 37, 17, 11,
	};
	std::unique_ptr<IpAddress> ipAddress = std::make_unique<IpAddress>();
	memcpy(ipAddress->value, initial, sizeof(initial));
	const bool parsed = parseIpAddressInPlace(ipAddress, ip, sizeof(ip));
	EXPECT_TRUE(parsed) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(ipAddress->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '73.79.83.89' with the rest (12 bytes) unchanged.";
}
TEST(ParseIp, ParseIp_Ipv4_Trim0)
{
	const char* ip = "192.168.101.217";
	auto const result = parseIpAddressStringOfLength(ip, strlen(ip));
	byte expected[] = { 192, 168, 101, 217 };
	EXPECT_TRUE(result) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '192.168.101.217'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_Trim1)
{
	const char* ip = "192.168.101.217";
	auto const result = parseIpAddressStringOfLength(ip, strlen(ip) - 1);
	byte expected[] = { 192, 168, 101, 217 };
	EXPECT_TRUE(result) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '192.168.101.217'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_Trim2)
{
	const char* ip = "192.168.101.217";
	auto const result = parseIpAddressStringOfLength(ip, strlen(ip) - 2);
	byte expected[] = { 192, 168, 101, 21 };
	EXPECT_TRUE(result) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '192.168.101.21'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_Trim3)
{
	const char* ip = "192.168.101.217";
	auto const result = parseIpAddressStringOfLength(ip, strlen(ip) - 3);
	byte expected[] = { 192, 168, 101, 2 };
	EXPECT_TRUE(result) <<
		L"Expected result to be non-NULL.";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected) - 1)) <<
		L"Expected result to be '192.168.101.2'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_PortNumber)
{
	auto const result = parseIpAddressString("1.2.3.4:80");
	byte expected[] = { 1, 2, 3, 4 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '1.2.3.4'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
TEST(ParseIp, ParseIp_Ipv4_CIDRFormat)
{
	auto const result = parseIpAddressString("1.2.3.4/32");
	byte expected[] = { 1, 2, 3, 4 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '1.2.3.4'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV4) <<
		L"Expected type to be IPv4";
}
// ------------------------------------------------------------------------------
// IPv6 tests
// ------------------------------------------------------------------------------
TEST(ParseIp, ParseIp_Ipv6_Low)
{
	auto const result = parseIpAddressString("0:0:0:0:0:0:0:0");
	byte expected[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}
TEST(ParseIp, ParseIp_Ipv6_Low_Abbreviated)
{
	auto const result = parseIpAddressString("::");
	const byte expected[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0'";
}
TEST(ParseIp, ParseIp_Ipv6_High)
{
	auto const result = parseIpAddressString("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF");
	byte expected[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}
TEST(ParseIp, ParseIp_Ipv6_AbbreviatedStart)
{
	const char *ipv6AbbreviatedStart = "::FFFF:FFFF:FFFF:FFFF";
	byte expected[IPV6_LENGTH] =
		{ 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255 };

	IpAddress result;
	const bool parsed =
		fiftyoneDegreesIpAddressParse(
			ipv6AbbreviatedStart,
			ipv6AbbreviatedStart + strlen(ipv6AbbreviatedStart),
			&result);

	EXPECT_TRUE(parsed) << "Abbreviated start IPv6 address "
		"should be successfully parsed, where the address is " <<
		ipv6AbbreviatedStart << ".";

	EXPECT_TRUE(result.type == IP_TYPE_IPV6) <<
		"IP address version was not identified correctly where where the " <<
		"IP address is " << ipv6AbbreviatedStart << ".";

	EXPECT_TRUE(
		CheckResult(result.value, expected, IPV6_LENGTH)) <<
		"The value of the abbreivated start IPv6 address is not correctly parsed.";
}
TEST(ParseIp, ParseIp_Ipv6_AbbreviatedMiddle)
{
	auto const result = parseIpAddressString("FFFF:FFFF::FFFF:FFFF");
	byte expected[] = { 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}
TEST(ParseIp, ParseIp_Ipv6_AbbreviatedEnd)
{
	auto const result = parseIpAddressString("FFFF:FFFF:FFFF:FFFF::");
	byte expected[] = { 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}
TEST(ParseIp, ParseIp_Ipv6_PortNumber)
{
	auto const result = parseIpAddressString("[2001::1]:80");
	byte expected[] = { 32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}
TEST(ParseIp, ParseIp_Ipv6_CIDRFormat)
{
	auto const result = parseIpAddressString("2001::1/128");
	byte expected[] = { 32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	EXPECT_TRUE(CheckResult(result->value, expected, sizeof(expected))) <<
		L"Expected result to be '32, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1'";
	EXPECT_EQ(result->type, FIFTYONE_DEGREES_IP_TYPE_IPV6) <<
		L"Expected type to be IPv6";
}

// ------------------------------------------------------------------------------
// Invalid data
// ------------------------------------------------------------------------------
TEST(ParseIp, ParseIp_Invalid_ipv4OutOfRange)
{
	const char *ipv4OutOfRange = "256.256.256.256";
	byte expected[IPV4_LENGTH] =
		{255, 255, 255, 255};

	IpAddress result;
	const bool parsed  =
		fiftyoneDegreesIpAddressParse(
			ipv4OutOfRange,
			ipv4OutOfRange + strlen(ipv4OutOfRange),
			&result);

	EXPECT_TRUE(parsed) << "Out of range IPv4 address "
		"should be successfull parsed, where the address is " <<
		ipv4OutOfRange << ".";

	EXPECT_TRUE(result.type == IP_TYPE_IPV4) <<
		"IP address version was not identified correctly where where the " <<
		"IP address is " << ipv4OutOfRange << ".";

	EXPECT_TRUE(
		CheckResult(result.value, expected, IPV4_LENGTH)) <<
		"The value of the out of range IPv4 address is not correctly restricted "
		"at 255.";
}
TEST(ParseIp, ParseIp_Invalid_ipv4TooMany)
{
	auto const result = parseIpAddressString("1.2.3.4.5");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_ipv4TooFew)
{
	auto const result = parseIpAddressString("1.2.3");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_letters)
{
	auto const result = parseIpAddressString("a.b.c.d");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_emptyString)
{
	auto const result = parseIpAddressString("");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_null)
{
	auto const result = parseIpAddressString(nullptr);
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_Number)
{
	auto const result = parseIpAddressString("1234");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_ipv6OutOfRange)
{
	auto const result = parseIpAddressString("10000::1");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_ipv6InvalidLetter)
{
	auto const result = parseIpAddressString("GFFF::1");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_ipv6TooMany)
{
	auto const result = parseIpAddressString("1:1:1:1:1:1:1:1:1");
	EXPECT_FALSE(result);
}
TEST(ParseIp, ParseIp_Invalid_ipv6TooFew)
{
	auto const result = parseIpAddressString("1:1:1:1:1:1:1");
	EXPECT_FALSE(result);
}
// ------------------------------------------------------------------------------
// Comparison
// ------------------------------------------------------------------------------
TEST(CompareIp, CompareIp_Ipv4_Bigger) {
	unsigned char ipAddress1[IPV4_LENGTH] = {9, 8, 7, 6};
	unsigned char ipAddress2[IPV4_LENGTH] = {9, 8, 6, 6};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV4) > 0) << "Result should be positive "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ipv4_Smaller) {
	unsigned char ipAddress1[IPV4_LENGTH] = {3, 4, 5, 6};
	unsigned char ipAddress2[IPV4_LENGTH] = {3, 4, 6, 6};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV4) < 0) << "Result should be negative "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ipv4_Equal) {
	unsigned char ipAddress1[IPV4_LENGTH] = {3, 4, 5, 6};
	unsigned char ipAddress2[IPV4_LENGTH] = {3, 4, 5, 6};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV4) == 0) << "Result should be 0 "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ipv6_Bigger) {
	unsigned char ipAddress1[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	unsigned char ipAddress2[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 9, 9, 8, 7, 6, 5, 4, 2, 2, 1};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV6) > 0) << "Result should be positive "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ipv6_Smaller) {
	unsigned char ipAddress1[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	unsigned char ipAddress2[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 12, 9, 9, 8, 7, 6, 5, 4, 4, 2, 1};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV6) < 0) << "Result should be negative "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ipv6_Equal) {
	unsigned char ipAddress1[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	unsigned char ipAddress2[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_IPV6) == 0) << "Result should be 0 "
		"where IP address 1 is bigger than IP address 2\n";
}

TEST(CompareIp, CompareIp_Ip_Invalid) {
	unsigned char ipAddress1[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	unsigned char ipAddress2[IPV6_LENGTH] = 
		{16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

	EXPECT_TRUE(
		fiftyoneDegreesIpAddressesCompare(
			ipAddress1, 
			ipAddress2, 
			IP_TYPE_INVALID) == 0) << "Result should be 0 "
		"where type is invalid\n";
}
