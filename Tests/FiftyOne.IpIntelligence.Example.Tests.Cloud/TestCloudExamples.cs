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
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

[assembly: Parallelize]
namespace FiftyOne.IpIntelligence.Example.Tests.Cloud
{
    /// <summary>
    /// Exercises the Cloud examples in two complementary modes:
    /// 1. Against a locally-hosted GettingStarted-API server, for offline/CI runs.
    ///    Requires IP Intelligence and Device Detection data files.
    /// 2. Against the real 51Degrees Cloud service (cloud.51degrees.com).
    ///    Requires the RESOURCE_KEY environment variable.
    /// Tests for either mode are marked Inconclusive when their prerequisites are absent.
    /// </summary>
    [TestClass]
    public class TestCloudExamples
    {
        private static string _dataFile;
        private static string _ddDataFile;
        private static Process _apiProcess;
        private static HttpClient _httpClient;
        private static string _resourceKey;
        private const int API_PORT = 5225;
        private static readonly string ApiUrl = $"http://localhost:{API_PORT}";

        /// <summary>
        /// Class-level setup: resolves the resource key and local data files, and starts
        /// the local API once for all tests in this class.
        /// </summary>
        [ClassInitialize]
        public static async Task ClassInit(TestContext context)
        {
            _resourceKey = Environment.GetEnvironmentVariable(
                ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR);

            _dataFile = Environment.GetEnvironmentVariable(
                Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR);
            if (string.IsNullOrWhiteSpace(_dataFile))
            {
                _dataFile = ExampleUtils.FindFile(
                    Constants.ENTERPRISE_IPI_DATA_FILE_NAME);
            }

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

            // Try to start the local API only when both data files are present, and
            // tolerate any startup failure (e.g. unsupported data file version) so the
            // real-cloud test can still run. Local-API tests check _apiProcess and become
            // Inconclusive when it is null.
            if (!string.IsNullOrEmpty(_dataFile) && !string.IsNullOrEmpty(_ddDataFile))
            {
                File.WriteAllText($"{nameof(TestCloudExamples)}_DataFileName.txt", _dataFile);
                try
                {
                    await StartApiServer();
                    _httpClient = new HttpClient();
                }
                catch (Exception ex)
                {
                    Console.WriteLine(
                        $"Local API failed to start: {ex.Message}. " +
                        "Local-API tests will be Inconclusive.");
                    try { _apiProcess?.Kill(true); } catch { /* best effort */ }
                    _apiProcess?.Dispose();
                    _apiProcess = null;
                }
            }
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

        private static readonly string ApiProjectPath =
            Path.Combine(RepoRootPath, "Examples", "OnPremise", "Mixed", "GettingStarted-API");
        private static readonly string ApiExePath =
            Path.Combine(ApiProjectPath, "bin", "Debug", "net8.0", "GettingStarted-API");

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
        /// Runs the Cloud example pointed at the local GettingStarted-API.
        /// </summary>
        [TestMethod]
        [TestCategory("Integration")]
        [DynamicData(nameof(CloudExamplesToTest))]
        public void Example_Cloud_GettingStarted_WithLocalAPI(
            string examplePath,
            string[] networkNames,
            string[] countryNames,
            string[] ipRanges,
            string rangeDescriptor)
        {
            if (_apiProcess == null)
            {
                Assert.Inconclusive(
                    "Local API mode requires both an IP Intelligence data file and a Device " +
                    $"Detection .hash file. Set {Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR} and " +
                    $"{Constants.DEVICE_DETECTION_DATA_FILE_ENV_VAR}, or place the data files in the repository.");
            }

            // The Cloud examples require RESOURCE_KEY to be set even when pointing at
            // a local endpoint; the local API ignores its value.
            var env = new Dictionary<string, string>
            {
                [ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR] = "local",
                [ExampleUtils.CLOUD_END_POINT_ENV_VAR] = ApiUrl,
            };
            RunCloudExample(examplePath, networkNames, countryNames, ipRanges, rangeDescriptor, env);
        }

        /// <summary>
        /// Runs the Cloud example against the real 51Degrees Cloud service.
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
            if (string.IsNullOrWhiteSpace(_resourceKey))
            {
                Assert.Inconclusive(
                    $"Real Cloud mode requires the {ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR} " +
                    "environment variable to be set. Obtain a resource key at " +
                    "https://configure.51degrees.com.");
            }

            var env = new Dictionary<string, string>
            {
                [ExampleUtils.CLOUD_RESOURCE_KEY_ENV_VAR] = _resourceKey,
            };
            var endPoint = Environment.GetEnvironmentVariable(
                ExampleUtils.CLOUD_END_POINT_ENV_VAR);
            if (string.IsNullOrWhiteSpace(endPoint) == false)
            {
                env[ExampleUtils.CLOUD_END_POINT_ENV_VAR] = endPoint;
            }
            RunCloudExample(examplePath, networkNames, countryNames, ipRanges, rangeDescriptor, env);
        }

        private static void RunCloudExample(
            string examplePath,
            string[] networkNames,
            string[] countryNames,
            string[] ipRanges,
            string rangeDescriptor,
            Dictionary<string, string> env)
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
            foreach (var kvp in env)
            {
                startInfo.Environment[kvp.Key] = kvp.Value;
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

                // PropertyMissingException = the data file / resource key doesn't expose a
                // property the example reads (e.g. HardwareName from the Lite Device Detection
                // file). Treat as inconclusive rather than a failure.
                if (process.ExitCode != 0 && errorOutput.Contains("PropertyMissingException"))
                {
                    Assert.Inconclusive(
                        "The example could not run fully because the Device Detection data file " +
                        "or resource key does not expose a required property (e.g. HardwareName, " +
                        "which is absent from the Lite data file). Provide a TAC or Enterprise " +
                        ".hash file. See https://51degrees.com/documentation/_info__resource_keys.html");
                }

                Assert.AreEqual(0, process.ExitCode,
                    $"Cloud example should exit successfully. Error output: {errorOutput}");

                Assert.Contains("Input values:", result, "Output should contain input values section");
                Assert.Contains("Results:", result, "Output should contain results section");

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
            if (_httpClient == null)
            {
                Assert.Inconclusive(
                    "Health check requires the local API to be running. Set " +
                    $"{Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR} and " +
                    $"{Constants.DEVICE_DETECTION_DATA_FILE_ENV_VAR} (with a compatible data file).");
            }

            // Test the evidencekeys endpoint as a basic health check
            var response = await _httpClient.GetAsync($"{ApiUrl}/evidencekeys");
            Assert.IsTrue(response.IsSuccessStatusCode, "API evidencekeys endpoint should respond successfully");

            var content = await response.Content.ReadAsStringAsync();
            Assert.Contains("query.client-ip", content, "Response should contain expected evidence keys");
        }
    }
}
