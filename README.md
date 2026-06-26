# ![51Degrees](https://51degrees.com/img/logo.png?utm_source=github&utm_medium=readme&utm_campaign=ip-intelligence-dotnet-examples&utm_content=readme.md&utm_term=top "Data rewards the curious") IP Intelligence Engine Examples

This repository contains the examples for [[ip-intelligence-dotnet](https://github.com/51Degrees/ip-intelligence-dotnet/)].

Projects can be found in the `Examples/` folder. Currently, only on-premise examples are available. See below for a list of examples.

## Cloud (coming soon)

Cloud examples will be added once the Cloud service for IP Intelligence becomes available.

## ⚠️ Required files

See `ip-intelligence-data/README` ([local](./ip-intelligence-data/README) / [GitHub](https://github.com/51Degrees/ip-intelligence-data/)) on how to pull and/or generate necessary files.

### Mixed Examples Data Files

For the **Mixed examples** that combine Device Detection and IP Intelligence:

1. **IP Intelligence data file**: Place `51Degrees-EnterpriseIpiV41.ipi` (or `51Degrees-LiteV41.ipi`) in the `ip-intelligence-data/` directory at the repository root.

2. **Device Detection data file**: Place `51Degrees-EnterpriseV41.hash` (or `51Degrees-LiteV41.hash`) in the `ip-intelligence-data/` directory at the repository root.

Both data files should be obtained from the respective repositories:
- Device Detection data files: [device-detection-data](https://github.com/51Degrees/device-detection-data)
- IP Intelligence data files: [ip-intelligence-data](https://github.com/51Degrees/ip-intelligence-data)

## On-premise data file

The on-premise examples need an IP Intelligence data file. The examples locate
the file in the following order:

1. The "51DEGREES_IPI_PATH" environment variable, which can be set to an
   explicit path to the data file. The legacy "IPINTELLIGENCEDATAFILE"
   environment variable is also still supported, and is checked after
   "51DEGREES_IPI_PATH".
2. A search of the folder hierarchy, walking up from the working directory,
   for the expected data file name.
3. The free 'Lite' data file in its expected location, which is the
   ip-intelligence-data submodule of this repository. See the Required files
   section above on how to pull and/or generate the data files.

## 📦 NuGet Package

Examples currently depend on a pre-release version of the [FiftyOne.IpIntelligence](https://www.nuget.org/packages/FiftyOne.IpIntelligence) package.  

❗Make sure to enable pre-release packages when installing it:
* Using the .NET CLI:
```sh
dotnet add package FiftyOne.IpIntelligence --prerelease
```

* In Visual Studio: check the “Include prerelease” box in the NuGet Package Manager UI.

## On-Premise

| Example                          | Target               | Use case                                                                |
| -------------------------------- | -------------------- | ----------------------------------------------------------------------- |
| Framework-Web                    | .NET Framework 4.6.2 | ASP.NET Framework project.                                              |
| GettingStarted-Console           | .NET 8.0             | Simple console app.                                                     |
| GettingStarted-Web               | .NET 8.0             | ASP.NET Core project.                                                   |
| Metadata-Console                 | .NET 8.0             | Accessing data file's metadata (e.g. listing properties).               |
| OfflineProcessing-Console        | .NET 8.0             | Batch-processing of IP addresses from a YAML file.                      |
| Performance-Console              | .NET 8.0             | "Clock-time" benchmark for assessing detection speed.                   |
| UpdateDataFile-Console           | .NET 8.0             | Auto-update features: Daily / on Start-Up / Filesystem Watcher          |
| **Mixed/GettingStarted-Console** | **.NET 8.0**         | **Combined Device Detection and IP Intelligence console app.**          |
| **Mixed/GettingStarted-Web**     | **.NET 8.0**         | **Combined Device Detection and IP Intelligence ASP.NET Core project.** |


## Cloud

A resource key configured with the properties needed to run the cloud examples
can be created for free [here](https://configure.51degrees.com/1QWJwHxl?utm_source=github&utm_medium=readme&utm_campaign=ip-intelligence-dotnet-examples&utm_content=readme.md&utm_term=cloud). To
use the resource key in the examples it can be supplied as an environment
variable called "51DEGREES_RESOURCE_KEY". The legacy environment variable
names "RESOURCE_KEY" and "SUPER_RESOURCE_KEY" are still supported, with the
aligned "51DEGREES_RESOURCE_KEY" name checked first.

* In order to test cloud examples against a custom endpoint - you need to launch `OnPremise/Mixed/GettingStarted-API` example 
and keep it running while launching other examples.  Depending on the IDE you use this can be either done conveniently from the IDE, or 
just using `dotnet run` in the `OnPremise/Mixed/GettingStarted-API` directory from the command line.  

| Example                          | Target               | Use case                                                                  |
| -------------------------------- | -------------------- | ------------------------------------------------------------------------- |
| Framework-Web                    | .NET Framework 4.6.2 | ASP.NET Framework project.                                                |
| GettingStarted-Console           | .NET 8.0             | Simple console app.                                                       |
| GettingStarted-Web               | .NET 8.0             | ASP.NET Core project.                                                     |
| Metadata-Console                 | .NET 8.0             | Get the available properties and evidence keys information from the Cloud |
| GetAllProperties                 | .NET 8.0             | Get all the available properties for an IP address from the Cloud         |
| **Mixed/GettingStarted-Console** | **.NET 8.0**         | **Combined Device Detection and IP Intelligence console app.**            |
| **Mixed/GettingStarted-Web**     | **.NET 8.0**         | **Combined Device Detection and IP Intelligence ASP.NET Core project.**   |


### Mixed Examples

The **Mixed examples** demonstrate how to combine Device Detection and IP Intelligence engines within a single application:

- **Mixed/GettingStarted-Console**: A console application that processes both device detection (from User-Agent / User-Agent Client Hints) and IP intelligence (from IP address) in parallel using the 51Degrees Pipeline API.

- **Mixed/GettingStarted-Web**: An ASP.NET Core web application featuring:
  - Combined device detection and IP intelligence results in a two-column layout
  - IP address lookup functionality with custom IP input
  - Client-side evidence collection for enhanced device detection
  - Client hints support for improved browser detection
  - All device detection and IP intelligence properties displayed

## Testing

Automated tests live in the `Tests/` folder and run as part of CI:

| Test project                                            | Covers                                              |
| ------------------------------------------------------- | --------------------------------------------------- |
| `FiftyOne.IpIntelligence.Example.Tests.Cloud`           | The Cloud console examples and Cloud web examples   |
| `FiftyOne.IpIntelligence.Example.Tests.OnPremise`       | The on-premise console examples                     |
| `FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedAPI.Tests` | The on-premise `Mixed/GettingStarted-API` example |

Most runnable examples have an associated test. The exceptions, and why:

- **`Cloud/Framework-Web` and `OnPremise/Framework-Web`** are legacy **.NET
  Framework 4.6.2** ASP.NET (MVC / full-framework) applications. They are excluded
  from the cross-platform `net10.0` solution filter
  (`FiftyOne.IpIntelligence.Examples.Core.slnf`) and from the CI test run (which
  targets .NET on Linux, macOS and Windows), cannot be hosted in-process by the
  `net10.0` test harness, and are therefore verified by compilation only.
- **`OnPremise/GettingStarted-Web` and `OnPremise/Mixed/GettingStarted-Web`** are
  exercised manually rather than by an automated test. Unlike the Cloud web
  examples (which are tested in-process), these resolve their on-premise data file
  through a relative-path configuration override; hosting them in-process from the
  test project corrupts the pipeline element configuration. They run correctly
  when launched normally (`dotnet run`), which is how they should be verified
  until the in-process hosting issue is addressed.

### Test prerequisites

The example tests are integration tests and need the same inputs the examples do:

- **Cloud example tests** require a resource key in the `51DEGREES_RESOURCE_KEY`
  environment variable (see the [Cloud](#cloud) section). Without it these tests
  fail with a message explaining how to obtain one.
- **On-premise example tests** require the enterprise IP Intelligence data file
  (located automatically, or via `51DEGREES_IPI_PATH`) and the **Mixed** examples
  additionally require a device detection data file. When a required data file is
  absent, those tests report **inconclusive** rather than failing, so the gap is
  visible without being masked as a regression.
