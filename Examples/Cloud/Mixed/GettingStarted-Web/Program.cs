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

using System.Threading;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;

namespace FiftyOne.IpIntelligence.Examples.Mixed.Cloud.GettingStartedWeb
{
    public class Program
    {
        public static void Main(string[] args)
        {
            // Start the server and then wait for the task to finish.
            Run(args).Wait();
        }

        /// <summary>
        /// Used by unit tests to run the example in an almost identical manner
        /// to a developer using the example. Returns the task that the web 
        /// server is running in so that the test can trigger the cancellation
        /// token and then wait for the server to shutdown before finishing.
        /// </summary>
        /// <param name="args"></param>
        /// <param name="stopToken"></param>
        /// <returns></returns>
        public static Task Run(
            string[] args,
            CancellationToken stopToken = default)
        {
            return CreateHostBuilder(args).Build().RunAsync(
                stopToken);
        }

        public static IHostBuilder CreateHostBuilder(string[] args) =>
            Host.CreateDefaultBuilder(args)
                .ConfigureWebHostDefaults(webBuilder =>
                {
                    webBuilder
                        .ConfigureAppConfiguration(config =>
                            config.AddJsonFile("appsettings.json"))
                        .UseStartup<Startup>()
                        .UseUrls(Constants.AllUrls);
                });
    }
}