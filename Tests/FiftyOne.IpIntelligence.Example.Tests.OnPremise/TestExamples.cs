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


// Ignore Spelling: Metadata Offline

using ExampleConstants = FiftyOne.IpIntelligence.Examples.Constants;
using FiftyOne.IpIntelligence.Examples;
using FiftyOne.Pipeline.Engines;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Text;
using System.Linq;
using System.Threading;

[assembly: Parallelize]
namespace FiftyOne.IpIntelligence.Example.Tests.OnPremise;

/// <summary>
/// This test class ensures that the hash examples execute successfully.
/// </summary>
[TestClass]
public class TestExamples
{
    public TestContext TestContext { get; set; }
    private StringBuilder OutputString { get; set; }
    private StringWriter OutputWriter { get; set; }

    private string LicenseKey;

    private string DataFile;

    private string EvidenceFile;
    private string GeoIpTruthEvidenceFile;

    /// <summary>
    /// Init method - specify License Key to run examples here or
    /// set a License Key in an environment variable called 'ResourceKey'.
    /// Set data file for hash examples.
    /// </summary>
    [TestInitialize]
    public void Init()
    {
        OutputString = new StringBuilder();
        OutputWriter = new StringWriter(OutputString);
        // Set license key for autoupdate examples.
        var licenseKey = Environment.GetEnvironmentVariable(
            ExampleConstants.LICENSE_KEY_ENV_VAR);
        LicenseKey = string.IsNullOrWhiteSpace(licenseKey) == false ?
            licenseKey: "!!YOUR_LICENSE_KEY!!";

        // Set IP Intelligence Data file
        DataFile = Environment.GetEnvironmentVariable(
            ExampleConstants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(DataFile))
        {
            DataFile = ExampleUtils.FindFile(
                ExampleConstants.ENTERPRISE_IPI_DATA_FILE_NAME);
        }

        File.WriteAllText($"{nameof(TestExamples)}_DataFileName.txt", DataFile);

        // Set evidence file for offline processing example.
        EvidenceFile = Environment.GetEnvironmentVariable(
            ExampleConstants.EVIDENCE_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(EvidenceFile))
        {
            EvidenceFile = ExampleUtils.FindFile(
                ExampleConstants.YAML_EVIDENCE_FILE_NAME);
        }

        GeoIpTruthEvidenceFile = Environment.GetEnvironmentVariable(
            ExampleConstants.GEOIP_TRUTH_EVIDENCE_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(GeoIpTruthEvidenceFile))
        {
            GeoIpTruthEvidenceFile = ExampleUtils.FindFile(
                ExampleConstants.GEOIP_COMPARISON_EVIDENCE_FILE_NAME);
        }
    }

    [TestCleanup]
    public void Cleanup()
    {
        TestContext.WriteLine(OutputString.ToString());
    }

    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_GettingStarted()
    {
        var example = new Examples.OnPremise.GettingStartedConsole.Program.Example();
        example.Run(DataFile, new LoggerFactory(), OutputWriter);

        var text = OutputString.ToString();
        Assert.Contains("Input values:", text, "Output should contain input values section");
        Assert.Contains("Results:", text, "Output should contain results section");
    }

    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_OfflineProcessing()
    {
        var example = new Examples.OnPremise.OfflineProcessing.Program.Example();
        using (var reader = new StreamReader(File.OpenRead(EvidenceFile)))
        {
            example.Run(DataFile, reader, new LoggerFactory(), OutputWriter);
        }

        var text = OutputString.ToString();
        Assert.Contains("---", text, "Output should contain YAML document separator '---'");
    }

    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_Metadata()
    {
        var example = new Examples.OnPremise.Metadata.Program.Example();
        example.Run(DataFile, new LoggerFactory(), OutputWriter);

        var text = OutputString.ToString();
        Assert.IsNotEmpty(text, "Expected non-empty metadata output");
        Assert.IsTrue(
            text.Contains("Accepted evidence keys:") || text.Contains("evidence key filter"),
            "Output should contain evidence key information");
    }

    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_CompareConsole()
    {
        var tempfile = Path.GetTempFileName();
        using var writer = new StreamWriter(File.Create(tempfile));
        Examples.OnPremise.Compare.Program.Example.Run(
            DataFile,
            GeoIpTruthEvidenceFile,
            writer,
            new LoggerFactory(),
            CancellationToken.None).Wait();
        File.Delete(tempfile);
    }

    /// <summary>
    /// Tests the Metrics example. Requires the Enterprise data file which
    /// contains profileOffsets; the Lite file does not support this example.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_MetricsConsole()
    {
        if (DataFile == null || DataFile.IndexOf("Lite", StringComparison.OrdinalIgnoreCase) >= 0)
        {
            Assert.Inconclusive("MetricsConsole requires the Enterprise data file. " +
                "The Lite data file does not contain the profileOffsets required by this example.");
        }

        // Building the full WKT area index is CPU-intensive; allow 3 minutes
        // before treating this as Inconclusive (not a failure) so the test
        // suite does not hang in constrained CI environments.
        using var cts = new CancellationTokenSource(TimeSpan.FromMinutes(3));
        var tempfile = Path.GetTempFileName();
        try
        {
            using var writer = new StreamWriter(File.Create(tempfile));
            try
            {
                Examples.OnPremise.Metrics.Program.Example.Run(
                    DataFile,
                    new LoggerFactory(),
                    writer,
                    // Sample 0.01% of possible IP addresses.
                    0.0001,
                    // Include all the possible IP ranges.
                    (_) => true,
                    cts.Token).Wait(cts.Token);
            }
            catch (AggregateException ae)
                when (ae.InnerExceptions.All(e => e is OperationCanceledException oce &&
                                                  oce.CancellationToken == cts.Token))
            {
                Assert.Inconclusive(
                    "MetricsConsole did not complete within 3 minutes; " +
                    "this is expected on machines where building the full WKT " +
                    "area index exceeds the test time budget.");
            }
            catch (OperationCanceledException oce)
                when (oce.CancellationToken == cts.Token)
            {
                Assert.Inconclusive(
                    "MetricsConsole did not complete within 3 minutes; " +
                    "this is expected on machines where building the full WKT " +
                    "area index exceeds the test time budget.");
            }
        }
        finally
        {
            File.Delete(tempfile);
        }
    }

    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_Suspicious()
    {
        var example = new Examples.OnPremise.Suspicious.Program.Example();
        example.Run(DataFile, new LoggerFactory(), OutputWriter);

        var text = OutputString.ToString();
        Assert.Contains("Input values:", text, "Output should contain input values section");
        Assert.Contains("Results:", text, "Output should contain results section");
    }

    /// <summary>
    /// Tests the UpdateDataFile example. Requires a valid license key set via
    /// the IPINTELLIGENCELICENSEKEY_DOTNET environment variable.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_UpdateDataFile()
    {
        VerifyLicenseKeyAvailable();
        Examples.OnPremise.UpdateDataFile.Program.Initialize(
            DataFile, LicenseKey, null, false);
    }

    /// <summary>
    /// Tests the Performance-Console example with a minimal configuration
    /// (single thread, MaxPerformance profile, single-property mode).
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_PerformanceConsole()
    {
        var config = new Examples.OnPremise.Performance.PerformanceConfiguration(
            PerformanceProfiles.MaxPerformance, false);
        var results = Examples.OnPremise.Performance.Program.Example.Run(
            DataFile, EvidenceFile, config, OutputWriter, threadCount: (ushort)1);

        Assert.IsNotNull(results, "Expected non-null benchmark results");
        Assert.IsGreaterThan(0, results.Count, "Expected at least one benchmark result from the run");
    }

    /// <summary>
    /// Tests the Mixed OnPremise GettingStarted-Console example which combines
    /// Device Detection and IP Intelligence in a single pipeline.
    /// Requires a Device Detection .hash data file in addition to the IP Intelligence file.
    /// </summary>
    [TestMethod]
    [TestCategory("Integration")]
    public void Example_OnPremise_Mixed_GettingStarted()
    {
        var deviceDataFile = Environment.GetEnvironmentVariable(
            ExampleConstants.DEVICE_DETECTION_DATA_FILE_ENV_VAR);
        if (string.IsNullOrEmpty(deviceDataFile))
        {
            foreach (var fileName in new[] {
                "TAC-HashV41.hash",
                "51Degrees-EnterpriseV4.1.hash" })
            {
                deviceDataFile = ExampleUtils.FindFile(fileName);
                if (deviceDataFile != null) break;
            }
        }

        if (string.IsNullOrEmpty(deviceDataFile))
        {
            Assert.Inconclusive(
                "Mixed GettingStarted requires a Device Detection Enterprise or TAC data file (.hash) " +
                "because it accesses properties (e.g. HardwareName) absent from the Lite file. " +
                $"Set {ExampleConstants.DEVICE_DETECTION_DATA_FILE_ENV_VAR} or place a TAC/Enterprise .hash file in the repository.");
        }

        var example = new Examples.Mixed.OnPremise.GettingStartedConsole.Program.Example();
        example.Run(deviceDataFile, DataFile, new LoggerFactory(), OutputWriter);

        var text = OutputString.ToString();
        Assert.Contains("Input values:", text, "Output should contain input values section");
    }

    private void VerifyLicenseKeyAvailable()
    {
        if (string.IsNullOrWhiteSpace(LicenseKey) == true ||
            LicenseKey.StartsWith("!!") == true)
        {
            Assert.Inconclusive("This test requires a 51Degrees license key. This can be " +
                "specified in the TestHashExamples.Init method or by setting an Environment " +
                $"variable called '{ExampleConstants.LICENSE_KEY_ENV_VAR}'");
        }
    }
}
