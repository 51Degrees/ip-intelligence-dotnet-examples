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
using FiftyOne.Pipeline.Engines.FiftyOne.Data;
using Microsoft.AspNetCore.Http;
using System;
using System.Collections.Generic;
using System.Linq;

namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.Model
{
    public class PropertyData
    {
        public string Name { get; set; }
        public string Value { get; set; }
        public string Category { get; set; }
        public string Component { get; set; }
        public string Description { get; set; }
    }

    public class IndexModel
    {
        // Properties for backward compatibility and special cases
        public string Latitude { get; private set; }
        public string Longitude { get; private set; }
        public string Areas { get; private set; }
        public string InputIpAddress { get; set; }
        
        // Dynamic properties list
        public List<PropertyData> Properties { get; private set; }

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
            
            // Initialize the properties list
            Properties = new List<PropertyData>();
            
            // Get all properties dynamically from metadata and populate them the same way as original code
            foreach (var component in Engine.Components)
            {
                foreach (var property in component.Properties)
                {
                    // Skip properties that are not available in the current data file
                    if (!property.Available)
                        continue;
                    
                    string propertyValue = GetPropertyValue(ipiData, property);
                    
                    // Debug: If we still see wrapper types in the result, catch them here
                    if (propertyValue != null && propertyValue.Contains("WeightedUTF8StringListSwigWrapper"))
                    {
                        propertyValue = $"DEBUG MAIN - Property {property.Name} returned wrapper: {propertyValue}";
                    }
                    
                    // Add to the properties list
                    Properties.Add(new PropertyData
                    {
                        Name = property.Name,
                        Value = propertyValue,
                        Category = property.Category ?? "Other",
                        Component = component.Name,
                        Description = property.Description
                    });
                    
                    // Keep special properties for map functionality
                    if (property.Name == "Latitude")
                        Latitude = propertyValue;
                    else if (property.Name == "Longitude")
                        Longitude = propertyValue;
                    else if (property.Name == "Areas")
                        Areas = propertyValue;
                }
            }
            
            // Sort properties by component and then by display order or name
            Properties = Properties
                .OrderBy(p => p.Component)
                .ThenBy(p => p.Category)
                .ThenBy(p => p.Name)
                .ToList();
        }
        
        private string GetPropertyValue(IIpIntelligenceData ipiData, IFiftyOneAspectPropertyMetaData property)
        {
            try
            {
                // Switch based on the property type from metadata
                if (property.Type == typeof(string))
                {
                    return GetStringPropertyValue(ipiData, property.Name);
                }
                else if (property.Type == typeof(bool))
                {
                    return GetWeightedPropertyValue(ipiData, property.Name, property.Type);
                }
                else if (property.Type == typeof(int))
                {
                    return GetWeightedPropertyValue(ipiData, property.Name, property.Type);
                }
                else if (property.Type == typeof(double) || property.Type == typeof(float))
                {
                    return GetWeightedPropertyValue(ipiData, property.Name, property.Type);
                }
                else if (property.Type == typeof(System.Net.IPAddress))
                {
                    return GetWeightedPropertyValue(ipiData, property.Name, property.Type);
                }
                else
                {
                    return GetStringPropertyValue(ipiData, property.Name);
                }
            }
            catch (Exception ex)
            {
                return $"Error: {ex.Message}";
            }
        }
        
        private string GetStringPropertyValue(IIpIntelligenceData ipiData, string propertyName)
        {
            try
            {
                // Generic approach - get the property and handle it appropriately
                var propInfo = ipiData.GetType().GetProperty(propertyName);
                if (propInfo != null)
                {
                    var value = propInfo.GetValue(ipiData);
                    if (value != null)
                    {
                        var valueType = value.GetType();
                        var fullTypeName = valueType.FullName;
                        var typeName = valueType.Name;
                        
                        // Check if this is an AspectPropertyValue containing a SWIG wrapper
                        if (typeName.StartsWith("AspectPropertyValue") && value.ToString().Contains("SwigWrapper"))
                        {
                            return FormatWeightedProperty(value);
                        }
                        
                        // Check if this is a wrapper type that needs special handling
                        if (fullTypeName.Contains("SwigWrapper") || fullTypeName.Contains("Wrapper"))
                        {
                            return FormatWeightedProperty(value);
                        }
                        
                        // Try GetHumanReadable first for normal string properties
                        var getHumanReadableMethod = valueType.GetMethod("GetHumanReadable");
                        if (getHumanReadableMethod != null)
                        {
                            return (string)getHumanReadableMethod.Invoke(value, null) ?? "Unknown";
                        }
                        
                        return value.ToString();
                    }
                }
                return "Unknown";
            }
            catch
            {
                return "Unknown";
            }
        }
        
        private string GetWeightedPropertyValue(IIpIntelligenceData ipiData, string propertyName, Type propertyType)
        {
            try
            {
                var propInfo = ipiData.GetType().GetProperty(propertyName);
                if (propInfo != null)
                {
                    var value = propInfo.GetValue(ipiData);
                    if (value != null)
                    {
                        var valueType = value.GetType();
                        
                        // Use the property type from metadata to determine the formatter
                        if (propertyType == typeof(bool))
                        {
                            return FormatBoolProperty(value as IAspectPropertyValue<IReadOnlyList<IWeightedValue<bool>>>);
                        }
                        else if (propertyType == typeof(int))
                        {
                            return FormatIntProperty(value as IAspectPropertyValue<IReadOnlyList<IWeightedValue<int>>>);
                        }
                        else if (propertyType == typeof(System.Net.IPAddress))
                        {
                            return FormatIPAddressProperty(value as IAspectPropertyValue<IReadOnlyList<IWeightedValue<System.Net.IPAddress>>>);
                        }
                        else if (propertyType == typeof(float))
                        {
                            return FormatFloatProperty(value as IAspectPropertyValue<IReadOnlyList<IWeightedValue<float>>>);
                        }
                        else
                        {
                            // Fallback to generic weighted property formatting
                            return FormatWeightedProperty(value);
                        }
                    }
                }
                return "Unknown";
            }
            catch 
            {
                return "Unknown";
            }
        }
        
        private string FormatWeightedProperty(object value)
        {
            // Handle different types of wrapper values
            var valueType = value.GetType();
            var typeName = valueType.Name;
            var fullTypeName = valueType.FullName;
            
            // Handle AspectPropertyValue containing string weighted values ONLY if it contains SWIG wrapper
            if (typeName.StartsWith("AspectPropertyValue") && 
                fullTypeName.Contains("IReadOnlyList") && 
                fullTypeName.Contains("IWeightedValue") && 
                fullTypeName.Contains("String") &&
                value.ToString().Contains("SwigWrapper"))
            {
                // This is AspectPropertyValue<IReadOnlyList<IWeightedValue<string>>>
                try
                {
                    var hasValueProp = valueType.GetProperty("HasValue");
                    var hasValue = hasValueProp != null ? (bool)hasValueProp.GetValue(value) : true;
                    
                    if (!hasValue)
                    {
                        var noValueProp = valueType.GetProperty("NoValueMessage");
                        var noValueMsg = noValueProp?.GetValue(value)?.ToString() ?? "No value available";
                        return $"Unknown ({noValueMsg})";
                    }
                    
                    var valueProp = valueType.GetProperty("Value");
                    if (valueProp != null)
                    {
                        var innerValue = valueProp.GetValue(value);
                        if (innerValue == null)
                        {
                            return "Unknown (No inner value)";
                        }
                        
                        if (innerValue is System.Collections.IEnumerable enumerable)
                        {
                            var results = new List<string>();
                            int itemCount = 0;
                            foreach (var wv in enumerable)
                            {
                                itemCount++;
                                if (wv == null)
                                {
                                    results.Add("null");
                                    continue;
                                }
                                
                                var wvType = wv.GetType();
                                var weightingProperty = wvType.GetProperty("RawWeighting");
                                var valueProperty = wvType.GetProperty("Value");
                                
                                if (weightingProperty != null && valueProperty != null)
                                {
                                    try
                                    {
                                        var rawWeight = (ushort)weightingProperty.GetValue(wv);
                                        var weight = rawWeight / (float)ushort.MaxValue;
                                        var val = valueProperty.GetValue(wv);
                                        
                                        // If weight is 1.0, just show the value, otherwise show weight
                                        if (Math.Abs(weight - 1.0f) < 0.0001f)
                                        {
                                            results.Add(val?.ToString() ?? "null");
                                        }
                                        else
                                        {
                                            results.Add($"({val} @ {weight:F4})");
                                        }
                                    }
                                    catch (Exception itemEx)
                                    {
                                        results.Add($"Error: {itemEx.Message}");
                                    }
                                }
                                else
                                {
                                    results.Add("Unknown (Missing properties)");
                                }
                            }
                            
                            return results.Count > 0 ? 
                                string.Join(", ", results) : 
                                "No values";
                        }
                        else
                        {
                            return $"Unknown (Not enumerable)";
                        }
                    }
                    else
                    {
                        return "Unknown (No Value property)";
                    }
                }
                catch (Exception ex)
                {
                    return $"Error extracting weighted string: {ex.Message}";
                }
            }
            
            // ALWAYS show debug info for WeightedUTF8StringListSwigWrapper to see what's available
            if (value.ToString().Contains("WeightedUTF8StringListSwigWrapper"))
            {
                try
                {
                    var allMethods = valueType.GetMethods().Where(m => m.IsPublic && !m.IsSpecialName).Select(m => m.Name).Distinct().ToArray();
                    var allProperties = valueType.GetProperties().Where(p => p.CanRead).Select(p => p.Name).ToArray();
                    
                    // Return debug info to see what's available
                    return $"DEBUG FORMAT - Methods: [{string.Join(", ", allMethods)}] Properties: [{string.Join(", ", allProperties)}] Type: {fullTypeName}";
                }
                catch (Exception ex)
                {
                    return $"DEBUG FORMAT - Failed to inspect {fullTypeName}: {ex.Message}";
                }
            }
            
            // If it's any other wrapper type, show basic debug info
            if (fullTypeName.Contains("Wrapper") || fullTypeName.Contains("Swig"))
            {
                return $"DEBUG FORMAT OTHER - Type: {fullTypeName}, Name: {typeName}";
            }
            
            // Check if this is an AspectPropertyValue type
            if (valueType.IsGenericType && valueType.GetGenericTypeDefinition().Name.StartsWith("AspectPropertyValue"))
            {
                // Check if it has a value
                var hasValueProp = valueType.GetProperty("HasValue");
                if (hasValueProp != null && !(bool)hasValueProp.GetValue(value))
                {
                    var noValueProp = valueType.GetProperty("NoValueMessage");
                    if (noValueProp != null)
                    {
                        return noValueProp.GetValue(value)?.ToString() ?? "Unknown";
                    }
                    return "Unknown";
                }
                
                // Get the inner value
                var valueProp = valueType.GetProperty("Value");
                if (valueProp != null)
                {
                    var innerValue = valueProp.GetValue(value);
                    if (innerValue != null)
                    {
                        // Handle IReadOnlyList<IWeightedValue<T>>
                        if (typeof(System.Collections.IEnumerable).IsAssignableFrom(innerValue.GetType()) && 
                            innerValue.GetType().IsGenericType)
                        {
                            var weightedValues = innerValue as System.Collections.IEnumerable;
                            var results = new List<string>();
                            
                            foreach (var wv in weightedValues)
                            {
                                var wvType = wv.GetType();
                                var weightingMethod = wvType.GetMethod("Weighting");
                                var valueProperty = wvType.GetProperty("Value");
                                
                                if (weightingMethod != null && valueProperty != null)
                                {
                                    var weight = (float)weightingMethod.Invoke(wv, null);
                                    var val = valueProperty.GetValue(wv);
                                    
                                    // If weight is 1.0, just show the value, otherwise show weight
                                    if (Math.Abs(weight - 1.0f) < 0.0001f)
                                    {
                                        results.Add(val?.ToString() ?? "");
                                    }
                                    else
                                    {
                                        results.Add($"({val} @ {weight:F4})");
                                    }
                                }
                            }
                            
                            return results.Count > 0 ? string.Join(", ", results) : "Unknown";
                        }
                        else
                        {
                            // Simple value
                            return innerValue.ToString();
                        }
                    }
                }
            }
            
            return value?.ToString() ?? "Unknown";
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

        private string FormatIPAddressProperty(IAspectPropertyValue<IReadOnlyList<IWeightedValue<System.Net.IPAddress>>> property)
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

        private string FormatFloatProperty(IAspectPropertyValue<IReadOnlyList<IWeightedValue<float>>> property)
        {
            if (!property.HasValue)
            {
                return property.NoValueMessage;
            }
            else if (property.Value.Count == 1 && property.Value[0].Weighting() == 1.0f)
            {
                return property.Value[0].Value.ToString("F6");
            }
            else
            {
                var values = property.Value.Select(x => 
                    Math.Abs(x.Weighting() - 1.0f) < 0.0001f ? x.Value.ToString("F6") : $"({x.Value:F6} @ {x.Weighting():F4})");
                return string.Join(", ", values);
            }
        }
    }
}
