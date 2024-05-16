/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#ifndef UDGMATHTOOLS_H
#define UDGMATHTOOLS_H

#include <QList>
#include <QVector>

#include "vector3.h"

class QVector2D;
class QVector3D;

namespace udg {

/**
This class is a collection of general mathematical functions.
 Includes the declaration of constants (Pi, Number E, ...), arithmetic operations,
 operations with vectors, etc.
*/
class MathTools {
public:
    enum IntersectionResults { ParallelLines, SkewIntersection, LinesIntersect };

    ///
    /// Declaration of constants
    ///
    static const double NumberEBase2Logarithm;
    static const long double ReversePiNumberLong;
    static const double PiNumber;
    static const long double PiNumberLong;
    static const long double PiNumberDivBy2Long;
    static const double DegreesToRadiansAsDouble;
    static const double RadiansToDegreesAsDouble;
    /// Epsilon, nombre extremadament petit
    static const double Epsilon;
    /// Maximum value of a double type
    static const double DoubleMaximumValue;

    ///
    /// Operacions aritmètiques
    ///

    /// Returns the bounded value between minimum and maximum corresponding to the given value.
    /// If loop is true, when value is out of bounds, the oppposite
    /// of the surpassed bound (min or max) is returned
    /// NOTE: This method has been implemented here to avoid the include of the cpp file
    /// at the end of the file because of the templated method
    template<typename T>
    static T getBoundedValue(T value, T minimum, T maximum, bool loop)
    {
        T boundedValue = value;

        if (loop)
        {
            if (boundedValue < minimum)
            {
                boundedValue = maximum;
            }
            else if (value > maximum)
            {
                boundedValue = minimum;
            }
        }
        else
        {
            boundedValue = qBound(minimum, value, maximum);
        }

        return boundedValue;
    }

    /// Returns true if value is between min and mix (both inclusive), false otherwise
    /// NOTE: This method has been implemented here to avoid the include
    /// of the cpp file at the end of the file because of the templated method
    template<typename T>
    static bool isInsideRange(T value, T min, T max)
    {
        return value >= min && value <= max;
    }

    /// Tells if the number is odd/even
    static bool isOdd(int x);
    static bool isEven(int x);

    ///It returns the truncated value to us
    static double truncate(double x);

    ///Rounds a real number to the nearest integer
    static int roundToNearestInteger(double x);

    /// Transforms the value passed by degree parameter to radians and vice versa
    static float degreesToRadians(float degrees);
    static float radiansToDegrees(float radians);

    /// Calculate the logarithm in base 2
    /// @param x Value of which the logarithm is calculated
    /// @param zero If true, if x is 0, it will return 0, otherwise
    /// will also calculate the logarithm without doing this check
    static double logTwo(const double x, const bool zero = true);

    ///Cubic root
    static double cubeRoot(double x);

    /// Determine if we can consider the two to be practically equal
    /// values passed by parameter if their difference is less than the value of Epsilon
    static bool closeEnough(float f1, float f2);

    /// Distance between 3D points
    static double getDistance3D(const double firstPoint[3], const double secondPoint[3]);

    /// Random number generation helpers

    /// Generates a random number in the specified range, double and int precision
    static double randomDouble(double minimum, double maximum);
    static int randomInt(int minimum, int maximum);

    /// Returns the distance between a point and the nearest edge.
    /// LastToFirstEdge means if we want the edge to be checked
    /// which forms the last and first point of the list.
    /// The closestPoint output parameter will tell us what the point is
    /// of the line closest to the indicated point and ClosestEdge will indicate the edge.
    static double getPointToClosestEdgeDistance(double point3D[3], const QList<QVector<double> > &pointsList, bool lastToFirstEdge, double closestPoint[3], int &closestEdge);

    /// Distance between a point and a finite segment defined by lineFirstPoint and lineSecondPoint
    /// The closestPoint output parameter will tell us what it is
    /// the point on the line closest to the indicated point
    static double getPointToFiniteLineDistance(double point[3], double lineFirstPoint[3], double lineSecondPoint[3], double closestPoint[3]);

    ///
    /// Intersection calculations
    ///

    /// Calculate the intersection of two planes defined by a point and a normal (p, n) and (q, m) respectively
    /// Returns 0 if the planes do not intersect because they are parallel, otherwise> 0
    static int planeIntersection(double p[3], double n[3], double q[3], double m[3], double r[3], double t[3]);

    /// Returns the point of intersection of two lines
    /// infinites defined by two segments
    /// @param p1 First point of the first line
    /// @param p2 Second point of the first line
    /// @param p3 First point of the second line
    /// @param p4 Second point of the second line
    /// @param state It will contain the result of the intersection: ParallelLines,
    /// LinesIntersect, SkewLines (they do not intersect but intersect, they are in parallel planes)
    static double* infiniteLinesIntersection(double *p1, double *p2, double *p3, double *p4, int &state);

    ///
    /// Calculations with vectors
    ///

    /// From the segment determined by the two points passed as parameters,
    /// calculates one of the possible director vectors
    static QVector3D directorVector(const QVector3D &point1, const QVector3D &point2);

    ///Calculates the modulus of a vector
    static double modulus(double vector[3]);

    ///Scalar product of the two vectors passed by parameter
    static double dotProduct(double vector1[3], double vector2[3]);

    /// Calculate the vector product of the two director vectors
    /// passed by parameter and leaves us the result in the third parameter
    static void crossProduct(double vectorDirector1[3], double vectorDirector2[3], double crossProductVector[3]);

    /// Normalizes the vector passed by parameter
    static double normalize(double vector[3]);

    /// Returns the angle in radians of a 2D vector with the horizontal axis. Returns values in the range [-pi, pi].
    static double angleInRadians(const QVector2D &vector);
    /// Returns the angle in degrees of a 2D vector with the horizontal axis. Returns values in the range [-180, 180].
    static double angleInDegrees(const QVector2D &vector);

    ///Calculate the angle between two vectors. Returns the value in radians or degrees
    static double angleInRadians(const Vector3 &vec1, const Vector3 &vec2);
    static double angleInDegrees(const Vector3 &vec1, const Vector3 &vec2);

    /// Returns true if \ a x is NaN.
    static bool isNaN(double x);

    ///Returns the power of 2 higher closest to v.
    static unsigned int roundUpToPowerOf2(unsigned int v);

    /// Returns the first multiple of a multiple greater than or equal to and.
    /// Only works if multiple> 0, but parameters are not checked.
    static unsigned int roundUpToMultipleOfNumber(unsigned int i, unsigned int multiple);

private:
    /// Initializes random seed if needed
    static void initializeRandomSeed();
};

} // End namespace udg

#endif
