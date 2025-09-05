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
using FiftyOne.Pipeline.Engines.Data;
using Microsoft.AspNetCore.Http;
using System;
using System.Collections.Generic;
using System.Linq;

namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.Model
{
    public class IndexModel
    {
        // Geographic Location Properties
        public string Country { get; private set; }
        public string CountryCode { get; private set; }
        public string CountryCode3 { get; private set; }
        public string ContinentName { get; private set; }
        public string ContinentCode2 { get; private set; }
        public string Region { get; private set; }
        public string State { get; private set; }
        public string County { get; private set; }
        public string Town { get; private set; }
        public string Suburb { get; private set; }
        public string ZipCode { get; private set; }
        public string Latitude { get; private set; }
        public string Longitude { get; private set; }
        public string Areas { get; private set; }
        public string AccuracyRadiusMax { get; private set; }
        public string AccuracyRadiusMin { get; private set; }
        public string LocationConfidence { get; private set; }
        
        // Regional Information
        public string IsEu { get; private set; }
        public string CurrencyCode { get; private set; }
        public string DialCode { get; private set; }
        public string LanguageCode { get; private set; }
        
        // Time Zone
        public string TimeZoneIana { get; private set; }
        public string TimeZoneOffset { get; private set; }
        
        // Network Registration
        public string Name { get; private set; }
        public string RegisteredOwner { get; private set; }
        public string RegisteredCountry { get; private set; }
        public string IpRangeStart { get; private set; }
        public string IpRangeEnd { get; private set; }
        
        // ASN Information
        public string AsnName { get; private set; }
        public string AsnNumber { get; private set; }
        
        // Connection Type
        public string ConnectionType { get; private set; }
        public string IsBroadband { get; private set; }
        public string IsCellular { get; private set; }
        public string Mcc { get; private set; }
        
        // Security & Anonymity
        public string IsHosted { get; private set; }
        public string IsProxy { get; private set; }
        public string IsVPN { get; private set; }
        public string IsTor { get; private set; }
        public string IsPublicRouter { get; private set; }
        public string HumanProbability { get; private set; }
        public string InputIpAddress { get; set; }

        public IFlowData FlowData { get; private set; }

        public IpiOnPremiseEngine Engine { get; private set; }

        public IAspectEngineDataFile DataFile { get; private set; }

        public List<EvidenceModel> Evidence { get; private set; }

        public IHeaderDictionary ResponseHeaders { get; private set; }

        public IndexModel(IFlowData flowData, IHeaderDictionary responseHeaders)
        {
            FlowData = flowData;
            ResponseHeaders = responseHeaders;

            // Get the engine that performed the detection.
            Engine = FlowData.Pipeline.GetElement<IpiOnPremiseEngine>();
            // Get meta-data about the data file.
            DataFile = Engine.DataFiles[0];
            // Get the evidence that was used when performing IP Intelligence
            Evidence = FlowData.GetEvidence().AsDictionary()
                .Select(e => new EvidenceModel()
                {
                    Key = e.Key,
                    Value = e.Value.ToString(),
                    Used = Engine.EvidenceKeyFilter.Include(e.Key)
                })
                .ToList();

            // Get the results of IP Intelligence.
            var ipiData = FlowData.Get<IIpIntelligenceData>();
            // Use helper functions to get a human-readable string representation of various
            // property values. These helpers handle situations such as the property missing
            // due to using a lite data file or the property not having a value because
            // IP intelligence didn't find a match.
            
            // Geographic Location Properties
            Country = ipiData.TryGetValue(d => d.Country.GetHumanReadable());
            CountryCode = ipiData.TryGetValue(d => d.CountryCode.GetHumanReadable());
            CountryCode3 = ipiData.TryGetValue(d => d.CountryCode3.GetHumanReadable());
            ContinentName = ipiData.TryGetValue(d => d.ContinentName.GetHumanReadable());
            ContinentCode2 = ipiData.TryGetValue(d => d.ContinentCode2.GetHumanReadable());
            Region = ipiData.TryGetValue(d => d.Region.GetHumanReadable());
            State = ipiData.TryGetValue(d => d.State.GetHumanReadable());
            County = ipiData.TryGetValue(d => d.County.GetHumanReadable());
            Town = ipiData.TryGetValue(d => d.Town.GetHumanReadable());
            Suburb = ipiData.TryGetValue(d => d.Suburb.GetHumanReadable());
            ZipCode = ipiData.TryGetValue(d => d.ZipCode.GetHumanReadable());
            Latitude = ipiData.TryGetValue(d => d.Latitude.GetHumanReadable());
            Longitude = ipiData.TryGetValue(d => d.Longitude.GetHumanReadable());
            Areas = ipiData.TryGetValue(d => d.Areas.GetHumanReadable());
            AccuracyRadiusMax = ipiData.TryGetValue(d => d.AccuracyRadiusMax.GetHumanReadable());
            AccuracyRadiusMin = ipiData.TryGetValue(d => d.AccuracyRadiusMin.GetHumanReadable());
            LocationConfidence = ipiData.TryGetValue(d => d.LocationConfidence.GetHumanReadable());
            
            // Regional Information
            IsEu = FormatBoolProperty(ipiData.IsEu);
            CurrencyCode = ipiData.TryGetValue(d => d.CurrencyCode.GetHumanReadable());
            DialCode = FormatIntProperty(ipiData.DialCode);
            LanguageCode = ipiData.TryGetValue(d => d.LanguageCode.GetHumanReadable());
            
            // Time Zone
            TimeZoneIana = ipiData.TryGetValue(d => d.TimeZoneIana.GetHumanReadable());
            TimeZoneOffset = ipiData.TryGetValue(d => d.TimeZoneOffset.GetHumanReadable());
            
            // Network Registration
            Name = ipiData.TryGetValue(d => d.RegisteredName.GetHumanReadable());
            RegisteredOwner = ipiData.TryGetValue(d => d.RegisteredOwner.GetHumanReadable());
            RegisteredCountry = ipiData.TryGetValue(d => d.RegisteredCountry.GetHumanReadable());
            IpRangeStart = ipiData.TryGetValue(d => d.IpRangeStart.GetHumanReadable());
            IpRangeEnd = ipiData.TryGetValue(d => d.IpRangeEnd.GetHumanReadable());
            
            // ASN Information
            AsnName = ipiData.TryGetValue(d => d.AsnName.GetHumanReadable());
            AsnNumber = ipiData.TryGetValue(d => d.AsnNumber.GetHumanReadable());
            
            // Connection Type
            ConnectionType = ipiData.TryGetValue(d => d.ConnectionType.GetHumanReadable());
            IsBroadband = FormatBoolProperty(ipiData.IsBroadband);
            IsCellular = FormatBoolProperty(ipiData.IsCellular);
            Mcc = ipiData.TryGetValue(d => d.Mcc.GetHumanReadable());
            
            // Security & Anonymity
            IsHosted = FormatBoolProperty(ipiData.IsHosted);
            IsProxy = FormatBoolProperty(ipiData.IsProxy);
            IsVPN = FormatBoolProperty(ipiData.IsVPN);
            IsTor = FormatBoolProperty(ipiData.IsTor);
            IsPublicRouter = FormatBoolProperty(ipiData.IsPublicRouter);
            HumanProbability = FormatIntProperty(ipiData.HumanProbability);
        }

        private string FormatBoolProperty(IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>> property)
        {
            if (!property.HasValue)
            {
                return property.NoValueMessage;
            }
            else if (property.Value.Count == 1 && property.Value[0].Weighting() == 1.0f)
            {
                return property.Value[0].Value.ToString();
            }
            else
            {
                var values = property.Value.Select(x => 
                    Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString() : $"({x.Value} @ {x.Weighting():F4})");
                return string.Join(", ", values);
            }
        }

        private string FormatIntProperty(IAspectPropertyValue<IReadOnlyList<IWeightedValue<int>>> property)
        {
            if (!property.HasValue)
            {
                return property.NoValueMessage;
            }
            else if (property.Value.Count == 1 && property.Value[0].Weighting() == 1.0f)
            {
                return property.Value[0].Value.ToString();
            }
            else
            {
                var values = property.Value.Select(x => 
                    Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString() : $"({x.Value} @ {x.Weighting():F4})");
                return string.Join(", ", values);
            }
        }
    }
}
