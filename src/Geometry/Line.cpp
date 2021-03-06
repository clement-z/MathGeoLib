/* Copyright Jukka Jyl�nki

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/** @file Line.cpp
	@author Jukka Jyl�nki
	@brief Implementation for the Line geometry object. */
#include "Line.h"
#include "Ray.h"
#include "LineSegment.h"
#include "../Math/float3x3.h"
#include "../Math/float3x4.h"
#include "../Math/float4x4.h"
#include "OBB.h"
#include "../Math/Quat.h"
#include "Frustum.h"
#include "Triangle.h"
#include "Plane.h"
#include "Polygon.h"
#include "Polyhedron.h"
#include "Sphere.h"
#include "AABB.h"
#include "Capsule.h"
#include "Circle.h"
#include "../Math/MathFunc.h"

#ifdef MATH_ENABLE_STL_SUPPORT
#include <iostream>
#endif

MATH_BEGIN_NAMESPACE

/// Computes the closest point pair on two lines.
/** The first line is specified by two points start0 and end0. The second line is specified by
	two points start1 and end1.
	The implementation of this function follows http://paulbourke.net/geometry/lineline3d/ .
	@param v0 The starting point of the first line.
	@param v1 The ending point of the first line.
	@param v2 The starting point of the second line.
	@param v3 The ending point of the second line.
	@param d [out] If specified, receives the normalized distance of the closest point along the first line.
		This pointer may be left null.
	@param d2 [out] If specified, receives the normalized distance of the closest point along the second line.
		This pointer may be left null.
	@return Returns the closest point on line start0<->end0 to the second line.
	@note This is a low-level utility function. You probably want to use ClosestPoint() or Distance() instead.
	@see ClosestPoint(), Distance(). */
vec Line::ClosestPointLineLine(const vec &v0, const vec &v1, const vec &v2, const vec &v3, float *d, float *d2)
{
#define DMNOP(m, n, o, p) ((v##m.x - v##n.x) * (v##o.x - v##p.x) + (v##m.y - v##n.y) * (v##o.y - v##p.y) + (v##m.z - v##n.z) * (v##o.z - v##p.z))
	float d0232 = DMNOP(0,2,3,2);
	float d3210 = DMNOP(3,2,1,0);
	float d3232 = DMNOP(3,2,3,2);
	float mu = (d0232 * d3210 - DMNOP(0,2,1,0)*d3232) / (DMNOP(1,0,1,0)*DMNOP(3,2,3,2) - DMNOP(3,2,1,0)*DMNOP(3,2,1,0));
#undef DMNOP
	if (d)
		*d = mu;

	if (d2)
		*d2 = (d0232 + mu * d3210) / d3232;

	return v0 + mu * (v1 - v0);
}

Line::Line(const vec &pos_, const vec &dir_)
:pos(pos_), dir(dir_)
{
	assume2(dir.IsNormalized(), dir, dir.LengthSq());
}

Line::Line(const Ray &ray)
:pos(ray.pos), dir(ray.dir)
{
	assume2(dir.IsNormalized(), dir, dir.LengthSq());
}

Line::Line(const LineSegment &lineSegment)
:pos(lineSegment.a), dir(lineSegment.Dir())
{
}

bool Line::IsFinite() const
{
	return pos.IsFinite() && dir.IsFinite();
}

vec Line::GetPoint(float d) const
{
	assume2(dir.IsNormalized(), dir, dir.LengthSq());
	return pos + d * dir;
}

void Line::Translate(const vec &offset)
{
	pos += offset;
}

void Line::Transform(const float3x3 &transform)
{
	pos = transform.Transform(pos);
	dir = transform.Transform(dir);
}

void Line::Transform(const float3x4 &transform)
{
	pos = transform.MulPos(pos);
	dir = transform.MulDir(dir);
}

void Line::Transform(const float4x4 &transform)
{
	pos = transform.MulPos(pos);
	dir = transform.MulDir(dir);
}

void Line::Transform(const Quat &transform)
{
	pos = transform.Transform(pos);
	dir = transform.Transform(dir);
}

bool Line::Contains(const vec &point, float distanceThreshold) const
{
	return ClosestPoint(point).DistanceSq(point) <= distanceThreshold;
}

bool Line::Contains(const Ray &ray, float epsilon) const
{
	return Contains(ray.pos, epsilon) && dir.Equals(ray.dir, epsilon);
}

bool Line::Contains(const LineSegment &lineSegment, float epsilon) const
{
	return Contains(lineSegment.a, epsilon) && Contains(lineSegment.b, epsilon);
}

bool Line::Equals(const Line &line, float epsilon) const
{
	assume2(dir.IsNormalized(), dir, dir.LengthSq());
	assume2(line.dir.IsNormalized(), line.dir, line.dir.LengthSq());
	// If the point of the other line is on this line, and the two lines point to the same, or exactly reverse directions,
	// they must be equal.
	return Contains(line.pos, epsilon) && EqualAbs(Abs(dir.Dot(line.dir)), 1.f, epsilon);
}

float Line::Distance(const vec &point, float *d) const
{
	return ClosestPoint(point, d).Distance(point);
}

float Line::Distance(const Ray &other, float *d, float *d2) const
{
	float u2;
	vec c = ClosestPoint(other, d, &u2);
	if (d2) *d2 = u2;
	return c.Distance(other.GetPoint(u2));
}

float Line::Distance(const Ray &other) const
{
	return Distance(other, 0, 0);
}

float Line::Distance(const Line &other, float *d, float *d2) const
{
	float u2;
	vec c = ClosestPoint(other, d, &u2);
	if (d2) *d2 = u2;
	return c.Distance(other.GetPoint(u2));
}

float Line::Distance(const Line &other) const
{
	return Distance(other, 0, 0);
}

float Line::Distance(const LineSegment &other, float *d, float *d2) const
{
	float u2;
	vec c = ClosestPoint(other, d, &u2);
	if (d2) *d2 = u2;
	mathassert(u2 >= 0.f);
	mathassert(u2 <= 1.f);
	return c.Distance(other.GetPoint(u2));
}

float Line::Distance(const LineSegment &other) const
{
	return Distance(other, 0, 0);
}

float Line::Distance(const Sphere &other) const
{
	return Max(0.f, Distance(other.pos) - other.r);
}

float Line::Distance(const Capsule &other) const
{
	return Max(0.f, Distance(other.l) - other.r);
}

bool Line::Intersects(const Triangle &triangle, float *d, vec *intersectionPoint) const
{
	return triangle.Intersects(*this, d, intersectionPoint);
}

bool Line::Intersects(const Plane &plane, float *d) const
{
	return plane.Intersects(*this, d);
}

bool Line::Intersects(const Sphere &s, vec *intersectionPoint, vec *intersectionNormal, float *d) const
{
	return s.Intersects(*this, intersectionPoint, intersectionNormal, d) > 0;
}

bool Line::Intersects(const AABB &aabb) const
{
	return aabb.Intersects(*this);
}

bool Line::Intersects(const AABB &aabb, float &dNear, float &dFar) const
{
	return aabb.Intersects(*this, dNear, dFar);
}

bool Line::Intersects(const OBB &obb) const
{
	return obb.Intersects(*this);
}

bool Line::Intersects(const OBB &obb, float &dNear, float &dFar) const
{
	return obb.Intersects(*this, dNear, dFar);
}

bool Line::Intersects(const Capsule &capsule) const
{
	return capsule.Intersects(*this);
}

bool Line::Intersects(const Polygon &polygon) const
{
	return polygon.Intersects(*this);
}

bool Line::Intersects(const Frustum &frustum) const
{
	return frustum.Intersects(*this);
}

bool Line::Intersects(const Polyhedron &polyhedron) const
{
	return polyhedron.Intersects(*this);
}

bool Line::IntersectsDisc(const Circle &disc) const
{
	return disc.IntersectsDisc(*this);
}

vec Line::ClosestPoint(const vec &targetPoint, float *d) const
{
	float u = Dot(targetPoint - pos, dir);
	if (d)
		*d = u;
	return GetPoint(u);
}

vec Line::ClosestPoint(const Ray &other, float *d, float *d2) const
{
	float t2;
	vec closestPoint = ClosestPointLineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, &t2);
	if (t2 <= 0.f)
	{
		if (d2)
			*d2 = 0.f;
		return ClosestPoint(other.pos, d);
	}
	else
	{
		if (d2)
			*d2 = t2;
		return closestPoint;
	}
}

vec Line::ClosestPoint(const Line &other, float *d, float *d2) const
{
	return ClosestPointLineLine(pos, pos + dir, other.pos, other.pos + other.dir, d, d2);
}

vec Line::ClosestPoint(const LineSegment &other, float *d, float *d2) const
{
	float t2;
	vec closestPoint = ClosestPointLineLine(pos, pos + dir, other.a, other.b, d, &t2);
	if (t2 <= 0.f)
	{
		if (d2)
			*d2 = 0.f;
		return ClosestPoint(other.a, d);
	}
	else if (t2 >= 1.f)
	{
		if (d2)
			*d2 = 1.f;
		return ClosestPoint(other.b, d);
	}
	else
	{
		if (d2)
			*d2 = t2;
		return closestPoint;
	}
}

vec Line::ClosestPoint(const Triangle &triangle, float *outU, float *outV, float *outD) const
{
	///\todo Optimize this function!
	vec closestPointTriangle = triangle.ClosestPoint(*this);
	if (outU || outV)
	{
		float2 uv = triangle.BarycentricUV(closestPointTriangle);
		if (outU)
			*outU = uv.x;
		if (outV)
			*outV = uv.y;
	}
	return ClosestPoint(closestPointTriangle, outD);
}

bool Line::AreCollinear(const vec &p1, const vec &p2, const vec &p3, float epsilon)
{
	return vec::AreCollinear(p1, p2, p3, epsilon);
}

Ray Line::ToRay() const
{
	return Ray(pos, dir);
}

LineSegment Line::ToLineSegment(float d) const
{
	return LineSegment(pos, GetPoint(d));
}

void Line::ProjectToAxis(const vec &direction, float &outMin, float &outMax) const
{
	// Most of the time, the projection of a line spans the whole 1D axis.
	// As a special case, if the line is perpendicular to the direction vector in question,
	// then the projection interval of this line is a single point.
	if (dir.IsPerpendicular(direction))
		outMin = outMax = Dot(direction, pos);
	else
	{
		outMin = -FLOAT_INF;
		outMax = FLOAT_INF;
	}
}

LineSegment Line::ToLineSegment(float dStart, float dEnd) const
{
	return LineSegment(GetPoint(dStart), GetPoint(dEnd));
}

Line operator *(const float3x3 &transform, const Line &l)
{
	return Line(transform * l.pos, transform * l.dir);
}

Line operator *(const float3x4 &transform, const Line &l)
{
	return Line(transform.MulPos(l.pos), transform.MulDir(l.dir));
}

Line operator *(const float4x4 &transform, const Line &l)
{
	return Line(transform.MulPos(l.pos), transform.MulDir(l.dir));
}

Line operator *(const Quat &transform, const Line &l)
{
	return Line(transform * l.pos, transform * l.dir);
}

#ifdef MATH_ENABLE_STL_SUPPORT
std::string Line::ToString() const
{
	char str[256];
	sprintf(str, "Line(pos:(%.2f, %.2f, %.2f) dir:(%.2f, %.2f, %.2f))",
		pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);
	return str;
}

std::ostream &operator <<(std::ostream &o, const Line &line)
{
	o << line.ToString();
	return o;
}

#endif

MATH_END_NAMESPACE
