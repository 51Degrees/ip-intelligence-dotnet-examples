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
using FiftyOne.Pipeline.Core.FlowElements;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace FiftyOne.IpIntelligence.Examples
{
    public static class ExampleUtils
    {
        /// <summary>
        /// The default environment variable key used to get the resource key 
        /// to use when running cloud examples.
        /// </summary>
        public const string CLOUD_RESOURCE_KEY_ENV_VAR = "SUPER_RESOURCE_KEY";

        /// <summary>
        /// The default environment variable key used to get the end point URL
        /// to use when running cloud examples. Can be used to override the
        /// appsettings.json configuration for testing custom end points.
        /// </summary>
        public const string CLOUD_END_POINT_ENV_VAR = "51D_CLOUD_ENDPOINT";

        /// <summary>
        /// Timeout used when searching for files.
        /// </summary>
        private const int FindFileTimeoutMs = 10000;

        /// <summary>
        /// If data file is older than this number of days then a warning will be displayed.
        /// </summary>
        public const int DataFileAgeWarning = 30;

        private const string DATA_OPTION = "--data-file";
        private const string DATA_OPTION_SHORT = "-d";
        private const string ADDRESSES_OPTION = "--ip-addresses-file";
        private const string ADDRESSES_OPTION_SHORT = "-a";
        private const string JSON_OPTION = "--json-output";
        private const string JSON_OPTION_SHORT = "-j";
        private const string HELP_OPTION = "--51help";
        private const string HELP_OPTION_SHORT = "-51h";

        private static string OptionMessage(string message, string option, string shortOption)
        {
            var padding = 32 - option.Length - shortOption.Length;
            return $"  {option}, {shortOption}{new string(' ', padding)}: {message}";
        }

        /// <summary>
        /// Print the available options to the output.
        /// </summary>
        private static void PrintHelp()
        {
            Console.WriteLine("Available options are:");
            Console.WriteLine(OptionMessage("Path to a 51Degrees IPI data file", DATA_OPTION, DATA_OPTION_SHORT));
            Console.WriteLine(OptionMessage("Path to a IP Addresses YAML file", ADDRESSES_OPTION, ADDRESSES_OPTION_SHORT));
            Console.WriteLine(OptionMessage("Path to a file to output JSON format results to", JSON_OPTION, JSON_OPTION_SHORT));
            Console.WriteLine(OptionMessage("Print this help", HELP_OPTION, HELP_OPTION_SHORT));
        }


        /// <summary>
        /// Parse the command line arguments passed to the example to get the common
        /// options.
        /// </summary>
        /// <param name="args">
        /// Command line options.
        /// </param>
        /// <returns>
        /// Parsed options, or null if help output is requested.
        /// </returns>
        /// <exception cref="ArgumentException">
        /// If an invalid argument is passed.
        /// </exception>
        public static ExampleOptions ParseOptions(string[] args)
        {
            var options = new ExampleOptions();
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i].StartsWith("-"))
                {
                    switch (args[i])
                    {
                        case DATA_OPTION:
                        case DATA_OPTION_SHORT:
                            // Set data file path
                            options.DataFilePath = args[i + 1];
                            break;
                        case ADDRESSES_OPTION:
                        case ADDRESSES_OPTION_SHORT:
                            // Set data file path
                            options.EvidenceFile = args[i + 1];
                            break;
                        case JSON_OPTION:
                        case JSON_OPTION_SHORT:
                            // Set data file path
                            options.JsonOutput = args[i + 1];
                            break;
                        case HELP_OPTION:
                        case HELP_OPTION_SHORT:
                            // Set data file path
                            PrintHelp();
                            return null;
                        default:
                            throw new ArgumentException(
                                $"The option '{args[i]}' is not recognized. " +
                                $"Use {HELP_OPTION} ({HELP_OPTION_SHORT}) to list options");
                    }
                }
                else
                {
                    // Do nothing, this is a value.
                }
            }
            return options;
        }

        /// <summary>
        /// Uses a background task to search for the specified filename within the working 
        /// directory.
        /// If the file cannot be found, the algorithm will move to the parent directory and 
        /// repeat the process.
        /// This continues until the file is found or a timeout is triggered.
        /// </summary>
        /// <param name="filename"></param>
        /// <param name="dir">
        /// The directory to start looking from. If not provided the current directory is used.
        /// </param>
        /// <returns></returns>
        public static string FindFile(
            string filename,
            DirectoryInfo dir = null)
        {
            var cancel = new CancellationTokenSource();
            // Start the file system search as a separate task.
            var searchTask = Task.Run(() => FindFile(filename, dir, cancel.Token));
            // Wait for either the search or a timeout task to complete.
            Task.WaitAny(searchTask, Task.Delay(FindFileTimeoutMs));
            cancel.Cancel();
            // If search has not got a result then return null.
            return searchTask.IsCompleted ? searchTask.Result : null;
        }

        private static string FindFile(
            string filename,
            DirectoryInfo dir,
            CancellationToken cancel)
        {
            if (dir == null)
            {
                dir = new DirectoryInfo(Directory.GetCurrentDirectory());
            }
            string result = null;

            try
            {
                var files = dir.GetFiles(filename, SearchOption.AllDirectories);
                if (files.Length == 0 &&
                    dir.Parent != null &&
                    cancel.IsCancellationRequested == false)
                {
                    result = FindFile(filename, dir.Parent, cancel);
                }
                else if (files.Length > 0)
                {
                    result = files[0].FullName;
                }
            }
            // No matter what goes wrong here, we just want to indicate that we
            // couldn't find the file by returning null.
            catch { result = null; }

            return result;
        }

        /// <summary>
        /// Get information about the specified data file
        /// </summary>
        /// <param name="dataFile"></param>
        /// <param name="engineBuilder"></param>
        public static DataFileInfo GetDataFileInfo(string dataFile, 
            IpiOnPremiseEngineBuilder engineBuilder)
        {
            DataFileInfo result = new DataFileInfo();

            using (var engine = engineBuilder
                .Build(dataFile, false))
            {
                result = GetDataFileInfo(engine);
            }

            return result;
        }

        /// <summary>
        /// Get information about the data file used by the specified engine
        /// </summary>
        /// <param name="engine"></param>
        public static DataFileInfo GetDataFileInfo(IpiOnPremiseEngine engine)
        {
            DataFileInfo result = new DataFileInfo();
            result.PublishDate = engine.DataFiles[0].DataPublishedDateTime;
            result.Tier = engine.DataSourceTier;
            result.Filepath = engine.DataFiles[0].DataFilePath;
            return result;
        }

        /// <summary>
        /// Display information about the data file and log warnings if specific requirements
        /// are not met.
        /// </summary>
        /// <param name="pipeline"></param>
        /// <param name="logger"></param>
        public static void CheckDataFile(IPipeline pipeline, ILogger logger)
        {
            // Get the 'engine' element within the pipeline that performs IP Intelligence.
            // We can use this to get details about the data file as well as meta-data describing
            // things such as the available properties.
            var engine = pipeline.GetElement<IpiOnPremiseEngine>();
            CheckDataFile(engine, logger);
        }

        /// <summary>
        /// Display information about the data file and log warnings if specific requirements
        /// are not met.
        /// </summary>
        /// <param name="engine"></param>
        /// <param name="logger"></param>
        public static void CheckDataFile(IpiOnPremiseEngine engine, ILogger logger)
        {
            if (engine != null)
            {
                var info = GetDataFileInfo(engine);
                LogDataFileInfo(info, logger);
                LogDataFileStandardWarnings(info, logger);
            }
        }

        /// <summary>
        /// Display information about the data file and log warnings if specific requirements
        /// are not met.
        /// </summary>
        /// <param name="engine"></param>
        /// <param name="logger"></param>
        public static void LogDataFileInfo(DataFileInfo info, ILogger logger)
        {
            if (info != null)
            {
                logger.LogInformation($"Using a '{info.Tier}' data file created " +
                    $"'{info.PublishDate}' from location '{info.Filepath}'");
            }
        }

        /// <summary>
        /// Display information about the data file and log warnings if specific requirements
        /// are not met.
        /// </summary>
        /// <param name="engine"></param>
        /// <param name="logger"></param>
        public static void LogDataFileStandardWarnings(DataFileInfo info, ILogger logger)
        {
            if (info != null)
            {
                if (DateTime.UtcNow > info.PublishDate.AddDays(DataFileAgeWarning))
                {
                    logger.LogWarning($"This example is using a data file that is more than " +
                        $"{DataFileAgeWarning} days old. A more recent data file may be needed " +
                        $"for a more precise detection. The latest lite " +
                        $"data file is available from the ip-intelligence-data repository on " +
                        $"GitHub https://github.com/51Degrees/ip-intelligence-data. Find out " +
                        $"about the Enterprise data file, which includes automatic daily " +
                        $"updates, on our pricing page: https://51degrees.com/pricing");
                }
                if (info.Tier == "Lite")
                {
                    logger.LogWarning($"This example is using the 'Lite' data file. This is " +
                        $"used for illustration, and has limited accuracy and capabilities. " +
                        $"Find out about the Enterprise data file on our pricing page: " +
                        $"https://51degrees.com/pricing");
                }
            }
        }

        /// <summary>
        /// Checks if the supplied 51Degrees resource key / license key is invalid.
        /// Note that this cannot determine if the key is definitely valid, just whether it is
        /// definitely invalid.
        /// </summary>
        /// <param name="key">
        /// The key to check.
        /// </param>
        /// <returns></returns>
        public static bool IsInvalidKey(string key)
        {
            try
            {
                if (key == null) 
                {
                    return true;
                }

                byte[] data = Convert.FromBase64String(key);
                string decodedString = Encoding.UTF8.GetString(data);

                return key.Trim().Length < 19 ||
                    decodedString.Length < 14;
            }
            catch (Exception)
            {
                return true;
            }
        }

        /// <summary>
        /// This collection contains the various input values that will be passed to the
        /// IP Intelligence algorithm when running examples
        /// </summary>
        public static readonly List<Dictionary<string, object>>
            EvidenceValues = new List<Dictionary<string, object>>()
        {
            new Dictionary<string, object>()
            {
                { "query.client-ip", "116.154.188.222" }
            },
            new Dictionary<string, object>()
            {
                { "query.client-ip", "45.236.48.61" }
            },
            new Dictionary<string, object>()
            {
                { "query.client-ip", "2001:0db8:085a:0000:0000:8a2e:0370:7334" }
            },
        };

        /// <summary>
        /// Checks if an environment variable exists with the key name provided
        /// and then runs the action with the value, or an empty string if the
        /// key does not exist.
        /// </summary>
        /// <param name="envVarName"></param>
        /// <param name="setValue"></param>
        public static void GetKeyFromEnv(
            string envVarName,
            Action<string> setValue)
        {
            var superKey = Environment.GetEnvironmentVariable(envVarName);
            if (string.IsNullOrWhiteSpace(superKey) == false)
            {
                setValue(superKey);
            }
            else
            {
                setValue(string.Empty);
            }
        }
    }
}
