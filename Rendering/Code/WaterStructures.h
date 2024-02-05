#pragma once

#include "Maths/Code/Vector.h"

namespace Rendering
{
	struct SingleSineDataSet final
	{
		SingleSineDataSet()
			: mAmplitude(0.1f)
			, mSteepnessFactor(1.0f)
			, mWaveLength(9.0f)
			, mSpeedOfWave(10.0f)
			, mDirectionOfWave(1.0f, 0.0f)
			, padding1(0.0f)
			, padding2(0.0f)
		{

		}

		SingleSineDataSet(float amplitude, Maths::Vector::Vector2D<float> direction, float speed, float wavelength)
			: mAmplitude(amplitude)
			, mSteepnessFactor(1.0f)
			, mWaveLength(wavelength)
			, mSpeedOfWave(speed)
			, mDirectionOfWave(direction)
			, padding1(0.0f)
			, padding2(0.0f)
		{

		}

		float                          mAmplitude;
		float                          mSteepnessFactor;
		float                          mWaveLength;
		float                          mSpeedOfWave;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		float                          padding1;
		float                          padding2;
	};

	struct SingleGerstnerWaveData final
	{
		SingleGerstnerWaveData()
			: mAmplitude(0.3f)
			, mSteepness(0.3f)
			, mSpeedOfWave(6.0f)
			, mWaveLength(20.0f)
			, mDirectionOfWave(0.5f, 0.1f)
			, mPadding()
		{

		}

		SingleGerstnerWaveData(float amplitude, Maths::Vector::Vector2D<float> direction, float speed, float wavelength, float steepness)
			: mAmplitude(amplitude)
			, mSteepness(steepness)
			, mSpeedOfWave(speed)
			, mWaveLength(wavelength)
			, mDirectionOfWave(direction)
			, mPadding()
		{
		}

		float                          mAmplitude;
		float                          mSteepness;
		float                          mSpeedOfWave;
		float                          mWaveLength;
		Maths::Vector::Vector2D<float> mDirectionOfWave;
		Maths::Vector::Vector2D<float> mPadding;
	};

	struct TessendorfWaveData final
	{
		TessendorfWaveData()
			: mWindVelocity(15.0f, 0.1f)
			, mGravity(9.81f)
			, mRepeatAfterTime(10.0f)
			, mLxLz(1024.0f, 1024.0f)
			, mPhilipsConstant(0.2f)
		{

		}

		Maths::Vector::Vector2D<float> mWindVelocity;
		float                          mGravity;
		float                          mRepeatAfterTime;
		Maths::Vector::Vector2D<float> mLxLz;
		float                          mPhilipsConstant;
	};

	struct RenderingWaterData
	{
		RenderingWaterData()
			: mWaterColour(0.2431f, 0.337f, 0.7176f)
			, mAmbientColour(0.1f, 0.1f, 0.2f)
			, mLightDirection(0.1f, -1.0f, 0.1f)
			, mReflectionFactor(1.0f)
		{
			mLightDirection.Normalise();
		}

		Maths::Vector::Vector3D<float> mWaterColour;
		Maths::Vector::Vector3D<float> mAmbientColour;
		Maths::Vector::Vector3D<float> mLightDirection;
		float                          mReflectionFactor;
	};

	// ---------------------------------------

	enum class SimulationMethods
	{
		Sine,
		Gerstner,
		Tessendorf
	};

	enum class SineWavePresets : char
	{
		Calm,
		Chopppy,
		Strange
	};

	enum class GerstnerWavePresets : char
	{
		Calm,
		Chopppy,
		Strange
	};

	enum class TessendorfWavePresets : char
	{
		Calm1,
		Calm2,
		Calm3,

		Chopppy1,
		Chopppy2
	};
}