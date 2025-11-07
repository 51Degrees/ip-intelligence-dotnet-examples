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

using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using FiftyOne.Pipeline.Engines.Data;
using FiftyOne.Pipeline.Engines.FiftyOne.Data;
using Microsoft.Extensions.Logging;
using NetTopologySuite.Geometries;
using NetTopologySuite.IO;
using NetTopologySuite.Operation.Union;
using ProjNet.CoordinateSystems;
using ProjNet.CoordinateSystems.Transformations;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Net;
using System.Runtime;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.DependencyInjection;
using System.Net.Sockets;
using System.Diagnostics;
using System.Data;
using CsvHelper;
using System.Globalization;
using CsvHelper.Configuration;
using GeoCoordinatePortable;
using CsvHelper.Configuration.Attributes;

/// <summary>
/// @example OnPremise/Metrics-Console/Program.cs
///
/// This example shows how to access Metrics about the IP Intelligence properties that are available 
/// in the data file. This can be useful for understanding what information is available and how to access it.
///
/// The example will output the available properties along with details about their data types and descriptions.
/// This helps you understand what IP Intelligence data you can access for your use case.
/// 
/// Finally, the evidence keys that are accepted by IP Intelligence are listed. These are the 
/// keys that, when added to the evidence collection in flow data, could have some impact on the
/// result returned by IP Intelligence.
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/Metrics-Console/Program.cs). 
/// 
/// This example requires an enterprise IP Intelligence data file (.ipi). 
/// To obtain an enterprise data file for testing, please [contact us](https://51degrees.com/contact-us).
/// 
/// Required NuGet Dependencies:
/// - [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence/)
/// - [Microsoft.Extensions.Logging.Console](https://www.nuget.org/packages/Microsoft.Extensions.Logging.Console/)
/// </summary>
namespace FiftyOne.IpIntelligence.Examples.OnPremise.Compare;

public class Program
{
    /// <summary>
    /// Time interval to wait between logging progress.
    /// </summary>
    private static readonly TimeSpan _logBuild = TimeSpan.FromSeconds(30);

    /// <summary>
    /// A record of latitude, longitude, IP address, and a date time that is
    /// considered truthful for the purposes of comparing with an IP to
    /// location solution result.
    /// </summary>
    public class Truth
    {
        /// <summary>
        /// The date and time in UTC of the observed truth.
        /// </summary>
        public DateTime DateTimeUtc { get; set; }

        /// <summary>
        /// The latitude of the device used.
        /// </summary>
        public double Latitude { get; set; }

        /// <summary>
        /// The longitude of the device used.
        /// </summary>
        public double Longitude { get; set; }

        /// <summary>
        /// The public IP address associated with the device.
        /// </summary>
        public string Ip { get; set; }

        /// <summary>
        /// The continent associated with the latitude and longitude.
        /// </summary>
        public string Continent { get; set; }

        /// <summary>
        /// The country associated with the latitude and longitude.
        /// </summary>
        public string Country { get; set; }
    }

    public class Result
    {
        /// <summary>
        /// The latitude returned for the IP address from the service being
        /// compared to the truth.
        /// </summary>
        [Name("LatitudeResult")]
        public double? Latitude { get; set; }

        /// <summary>
        /// The longitude returned for the IP address from the service being
        /// compared to the truth.
        /// </summary>
        [Name("LongitudeResult")]
        public double? Longitude { get; set; }

        /// <summary>
        /// An indicator concerning how confident the service is in the result.
        /// </summary>
        public string Confidence { get; set; }

        /// <summary>
        /// The distance in kilometres between the true latitude and longitude
        /// of the device and the latitude and longitude from the service.
        /// </summary>
        public double DistanceKms { get; set; }
    }

    public class Output(Truth truth, Result result)
    {
        public Truth Truth => truth;
        public Result Result => result;
    }

    public class Consumer
    {
        /// <summary>
        /// The task associated with the consumer.
        /// </summary>
        public Task<IReadOnlyList<Output>> Task;

        /// <summary>
        /// The number of IP pipeline processes that have been completed in
        /// a given period of time. Used for logging during processing.
        /// </summary>
        public int Count;
    }
        
    public class Configuration
    {
        /// <summary>
        /// IPI data file.
        /// </summary>
        public string DataFile;

        /// <summary>
        /// The source truth CSV file.
        /// </summary>
        public string CsvTruthFile;

        /// <summary>
        /// Stream to write the output to.
        /// </summary>
        public StreamWriter Output;
                
        /// <summary>
        /// Logger factory for reporting progress.
        /// </summary>
        public ILoggerFactory LoggerFactory;
    }

    public sealed class Worker(
        IHostApplicationLifetime hostApplicationLifetime,
        Configuration configuration) 
        : BackgroundService
    {
        protected override async Task ExecuteAsync(
            CancellationToken stoppingToken)
        {
            await new Example().Run(
                configuration.DataFile,
                configuration.CsvTruthFile,
                configuration.Output,
                configuration.LoggerFactory,
                stoppingToken);
            hostApplicationLifetime.StopApplication();
        }
    }

    public class Example : ExampleBase
    {
        public async Task Run(
            string dataFile, 
            string csvTruthFile,
            TextWriter output,
            ILoggerFactory loggerFactory,
            CancellationToken stoppingToken)
        {
            // Ensure that batch latency mode is always enabled.
            GCSettings.LatencyMode = GCLatencyMode.Batch;

            // Build a new on-premise IP Intelligence engine with the low memory performance profile.
            // Note that there is no need to construct a complete pipeline in order to access
            // the meta-data.
            // If you already have a pipeline and just want to get a reference to the engine 
            // then you can use `var engine = pipeline.GetElement<IpiOnPremiseEngine>();`
            using var ipiEngine = new IpiOnPremiseEngineBuilder(loggerFactory)
                // We use the max performance profile for optimal detection speed in this
                // example. See the documentation for more detail on this and other
                // configuration options.
                // https://51degrees.com/documentation/_features__automatic_datafile_updates.html
                .SetPerformanceProfile(PerformanceProfiles.MaxPerformance)
                // inhibit auto-update of the data file for this test
                .SetAutoUpdate(false)
                .SetDataFileSystemWatcher(false)
                .SetDataUpdateOnStartup(false)
                // Set to only return from processing the properties needed.
                .SetProperty("Latitude")
                .SetProperty("Longitude")
                .SetProperty("LocationConfidence")
                // Optimize for the expected parallel workload.
                .SetConcurrency((ushort)Environment.ProcessorCount)
                .Build(dataFile, false);
            using var pipeline = new PipelineBuilder(loggerFactory)
                .AddFlowElement(ipiEngine)
                .SetAutoDisposeElements(false)
                .Build();
            using var reader = File.OpenText(csvTruthFile);
            var config = CsvConfiguration.FromAttributes<Truth>(
                CultureInfo.InvariantCulture);
            config.MissingFieldFound = null;
            using var source = new CsvReader(reader, config);
            var logger = loggerFactory.CreateLogger<Example>();

            // Create a collection of ranges to ensure that there are always
            // items available for the consumers.
            var truth = new BlockingCollection<Truth>(
                Environment.ProcessorCount);
            var consumers = CreateConsumers(
                pipeline,
                truth,
                stoppingToken);
            logger.LogInformation(
                "Created '{0}' consumer processors",
                consumers.Length);

            // Use the main thread as the producer adding ranges for the
            // consumers to process.
            AddTruth(ipiEngine, logger, source, truth, consumers, stoppingToken);

            // Combine all the consumer groups into the main groups.
            using var writer = new CsvWriter(output, 
                new CsvConfiguration(CultureInfo.InvariantCulture)
                {
                    Delimiter = ","
                });
            foreach (var consumer in consumers)
            {
                await consumer.Task;
                logger.LogInformation(
                    "Finished consumer '{0}'", 
                    consumer.Task.Id);
                writer.WriteRecords(consumer.Task.Result);
            }

            ExampleUtils.CheckDataFile(
                ipiEngine,
                loggerFactory.CreateLogger<Program>());
        }

        /// <summary>
        /// Create and start the consumers which will be waiting on the
        /// producer to start.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="pipeline"></param>
        /// <param name="truth"></param>
        /// <param name="stoppingToken"></param>
        /// <returns></returns>
        private static Consumer[] 
            CreateConsumers(
                IPipeline pipeline,
                BlockingCollection<Truth> truth,
                CancellationToken stoppingToken)
        {
            return Enumerable.Range(
                0,
                Environment.ProcessorCount).Select(_ =>
                {
                    var consumer = new Consumer();
                    consumer.Task = Task.Factory.StartNew(() =>
                        ProcessTruth(
                            pipeline,
                            truth,
                            consumer,
                            stoppingToken),
                        TaskCreationOptions.LongRunning);
                    return consumer;
                }).ToArray();
        }

        private static void AddTruth(
            IpiOnPremiseEngine ipiEngine, 
            ILogger<Example> logger,
            CsvReader source,
            BlockingCollection<Truth> truth,
            Consumer[] consumers, 
            CancellationToken stoppingToken)
        {
            var process = Process.GetCurrentProcess();
            var added = 0;
            var lastLog = DateTime.UtcNow;
            var nextLog = lastLog.Add(_logBuild);
            var lastProcessorTime = process.TotalProcessorTime;
            foreach (var item in source.GetRecords<Truth>().TakeWhile(
                _ => stoppingToken.IsCancellationRequested == false))
            {
                try
                {
                    truth.TryAdd(item, -1, stoppingToken);
                    added++;
                    if (DateTime.UtcNow >= nextLog)
                    {
                        // Log the ranges and other telemetry.
                        logger.LogInformation(
                            "Processed '{0}' source records with '{1}' in " +
                            "queue, most recent '{2:N2},{3:N2}', and '{4}' " +
                            "consumers",
                            added,
                            truth.Count,
                            item.Latitude,
                            item.Longitude,
                            consumers.Count(i => i.Task.IsCompleted == false));

                        // Get the elapsed time since last logged.
                        var elapsed = DateTime.UtcNow - lastLog;

                        // The amount of CPU used is the processor time
                        // difference divided by the wall clock time
                        // difference.
                        var cpu = 
                            (process.TotalProcessorTime - lastProcessorTime) /
                            elapsed;

                        // Work out the number of queries per second.
                        var total = consumers.Sum(i => i.Count);
                        var qps = total / elapsed.TotalSeconds;

                        // Log the resource usage.
                        logger.LogInformation(
                            "'{0:F2}' processors used, '{1:N0} qps, " +
                            "'{2}' threads, '{3}' handles, and '{4:N0}MB' " +
                            "memory used",
                            cpu,
                            qps, 
                            process.Threads.Count,                            
                            process.HandleCount,
                            process.WorkingSet64 / 1000);

                        // Reset the count of queries to IPI.
                        foreach (var consumer in consumers)
                        {
                            consumer.Count = 0;
                        }

                        // Reset the other logging parameters.
                        lastLog = DateTime.UtcNow;
                        nextLog = DateTime.UtcNow.Add(_logBuild);
                        lastProcessorTime = process.TotalProcessorTime;
                    }
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit as if the ranges had been fully
                    // consumed.
                }
            }
            truth.CompleteAdding();
            logger.LogInformation("Finished adding '{0}' sources", added);
        }

        private static IReadOnlyList<Output> ProcessTruth(
            IPipeline pipeline,
            BlockingCollection<Truth> source,
            Consumer consumer,
            CancellationToken stoppingToken)
        {
            var output = new List<Output>();
            while (source.IsCompleted == false &&
                stoppingToken.IsCancellationRequested == false)
            {
                try
                {
                    if (source.TryTake(out var truth, -1, stoppingToken))
                    {
                        var result = ProcessTruth(pipeline, truth);
                        if (result != null)
                        {
                            output.Add(new Output(truth, result));
                        }
                    }
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit the consumer.
                }
            }
            return output;
        }

        private static Result ProcessTruth(IPipeline pipeline, Truth truth)
        {
            // Get the data for the IP address.
            using var flowData = pipeline.CreateFlowData();
            flowData.AddEvidence("query.client-ip", truth.Ip);
            flowData.Process();
            var data = flowData.Get<IIpIntelligenceData>();

            // Get the latitude and longitude from the service.
            var latitude = GetValue(data.Latitude);
            var longitude = GetValue(data.Longitude);

            // Get the truth and result as points.
            var truthPoint = new GeoCoordinate(
                truth.Latitude, 
                truth.Longitude);
            var resultPoint = new GeoCoordinate(latitude, longitude);

            // Return the result including the latitude, longitude, and
            // distance in kilometers between the result and the truth.
            return new Result()
            {
                Latitude = latitude,
                Longitude = longitude,
                Confidence = GetValue(data.LocationConfidence),
                DistanceKms = truthPoint.GetDistanceTo(resultPoint) / 1000
            };
        }

        private static T? GetValue<T>(
            IAspectPropertyValue<IReadOnlyList<IWeightedValue<T>>> 
            value)
        {
            if (value.HasValue)
            {
                return value.Value[0].Value;
            }
            return default;
        }
    }

    static void Main(string[] args)
    {
        var configuration = new Configuration();

        // Use the supplied path for the data file or find the lite file that is included
        // in the repository.
        configuration.DataFile = args.Length > 0 ? args[0] :
            // In this example, by default, the 51degrees IP Intelligence data file needs to be somewhere in the
            // project space, or you may specify another file as a command line parameter.
            //
            // For testing, contact us to obtain an enterprise data file: https://51degrees.com/contact-us
            Examples.ExampleUtils.FindFile(Constants.ENTERPRISE_IPI_DATA_FILE_NAME);

        // Get the of the CSV file containing source truth.
        configuration.CsvTruthFile = args.Length > 1 ? args[1] : "todo";

        // Get the location for the output file. Use the same location as the
        // evidence if a path is not supplied on the command line.
        var outputFile = args.Length > 2 ? args[2] : "metrics-output.csv";

        File.WriteAllText("Metrics_DataFileName.txt", configuration.DataFile);

        // Configure a logger to output to the console.
        configuration.LoggerFactory = LoggerFactory.Create(b => b.AddConsole());
        
        if (configuration.DataFile != null)
        {
            using var output = File.CreateText(outputFile);
            configuration.Output = output;
            var builder = Host.CreateApplicationBuilder([]);
            builder.Services.AddSingleton(configuration);
            builder.Services.AddHostedService<Worker>();
            using var host = builder.Build();
            host.Run();
        }
        else
        {
            var logger = configuration.LoggerFactory.CreateLogger<Program>();
            logger.LogError("Failed to find a IP Intelligence data file. Make sure the " +
                "ip-intelligence-data submodule has been updated by running " +
                "`git submodule update --recursive`. By default, the 'lite' file included " +
                "with this code will be used. A different file can be specified " +
                "by supplying the full path as a command line argument");
        }

        // Dispose the logger to ensure any messages get flushed
        configuration.LoggerFactory.Dispose();
    }
}