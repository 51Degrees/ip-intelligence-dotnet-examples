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

using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using FiftyOne.Pipeline.Engines.Data;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

/// <summary>
/// @example OnPremise/GettingStarted-Console/Program.cs
/// 
/// This example shows how to use 51Degrees On-premise IP Intelligence to determine location and network details from IP addresses.
/// 
/// You will learn:
/// 
/// 1. How to create a Pipeline that uses 51Degrees On-premise IP Intelligence
/// 2. How to pass input data (evidence) to the Pipeline
/// 3. How to retrieve the results
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/GettingStarted-Console/Program.cs). 
/// 
/// This example requires an enterprise IP Intelligence data file (.ipi). 
/// To obtain an enterprise data file for testing, please [contact us](https://51degrees.com/contact-us).
/// 
/// Required NuGet Dependencies:
/// - [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence/)
/// </summary>
namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedConsole
{
    public class Program
    {
        public class Example : ExampleBase
        {
            public void Run(string dataFile, ILoggerFactory loggerFactory, TextWriter output)
            {
                // In this example, we use the IpiPipelineBuilder and configure it
                // in code. For more information about builders in general see the documentation at
                // https://51degrees.com/documentation/_concepts__configuration__builders__index.html

                // Note that we wrap the creation of a pipeline in a using to control its life cycle
                using (var pipeline = new IpiPipelineBuilder()
                    .UseOnPremise(dataFile, null, false)
                    // We use the max performance profile for optimal detection speed in this
                    // example. See the documentation for more detail on this and other
                    // configuration options.
                    // https://51degrees.com/documentation/_features__automatic_datafile_updates.html
                    // https://51degrees.com/documentation/_features__usage_sharing.html
                    .SetPerformanceProfile(PerformanceProfiles.MaxPerformance)
                    // inhibit sharing usage for this example, usually this should be set to "true"
                    .SetShareUsage(false)
                    // inhibit auto-update of the data file for this test
                    .SetAutoUpdate(false)
                    .SetDataUpdateOnStartUp(false)
                    .SetDataFileSystemWatcher(false)
                    .SetProperty("RegisteredCountry")
                    .SetProperty("RegisteredOwner")
                    .SetProperty("RegisteredName")
                    .SetProperty("IpRangeStart")
                    .SetProperty("IpRangeEnd")
                    .SetProperty("Country")
                    .SetProperty("CountryCode")
                    .SetProperty("CountryCode3")
                    .SetProperty("Region")
                    .SetProperty("State")
                    .SetProperty("Town")
                    .SetProperty("Latitude")
                    .SetProperty("Longitude")
                    .SetProperty("Areas")
                    .SetProperty("AccuracyRadiusMin")
                    .SetProperty("TimeZoneOffset")
                    .Build())
                {
                    // carry out some sample detections
                    // and collect IP addresses
                    foreach (var evidence in Examples.ExampleUtils.EvidenceValues)
                    {
                        AnalyseEvidence(evidence, pipeline, output);
                    }

                    ExampleUtils.CheckDataFile(pipeline, loggerFactory.CreateLogger<Program>());
                }
            }

            private void AnalyseEvidence(
                Dictionary<string, object> evidence,
                IPipeline pipeline, 
                TextWriter output)
            {
                // FlowData is a data structure that is used to convey information required for
                // detection and the results of the detection through the pipeline.
                // Information required for detection is called "evidence" and usually consists
                // of a number of HTTP Header field values, in this case represented by a
                // Dictionary<string, object> of header name/value entries.
                //
                // FlowData is wrapped in a using block in order to ensure that the unmanaged
                // resources allocated by the native IP Intelligence library are freed
                using (var data = pipeline.CreateFlowData())
                {
                    StringBuilder message = new StringBuilder();

                    // list the evidence
                    message.AppendLine("Input values:");
                    foreach (var entry in evidence)
                    {
                        message.AppendLine($"\t{entry.Key}: {entry.Value}");
                    }
                    output.WriteLine(message.ToString());

                    // Add the evidence values to the flow data
                    data.AddEvidence(evidence);

                    // Process the flow data.
                    data.Process();

                    message = new StringBuilder();
                    message.AppendLine("Results:");
                    // Now that it's been processed, the flow data will have been populated with
                    // the result. In this case, we want information about the IP address, which we
                    // can get by asking for a result matching the `IIpIntelligenceData` interface.
                    var ipData = data.Get<IIpIntelligenceData>();

                    // Output all the properties
                    OutputListProperty(nameof(ipData.RegisteredName), ipData.RegisteredName, message);
                    OutputListProperty(nameof(ipData.RegisteredOwner), ipData.RegisteredOwner, message);
                    OutputListProperty(nameof(ipData.RegisteredCountry), ipData.RegisteredCountry, message);
                    OutputWeightedIPAddressValues(nameof(ipData.IpRangeStart), ipData.IpRangeStart, message);
                    OutputWeightedIPAddressValues(nameof(ipData.IpRangeEnd), ipData.IpRangeEnd, message);
                    OutputListProperty(nameof(ipData.Country), ipData.Country, message);
                    OutputListProperty(nameof(ipData.CountryCode), ipData.CountryCode, message);
                    OutputListProperty(nameof(ipData.CountryCode3), ipData.CountryCode3, message);
                    OutputListProperty(nameof(ipData.Region), ipData.Region, message);
                    OutputListProperty(nameof(ipData.State), ipData.State, message);
                    OutputListProperty(nameof(ipData.Town), ipData.Town, message);
                    OutputWeightedFloatValues(nameof(ipData.Latitude), ipData.Latitude, message);
                    OutputWeightedFloatValues(nameof(ipData.Longitude), ipData.Longitude, message);
                    OutputListProperty(nameof(ipData.Areas), ipData.Areas, message);
                    OutputWeightedIntValues(nameof(ipData.AccuracyRadiusMin), ipData.AccuracyRadiusMin, message);
                    OutputWeightedIntValues(nameof(ipData.TimeZoneOffset), ipData.TimeZoneOffset, message);
                    output.WriteLine(message.ToString());
                }
            }

            private void OutputListProperty(string name, 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<string>>> property,
                StringBuilder message)
            {
                if (!property.HasValue)
                {
                    message.AppendLine($"\t{name}: {property.NoValueMessage}");
                }
                else
                {
                    var values = string.Join(", ", property.Value.Select(x => 
                        x.Weighting() == 1 ? $"'{x.Value}'" : $"('{x.Value}' @ {x.Weighting()})"));
                    message.AppendLine($"\t{name} ({property.Value.Count}): {values}");
                }
            }

            private void OutputWeightedIntValues(string name, 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<int>>> property,
                StringBuilder message)
            {
                if (!property.HasValue)
                {
                    message.AppendLine($"\t{name}: {property.NoValueMessage}");
                }
                else
                {
                    var values = property.Value.Select(x => 
                        Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString() : $"({x.Value} @ {x.Weighting():F4})");
                    message.AppendLine($"\t{name} ({property.Value.Count}): {string.Join(", ", values)}");
                }
            }

            private void OutputWeightedFloatValues(string name, 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<float>>> property,
                StringBuilder message)
            {
                if (!property.HasValue)
                {
                    message.AppendLine($"\t{name}: {property.NoValueMessage}");
                }
                else
                {
                    var values = property.Value.Select(x => 
                        Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString("F6") : $"({x.Value:F6} @ {x.Weighting():F4})");
                    message.AppendLine($"\t{name} ({property.Value.Count}): {string.Join(", ", values)}");
                }
            }

            private void OutputWeightedIPAddressValues(string name, 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<System.Net.IPAddress>>> property,
                StringBuilder message)
            {
                if (!property.HasValue)
                {
                    message.AppendLine($"\t{name}: {property.NoValueMessage}");
                }
                else
                {
                    var values = property.Value.Select(x => 
                        Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString() : $"({x.Value} @ {x.Weighting():F4})");
                    message.AppendLine($"\t{name} ({property.Value.Count}): {string.Join(", ", values)}");
                }
            }
        }

        static void Main(string[] args)
        {
            // Use the supplied path for the data file or find the lite file that is included
            // in the repository.
            var dataFile = args.Length > 0 ? args[0] :
                // In this example, by default, the 51degrees "Lite" file needs to be somewhere in the
                // project space, or you may specify another file as a command line parameter.
                //
                // Note that the Lite data file is only used for illustration, and has limited accuracy
                // and capabilities. Find out about the Enterprise data file on our pricing page:
                // https://51degrees.com/pricing

                Examples.ExampleUtils.FindFile(Constants.ENTERPRISE_IPI_DATA_FILE_NAME);

            File.WriteAllText("GettigStarted_DataFileName.txt", dataFile);

            // Configure a logger to output to the console.
            var loggerFactory = LoggerFactory.Create(b => b.AddConsole());
            var logger = loggerFactory.CreateLogger<Program>();

            if (dataFile != null)
            {
                new Example().Run(dataFile, loggerFactory, Console.Out);
            } 
            else
            {
                logger.LogError("Failed to find a IP Intelligence data file. Make sure the " +
                    "ip-intelligence-data submodule has been updated by running " +
                    "`git submodule update --recursive`.");
            }

            // Dispose the logger to ensure any messages get flushed
            loggerFactory.Dispose();
        }
    }
}