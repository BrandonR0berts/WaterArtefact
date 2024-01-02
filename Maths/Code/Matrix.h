#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "Vector.h"

#include "Include/glm/mat2x2.hpp"
#include "Include/glm/mat3x3.hpp"
#include "Include/glm/mat4x4.hpp"

namespace Maths
{
	namespace Matrix
	{
		// --------------------------------------------------------

		class Matrix2X2 final
		{
		public:

			// --------------------------------------------------------

			// Default to identity matrix
			Matrix2X2() : mData{ 1.0f, 0.0f,
								 0.0f, 1.0f }
			{ }

			~Matrix2X2() {}

			// --------------------------------------------------------

			// Getters
			Vector::Vector2D<float>& GetRow(unsigned int row)
			{
				return mVectorVersion[row];
			}

			float& operator[] (unsigned int index)
			{
				return mData[index];
			}

			// --------------------------------------------------------

			void scaleX(float scale)
			{
				mData[0] *= scale;
			}

			void scaleY(float scale)
			{
				mData[3] *= scale;
			}

			void scale(Vector::Vector2D<float> scale)
			{
				mData[0] *= scale.x;
				mData[3] *= scale.y;
			}

			void SetScale(float x, float y)
			{
				mData[0] = x;
				mData[3] = y;
			}

			void SetScale(Vector::Vector2D<float> newScale)
			{
				mData[0] = newScale.x;
				mData[3] = newScale.y;
			}

			// --------------------------------------------------------

			void RotateAroundXAxis(float radians)
			{

			}

			void RotateAroundYAxis(float radians)
			{

			}

			void RotateAroundGeneralAxis(float radians, Vector::Vector2D<float> axis)
			{

			}

			// --------------------------------------------------------

			void SetFromArray(float(&data)[4])
			{
				for (unsigned int i = 0; i < 4; i++)
				{
					mData[i] = data[i];
				}
			}

			void SetFromArray(glm::mat2 matrix)
			{
				for (unsigned int i = 0; i < 4; ++i)
				{
					mData[i] = matrix[i / 2][i % 2];
				}
			}

			// --------------------------------------------------------

			// Common maths functions
			float Det()
			{
				return (mData[0] * mData[3]) - (mData[1] * mData[2]);
			}

			Matrix2X2& MultiplyBy(Matrix::Matrix2X2& other)
			{
				float tempData[4] = { mData[0], mData[1], mData[2], mData[3] };

				mData[0] = (tempData[0] * other.mData[0]) + (tempData[1] * other.mData[2]);
				mData[1] = (tempData[0] * other.mData[1]) + (tempData[1] * other.mData[3]);
				mData[2] = (tempData[2] * other.mData[0]) + (tempData[3] * other.mData[2]);
				mData[3] = (tempData[2] * other.mData[1]) + (tempData[3] * other.mData[3]);

				return *this;
			}

			// Multiplies all elements within the matrix by the factor
			Matrix2X2& MultiplyBy(float other)
			{
				for (unsigned int i = 0; i < 4; i++)
				{
					mData[i] *= other;
				}

				return *this;
			}

			Matrix2X2& operator *(Matrix::Matrix2X2& other)
			{
				return MultiplyBy(other);
			}

			Matrix2X2& operator *(float other)
			{
				return MultiplyBy(other);
			}

			Matrix2X2& operator *=(Matrix::Matrix2X2& other)
			{
				return MultiplyBy(other);
			}

			Matrix2X2& operator *=(float other)
			{
				return MultiplyBy(other);
			}

			// Divides all elements in the matrix by the factor
			Matrix2X2& DivideBy(float other)
			{
				for (unsigned int i = 0; i < 4; i++)
				{
					mData[i] /= other;
				}

				return *this;
			}

			Matrix2X2& operator /(float other)
			{
				return DivideBy(other);
			}

			// Returns a new matrix that is this matrix but transposed
			Matrix2X2 Transposed()
			{
				Matrix2X2 newMatrix;

				// Diagional is the same
				newMatrix[0] = mData[0];
				newMatrix[3] = mData[3];

				// Edges are flipped
				newMatrix[1] = mData[2];
				newMatrix[2] = mData[1];

				return newMatrix;
			}

			// Tranposes this matrix and returns it back
			Matrix2X2& Transpose()
			{
				float temp = mData[1];
				mData[1] = mData[2];
				mData[2] = temp;

				return *this;
			}

			// --------------------------------------------------------

		private:
			union
			{
				// data[0, 1] = row 1. 
				// data[2, 3] = row 2
				float mData[4];

				Vector::Vector2D<float> mVectorVersion[2];
			};
		};

		// --------------------------------------------------------

		class Matrix3X3 final
		{
		public:

			// --------------------------------------------------------

			// Default to identity matrix
			Matrix3X3() : mData{ 1.0f, 0.0f, 0.0f,
								 0.0f, 1.0f, 0.0f,
								 0.0f, 0.0f, 1.0f }
			{ }

			Matrix3X3(float data[9])
			{
				for (unsigned int i = 0; i < 9; i++)
				{
					mData[i] = data[i];
				}
			}

			~Matrix3X3() {}

			// --------------------------------------------------------

			// Getters
			Vector::Vector3D<float>& GetRow(unsigned int row)
			{
				return mVectorVersion[row];
			}

			float& operator[] (unsigned int index)
			{
				return mData[index];
			}

			// --------------------------------------------------------

			// Setters
			void transformX(float distance)
			{
				mData[6] += distance;
			}

			void transformY(float distance)
			{
				mData[7] += distance;
			}

			void transformZ(float distance)
			{
				mData[8] += distance;
			}

			void transform(Vector::Vector3D<float> transform)
			{
				mData[6] += transform.x;
				mData[7] += transform.x;
				mData[8] += transform.x;
			}

			void SetTransform(float x, float y, float z)
			{
				mData[6] = x;
				mData[7] = y;
				mData[8] = z;
			}

			void SetTransform(float x, float y)
			{
				mData[6] = x;
				mData[7] = y;
			}

			void SetTransform(Vector::Vector3D<float> newPosition)
			{
				mData[6] = newPosition.x;
				mData[7] = newPosition.y;
				mData[8] = newPosition.z;
			}

			// --------------------------------------------------------

			void scaleX(float scale)
			{
				mData[0] *= scale;
			}

			void scaleY(float scale)
			{
				mData[4] *= scale;
			}

			void scaleZ(float scale)
			{
				mData[8] *= scale;
			}

			void scale(Vector::Vector3D<float> scale)
			{
				mData[0] *= scale.x;
				mData[4] *= scale.y;
				mData[8] *= scale.z;
			}

			void SetScale(float x, float y, float z)
			{
				mData[0] = x;
				mData[4] = y;
				mData[8] = z;
			}

			void SetScale(Vector::Vector3D<float> newScale)
			{
				mData[0] = newScale.x;
				mData[4] = newScale.y;
				mData[8] = newScale.z;
			}

			// --------------------------------------------------------

			void RotateAroundXAxis(float radians)
			{

			}

			void RotateAroundYAxis(float radians)
			{

			}

			void RotateAroundZAxis(float radians)
			{

			}

			void RotateAroundGeneralAxis(float radians, Vector::Vector3D<float> axis)
			{

			}

			// --------------------------------------------------------

			void SetFromArray(float(&data)[9])
			{
				for (unsigned int i = 0; i < 9; i++)
				{
					mData[i] = data[i];
				}
			}

			void SetFromArray(glm::mat3 matrix)
			{
				for (unsigned int i = 0; i < 9; ++i)
				{
					mData[i] = matrix[i / 3][i % 3];
				}
			}

			// --------------------------------------------------------

			// Common maths functions
			float Det()
			{
				return 0;
			}

			// multiply this matrix by another one
			Matrix3X3& MultiplyBy(Matrix::Matrix3X3& other)
			{
				Matrix3X3 copy = *this;

				mData[0] = (copy.mData[0] * other.mData[0]) + (copy.mData[1] * other.mData[3]) + (copy.mData[2] * other.mData[6]);
				mData[1] = (copy.mData[0] * other.mData[1]) + (copy.mData[1] * other.mData[4]) + (copy.mData[2] * other.mData[7]);
				mData[2] = (copy.mData[0] * other.mData[2]) + (copy.mData[1] * other.mData[5]) + (copy.mData[2] * other.mData[8]);

				mData[3] = (copy.mData[3] * other.mData[0]) + (copy.mData[4] * other.mData[3]) + (copy.mData[5] * other.mData[6]);
				mData[4] = (copy.mData[3] * other.mData[1]) + (copy.mData[4] * other.mData[4]) + (copy.mData[5] * other.mData[7]);
				mData[5] = (copy.mData[3] * other.mData[2]) + (copy.mData[4] * other.mData[5]) + (copy.mData[5] * other.mData[8]);

				mData[6] = (copy.mData[6] * other.mData[0]) + (copy.mData[7] * other.mData[3]) + (copy.mData[8] * other.mData[6]);
				mData[7] = (copy.mData[6] * other.mData[1]) + (copy.mData[7] * other.mData[4]) + (copy.mData[8] * other.mData[7]);
				mData[8] = (copy.mData[6] * other.mData[2]) + (copy.mData[7] * other.mData[5]) + (copy.mData[8] * other.mData[8]);

				return *this;
			}

			// Multiply every element in this matrix by the factor
			Matrix3X3& MultiplyBy(float other)
			{
				mData[0] = mData[0] * other;
				mData[1] = mData[1] * other;
				mData[2] = mData[2] * other;

				mData[3] = mData[3] * other;
				mData[4] = mData[4] * other;
				mData[5] = mData[5] * other;

				mData[6] = mData[6] * other;
				mData[7] = mData[7] * other;
				mData[8] = mData[8] * other;

				return *this;
			}

			Maths::Vector::Vector3D<float> MultiplyBy(Maths::Vector::Vector3D<float>& other)
			{
				Maths::Vector::Vector3D<float> result;

				result.x = (mData[0] * other.x) + (mData[1] * other.y) + (mData[2] * other.z);
				result.y = (mData[3] * other.x) + (mData[4] * other.y) + (mData[5] * other.z);
				result.z = (mData[6] * other.x) + (mData[7] * other.y) + (mData[8] * other.z);

				return result;
			}

			Matrix3X3& operator *(Matrix::Matrix3X3& other)
			{
				return MultiplyBy(other);
			}

			Matrix3X3& operator *(float other)
			{
				return MultiplyBy(other);
			}

			Matrix3X3& operator *=(Matrix::Matrix3X3& other)
			{
				return MultiplyBy(other);
			}

			Matrix3X3& operator *=(float other)
			{
				return MultiplyBy(other);
			}

			Matrix3X3& DivideBy(float other)
			{
				return *this;
			}

			Matrix3X3& operator /(float other)
			{
				return DivideBy(other);
			}

			// Returns a new matrix that is this matrix but transposed
			Matrix3X3 Transposed()
			{
				return Matrix3X3();
			}

			// Tranposes this matrix and returns it back
			Matrix3X3& Transpose()
			{


				return *this;
			}

			// --------------------------------------------------------

		private:
			union
			{
				// data[0, 1, 2] = row 1. 
				// data[3, 4, 5] = row 2
				// data[6, 7, 8] = row 3
				float mData[9];

				Vector::Vector3D<float> mVectorVersion[3];
			};
		};

		// ----------------------------------------------------

		class Matrix4X4 final
		{
		public:

			// --------------------------------------------------------

			// Default to identity matrix
			Matrix4X4() : mData{ 1.0f, 0.0f, 0.0f, 0.0f,
								 0.0f, 1.0f, 0.0f, 0.0f,
								 0.0f, 0.0f, 1.0f, 0.0f,
								 0.0f, 0.0f, 0.0f, 1.0f }
			{ }

			Matrix4X4(glm::mat4& other)
			{
				for (unsigned int i = 0; i < 16; i++)
				{
					mData[i] = other[i / 4][i % 4];
				}
			}

			Matrix4X4(float data[16])
			{
				for (unsigned int i = 0; i < 16; i++)
				{
					mData[i] = data[i];
				}
			}

			~Matrix4X4() {}

			// --------------------------------------------------------

			// Getters
			Vector::Vector4D<float>& GetRow(unsigned int row)
			{
				return mVectorVersion[row];
			}

			float& operator[] (unsigned int index)
			{
				return mData[index];
			}

			// --------------------------------------------------------

			// Setters
			void transformX(float distance)
			{
				//mData[3] += distance;
				mData[12] += distance;
			}

			void transformY(float distance)
			{
				//mData[7] += distance;
				mData[13] += distance;
			}

			void transformZ(float distance)
			{
				//mData[11] += distance;
				mData[14] += distance;
			}

			void transform(Vector::Vector3D<float> transform)
			{
				mData[12] += transform.x;
				mData[13] += transform.y;
				mData[14] += transform.z;
			}

			void SetTransform(float x, float y, float z)
			{
				mData[12] = x;
				mData[13] = y;
				mData[14] = z;
			}

			void SetTransform(float x, float y)
			{
				mData[12] = x;
				mData[13] = y;
			}

			void SetTransform(Vector::Vector3D<float> newPosition)
			{
				mData[12] = newPosition.x;
				mData[13] = newPosition.y;
				mData[14] = newPosition.z;
			}

			// --------------------------------------------------------

			void scaleX(float scale)
			{
				mData[0] *= scale;
			}

			void scaleY(float scale)
			{
				mData[5] *= scale;
			}

			void scaleZ(float scale)
			{
				mData[10] *= scale;
			}

			void scale(Vector::Vector3D<float> scale)
			{
				mData[0] *= scale.x;
				mData[5] *= scale.y;
				mData[10] *= scale.z;
			}

			void SetScale(float x, float y, float z)
			{
				mData[0] = x;
				mData[5] = y;
				mData[10] = z;
			}

			void SetScale(Vector::Vector3D<float> newScale)
			{
				mData[0] = newScale.x;
				mData[5] = newScale.y;
				mData[10] = newScale.z;
			}

			// --------------------------------------------------------

			void RotateAroundXAxis(float radians)
			{

			}

			void RotateAroundYAxis(float radians)
			{

			}

			void RotateAroundZAxis(float radians)
			{

			}

			void RotateAroundGeneralAxis(float radians, Vector::Vector3D<float> axis)
			{

			}

			// --------------------------------------------------------

			void SetFromArray(float(&data)[16])
			{
				for (unsigned int i = 0; i < 16; i++)
				{
					mData[i] = data[i];
				}
			}

			void SetFromArray(glm::mat4 matrix)
			{
				for (unsigned int i = 0; i < 16; i++)
				{
					mData[i] = matrix[i / 4][i % 4];
				}
			}

			// --------------------------------------------------------

			// Common maths functions
			float Det()
			{
				return 0;
			}

			Matrix4X4& MultiplyBy(Matrix::Matrix4X4& other)
			{
				Matrix4X4 copy = *this;

				// Top row multipled by each column in turn
				mData[0] = (copy[0] * other[0]) + (copy[1] * other[4]) + (copy[2] * other[8])  + (copy[3] * other[12]);
				mData[1] = (copy[0] * other[1]) + (copy[1] * other[5]) + (copy[2] * other[9])  + (copy[3] * other[13]);
				mData[2] = (copy[0] * other[2]) + (copy[1] * other[6]) + (copy[2] * other[10]) + (copy[3] * other[14]);
				mData[3] = (copy[0] * other[3]) + (copy[1] * other[7]) + (copy[2] * other[11]) + (copy[3] * other[15]);

				// Second row multiplied by each column in turn
				mData[4] = (copy[4] * other[0]) + (copy[5] * other[4]) + (copy[6] * other[8])  + (copy[7] * other[12]);
				mData[5] = (copy[4] * other[1]) + (copy[5] * other[5]) + (copy[6] * other[9])  + (copy[7] * other[13]);
				mData[6] = (copy[4] * other[2]) + (copy[5] * other[6]) + (copy[6] * other[10]) + (copy[7] * other[14]);
				mData[7] = (copy[4] * other[3]) + (copy[5] * other[7]) + (copy[6] * other[11]) + (copy[7] * other[15]);

				// Third row multiplied by each column in turn
				mData[8]  = (copy[8] * other[0]) + (copy[9] * other[4]) + (copy[10] * other[8])  + (copy[11] * other[12]);
				mData[9]  = (copy[8] * other[1]) + (copy[9] * other[5]) + (copy[10] * other[9])  + (copy[11] * other[13]);
				mData[10] = (copy[8] * other[2]) + (copy[9] * other[6]) + (copy[10] * other[10]) + (copy[11] * other[14]);
				mData[11] = (copy[8] * other[3]) + (copy[9] * other[7]) + (copy[10] * other[11]) + (copy[11] * other[15]);

				// Fourth row multiplied by each column in turn
				mData[12] = (copy[12] * other[0]) + (copy[13] * other[4]) + (copy[14] * other[8])  + (copy[15] * other[12]);
				mData[13] = (copy[12] * other[1]) + (copy[13] * other[5]) + (copy[14] * other[9])  + (copy[15] * other[13]);
				mData[14] = (copy[12] * other[2]) + (copy[13] * other[6]) + (copy[14] * other[10]) + (copy[15] * other[14]);
				mData[15] = (copy[12] * other[3]) + (copy[13] * other[7]) + (copy[14] * other[11]) + (copy[15] * other[15]);

				return *this;
			}

			Matrix4X4& MultiplyBy(glm::mat4& other)
			{
				Matrix4X4 copy = *this;

				// Top row multipled by each column in turn
				mData[0] = (copy[0] * other[0][0]) + (copy[1] * other[1][0]) + (copy[2] * other[2][0])  + (copy[3] * other[3][0]);
				mData[1] = (copy[0] * other[0][1]) + (copy[1] * other[1][1]) + (copy[2] * other[2][1])  + (copy[3] * other[3][1]);
				mData[2] = (copy[0] * other[0][2]) + (copy[1] * other[1][2]) + (copy[2] * other[2][2]) + (copy[3] * other[3][2]);
				mData[3] = (copy[0] * other[0][3]) + (copy[1] * other[1][3]) + (copy[2] * other[2][3]) + (copy[3] * other[3][3]);

				// Second row multiplied by each column in turn
				mData[4] = (copy[4] * other[0][0]) + (copy[5] * other[1][0]) + (copy[6] * other[2][0])  + (copy[7] * other[3][0]);
				mData[5] = (copy[4] * other[0][1]) + (copy[5] * other[1][1]) + (copy[6] * other[2][1])  + (copy[7] * other[3][1]);
				mData[6] = (copy[4] * other[0][2]) + (copy[5] * other[1][2]) + (copy[6] * other[2][2]) + (copy[7] * other[3][2]);
				mData[7] = (copy[4] * other[0][3]) + (copy[5] * other[1][3]) + (copy[6] * other[2][3]) + (copy[7] * other[3][3]);

				// Third row multiplied by each column in turn
				mData[8]  = (copy[8] * other[0][0]) + (copy[9] * other[1][0]) + (copy[10] * other[2][0])  + (copy[11] * other[3][0]);
				mData[9]  = (copy[8] * other[0][1]) + (copy[9] * other[1][1]) + (copy[10] * other[2][1])  + (copy[11] * other[3][1]);
				mData[10] = (copy[8] * other[0][2]) + (copy[9] * other[1][2]) + (copy[10] * other[2][2]) + (copy[11] * other[3][2]);
				mData[11] = (copy[8] * other[0][3]) + (copy[9] * other[1][3]) + (copy[10] * other[2][3]) + (copy[11] * other[3][3]);

				// Fourth row multiplied by each column in turn
				mData[12] = (copy[12] * other[0][0]) + (copy[13] * other[1][0]) + (copy[14] * other[2][0])  + (copy[15] * other[3][0]);
				mData[13] = (copy[12] * other[0][1]) + (copy[13] * other[1][1]) + (copy[14] * other[2][1])  + (copy[15] * other[3][1]);
				mData[14] = (copy[12] * other[0][2]) + (copy[13] * other[1][2]) + (copy[14] * other[2][2]) + (copy[15] * other[3][2]);
				mData[15] = (copy[12] * other[0][3]) + (copy[13] * other[1][3]) + (copy[14] * other[2][3]) + (copy[15] * other[3][3]);

				return *this;
			}

			Matrix4X4& MultiplyBy(float other)
			{

			}

			Matrix4X4& operator *(Matrix::Matrix4X4& other)
			{
				return MultiplyBy(other);
			}

			Vector::Vector4D<float> operator *(Vector::Vector4D<float> other)
			{
				Vector::Vector4D<float> returnValue;

				returnValue.x = (mData[0]  * other.x) + (mData[1]  * other.y) + (mData[2]  * other.z) + (mData[3]  * other.w);
				returnValue.y = (mData[4]  * other.x) + (mData[5]  * other.y) + (mData[6]  * other.z) + (mData[7]  * other.w);
				returnValue.z = (mData[8]  * other.x) + (mData[9]  * other.y) + (mData[10] * other.z) + (mData[11] * other.w);
				returnValue.w = (mData[12] * other.x) + (mData[13] * other.y) + (mData[14] * other.z) + (mData[15] * other.w);

				return returnValue;
			}

			Matrix4X4& operator *(float other)
			{
				return MultiplyBy(other);
			}

			Matrix4X4& operator *=(Matrix::Matrix4X4& other)
			{
				return MultiplyBy(other);
			}

			Matrix4X4& operator *=(float other)
			{
				return MultiplyBy(other);
			}

			Matrix4X4& DivideBy(float other)
			{
				return *this;
			}

			Matrix4X4& operator /(float other)
			{
				return DivideBy(other);
			}

			// Returns a new matrix that is this matrix but transposed
			Matrix4X4 Transposed()
			{
				return Matrix4X4();
			}

			// Tranposes this matrix and returns it back
			Matrix4X4& Transpose()
			{
				Matrix4X4 copy = *this;

				mData[1] = copy[4];
				mData[2] = copy[8];
				mData[3] = copy[12];

				mData[4]  = copy[1];
				mData[8]  = copy[2];
				mData[12] = copy[3];

				mData[6]  = copy[9];
				mData[7]  = copy[13];

				mData[9]  = copy[6];
				mData[13] = copy[7];

				mData[11] = copy[14];

				mData[14] = copy[11];

				return *this;
			}

			// --------------------------------------------------------

			Vector::Vector4D<float> GetColumn(unsigned int index)
			{
				Vector::Vector4D<float> returnValue = Vector::Vector4D<float>();

				switch (index)
				{
				default:
				case 0:
					returnValue.x = mData[0];
					returnValue.y = mData[4];
					returnValue.z = mData[8];
					returnValue.w = mData[12];
				break;

				case 1:
					returnValue.x = mData[1];
					returnValue.y = mData[5];
					returnValue.z = mData[9];
					returnValue.w = mData[13];
				break;

				case 2:
					returnValue.x = mData[2];
					returnValue.y = mData[6];
					returnValue.z = mData[10];
					returnValue.w = mData[14];
				break;

				case 3:
					returnValue.x = mData[3];
					returnValue.y = mData[7];
					returnValue.z = mData[11];
					returnValue.w = mData[15];
				break;
				}

				return returnValue;
			}

			static glm::mat4 CreateModelMatrix(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector3D<float> scale, Maths::Vector::Vector3D<float> rotation)
			{
				glm::mat4 modelMatrix(1.0f);

				glm::quat quatCast       = Maths::Vector::ConvertEulerToQuaternion(Maths::Vector::ToRadians(rotation));
				glm::mat4 rotationMatrix = glm::mat4_cast(quatCast);

				modelMatrix              = glm::translate(modelMatrix, (glm::vec3)position);
				modelMatrix              = modelMatrix * rotationMatrix;
				modelMatrix              = glm::scale(modelMatrix, (glm::vec3)scale);

				return modelMatrix;
			}

			static glm::mat4 CreateModelMatrix(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector3D<float> scale, Maths::Vector::Vector4D<float> rotation)
			{
				glm::mat4 modelMatrix(1.0f);

				glm::quat quatCast       = (glm::quat)rotation;
				glm::mat4 rotationMatrix = glm::mat4_cast(quatCast);

				modelMatrix              = glm::translate(modelMatrix, (glm::vec3)position);
				modelMatrix              = modelMatrix * rotationMatrix;
				modelMatrix              = glm::scale(modelMatrix, (glm::vec3)scale);

				return modelMatrix;
			}


		private:
			union
			{
				// data[0,  1,  2,  3]  = row 1. 
				// data[4,  5,  6,  7]  = row 2
				// data[8,  9,  10, 11] = row 3
				// data[12, 13, 14, 15] = row 4
				float           mData[16];


				Vector::Vector4D<float> mVectorVersion[4];
			};
		};

		// ----------------------------------------------------

	};

	// --------------------------------------------------------------------------------------
};

#endif