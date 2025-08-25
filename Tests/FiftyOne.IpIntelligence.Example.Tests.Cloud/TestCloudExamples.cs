/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

using FiftyOne.IpIntelligence.Examples;
using FiftyOne.Pipeline.Core.Configuration;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net.Http;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace FiftyOne.IpIntelligence.Example.Tests.Cloud
{
    /// <summary>
    /// This test class ensures that the Cloud examples execute successfully
    /// when running against the local API provided by GettingStarted-API.
    /// </summary>
    /// <remarks>
    /// Note that these tests do not generally ensure the correctness 
    /// of the example, only that the example will run without 
    /// crashing or throwing any unhandled exceptions.
    /// </remarks>
    [TestClass]
    public class TestCloudExamples
    {
        private string DataFile;
        private Process apiProcess;
        private const int API_PORT = 5225;
        private const string API_URL = "http://localhost:5225";
        private HttpClient httpClient;

        /// <summary>
        /// Init method - starts the GettingStarted-API in the background
        /// and sets up the data file for testing.
        /// </summary>
        [TestInitialize]
        public async Task Init()
        {
            // Set IP Intelligence Data file
            DataFile = Environment.GetEnvironmentVariable(
                Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR);
            if (string.IsNullOrWhiteSpace(DataFile))
            {
                DataFile = ExampleUtils.FindFile(
                    Constants.LITE_IPI_DATA_FILE_NAME);
            }

            // Write data file path for debugging
            File.WriteAllText($"{nameof(TestCloudExamples)}_DataFileName.txt", DataFile);

            // Start the API server
            await StartApiServer();
            
            // Create HTTP client for health checks
            httpClient = new HttpClient();
        }

        /// <summary>
        /// Cleanup method - stops the API server
        /// </summary>
        [TestCleanup]
        public void Cleanup()
        {
            httpClient?.Dispose();
            StopApiServer();
        }

        private async Task StartApiServer()
        {
            // Build path to the API project
            var apiProjectPath = Path.Combine(
                Path.GetDirectoryName(typeof(TestCloudExamples).Assembly.Location),
                "..", "..", "..", "..", "..",
                "Examples", "OnPremise", "Mixed", "GettingStarted-API");
            
            apiProjectPath = Path.GetFullPath(apiProjectPath);
            var apiExePath = Path.Combine(apiProjectPath, "bin", "Debug", "net8.0", 
                "GettingStarted-API");

            // Check if the executable exists, if not, build it first
            if (!File.Exists(apiExePath) && !File.Exists(apiExePath + ".exe"))
            {
                Console.WriteLine("Building GettingStarted-API project...");
                var buildProcess = Process.Start(new ProcessStartInfo
                {
                    FileName = "dotnet",
                    Arguments = $"build \"{Path.Combine(apiProjectPath, "GettingStarted-API.csproj")}\" -c Debug",
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    WorkingDirectory = apiProjectPath
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
                Arguments = $"run --project \"{Path.Combine(apiProjectPath, "GettingStarted-API.csproj")}\" --no-build",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WorkingDirectory = apiProjectPath,
                CreateNoWindow = true
            };

            // Set environment variables for the API process
            startInfo.Environment["ASPNETCORE_URLS"] = API_URL;
            startInfo.Environment["ASPNETCORE_ENVIRONMENT"] = "Development";
            startInfo.Environment[Constants.IP_INTELLIGENCE_DATA_FILE_ENV_VAR] = DataFile;

            // Start the process
            apiProcess = new Process { StartInfo = startInfo };
            
            // Capture output for debugging
            apiProcess.OutputDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    Console.WriteLine($"API Output: {e.Data}");
                }
            };
            
            apiProcess.ErrorDataReceived += (sender, e) =>
            {
                if (!string.IsNullOrEmpty(e.Data))
                {
                    Console.WriteLine($"API Error: {e.Data}");
                }
            };

            apiProcess.Start();
            apiProcess.BeginOutputReadLine();
            apiProcess.BeginErrorReadLine();

            // Wait for the API to be ready (with timeout)
            await WaitForApiToBeReady();
        }

        private async Task WaitForApiToBeReady()
        {
            using var client = new HttpClient();
            var stopwatch = Stopwatch.StartNew();
            var timeout = TimeSpan.FromSeconds(30);

            while (stopwatch.Elapsed < timeout)
            {
                try
                {
                    // Try to access the evidencekeys endpoint as a health check
                    var response = await client.GetAsync($"{API_URL}/evidencekeys");
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

        private void StopApiServer()
        {
            if (apiProcess != null && !apiProcess.HasExited)
            {
                try
                {
                    apiProcess.Kill(true);
                    apiProcess.WaitForExit(5000);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Error stopping API server: {ex.Message}");
                }
                finally
                {
                    apiProcess?.Dispose();
                }
            }
        }

        /// <summary>
        /// Test the Cloud GettingStarted Example using the local API
        /// </summary>
        /// <remarks>
        /// This test verifies that the Cloud example runs successfully when configured
        /// to use a local API endpoint (GettingStarted-API) for testing purposes.
        /// This demonstrates how Cloud examples can be tested against a local
        /// API server instead of relying on external cloud services.
        /// </remarks>
        [TestMethod]
        public void Example_Cloud_GettingStarted_WithLocalAPI()
        {
            // Get path to the Cloud example project
            var cloudExamplePath = Path.Combine(
                Path.GetDirectoryName(typeof(TestCloudExamples).Assembly.Location),
                "..", "..", "..", "..", "..",
                "Examples", "Cloud", "GettingStarted-Console");
            
            cloudExamplePath = Path.GetFullPath(cloudExamplePath);
            
            // Run the actual example using dotnet run, just like a user would
            var startInfo = new ProcessStartInfo
            {
                FileName = "dotnet",
                Arguments = "run",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WorkingDirectory = cloudExamplePath,
                CreateNoWindow = true
            };

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
                Assert.IsTrue(process.WaitForExit(30000), "Cloud example should complete within 30 seconds");
                
                var result = output.ToString();
                var errorOutput = error.ToString();
                
                Console.WriteLine("Cloud Example Output:");
                Console.WriteLine(result);
                
                if (!string.IsNullOrEmpty(errorOutput))
                {
                    Console.WriteLine("Cloud Example Error Output:");
                    Console.WriteLine(errorOutput);
                }
                
                // Verify the example ran successfully
                Assert.AreEqual(0, process.ExitCode, $"Cloud example should exit successfully. Error output: {errorOutput}");
                
                // Verify the example produced IP intelligence results
                Assert.IsTrue(result.Contains("Input values:"), "Output should contain input values section");
                Assert.IsTrue(result.Contains("Results:"), "Output should contain results section");
                
                // Check for actual IP intelligence data values (not just field names)
                // Based on the actual output, verify we get real IP intelligence data
                Assert.IsTrue(result.Contains("UNICOM") || result.Contains("unicom"), 
                    "Output should contain UNICOM as registered name");
                Assert.IsTrue(result.Contains("China") || result.Contains("CN") || result.Contains("cn"), 
                    "Output should contain China or CN as country");
                Assert.IsTrue(result.Contains("116.128.0.0") || result.Contains("116.191.255.255"), 
                    "Output should contain IP range values starting with 116.x");
                
                // Verify we have actual IP address data in the range information
                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"\d+\.\d+\.\d+\.\d+"), 
                    "Output should contain IP address values in dotted decimal format");
                    
                // Verify we have numeric coordinate or range data
                Assert.IsTrue(System.Text.RegularExpressions.Regex.IsMatch(result, @"[\d\-]\d+\.\d+"), 
                    "Output should contain numeric values (coordinates, ranges, etc.)");
                    
                // Ensure no errors occurred
                Assert.IsFalse(result.Contains("Exception"), "Output should not contain exceptions");
                Assert.IsFalse(result.Contains("Error"), "Output should not contain errors");
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
        public async Task Example_API_HealthCheck()
        {
            // Test the evidencekeys endpoint as a basic health check
            var response = await httpClient.GetAsync($"{API_URL}/evidencekeys");
            Assert.IsTrue(response.IsSuccessStatusCode, "API evidencekeys endpoint should respond successfully");
            
            var content = await response.Content.ReadAsStringAsync();
            Assert.IsTrue(content.Contains("query.client-ip"), "Response should contain expected evidence keys");
        }
    }
}