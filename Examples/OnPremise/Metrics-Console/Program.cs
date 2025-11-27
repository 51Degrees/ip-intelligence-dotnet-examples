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

// Ignore Spelling: Wkt Ip

using CsvHelper;
using CsvHelper.Configuration;
using CsvHelper.Configuration.Attributes;
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
using NetTopologySuite.Algorithm;
using System;
using System.Buffers;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime;
using System.Threading;
using System.Threading.Channels;
using System.Threading.Tasks;

/// <summary>
/// @example OnPremise/Metrics-Console/Program.cs
///
/// This example takes all public IP addresses and passes them to the IP
/// Intelligence service, recording the average geographic area in square
/// kilometers, the number of polygons that form the area, and the equivalent
/// radius if the area can be represented as a circle.
/// 
/// Depending on the available processor cores the example can take a long time
/// to complete.
/// 
/// The sample of IP addresses used in the metrics can be adjusted as a
/// parameter.
/// 
/// This example is primarily designed for those who are interested in 
/// verifying the published metrics associated with 51Degrees' 
/// IP intelligence service.
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
            IsRegisteredCountryValid(profile))
        {
            return GetRange(profile);
        }
        return default;
    }

    private static bool IsRegisteredCountryValid(IProfileMetaData profile)
    {
        var value = profile.GetValue("RegisteredCountry", "Unknown");
        if (value == null) return true;
        value.Dispose();
        return false;
    }

    private static (string, string) GetRange(IProfileMetaData profile)
    {
        return new(
            GetValue(profile, "IpRangeStart"), 
            GetValue(profile, "IpRangeEnd"));
    }

    private static string GetValue(IProfileMetaData profile, string name)
    {
        foreach(var value in profile.GetValues(name))
        {
            var result = value.Name;
            value.Dispose();
            return result;
        }
        return null;
    }

    /// <summary>
    /// Increments the IP address in the buffer by 1.
    /// </summary>
    /// <param name="buffer">The IP address bytes to increment</param>
    /// <returns>
    /// True if successful, false if overflow (address was max value)
    /// </returns>
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

    public class Metric : Key
    {
        /// <summary>
        /// Number of IP addresses that relate to this metric.
        /// </summary>
        [Index(9)]
        public int IpCount { get; set; } = 0;

        /// <summary>
        /// The total area in km squared of all IPs.
        /// </summary>
        [Ignore]
        public long TotalAreaKm { get; set; } = 0;

        /// <summary>
        /// Number of areas included.
        /// </summary>
        [Index(10)]
        public int AreaCount { get; set; } = 0;

        /// <summary>
        /// Average area in km squared for the IPs, or 0 if there is no area
        /// available.
        /// </summary>
        [Index(11)]
        public long AverageAreaKm =>
            AreaCount > 0 ? TotalAreaKm / AreaCount : 0;

        /// <summary>
        /// The radius that a circle would need to so that it covered the same
        /// area as the <see cref="AverageAreaKm"/>.
        /// </summary>
        [Index(12)]
        public int EquivalentRadiusKm =>
            (int)Math.Sqrt(AverageAreaKm / Math.PI);

        /// <summary>
        /// Average number of polygons for the metric.
        /// </summary>
        [Index(14)]
        public double AveragePolygons
        {
            get
            {
                if (_polygons.Count == 0)
                {
                    return 0;
                }
                var weightedSum = _polygons.Sum(i => i.Key * i.Value.Value);
                var total = _polygons.Sum(i => i.Value.Value);
                return (double)weightedSum / total;
            }
        }

        /// <summary>
        /// Key is the number of areas, and the value the number of IPs that
        /// contain the areas.
        /// </summary>
        private readonly Dictionary<int, Counter> _polygons = [];

        public Metric(Key key) : base(
            key.ContinentName,
            key.Country,
            key.LocationConfidence,
            key.ConnectionType,
            key.IsVPN,
            key.IsProxy,
            key.IsTor,
            key.IsPublicRouter)
        { }

        public override string ToString() =>
            $"{base.ToString()}{IpCount},{AverageAreaKm}," +
            $"{EquivalentRadiusKm},{AveragePolygons.ToString("0.##")}";

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

    /// <summary>
    /// Key used for each metric.
    /// </summary>
    public class Key : IEquatable<Key>, IComparable<Key>
    {
        public Key(
            string continentName,
            string country, 
            string locationConfidence,
            string connectionType,
            string isVPN,
            string isProxy,
            string isTor,
            string isPublicRouter)
        {
            ContinentName = continentName;
            Country = country;
            LocationConfidence = locationConfidence;
            ConnectionType = connectionType;
            IsVPN = isVPN;
            IsProxy = isProxy;
            IsTor = isTor;
            IsPublicRouter = isPublicRouter;
            _hashCode = HashCode.Combine(
                continentName,
                country,
                locationConfidence,
                connectionType,
                isVPN,
                isProxy,
                isTor,
                isPublicRouter);
        }

        [Index(1)]
        public string ContinentName { get; set; }

        [Index(2)]
        public string Country { get; set; }

        [Index(3)]
        public string LocationConfidence { get; set; }

        [Index(4)]
        public string ConnectionType { get; set; }

        [Index(5)]
        public string IsVPN { get; set; }

        [Index(6)]
        public string IsProxy { get; set; }

        [Index(7)]
        public string IsTor { get; set; }

        [Index(8)]
        public string IsPublicRouter { get; set; }

        private readonly int _hashCode;

        public bool Equals(Key other)
        {
            return ContinentName.Equals(other.ContinentName) &&
                Country.Equals(other.Country) &&
                LocationConfidence.Equals(other.LocationConfidence) &&
                ConnectionType.Equals(other.ConnectionType) &&
                IsVPN.Equals(other.IsVPN) &&
                IsProxy.Equals(other.IsProxy) &&
                IsTor.Equals(other.IsTor) &&
                IsPublicRouter.Equals(other.IsPublicRouter);
        }

        public int CompareTo(Key other)
        {
            var difference = ContinentName.CompareTo(other.ContinentName);
            if (difference != 0) return difference;
            difference = Country.CompareTo(other.Country);
            if (difference != 0) return difference;
            difference = LocationConfidence.CompareTo(other.LocationConfidence);
            if (difference != 0) return difference;
            difference = ConnectionType.CompareTo(other.ConnectionType);
            if (difference != 0) return difference;
            difference = IsVPN.CompareTo(other.IsVPN);
            if (difference != 0) return difference;
            difference = IsProxy.CompareTo(other.IsProxy);
            if (difference != 0) return difference;
            difference = IsTor.CompareTo(other.IsTor);
            if (difference != 0) return difference;
            difference = IsPublicRouter.CompareTo(other.IsPublicRouter);
            if (difference != 0) return difference;
            return 0;
        }

        public override int GetHashCode() => _hashCode;
    }

    /// <summary>
    /// Factory used to create <see cref="Key"/>.
    /// </summary>
    public class KeyFactory
    {
        /// <summary>
        /// Returns the key for the data instance provided.
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        public Key Create(IIpIntelligenceData data)
        {
            return new Key(
                GetValue(data.ContinentName),
                GetValue(data.Country),
                GetValue(data.LocationConfidence),
                GetValue(data.ConnectionType),
                GetValue(data.IsVPN),
                GetValue(data.IsProxy),
                GetValue(data.IsTor),
                GetValue(data.IsPublicRouter));
        }

        /// <summary>
        /// Returns the string value or the no value message.
        /// <param name="value"></param>
        /// <returns></returns>
        private static string GetValue(IAspectPropertyValue<string> value)
        {
            if (value.HasValue)
                return value.Value;
            return value.NoValueMessage;
        }

        /// <summary>
        /// Returns the string value or the no value message.
        /// <param name="value"></param>
        /// <returns></returns>
        private static string GetValue(IAspectPropertyValue<bool> value)
        {
            if (value.HasValue)
                return value.Value.ToString();
            return value.NoValueMessage;
        }
    }

    public class Consumer
    {
        /// <summary>
        /// The task associated with the consumer.
        /// </summary>
        public Task<IReadOnlyDictionary<Key, Metric>> Task;

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
            var wktAreas = new Dictionary<string, Result>();
            var nextLog = DateTime.UtcNow.Add(_logBuild);
            Parallel.ForEach(
                property.GetValues(),
                new ParallelOptions() { CancellationToken = stoppingToken },
                () => new Dictionary<string, Result>(),
                (wkt, state, local) =>
                {
                    local.Add(wkt.Name, Calculations.GetAreas(wkt.Name, 0, 0));
                    wkt.Dispose();
                    return local;
                },
                local =>
                {
                    lock (wktAreas)
                    {
                        foreach (var pair in local)
                        {
                            wktAreas.Add(pair.Key, pair.Value);
                        }
                        if (DateTime.UtcNow >= nextLog)
                        {
                            logger.LogInformation(
                                "Mapped '{0}' areas",
                                wktAreas.Count);
                            nextLog = DateTime.UtcNow.Add(_logBuild);
                        }
                    }
                });
            return wktAreas;
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

        /// <summary>
        /// Function used to determine if an IP address range should be
        /// included.
        /// </summary>
        public Func<(string, string), bool> Condition;
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
                configuration.Condition,
                stoppingToken);
            hostApplicationLifetime.StopApplication();
        }
    }

    public class Example : ExampleBase
    {
        /// <summary>
        /// Runs the example as a task.
        /// </summary>
        /// <param name="dataFile">
        /// IP Intelligence data file to compile metrics for.
        /// </param>
        /// <param name="loggerFactory"></param>
        /// <param name="output">
        /// Output CSV writer for the metrics.
        /// </param>
        /// <param name="samplePercentage">
        /// Percentage of possible IP addresses to include in the metric.
        /// </param>
        /// <param name="condition">
        /// Function used to determine if an IP address range should be
        /// included.
        /// </param>
        /// <param name="stoppingToken">
        /// Cancellation token that when fired will gracefully stop the example
        /// and write the output metrics from the data processed to date.
        /// </param>
        /// <param name="logger"></param>
        /// <returns></returns>
        public static async Task Run(
            string dataFile,
            ILoggerFactory loggerFactory,
            TextWriter output,
            double samplePercentage,
            Func<(string, string), bool> condition,
            CancellationToken stoppingToken,
            ILogger logger = null)
        {
            logger ??= loggerFactory.CreateLogger<Example>();

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
                    typeof(Key).GetProperties().Select(i => i.Name).Concat(
                        AreaIndex.Properties).ToList())
                // Optimize for the expected parallel workload.
                .SetConcurrency((ushort)Environment.ProcessorCount)
                .Build(dataFile, false);
            using var pipeline = new PipelineBuilder(loggerFactory)
                .AddFlowElement(ipiEngine)
                .SetAutoDisposeElements(false)
                .Build();

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
                Channel.CreateBounded<(string, string)>(
                new BoundedChannelOptions(Environment.ProcessorCount) 
                {  
                    SingleWriter = true
                });
            var consumers = CreateConsumers(
                pipeline,
                areaIndex,
                factory,
                ranges,
                samplePercentage,
                stoppingToken);
            logger.LogInformation(
                "Created '{0}' consumer processors sampling '{1:P2}' of IPs",
                consumers.Length,
                samplePercentage);

            // Use the main thread as the producer adding ranges for the
            // consumers to process.
            AddRanges(
                ipiEngine,
                logger, 
                ranges,
                condition,
                consumers,
                stoppingToken);

            // Combine all the consumer groups into the main groups.
            var groups = new Dictionary<Key, Metric>();
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

            WriteToCsv(output, groups.Values);

            ExampleUtils.CheckDataFile(
                ipiEngine,
                loggerFactory.CreateLogger<Program>());
        }

        /// <summary>
        /// Write the metrics to the provided output in CSV format.
        /// </summary>
        /// <param name="output"></param>
        /// <param name="metrics"></param>
        private static void WriteToCsv(
            TextWriter output,
            IEnumerable<Metric> metrics)
        {
            using var writer = new CsvWriter(
                output,
                new CsvConfiguration(CultureInfo.InvariantCulture)
                {
                    Delimiter = ",",
                });

            // Write Records
            writer.WriteRecords(metrics.OrderBy(i => i));

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
                IPipeline pipeline,
                AreaIndex areaIndex,
                KeyFactory factory,
                Channel<(string, string)> ranges,
                double samplePercentage,
                CancellationToken stoppingToken)
        {
            return Enumerable.Range(
                0,
                Environment.ProcessorCount).Select(_ =>
                {
                    var consumer = new Consumer();
                    consumer.Task = ProcessRange(
                        pipeline,
                        areaIndex,
                        factory,
                        ranges,
                        consumer,
                        samplePercentage,
                        stoppingToken);
                    return consumer;
                }).ToArray();
        }

        private async static void AddRanges(
            IpiOnPremiseEngine ipiEngine,
            ILogger logger,
            Channel<(string, string)> ranges,
            Func<(string, string), bool> condition,
            Consumer[] consumers,
            CancellationToken stoppingToken)
        {
            var process = Process.GetCurrentProcess();
            var added = 0;
            var lastLog = DateTime.UtcNow;
            var nextLog = lastLog.Add(_logBuild);
            var lastProcessorTime = process.TotalProcessorTime;
            var source = condition == null ?
                ipiEngine.ValidRanges() :
                ipiEngine.ValidRanges().Where(i => condition(i));
            foreach (var range in source.TakeWhile(_ => 
                stoppingToken.IsCancellationRequested == false))
            {
                try
                {
                    await ranges.Writer.WaitToWriteAsync(stoppingToken);
                    await ranges.Writer.WriteAsync(range, stoppingToken);
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
            ranges.Writer.Complete();
            logger.LogInformation("Finished adding '{0}' ranges", added);
        }

        private static DateTime Log(
            ILogger logger,
            Channel<(string, string)> ranges,
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
                ranges.Reader.Count,
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

        private async static Task<IReadOnlyDictionary<Key, Metric>> 
            ProcessRange(
                IPipeline pipeline,
                AreaIndex dataSet,
                KeyFactory factory,
                Channel<(string, string)> ranges,
                Consumer consumer,
                double samplePercentage,
                CancellationToken stoppingToken)
        {
            var random = new Random();
            var groups = new Dictionary<Key, Metric>();
            var buffer4a = new byte[4];
            var buffer4b = new byte[4];
            var buffer6a = new byte[16];
            var buffer6b = new byte[16];
            byte[] currentIpBuffer;
            byte[] currentEndBuffer;
            try
            {
                await foreach (var range in ranges.Reader.ReadAllAsync(
                    stoppingToken))
                {
                    // Get the next range.
                    var startIp = IPAddress.Parse(range.Item1);
                    var endIp = IPAddress.Parse(range.Item2);

                    // Get a previously allocated byte array for this family.
                    switch (startIp.AddressFamily)
                    {
                        case AddressFamily.InterNetwork:
                            currentIpBuffer = buffer4a;
                            currentEndBuffer = buffer4b;
                            break;
                        default:
                            currentIpBuffer = buffer6a;
                            currentEndBuffer = buffer6b;
                            break;
                    }

                    // Add the current range bytes to these buffers.
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
                // Do nothing.
            }
            return groups;
        }

        private static void ProcessIp(
            IPipeline pipeline,
            AreaIndex dataSet,
            Dictionary<Key, Metric> groups,
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
                metric = new Metric(key);
                groups.Add(key, metric);
            }

            // Increase the number of IP addresses that relate to this key.
            metric.IpCount++;

            // Increase the total area and number of areas for the metric only
            // where a non zero area is available.
            if (data.Areas.HasValue && 
                dataSet.WktAreas.TryGetValue(
                    data.Areas.Value.Value,
                    out var value) && 
                value != null)
            {
                metric.TotalAreaKm += value.SquareKms;
                metric.IncrementPolygons(value.Geometries);
                metric.AreaCount++;
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

        // Only include IP addresses with periods in them. i.e. IPv4. There are
        // too many IPv6 addresses for the metrics example to complete in a 
        // short time frame.
        // configuration.Condition = (i) => i.Item1.Contains(".");

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