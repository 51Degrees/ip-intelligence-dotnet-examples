<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="Framework_Web.Default" %>

<%@ Import Namespace="FiftyOne.IpIntelligence" %>
<%@ Import Namespace="FiftyOne.IpIntelligence.Examples" %>
<%@ Import Namespace="FiftyOne.IpIntelligence.Engine.OnPremise.FlowElements" %>
<%@ Import Namespace="FiftyOne.Pipeline.Web.Framework.Providers" %>

<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title>51Degrees Example</title>

    <link rel="stylesheet" href="Content/Site.css" />

    <%-- This JavaScript is dynamically generated based on the details of the detected device. 
        Including a reference to it allows us to collect client side evidence 
        (used for things such as Apple model detection) and access device detection 
        results in client side code.
        Note that there doesn't need to be a physical file. The 51Degrees Pipeline will 
        intercept the request and serve it automatically.
        --%>
    <script async src='51Degrees.core.js' type='text/javascript'></script>
</head>
<body>
    <h2>Web Integration Example</h2>

    <p>
        This example demonstrates the use of the Pipeline API to perform device detection within a
        simple ASP.NET Core web project.
    </p>

    <noscript>
        <div class="example-alert">
            WARNING: JavaScript is disabled in your browser.
        </div>
    </noscript>

    <div id="content">
        <h3>Detection results</h3>
        <p>
            The following values are determined by sever-side device detection
            on the first request:
        </p>
        <table>
            <tr>
                <th>Key</th>
                <th>Value</th>
            </tr>
            <% 
                // Put the flow data and device data instances into local variables so we don't
                // have to keep grabbing them.
                var flowData = ((PipelineCapabilities)Request.Browser).FlowData;
                var deviceData = flowData.Get<IIpIntelligenceData>();                
                // Get the engine that is used to make requests to the cloud service.
                var engine = flowData.Pipeline.GetElement<IpiOnPremiseEngine>(); 
                // Note that below we are using some helper methods from the
                // FiftyOne.DeviceDeteciton.Examples project (TryGetValue and GetHumanReadable)
                // These are mostly intended to handle scenarios where device detection does
                // not have an answer.
                // In a production scenario, you will probably want to handle these scenarios 
                // differently. Feel free to copy these helpers if they are useful though.
            %>
            <tr class="lightyellow"><td><b>Registered Name:</b></td><td> <%= deviceData.TryGetValue(d => d.RegisteredName.GetHumanReadable()) %></td></tr>
        </table>
        <br />
    
        <div id="evidence">
            <h3>Evidence used</h3>
            <p class="smaller">Evidence was <span class="lightgreen">used</span> / <span class="lightyellow">present</span> for detection</p>
            <table>
                <tr>
                    <th>Key</th>
                    <th>Value</th>
                </tr>
                <% foreach (var evidence in flowData.GetEvidence().AsDictionary()) { %>
                    <tr class="<%= engine.EvidenceKeyFilter.Include(evidence.Key) ? "lightgreen" : "lightyellow" %>">
                        <td><b><%= evidence.Key %></b></td>
                        <td><%= evidence.Value %></td>
                    </tr>
                <% } %>
            </table>
        </div>
        <br />

        <div id="response-headers">
            <h3>Response headers</h3>
            <table>
                <tr>
                    <th>Key</th>
                    <th>Value</th>
                </tr>
                <% foreach (var key in Response.Headers.AllKeys) { %>
                    <tr class="lightyellow">
                        <td><b><%= key %></b></td>
                        <td><%= string.Join(", ", Response.Headers.GetValues(key)) %></td>
                    </tr>
                <% } %>
            </table>
        </div>
        <br />
        <% if (engine.DataSourceTier == "Lite") { %>
            <div class="example-alert">
                WARNING: You are using the free 'Lite' data file. This does not include the client-side
                evidence capabilities of the paid-for data file, so you will not see any additional
                data below. Find out about the Enterprise data file on our
                <a href="https://51degrees.com/pricing">pricing page</a>.
            </div>
        <% } %>
    </div>   

</body>
</html>
