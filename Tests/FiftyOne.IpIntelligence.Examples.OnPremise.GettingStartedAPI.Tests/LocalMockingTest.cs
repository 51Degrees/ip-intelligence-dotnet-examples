/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
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

using System;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.AspNetCore.Builder;

namespace FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedAPI.Tests;

[TestClass]
public class LocalMockingTest
{
    private WebApplication? _cloudApp;

    [TestInitialize]
    public Task TestInitialize()
    {
        try
        {
            _cloudApp = new Program().BuildWebApp();
        }
        catch (Exception ex) when (IsDataFileVersionError(ex))
        {
            // The example resolves its data file from appsettings.json by name
            // (the copy in the ip-intelligence-data directory). A data/engine
            // version mismatch there is an environment problem, not a test
            // failure - skip cleanly with the underlying cause surfaced.
            Assert.Inconclusive(
                "The GettingStarted-API example could not build its pipeline " +
                "because its configured IP Intelligence data file is an " +
                "unsupported version (update the ip-intelligence-data file the " +
                $"example's appsettings.json points at). Details: {ex.Message}");
        }
        return _cloudApp!.StartAsync();
    }

    /// <summary>
    /// Returns true if the exception (or any inner exception) indicates the
    /// data file is missing or an unsupported version.
    /// </summary>
    private static bool IsDataFileVersionError(Exception ex)
    {
        for (var e = ex; e is not null; e = e.InnerException)
        {
            if (e.Message.Contains("unsupported version", StringComparison.OrdinalIgnoreCase) ||
                e.Message.Contains("Check you have the latest data", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }
        }
        return false;
    }

    [TestCleanup]
    public async Task TestCleanup()
    {
        if (_cloudApp is null)
        {
            return;
        }
        var source = new CancellationTokenSource(TimeSpan.FromSeconds(5));
        await _cloudApp.StopAsync(source.Token);
        await _cloudApp.DisposeAsync();
        _cloudApp = null;
    }
        
    [TestMethod]
    public async Task DoHttpRequest()
    {
        using var client = new HttpClient();
        client.BaseAddress = new Uri(
            _cloudApp!.Urls.First().Replace("0.0.0.0", "localhost"));
        client.DefaultRequestHeaders.Accept.Clear();
        client.DefaultRequestHeaders.Accept.Add(
            new MediaTypeWithQualityHeaderValue("application/json"));

        var response = await client.GetAsync("/accessibleproperties");
        response.EnsureSuccessStatusCode();

        var json = await response.Content.ReadAsStringAsync();
        Assert.AreNotEqual(string.Empty, json,
            "JSON response was empty.");
        Assert.DoesNotContain("device", json, 
            StringComparison.OrdinalIgnoreCase,
            "JSON response contains DD property. Config override failed?.");
    }
}
