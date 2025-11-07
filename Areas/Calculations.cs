// Ignore Spelling: wkt

/********************************************************************
 *This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 *The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

using NetTopologySuite.Geometries;
using NetTopologySuite.IO;
using ProjNet.CoordinateSystems;
using ProjNet.CoordinateSystems.Transformations;
using System;

namespace Examples.OnPremise.Areas;

public class Calculations
{
    private static readonly WKTReader _wktReader = new();

    private static readonly CoordinateTransformationFactory
        _transformFactory = new();

    public static Result GetAreas(string wkt)
    {
        var geo = _wktReader.Read(wkt);
        if (geo != null)
        {
            return new (
                (int)Math.Round(GetAreas(geo)),
                geo.NumGeometries);
        }
        return new(0,0);
    }

    private static double GetAreas(Geometry geo)
    {
        var area = 0.0;
        if (geo.NumGeometries > 0)
        {
            for (var i = 0; i < geo.NumGeometries; i++)
            {
                area += GetArea(geo.GetGeometryN(i));
            }
        }
        else
        {
            area += GetArea(geo);
        }
        return area;
    }

    private static double GetArea(Geometry geo)
    {
        // Create UTM projected coordinate system for a specific zone
        // (e.g., Zone 30N for London)
        var utmZone = (int)Math.Floor((geo.InteriorPoint.X + 180) / 6) + 1;
        var isNorthernHemisphere = geo.InteriorPoint.Y >= 0;

        // Create a coordinate transformation
        var transform = _transformFactory.CreateFromCoordinateSystems(
            // Define the source (WGS84) and target (Projected) coordinate
            // systems
            GeographicCoordinateSystem.WGS84,
            // Adjust UTM zone and hemisphere as needed
            ProjectedCoordinateSystem.WGS84_UTM(
                utmZone,
                isNorthernHemisphere));

        // Re-project the polygon to the UTM coordinate system
        var transformedPolygon = TransformGeometry(
            geo,
            transform.MathTransform);

        // Calculate area in square meters and convert to square kilometers
        return transformedPolygon.Area / 1_000_000;
    }

    private static Geometry TransformGeometry(
        Geometry geometry,
        MathTransform transform)
    {
        var factory = geometry.Factory;
        var coordinates = geometry.Coordinates;

        for (int i = 0; i < coordinates.Length; i++)
        {
            var transformed =
                transform.Transform([coordinates[i].X, coordinates[i].Y]);
            coordinates[i].X = transformed[0];
            coordinates[i].Y = transformed[1];
        }

        return factory.CreateGeometry(geometry);
    }
}
