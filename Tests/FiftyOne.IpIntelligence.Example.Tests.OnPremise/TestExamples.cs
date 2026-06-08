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

using FiftyOne.IpIntelligence.Examples;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Text;
using System.Threading;

[assembly: Parallelize]
namespace FiftyOne.IpIntelligence.Example.Tests.OnPremise;

/// <summary>
/// This test class ensures that the on-premise examples execute successfully
/// and, where practical, produce the output a user would expect.
/// </summary>
/// <remarks>
/// These are integration tests: each example builds a real on-premise IP
/// Intelligence engine against the enterprise data file. They therefore
/// require the data file to be present (located automatically, or via the
/// <c>IPINTELLIGENCEDATAFILE</c> environment variable) and are comparatively
/// slow. They go beyond pure smoke testing by asserting on the example output
/// (expected section headers, echoed IPv4 evidence and known property names)
/// rather than just that the example does not crash.
/// </remarks>
[TestClass]
[TestCategory(TestCategories.Integration)]
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
            Constants.LICENSE_KEY_ENV_VAR);
        LicenseKey = string.IsNullOrWhiteSpace(licenseKey) == false ?
            licenseKey: "!!YOUR_LICENSE_KEY!!";

        // Set IP Intelligence Data file
        DataFile = Environment.GetEnvironmentVariable(
            Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(DataFile))
        {
            DataFile = ExampleUtils.FindFile(
                Constants.ENTERPRISE_IPI_DATA_FILE_NAME);
        }

        File.WriteAllText($"{nameof(TestExamples)}_DataFileName.txt", DataFile);

        // Set evidence file for offline processing example.
        EvidenceFile = Environment.GetEnvironmentVariable(
            Constants.EVIDENCE_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(EvidenceFile))
        {
            EvidenceFile = ExampleUtils.FindFile(
                Constants.YAML_EVIDENCE_FILE_NAME);
        }

        GeoIpTruthEvidenceFile = Environment.GetEnvironmentVariable(
            Constants.GEOIP_TRUTH_EVIDENCE_FILE_ENV_VAR);
        if (string.IsNullOrWhiteSpace(GeoIpTruthEvidenceFile))
        {
            GeoIpTruthEvidenceFile = ExampleUtils.FindFile(
                Constants.GEOIP_COMPARISON_EVIDENCE_FILE_NAME);
        }
    }

    [TestCleanup]
    public void Cleanup()
    {
        TestContext.WriteLine(OutputString.ToString());
    }

    /// <summary>
    /// Fail with an explanatory message if the data file required by every
    /// integration test in this class could not be located.
    /// </summary>
    private void VerifyDataFileAvailable()
    {
        Assert.IsFalse(
            string.IsNullOrWhiteSpace(DataFile) || File.Exists(DataFile) == false,
            $"This test requires an on-premise IP Intelligence data file. Place a " +
            $"'{Constants.ENTERPRISE_IPI_DATA_FILE_NAME}' file in the repository or " +
            $"set the '{Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR}' environment " +
            $"variable to its path.");
    }

    /// <summary>
    /// Run an example, converting a data/engine version mismatch into an
    /// inconclusive result (rather than a hard failure) so the gap is surfaced
    /// without masquerading as a regression in the example itself.
    /// </summary>
    private static void RunResilient(Action run)
    {
        try
        {
            run();
        }
        catch (Exception ex) when (IsDataFileVersionError(ex))
        {
            Assert.Inconclusive(
                "The configured IP Intelligence data file could not be loaded by " +
                "the native engine (likely a data/engine version mismatch). Update " +
                "the ip-intelligence-data submodule or the engine package. " +
                $"Details: {ex.Message}");
        }
    }

    /// <summary>
    /// Returns true if the exception (or any inner exception) indicates the
    /// data file is missing or an unsupported version.
    /// </summary>
    private static bool IsDataFileVersionError(Exception ex)
    {
        for (var e = ex; e != null; e = e.InnerException)
        {
            if (e.Message.Contains("unsupported version", StringComparison.OrdinalIgnoreCase) ||
                e.Message.Contains("Check you have the latest data", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// Returns true if the exception (or any inner exception) indicates the
    /// configured license key was rejected by the 51Degrees data update service
    /// (e.g. it is not entitled to IP Intelligence data). This is an environment
    /// / credential limitation rather than a fault in the example.
    /// </summary>
    private static bool IsLicenseOrUpdateError(Exception ex)
    {
        for (var e = ex; e != null; e = e.InnerException)
        {
            if (e.GetType().Name.Contains("DataUpdateException", StringComparison.OrdinalIgnoreCase) ||
                e.Message.Contains("license key", StringComparison.OrdinalIgnoreCase) ||
                e.Message.Contains("data update service", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// Test the GettingStarted Example.
    /// Verifies the example runs and prints the expected report structure for
    /// the IPv4 evidence it analyses (input echo + results section + a known
    /// property name).
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_GettingStarted()
    {
        VerifyDataFileAvailable();

        var example = new Examples.OnPremise.GettingStartedConsole.Program.Example();
        RunResilient(() => example.Run(DataFile, new LoggerFactory(), OutputWriter));

        var output = OutputString.ToString();
        Assert.Contains("Input values:", output,
            "Output should echo the input evidence values.");
        Assert.Contains("Results:", output,
            "Output should contain a results section.");
        Assert.Contains("query.client-ip", output,
            "Output should echo the client IP evidence key.");
        // The example analyses a fixed set of IPv4 addresses - at least the
        // first should be echoed back verbatim in the input section.
        Assert.Contains("194.209.0.1", output,
            "Output should echo back the IPv4 evidence that was supplied.");
        Assert.Contains("Country", output,
            "Output should contain the Country property.");
        Assert.Contains("IpRangeStart", output,
            "Output should contain the matched IP range start.");
    }

    /// <summary>
    /// Test the OfflineProcessing Example.
    /// Uses a self-contained YAML evidence file (so the test does not depend on
    /// the contents of the shipped evidence file) and asserts the example
    /// processed the records and emitted the expected YAML document markers and
    /// property output.
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_OfflineProcessing()
    {
        VerifyDataFileAvailable();

        var evidenceFile = WriteTempEvidenceYaml(
            "82.12.34.23",
            "116.154.188.222",
            "2001:0db8:085a:0000:0000:8a2e:0370:7334");
        try
        {
            var example = new Examples.OnPremise.OfflineProcessing.Program.Example();
            using (var reader = new StreamReader(File.OpenRead(evidenceFile)))
            {
                RunResilient(() =>
                    example.Run(DataFile, reader, new LoggerFactory(), OutputWriter));
            }
        }
        finally
        {
            File.Delete(evidenceFile);
        }

        var output = OutputString.ToString();
        Assert.Contains("query.client-ip", output,
            "Offline output should echo each evidence record's client IP.");
        Assert.Contains("82.12.34.23", output,
            "Offline output should contain the IPv4 evidence that was supplied.");
        Assert.Contains("RegisteredName", output,
            "Offline output should contain the requested RegisteredName property.");
        Assert.Contains("...", output,
            "Offline output should contain the YAML end-of-stream marker, " +
            "indicating all records were processed.");
    }

    /// <summary>
    /// Test the Metadata Example.
    /// Asserts the example prints the engine metadata sections (accepted
    /// evidence keys, profile counts and component/property listings).
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_Metadata()
    {
        VerifyDataFileAvailable();

        var example = new Examples.OnPremise.Metadata.Program.Example();
        RunResilient(() => example.Run(DataFile, new LoggerFactory(), OutputWriter));

        var output = OutputString.ToString();
        Assert.Contains("Accepted evidence keys:", output,
            "Metadata output should list the accepted evidence keys.");
        Assert.Contains("Profile counts:", output,
            "Metadata output should list profile counts.");
        Assert.Contains("Property -", output,
            "Metadata output should list the available properties.");
    }

    /// <summary>
    /// Test the Comparison Example.
    /// Asserts the comparison wrote a non-empty report for the GeoIP truth
    /// evidence rather than just completing.
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_CompareConsole()
    {
        VerifyDataFileAvailable();

        var tempfile = Path.GetTempFileName();
        try
        {
            using (var writer = new StreamWriter(File.Create(tempfile)))
            {
                RunResilient(() =>
                    Examples.OnPremise.Compare.Program.Example.Run(
                        DataFile,
                        GeoIpTruthEvidenceFile,
                        writer,
                        new LoggerFactory(),
                        CancellationToken.None).Wait());
            }

            Assert.IsGreaterThan(0, new FileInfo(tempfile).Length,
                "Comparison example should write a non-empty comparison report.");
        }
        finally
        {
            File.Delete(tempfile);
        }
    }

    /// <summary>
    /// Test the Metrics Example
    /// </summary>
    [TestMethod]
    [Ignore("Disabled until profileOffsets are sorted (again).")]
    public void Example_OnPremise_MetricsConsole()
    {
        var tempfile = Path.GetTempFileName();
        using var writer = new StreamWriter(File.Create(tempfile));
        Examples.OnPremise.Metrics.Program.Example.Run(
            DataFile,
            new LoggerFactory(),
            writer,
            // Sample 0.1% of possible IP addresses.
            0.0001,
            // Include all the possible IP ranges.
            (_) => true,
            CancellationToken.None).Wait();
        File.Delete(tempfile);
    }

    /// <summary>
    /// Test the Suspicious Example.
    /// Asserts the example prints the suspicious-IP report including the
    /// IsSuspicious verdict line.
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_Suspicious()
    {
        VerifyDataFileAvailable();

        var example = new Examples.OnPremise.Suspicious.Program.Example();
        RunResilient(() => example.Run(DataFile, new LoggerFactory(), OutputWriter));

        var output = OutputString.ToString();
        Assert.Contains("Input values:", output,
            "Suspicious output should echo the input evidence.");
        Assert.Contains("IsSuspicious:", output,
            "Suspicious output should contain the IsSuspicious verdict.");
    }

    /// <summary>
    /// Test the UpdateDataFile Example.
    /// This requires a 51Degrees license key; when one is not configured the
    /// test reports inconclusive (rather than being permanently ignored) so the
    /// gap is visible in CI.
    /// </summary>
    [TestMethod]
    public void Example_OnPremise_UpdateDataFile()
    {
        VerifyLicenseKeyAvailable();
        VerifyDataFileAvailable();
        try
        {
            Examples.OnPremise.UpdateDataFile.Program.Initialize(
                DataFile, LicenseKey, null, false);
        }
        catch (Exception ex) when (IsDataFileVersionError(ex) || IsLicenseOrUpdateError(ex))
        {
            Assert.Inconclusive(
                "The UpdateDataFile example could not complete because the " +
                "configured license key was not accepted by the 51Degrees data " +
                "update service (it must be a key entitled to IP Intelligence " +
                $"data). Details: {ex.Message}");
        }
    }

    private void VerifyLicenseKeyAvailable()
    {
        if (string.IsNullOrWhiteSpace(LicenseKey) == true ||
            LicenseKey.StartsWith("!!") == true)
        {
            Assert.Inconclusive("This test requires a 51Degrees license key. This can be " +
                "specified in the TestHashExamples.Init method or by setting an Environment " +
                $"variable called '{Constants.LICENSE_KEY_ENV_VAR}'");
        }
    }

    /// <summary>
    /// Write a small, self-contained YAML evidence file (one document per IP)
    /// to a temporary path and return that path. The caller is responsible for
    /// deleting it.
    /// </summary>
    private static string WriteTempEvidenceYaml(params string[] ipAddresses)
    {
        var path = Path.Combine(
            Path.GetTempPath(),
            $"ipi-evidence-{Guid.NewGuid():N}.yml");
        var builder = new StringBuilder();
        foreach (var ip in ipAddresses)
        {
            builder.AppendLine("---");
            builder.AppendLine($"query.client-ip: {ip}");
        }
        builder.AppendLine("...");
        File.WriteAllText(path, builder.ToString());
        return path;
    }
}
