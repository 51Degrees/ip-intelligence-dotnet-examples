FROM mcr.microsoft.com/dotnet/sdk:8.0-noble AS build

# Install build tools for native library compilation
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libatomic1 \
    && rm -rf /var/lib/apt/lists/*

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
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libatomic1 \
        libstdc++6 \
        curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
USER app

# Copy the published application
COPY --from=build --chown=app /app/publish .

# Download the IP Intelligence data file directly to the final image
RUN --mount=type=secret,required=true,id=IPI_DATA_FILE_URL,env=IPI_DATA_FILE_URL \
    mkdir -p  /app/data && \
    curl -sSL "$IPI_DATA_FILE_URL" | gzip -cd >/app/data/51Degrees-EnterpriseIpiV41.ipi

EXPOSE 5225

CMD ["dotnet", "FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.dll"]
