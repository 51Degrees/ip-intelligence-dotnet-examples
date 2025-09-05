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

// ------------------------------------------------------------------------------
// Simple IPv4 tests
// ------------------------------------------------------------------------------
//
//TEST(ParseIpHeader, ParseEvidence_Ipv4_Simple)
//{
//	char* ip = "1.2.3.4";
//	fiftyoneDegreesIpAddressEvidence* result = parseIpAddresses(malloc, ip);
//	EXPECT_STREQ("1.2.3.4", result->address) <<
//		L"Expected result to be '1.2.3.4'";
//	EXPECT_EQ(NULL, result[1]) <<
//		L"Expected only one result";
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_SpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4 5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2]),
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_CommaSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4,5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_CommaAndSpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4, 5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_CIDRFormat)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4/32");
//	EXPECT_EQ("1.2.3.4/32", result[0],
//		L"Expected first result to be '1.2.3.4/32'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_PortNumber)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4:80");
//	EXPECT_EQ("1.2.3.4:80", result[0],
//		L"Expected first result to be '1.2.3.4:80'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv4_Mixed)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("1.2.3.4, 5.6.7.8/32,9.10.11.12:80");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8/32", result[1],
//		L"Expected second result to be '5.6.7.8/32'");
//	EXPECT_EQ("9.10.11.12:80", result[2],
//		L"Expected third result to be '9.10.11.12:80'");
//	EXPECT_EQ(NULL,result[3],
//		L"Expected only three results");
//}
//// ------------------------------------------------------------------------------
//
//
//// ------------------------------------------------------------------------------
//// Simple IPv6 tests
//// ------------------------------------------------------------------------------
//TEST(ParseIpHeader, ParseEvidence_Ipv6_Simple)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0:50:abcd:ef01:1");
//	EXPECT_EQ("2001:db8:a0b:12f0:50:abcd:ef01:1", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0:50:abcd:ef01:1'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_Abbreviated)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_CIDRFormat)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::/128");
//	EXPECT_EQ("2001:db8:a0b:12f0::/128", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::/128'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_PortNumber)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("[2001:db8:a0b:12f0::]:80");
//	EXPECT_EQ("[2001:db8:a0b:12f0::]:80", result[0],
//		L"Expected first result to be '[2001:db8:a0b:12f0::]:80'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_SpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0:: 2001::802:b48a");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_CommaSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::,2001::802:b48a");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_CommaAndSpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::, 2001::802:b48a");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_Mixed)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::/128 [2001::802:b48a]:80,FFFF:BBB1::");
//	EXPECT_EQ("2001:db8:a0b:12f0::/128", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::/128'");
//	EXPECT_EQ("[2001::802:b48a]:80", result[1],
//		L"Expected second result to be '[2001::802:b48a]:80'");
//	EXPECT_EQ("FFFF:BBB1::", result[2],
//		L"Expected third result to be 'FFFF:BBB1::'");
//	EXPECT_EQ(NULL,result[3],
//		L"Expected only three results");
//}
//// ------------------------------------------------------------------------------
//
//
//// ------------------------------------------------------------------------------
//// Unusual data
//// ------------------------------------------------------------------------------
//TEST(ParseIpHeader, ParseEvidence_Ipv6_Null)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader(NULL);
//	EXPECT_EQ(NULL,result[0],
//		L"Expected no results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_EmptyString)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("");
//	EXPECT_EQ(NULL,result[0],
//		L"Expected no results");
//}
//TEST(ParseIpHeader, ParseEvidence_Ipv6_MassiveString)
//{
//	bool exception = false;
//	try
//	{
//		char** result = fiftyoneDegreesIpAddressParseHeader((char*)malloc(1000000));
//	}
//	catch (const std::exception&)
//	{
//		exception = true;
//	}
//	Assert::IsTrue(exception,
//		L"Expected exception");
//}
//
//// ------------------------------------------------------------------------------
//// Mixed IPv4 IPv6
//// ------------------------------------------------------------------------------
//
//TEST(ParseIpHeader, ParseEvidence_Ipv4And6_Mixed)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("2001:db8:a0b:12f0::/128 1.2.3.4:80,FFFF:BBB1::");
//	EXPECT_EQ("2001:db8:a0b:12f0::/128", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::/128'");
//	EXPECT_EQ("1.2.3.4:80", result[1],
//		L"Expected second result to be '1.2.3.4:80'");
//	EXPECT_EQ("FFFF:BBB1::", result[2],
//		L"Expected third result to be 'FFFF:BBB1::'");
//	EXPECT_EQ(NULL,result[3],
//		L"Expected only three results");
//}
//
//
//
//// ------------------------------------------------------------------------------
//// Forwarded header IPv4
//// ------------------------------------------------------------------------------
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_Simple)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected result to be '1.2.3.4'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_SpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4 for=5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_CommaSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4,for=5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_CommaAndSpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4, for=5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_CIDRFormat)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"1.2.3.4/32\"");
//	EXPECT_EQ("1.2.3.4/32", result[0],
//		L"Expected first result to be '1.2.3.4/32'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_PortNumber)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"1.2.3.4:80\"");
//	EXPECT_EQ("1.2.3.4:80", result[0],
//		L"Expected first result to be '1.2.3.4:80'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv4_AdditionalFields)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4;proto=http;by=5.6.7.8;host=me.net");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected second result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//// ------------------------------------------------------------------------------
//// Forwarded header IPv6
//// ------------------------------------------------------------------------------
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_Simple)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0:50:abcd:ef01:1\"");
//	EXPECT_EQ("2001:db8:a0b:12f0:50:abcd:ef01:1", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0:50:abcd:ef01:1'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_Abbreviated)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::\"");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_CIDRFormat)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::/128\"");
//	EXPECT_EQ("2001:db8:a0b:12f0::/128", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::/128'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_PortNumber)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"[2001:db8:a0b:12f0::]:80\"");
//	EXPECT_EQ("[2001:db8:a0b:12f0::]:80", result[0],
//		L"Expected first result to be '[2001:db8:a0b:12f0::]:80'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_SpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::\" for=\"2001::802:b48a\"");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_CommaSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::\",for=\"2001::802:b48a\"");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_CommaAndSpaceSeparator)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::\", for=\"2001::802:b48a\"");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ("2001::802:b48a", result[1],
//		L"Expected second result to be '2001::802:b48a'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_Ipv6_AdditionalFields)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=\"2001:db8:a0b:12f0::\";proto=https;by=\"2001::802:b48a\";host=www.site.com");
//	EXPECT_EQ("2001:db8:a0b:12f0::", result[0],
//		L"Expected first result to be '2001:db8:a0b:12f0::'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//// ------------------------------------------------------------------------------
//// Forwarded header extras
//// ------------------------------------------------------------------------------
//TEST(ParseIpHeader, ParseEvidence_Forwarded_InvalidButCommon)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=[2a02:c7d:c64:2d00:106c:6508:1652:51dd]\"\"");
//	EXPECT_EQ("2a02:c7d:c64:2d00:106c:6508:1652:51dd", result[0],
//		L"Expected first result to be '2a02:c7d:c64:2d00:106c:6508:1652:51dd'");
//	EXPECT_EQ(NULL,result[1],
//		L"Expected only one result");
//}
//TEST(ParseIpHeader, ParseEvidence_Forwarded_AnnonymisingToken)
//{
//	char** result = fiftyoneDegreesIpAddressParseHeader("for=1.2.3.4, for=\"_gazonk\", for=5.6.7.8");
//	EXPECT_EQ("1.2.3.4", result[0],
//		L"Expected first result to be '1.2.3.4'");
//	EXPECT_EQ("5.6.7.8", result[1],
//		L"Expected first result to be '5.6.7.8'");
//	EXPECT_EQ(NULL,result[2],
//		L"Expected only two results");
//}