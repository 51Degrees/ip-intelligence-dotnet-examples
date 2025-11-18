/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2023 51 Degrees Mobile Experts Limited, Davidson House,
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

using FiftyOne.Pipeline.Core.FlowElements;
using FiftyOne.Pipeline.Engines.Data;
using System;
using System.Collections;
using System.Linq;
/// <summary>
/// @example Cloud/GetAllProperties-Console/Program.cs
/// 
/// This example shows how to retrieve all available IP Intelligence properties from the 51Degrees Cloud service.
/// 
/// You will learn:
/// 
/// 1. How to create a Pipeline that uses 51Degrees Cloud IP Intelligence
/// 2. How to process an IP address and retrieve all available properties
/// 3. How to enumerate through all the properties returned by the service
/// 
/// This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-dotnet-examples/blob/main/Examples/Cloud/GetAllProperties-Console/Program.cs).
/// 
/// To run this example, you will eventually need to create a Resource Key, but for now you should use the GettingStarted-API example - just run it and point this example to its custom endpoint to simulate a custom hosted Cloud service. Resource Key is used as shorthand to store the particular set of properties you are interested in as well as any associated License Keys that entitle you to increased request limits and/or paid-for properties, but it is not yet available for IP Intelligence.
/// </summary>
namespace FiftyOne.IpIntelligence.Examples.Cloud.GetAllProperties
{
    public class Program
    {
        public class Example
        {
            private const string SomeIpAddress = "8.8.8.8";

            public void Run(string resourceKey, string cloudEndPoint = "")
            {
                var builder = new IpiPipelineBuilder()
                    // Tell it that we want to use cloud and pass our resource key.
                    .UseCloud(resourceKey)
                    .SetEndPoint(cloudEndPoint)
                    ;

                // If a cloud endpoint has been provided then set the
                // cloud pipeline endpoint. 
                if (string.IsNullOrWhiteSpace(cloudEndPoint) == false) 
                {
                    builder.SetEndPoint(cloudEndPoint);
                }

                // Create the pipeline
                using (var pipeline = builder.Build())
                {
                    // Output details for the IP address
                    AnalyseEvidence(SomeIpAddress, pipeline);
                }
            }

            static void AnalyseEvidence(string ipAddress, IPipeline pipeline)
            {
                // Create the FlowData instance.
                using (var data = pipeline.CreateFlowData())
                {
                    // Add a User-Agent as evidence.
                    data.AddEvidence(FiftyOne.Pipeline.Core.Constants.EVIDENCE_CLIENTIP_KEY, ipAddress);
                    // Process the supplied evidence.
                    data.Process();
                    // Get device data from the flow data.
                    var device = data.Get<IIpIntelligenceData>();
                    Console.WriteLine($"What property values are associated with " +
                        $"the IP '{ipAddress}'?");

                    // Iterate through device data results, displaying all values.
                    foreach (var property in device.AsDictionary()
                        .OrderBy(p => p.Key))
                    {
                        Console.WriteLine($"{property.Key} = {GetValueToOutput(property.Value)}");
                    }
                }
            }

            /// <summary>
            /// Convert the given value into a human-readable string representation 
            /// </summary>
            /// <param name="propertyValue">
            /// Property value object to be converted
            /// </param>
            /// <returns></returns>
            private static string GetValueToOutput(object propertyValue)
            {
                if (propertyValue == null)
                {
                    return "NULL";
                }

                var basePropetyType = propertyValue.GetType();
                var basePropertyValue = propertyValue;

                if (propertyValue is IAspectPropertyValue aspectPropertyValue)
                {
                    if (aspectPropertyValue.HasValue)
                    {
                        // Get the type and value parameters from the 
                        // AspectPropertyValue instance.
                        basePropetyType = basePropetyType.GenericTypeArguments[0];
                        basePropertyValue = aspectPropertyValue.Value;
                    }
                    else
                    {
                        // The property has no value so output the reason.
                        basePropetyType = typeof(string);
                        basePropertyValue = $"NO VALUE ({aspectPropertyValue.NoValueMessage})";
                    }
                }

                if (basePropetyType != typeof(string) &&
                    typeof(IEnumerable).IsAssignableFrom(basePropetyType))
                {
                    // Property is an IEnumerable (that is not a string)
                    // so return a comma-separated list of values.
                    var collection = basePropertyValue as IEnumerable;
                    var output = "";
                    foreach (var entry in collection)
                    {
                        if (output.Length > 0) { output += ","; }
                        output += entry.ToString();
                    }
                    return output;
                }
                else
                {
                    var str = basePropertyValue.ToString();
                    // Truncate any long strings to 200 characters
                    if (str.Length > 200)
                    {
                        str = str.Remove(200);
                        str += "...";
                    }
                    return str;
                }
            }
        }

        static void Main(string[] args)
        {
            // Obtain a resource key for free at https://configure.51degrees.com
            string resourceKey = "testResourceKey";

            if (resourceKey.StartsWith("!!"))
            {
                Console.WriteLine("You need to create a resource key at " +
                    "https://configure.51degrees.com and paste it into the code, " +
                    "replacing !!YOUR_RESOURCE_KEY!!.");
                Console.WriteLine("Make sure to include all the properties " +
                    "that you want to see displayed by this example.");

            }
            else
            {
                new Example().Run(resourceKey, "http://localhost:5225");
            }
#if (DEBUG)
            Console.WriteLine("Done. Press any key to exit.");
            Console.ReadKey();
#endif
        }
    }
}
