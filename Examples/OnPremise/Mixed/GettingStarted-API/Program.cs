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

using FiftyOne.DeviceDetection.Hash.Engine.OnPremise.FlowElements;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.IpIntelligence.Examples.Mixed.OnPremise;
using FiftyOne.Pipeline.Core.Configuration;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines.FiftyOne.Data;
using FiftyOne.Pipeline.Engines.FlowElements;
using FiftyOne.Pipeline.Engines.Services;
using FiftyOne.Pipeline.JsonBuilder.Data;
using FiftyOne.Pipeline.Web.Shared;
using System.IO.Compression;
using System.Security.Cryptography;

/// <summary>
/// @example OnPremise/GettingStarted-API/Program.cs
/// 
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/master/Examples/OnPremise/GettingStarted-API/Program.cs). 
/// 
/// 
/// Required NuGet Dependencies:
/// - [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence/)
/// </summary>
namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedAPI
{
    /// <summary>
    /// Local version of IPI API.
    /// </summary>
    /// <seealso cref="https://cloud.51degrees.com/api-docs/index.html"/>
    public class Program
    {
        public static void Main(string[] args)
        {
            var builder = WebApplication.CreateBuilder(args);
            builder.WebHost.UseUrls("http://0.0.0.0:5225");
            AppendConfigOverrides(builder.Configuration);

            // Add services to the container.
            builder.Services.AddAuthorization();


            // Add the hash engine builder to services so that the system can find the builder
            // when it needs to.
            // builder.Services.AddTransient<IDataUpdateService, DataUpdateService>();
            builder.Services.AddHttpClient<IDataUpdateService, DataUpdateService>(client =>
            {
                client.Timeout = TimeSpan.FromMinutes(20);
            });
            builder.Services.AddSingleton<IpiOnPremiseEngineBuilder>();
            builder.Services.AddSingleton<DeviceDetectionHashEngineBuilder>();

            // Configure the services needed by IP Intelligence and create the 51Degrees Pipeline
            // instance that will be used to process requests.
            builder.Services.AddFiftyOne(builder.Configuration);

            var app = builder.Build();


            app.UseHttpsRedirection();
            app.UseAuthorization();

            app.Map("/accessibleproperties", AccessibleProperties).WithName(nameof(AccessibleProperties));
            app.Map("/accessibleproperties/{resource}", AccessibleProperties).WithName(nameof(AccessibleProperties) + "WithResource");

            app.Map("/evidencekeys", EvidenceKeys).WithName(nameof(EvidenceKeys));
            app.Map("/evidencekeys/{resource}", EvidenceKeys).WithName(nameof(EvidenceKeys) + "WithResource");

            app.Map("/json", ProcessEvidence).WithName(nameof(ProcessEvidence));
            app.Map("/{resource}.json", ProcessEvidence).WithName(nameof(ProcessEvidence) + "WithResource");
            
            app.MapGet("/download-ipi-gz", GetDataFile).WithName(nameof(GetDataFile));
            
            // Force pipeline initialization before accepting first request.
            app.Services.GetService<IPipeline>();
            
            app.Run();
        }

        private static async Task GetDataFile(
            HttpContext context,
            IPipeline pipeline,
            ILogger<Program> logger)
        {
            var token = context.RequestAborted;
            var sourcePath = pipeline.GetElement<IpiOnPremiseEngine>().DataFiles[0].DataFilePath;

            try
            {
                // Buffer the compressed content
                logger.LogInformation("({TS:O}) [{FUNC}] compressing the file...",
                    DateTime.Now, nameof(GetDataFile));

                await using var sourceStream = File.OpenRead(sourcePath);
                await using var memoryStream = new MemoryStream();
                await using (var gzipStream = new GZipStream(memoryStream, CompressionLevel.Fastest, leaveOpen: true))
                {
                    await sourceStream.CopyToAsync(gzipStream, token);
                }

                logger.LogInformation("({TS:O}) [{FUNC}] calculating MD5...",
                    DateTime.Now, nameof(GetDataFile));

                // Reset position to read from beginning
                memoryStream.Position = 0;

                // Compute MD5 of compressed content
                using var md5 = MD5.Create();
                var hashBytes = await md5.ComputeHashAsync(memoryStream, token);
                // var hashBase64 = Convert.ToBase64String(hashBytes);
                // var hashHex = Convert.ToHexStringLower(hashBytes);
                var hashHex = Convert.ToHexString(hashBytes).ToLowerInvariant();
                string md5Hash = hashHex;

                logger.LogInformation("({TS:O}) [{FUNC}] MD5 = {MD5}",
                    DateTime.Now, nameof(GetDataFile), md5Hash);

                // Reset again for streaming
                memoryStream.Position = 0;

                // Set headers
                context.Response.ContentType = "application/gzip";
                context.Response.Headers.ContentDisposition =
                    $"attachment; filename=\"{Path.GetFileName(sourcePath)}.gz\"";
                context.Response.Headers["Content-MD5"] = md5Hash;

                // Stream the buffered content
                await memoryStream.CopyToAsync(context.Response.Body, token);
            }
            catch (OperationCanceledException)
            {
                logger.LogInformation("({TS:O}) [{FUNC}] delivery finished ({Outcome}).",
                    DateTime.Now, nameof(GetDataFile), "cancelled");
                throw;
            }

            logger.LogInformation("({TS:O}) [{FUNC}] delivery finished ({Outcome}).",
                DateTime.Now, nameof(GetDataFile), "done");
        }

        private static IResult AccessibleProperties(string? resource, HttpContext httpContext, IPipeline pipeline)
        {
            Dictionary<string, ProductMetaData> products = new(
                pipeline.FlowElements
                .SelectMany(x => x is IAspectEngine eng ? [eng] : (IEnumerable<IAspectEngine>)[])
                .Select(eng => new KeyValuePair<string, ProductMetaData>(eng.ElementDataKey, new ProductMetaData
                {
                    Properties = GetEngineProperties(eng).ToList(),
                })));

            return Results.Json(new
            {
                Products = products,
            });
        }

        private static IEnumerable<PropertyMetaData> GetEngineProperties(IAspectEngine aspectEngine)
        {
            if (aspectEngine is IpiOnPremiseEngine ipiEngine)
            {
                return ipiEngine.Components.SelectMany(comp
                    => comp.Properties.Select(prop => PropMetadataForIpiComponent(prop, comp)));
            }
            return aspectEngine.Properties.Select(prop => new PropertyMetaData(prop));
        }

        private static PropertyMetaData PropMetadataForIpiComponent(
            IFiftyOneAspectPropertyMetaData property,
            IComponentMetaData component)
        {
            var result = new PropertyMetaData(property)
            {
                Category = component.Name
            };
            if (GetTypeOverrideForPropertyType(property.Type) is { } typeOverride)
            {
                result.Type = typeOverride;
            }
            return result;
        }

        private static string? GetTypeOverrideForPropertyType(Type type)
        {
            if (!type.IsGenericType)
                return null;
            var enumerableType = type.GetGenericTypeDefinition() == typeof(IWeightedValue<>)
                ? type
                : type.GetInterfaces().FirstOrDefault(
                x => x.IsGenericType
                && x.GetGenericTypeDefinition() == typeof(IEnumerable<>));
            if (enumerableType is null)
                return null;
            var topTypes = enumerableType.GetGenericArguments();
            if (topTypes is null)
                return null;
            if (topTypes.Length != 1)
                return null;
            if (!topTypes[0].IsGenericType)
                return null;
            var weightedType = topTypes[0].GetGenericTypeDefinition() == typeof(IWeightedValue<>)
                ? topTypes[0]
                : topTypes[0].GetInterfaces().FirstOrDefault(
                    x => x.IsGenericType
                    && x.GetGenericTypeDefinition() == typeof(IWeightedValue<>));
            if (weightedType is null)
                return null;
            var deepTypes = weightedType.GetGenericArguments();
            if (weightedType is null)
                return null;
            if (deepTypes.Length != 1)
                return null;
            if (deepTypes[0].IsGenericType)
                return null;
            return "Weighted" + deepTypes[0].Name;
        }

        private static IResult EvidenceKeys(string? resource, HttpContext httpContext, IPipeline pipeline)
        {
            var keys = pipeline.FlowElements
            .Select(x => x.EvidenceKeyFilter as EvidenceKeyFilterWhitelist)
            .Where(x => x is not null)
            .SelectMany(x => x?.Whitelist.Keys ?? [])
            .Distinct()
            .ToList();
            return Results.Json(keys);
        }

        private static IResult ProcessEvidence(string? resource, HttpContext context, IPipeline pipeline) 
        {
            var aggregated = new Dictionary<string, object>(StringComparer.OrdinalIgnoreCase);

            foreach (var kvp in context.Request.Query)
            {
                aggregated["query." + kvp.Key] = kvp.Value.LastOrDefault() ?? string.Empty;
            }
            foreach (var kvp in context.Request.Headers)
            {
                aggregated["header." + kvp.Key] = kvp.Value.LastOrDefault() ?? string.Empty;
            }
            foreach (var kvp in context.Request.Cookies)
            {
                aggregated["cookie." + kvp.Key] = kvp.Value;
            }
            if (context.Request.HasFormContentType)
            {
                foreach (var kvp in context.Request.Form)
                {
                    aggregated["query." + kvp.Key] = kvp.Value.ToString();
                }
            }

            using var flowData = pipeline.CreateFlowData();
            flowData.AddEvidence(aggregated);
            flowData.Process();

            if (flowData.Get<IJsonBuilderElementData>()?.Json is { } json)
            {
                return Results.Text(json, "application/json");
            }
            return Results.NotFound("No results.");
        }

        /// <summary>
        /// Typically, something like this will not be necessary.
        /// The IP Intelligence API will accept an absolute or relative path for the data file.
        /// However, if a relative path is specified, it will only look in the current working 
        /// directory.
        /// In our examples, we have many different projects and we don't want to have a copy of 
        /// the data file for every single one.
        /// In order to handle this, we dynamically search the project directories for the data 
        /// file location and then override the configured setting with the absolute path if 
        /// necessary.
        /// In a real-world scenario, you can just put the data file in your working directory
        /// or use an absolute path in the configuration file.
        /// </summary>
        private static void AppendConfigOverrides(ConfigurationManager configurationManager)
        {
            var overrides = new Dictionary<string, string?>();

            // Bind the configuration to a pipeline options instance
            PipelineOptions options = new PipelineWebIntegrationOptions();
            var section = configurationManager.GetRequiredSection("PipelineOptions");
            // Use the 'ErrorOnUnknownConfiguration' option to warn us if we've got any
            // misnamed configuration keys.
            section.Bind(options, (o) => { o.ErrorOnUnknownConfiguration = true; });

            AddOverrides_IPI(options, overrides);
            AddOverrides_DD(options, overrides);

            configurationManager.AddInMemoryCollection(overrides);
        }

        private static void AddOverrides_IPI(PipelineOptions options, Dictionary<string, string?> overrides)
        {
            // Get the index of the IP Intelligence engine element in the config file so that
            // we can create an override key for it.
            var ipiEngineOptions = options.GetElementConfig(nameof(IpiOnPremiseEngine));
            var ipiEngineIndex = options.Elements.IndexOf(ipiEngineOptions);
            var dataFileConfigKey = $"PipelineOptions:Elements:{ipiEngineIndex}" +
                                    $":BuildParameters:DataFile";

            var dataFile = options.GetIpiDataFile();
            var foundDataFile = false;
            if (string.IsNullOrEmpty(dataFile))
            {
                throw new Exception($"A data file must be specified in the appsettings.json file.");
            }
            // The data file location provided in the configuration may be using an absolute or
            // relative path. If it is relative then search for a matching file using the 
            // ExampleUtils.FindFile function.
            if (Path.IsPathRooted(dataFile) == false)
            {
                var newPath = Examples.ExampleUtils.FindFile(dataFile);
                if (newPath != null)
                {
                    // Add an override for the absolute path to the data file.
                    overrides.Add(dataFileConfigKey, newPath);
                    foundDataFile = true;
                }
            }
            else
            {
                foundDataFile = File.Exists(dataFile);
            }

            if (foundDataFile == false)
            {
                throw new Exception($"Failed to find a IP Intelligence data file matching " +
                                    $"'{dataFile}'. If using the lite file, then make sure the " +
                                    $"ip-intelligence-data submodule has been updated by running " +
                                    "`git submodule update --recursive`. Otherwise, ensure that the filename " +
                                    "is correct in appsettings.json.");
            }
        }

        private static void AddOverrides_DD(PipelineOptions options, Dictionary<string, string?> overrides)
        {
            // Get the index of the device detection engine element in the config file so that
            // we can create an override key for it.
            var hashEngineOptions = options.GetElementConfig(nameof(DeviceDetectionHashEngine));
            var hashEngineIndex = options.Elements.IndexOf(hashEngineOptions);
            var dataFileConfigKey = $"PipelineOptions:Elements:{hashEngineIndex}" +
                                    $":BuildParameters:DataFile";

            var dataFile = options.GetHashDataFile();
            var foundDataFile = false;
            if (string.IsNullOrEmpty(dataFile))
            {
                throw new Exception($"A data file must be specified in the appsettings.json file.");
            }
            // The data file location provided in the configuration may be using an absolute or
            // relative path. If it is relative then search for a matching file using the 
            // ExampleUtils.FindFile function.
            if (Path.IsPathRooted(dataFile) == false)
            {
                var newPath = Examples.ExampleUtils.FindFile(dataFile);
                if (newPath != null)
                {
                    // Add an override for the absolute path to the data file.
                    overrides.Add(dataFileConfigKey, newPath);
                    foundDataFile = true;
                }
            }
            else
            {
                foundDataFile = File.Exists(dataFile);
            }

            if (foundDataFile == false)
            {
                throw new Exception($"Failed to find a device detection data file matching " +
                                    $"'{dataFile}'. If using the lite file, then make sure the " +
                                    $"device-detection-data submodule has been updated by running " +
                                    "`git submodule update --recursive`. Otherwise, ensure that the filename " +
                                    "is correct in appsettings.json.");
            }
        }
    }
}
