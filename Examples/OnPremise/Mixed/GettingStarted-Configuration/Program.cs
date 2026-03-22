using FiftyOne.DeviceDetection;
using FiftyOne.DeviceDetection.Hash.Engine.OnPremise.FlowElements;
using FiftyOne.IpIntelligence;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.Pipeline.Core.Configuration;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines.Data;
using FiftyOne.Pipeline.Engines.FiftyOne.FlowElements;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Text;
using FiftyOne.IpIntelligence.Translation.FlowElements;

namespace FiftyOne.IpIntelligence.Examples.Mixed.OnPremise.GettingStartedConfiguration
{
    class Program
    {
        static void Main(string[] args)
        {
            // Load the configuration file
            var config = new ConfigurationBuilder()
                .AddJsonFile("appsettings.json")
                .Build();

            // Bind the configuration to a PipelineOptions instance
            var options = new PipelineOptions();
            config.GetRequiredSection("PipelineOptions")
                .Bind(options, o => o.ErrorOnUnknownConfiguration = true);

            // Set up dependency injection with required builder registrations
            var services = new ServiceCollection();
            services.AddLogging(b => b.AddConsole());
            services.AddSingleton<DeviceDetectionHashEngineBuilder>();
            services.AddSingleton<IpiOnPremiseEngineBuilder>();
            services.AddSingleton<CountryCodeTranslationEngineBuilder>();
            services.AddSingleton<CountriesTranslationEngineBuilder>();
            var serviceProvider = services.BuildServiceProvider();

            var loggerFactory = serviceProvider.GetRequiredService<ILoggerFactory>();

            // Build the pipeline from the JSON configuration
            using var pipeline = new FiftyOnePipelineBuilder(loggerFactory, serviceProvider)
                .BuildFromConfiguration(options);

            // Define evidence: User-Agent for device detection, IP for IP intelligence
            var evidence = new Dictionary<string, object>
            {
                { "header.user-agent", "Mozilla/5.0 (Linux; Android 11; SM-G973F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.120 Mobile Safari/537.36" },
                { "query.client-ip", "81.145.129.114" }
            };

            // Create flow data, add evidence and process
            using var data = pipeline.CreateFlowData();
            data.AddEvidence(evidence);
            data.Process();

            // Output Device Detection results
            var device = data.Get<IDeviceData>();
            Console.WriteLine("--- Device Detection ---");
            OutputValue("IsMobile", device.IsMobile);
            OutputValue("PlatformName", device.PlatformName);
            OutputValue("BrowserName", device.BrowserName);
            OutputValue("HardwareVendor", device.HardwareVendor);

            Console.WriteLine();

            // Output IP Intelligence results
            var ip = data.Get<IIpIntelligenceData>();
            Console.WriteLine("--- IP Intelligence ---");
            OutputValue("Country", ip.Country);
            OutputValue("CountryCode", ip.CountryCode);
            OutputValue("RegisteredName", ip.RegisteredName);
        }

        static void OutputValue(string name, IAspectPropertyValue value)
        {
            Console.WriteLine(value.HasValue
                ? $"  {name}: {value.Value}"
                : $"  {name}: {value.NoValueMessage}");
        }
    }
}
