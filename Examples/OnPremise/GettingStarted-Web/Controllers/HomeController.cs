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

        public IActionResult Index()
        {
            // Log warnings if the data file is too old or the 'Lite' file is being used.
            if(_checkedDataFile == false)
            {
                ExampleUtils.CheckDataFile(_provider.GetFlowData().Pipeline, _logger);
                _checkedDataFile = true;
            }
            // Use the provider to get the flow data. This contains the results of
            // IP address lookup that has been performed by the pipeline.
            var model = new IndexModel(_provider.GetFlowData(), Response.Headers);
            return View(model);
        }

        [HttpPost]
        public IActionResult Index(string ipAddress)
        {
            // Log warnings if the data file is too old or the 'Lite' file is being used.
            if(_checkedDataFile == false)
            {
                ExampleUtils.CheckDataFile(_pipeline, _logger);
                _checkedDataFile = true;
            }

            IFlowData flowData;
            
            if (!string.IsNullOrWhiteSpace(ipAddress))
            {
                // Create new flow data with custom IP address
                flowData = _pipeline.CreateFlowData();
                flowData.AddEvidence("server.client-ip", ipAddress);
                flowData.Process();
            }
            else
            {
                // Use the default flow data from the current request
                flowData = _provider.GetFlowData();
            }

            var model = new IndexModel(flowData, Response.Headers);
            model.InputIpAddress = ipAddress;
            return View(model);
        }
    }
}
