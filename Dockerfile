FROM mcr.microsoft.com/dotnet/sdk:8.0-noble AS build

# Install build tools for native library compilation
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libatomic1 \
        wget \
        apt-transport-https \
        software-properties-common && \
    wget -q "https://packages.microsoft.com/config/ubuntu/$(lsb_release -rs)/packages-microsoft-prod.deb" && \
    dpkg -i packages-microsoft-prod.deb && \
    apt-get update && \
    apt-get install -y powershell && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/* packages-microsoft-prod.deb

# Copy only the Examples directory to avoid heavy ip-intelligence-data
COPY Examples /app
WORKDIR /app

# Restore dependencies for the web project (will restore dependencies)
RUN dotnet restore OnPremise/GettingStarted-Web/GettingStarted-Web.csproj

# Build and publish the web example with proper platform targeting
ARG CONFIG=Release
RUN dotnet publish OnPremise/GettingStarted-Web/GettingStarted-Web.csproj \
    -c "$CONFIG" \
    -p:Platform=x64 \
    -p:BuildType=Release \
    --arch x64 \
    -o /app/publish

FROM mcr.microsoft.com/dotnet/aspnet:8.0-noble

# Install runtime dependencies for native library support
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libatomic1 \
        libstdc++6 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

WORKDIR /app

# Copy the published application
COPY --from=build /app/publish .

# Copy the IP Intelligence data file directly to the final image
COPY ip-intelligence-data/51Degrees-EnterpriseIpiV41-AllProperties.ipi /app/data/51Degrees-EnterpriseIpiV41-AllProperties.ipi

EXPOSE 5225

CMD ["dotnet", "FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.dll"]