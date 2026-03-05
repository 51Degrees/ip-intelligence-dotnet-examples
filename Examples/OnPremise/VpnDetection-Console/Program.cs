using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace FiftyOne.IpIntelligence.Examples.OnPremise.VpnDetection;

/// <summary>
/// @example OnPremise/VpnDetection-Console/Program.cs
/// 
/// This example shows how to combine 51Degrees On-premise IP Intelligence
/// properties to determine whether an IP is likely to be a VPN.
/// 
/// You will learn:
/// 
/// 1. How to get diversity properties from the IP Intelligence engine.
/// 2. How to combine diversity values with other network related values to
/// assess the likelihood of an IP address being a VPN or proxy.
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/VpnDetection-Console/Program.cs). 
/// 
/// This example requires an enterprise IP Intelligence data file (.ipi). 
/// To obtain an enterprise data file for testing, please [contact us](https://51degrees.com/contact-us).
/// 
/// Required NuGet Dependencies:
/// - [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence/)
/// </summary>
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
                .Build())
            {
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
            // Process the evidence
            using var flowData = pipeline.CreateFlowData();
            flowData.AddEvidence(evidence);
            flowData.Process();
            var ipData = flowData.Get<IIpIntelligenceData>();

            // In this example we use the IsCellular, HardwareDiversity,
            // and LocationConfidence properties to determine if the IP
            // address is likely to be a VPN or proxy.
            var isCellular = ipData.IsCellular.HasValue && ipData.IsCellular.Value;
            var hardwareDiversity = ipData.HardwareDiversity.HasValue ?
                ipData.HardwareDiversity.Value : 0;
            var locationConfidence = ipData.LocationConfidence.HasValue
                ? ipData.LocationConfidence.Value : null;
            // Here we can say that if there is a high diversity of hardware
            // profiles in the IP, then is could be either VPN, cellular, proxy,
            // or other hosting.
            // We can then rule out cellular with the IsCellular property,
            // and with a low LocationConfidence, we can assume it is a VPN,
            // or proxy, rather than other hosting. High hardware diversity
            // makes it more likely to be VPN or proxy, but other hosting
            // networks can also fall into this.
            // Many other properties can be used to draw conclusions about the
            // likelihood of VPNs, and this is a very basic example that should
            // not be used in production without further testing and tuning.
            var isVpn = hardwareDiversity >= 7 &&
                isCellular == false &&
                locationConfidence == "Low";
            var message = new StringBuilder();
            message.AppendLine("Input values:");
            foreach (var entry in evidence)
            {
                message.AppendLine($"\t{entry.Key}: {entry.Value}");
            }

            message.AppendLine("Results:");
            message.AppendLine($"\tIsCellular: {isCellular}");
            message.AppendLine($"\tHardwareDiversity: {hardwareDiversity}");
            message.AppendLine($"\tLocationConfidence: {locationConfidence}");
            message.AppendLine($"\tIsVpn: {isVpn}");
            output.WriteLine(message.ToString());
        }
    }

    static void Main(string[] args)
    {
        // Use the supplied path for the data file or find the lite file that is included
        // in the repository.
        var dataFile = args.Length > 0 ? args[0] :
        // In this example, by default, the 51degrees IP Intelligence data file needs to be somewhere in the
        // project space, or you may specify another file as a command line parameter.
        //
        // For testing, contact us to obtain an enterprise data file: https://51degrees.com/contact-us
        Examples.ExampleUtils.FindFile(Constants.ENTERPRISE_IPI_DATA_FILE_NAME);

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
