#pragma once

#include "Maths/Code/Vector.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Rendering
{
	// -------------------------------------------------------------

	class Camera abstract
	{
	public:
		Camera(float nearDistance,
			   float farDistance,
			   float width,
			   float height,
			   float aspectRatio,
			   float FOV,
			   Maths::Vector::Vector3D<float> up,
			   Maths::Vector::Vector3D<float> forward,
			   Maths::Vector::Vector3D<float> position,
			   bool                           collideWithEnviroment);
		~Camera();

		virtual void      Update(const float deltaTime);

		// Matrix getter
		virtual glm::mat4 GetViewMatrix();
		virtual glm::mat4 GetPerspectiveMatrix();
		virtual glm::mat4 GetOrthoMatrix();

		float* GetIdentityMatrix() 
		{
			static float identity[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
					    			      0.0f, 1.0f, 0.0f, 0.0f,
								          0.0f, 0.0f, 1.0f, 0.0f,
								          0.0f, 0.0f, 0.0f, 1.0f };
			return identity;
		}	

		virtual bool                   GetPointIsInFrustrum(Maths::Vector::Vector3D<float> position);
		        float                  GetFOVAsRadians()        { return glm::radians(mFOV); }
				float                  GetFOV()           const { return mFOV; }

		Maths::Vector::Vector3D<float> GetPosition()      const { return mPosition; }
		Maths::Vector::Vector3D<float> GetForward()       const { return mForward; }
		Maths::Vector::Vector3D<float> GetUp()            const { return mUp; }

		float                          GetFarDistance()   const { return mFarDistance; }
		float                          GetNearDistance()  const { return mNearDistance; }

		float                          GetAspectRatio()   const { return mAspectRatio; }

		void                           SetPosition(Maths::Vector::Vector3D<float> newPosition);
		void                           SetForward(Maths::Vector::Vector3D<float> forward, Maths::Vector::Vector3D<float> right);
		void                           SetUp(Maths::Vector::Vector3D<float> newPosition);

		void                           SetFOV(float FOV);
		void                           SetFOVAsRadians(float FOV);

		void                           SetNearDistance(float distance);
		void                           SetFarDistance(float distance);
		void                           SetAspectRatio(float aspect);

		// Calculates the cube extent corners of the view frustum - this is far faster to use for visibility calculations than the actual view frustum
		std::vector<Maths::Vector::Vector3D<float>> GetRoughFrustumBox();

		void SetResolution(float width, float height);

	protected:
		float                          mNearDistance;
		float                          mFarDistance;

		float                          mWidth;
		float                          mHeight;

		float                          mAspectRatio;
		float                          mFOV;

		Maths::Vector::Vector3D<float> mUp;
		Maths::Vector::Vector3D<float> mStartUp;

		Maths::Vector::Vector3D<float> mRight;
		Maths::Vector::Vector3D<float> mStartRight;

		Maths::Vector::Vector3D<float> mStartForward;
		Maths::Vector::Vector3D<float> mForward;

		Maths::Vector::Vector3D<float> mPosition;

		glm::mat4                      mViewMatrix;

		float                          mMovementSpeed;

		bool                           mCollideWithEnviroment;
	};

	// -------------------------------------------------------------

	class DebugCamera final : public Camera
	{
	public:
		DebugCamera(float nearDistance,
			   float farDistance,
			   float width,
			   float height,
			   float aspectRatio,
			   float FOV,
			   Maths::Vector::Vector3D<float> up,
			   Maths::Vector::Vector3D<float> forward,
			   Maths::Vector::Vector3D<float> position) 
			: Camera(nearDistance, farDistance, width, height, aspectRatio, FOV, up, forward, position, false)
			, mAngleFromVertical(0.0f)
			, mAngleFromHorizonal(0.0f)
		{
		
		}

		~DebugCamera() {}

		void Update(const float deltaTime) override;

	private:
		void HandlePCInput(const float deltaTime);
		void HandleControllerInput(const float deltaTime);

		void UpdateLookDirection();

		float mAngleFromVertical;  // Angle for left/right looking
		float mAngleFromHorizonal; // Angle for up/down looking
	};

	// -------------------------------------------------------------

	class FirstPersonCamera final : public Camera
	{
	public:
		FirstPersonCamera();
		~FirstPersonCamera();

	private:
	};

	// -------------------------------------------------------------

	class ThirdPersonCamera final : public Camera
	{
	public:
		ThirdPersonCamera();
		~ThirdPersonCamera();

	private:
		Maths::Vector::Vector3D<float> mFocalPoint;
		float                          mDistanceToFocalPoint;
	};

	// -------------------------------------------------------------

	class OrthographicCamera final : public Camera
	{
	public:
		OrthographicCamera( float nearDistance,
							float farDistance,
							float width,
							float height,
							Maths::Vector::Vector3D<float> up,
							Maths::Vector::Vector3D<float> forward,
							Maths::Vector::Vector3D<float> position);
		~OrthographicCamera();

		glm::mat4 GetPerspectiveMatrix() override;

	private:
		float mAngleFromVertical;  // Angle for left/right looking
		float mAngleFromHorizonal; // Angle for up/down looking
	};

	// -------------------------------------------------------------

}