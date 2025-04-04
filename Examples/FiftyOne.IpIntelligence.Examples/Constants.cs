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

using System.Linq;

namespace FiftyOne.IpIntelligence.Examples
{
    public class Constants
    {
        /// <summary>
        /// The default name of the paid-for enterprise device detection data file.
        /// </summary>
        public const string ENTERPRISE_IPI_DATA_FILE_NAME = "Enterprise-IpiV41.ipi";

        /// <summary>
        /// The default name of the free 'lite' device detection data file.
        /// </summary>
        public const string LITE_IPI_DATA_FILE_NAME = "51Degrees-LiteV41.ipi";

        /// <summary>
        /// Name of the file to use for the test evidence.
        /// </summary>
        public const string YAML_EVIDENCE_FILE_NAME = "evidence.yml";

        /// <summary>
        /// Environment variable key for the license key file to use for the 
        /// tests.
        /// </summary>
        public const string LICENSE_KEY_ENV_VAR = "IPINTELLIGENCELICENSEKEY_DOTNET";

        /// <summary>
        /// Environment variable key for the data file to use for the tests.
        /// </summary>
        public const string IP_INTELLIGENCE_DATA_FILE_ENV_VAR = "IPINTELLIGENCEDATAFILE";

        /// <summary>
        /// Environment variable key for the evidence file to use for the tests.
        /// </summary>
        public const string EVIDENCE_FILE_ENV_VAR = "EVIDENCEFILE";

        /// <summary>
        /// Ports used for running web examples and their tests.
        /// </summary>
        public static readonly int[] LOCALHOST_HTTP_PORTS = new int[] { 5101 };
        public static readonly int[] LOCALHOST_HTTPS_PORTS = new int[] { 5001, 5002 };

        /// <summary>
        /// The URLs that the web server should listen on.
        /// </summary>
        public static string[] AllUrls =>
            Constants.LOCALHOST_HTTP_PORTS.Select(i =>
                $"http://localhost:{i}").Concat(
                Constants.LOCALHOST_HTTPS_PORTS.Select(i =>
                    $"https://localhost:{i}")).ToArray();
    }
}
