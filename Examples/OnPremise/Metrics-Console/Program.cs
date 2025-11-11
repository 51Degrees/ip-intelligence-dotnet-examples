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
using Examples.OnPremise.Areas;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines;
using FiftyOne.Pipeline.Engines.Data;
using FiftyOne.Pipeline.Engines.FiftyOne.Data;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime;
using System.Threading;
using System.Threading.Tasks;

/// <summary>
/// @example OnPremise/Metrics-Console/Program.cs
///
/// This example uses all the public IP addresses and passes them to the IP
/// Intelligence service recording the average geographic area in square
/// kilometers, the number of polygons that form the area, and the equivalent
/// circle radius if the area could be represented as a circle.
/// 
/// Depending on the available processor cores the example can take a long time
/// to complete.
/// 
/// The sample of IP addresses used in the metrics can be adjusted as a
/// parameter.
/// 
/// It is primarily designed for those who are interested to verify the
/// published metrics associated with 51Degrees IP intelligence service.
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
namespace FiftyOne.IpIntelligence.Examples.OnPremise.Metrics;

public static class Extensions
{
    public static IEnumerable<(string, string)>
        ValidRanges(this IpiOnPremiseEngine engine)
    {
        var network = engine.Components.Single(i =>
            "Network".Equals(
                i.Name,
                StringComparison.InvariantCultureIgnoreCase));
        foreach (var profile in engine.Profiles)
        {
            var range = GetRange(network, profile);
            if (range.Item1 != null && range.Item2 != null)
            {
                yield return range;
            }
            profile.Dispose();
        }
    }

    private static (string, string) GetRange(
        IComponentMetaData network,
        IProfileMetaData profile)
    {
        if (network.Equals(profile.Component) &&
            profile.GetValue("RegisteredCountry", "Unknown") == null)
        {
            return GetRange(profile);
        }
        return default;
    }

    private static (string, string) GetRange(IProfileMetaData profile)
    {
        var start = profile.GetValues("IpRangeStart").Single();
        var end = profile.GetValues("IpRangeEnd").Single();
        return new(start.Name, end.Name);
    }

    /// <summary>
    /// Increments the IP address in the buffer by 1.
    /// </summary>
    /// <param name="buffer">The IP address bytes to increment</param>
    /// <returns>True if successful, false if overflow (address was max value)</returns>
    public static bool TryGetNextAddress(Span<byte> buffer)
    {
        for (int i = buffer.Length - 1; i >= 0; i--)
        {
            if (buffer[i] < byte.MaxValue)
            {
                buffer[i]++;
                return true;
            }
            buffer[i] = 0;
        }
        return false; // Overflow
    }

    /// <summary>
    /// Compares two IP addresses represented as byte spans.
    /// </summary>
    /// <param name="a">First IP address bytes</param>
    /// <param name="b">Second IP address bytes</param>
    /// <returns>True if they are equal</returns>
    public static bool IpEquals(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b)
    {
        return a.SequenceEqual(b);
    }

    /// <summary>
    /// Get the next address after this one.
    /// </summary>
    /// <param name="ip"></param>
    /// <returns></returns>
    public static IPAddress GetNextAddress(this IPAddress ip)
    {
        return ip.GetNextAddress(0);
    }

    /// <summary>
    /// Get the next address after this one.
    /// </summary>
    /// <param name="ip"></param>
    /// <param name="ignoreBytes">
    /// The number of bytes to ignore, starting with the least 
    /// significant.
    /// e.g. 
    /// With ip 0.0.0.0 and ignore bytes 0, the result would be 0.0.0.1
    /// With ip 0.0.0.0 and ignore bytes 1, the result would be 0.0.1.0
    /// </param>
    /// <returns>
    /// The provided IPAddress with 1 added to the least significant 
    /// possible byte
    /// </returns>
    public static IPAddress GetNextAddress(
        this IPAddress ip,
        int ignoreBytes)
    {
        IPAddress result = null;

        var bytes = ip.GetAddressBytes();
        var newBytes = new byte[bytes.Length];

        var added = false;

        // Work through the IP address bytes from least significant 
        // to most significant.
        for (var i = newBytes.Length - 1; i >= 0; i--)
        {
            // If we've already added to a byte or if this byte
            // is being ignored, just copy the existing byte value 
            // to the new array.
            if (added || newBytes.Length - i <= ignoreBytes)
            {
                newBytes[i] = bytes[i];
            }
            // Otherwise, either add one to the byte value or, if it's 
            // already 255, set it to 0.
            else
            {
                if (bytes[i] == byte.MaxValue)
                {
                    newBytes[i] = 0;
                }
                else
                {
                    newBytes[i] = (byte)(bytes[i] + 1);
                    added = true;
                }
            }
        }
        // As long as we've managed to add to one of the bytes, set the
        // result from the new byte array.
        // Otherwise, return null. 
        // (e.g. if the provided IP was 255.255.255.255)
        if (added != false)
        {
            result = new IPAddress(newBytes);
        }

        return result;
    }
}

public class Program
{
    /// <summary>
    /// Time interval to wait between logging progress.
    /// </summary>
    private static readonly TimeSpan _logBuild = TimeSpan.FromSeconds(30);

    /// <summary>
    /// The percentage of registered IP addresses to randomly include in the
    /// metrics sample. 1 is 100%. Higher values will result in longer elapsed
    /// processing.
    /// </summary>
    private const double DEFAULT_SAMPLE_PERCENTAGE = 0.1;

    public class Counter
    {
        public int Value = 1;
    }

    public class Metric
    {
        /// <summary>
        /// Properties for the fields returned in the ToString method.
        /// </summary>
        public static readonly string[] Properties = [
            "IpCount",
            "AverageAreaKm",
            "EquivalentRadiusKm",
            "AveragePolygons"];

        /// <summary>
        /// The total area in km squared of all IPs.
        /// </summary>
        public long TotalAreaKm = 0;

        /// <summary>
        /// Number of areas included.
        /// </summary>
        public int AreaCount = 0;

        /// <summary>
        /// Number of IP addresses that relate to this metric.
        /// </summary>
        public int IpCount = 0;

        /// <summary>
        /// Average area in km squared for the IPs, or 0 if there is no area
        /// available.
        /// </summary>
        public long AverageAreaKm =>
            AreaCount > 0 ? TotalAreaKm / AreaCount : 0;

        /// <summary>
        /// The radius that a circle would need to so that it covered the same
        /// area as the <see cref="AverageAreaKm"/>.
        /// </summary>
        public int EquivalentRadiusKm =>
            (int)Math.Sqrt(AverageAreaKm / Math.PI);

        /// <summary>
        /// Key is the number of areas, and the value the number of IPs that
        /// contain the areas.
        /// </summary>
        public IReadOnlyDictionary<int, int> Polygons =>
            _polygons.ToDictionary(k => k.Key, v => v.Value.Value);
        private Dictionary<int, Counter> _polygons = [];

        /// <summary>
        /// Average number of polygons for the metric.
        /// </summary>
        public double AveragePolygons
        {
            get
            {
                if (Polygons.Count == 0)
                {
                    return 0;
                }
                var weightedSum = Polygons.Sum(i => i.Key * i.Value);
                var total = Polygons.Sum(i => i.Value);
                return (double)weightedSum / total;
            }
        }

        public override string ToString() =>
            $"{IpCount},{AverageAreaKm},{EquivalentRadiusKm}," +
            $"{AveragePolygons.ToString("0.##")}";

        /// <summary>
        /// Increase the count of IP addresses with the number of polygons
        /// provided.
        /// </summary>
        /// <param name="polygons"></param>
        public void IncrementPolygons(int polygons)
        {
            if (_polygons.TryGetValue(polygons, out var counter))
            {
                counter.Value++;
            }
            else
            {
                _polygons.Add(polygons, new Counter());
            }
        }

        /// <summary>
        /// Merge the other metric instance with this one.
        /// </summary>
        /// <param name="other"></param>
        public void Merge(Metric other)
        {
            IpCount += other.IpCount;
            AreaCount += other.AreaCount;
            TotalAreaKm += other.TotalAreaKm;
            foreach (var pair in other._polygons)
            {
                if (_polygons.TryGetValue(pair.Key, out var counter))
                {
                    counter.Value += pair.Value.Value;
                }
                else
                {
                    _polygons.Add(pair.Key, pair.Value);
                }
            }
        }
    }

    public class KeyFactory
    {
        /// <summary>
        /// The properties used to form the key.
        /// </summary>
        public static readonly string[] Keys = [
            "ContinentName",
            "Country",
            "LocationConfidence",
            "ConnectionType",
            "IsVPN",
            "IsProxy",
            "IsTor",
            "IsPublicRouter"];

        /// <summary>
        /// Returns the key for the data instance provided.
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        public string Create(IIpIntelligenceData data)
        {
            return String.Join(
                ",",
                Keys.Select(i => "\"" + GetValue(data[i]) + "\""));
        }

        /// <summary>
        /// Use pattern matching to return the value as a string 
        /// for inclusion in the key, or "Missing" if the value can't be
        /// turned into a string.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        private static string GetValue(object obj)
        {
            return obj switch
            {
                // Single value
                IAspectPropertyValue<string> 
                { HasValue: true, Value: var str } => str,

                // weighted string 
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<string>>> 
                { HasValue: true, Value: { Count: 1 } list }
                    => list[0].Value,

                // weighted bool
                IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>> 
                { HasValue: true, Value: { Count: 1 } list }
                    => list[0].Value.ToString(),

                // default
                _ => "Missing"
            };
        }
    }

    public class Consumer
    {
        /// <summary>
        /// The task associated with the consumer.
        /// </summary>
        public Task<IReadOnlyDictionary<string, Metric>> Task;

        /// <summary>
        /// The number of IP pipeline processes that have been completed in
        /// a given period of time. Used for logging during processing.
        /// </summary>
        public int Count;
    }

    public class AreaIndex
    {
        /// <summary>
        /// Properties used by the Area index.
        /// </summary>
        public static readonly string[] Properties = ["Areas"];

        /// <summary>
        /// Dictionary that takes WKT value and returns the geographic area
        /// in square kms and the number polygons that form the area.
        /// </summary>
        public IReadOnlyDictionary<string, Result> WktAreas;

        public AreaIndex(
            IFiftyOneAspectPropertyMetaData property,
            ILogger logger,
            CancellationToken stoppingToken)
        {
            WktAreas = BuildWktAreas(property, logger, stoppingToken);
        }

        private static IReadOnlyDictionary<string, Result> BuildWktAreas(
            IFiftyOneAspectPropertyMetaData property,
            ILogger logger,
            CancellationToken stoppingToken)
        {
            var wktAreas = new ConcurrentDictionary<string, Result>();
            var wktAreasTasks = new BlockingCollection<Task>(
                Environment.ProcessorCount);

            var producer = Task.Run(() =>
                AddAreas(
                    property,
                    wktAreas,
                    wktAreasTasks,
                    stoppingToken),
                stoppingToken);

            var nextLog = DateTime.UtcNow.Add(_logBuild);
            while (wktAreasTasks.IsCompleted == false &&
                wktAreasTasks.TryTake(out var task, -1, stoppingToken))
            {
                task.Wait(stoppingToken);
                if (DateTime.UtcNow >= nextLog)
                {
                    logger.LogInformation(
                        "Mapped '{0}' areas",
                        wktAreas.Count);
                    nextLog = DateTime.UtcNow.Add(_logBuild);
                }
            }

            producer.Wait(stoppingToken);

            return wktAreas;
        }

        private static void AddAreas(
            IFiftyOneAspectPropertyMetaData property,
            ConcurrentDictionary<string, Result> wktAreas,
            BlockingCollection<Task> wktAreasTasks,
            CancellationToken stoppingToken)
        {
            foreach (var area in property.GetValues())
            {
                wktAreasTasks.TryAdd(Task.Run(() =>
                    AddAreas(wktAreas, area.Name), stoppingToken),
                    -1, stoppingToken);
            }
            wktAreasTasks.CompleteAdding();
        }

        private static void AddAreas(
            ConcurrentDictionary<string, Result> areas,
            string wkt)
        {
            areas.GetOrAdd(wkt, (_) => Calculations.GetAreas(wkt, 0, 0));
        }
    }

    public class Configuration
    {
        /// <summary>
        /// IPI data file.
        /// </summary>
        public string DataFile;

        /// <summary>
        /// Stream to write the output to.
        /// </summary>
        public StreamWriter Output;

        /// <summary>
        /// Sample of IP addresses to sample for the metrics.
        /// </summary>
        public double SamplePercentage;

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
            await Example.Run(
                configuration.DataFile,
                configuration.LoggerFactory,
                configuration.Output,
                configuration.SamplePercentage,
                stoppingToken);
            hostApplicationLifetime.StopApplication();
        }
    }

    public class Example : ExampleBase
    {
        public static async Task Run(
            string dataFile,
            ILoggerFactory loggerFactory,
            TextWriter output,
            double samplePercentage,
            CancellationToken stoppingToken)
        {
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
                .SetProperties(
                    KeyFactory.Keys.Concat(AreaIndex.Properties).ToList())
                // Optimize for the expected parallel workload.
                .SetConcurrency((ushort)Environment.ProcessorCount)
                .Build(dataFile, false);
            using var pipeline = new PipelineBuilder(loggerFactory)
                .AddFlowElement(ipiEngine)
                .SetAutoDisposeElements(false)
                .Build();
            var logger = loggerFactory.CreateLogger<Example>();

            // Log data file information
            var dataFilePublishDate = ipiEngine.DataFiles[0].DataPublishedDateTime;
            var dataFileTier = ipiEngine.DataSourceTier;
            logger.LogInformation(
                "Using data file: {0} (tier: {1}, published: {2:yyyy-MM-dd})",
                dataFile,
                dataFileTier,
                dataFilePublishDate);

            logger.LogInformation("Server GC: '{0}'", GCSettings.IsServerGC);

            // Create the data set structure to rapidly retrieve the data
            // needed to calculate the metrics and to determine the valid
            // IP ranges.
            logger.LogInformation("Creating area index");
            var areaIndex = new AreaIndex(
                ipiEngine.Properties.Single(i => "Areas".Equals(
                    i.Name,
                    StringComparison.InvariantCultureIgnoreCase)),
                logger,
                stoppingToken);
            logger.LogInformation(
                "Created index for '{0}' areas",
                areaIndex.WktAreas.Count);

            // Perform IP lookup for all the IP addresses in the ranges
            // and record the results.
            var factory = new KeyFactory();

            // Create a collection of ranges to ensure that there are always
            // items available for the consumers.
            var ranges =
                new BlockingCollection<(string, string)>(
                Environment.ProcessorCount);
            var consumers = CreateConsumers(
                samplePercentage,
                pipeline,
                areaIndex,
                factory,
                ranges,
                stoppingToken);
            logger.LogInformation(
                "Created '{0}' consumer processors sampling '{1:P2}' of IPs",
                consumers.Length,
                samplePercentage);

            // Use the main thread as the producer adding ranges for the
            // consumers to process.
            AddRanges(ipiEngine, logger, ranges, consumers, stoppingToken);

            // Combine all the consumer groups into the main groups.
            var groups = new Dictionary<string, Metric>();
            foreach (var consumer in consumers)
            {
                await consumer.Task;
                logger.LogInformation(
                    "Finished consumer '{0}'",
                    consumer.Task.Id);
                foreach (var group in consumer.Task.Result)
                {
                    if (groups.TryGetValue(group.Key, out var metric))
                    {
                        metric.Merge(group.Value);
                    }
                    else
                    {
                        groups.Add(group.Key, group.Value);
                    }
                }
            }

            WriteToCsv(output, groups);

            ExampleUtils.CheckDataFile(
                ipiEngine,
                loggerFactory.CreateLogger<Program>());
        }

        /// <summary>
        /// Write the metrics to the provided output in CSV format.
        /// </summary>
        /// <param name="output"></param>
        /// <param name="groups"></param>
        private static void WriteToCsv(TextWriter output, Dictionary<string, Metric> groups)
        {
            using var writer = new CsvWriter(
                output,
                new CsvConfiguration(CultureInfo.InvariantCulture)
                {
                    Delimiter = ","
                });

            // write Header
            writer.WriteRecord(KeyFactory.Keys.Concat(Metric.Properties));
            writer.NextRecord();

            writer.WriteRecords(groups.OrderBy(i => i.Key));

            writer.Flush();
        }

        /// <summary>
        /// Create and start the consumers which will be waiting on the
        /// producer to start.
        /// </summary>
        /// <param name="samplePercentage"></param>
        /// <param name="pipeline"></param>
        /// <param name="areaIndex"></param>
        /// <param name="factory"></param>
        /// <param name="ranges"></param>
        /// <param name="stoppingToken"></param>
        /// <returns></returns>
        private static Consumer[]
            CreateConsumers(
                double samplePercentage,
                IPipeline pipeline,
                AreaIndex areaIndex,
                KeyFactory factory,
                BlockingCollection<(string, string)> ranges,
                CancellationToken stoppingToken)
        {
            return Enumerable.Range(
                0,
                Environment.ProcessorCount).Select(_ =>
                {
                    var consumer = new Consumer();
                    consumer.Task = Task.Factory.StartNew(() =>
                        ProcessRange(
                            pipeline,
                            areaIndex,
                            factory,
                            samplePercentage,
                            ranges,
                            consumer,
                            stoppingToken),
                        TaskCreationOptions.LongRunning);
                    return consumer;
                }).ToArray();
        }

        private static void AddRanges(
            IpiOnPremiseEngine ipiEngine,
            ILogger<Example> logger,
            BlockingCollection<(string, string)> ranges,
            Consumer[] consumers,
            CancellationToken stoppingToken)
        {
            var process = Process.GetCurrentProcess();
            var added = 0;
            var lastLog = DateTime.UtcNow;
            var nextLog = lastLog.Add(_logBuild);
            var lastProcessorTime = process.TotalProcessorTime;
            foreach (var range in ipiEngine.ValidRanges().TakeWhile(
                _ => stoppingToken.IsCancellationRequested == false))
            {
                try
                {
                    ranges.TryAdd(range, -1, stoppingToken);
                    added++;
                    if (DateTime.UtcNow >= nextLog)
                    {
                        nextLog = Log(
                            logger, 
                            ranges, 
                            consumers, 
                            process, 
                            added, 
                            ref lastLog,
                            ref lastProcessorTime, 
                            range);
                    }
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit as if the ranges had been fully
                    // consumed.
                }
            }
            ranges.CompleteAdding();
            logger.LogInformation("Finished adding '{0}' ranges", added);
        }

        private static DateTime Log(
            ILogger<Example> logger,
            BlockingCollection<(string, string)> ranges,
            Consumer[] consumers, 
            Process process, 
            int added, 
            ref DateTime lastLog, 
            ref TimeSpan lastProcessorTime, 
            (string, string) range)
        {
            // Log the ranges and other telemetry.
            logger.LogInformation(
                "Processed '{0}' ranges with '{1}' in " +
                "queue, most recent '{2}', and '{3}' " +
                "consumers",
                added,
                ranges.Count,
                range.Item1,
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
            lastProcessorTime = process.TotalProcessorTime;
            return DateTime.UtcNow.Add(_logBuild);
        }

        private static IReadOnlyDictionary<string, Metric> ProcessRange(
            IPipeline pipeline,
            AreaIndex dataSet,
            KeyFactory factory,
            double samplePercentage,
            BlockingCollection<(string, string)> ranges,
            Consumer consumer,
            CancellationToken stoppingToken)
        {
            var random = new Random();
            var groups = new Dictionary<string, Metric>();

            // Allocate buffers outside the loop to avoid stack overflow warnings
            Span<byte> ipBuffer = stackalloc byte[16]; // Max size for IPv6
            Span<byte> endBuffer = stackalloc byte[16];

            while (ranges.IsCompleted == false &&
                stoppingToken.IsCancellationRequested == false)
            {
                try
                {
                    if (ranges.TryTake(out var range, -1, stoppingToken))
                    {
                        var startIp = IPAddress.Parse(range.Item1);
                        var endIp = IPAddress.Parse(range.Item2);

                        // Determine buffer size based on address family
                        int bufferSize = 
                            startIp.AddressFamily == 
                            AddressFamily.InterNetwork ? 4 : 16;

                        // Get the relevant slice of the buffer for this
                        // address family
                        var currentIpBuffer = ipBuffer.Slice(
                            0, 
                            bufferSize);
                        var currentEndBuffer = endBuffer.Slice(
                            0, 
                            bufferSize);

                        startIp.TryWriteBytes(currentIpBuffer, out _);
                        endIp.TryWriteBytes(currentEndBuffer, out _);

                        while (!Extensions.IpEquals(
                            currentIpBuffer, 
                            currentEndBuffer) &&
                            stoppingToken.IsCancellationRequested == false)
                        {
                            if (random.NextDouble() <= samplePercentage)
                            {
                                // Only allocate IPAddress object when we
                                // actually need to process it
                                var ip = new IPAddress(currentIpBuffer);
                                ProcessIp(
                                    pipeline,
                                    dataSet,
                                    groups,
                                    factory,
                                    ip);
                                consumer.Count++;
                            }

                            // Increment to next IP address (operates on stack
                            // buffer, no allocation)
                            if (!Extensions.TryGetNextAddress(currentIpBuffer))
                                break; // Overflow, reached maximum IP
                        }
                    }
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit the consumer.
                }
            }
            return groups;
        }

        private static void ProcessIp(
            IPipeline pipeline,
            AreaIndex dataSet,
            Dictionary<string, Metric> groups,
            KeyFactory factory,
            IPAddress ipAddress)
        {
            // Get the data for the IP address.
            using var flowData = pipeline.CreateFlowData();
            flowData.AddEvidence("query.client-ip", ipAddress.ToString());
            flowData.Process();
            var data = flowData.Get<IIpIntelligenceData>();

            // Get the metric instance for the group key.
            var key = factory.Create(data);
            if (groups.TryGetValue(key, out var metric) == false)
            {
                metric = new Metric();
                groups.Add(key, metric);
            }

            // Increase the number of IP addresses that relate to this key.
            metric.IpCount++;

            // Increase the total area and number of areas for the metric only
            // where a non zero area is available.
            foreach (var area in data.Areas.Value)
            {
                if (dataSet.WktAreas.TryGetValue(
                    area.Value,
                    out var value) && value != null)
                {
                    metric.TotalAreaKm += value.SquareKms;
                    metric.IncrementPolygons(value.Geometries);
                    metric.AreaCount++;
                }
            }
        }
    }

    static void Main(string[] args)
    {
        var configuration = new Configuration
        {
            // Use the supplied path for the data file or find the lite file that is included
            // in the repository.
            DataFile = args.Length > 0 ? args[0] :
            // In this example, by default, the 51degrees IP Intelligence data file needs to be somewhere in the
            // project space, or you may specify another file as a command line parameter.
            //
            // For testing, contact us to obtain an enterprise data file: https://51degrees.com/contact-us
            Examples.ExampleUtils.FindFile(
                Constants.ENTERPRISE_IPI_DATA_FILE_NAME)
        };

        // Get the location for the output file. Use the same location as the
        // evidence if a path is not supplied on the command line.
        var outputFile = args.Length > 1 ? args[1] : "metrics-output.csv";

        // Get the sample percentage or use the default.
        configuration.SamplePercentage = args.Length > 2 ?
            double.Parse(args[2]) :
            DEFAULT_SAMPLE_PERCENTAGE;

        File.WriteAllText("Metrics_DataFileName.txt", configuration.DataFile);

        // Configure a logger to output to the console with timestamps.
        configuration.LoggerFactory = LoggerFactory.Create(b =>
            b.AddSimpleConsole(options =>
            {
                options.IncludeScopes = false;
                options.SingleLine = true;
                options.TimestampFormat = "yyyy-MM-dd HH:mm:ss ";
            }));
        var logger = configuration.LoggerFactory.CreateLogger<Program>();

        if (configuration.DataFile != null)
        {
            logger.LogInformation($"Found IPI data file: {configuration.DataFile}");
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