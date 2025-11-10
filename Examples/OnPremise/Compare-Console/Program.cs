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

using CsvHelper;
using CsvHelper.Configuration;
using CsvHelper.Configuration.Attributes;
using Examples.OnPremise.Areas;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using FiftyOne.Pipeline.Engines.Data;
using GeoCoordinatePortable;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Runtime;
using System.Threading;
using System.Threading.Tasks;

/// <summary>
/// @example OnPremise/Compare-Console/Program.cs
///
/// This example compares a CSV file containing known true IP addresses with
/// associated latitude and longitudes against an IP Intelligence service that
/// can return latitude and longitude and area information for a given IP 
/// address. This can be useful for understanding how the results of IP to
/// location compare to real world information when evaluating difference
/// solutions.
///
/// The example will ingest the following fields from a CSV file.
/// 
/// Date Time
/// IP address
/// Address Family (optional)
/// Latitude
/// Longitude
/// Continent (optional)
/// Country (optional)
/// 
/// See the `Truth` class for all fields and descriptions.
/// 
/// The IP Intelligence service is used to obtain the latitude and longitude
/// from the IP address. The distance in kilometers is then calculate along
/// with the confidence if available, the total geographic area covered by
/// the area returned, and an indicator as to if the provided latitude and
/// longitude is within the area returned.
/// 
/// Output fields are defined in the `Result` class and include.
/// 
/// Latitude
/// Longitude
/// Found
/// Geometries
/// SquareKms
/// DistanceKms
/// 
/// The output CSV file contains the input truth and the result fields for easy
/// evaluation.
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/Compare-Console/Program.cs). 
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
        /// The version of the IP address.
        /// </summary>
        public string AddressFamily { get; set; }

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

        /// <summary>
        /// The area in square kilometers that the IP address is likely to be
        /// found within.
        /// </summary>
        public int SquareKms { get; set; }

        /// <summary>
        /// The number of non overlapping areas that the IP address might be
        /// located in.
        /// </summary>
        public int Geometries { get; set; }

        /// <summary>
        /// True if the latitude and longitude provided in the truth was found
        /// in the area returned.
        /// </summary>
        public bool Contains { get; set; }
    }

    /// <summary>
    /// Classes used when writing out the CSV file both the truth provided and
    /// the result from the IP Intelligence service.
    /// </summary>
    /// <param name="truth"></param>
    /// <param name="result"></param>
    public class Output(Truth truth, Result result)
    {
        public Truth Truth => truth;
        public Result Result => result;
    }

    /// <summary>
    /// Wrapper for a consumer task that returns a list of records.
    /// </summary>
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
    
    /// <summary>
    /// Configuration passed to the worker host service via dependency
    /// injection.
    /// </summary>
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

    /// <summary>
    /// BackgroundService for running the <see cref="Example"/>.
    /// </summary>
    /// <param name="hostApplicationLifetime"></param>
    /// <param name="configuration"></param>
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

    /// <summary>
    /// Implementation of the example that can be called from the Program's
    /// main method or any other consuming service.
    /// </summary>
    public class Example : ExampleBase
    {
        public async Task Run(
            string dataFile, 
            string csvTruthFile,
            TextWriter output,
            ILoggerFactory loggerFactory,
            CancellationToken stoppingToken)
        {
            var logger = loggerFactory.CreateLogger<Example>();

            // Ensure that batch latency mode is always enabled.
            GCSettings.LatencyMode = GCLatencyMode.Batch;

            // Build a new on-premise IP Intelligence engine with the max
            // performance profile.
            using var ipiEngine = new IpiOnPremiseEngineBuilder(loggerFactory)
                // We use the max performance profile for optimal detection
                // speed in this example. See the documentation for more detail
                // on this and other configuration options.
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
                .SetProperty("Areas")
                // Optimize for the expected parallel workload.
                .SetConcurrency((ushort)Environment.ProcessorCount)
                .Build(dataFile, false);

            // Build a pipeline to consumer the IP intelligence engine. Needed
            // so that flowdata can be used to pass evidence in and get
            // results.
            using var pipeline = new PipelineBuilder(loggerFactory)
                .AddFlowElement(ipiEngine)
                .SetAutoDisposeElements(false)
                .Build();

            // Create a reader for the source of truth.
            using var reader = File.OpenText(csvTruthFile);
            var config = CsvConfiguration.FromAttributes<Truth>(
                CultureInfo.InvariantCulture);
            config.MissingFieldFound = null;
            using var source = new CsvReader(reader, config);

            // Create a collection of truths to ensure that there are records
            // always available to the consumer.
            var truth = new BlockingCollection<Truth>(
                Environment.ProcessorCount);

            // Create consumers that are used to add the result to the truth.
            // These run in parallel to ensure best performance as the IPI and 
            // area calculations can be time consuming compared to reading new
            // truth records.
            var consumers = CreateConsumers(
                pipeline,
                truth,
                stoppingToken);
            logger.LogInformation(
                "Created '{0}' consumer processors",
                consumers.Length);

            // Use the main thread as the producer adding truths for the
            // consumers to process.
            AddTruth(ipiEngine, logger, source, truth, consumers, stoppingToken);

            // Create the write for the destination output.
            using var writer = new CsvWriter(
                output,
                new CsvConfiguration(CultureInfo.InvariantCulture)
                {
                    Delimiter = ","
                });

            // Wait for the consumer to stop and then write out the records it
            // generated.
            foreach (var consumer in consumers)
            {
                await consumer.Task;
                logger.LogInformation(
                    "Finished consumer '{0}'", 
                    consumer.Task.Id);
                writer.WriteRecords(consumer.Task.Result);
            }

            // Finally check the data file used for consistency with the other
            // examples.
            ExampleUtils.CheckDataFile(
                ipiEngine,
                loggerFactory.CreateLogger<Program>());
        }

        /// <summary>
        /// Create and start the consumers which will be waiting on the
        /// producer to start. The number of consumers matches the number of
        /// processor cores.
        /// </summary>
        /// <param name="pipeline"></param>
        /// <param name="truth"></param>
        /// <param name="stoppingToken"></param>
        /// <returns></returns>
        private static Consumer[] CreateConsumers(
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
            var lastLog = DateTime.UtcNow;
            var nextLog = lastLog.Add(_logBuild);
            var lastProcessorTime = process.TotalProcessorTime;
            var ips = new HashSet<string>();
            foreach (var item in source.GetRecords<Truth>().TakeWhile(
                _ => stoppingToken.IsCancellationRequested == false))
            {
                try
                {
                    if (ips.Contains(item.Ip) == false)
                    {
                        truth.TryAdd(item, -1, stoppingToken);
                        ips.Add(item.Ip);
                    }
                    LogProgress(
                        logger, 
                        truth, 
                        consumers, 
                        process,
                        ips.Count, 
                        ref lastLog, 
                        ref nextLog, 
                        ref lastProcessorTime, 
                        item);
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit as if the truths had been fully
                    // consumed.
                }
            }
            truth.CompleteAdding();
            logger.LogInformation("Finished adding '{0}' sources", ips.Count);
        }

        /// <summary>
        /// Logs progress from the producer.
        /// </summary>
        /// <param name="logger"></param>
        /// <param name="truth"></param>
        /// <param name="consumers"></param>
        /// <param name="process"></param>
        /// <param name="added"></param>
        /// <param name="lastLog"></param>
        /// <param name="nextLog"></param>
        /// <param name="lastProcessorTime"></param>
        /// <param name="item"></param>
        private static void LogProgress(
            ILogger<Example> logger,
            BlockingCollection<Truth> truth, 
            Consumer[] consumers,
            Process process, 
            int added, 
            ref DateTime lastLog, 
            ref DateTime nextLog,
            ref TimeSpan lastProcessorTime,
            Truth item)
        {
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

        /// <summary>
        /// Processes the truths in the blocking collection until completed or
        /// stopped.
        /// </summary>
        /// <param name="pipeline"></param>
        /// <param name="source"></param>
        /// <param name="consumer"></param>
        /// <param name="stoppingToken"></param>
        /// <returns>
        /// A list of the output results.
        /// </returns>
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

        /// <summary>
        /// Process the specific truth returning the result.
        /// </summary>
        /// <param name="pipeline"></param>
        /// <param name="truth"></param>
        /// <returns></returns>
        private static Result ProcessTruth(IPipeline pipeline, Truth truth)
        {
            // Get the data for the IP address.
            using var flowData = pipeline.CreateFlowData();
            flowData.AddEvidence("query.client-ip", truth.Ip);
            flowData.Process();
            var data = flowData.Get<IIpIntelligenceData>();

            // Set the address family of the source truth does not provide it.
            if (String.IsNullOrEmpty(truth.AddressFamily))
            {
                truth.AddressFamily = IPAddress.Parse(truth.Ip)
                    .AddressFamily.ToString();
            }

            // Get the truth and result as points.
            var truthPoint = new GeoCoordinate(
                truth.Latitude, 
                truth.Longitude);
            var resultPoint = new GeoCoordinate(
                GetValue(data.Latitude), 
                GetValue(data.Longitude));

            // Get the area result for the returned data and the true latitude
            // and longitude.
            var area = Calculations.GetAreas(
                GetValue(data.Areas), 
                truth.Latitude,
                truth.Longitude);

            // Return the result including the latitude, longitude, and
            // distance in kilometers between the result and the truth.
            return new Result()
            {
                Latitude = resultPoint.Latitude,
                Longitude = resultPoint.Longitude,
                Confidence = GetValue(data.LocationConfidence),
                DistanceKms = truthPoint.GetDistanceTo(resultPoint) / 1000,
                SquareKms = area.SquareKms,
                Geometries = area.Geometries,
                Contains = area.Contains
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

        // Use the supplied path for the data file or find the lite file that
        // is included in the repository.
        configuration.DataFile = args.Length > 0 ? args[0] :
            // In this example, by default, the 51Degrees IP Intelligence data
            // file needs to be somewhere in the project space, or you may
            // specify another file as a command line parameter.
            //
            // For testing, contact us to obtain an enterprise data file:
            // https://51degrees.com/contact-us
            Examples.ExampleUtils.FindFile(
                Constants.ENTERPRISE_IPI_DATA_FILE_NAME);

        // Get the of the CSV file containing source truth.
        // TODO: Provide a simple example for testing purposes.
        configuration.CsvTruthFile = args.Length > 1 ? args[1] : "todo";

        // Get the location for the output file. Use the same location as the
        // evidence if a path is not supplied on the command line.
        var outputFile = args.Length > 2 ? args[2] : "compare-output.csv";

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
            logger.LogError("Failed to find a IP Intelligence data file. " +
                "Make sure the ip-intelligence-data submodule has been " +
                "updated by running `git submodule update --recursive`. By " +
                "default, the 'lite' file included with this code will be " +
                "used. A different file can be specified by supplying the " +
                "full path as a command line argument");
        }

        // Dispose the logger to ensure any messages get flushed
        configuration.LoggerFactory.Dispose();
    }
}