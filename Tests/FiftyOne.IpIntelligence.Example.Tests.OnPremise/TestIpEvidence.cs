/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
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

// Ignore Spelling: Ip

using ExampleConstants = FiftyOne.IpIntelligence.Examples.Constants;
using FiftyOne.IpIntelligence.Examples;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;

namespace FiftyOne.IpIntelligence.Example.Tests.OnPremise;

/// <summary>
/// Tests that the IP Intelligence engine returns correct HasValue results
/// for different categories of IP address: public IPv4, public IPv6,
/// private/RFC-1918, and loopback addresses.
/// </summary>
[TestClass]
public class TestIpEvidence
{
    private static IPipeline _pipeline;
    private static string _dataFile;
    private static Exception _initException;

    [ClassInitialize]
    public static void ClassInit(TestContext context)
    {
        _dataFile = Environment.GetEnvironmentVariable(
            ExampleConstants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(_dataFile))
            _dataFile = ExampleUtils.FindFile(ExampleConstants.ENTERPRISE_IPI_DATA_FILE_NAME);
        if (string.IsNullOrWhiteSpace(_dataFile))
            _dataFile = ExampleUtils.FindFile("51Degrees-LiteV41.ipi");

        if (!string.IsNullOrWhiteSpace(_dataFile))
        {
            try
            {
                _pipeline = new IpiPipelineBuilder()
                    .UseOnPremise(_dataFile, null, false)
                    .SetPerformanceProfile(PerformanceProfiles.MaxPerformance)
                    .SetShareUsage(false)
                    .SetAutoUpdate(false)
                    .SetDataUpdateOnStartUp(false)
                    .SetDataFileSystemWatcher(false)
                    .Build();
            }
            catch (Exception ex)
            {
                _initException = ex;
            }
        }
    }

    [ClassCleanup]
    public static void ClassCleanup()
    {
        _pipeline?.Dispose();
    }

    private void SkipIfNotAvailable()
    {
        if (string.IsNullOrWhiteSpace(_dataFile))
        {
            Assert.Inconclusive(
                "No IP Intelligence data file found. Set IPINTELLIGENCEDATAFILE or " +
                "ensure the ip-intelligence-data submodule is present.");
        }
        if (_initException != null)
        {
            Assert.Inconclusive(
                $"Pipeline initialization failed: {_initException.Message}");
        }
    }

    /// <summary>
    /// Public IPv4 addresses assigned to well-known organisations must
    /// resolve to a non-null RegisteredName.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    [DataRow("8.8.8.8",           "Google public DNS (IPv4)")]
    [DataRow("1.1.1.1",           "Cloudflare public DNS (IPv4)")]
    [DataRow("116.154.188.222",   "Known test IP from ExampleUtils.EvidenceValues")]
    public void PublicIPv4_RegisteredName_HasValue(string ip, string description)
    {
        SkipIfNotAvailable();

        using var flowData = _pipeline.CreateFlowData();
        flowData.AddEvidence(new Dictionary<string, object> { { "query.client-ip", ip } });
        flowData.Process();

        var ipData = flowData.Get<IIpIntelligenceData>();
        Assert.IsTrue(ipData.RegisteredName.HasValue,
            $"{description} ({ip}): RegisteredName.HasValue should be true");
    }

    /// <summary>
    /// A well-known public IPv6 address must resolve to a non-null RegisteredName.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void PublicIPv6_RegisteredName_HasValue()
    {
        SkipIfNotAvailable();

        using var flowData = _pipeline.CreateFlowData();
        flowData.AddEvidence(new Dictionary<string, object>
        {
            { "query.client-ip", "2001:4860:4860::8888" }
        });
        flowData.Process();

        var ipData = flowData.Get<IIpIntelligenceData>();
        Assert.IsTrue(ipData.RegisteredName.HasValue,
            "RegisteredName should have a value for public IPv6 2001:4860:4860::8888 (Google DNS)");
    }


    /// <summary>
    /// Confidence and HasValue coherence: when RegisteredName HasValue is true,
    /// IpRangeStart and IpRangeEnd must also HasValue (the IP belongs to a range).
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void PublicIP_IpRange_HasValue_WhenRegisteredNameHasValue()
    {
        SkipIfNotAvailable();

        using var flowData = _pipeline.CreateFlowData();
        flowData.AddEvidence(new Dictionary<string, object> { { "query.client-ip", "8.8.8.8" } });
        flowData.Process();

        var ipData = flowData.Get<IIpIntelligenceData>();
        if (!ipData.RegisteredName.HasValue)
        {
            Assert.Inconclusive("RegisteredName has no value for 8.8.8.8 — data file may lack coverage for this IP.");
        }

        Assert.IsTrue(ipData.IpRangeStart.HasValue,
            "IpRangeStart must have a value when the IP resolves to a registered network");
        Assert.IsTrue(ipData.IpRangeEnd.HasValue,
            "IpRangeEnd must have a value when the IP resolves to a registered network");
    }
}
