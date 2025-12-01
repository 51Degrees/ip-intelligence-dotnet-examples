using NetTopologySuite.Geometries;
using ProjNet.CoordinateSystems;
using ProjNet.CoordinateSystems.Transformations;
using System;

namespace Examples.OnPremise.Areas;

public class Rectangle
{
    /// <summary>
    /// Used to handle area calculations that are aware the earth is not a
    /// sphere and has more complex mappings between WGS84 latitudes and
    /// longitudes are geographic areas.
    /// </summary>
    private static readonly CoordinateTransformationFactory
        _transformFactory = new();

    public ICoordinateTransformation Transformation { get; private set; }

    public Polygon Polygon { get; private set; }

    public Rectangle(Polygon polygon)
    {
        Polygon = polygon;
        Transformation = CreateTransform(
            polygon.InteriorPoint.X,
            polygon.InteriorPoint.Y);
    }

    public static ICoordinateTransformation CreateTransform(double x, double y)
    {
        // Create UTM projected coordinate system for a specific zone
        var utmZone = (int)Math.Floor((x + 180) / 6) + 1;
        var isNorthernHemisphere = y >= 0;

        // Create a coordinate transformation
        return _transformFactory.CreateFromCoordinateSystems(
            // Define the source (WGS84) and target (Projected) coordinate
            // systems
            GeographicCoordinateSystem.WGS84,
            // Adjust UTM zone and hemisphere as needed
            ProjectedCoordinateSystem.WGS84_UTM(
                utmZone,
                isNorthernHemisphere));
    }
}
