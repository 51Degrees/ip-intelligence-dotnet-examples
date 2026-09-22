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
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
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
            return _cloudApp.StartAsync();
        }
        catch (Exception ex)
        {
            // The wrapper exception surfaced by CI ("The type initializer for
            // '...IpIntelligenceEngineModulePINVOKE' threw an exception.") hides
            // the real native-load failure. Dump the full exception chain and a
            // native-asset probe so the next run shows the actual cause
            // (e.g. DllNotFoundException vs BadImageFormatException).
            Console.WriteLine(DescribeNativeLoadFailure(ex));
            throw;
        }
    }

    private static string DescribeNativeLoadFailure(Exception ex)
    {
        var sb = new StringBuilder();
        sb.AppendLine("===== NATIVE LOAD DIAGNOSTICS BEGIN =====");
        sb.AppendLine($"ProcessArch : {RuntimeInformation.ProcessArchitecture}");
        sb.AppendLine($"OSArch      : {RuntimeInformation.OSArchitecture}");
        sb.AppendLine($"RID         : {RuntimeInformation.RuntimeIdentifier}");
        sb.AppendLine($"BaseDir     : {AppContext.BaseDirectory}");
        sb.AppendLine($"CurrentDir  : {Directory.GetCurrentDirectory()}");

        sb.AppendLine("----- Full exception chain -----");
        for (var current = ex; current is not null; current = current.InnerException)
        {
            sb.AppendLine($"[{current.GetType().FullName}] {current.Message}");
            if (current is DllNotFoundException || current is BadImageFormatException)
            {
                sb.AppendLine("  ^^ NATIVE LOAD FAILURE (see above type)");
            }
        }

        // The pipeline flattens the real cause into PipelineConfigurationException's
        // message text instead of chaining it, so reproduce the native load
        // directly here to capture the true leaf exception (Win32 reason).
        sb.AppendLine("----- Direct native load attempt -----");
        var nativePath = Path.Combine(
            AppContext.BaseDirectory, "runtimes", "win-x64", "native",
            "FiftyOne.IpIntelligence.Engine.OnPremise.Native.dll");
        try
        {
            var handle = NativeLibrary.Load(nativePath);
            sb.AppendLine($"  NativeLibrary.Load OK (handle=0x{handle:X}) for {nativePath}");
            NativeLibrary.Free(handle);
        }
        catch (Exception loadEx)
        {
            sb.AppendLine($"  NativeLibrary.Load FAILED for {nativePath}");
            sb.AppendLine($"  [{loadEx.GetType().FullName}] {loadEx.Message}");
        }

        sb.AppendLine("----- SWIG type initializer attempt -----");
        try
        {
            var pinvoke = Type.GetType(
                "FiftyOne.IpIntelligence.Engine.OnPremise.Interop.IpIntelligenceEngineModulePINVOKE, "
                + "FiftyOne.IpIntelligence.Engine.OnPremise");
            if (pinvoke is null)
            {
                sb.AppendLine("  Could not resolve PINVOKE type.");
            }
            else
            {
                RuntimeHelpers.RunClassConstructor(pinvoke.TypeHandle);
                sb.AppendLine("  Type initializer ran without throwing.");
            }
        }
        catch (Exception initEx)
        {
            for (var cur = initEx; cur is not null; cur = cur.InnerException)
            {
                sb.AppendLine($"  [{cur.GetType().FullName}] {cur.Message}");
                if (cur is DllNotFoundException || cur is BadImageFormatException)
                {
                    sb.AppendLine("    ^^ NATIVE LOAD FAILURE (see above type)");
                }
            }
        }

        sb.AppendLine("----- Native asset probe -----");
        foreach (var name in new[]
        {
            "FiftyOne.IpIntelligence.Engine.OnPremise.Native.dll",
            "FiftyOne.DeviceDetection.Hash.Engine.OnPremise.Native.dll",
        })
        {
            foreach (var probe in new[]
            {
                Path.Combine(AppContext.BaseDirectory, name),
                Path.Combine(AppContext.BaseDirectory, "runtimes", "win-x64", "native", name),
            })
            {
                sb.AppendLine($"  Exists={File.Exists(probe),-5} {probe}");
            }
        }
        sb.AppendLine("===== NATIVE LOAD DIAGNOSTICS END =====");
        return sb.ToString();
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
