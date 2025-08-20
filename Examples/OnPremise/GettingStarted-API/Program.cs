
using FiftyOne.IpIntelligence;
using FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements;
using FiftyOne.IpIntelligence.Examples;
using FiftyOne.IpIntelligence.Examples.OnPremise;
using FiftyOne.Pipeline.Core.Configuration;
using FiftyOne.Pipeline.Core.Data;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines.FlowElements;
using FiftyOne.Pipeline.JsonBuilder.Data;
using FiftyOne.Pipeline.Web.Services;
using FiftyOne.Pipeline.Web.Shared;
using Microsoft.AspNetCore.Http.HttpResults;
using Microsoft.Extensions.Configuration;
using Stubble.Core.Contexts;

namespace GettingStarted_API
{
    public class Program
    {
        public static void Main(string[] args)
        {
            var builder = WebApplication.CreateBuilder(args);
            builder.WebHost.UseUrls("http://localhost:5225");
            AppendConfigOverrides(builder.Configuration);

            // Add services to the container.
            builder.Services.AddAuthorization();


            // Add Swagger services

            // Add the hash engine builder to services so that the system can find the builder
            // when it needs to.
            builder.Services.AddSingleton<IpiOnPremiseEngineBuilder>();
            // Configure the services needed by IP Intelligence and create the 51Degrees Pipeline
            // instance that will be used to process requests.
            builder.Services.AddFiftyOne(builder.Configuration);

            var app = builder.Build();


            app.UseHttpsRedirection();
            app.UseAuthorization();

            var summaries = new[]
            {
                "Freezing", "Bracing", "Chilly", "Cool", "Mild", "Warm", "Balmy", "Hot", "Sweltering", "Scorching"
            };

            app.MapGet("/ipi/{ipAddress}", (string ipAddress, HttpContext httpContext, IPipeline pipeline) =>
            {
                var theIP = !string.IsNullOrWhiteSpace(ipAddress)
                    ? ipAddress
                    : httpContext.Connection.RemoteIpAddress?.ToString();
                if (theIP is not null)
                {
                    using var flowData = pipeline.CreateFlowData();
                    flowData.AddEvidence("query.client-ip", theIP);
                    flowData.Process();
                    if (flowData.Get<IJsonBuilderElementData>()?.Json is { } json)
                    {
                        return Results.Text(json, "application/json");
                    }
                }
                return Results.NotFound("No IP");
            })
            .WithName("IpIntelligence");

            app.MapGet("/ip-name/{ipAddress}", (string ipAddress, HttpContext httpContext, IPipeline pipeline) =>
            {
                var theIP = !string.IsNullOrWhiteSpace(ipAddress)
                    ? ipAddress
                    : httpContext.Connection.RemoteIpAddress?.ToString();
                if (theIP is not null)
                {
                    using var flowData = pipeline.CreateFlowData();
                    flowData.AddEvidence("query.client-ip", theIP);
                    flowData.Process();
                    var ipiData = flowData.Get<IIpIntelligenceData>();
                    if (ipiData?.RegisteredName is { } name && name?.HasValue == true)
                    {
                        return Results.Text(string.Join(", ",
                            name.Value.Select(
                                weighted => $"(Weighting={weighted.Weighting()}, Value={weighted.Value})")),
                            "text/plain");
                    }
                }
                return Results.NotFound("No IP");
            })
            .WithName("IpRangeName");

            app.MapPost("/json", (HttpContext context, IPipeline pipeline) =>
            {
                var aggregated = new Dictionary<string, object>(StringComparer.OrdinalIgnoreCase);

                // Query Parameters
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
                foreach (var kvp in context.Request.Form)
                {
                    aggregated["query." + kvp.Key] = kvp.Value.ToString();
                }

                using var flowData = pipeline.CreateFlowData();
                flowData.AddEvidence(aggregated);
                flowData.Process();

                if (flowData.Get<IJsonBuilderElementData>()?.Json is { } json)
                {
                    return Results.Text(json, "application/json");
                }
                return Results.NotFound("No results.");
            })
            .WithName("ProcessEvidence");

            app.MapGet("/accessibleproperties", (string resource, HttpContext httpContext, IPipeline pipeline) =>
            {
                Dictionary<string, ProductMetaData> products = new(
                    pipeline.FlowElements
                    .SelectMany(x => x is IAspectEngine eng ? [eng] : (IEnumerable<IAspectEngine>)[])
                    .Select(eng => new KeyValuePair<string, ProductMetaData>(eng.ElementDataKey, new ProductMetaData
                    {
                        Properties = eng!.Properties.Select(prop => new PropertyMetaData(prop)).ToList(),
                    })));

                return Results.Json(new 
                {
                    Products = products,
                });
            })
            .WithName("AccessibleProperties");

            app.MapGet("/evidencekeys", (HttpContext httpContext, IPipeline pipeline) =>
            {
                var keys = pipeline.FlowElements
                .Select(x => x.EvidenceKeyFilter as EvidenceKeyFilterWhitelist)
                .Where(x => x is not null)
                .SelectMany(x => x?.Whitelist.Keys ?? [])
                .Distinct()
                .ToList();
                return Results.Json(keys);
            })
            .WithName("EvidenceKeys");

            app.Run();
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

            // Get the index of the IP Intelligence engine element in the config file so that
            // we can create an override key for it.
            var ipiEngineOptions = options.GetElementConfig(nameof(IpiOnPremiseEngine));
            var ipiEngineIndex = options.Elements.IndexOf(ipiEngineOptions);
            var dataFileConfigKey = $"PipelineOptions:Elements:{ipiEngineIndex}" +
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
            else if (Path.IsPathRooted(dataFile) == false)
            {
                var newPath = FiftyOne.IpIntelligence.Examples.ExampleUtils.FindFile(dataFile);
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

            configurationManager.AddInMemoryCollection(overrides);
        }
    }
}
