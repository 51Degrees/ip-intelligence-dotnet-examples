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

using FiftyOne.IpIntelligence.Engine.OnPremise.Data;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Engines;
using FiftyOne.Pipeline.Engines.FiftyOne.Data;
using Microsoft.Extensions.Logging;
using System;
using System.IO;
using System.Linq;
using System.Text;

/// <summary>
/// @example OnPremise/Metadata-Console/Program.cs
///
/// The IP Intelligence data file contains meta data that can provide additional information
/// about the various records in the data model.
/// This example shows how to access this data and display the values available.
/// 
/// To help navigate the data, it's useful to have an understanding of the types of records that
/// are present:
/// - Component - A record relating to a major aspect of the entity making a web request. There are currently 4 components: Hardware, Software Platform (OS), Browser and Crawler.
/// - Profile - A record containing the details for a specific instance of a component. An example of a hardware profile would be the profile for the iPhone 13. An example of a platform profile would be Android 12.1.0.
/// - Property - Each property will have a specific value (or values) for each profile. An example of a hardware property is 'IsMobile'. An example of a browser property is 'BrowserName'.
/// 
/// The example will output each component in turn, with a list of the properties associated with
/// each component. Some of the possible values for each property are also displayed.
/// There are too many profiles to display, so we just list the number of profiles for each 
/// component.
/// 
/// Finally, the evidence keys that are accepted by IP Intelligence are listed. These are the 
/// keys that, when added to the evidence collection in flow data, could have some impact on the
/// result returned by IP Intelligence.
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/Metadata-Console/Program.cs). 
/// 
/// @include{doc} example-require-datafile.txt
/// 
/// Required NuGet Dependencies:
/// - FiftyOne.IpIntelligence
/// - Microsoft.Extensions.Logging.Console
/// </summary>
namespace FiftyOne.IpIntelligence.Examples.OnPremise.Metadata
{
    public class Program
    {
        public class Example : ExampleBase
        {
            public void Run(string dataFile, ILoggerFactory loggerFactory, TextWriter output)
            {
                // Build a new on-premise IP Intelligence engine with the low memory performance profile.
                // Note that there is no need to construct a complete pipeline in order to access
                // the meta-data.
                // If you already have a pipeline and just want to get a reference to the engine 
                // then you can use `var engine = pipeline.GetElement<IpiOnPremiseEngine>();`
                using (var ddEngine = new IpiOnPremiseEngineBuilder(loggerFactory)
                    // We use the low memory profile as its performance is sufficient for this
                    // example. See the documentation for more detail on this and other
                    // configuration options:
                    // https://51degrees.com/documentation/_ip_intelligence__features__performance_options.html
                    // https://51degrees.com/documentation/_features__automatic_datafile_updates.html
                    .SetPerformanceProfile(PerformanceProfiles.MaxPerformance)
                    // inhibit auto-update of the data file for this test
                    .SetAutoUpdate(false)
                    .SetDataFileSystemWatcher(false)
                    .SetDataUpdateOnStartup(false)
                    .Build(dataFile, false))
                {
                    OutputComponents(ddEngine, output);
                    // TODO: Restore showing other data
                    // OutputProfileDetails(ddEngine, output);
                    OutputEvidenceKeyDetails(ddEngine, output);

                    ExampleUtils.CheckDataFile(ddEngine, loggerFactory.CreateLogger<Program>());
                }
            }

            private void OutputEvidenceKeyDetails(IpiOnPremiseEngine ddEngine, TextWriter output)
            {
                output.WriteLine();
                if(typeof(EvidenceKeyFilterWhitelist).IsAssignableFrom(ddEngine.EvidenceKeyFilter.GetType()))
                {
                    // If the evidence key filter extends EvidenceKeyFilterWhitelist then we can
                    // display a list of accepted keys.
                    var filter = ddEngine.EvidenceKeyFilter as EvidenceKeyFilterWhitelist;
                    output.WriteLine($"Accepted evidence keys:");
                    foreach (var entry in filter.Whitelist)
                    {
                        output.WriteLine($"\t{entry.Key}");
                    }
                }
                else
                {
                    output.WriteLine($"The evidence key filter has type " +
                        $"{ddEngine.EvidenceKeyFilter.GetType().Name}. As this does not extend " +
                        $"EvidenceKeyFilterWhitelist, a list of accepted values cannot be " +
                        $"displayed. As an alternative, you can pass evidence keys to " +
                        $"filter.Include(string) to see if a particular key will be included " +
                        $"or not.");
                    output.WriteLine($"For example, header.user-agent is " +
                        (ddEngine.EvidenceKeyFilter.Include("header.user-agent") ? "" : "not ") +
                        "accepted.");
                }

            }

            private void OutputProfileDetails(IpiOnPremiseEngine ddEngine, TextWriter output)
            {
                // Group the profiles by component and then output the number of profiles 
                // for each component.
                var groups = ddEngine.Profiles.GroupBy(p => p.Component.Name);
                output.WriteLine();
                output.WriteLine($"Profile counts:");
                foreach (var group in groups)
                {
                    output.WriteLine($"{group.Key} Profiles: {group.Count()}");
                }
            }

            private void OutputComponents(IpiOnPremiseEngine ddEngine, TextWriter output)
            {
                foreach (var component in ddEngine.Components)
                {
                    // Output the component name as well as a list of all the associated properties.
                    // If we're outputting to console then we also add some formatting to make it 
                    // more readable.
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.BackgroundColor = ConsoleColor.DarkBlue;
                    output.Write($"Component - {component.Name}");
                    Console.ResetColor();
                    output.WriteLine();

                    OutputProperties(component, output);
                }
            }

            private void OutputProperties(IComponentMetaData component, TextWriter output)
            {
                StringBuilder values = new StringBuilder();
                foreach (var property in component.Properties)
                {
                    // Output some details about the property.
                    // If we're outputting to console then we also add some formatting to make it 
                    // more readable.
                    Console.ForegroundColor = ConsoleColor.White;
                    Console.BackgroundColor = ConsoleColor.DarkRed;
                    output.Write($"Property - {property.Name}");
                    Console.ResetColor();
                    output.WriteLine($"[Category: {property.Category}]" +
                        $"({property.Type.Name}) - {property.Description}");

                    // Next, output a list of the possible values this property can have.
                    {
                        values.Clear();
                        values.Append("Possible values (");
                        long count = (property as PropertyMetaDataIpi)?.GetValuesCount() ?? (long)property.Values.Count();
                        int limit = (
                            // The next line can be uncommented if you want to see ALL values for some SPECIFIC property
                            //property.Name == "RegisteredCountry" ? (int)Math.Min(count, int.MaxValue) : 
                            20); // <- or just change SHARED maximum here
                        values.Append(count);
                        values.AppendLine("):");
                        bool first = true;
                        foreach (var value in property.GetValues().Take(limit))
                        {
                            if (!first)
                            {
                                values.Append(',');
                            }
                            first = false;
                            // add value
                            values.Append('\'');
                            values.Append(TruncateToNl(value.Name));
                            values.Append('\'');
                            // add description if exists
                            if (string.IsNullOrEmpty(value.Description) == false)
                            {
                                values.Append($"({value.Description})");
                            }
                        }
                        if (count > limit)
                        {
                            values.Append($" + {count - limit} more ...");
                        } else if (count > 0)
                        {
                            values.Append('.');
                        }
                        output.WriteLine(values);
                    }
                }
            }

            // Truncate value if it contains newline (esp for the JavaScript property)
            private string TruncateToNl(string s)
            {
                var lines = s.Split(new char[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
                var result = lines.FirstOrDefault();
                if (lines.Length > 1)
                {
                    result += " ...";
                }
                return result;
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

            File.WriteAllText("Metadata_DataFileName.txt", dataFile);

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
                    "`git submodule update --recursive`. By default, the 'lite' file included " +
                    "with this code will be used. A different file can be specified " +
                    "by supplying the full path as a command line argument");
            }

            // Dispose the logger to ensure any messages get flushed
            loggerFactory.Dispose();
        }
    }
}