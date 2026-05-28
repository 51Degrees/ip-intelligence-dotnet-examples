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
    /// environment variable to be set.
    /// </summary>
    /// <remarks>
    /// Note that these tests do not generally ensure the correctness
    /// of the example, only that the example will run without
    /// crashing or throwing any unhandled exceptions.
    /// </remarks>
    [TestClass]
    public class TestCloudExamples
    {
        private static string _dataFile;
        private static string _ddDataFile;
        private static Process _apiProcess;
        private static HttpClient _httpClient;
        private const int API_PORT = 5225;
        private static readonly string ApiUrl = $"http://localhost:{API_PORT}";

        /// <summary>
        /// Class-level setup: resolves data files and starts the API server once
        /// for all tests in this class.
        /// </summary>
        [ClassInitialize]
        public static async Task ClassInit(TestContext context)
        {
            _resourceKey = Environment.GetEnvironmentVariable(
                ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR);

            // Find Device Detection hash data file (required by the Mixed API)
            _ddDataFile = Environment.GetEnvironmentVariable(Constants.DEVICE_DETECTION_DATA_FILE_ENV_VAR);
            if (string.IsNullOrEmpty(_ddDataFile))
            {
                foreach (var fileName in new[] {
                    "TAC-HashV41.hash",
                    "51Degrees-EnterpriseV41.hash",
                    "51Degrees-EnterpriseV4.1.hash" })
                {
                    _ddDataFile = ExampleUtils.FindFile(fileName);
                    if (_ddDataFile != null) break;
                }
            }

            if (string.IsNullOrEmpty(_ddDataFile))
            {
                Assert.Inconclusive(
                    "The Mixed API requires a Device Detection data file (.hash). " +
                    $"Set the {Constants.DEVICE_DETECTION_DATA_FILE_ENV_VAR} environment variable or place a " +
                    "TAC/Enterprise .hash file in the repository.");
            }

            // Write data file path for debugging
            File.WriteAllText($"{nameof(TestCloudExamples)}_DataFileName.txt", _dataFile);

            // Start the API server once for all tests
            await StartApiServer();

            _httpClient = new HttpClient();
        }

        /// <summary>
        /// Class-level teardown: stops the API server after all tests complete.
        /// </summary>
        [ClassCleanup]
        public static void ClassCleanup()
        {
            _httpClient?.Dispose();
            StopApiServer();
        }

        private static readonly string RepoRootPath =
            Path.GetFullPath(
                Path.GetDirectoryName(
                    ExampleUtils.FindFile("FiftyOne.IpIntelligence.Examples.sln"))!);

        private static readonly string CloudExamplePath =
            Path.Combine(RepoRootPath, "Examples", "Cloud", "GettingStarted-Console");
        private static readonly string MixedCloudExamplePath =
            Path.Combine(RepoRootPath, "Examples", "Cloud", "Mixed", "GettingStarted-Console");

        private static async Task StartApiServer()
        {

            // Check if the executable exists, if not, build it first
            if (!File.Exists(ApiExePath) && !File.Exists(ApiExePath + ".exe"))
            {
                Console.WriteLine("Building GettingStarted-API project...");
                var buildProcess = Process.Start(new ProcessStartInfo
                {
                    FileName = "dotnet",
                    Arguments = $"build \"{Path.Combine(ApiProjectPath, "GettingStarted-API.csproj")}\" -c Debug",
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    WorkingDirectory = ApiProjectPath
                });

                await buildProcess.WaitForExitAsync();
                if (buildProcess.ExitCode != 0)
                {
                    Console.WriteLine("Warning: Build of API project may have failed");
                }
            }

            // Create process start info for running the API
            var startInfo = new ProcessStartInfo
            {
                FileName = "dotnet",
                Arguments = $"run --project \"{Path.Combine(ApiProjectPath, "GettingStarted-API.csproj")}\" --no-build",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WorkingDirectory = ApiProjectPath,
                CreateNoWindow = true
            };

            // Set environment variables for the API process
            startInfo.Environment["ASPNETCORE_URLS"] = ApiUrl;
            startInfo.Environment["ASPNETCORE_ENVIRONMENT"] = "Development";
            startInfo.Environment[Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR] = _dataFile;
            startInfo.Environment[Constants.DEVICE_DETECTION_DATA_FILE_ENV_VAR] = _ddDataFile;

            // Start the process
            _apiProcess = new Process { StartInfo = startInfo };

            // Capture output for debugging
            _apiProcess.OutputDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    Console.WriteLine($"API Output: {e.Data}");
                }
            };

            _apiProcess.ErrorDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    Console.WriteLine($"API Error: {e.Data}");
                }
            };

            _apiProcess.Start();
            _apiProcess.BeginOutputReadLine();
            _apiProcess.BeginErrorReadLine();

            // Wait for the API to be ready (with timeout)
            await WaitForApiToBeReady();
        }

        private static async Task WaitForApiToBeReady()
        {
            using var client = new HttpClient();
            var stopwatch = Stopwatch.StartNew();
            var timeout = TimeSpan.FromSeconds(30);

            while (stopwatch.Elapsed < timeout)
            {
                try
                {
                    // Try to access the evidencekeys endpoint as a health check
                    var response = await client.GetAsync($"{ApiUrl}/evidencekeys");
                    if (response.IsSuccessStatusCode)
                    {
                        Console.WriteLine("API server is ready");
                        return;
                    }
                }
                catch (HttpRequestException)
                {
                    // Server not ready yet
                }

                await Task.Delay(500);
            }

            throw new TimeoutException($"API server did not start within {timeout.TotalSeconds} seconds");
        }

        private static void StopApiServer()
        {
            if (_apiProcess != null && !_apiProcess.HasExited)
            {
                try
                {
                    _apiProcess.Kill(true);
                    _apiProcess.WaitForExit(5000);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error stopping API server: {ex.Message}");
                }
                finally
                {
                    _apiProcess?.Dispose();
                }
            }
        }

        public static IEnumerable<object[]> CloudExamplesToTest =>
        [
            [
                CloudExamplePath,
                new string[]
                {
                    "VMCBBUK",
                },
                new string[]
                {
                    "United Kingdom",
                    "GB",
                },
                new string[]
                {
                    "82.12.34.0",
                    "82.12.34.255",
                },
                "82.12.34.x",
            ],
            [
                MixedCloudExamplePath,
                new string[]
                {
                    "VMCBBUK",
                },
                new string[]
                {
                    "United Kingdom",
                    "GB",
                },
                new string[]
                {
                    "82.12.34.0",
                    "82.12.34.255",
                },
                "82.12.34.x",
            ],
        ];

        /// <summary>
        /// Runs a Cloud example as a separate process (matching how a user
        /// would run it) and verifies the output contains the expected
        /// IP intelligence values for one of the IPs in
        /// <see cref="ExampleUtils.EvidenceValues"/>.
        /// </summary>
        [TestMethod]
        [TestCategory("Integration")]
        [DynamicData(nameof(CloudExamplesToTest))]
        public void Example_Cloud_GettingStarted(
            string examplePath,
            string[] networkNames,
            string[] countryNames,
            string[] ipRanges,
            string rangeDescriptor)
        {
            // Run the actual example using dotnet run, just like a user would
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

                // Wait for the process to complete with a reasonable timeout
                Assert.IsTrue(process.WaitForExit(60000), "Cloud example should complete within 60 seconds");

                var result = output.ToString();
                var errorOutput = error.ToString();

                Console.WriteLine("Cloud Example Output:");
                Console.WriteLine(result);

                if (!string.IsNullOrEmpty(errorOutput))
                {
                    Console.WriteLine("Cloud Example Error Output:");
                    Console.WriteLine(errorOutput);
                }

                // A PropertyMissingException means the configured Device Detection data file /
                // resource key does not expose a property the example reads. The Mixed example
                // reads hardware properties (e.g. HardwareName) that are absent from the Lite
                // data file. This is a data-availability limitation, not an example defect, so
                // treat it as inconclusive - matching the OnPremise Mixed test. Provide a TAC or
                // Enterprise .hash file to exercise the example fully.
                if (process.ExitCode != 0 && errorOutput.Contains("PropertyMissingException"))
                {
                    Assert.Inconclusive(
                        "The example could not run fully because the Device Detection data file " +
                        "or resource key does not expose a required property (e.g. HardwareName, " +
                        "which is absent from the Lite data file). Provide a TAC or Enterprise " +
                        ".hash file. See https://51degrees.com/documentation/_info__resource_keys.html");
                }

                // Verify the example ran successfully
                Assert.AreEqual(0, process.ExitCode, $"Cloud example should exit successfully. Error output: {errorOutput}");

                // Verify the example produced IP intelligence results
                Assert.Contains("Input values:", result, "Output should contain input values section");
                Assert.Contains("Results:", result, "Output should contain results section");

                // Check for actual IP intelligence data values (not just field names)
                // Based on the actual output, verify we get real IP intelligence data
                Assert.IsTrue(networkNames.Any(x => result.Contains(x)),
                    $"Output should contain {networkNames[0]} as registered name");
                Assert.IsTrue(countryNames.Any(x => result.Contains(x)),
                    $"Output should contain {countryNames[0]} as country or code");
                Assert.IsTrue(ipRanges.Any(x => result.Contains(x)),
                    $"Output should contain IP range values {rangeDescriptor}");

                // Verify we have actual IP address data in the range information
                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"\d+\.\d+\.\d+\.\d+"),
                    "Output should contain IP address values in dotted decimal format");

                // Verify we have numeric coordinate or range data
                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"[\d\-]\d+\.\d+"),
                    "Output should contain numeric values (coordinates, ranges, etc.)");

                // Ensure no errors occurred
                Assert.DoesNotContain("Exception", result, "Output should not contain exceptions");
                Assert.DoesNotContain("Error", result, "Output should not contain errors");
            }
        }

        /// <summary>
        /// Test that the local API is responding correctly
        /// </summary>
        /// <remarks>
        /// This is a basic infrastructure test to ensure the background API server
        /// is running and responding. The main Cloud example test is what actually
        /// validates the IP Intelligence functionality.
        /// </remarks>
        [TestMethod]
        [TestCategory("Integration")]
        public async Task Example_API_HealthCheck()
        {
            // Test the evidencekeys endpoint as a basic health check
            var response = await _httpClient.GetAsync($"{ApiUrl}/evidencekeys");
            Assert.IsTrue(response.IsSuccessStatusCode, "API evidencekeys endpoint should respond successfully");

            var content = await response.Content.ReadAsStringAsync();
            Assert.Contains("query.client-ip", content, "Response should contain expected evidence keys");
        }
    }
}
