#pragma once

#include "Vector.h"
#include "Matrix.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <limits>
#include <vector>

namespace Maths
{
	// ------------------------

	struct Ray
	{
		Ray()
			: mStart(0.0f, 0.0f, 0.0f)
			, mDirection(0.0f, 0.0f, 0.0f)
		{

		}

		Ray(Maths::Vector::Vector3D<float> start, Maths::Vector::Vector3D<float> direction)
			: mStart(start)
			, mDirection(direction)
		{

		}

		Maths::Vector::Vector3D<float> mStart;
		Maths::Vector::Vector3D<float> mDirection;
	};

	// ------------------------

	static Ray ProjectIntoWorld(Maths::Vector::Vector2D<float> screenPosition, glm::mat4 viewMatrix, glm::mat4 projectionMatrix, float screenHeight, float screenWidth, Maths::Vector::Vector3D<float> cameraPosition)
	{
		// Clip space coords - (-1.0 -> 1.0, -1.0 -> 1.0)
		float x =  ((screenPosition.x / screenWidth)  * 2.0f) - 1.0f;
		float y = (-(screenPosition.y / screenHeight) * 2.0f) + 1.0f;

		// Need to get from clip space to view space and then to world space
		Maths::Vector::Vector4D<float> origin = Maths::Vector::Vector4D<float>(x, y, -1.0, 1.0);

		glm::mat4 inverseViewProjectionMat = glm::inverse(projectionMatrix * viewMatrix);

		origin = inverseViewProjectionMat * origin;

		origin.w = 1.0f / origin.w;
		origin.x *= origin.w;
		origin.y *= origin.w;
		origin.z *= origin.w;

		Ray returnRay;
		returnRay.mDirection = Maths::Vector::Vector3D<float>(origin.x, origin.y, origin.z) - cameraPosition;
		returnRay.mDirection.Normalise();

		returnRay.mStart = Maths::Vector::Vector3D<float>(origin.x, origin.y, origin.z);

		return returnRay;
	}

	// ------------------------

    // https://math.stackexchange.com/questions/1036959/midpoint-of-the-shortest-distance-between-2-rays-in-3d
	static float DistanceBetweenLines(Ray ray1, Ray ray2)
	{
		using vec3 = Vector::Vector3D<float>;

		vec3 a = ray1.mStart;
		vec3 b = ray1.mDirection;
		b = b.Normalise();

		vec3 c = ray2.mStart;
		vec3 d = ray2.mDirection;
		d = d.Normalise();

		float bDotD = b.Dot(d);
		float aDotD = a.Dot(d);
		float bDotC = b.Dot(c);
		float cDotD = c.Dot(d);
		float aDotB = a.Dot(b);

		float sTop    = (bDotD * (aDotD - bDotC)) - (aDotD * cDotD);
		float sBottom = (bDotD * bDotD) - 1.0f;

		float tTop    = (bDotD * (cDotD - aDotD)) - (bDotC * aDotB);
		float tBottom = (bDotD * bDotD) - 1.0f;

		float t = tTop / tBottom;
		float s = sTop / sBottom;

		vec3 midPointOnA = a + (b * t);
		vec3 midPointOnC = c + (d * s);

		return (midPointOnA - midPointOnC).Magnitude();
	}

	// ------------------------

	static Maths::Vector::Vector3D<float> FindCollisionWithYZPlane(Ray ray, float distanceAlongAxis)
	{
		Maths::Vector::Vector3D<float> collisionPosition = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);

		float t = (distanceAlongAxis - ray.mStart.x) / ray.mDirection.x;

		collisionPosition.y = ray.mStart.y + (t * ray.mDirection.y);
		collisionPosition.z = ray.mStart.z + (t * ray.mDirection.z);

		return collisionPosition;
	}

	// ------------------------

	static Maths::Vector::Vector3D<float> FindCollisionWithXZPlane(Ray ray, float distanceAlongAxis)
	{
		Maths::Vector::Vector3D<float> collisionPosition = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);

		float t = (distanceAlongAxis - ray.mStart.y) / ray.mDirection.y;

		collisionPosition.x = ray.mStart.x + (t * ray.mDirection.x);
		collisionPosition.z = ray.mStart.z + (t * ray.mDirection.z);

		return collisionPosition;
	}

	// ------------------------

	static Maths::Vector::Vector3D<float> FindCollisionWithXYPlane(Ray ray, float distanceAlongAxis)
	{
		Maths::Vector::Vector3D<float> collisionPosition = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);

		float t = (distanceAlongAxis - ray.mStart.z) / ray.mDirection.z;

		collisionPosition.x = ray.mStart.x + (t * ray.mDirection.x);
		collisionPosition.y = ray.mStart.y + (t * ray.mDirection.y);

		return collisionPosition;
	}

	// ------------------------

	static Maths::Vector::Vector3D<float> FindCollisionWithPlane(Maths::Vector::Vector3D<float> planeNormal, Maths::Vector::Vector3D<float> pointOnPlane, Ray ray)
	{
		// Calculate how aligned with each other the plane and the ray's direction are
		double dotProductAlignment = Maths::Vector::Dot(planeNormal, ray.mDirection);

		// If it is really small then the two are parallel and never collide
		if (dotProductAlignment > 0.00001)
		{
			Maths::Vector::Vector3D<float> toPlane = pointOnPlane - ray.mStart;
			double t = Maths::Vector::Dot(toPlane, planeNormal) / dotProductAlignment;

			return ray.mStart + (ray.mDirection * (float)t);
		}

		return Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);
	}

	// ------------------------

	static float DistanceToPoint(Ray ray, Vector::Vector3D<float> point)
	{
		Vector::Vector3D<float> pointMinusStart = point - ray.mStart;

		float pointMinusStartDotDirection = pointMinusStart.Dot(ray.mDirection);

		float distance = std::abs((pointMinusStart - (ray.mDirection * pointMinusStartDotDirection)).Magnitude());
	}

	// ------------------------

	static Vector::Vector3D<float> ClosestPointOnLineSegment(Vector::Vector3D<float> start, Vector::Vector3D<float> end, Vector::Vector3D<float> point)
	{
		Vector::Vector3D<float> direction = end - start;
		float                   directionMagnitideSquared = direction.Dot(direction);
		float                   t = (float)Maths::Vector::Dot(point - start, direction) / directionMagnitideSquared;

		return start + (direction * std::min<float>(std::max<float>(t, 0.0f), 1.0f));
	}

	// ------------------------

	static float ToDegrees(float input)
	{
		return glm::degrees(input);
	}

	// ------------------------

	static Vector::Vector2D<float> ToDegrees(Vector::Vector2D<float> input)
	{
		return Vector::Vector2D<float>(ToDegrees(input.x), ToDegrees(input.y));
	}

	// ------------------------

	static Vector::Vector3D<float> ToDegrees(Vector::Vector3D<float> input)
	{
		return Vector::Vector3D<float>(ToDegrees(input.x), ToDegrees(input.y), ToDegrees(input.z));
	}

	// ------------------------

	static Vector::Vector4D<float> ToDegrees(Vector::Vector4D<float> input)
	{
		return Vector::Vector4D<float>(ToDegrees(input.x), ToDegrees(input.y), ToDegrees(input.z), ToDegrees(input.w));
	}

	// ------------------------
}