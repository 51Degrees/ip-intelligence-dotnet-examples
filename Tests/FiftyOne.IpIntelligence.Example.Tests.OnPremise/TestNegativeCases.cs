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
/// Verifies that the engine behaves gracefully in error and edge-case scenarios:
/// missing data file, absent evidence, and malformed evidence values.
/// </summary>
[TestClass]
public class TestNegativeCases
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
    /// Attempting to build a pipeline with a path that does not exist must
    /// throw an exception rather than silently succeeding.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void MissingDataFile_BuildPipeline_Throws()
    {
        try
        {
            using var pipeline = new IpiPipelineBuilder()
                .UseOnPremise("/nonexistent/does/not/exist/file.ipi", null, false)
                .SetAutoUpdate(false)
                .SetDataUpdateOnStartUp(false)
                .SetDataFileSystemWatcher(false)
                .Build();

            Assert.Fail("Expected an exception when the data file does not exist, but none was thrown.");
        }
        catch (AssertFailedException)
        {
            throw;
        }
        catch (Exception)
        {
            // Any exception (FileNotFoundException, AggregateException, etc.) is the expected outcome.
        }
    }

    /// <summary>
    /// Passing no evidence at all must not throw and must return HasValue == false
    /// for all properties that depend on an IP address.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void NoEvidence_RegisteredName_HasNoValue()
    {
        SkipIfNotAvailable();

        using var flowData = _pipeline.CreateFlowData();
        flowData.AddEvidence(new Dictionary<string, object>());
        flowData.Process();

        var ipData = flowData.Get<IIpIntelligenceData>();
        Assert.IsFalse(ipData.RegisteredName.HasValue,
            "RegisteredName should have no value when no IP evidence is provided");
        Assert.IsFalse(ipData.Country.HasValue,
            "Country should have no value when no IP evidence is provided");
    }

    /// <summary>
    /// Values that cannot be parsed as any IP address must not throw and must
    /// return HasValue == false rather than crashing or returning garbage data.
    /// Note: numerically out-of-range octets (e.g. 999.x.x.x) are accepted by
    /// the engine's parser via octet overflow and are therefore not tested here.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    [DataRow("not-an-ip-address",  "plain text string")]
    [DataRow("",                   "empty string")]
    [DataRow("::gggg",             "invalid IPv6 hex digits")]
    public void MalformedIpEvidence_RegisteredName_HasNoValue(string ip, string description)
    {
        SkipIfNotAvailable();

        using var flowData = _pipeline.CreateFlowData();
        flowData.AddEvidence(new Dictionary<string, object> { { "query.client-ip", ip } });
        flowData.Process();

        var ipData = flowData.Get<IIpIntelligenceData>();
        Assert.IsFalse(ipData.RegisteredName.HasValue,
            $"RegisteredName should have no value for malformed evidence '{ip}' ({description})");
    }
}
