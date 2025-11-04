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
namespace FiftyOne.IpIntelligence.Examples.OnPremise.Metrics;

public static class Extensions
{
    public static IEnumerable<(IPAddress, IPAddress)> 
        ValidRanges(this IpiOnPremiseEngine engine)
    {
        var network = engine.Components.Single(i =>
            "Network".Equals(
                i.Name,
                StringComparison.InvariantCultureIgnoreCase));
        foreach(var profile in engine.Profiles)
        {
            var range = GetRange(network, profile);
            if (range.Item1 != null && range.Item2 != null)
            {
                yield return range;
            }
        }
    }

    private static (IPAddress, IPAddress) GetRange(
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

    private static (IPAddress, IPAddress) GetRange(IProfileMetaData profile)
    {
        var start = profile.GetValues("IpRangeStart").Single();
        var end = profile.GetValues("IpRangeEnd").Single();
        return new(IPAddress.Parse(start.Name), IPAddress.Parse(end.Name));
    }
}

public class Program
{
    /// <summary>
    /// Time interval to wait between logging progress.
    /// </summary>
    private static readonly TimeSpan _logBuild = TimeSpan.FromSeconds(30);

    public class Metric
    {
        /// <summary>
        /// Properties for the fields returned in the ToString method.
        /// </summary>
        public static readonly string[] Properties = 
            ["IpCount", "AverageAreaKm", "EquivalentRadiusKm"];

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
            (int)Math.Sqrt((double)AverageAreaKm / Math.PI);

        public override string ToString() => 
            $"{IpCount},{AverageAreaKm},{EquivalentRadiusKm}";

        /// <summary>
        /// Merge the other metric instance with this one.
        /// </summary>
        /// <param name="other"></param>
        public void Merge(Metric other)
        {
            IpCount += other.IpCount;
            AreaCount += other.AreaCount;
            TotalAreaKm += other.TotalAreaKm;
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
        /// Returns the value as a string for inclusion in the key, or 
        /// "Missing" if the value can't be turned into a string.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        private static string GetValue(object obj)
        {
            if (obj is IAspectPropertyValue<IReadOnlyList<IWeightedValue<string>>>)
            {
                var value = obj as 
                    IAspectPropertyValue<IReadOnlyList<IWeightedValue<string>>>;
                if (value.HasValue && value.Value.Count == 1)
                {
                    return value.Value[0].Value;
                }
            }
            if (obj is IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>>)
            {
                var value = obj as
                    IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>>;
                if (value.HasValue && value.Value.Count == 1)
                {
                    return value.Value[0].Value.ToString();
                }
            }
            return "Missing";
        }
    }

    public class AreaIndex
    {
        /// <summary>
        /// Properties used by the Area index.
        /// </summary>
        public static readonly string[] Properties = ["Areas"];

        private static readonly WKTReader _wktReader = new();

        private static readonly CoordinateTransformationFactory
            _transformFactory = new();

        /// <summary>
        /// Dictionary that takes WKT value and returns the geographic area
        /// in square kms.
        /// </summary>
        public IReadOnlyDictionary<string, long> WktAreas;

        public AreaIndex(
            IFiftyOneAspectPropertyMetaData property,
            ILogger logger,
            CancellationToken stoppingToken)
        {
            WktAreas = BuildWktAreas(property, logger, stoppingToken);
        }

        private static IReadOnlyDictionary<string, long> BuildWktAreas(
            IFiftyOneAspectPropertyMetaData property,
            ILogger logger,
            CancellationToken stoppingToken)
        {
            var wktAreas = new ConcurrentDictionary<string, long>();
            var wktAreasTasks = new BlockingCollection<Task>(
                Environment.ProcessorCount);

            var producer = Task.Run(() =>
                AddAreas(
                    property, 
                    logger, 
                    wktAreas, 
                    wktAreasTasks, 
                    stoppingToken),
                stoppingToken);

            var nextLog = DateTime.UtcNow.Add(_logBuild);
            while (wktAreasTasks.IsCompleted == false &&
                wktAreasTasks.TryTake(out var task, -1, stoppingToken))
            {
                task.Wait();
                if (DateTime.UtcNow >= nextLog)
                {
                    logger.LogInformation(
                        "Mapped '{0}' areas",
                        wktAreas.Count);
                    nextLog = DateTime.UtcNow.Add(_logBuild);
                }
            }

            producer.Wait();

            return wktAreas;
        }

        private static void AddAreas(
            IFiftyOneAspectPropertyMetaData property,
            ILogger logger,
            ConcurrentDictionary<string, long> wktAreas,
            BlockingCollection<Task> wktAreasTasks,
            CancellationToken stoppingToken)
        {
            foreach (var area in property.GetValues())
            {
                wktAreasTasks.TryAdd(Task.Run(() =>
                    AddAreas(wktAreas, area.Name, logger)), -1, stoppingToken);
            }
            wktAreasTasks.CompleteAdding();
        }

        private static void AddAreas(
            ConcurrentDictionary<string, long> areas,
            string wkt,
            ILogger logger)
        {
            areas.GetOrAdd(wkt, (_) =>
            {
                try
                {
                    var geo = _wktReader.Read(wkt);
                    if (geo != null)
                    {
                        return (long)Math.Round(GetAreas(geo));
                    }
                }
                catch (Exception ex)
                {
                    logger.LogWarning(ex, wkt);
                }
                return 0L;
            });
        }

        private static double GetAreas(Geometry geo)
        {
            var area = 0.0;
            if (geo.NumGeometries > 0)
            {
                for (var i = 0; i < geo.NumGeometries; i++)
                {
                    area += GetArea(geo.GetGeometryN(i));
                }
            }
            else
            {
                area += GetArea(geo);
            }
            return area;
        }

        private static double GetArea(Geometry geo)
        {
            // Create UTM projected coordinate system for a specific zone
            // (e.g., Zone 30N for London)
            var utmZone = (int)Math.Floor((geo.InteriorPoint.X + 180) / 6) + 1;
            var isNorthernHemisphere = geo.InteriorPoint.Y >= 0;

            // Create a coordinate transformation
            var transform = _transformFactory.CreateFromCoordinateSystems(
                // Define the source (WGS84) and target (Projected) coordinate
                // systems
                GeographicCoordinateSystem.WGS84,
                // Adjust UTM zone and hemisphere as needed
                ProjectedCoordinateSystem.WGS84_UTM(
                    utmZone,
                    isNorthernHemisphere));

            // Re-project the polygon to the UTM coordinate system
            var transformedPolygon = TransformGeometry(
                geo,
                transform.MathTransform);

            // Calculate area in square meters and convert to square kilometers
            return transformedPolygon.Area / 1_000_000;
        }

        private static Geometry TransformGeometry(
            Geometry geometry,
            MathTransform transform)
        {
            var factory = geometry.Factory;
            var coordinates = geometry.Coordinates;

            for (int i = 0; i < coordinates.Length; i++)
            {
                var transformed =
                    transform.Transform([coordinates[i].X, coordinates[i].Y]);
                coordinates[i].X = transformed[0];
                coordinates[i].Y = transformed[1];
            }

            return factory.CreateGeometry(geometry);
        }
    }

    public sealed class Worker(
        IHostApplicationLifetime hostApplicationLifetime,
        string dataFile, 
        ILoggerFactory loggerFactory, 
        StreamWriter output) 
        : BackgroundService
    {
        protected override async Task ExecuteAsync(
            CancellationToken stoppingToken)
        {
            await new Example().Run(
                dataFile,
                loggerFactory, 
                output, 
                stoppingToken);
            hostApplicationLifetime.StopApplication();
        }
    }

    public class Example : ExampleBase
    {
        public async Task Run(
            string dataFile, 
            ILoggerFactory loggerFactory,
            TextWriter output, 
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
                .SetProperties(
                    KeyFactory.Keys.Concat(AreaIndex.Properties).ToList())
                .Build(dataFile, false);
            using var pipeline = new PipelineBuilder(loggerFactory)
                .AddFlowElement(ipiEngine).Build();
            var logger = loggerFactory.CreateLogger<Example>();

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
                new BlockingCollection<(IPAddress, IPAddress)>(
                Environment.ProcessorCount);

            // Start the consumers, they will be waiting for the producer to
            // start.
            var consumers = Enumerable.Range(
                0, 
                Environment.ProcessorCount).Select(_ =>
                Task.Factory.StartNew(() =>
                    ProcessRange(
                        pipeline, 
                        areaIndex, 
                        factory, 
                        ranges, 
                        stoppingToken),
                    TaskCreationOptions.LongRunning
                )).ToArray();
            logger.LogInformation(
                "Created '{0}' processors",
                consumers.Length);

            // Use the main thread as the producer adding ranges for the
            // consumers to process.
            var added = 0;
            var nextLog = DateTime.UtcNow.Add(_logBuild);
            foreach (var range in ipiEngine.ValidRanges().TakeWhile(
                _ => stoppingToken.IsCancellationRequested == false))
            {
                try
                {
                    ranges.TryAdd(range, -1, stoppingToken);
                    added++;
                    if (DateTime.UtcNow >= nextLog)
                    {
                        logger.LogInformation(
                            "Processed '{0}' ranges with '{1}' in queue, " +
                            "most recent '{2}', and '{3}' consumers",
                            added,
                            ranges.Count,
                            range.Item1,
                            consumers.Count(i => i.IsCompleted == false));
                        nextLog = DateTime.UtcNow.Add(_logBuild);
                    }
                }
                catch (OperationCanceledException)
                {
                    // Do nothing and exit as if the ranges had been fully
                    // consumed.
                }
            }
            ranges.CompleteAdding();

            // Combine all the consumer groups into the main groups.
            var groups = new Dictionary<string, Metric>();
            foreach(var consumer in consumers)
            {
                await consumer;
                foreach(var group in consumer.Result)
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

            // Write out the headings as the first line.
            output.WriteLine(String.Join(
                ",", 
                KeyFactory.Keys.Concat(Metric.Properties)
                .Select(i => "\"" + i + "\"")));

            // Write out all the groups.
            foreach (var group in groups.OrderBy(i => i.Key))
            {
                output.WriteLine(group.Key + "," + group.Value.ToString());
            }

            ExampleUtils.CheckDataFile(
                ipiEngine,
                loggerFactory.CreateLogger<Program>());
        }

        private static IReadOnlyDictionary<string, Metric> ProcessRange(
            IPipeline pipeline, 
            AreaIndex dataSet, 
            KeyFactory factory, 
            BlockingCollection<(IPAddress, IPAddress)> ranges,
            CancellationToken stoppingToken)
        {
            var groups = new Dictionary<string, Metric>();
            while (ranges.IsCompleted == false &&
                stoppingToken.IsCancellationRequested == false)
            {
                try
                {
                    if (ranges.TryTake(out var range, -1, stoppingToken))
                    {
                        var current = range.Item1;
                        while (range.Item2.Equals(current) == false &&
                            stoppingToken.IsCancellationRequested == false)
                        {
                            ProcessIp(pipeline, dataSet, groups, factory, current);
                            current = current.GetNextAddress();
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

            // Increase the number of IP address that relate to this key.
            metric.IpCount++;

            // Increase the total area and number of areas for the metric only
            // where a non zero area is available.
            foreach (var area in data.Areas.Value)
            {
                if (dataSet.WktAreas.TryGetValue(
                    area.Value,
                    out var value) && value > 0)
                {
                    metric.TotalAreaKm += value;
                    metric.AreaCount++;
                }
            }
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
        // Finally, get the location for the output file. Use the same location as the
        // evidence if a path is not supplied on the command line.
        var outputFile = args.Length > 1 ? args[1] : "metrics-output.csv";

        File.WriteAllText("Metrics_DataFileName.txt", dataFile);

        // Configure a logger to output to the console.
        var loggerFactory = LoggerFactory.Create(b => b.AddConsole());
        
        if (dataFile != null)
        {
            using var stream = File.CreateText(outputFile);
            var builder = Host.CreateApplicationBuilder([]);
            builder.Services.AddSingleton(dataFile);
            builder.Services.AddSingleton(stream);
            builder.Services.AddSingleton(loggerFactory);
            builder.Services.AddHostedService<Worker>();
            using var host = builder.Build();
            host.Run();
        }
        else
        {
            var logger = loggerFactory.CreateLogger<Program>();
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