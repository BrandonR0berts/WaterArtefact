#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <math.h>
#include <string>
#include <vector>

// This file defines Vector2D, Vector3D and Vector4D functionality

// ==============================================================
// --------------------------------------------------------------

namespace Maths
{
	namespace Vector
	{
		// ---------------------------

		static std::vector<int> SplitCommaString_int(std::string inputString)
		{
			std::vector<int> returnValue;
			std::string      currentValue;

			// Search through the string for commas
			for (unsigned int i = 0; i < inputString.length(); i++)
			{
				if (inputString[i] == ' ')
					continue;

				// See if we should move onto the next number in the list
				if (inputString[i] == ',')
				{
					if (currentValue.empty())
						continue;

					returnValue.push_back(std::stoi(currentValue));
					currentValue = "";
				}
				else
				{
					currentValue += inputString[i];
				}
			}

			if (!currentValue.empty())
			{
				returnValue.push_back(std::stoi(currentValue));
			}

			return returnValue;
		}

		// ---------------------------

		static std::vector<float> SplitCommaString_float(std::string inputString)
		{
			std::vector<float> returnValue;
			std::string        currentValue = "";

			// Search through the string for commas and spaces
			for (unsigned int i = 0; i < inputString.length(); i++)
			{
				// See if we should move onto the next number in the list
				if (inputString[i] == ',')
				{
					if (currentValue.empty())
						continue;

					returnValue.push_back(std::stof(currentValue));
					currentValue = "";
				}
				else
				{
					currentValue += inputString[i];
				}
			}

			returnValue.push_back(std::stof(currentValue));

			return returnValue;
		}

		// ---------------------------

		template <typename type>
		class Vector2D final
		{
		public:
			Vector2D()
				: x(type(0.0))
				, y(type(0.0))
			{

			}

			Vector2D(type startX, type startY)
				: x(startX)
				, y(startY)
			{

			}

			Vector2D(std::vector<type>& inVector)
				: x(type(0.0))
				, y(type(0.0))
			{
				unsigned int size = inVector.size();
				switch (size)
				{
				case 2:
					y = inVector[1];
				case 1:
					x = inVector[0];
				break;
				}
			}

			~Vector2D() = default;


			// ----------------------------------------------------------------------------------------------

			// Single Operator overloads
			Vector2D operator +(const Vector2D& other)
			{
				return Vector2D(x + other.x, y + other.y);
			}

			Vector2D operator -(const Vector2D& other)
			{
				return Vector2D(x - other.x, y - other.y);
			}

			Vector2D operator *(const double& other)
			{
				return Vector2D(x * (type)other, y * (type)other);
			}

			Vector2D operator /(const double& other)
			{
				return Vector2D(x / other, y / other);
			}

			// ----------------------------------------------------------------------------------------------

			// Double Operator overloads
			Vector2D& operator ++()
			{
				x++;
				y++;

				return *this;
			}

			Vector2D& operator --()
			{
				x--;
				y--;

				return *this;
			}

			Vector2D& operator +=(const Vector2D& other)
			{
				x += other.x;
				y += other.y;

				return *this;
			}

			Vector2D& operator -=(const Vector2D& other)
			{
				x -= other.x;
				y -= other.y;

				return *this;
			}

			Vector2D& operator *=(const double& other)
			{
				x *= other;
				y *= other;

				return *this;
			}

			Vector2D& operator /=(const double& other)
			{
				x /= other;
				y /= other;

				return *this;
			}

			bool operator==(const Maths::Vector::Vector2D<type>& other)
			{
				return x == other.x && y == other.y;
			}

			bool operator!=(const Maths::Vector::Vector2D<type>& other)
			{
				return x != other.x || y != other.y;
			}

			// ----------------------------------------------------------------------------------------------

			// Mathematical useful functions

			double Dot(Vector2D& other)
			{
				return (x * other.x) + (y * other.y);
			}

			Vector2D Normalised()
			{
				double magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				return Vector2D(x / magnitude, y / magnitude);
			}

			Vector2D& Normalise()
			{
				double magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				x /= magnitude;
				y /= magnitude;

				return *this;
			}

			double Magnitude()
			{
				return sqrt((x * x) + (y * y));
			}

			double MagnitudeSquared()
			{
				return (x * x) + (y * y);
			}

			// ----------------------------------------------------------------------------------------------

			std::string to_string()
			{
				std::string returnValue = std::to_string(x) + ", " + std::to_string(y);
				return returnValue;
			}

			// ----------------------------------------------------------------------------------------------

			type x;
			type y;
		};


		// ==============================================================
		// --------------------------------------------------------------


		template <typename type>
		class Vector3D final
		{
		public:
			Vector3D() 
				: x(type(0.0))
				, y(type(0.0))
				, z(type(0.0))
			{

			}

			Vector3D(type startX, type startY, type startZ) 
				: x(startX)
				, y(startY)
				, z(startZ)
			{

			}

			Vector3D(std::vector<type>& inVector)
				: x(type(0.0))
				, y(type(0.0))
				, z(type(0.0))
			{
				size_t size = inVector.size();
				switch (size)
				{
				case 3:
					z = inVector[2];
				case 2:
					y = inVector[1];
				case 1:
					x = inVector[0];
				break;
				}
			}

			Vector3D(glm::vec3 vector) : x(vector.x), y(vector.y), z(vector.z) { ; }

			~Vector3D() = default;

			operator glm::vec3() const
			{
				return glm::vec3(x, y, z);
			}

			// ----------------------------------------------------------------------------------------------

			// Single Operator overloads
			Vector3D operator +(const Vector3D& other)
			{
				return Vector3D(x + other.x, y + other.y, z + other.z);
			}

			Vector3D operator -(const Vector3D& other)
			{
				return Vector3D(x - other.x, y - other.y, z - other.z);
			}

			Vector3D operator *(const double& other)
			{
				return Vector3D(x * (type)other, y * (type)other, z * (type)other);
			}

			Vector3D operator /(const double& other)
			{
				return Vector3D(x / other, y / other, z / other);
			}

			Vector3D operator /(const type& other)
			{
				return Vector3D(x / other, y / other, z / other);
			}

			// ----------------------------------------------------------------------------------------------

			// Double Operator overloads
			Vector3D& operator ++()
			{
				x++;
				y++;
				z++;

				return *this;
			}

			Vector3D& operator --()
			{
				x--;
				y--;
				z--;

				return *this;
			}

			Vector3D& operator +=(const Vector3D& other)
			{
				x += other.x;
				y += other.y;
				z += other.z;

				return *this;
			}

			Vector3D& operator -=(const Vector3D& other)
			{
				x -= other.x;
				y -= other.y;
				z -= other.z;

				return *this;
			}

			Vector3D& operator *=(const double& other)
			{
				x *= other;
				y *= other;
				z *= other;

				return *this;
			}

			Vector3D& operator /=(const double& other)
			{
				x /= other;
				y /= other;
				z /= other;

				return *this;
			}

			bool operator ==(const Vector3D& other)
			{
				bool equal = (x == other.x) && (y == other.y) && (z == other.z);

				return equal;
			}

			type& operator[](const unsigned int index)
			{
				if (index == 0)
					return x;

				if (index == 1)
					return y;

				return z;
			}

			// ----------------------------------------------------------------------------------------------

			// Mathematical useful functions

			type Dot(Vector3D& other)
			{
				return (x * other.x) + (y * other.y) + (z * other.z);
			}

			Vector3D Cross(Vector3D& other)
			{
				return Vector3D((y * other.z) - (z * other.y),
					            (z * other.x) - (x * other.z),
					            (x * other.y) - (y * other.x));
			}

			Vector3D Normalised()
			{
				type magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				return Vector3D(x / magnitude, y / magnitude, z / magnitude);
			}

			Vector3D& Normalise()
			{
				type magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				x /= magnitude;
				y /= magnitude;
				z /= magnitude;

				return *this;
			}

			type Magnitude()
			{
				return sqrt((x * x) + (y * y) + (z * z));
			}

			type MagnitudeSquared()
			{
				return (x * x) + (y * y) + (z * z);
			}

			// This performs the calculation in the order of: result = matrix * this
			Vector3D<type> MultipyByMatrix(glm::mat3 matrix)
			{
				// Cast to glm vector
				glm::vec3 glmVec = *this;

				return matrix * glmVec;
			}

			// ----------------------------------------------------------------------------------------------

			std::string to_string()
			{
				std::string returnValue = std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
				return returnValue;
			}

			// ----------------------------------------------------------------------------------------------

			type x;
			type y;
			type z;
		};


		// ==============================================================
		// --------------------------------------------------------------


		template <typename type>
		class Vector4D final
		{
		public:
			Vector4D() 
				: x(type(0.0))
				, y(type(0.0))
				, z(type(0.0))
				, w(type(0.0))
			{

			}

			Vector4D(type fillWith)
				: x(fillWith)
				, y(fillWith)
				, z(fillWith)
				, w(fillWith)
			{

			}

			Vector4D(type startX, type startY, type startZ, type startW) 
				: x(startX)
				, y(startY)
				, z(startZ)
				, w(startW)
			{

			}

			Vector4D(glm::vec3 vector, type newW) : x(vector.x), y(vector.y), z(vector.z), w(newW)     { ; }
			Vector4D(glm::vec4 vector)            : x(vector.x), y(vector.y), z(vector.z), w(vector.w) { ; }

			Vector4D(std::vector<type>& inVector)
				: x(type(0.0))
				, y(type(0.0))
				, z(type(0.0))
				, w(type(0.0))
			{
				unsigned int size = (unsigned int)inVector.size();
				switch (size)
				{
				case 4:
					w = inVector[3];
				case 3:
					z = inVector[2];
				case 2:
					y = inVector[1];
				case 1:
					x = inVector[0];
				break;
				}
			}

			~Vector4D() = default;

			operator glm::vec4() const
			{
				return glm::vec4(x, y, z, w);
			}

			operator glm::quat() const
			{
				return glm::quat(w, x, y, z);
			}

			bool operator ==(const Vector4D& other)
			{
				bool equal = (x == other.x) && (y == other.y) && (z == other.z) && (w == other.w);

				return equal;
			}

			// ----------------------------------------------------------------------------------------------

			// Single Operator overloads
			Vector4D operator +(const Vector4D& other)
			{
				return Vector4D(x + other.x, y + other.y, z + other.z, w + other.w);
			}

			Vector4D operator -(const Vector4D& other)
			{
				return Vector4D(x - other.x, y - other.y, z - other.z, w - other.w);
			}

			Vector4D operator *(const double& other)
			{
				return Vector4D((type)(x * other), (type)(y * other), (type)(z * other), (type)(w * other));
			}

			Vector4D operator /(const double& other)
			{
				return Vector4D(x / other, y / other, z / other, w / other);
			}

			Vector4D operator /(const type& other)
			{
				return Vector4D(x / other, y / other, z / other, w / other);
			}

			// ----------------------------------------------------------------------------------------------

			// Double Operator overloads
			Vector4D& operator ++()
			{
				x++;
				y++;
				z++;
				w++;

				return *this;
			}

			Vector4D& operator --()
			{
				x--;
				y--;
				z--;
				w--;

				return *this;
			}

			Vector4D& operator +=(const Vector4D& other)
			{
				x += other.x;
				y += other.y;
				z += other.z;
				w += other.w;

				return *this;
			}

			Vector4D& operator -=(const Vector4D& other)
			{
				x -= other.x;
				y -= other.y;
				z -= other.z;
				w -= other.w;

				return *this;
			}

			Vector4D& operator *=(const double& other)
			{
				x *= other;
				y *= other;
				z *= other;
				w *= other;

				return *this;
			}

			Vector4D& operator /=(const double& other)
			{
				x /= other;
				y /= other;
				z /= other;
				w /= other;

				return *this;
			}

			// ----------------------------------------------------------------------------------------------

			// Mathematical useful functions

			double Dot(Vector4D& other)
			{
				return (x * other.x) + (y * other.y) + (z * other.z) + (z * other.w);
			}

			Vector4D Normalised()
			{
				double magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				return Vector4D(x / magnitude, y / magnitude, z / magnitude, w / magnitude);
			}

			Vector4D& Normalise()
			{
				double magnitude = Magnitude();

				if (magnitude == 0.0f)
					return *this;

				x /= (type)magnitude;
				y /= (type)magnitude;
				z /= (type)magnitude;
				w /= (type)magnitude;

				return *this;
			}

			double Magnitude()
			{
				return sqrt((x * x) + (y * y) + (z * z) + (w * w));
			}

			double MagnitudeSquared()
			{
				return (x * x) + (y * y) + (z * z) + (w * w);
			}

			static Vector4D GetVectorFromString(std::string stringFormat)
			{
				Vector4D returnValue;

				unsigned int numberOn = 0;
				std::string numbers[4];

				// Search through the string for commas
				for (unsigned int i = 0; i < stringFormat.length(); i++)
				{
					// See if we should move onto the next number in the list
					if (stringFormat[i] == ',')
					{
						// Just take the first four numbers if there are more than four
						if (numberOn == 3)
							break;

						numberOn++;
						continue;
					}
					else
					{
						numbers[numberOn] += stringFormat[i];
					}
				}

				// Now convert the strings to numbers
				returnValue.x = std::stof(numbers[0]);
				returnValue.y = std::stof(numbers[1]);
				returnValue.z = std::stof(numbers[2]);
				returnValue.w = std::stof(numbers[3]);

				return returnValue;
			}

			// ----------------------------------------------------------------------------------------------

			std::string to_string()
			{
				std::string returnValue = std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w);
				return returnValue;
			}

			// ----------------------------------------------------------------------------------------------

			type x;
			type y;
			type z;
			type w;
		};

		static double Dot(Vector::Vector2D<float> a, Vector::Vector2D<float> b)
		{
			return a.Dot(b);
		}

		static double Dot(Vector::Vector3D<float> a, Vector::Vector3D<float> b)
		{
			return a.Dot(b);
		}

		static double Dot(Vector::Vector4D<float> a, Vector::Vector4D<float> b)
		{
			return a.Dot(b);
		}

		// ---------------------------

		static Vector::Vector3D<float> ConvertQuaternionToEuler(Vector::Vector4D<float> quat)
		{
			Vector::Vector3D<float> result = glm::eulerAngles((glm::quat)quat);

			return result;
		}

		// ------------------------

		static Vector::Vector4D<float> ConvertEulerToQuaternion(Vector::Vector3D<float> euler)
		{
			glm::quat result = glm::quat((glm::vec3)euler);

			return Vector::Vector4D<float>(result.x, result.y, result.z, result.w);
		}

		// ------------------------

		static float ToRadians(float input)
		{
			return glm::radians(input);
		}

		// ------------------------

		static Vector2D<float> ToRadians(Vector::Vector2D<float> input)
		{
			return Vector2D<float>(ToRadians(input.x), ToRadians(input.y));
		}

		// ------------------------

		static Vector3D<float> ToRadians(Vector::Vector3D<float> input)
		{
			return Vector3D<float>(ToRadians(input.x), ToRadians(input.y), ToRadians(input.z));
		}

		// ------------------------

		static Vector4D<float> ToRadians(Vector::Vector4D<float> input)
		{
			return Vector4D<float>(ToRadians(input.x), ToRadians(input.y), ToRadians(input.z), ToRadians(input.w));
		}

		// ------------------------
	}
}

// ==============================================================
// --------------------------------------------------------------

#endif