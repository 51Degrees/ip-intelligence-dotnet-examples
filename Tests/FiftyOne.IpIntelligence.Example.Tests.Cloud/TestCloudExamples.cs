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


// Ignore Spelling: ip API

using FiftyOne.IpIntelligence.Examples;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

[assembly: Parallelize]
namespace FiftyOne.IpIntelligence.Example.Tests.Cloud
{
    /// <summary>
    /// Runs the Cloud examples against the real 51Degrees Cloud service
    /// (cloud.51degrees.com by default). Requires the RESOURCE_KEY
    /// environment variable to be set; otherwise the tests are skipped as
    /// inconclusive.
    /// </summary>
    /// <remarks>
    /// Note that these tests do not generally ensure the correctness
    /// of the example, only that the example will run without
    /// crashing or throwing any unhandled exceptions.
    /// </remarks>
    [TestClass]
    public class TestCloudExamples
    {
        private string _resourceKey;

        [TestInitialize]
        public void Init()
        {
            _resourceKey = Environment.GetEnvironmentVariable(
                ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR);

            if (string.IsNullOrWhiteSpace(_resourceKey))
            {
                Assert.Inconclusive(
                    $"Skipping cloud test: environment variable " +
                    $"'{ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR}' is not set. " +
                    $"Obtain a resource key at https://configure.51degrees.com " +
                    $"and export it to run this test.");
            }
        }

        private static readonly string RepoRootPath =
            Path.GetFullPath(
                Path.GetDirectoryName(
                    ExampleUtils.FindFile("FiftyOne.IpIntelligence.Examples.sln"))!);

        private static readonly string CloudExamplePath =
            Path.Combine(RepoRootPath, "Examples", "Cloud", "GettingStarted-Console");
        private static readonly string MixedCloudExamplePath =
            Path.Combine(RepoRootPath, "Examples", "Cloud", "Mixed", "GettingStarted-Console");

        public static IEnumerable<object[]> CloudExamplesToTest =>
        [
            [
                CloudExamplePath,
                new string[]
                {
                    "CHINANET-GD",
                },
                new string[]
                {
                    "China",
                    "CN",
                    "cn",
                },
                new string[]
                {
                    "1.3.0.0",
                    "1.3.255.255",
                },
                "starting with 1.3.x",
            ],
            [
                MixedCloudExamplePath,
                new string[]
                {
                    "CHINANET-GD",
                },
                new string[]
                {
                    "China",
                    "CN",
                    "cn",
                },
                new string[]
                {
                    "1.3.0.0",
                    "1.3.255.255",
                },
                "starting with 1.3.x",
            ],
        ];

        /// <summary>
        /// Runs a Cloud example as a separate process (matching how a user
        /// would run it) and verifies the output contains the expected
        /// IP intelligence values for one of the IPs in
        /// <see cref="ExampleUtils.EvidenceValues"/>.
        /// </summary>
        [TestMethod]
        [DynamicData(nameof(CloudExamplesToTest))]
        public void Example_Cloud_GettingStarted(
            string examplePath,
            string[] networkNames,
            string[] countryNames,
            string[] ipRanges,
            string rangeDescriptor)
        {
            var startInfo = new ProcessStartInfo
            {
                FileName = "dotnet",
                Arguments = "run",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WorkingDirectory = examplePath,
                CreateNoWindow = true,
            };
            // Pass the resource key (and optional custom endpoint) through to
            // the example process; it picks them up via the same env vars.
            startInfo.Environment[ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR] = _resourceKey;
            var endPoint = Environment.GetEnvironmentVariable(
                ExampleUtils.CLOUD_END_POINT_ENV_VAR);
            if (string.IsNullOrWhiteSpace(endPoint) == false)
            {
                startInfo.Environment[ExampleUtils.CLOUD_END_POINT_ENV_VAR] = endPoint;
            }

            using (var process = new Process { StartInfo = startInfo })
            {
                var output = new StringBuilder();
                var error = new StringBuilder();

                process.OutputDataReceived += (sender, e) =>
                {
                    if (e.Data != null)
                        output.AppendLine(e.Data);
                };

                process.ErrorDataReceived += (sender, e) =>
                {
                    if (e.Data != null)
                        error.AppendLine(e.Data);
                };

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                Assert.IsTrue(process.WaitForExit(60000),
                    "Cloud example should complete within 60 seconds");

                var result = output.ToString();
                var errorOutput = error.ToString();

                Console.WriteLine("Cloud Example Output:");
                Console.WriteLine(result);

                if (!string.IsNullOrEmpty(errorOutput))
                {
                    Console.WriteLine("Cloud Example Error Output:");
                    Console.WriteLine(errorOutput);
                }

                Assert.AreEqual(0, process.ExitCode,
                    $"Cloud example should exit successfully. Error output: {errorOutput}");

                Assert.Contains("Input values:", result,
                    "Output should contain input values section");
                Assert.Contains("Results:", result,
                    "Output should contain results section");

                Assert.IsTrue(networkNames.Any(x => result.Contains(x)),
                    $"Output should contain {networkNames[0]} as registered name");
                Assert.IsTrue(countryNames.Any(x => result.Contains(x)),
                    $"Output should contain {countryNames[0]} as country or code");
                Assert.IsTrue(ipRanges.Any(x => result.Contains(x)),
                    $"Output should contain IP range values {rangeDescriptor}");

                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"\d+\.\d+\.\d+\.\d+"),
                    "Output should contain IP address values in dotted decimal format");

                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"[\d\-]\d+\.\d+"),
                    "Output should contain numeric values (coordinates, ranges, etc.)");

                Assert.DoesNotContain("Exception", result,
                    "Output should not contain exceptions");
                Assert.DoesNotContain("Error", result,
                    "Output should not contain errors");
            }
        }
    }
}
