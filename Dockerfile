FROM mcr.microsoft.com/dotnet/sdk:8.0-noble AS build
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libatomic1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY Examples .
ARG CONFIG=Release
RUN dotnet publish OnPremise/GettingStarted-Web/GettingStarted-Web.csproj \
    -c "$CONFIG" \
    -p:Platform=x64 \
    -p:BuildType=Release \
    --arch x64 \
    -o /app/publish


FROM mcr.microsoft.com/dotnet/aspnet:8.0-noble
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libatomic1 \
        libstdc++6 \
        curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build --chown=app /app/publish .
RUN --mount=type=secret,required=true,id=IPI_DATA_FILE_URL,env=IPI_DATA_FILE_URL \
    mkdir -p data \
    && curl -sSL "$IPI_DATA_FILE_URL" | gzip -cd >/app/data/51Degrees-EnterpriseIpiV41.ipi \
    && chown -R app:app data
USER app
CMD ["dotnet", "FiftyOne.IpIntelligence.Examples.OnPremise.GettingStartedWeb.dll"]
EXPOSE 5225
