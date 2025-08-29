FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
USER $APP_UID
WORKDIR /app
EXPOSE 5225


FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
RUN apt-get update && apt-get install -y --no-install-recommends libatomic1 cmake make gcc g++ && rm -rf /var/lib/apt/lists/*
RUN mkdir src && chown -R ${APP_UID}:${APP_UID} /src
RUN mkdir app && chown -R ${APP_UID}:${APP_UID} /app
RUN pwsh --version
WORKDIR "/src/"
RUN git clone 'https://github.com/51Degrees/pipeline-dotnet.git' --depth 1 --recurse-submodules --shallow-submodules --branch 'version/4.5'
RUN git clone 'https://github.com/51Degrees/ip-intelligence-dotnet.git' --depth 1 --recurse-submodules --shallow-submodules --branch 'feature/rel-proj-refs'
RUN git clone 'https://github.com/postindustria-tech/device-detection-dotnet.git' --depth 1 --recurse-submodules --shallow-submodules --branch 'feature/rel-proj-refs'
WORKDIR "ip-intelligence-dotnet-examples"
COPY "Examples" "Examples"
COPY "Tests" "Tests"
COPY "FiftyOne.IpIntelligence.Examples.sln" "FiftyOne.IpIntelligence.Examples.sln"
COPY "FiftyOne.IpIntelligence.Examples.Core.slnf" "FiftyOne.IpIntelligence.Examples.Core.slnf"
RUN ls
RUN chown -R ${APP_UID}:${APP_UID} ..
USER $APP_UID
RUN dotnet restore "./FiftyOne.IpIntelligence.Examples.Core.slnf"
WORKDIR "Examples/OnPremise/Mixed/GettingStarted-API"
RUN dotnet build "./GettingStarted-API.csproj" -c $BUILD_CONFIGURATION -o /app/build -r linux-x64 /p:Platform=x64


FROM build AS publish
ARG BUILD_CONFIGURATION=Release
RUN dotnet publish "./GettingStarted-API.csproj" -c $BUILD_CONFIGURATION -o /app/publish -r linux-x64 /p:Platform=x64 /p:UseAppHost=false


FROM base AS final
USER root
RUN apt-get update && apt-get install -y --no-install-recommends libatomic1 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=publish /app/publish .
COPY ip-intelligence-data/51Degrees-EnterpriseIpiV41.ipi .
COPY ip-intelligence-data/51Degrees-EnterpriseV41.hash .
RUN chown -R ${APP_UID}:${APP_UID} .
USER $APP_UID
RUN ls .
RUN cat appsettings.json
ENV LD_DEBUG=libs
ENTRYPOINT ["dotnet", "GettingStarted-API.dll"]
