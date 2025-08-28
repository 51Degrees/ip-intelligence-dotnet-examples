# See https://aka.ms/customizecontainer to learn how to customize your debug container and how Visual Studio uses this Dockerfile to build your images for faster debugging.

# This stage is used when running from VS in fast mode (Default for Debug configuration)
FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
USER $APP_UID
WORKDIR /app
EXPOSE 5225


# This stage is used to build the service project
FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR /src
COPY ["Examples/OnPremise/Mixed/GettingStarted-API/GettingStarted-API.csproj", "Examples/OnPremise/Mixed/GettingStarted-API/"]
COPY ["Examples/FiftyOne.IpIntelligence.Examples.OnPremise/FiftyOne.IpIntelligence.Examples.OnPremise.csproj", "Examples/FiftyOne.IpIntelligence.Examples.OnPremise/"]
COPY ["Examples/FiftyOne.IpIntelligence.Examples/FiftyOne.IpIntelligence.Examples.csproj", "Examples/FiftyOne.IpIntelligence.Examples/"]
RUN dotnet restore "./Examples/OnPremise/Mixed/GettingStarted-API/GettingStarted-API.csproj"
COPY . .
WORKDIR "/src/Examples/OnPremise/Mixed/GettingStarted-API"
RUN dotnet build "./GettingStarted-API.csproj" -c $BUILD_CONFIGURATION -o /app/build -r linux-x64 /p:Platform=x64

# This stage is used to publish the service project to be copied to the final stage
FROM build AS publish
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "./GettingStarted-API.csproj" -c $BUILD_CONFIGURATION -o /app/publish -r linux-x64 /p:Platform=x64 /p:UseAppHost=false

# This stage is used in production or when running from VS in regular mode (Default when not using the Debug configuration)
FROM base AS final
USER root
RUN apt-get update && apt-get install -y --no-install-recommends libatomic1 && rm -rf /var/lib/apt/lists/*
USER $APP_UID
WORKDIR /app
COPY --from=publish /app/publish .
COPY ip-intelligence-data/51Degrees-EnterpriseIpiV41.ipi .
COPY ip-intelligence-data/51Degrees-EnterpriseV41.hash .
RUN ls .
ENV LD_DEBUG=libs
ENTRYPOINT ["dotnet", "GettingStarted-API.dll"]
