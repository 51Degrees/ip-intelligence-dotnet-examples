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

using Microsoft.AspNetCore.Mvc;
using FiftyOne.Pipeline.Web.Services;
using FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.Model;
using Microsoft.Extensions.Logging;
using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Core.Data;
using System.Net;
using System.Linq;
using System.IO;

namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.Controllers
{
    public class HomeController : Controller
    {
        private static bool _checkedDataFile = false;

        private IFlowDataProvider _provider;
        private ILogger<HomeController> _logger;
        private IPipeline _pipeline;

        // The controller has a dependency on IFlowDataProvider. This is used to access the 
        // IFlowData that contains the IP Intelligence results for the current HTTP request.
        public HomeController(IFlowDataProvider provider, ILogger<HomeController> logger, IPipeline pipeline)
        {
            _provider = provider;
            _logger = logger;
            _pipeline = pipeline;
        }

        public IActionResult Index(string ipAddress = null)
        {
            // Log warnings if the data file is too old or the 'Lite' file is being used.
            if(_checkedDataFile == false)
            {
                ExampleUtils.CheckDataFile(_provider.GetFlowData().Pipeline, _logger);
                _checkedDataFile = true;
            }

            // Get the visitor's IP address from the request
            // Try X-Forwarded-For first (for Azure App Service/proxy scenarios), then RemoteIpAddress
            var visitorIp = GetClientIpAddress();
            
            // If no custom IP provided, use the visitor's IP
            var targetIp = !string.IsNullOrWhiteSpace(ipAddress) ? ipAddress : visitorIp;

            IFlowData flowData;
            
            if (!string.IsNullOrWhiteSpace(targetIp))
            {
                // Create new flow data with specified IP address (custom or visitor's)
                using (flowData = _pipeline.CreateFlowData())
                {
                    flowData.AddEvidence("server.client-ip", targetIp);
                    
                    // Add all HTTP headers as evidence
                    foreach (var header in HttpContext.Request.Headers)
                    {
                        flowData.AddEvidence($"header.{header.Key.ToLower()}", string.Join(",", header.Value));
                    }
                    
                    flowData.Process();
                    
                    var model = new IndexModel(flowData, Response.Headers);
                    model.InputIpAddress = targetIp;
                    model.BuildTimestamp = GetBuildTimestamp();
                    return View(model);
                }
            }
            else
            {
                // Fallback to default flow data if we can't determine IP
                using (flowData = _pipeline.CreateFlowData())
                {
                    // Add all HTTP headers as evidence
                    foreach (var header in HttpContext.Request.Headers)
                    {
                        flowData.AddEvidence($"header.{header.Key.ToLower()}", string.Join(",", header.Value));
                    }
                    
                    flowData.Process();
                    
                    var model = new IndexModel(flowData, Response.Headers);
                    model.InputIpAddress = visitorIp;
                    model.BuildTimestamp = GetBuildTimestamp();
                    return View(model);
                }
            }
        }

        private string GetClientIpAddress()
        {
            // After UseForwardedHeaders middleware, RemoteIpAddress should contain the real client IP
            // But we'll also check X-Forwarded-For as a backup
            var remoteIp = HttpContext.Connection.RemoteIpAddress?.ToString();
            
            // If RemoteIpAddress is still an internal IP, check X-Forwarded-For directly
            if (remoteIp == "169.254.130.1" || remoteIp == "::1" || remoteIp == "127.0.0.1" || 
                (remoteIp != null && (remoteIp.StartsWith("172.") || remoteIp.StartsWith("10.") || remoteIp.StartsWith("192.168."))))
            {
                var forwardedFor = HttpContext.Request.Headers["X-Forwarded-For"].FirstOrDefault();
                if (!string.IsNullOrEmpty(forwardedFor))
                {
                    // X-Forwarded-For can contain multiple IPs, take the first (original client)
                    var clientIp = forwardedFor.Split(',')[0].Trim();
                    return StripPortFromIp(clientIp);
                }
            }
            
            return StripPortFromIp(remoteIp);
        }

        private string StripPortFromIp(string ipAddress)
        {
            if (string.IsNullOrEmpty(ipAddress))
                return ipAddress;
                
            // Handle IPv4 with port (e.g., "192.168.1.1:8080")
            if (ipAddress.Contains(":") && !ipAddress.Contains("::"))
            {
                var lastColonIndex = ipAddress.LastIndexOf(':');
                // Check if what follows the colon is a port number
                if (lastColonIndex > 0 && int.TryParse(ipAddress.Substring(lastColonIndex + 1), out _))
                {
                    return ipAddress.Substring(0, lastColonIndex);
                }
            }
            
            return ipAddress;
        }
        
        private string GetBuildTimestamp()
        {
            var timestampPath = Path.Combine(Directory.GetCurrentDirectory(), "wwwroot", "build-timestamp.txt");
            if (System.IO.File.Exists(timestampPath))
            {
                return System.IO.File.ReadAllText(timestampPath).Trim();
            }
            return "Unknown";
        }
    }
}
