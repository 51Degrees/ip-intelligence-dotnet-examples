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
using System.Net;
using System.Text;

/// <summary>
/// @example OnPremise/GettingStarted-Console/Program.cs
/// 
/// @include{doc} example-getting-started-onpremise.txt
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/GettingStarted-Console/Program.cs). 
/// 
/// @include{doc} example-require-datafile.txt
/// 
/// Required NuGet Dependencies:
/// - FiftyOne.IpIntelligence
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
                    // We use the low memory profile as its performance is sufficient for this
                    // example. See the documentation for more detail on this and other
                    // configuration options:
                    // https://51degrees.com/documentation/_ip_intelligence__features__performance_options.html
                    // https://51degrees.com/documentation/_features__automatic_datafile_updates.html
                    // https://51degrees.com/documentation/_features__usage_sharing.html
                    .SetPerformanceProfile(PerformanceProfiles.MaxPerformance)
                    // inhibit sharing usage for this example, usually this should be set to "true"
                    .SetShareUsage(false)
                    // inhibit auto-update of the data file for this test
                    .SetAutoUpdate(false)
                    .SetDataUpdateOnStartUp(false)
                    .SetDataFileSystemWatcher(false)
                    // Set all available properties
                    .SetProperty("AccuracyRadiusMax")
                    .SetProperty("AccuracyRadiusMin")
                    .SetProperty("Areas")
                    .SetProperty("AsnName")
                    .SetProperty("AsnNumber")
                    .SetProperty("ConnectionType")
                    .SetProperty("ContinentCode2")
                    .SetProperty("ContinentName")
                    .SetProperty("Country")
                    .SetProperty("CountryCode")
                    .SetProperty("CountryCode3")
                    .SetProperty("County")
                    .SetProperty("CurrencyCode")
                    .SetProperty("DialCode")
                    .SetProperty("HumanProbability")
                    .SetProperty("IpRangeEnd")
                    .SetProperty("IpRangeStart")
                    .SetProperty("IsBroadband")
                    .SetProperty("IsCellular")
                    .SetProperty("IsEu")
                    .SetProperty("IsHosted")
                    .SetProperty("IsProxy")
                    .SetProperty("IsPublicRouter")
                    .SetProperty("IsTor")
                    .SetProperty("IsVPN")
                    .SetProperty("LanguageCode")
                    .SetProperty("Latitude")
                    .SetProperty("LocationConfidence")
                    .SetProperty("Longitude")
                    .SetProperty("Mcc")
                    .SetProperty("Region")
                    .SetProperty("RegisteredCountry")
                    .SetProperty("RegisteredName")
                    .SetProperty("RegisteredOwner")
                    .SetProperty("State")
                    .SetProperty("Suburb")
                    .SetProperty("TimeZoneIana")
                    .SetProperty("TimeZoneOffset")
                    .SetProperty("Town")
                    .SetProperty("ZipCode")
                    .Build())
                {
                    // carry out some sample detections
                    // and collect IP addresses
                    foreach (var evidence in ExampleUtils.EvidenceValues)
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

                    // Output all available properties
                    message.AppendLine("\t--- Accuracy & Location Confidence ---");
                    OutputWeightedIntValues(nameof(ipData.AccuracyRadiusMax), ipData.AccuracyRadiusMax, message);
                    OutputWeightedIntValues(nameof(ipData.AccuracyRadiusMin), ipData.AccuracyRadiusMin, message);
                    OutputListProperty(nameof(ipData.LocationConfidence), ipData.LocationConfidence, message);
                    
                    message.AppendLine("\t--- Geographic Location ---");
                    OutputListProperty(nameof(ipData.Country), ipData.Country, message);
                    OutputListProperty(nameof(ipData.CountryCode), ipData.CountryCode, message);
                    OutputListProperty(nameof(ipData.CountryCode3), ipData.CountryCode3, message);
                    OutputListProperty(nameof(ipData.ContinentName), ipData.ContinentName, message);
                    OutputListProperty(nameof(ipData.ContinentCode2), ipData.ContinentCode2, message);
                    OutputListProperty(nameof(ipData.Region), ipData.Region, message);
                    OutputListProperty(nameof(ipData.State), ipData.State, message);
                    OutputListProperty(nameof(ipData.County), ipData.County, message);
                    OutputListProperty(nameof(ipData.Town), ipData.Town, message);
                    OutputListProperty(nameof(ipData.Suburb), ipData.Suburb, message);
                    OutputListProperty(nameof(ipData.ZipCode), ipData.ZipCode, message);
                    OutputWeightedFloatValues(nameof(ipData.Latitude), ipData.Latitude, message);
                    OutputWeightedFloatValues(nameof(ipData.Longitude), ipData.Longitude, message);
                    OutputListProperty(nameof(ipData.Areas), ipData.Areas, message);
                    
                    message.AppendLine("\t--- Regional Information ---");
                    OutputWeightedBoolValues(nameof(ipData.IsEu), ipData.IsEu, message);
                    OutputListProperty(nameof(ipData.CurrencyCode), ipData.CurrencyCode, message);
                    OutputWeightedIntValues(nameof(ipData.DialCode), ipData.DialCode, message);
                    OutputListProperty(nameof(ipData.LanguageCode), ipData.LanguageCode, message);
                    
                    message.AppendLine("\t--- Time Zone ---");
                    OutputListProperty(nameof(ipData.TimeZoneIana), ipData.TimeZoneIana, message);
                    OutputWeightedIntValues(nameof(ipData.TimeZoneOffset), ipData.TimeZoneOffset, message);
                    
                    message.AppendLine("\t--- Network Registration ---");
                    OutputListProperty(nameof(ipData.RegisteredName), ipData.RegisteredName, message);
                    OutputListProperty(nameof(ipData.RegisteredOwner), ipData.RegisteredOwner, message);
                    OutputListProperty(nameof(ipData.RegisteredCountry), ipData.RegisteredCountry, message);
                    OutputWeightedIPAddressValues(nameof(ipData.IpRangeStart), ipData.IpRangeStart, message);
                    OutputWeightedIPAddressValues(nameof(ipData.IpRangeEnd), ipData.IpRangeEnd, message);
                    
                    message.AppendLine("\t--- ASN Information ---");
                    OutputListProperty(nameof(ipData.AsnName), ipData.AsnName, message);
                    OutputListProperty(nameof(ipData.AsnNumber), ipData.AsnNumber, message);
                    
                    message.AppendLine("\t--- Connection Type ---");
                    OutputListProperty(nameof(ipData.ConnectionType), ipData.ConnectionType, message);
                    OutputWeightedBoolValues(nameof(ipData.IsBroadband), ipData.IsBroadband, message);
                    OutputWeightedBoolValues(nameof(ipData.IsCellular), ipData.IsCellular, message);
                    OutputListProperty(nameof(ipData.Mcc), ipData.Mcc, message);
                    
                    message.AppendLine("\t--- Security & Anonymity ---");
                    OutputWeightedBoolValues(nameof(ipData.IsHosted), ipData.IsHosted, message);
                    OutputWeightedBoolValues(nameof(ipData.IsProxy), ipData.IsProxy, message);
                    OutputWeightedBoolValues(nameof(ipData.IsVPN), ipData.IsVPN, message);
                    OutputWeightedBoolValues(nameof(ipData.IsTor), ipData.IsTor, message);
                    OutputWeightedBoolValues(nameof(ipData.IsPublicRouter), ipData.IsPublicRouter, message);
                    OutputWeightedIntValues(nameof(ipData.HumanProbability), ipData.HumanProbability, message);
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


            private void OutputValue(string name, 
                IAspectPropertyValue value,
                StringBuilder message)
            {
                // Individual result values have a wrapper called `AspectPropertyValue`.
                // This functions similarly to a null-able type.
                // If the value has not been set then trying to access the `Value` property will
                // throw an exception. `AspectPropertyValue` also includes the `NoValueMessage`
                // property, which describes why the value has not been set.
                if (!value.HasValue)
                {
                    message.AppendLine($"\t{name}: {value.NoValueMessage}");
                }
                else
                {
                    message.AppendLine($"\t{name}: {value.Value}");
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

            private void OutputWeightedBoolValues(string name, 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>> property,
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

                ExampleUtils.FindFile(Constants.LITE_IPI_DATA_FILE_NAME);

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