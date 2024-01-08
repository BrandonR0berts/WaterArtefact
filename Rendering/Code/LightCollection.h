#pragma once

#include "Maths/Code/Vector.h"

#include "Buffers.h"

namespace Rendering
{
	// ---------------

	struct DirectionalLight
	{
		DirectionalLight(Maths::Vector::Vector3D<float> colour, Maths::Vector::Vector3D<float> direction, Maths::Vector::Vector3D<float> position)
			: mColour(colour)
			, mDirection(direction.Normalised())
			, mPosition(position)
		{

		}

		DirectionalLight(const DirectionalLight& other)
			: mColour(other.mColour)
			, mDirection(other.mDirection)
			, mPosition(other.mPosition)
		{
		}

		Maths::Vector::Vector3D<float> mColour;
		Maths::Vector::Vector3D<float> mDirection;
		Maths::Vector::Vector3D<float> mPosition;
	};

	// ---------------

	struct PointLight
	{
		PointLight(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector3D<float> colour, float intensity)
			: mPosition(position)
			, mColour(colour)
			, mIntensity(intensity)
		{

		}

		Maths::Vector::Vector3D<float> mPosition;
		Maths::Vector::Vector3D<float> mColour;
		float mIntensity;
	};

	// ------------------------------------

	class LightCollection
	{
	public:
		LightCollection();
		~LightCollection();

		void AddDirectionalLight(Maths::Vector::Vector3D<float> direction, Maths::Vector::Vector3D<float> colour, Maths::Vector::Vector3D<float> position);
		void AddPointLight(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector3D<float> colour, float intensity);

		void RemoveLight(Maths::Vector::Vector3D<float> dataToCheck, bool directional);
		void RemoveLight(unsigned int index, bool directional); // Use this one as the once above is not 100%

		void ClearAllLights();

		void UpdateLightBufferData();
		void Update(const float deltaTime);


		std::vector<PointLight>&       GetAllPointLights()       { return mPointLights;       }
		std::vector<DirectionalLight>& GetAllDirectionalLights() { return mDirectionalLights; }

#ifdef _DEBUG_BUILD
		void RenderImGuiData();
#endif

		LightCollection& operator+=(const LightCollection& other)
		{
			unsigned int directionLightCount = (unsigned int)other.mDirectionalLights.size();
			for (unsigned int i = 0; i < directionLightCount; i++)
			{
				mDirectionalLights.push_back(other.mDirectionalLights[i]);
			}

			unsigned int pointLightsSize = (unsigned int)other.mPointLights.size();
			for (unsigned int i = 0; i < pointLightsSize; i++)
			{
				mPointLights.push_back(other.mPointLights[i]);
			}

			return *this;
		}

		unsigned int size();

	private:
		std::vector<PointLight>       mPointLights;
		std::vector<DirectionalLight> mDirectionalLights;

		Rendering::Buffers::ShaderStorageBufferObject* mLightSSBO;

		// Sizes in floats (4 bytes)
		static const unsigned int SizeOfPointLightOnGPU       = 7; // 2 vec3's and 1 float
		static const unsigned int SizeOfDirectionalLightOnGPU = 6; // 2 vec3's
	};

	// ------------------------------------
}