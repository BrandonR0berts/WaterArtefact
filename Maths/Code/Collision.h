#pragma once

#include "Maths/Code/Vector.h"
#include "Maths/Code/Common.h"

#include <glm/gtx/quaternion.hpp>

namespace Maths
{
	namespace Collision
	{
		// ---------------------------

		struct Triangle final
		{
			Vector::Vector3D<float> mPoint1;
			Vector::Vector3D<float> mPoint2;
			Vector::Vector3D<float> mPoint3;
		};

		// ---------------------------

		struct Sphere final
		{
			Vector::Vector3D<float> mCentre;
			float                   mRadius;
		};

		// ---------------------------

		struct Capsule final
		{
			Vector::Vector3D<float> mPoint1;
			Vector::Vector3D<float> mPoint2;
			float                   mRadius;
		};

		// ---------------------------

		struct AABB final
		{
			Vector::Vector3D<float> mCentre;
			Vector::Vector3D<float> mHalfExtents;
		};

		// ---------------------------

		struct OBB final
		{
			Vector::Vector3D<float> mCentre;
			Vector::Vector3D<float> mHalfExtents;
			Vector::Vector4D<float> mRotation;

			// ------

			Maths::Vector::Vector3D<Vector::Vector3D<float>> CalculateLocalAxis()
			{
				Maths::Vector::Vector3D<Vector::Vector3D<float>> localAxis = Vector::Vector3D<Vector::Vector3D<float>>(Vector::Vector3D<float>(1.0f, 0.0f, 0.0f), 
					                                                                                                   Vector::Vector3D<float>(0.0f, 1.0f, 0.0f), 
					                                                                                                   Vector::Vector3D<float>(0.0f, 0.0f, 1.0f));

				// Convert quaternion to rotation matrix
				glm::mat4 RotationMatrix = glm::toMat4((glm::quat)mRotation);
				
				// Now apply the rotation to the world axis to get the resulting axis
				for (int i = 0; i < 3; i++)
				{
					localAxis[i] = localAxis[i].MultipyByMatrix(RotationMatrix);
				}

				return localAxis;
			}

			// ------
		};

		// ---------------------------

		struct Plane final
		{
			Vector::Vector3D<float> mNormal;
			float                   mD;
		};

		// ---------------------------

		struct LineSegment final
		{
			Vector::Vector3D<float> mStart;
			Vector::Vector3D<float> mEnd;

			Vector::Vector3D<float> GetCentre()
			{
				return (mStart + mEnd) * 0.5f;
			}
		};

		// ---------------------------
		// ---------------------------
		// ---------------------------

		static Vector::Vector3D<float> ClosestPointOnPlane(Plane plane, Vector::Vector3D<float> point)
		{
			// Solve for t which satisfies the equation of the plane, through moving in the direction of the normal (or opposite direction)
			double t = (Vector::Dot(plane.mNormal, point) - plane.mD) / plane.mNormal.Dot(plane.mNormal);

			// Now return the point found that satisfies the equation
			return point - (plane.mNormal * (float)t);
		}

		// ---------------------------

		static float DistanceToPlaneFromPoint(Plane plane, Vector::Vector3D<float> point)
		{
			return ((float)Vector::Dot(plane.mNormal, point)) / (float)plane.mNormal.Dot(plane.mNormal);
		}

		// ---------------------------

		static Vector::Vector3D<float> ClosestPointToLineSegment(LineSegment segment, Vector::Vector3D<float> point)
		{
			Vector::Vector3D<float> direction = segment.mEnd - segment.mStart;

			// Project the point onto the direction vector
			float t = (float)Vector::Dot(point - segment.mStart, direction) / (float)Vector::Dot(direction, direction);

			// Clamp to extents
			if (t < 0.0f)
				t = 0.0f;

			if (t > 1.0f) 
				t = 1.0f;

			// Return final point
			return segment.mStart + (direction * t);
		}

		// ---------------------------

		static float SquareDistanceFromPointToSegment(LineSegment segment, Vector::Vector3D<float> point)
		{
			using vec3 = Maths::Vector::Vector3D<float>;

			vec3 direction    = segment.mEnd - segment.mStart; // ab
			vec3 startToPoint = point        - segment.mStart; // ac
			vec3 endToPoint   = point        - segment.mEnd;   // bc

			float e           = (float)Vector::Dot(startToPoint, direction);

			if (e <= 0.0f)
			{
				return (float)Vector::Dot(startToPoint, startToPoint);
			}

			float f = (float)Vector::Dot(direction, direction);

			if (e >= f)
			{
				return (float)Vector::Dot(endToPoint, endToPoint);
			}

			// The point does actually fall onto the line segment, so calculate where it is
			return (float)(Vector::Dot(startToPoint, startToPoint) - (e * e) / f);
		}

		// ---------------------------

		static float SquaredDistanceFromPointToAABB(AABB box, Vector::Vector3D<float> point)
		{
			float squaredDistance = 0.0f;

			Vector::Vector3D<float> boxMax = box.mCentre + box.mHalfExtents;
			Vector::Vector3D<float> boxMin = box.mCentre - box.mHalfExtents;

			// Clamp each axis that is outside the box to being in the box
			for (int i = 0; i < 3; i++)
			{
				float axis = point[i];

				if (axis < boxMin[i]) squaredDistance += (boxMin[i] - axis) * (boxMin[i] - axis);
				if (axis > boxMin[i]) squaredDistance += (axis - boxMax[i]) * (axis - boxMax[i]);
			}

			return squaredDistance;
		}

		// ---------------------------

		static Vector::Vector3D<float> ClosestPointOnAABBToPoint(AABB box, Vector::Vector3D<float> point)
		{
			Vector::Vector3D<float> boxMax = box.mCentre + box.mHalfExtents;
			Vector::Vector3D<float> boxMin = box.mCentre - box.mHalfExtents;

			// Clamp each axis that is outside the box to being in the box
			for (int i = 0; i < 3; i++)
			{
				float axis = point[i];

				axis = std::min(axis, boxMax[i]);
				axis = std::max(axis, boxMin[i]);

				point[i] = axis;
			}

			return point;
		}

		// ---------------------------
		 
		static float DistanceFromPointToOBB(OBB box, Vector::Vector3D<float> point)
		{
			Vector::Vector3D<float> centreToPoint   = point - box.mCentre;
			float                   squaredDistance = 0.0f;

			Maths::Vector::Vector3D<Vector::Vector3D<float>> localAxis = box.CalculateLocalAxis();

			for (int i = 0; i < 3; i++)
			{
				// Project vector to point onto each axis and get the distance
				float distance = (float)Vector::Dot(centreToPoint, localAxis[i]);
				float excess   = 0.0f; // The distance beyond the side of the box

				if (distance < -box.mHalfExtents[i])
				{
					excess = distance + box.mHalfExtents[i];
				}
				else if (distance > box.mHalfExtents[i])
				{
					excess = distance - box.mHalfExtents[i];
				}

				squaredDistance += excess;
			}

			return squaredDistance;
		}

		// ---------------------------

		static Vector::Vector3D<float> ClosestPointOnOBBToPoint(OBB box, Vector::Vector3D<float> point)
		{
			// Calculate the local axis
			Maths::Vector::Vector3D<Vector::Vector3D<float>> localAxis = box.CalculateLocalAxis();

			Vector::Vector3D<float> centreToPoint = point - box.mCentre;
			Vector::Vector3D<float> newPoint      = box.mCentre;

			// Now clamp the point to the box along the right axis (after projecting onto the axis)
			for (int i = 0; i < 3; i++)
			{
				// Project
				float distance = (float)Vector::Dot(centreToPoint, localAxis[i]);

				// See if the distance is further than the extents of the box
				if (distance >  box.mHalfExtents[i]) distance =  box.mHalfExtents[i];
				if (distance < -box.mHalfExtents[i]) distance = -box.mHalfExtents[i];

				// Move the new point to the side of the box
				newPoint += localAxis[i] * distance;
			}

			return newPoint;
		}

		// --------------------------- 

		// Im not 100% on how this works, but will come back to it as some point later on to fully understand it
		static Vector::Vector3D<float> ClosestPointOnTriangleToPoint(Triangle triangle, Vector::Vector3D<float> point)
		{
			// ---------
			 
			// Check if the point is in vertex region outside of the first point
			Vector::Vector3D<float> triangleEdge1 = triangle.mPoint2 - triangle.mPoint1;
			Vector::Vector3D<float> triangleEdge2 = triangle.mPoint3 - triangle.mPoint1;

			Vector::Vector3D<float> toPoint       = point - triangle.mPoint1;

			float                   distance1     = (float)Vector::Dot(triangleEdge1, toPoint);
			float                   distance2     = (float)Vector::Dot(triangleEdge2, toPoint);

			if (distance1 <= 0.0f && distance2 <= 0.0f) 
				return triangle.mPoint1;

			// ---------

			Vector::Vector3D<float> toPoint2  = point - triangle.mPoint2;
			float                   distance3 = (float)Vector::Dot(triangleEdge1, toPoint2);
			float                   distance4 = (float)Vector::Dot(triangleEdge2, toPoint2);

			if (distance3 >= 0.0f && distance4 <= 0.0f)
				return triangle.mPoint2;

			// ---------

			float vc = (distance1 * distance4) - (distance3 * distance2);

			if (vc <= 0.0f && distance1 >= 0.0f && distance3 <= 0.0f)
			{
				float v = distance1 / (distance1 - distance3);
				return triangle.mPoint1 + (triangleEdge1 * v);
			}

			// ---------

			Vector::Vector3D<float> toPoint3 = point - triangle.mPoint3;

			float distance5 = (float)Vector::Dot(triangleEdge1, toPoint3);
			float distance6 = (float)Vector::Dot(triangleEdge2, toPoint3);

			if (distance6 >= 0.0f && distance5 <= distance6)
			{
				return triangle.mPoint3;
			}

			// ---------

			float vb = (distance5 * distance2) - (distance1 * distance6);

			if (vb <= 0.0f && distance2 >= 0.0f && distance6 <= 0.0f)
			{
				float w = distance2 / (distance2 - distance6);
				return triangle.mPoint1 + (triangleEdge2 * w);
			}

			// ---------

			float va = (distance3 * distance6) - (distance5 * distance4);

			if (va <= 0.0f && (distance4 - distance3) >= 0.0f && (distance5 - distance6) >= 0.0f)
			{
				float w = (distance4 - distance3) / ((distance4 - distance3) + (distance5 - distance6));
				return triangle.mPoint2 + ((triangle.mPoint3 - triangle.mPoint2) * w);
			}

			// ---------

			float denom = 1.0f / (va + vb + vc);
			float v = vb * denom;
			float w = vc * denom;

			return triangle.mPoint1 + (triangleEdge1 * v) + (triangleEdge2 * w);
		}

		// --------------------------- 
		// ---------------------------
		// ---------------------------

		static bool TriangleToTriangle(Triangle triangle1, Triangle triangle2)
		{
			return false;
		}

		// ---------------------------

		static bool TriangleToSphere(Triangle triangle, Sphere sphere)
		{
			return false;
		}

		// ---------------------------

		static bool TriangleToCapsule(Triangle triangle, Capsule capsule)
		{
			return false;
		}

		// ---------------------------

		static bool TriangleToAABB(Triangle triangle, AABB aabb)
		{
			return false;
		}

		// ---------------------------

		static bool TriangleToOBB(Triangle triangle, OBB obb)
		{
			return false;
		}

		// ---------------------------
		// ---------------------------

		static bool SphereToSphere(Sphere sphere1, Sphere sphere2)
		{
			// Get distance between centres
			Vector::Vector3D<float> difference = (sphere1.mCentre - sphere2.mCentre);
			float distanceSquared = difference.Dot(difference);
			float radiusSum       = sphere1.mRadius + sphere2.mRadius;

			return distanceSquared <= (radiusSum * radiusSum);
		}

		// ---------------------------

		static bool SphereToAABB(Sphere sphere1, AABB aabb)
		{
			return false;
		}

		// ---------------------------

		static bool SphereToOBB(Sphere sphere1, OBB obb)
		{
			return false;
		}

		// ---------------------------

		static bool SphereToCapsule(Sphere sphere, Capsule capsule)
		{
			return false;
		}

		// ---------------------------
		// ---------------------------

		static bool CapsuleToCapsule(Capsule capsule1, Capsule capsule2)
		{
			return false;
		}

		// ---------------------------

		static bool CapsuleToAABB(Capsule capsule1, AABB aabb)
		{
			return false;
		}

		// ---------------------------

		static bool CapsuleToOBB(Capsule capsule1, OBB obb)
		{
			return false;
		}

		// ---------------------------
		// ---------------------------

		static bool AABBToAABB(AABB aabb1, AABB aabb2)
		{
			// This only collides if all axis overlap
			if (std::abs(aabb1.mCentre.x - aabb2.mCentre.x) > (aabb1.mHalfExtents.x + aabb2.mHalfExtents.x)) return false;
			if (std::abs(aabb1.mCentre.y - aabb2.mCentre.y) > (aabb1.mHalfExtents.y + aabb2.mHalfExtents.y)) return false;
			if (std::abs(aabb1.mCentre.z - aabb2.mCentre.z) > (aabb1.mHalfExtents.z + aabb2.mHalfExtents.z)) return false;

			return true;
		}

		// ---------------------------

		static bool AABBToOBB(AABB aabb, OBB obb)
		{
			return false;
		}

		// ---------------------------

		// Searching for the intersection of 
		static bool LineToAABB(AABB aabb, Ray ray)
		{
			float tMin    = 0.0f;
			float tMax    = FLT_MAX;
			float EPSILON = 0.0001f;

			Maths::Vector::Vector3D<float> boxMax = aabb.mCentre + aabb.mHalfExtents;
			Maths::Vector::Vector3D<float> boxMin = aabb.mCentre - aabb.mHalfExtents;

			for (unsigned int i = 0; i < 3; i++)
			{
				if (abs(ray.mDirection[i]) < EPSILON)
				{
					// Ray is parallel to slab, so the start must be in the slab for a hit
					if (ray.mStart[i] < boxMin[i] || ray.mStart[i] > boxMax[i])
						return false;
				}
				else
				{
					float ood = 1.0f / ray.mDirection[i];
					float t1  = (boxMin[i] - ray.mStart[i]) * ood;
					float t2  = (boxMax[i] - ray.mStart[i]) * ood;

					// t1 is the intersection with the near plane and t2 is the far plane
					if (t1 > t2)
					{
						float temp = t1;
						      t1   = t2;
						      t2   = temp;
					}

					tMin = std::max(tMin, t1);
					tMax = std::min(tMax, t2);

					if (tMin > tMax) 
						return false;
				}
			}

			return true;
		}

		// ---------------------------

		static bool SegmentToAABB(AABB aabb, LineSegment line)
		{
			using vec3 = Maths::Vector::Vector3D<float>;

			vec3 segmentMidpoint         = line.GetCentre();
			vec3 segmentHalflengthVector = line.mEnd - segmentMidpoint;

			// Translate the segment and box to the origin
			segmentMidpoint -= aabb.mCentre;

			// ---------

			// Seperating axis
			float xAxis = abs(segmentHalflengthVector.x);
			if (abs(segmentMidpoint.x) > aabb.mHalfExtents.x + xAxis) return false;

			float yAxis = abs(segmentHalflengthVector.y);
			if (abs(segmentMidpoint.y) > aabb.mHalfExtents.y + yAxis) return false;

			float zAxis = abs(segmentHalflengthVector.z);
			if (abs(segmentMidpoint.z) > aabb.mHalfExtents.z + zAxis) return false;

			// ---------

			float EPSILON = 0.0001f;

			xAxis += EPSILON;
			yAxis += EPSILON;
			zAxis += EPSILON;

			// ---------

			// Now try the cross products of the segment direction to the coordinate axis
			if (abs(segmentMidpoint.y * segmentHalflengthVector.z - segmentMidpoint.z * segmentHalflengthVector.y) > aabb.mHalfExtents.y * zAxis + aabb.mHalfExtents.z * yAxis) return false;
			if (abs(segmentMidpoint.z * segmentHalflengthVector.x - segmentMidpoint.x * segmentHalflengthVector.z) > aabb.mHalfExtents.x * zAxis + aabb.mHalfExtents.z * xAxis) return false;
			if (abs(segmentMidpoint.x * segmentHalflengthVector.y - segmentMidpoint.y * segmentHalflengthVector.x) > aabb.mHalfExtents.x * yAxis + aabb.mHalfExtents.y * xAxis) return false;

			// ---------

			return true;
		}

		// ---------------------------

		static bool OBBToOBB(OBB obb1, OBB obb2)
		{
			// First convert from a quaternion to a rotation matrix for both boxes

			// Then use this rotation matrix to find the local axis for each box

			// Now do seperating axis



			return true;
		}

		// ---------------------------
		// ---------------------------
		// ---------------------------

		static float DistanceBetweenTwoLines(Ray ray1, Ray ray2)
		{
			float a = ray1.mDirection.Dot(ray1.mDirection);
			float b = ray1.mDirection.Dot(ray2.mDirection);

			Maths::Vector::Vector3D<float> r = ray1.mStart - ray2.mStart;
			float c = ray1.mDirection.Dot(r);
			float e = ray2.mDirection.Dot(ray2.mDirection);
			float f = ray2.mDirection.Dot(r);

			float d = (a * e) - (b * b);

			if (d == 0.0f)
			{
				// Lines are parallel, so do the difference between the start points
				return (ray1.mStart - ray2.mStart).Magnitude();
			}

			float s = ((b * f) - (c * e)) / d;
			float t = ((a * f) - (b * c)) / d;

			Maths::Vector::Vector3D<float> pointOnLine1 = ray1.mStart + (ray1.mDirection * s);
			Maths::Vector::Vector3D<float> pointOnLine2 = ray2.mStart + (ray2.mDirection * t);

			return (pointOnLine1 - pointOnLine2).Magnitude();
		}

		// ---------------------------
	}
}