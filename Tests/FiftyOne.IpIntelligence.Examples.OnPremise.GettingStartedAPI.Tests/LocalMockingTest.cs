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
        _cloudApp = new Program().BuildWebApp([]);
        return _cloudApp.StartAsync();
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
