FROM mcr.microsoft.com/dotnet/sdk:8.0-noble AS build

# Install build tools for native library compilation
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libatomic1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

COPY Examples /app
WORKDIR /app

# Remove Device Detection overrides and use docker settings
RUN sed -i '/AddOverrides_DD.*;$/d' OnPremise/Mixed/GettingStarted-API/Program.cs && \
    cp OnPremise/Mixed/GettingStarted-API/docker-appsettings.json OnPremise/Mixed/GettingStarted-API/appsettings.json

# Restore dependencies for the API project (will restore all dependencies)
RUN dotnet restore OnPremise/Mixed/GettingStarted-API/GettingStarted-API.csproj

# Build and publish the API project with proper platform targeting
ARG CONFIG=Release
RUN dotnet publish OnPremise/Mixed/GettingStarted-API/GettingStarted-API.csproj \
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

CMD ["dotnet", "GettingStarted-API.dll"]
