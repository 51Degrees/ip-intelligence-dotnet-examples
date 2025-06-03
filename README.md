# ![51Degrees](https://51degrees.com/img/logo.png?utm_source=github&utm_medium=repository&utm_content=readme_main&utm_campaign=dotnet-open-source "Data rewards the curious") IP Intelligence Engine Examples

This repository contains the examples for [[ip-intelligence-dotnet](https://github.com/51Degrees/ip-intelligence-dotnet/)].

Projects can be found in the `Examples/` folder. Currently, only on-premise examples are available. See below for a list of examples.

## Cloud (coming soon)

Cloud examples will be added once the cloud service for IP Intelligence becomes available.

## ⚠️ Required files

See `ip-intelligence-data/README` ([local](./ip-intelligence-data/README) / [GitHub](https://github.com/51Degrees/ip-intelligence-data/)) on how to pull and/or generate necessary files.

## 📦 NuGet Package

Examples currently depend on a pre-release version of the [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence) package.  

❗Make sure to enable pre-release packages when installing it:
* Using the .NET CLI:
```sh
dotnet add package FiftyOne.IpIntelligence --prerelease
```

* In Visual Studio: check the “Include prerelease” box in the NuGet Package Manager UI.

## On-Premise

|Example|Target|Use case|
|---|---|---|
|Framework-Web|.NET Framework 4.6.2|ASP.NET Framework project.|
|GettingStarted-Console|.NET 8.0|Simple console app.|
|GettingStarted-Web|.NET 8.0|ASP.NET Core project.|
|Metadata-Console|.NET 8.0|Accessing data file's metadata (e.g. listing properties).|
|OfflineProcessing-Console|.NET 8.0|Batch-processing of IP addresses from a YAML file.|
|Performance-Console|.NET 8.0|"Clock-time" benchmark for assessing detection speed.|
|UpdateDataFile-Console|.NET 8.0|Auto-update features: Daily / on Start-Up / Filesystem Watcher|
