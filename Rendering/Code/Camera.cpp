#include "Camera.h"
#include "Window.h"

#include "Input/Code/Input.h"
#include "Input/Code/KeyboardInput.h"
#include "Input/Code/MouseInput.h"

#include "Maths/Code/Matrix.h"

#include <iostream>

namespace Rendering
{
	// -------------------------------------------------------------------- //

	Camera::Camera(float nearDistance,
		float farDistance,
		float width,
		float height,
		float aspectRatio,
		float FOV,
		Maths::Vector::Vector3D<float> up,
		Maths::Vector::Vector3D<float> forward,
		Maths::Vector::Vector3D<float> position,
		bool                           collideWithEnviroment)
		: mNearDistance(nearDistance)
		, mFarDistance(farDistance)
		, mWidth(width)
		, mHeight(height)
		, mAspectRatio(aspectRatio)
		, mFOV(FOV)
		, mUp(up)
		, mStartUp(up)
		, mStartRight()
		, mStartForward(forward)
		, mForward(forward)
		, mPosition(position)
		, mCollideWithEnviroment(collideWithEnviroment)
		, mViewMatrix()
		, mMovementSpeed(4.0f)
		, mRight()
	{
		// Initialise the view matrix
		GetViewMatrix();

		mRight      = mUp.Cross(mForward);
		mStartRight = mRight;
	}

	// -------------------------------------------------------------------- //

	Camera::~Camera()
	{

	}

	// -------------------------------------------------------------------- //

	void Camera::Update(const float deltaTime)
	{

	}

	// -------------------------------------------------------------------- //

	bool Camera::GetPointIsInFrustrum(Maths::Vector::Vector3D<float> position)
	{
		return false;
	}

	// -------------------------------------------------------------------- //

	glm::mat4 Camera::GetViewMatrix()
	{
		mViewMatrix = glm::lookAt((glm::vec3)mPosition, (glm::vec3)(mPosition + mForward), (glm::vec3)mUp);

		return mViewMatrix;
	}

	// -------------------------------------------------------------------- //

	glm::mat4 Camera::GetPerspectiveMatrix()
	{
		return glm::perspective(GetFOVAsRadians(), mAspectRatio, mNearDistance, mFarDistance);
	}

	// -------------------------------------------------------------------- //

	glm::mat4 Camera::GetOrthoMatrix()
	{
		float height = (float)Window::GetWindowHeight();
		float width  = (float)Window::GetWindowWidth();
		return glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f);
	}

	// -------------------------------------------------------------------- //

	std::vector<Maths::Vector::Vector3D<float>> Camera::GetRoughFrustumBox()
	{
		std::vector<Maths::Vector::Vector3D<float>> returnValue;
		returnValue.reserve(8);

		Maths::Vector::Vector3D<float> centreOfFarPlane  = mPosition + (mForward * mFarDistance);
		Maths::Vector::Vector3D<float> centreOfNearPlane = mPosition + (mForward * mNearDistance);

		// Far plane
		Maths::Vector::Vector3D<float> halfHeight = mUp    * (mHeight / 2.0f);
		Maths::Vector::Vector3D<float> halfWidth  = mRight * (mWidth  / 2.0f);

		returnValue.push_back(centreOfFarPlane + halfWidth + halfHeight); // Top right pos
		returnValue.push_back(centreOfFarPlane + halfWidth - halfHeight); // Bottom right pos

		returnValue.push_back(centreOfFarPlane - halfWidth + halfHeight); // Top left pos
		returnValue.push_back(centreOfFarPlane - halfWidth - halfHeight); // Bottom left pos

		// Near plane
		returnValue.push_back(centreOfNearPlane + halfWidth + halfHeight); // Top right pos
		returnValue.push_back(centreOfNearPlane + halfWidth - halfHeight); // Bottom right pos

		returnValue.push_back(centreOfNearPlane - halfWidth + halfHeight); // Top left pos
		returnValue.push_back(centreOfNearPlane - halfWidth - halfHeight); // Bottom left pos

		return returnValue;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetForward(Maths::Vector::Vector3D<float> forward, Maths::Vector::Vector3D<float> right)
	{
		mForward    = forward;
		mRight      = right;
		mUp         = glm::cross((glm::vec3)forward, (glm::vec3)right);
	}

	// -------------------------------------------------------------------- //

	void Camera::SetResolution(float width, float height)
	{
		mWidth       = width;
		mHeight      = height;

		mAspectRatio = width / height;

		GetViewMatrix();
	}

	// -------------------------------------------------------------------- //

	void Camera::SetPosition(Maths::Vector::Vector3D<float> newPosition)
	{
		mPosition = newPosition;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetUp(Maths::Vector::Vector3D<float> newUp)
	{
		mUp = newUp;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetFOV(float FOV)
	{
		mFOV = FOV;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetFOVAsRadians(float FOV)
	{
		mFOV = glm::degrees(FOV);
	}

	// -------------------------------------------------------------------- //

	void Camera::SetNearDistance(float distance)
	{
		mNearDistance = distance;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetFarDistance(float distance)
	{
		mFarDistance = distance;
	}

	// -------------------------------------------------------------------- //

	void Camera::SetAspectRatio(float aspect)
	{
		mAspectRatio = aspect;
	}

	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //

	void DebugCamera::Update(const float deltaTime)
	{
		// Make sure to call the parent's upate
		Camera::Update(deltaTime);

		HandlePCInput(deltaTime);
	}

	// -------------------------------------------------------------------- //

	void DebugCamera::HandlePCInput(const float deltaTime)
	{
		std::vector<Input::InputDevice*>& keyboards = Input::InputSingleton::Get()->GetAllKeyboardsAttached();
		std::vector<Input::InputDevice*>& mice      = Input::InputSingleton::Get()->GetAllMiceAttached();
		
		if(keyboards.empty() || mice.empty())
			return;

		float movementSpeed = mMovementSpeed;

		// Check to see if the slow camera movement button is pressed (side mouse 1)
		Input::MouseInput::MouseInputDevice*       mouse    = (Input::MouseInput::MouseInputDevice*)mice[0];
		Input::KeyboardInput::KeyboardInputDevice* keyboard = (Input::KeyboardInput::KeyboardInputDevice*)keyboards[0];
		if (mouse)
		{
			if (mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_SIDE_1) == Input::ButtonInputState::HELD ||
				mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_SIDE_1) == Input::ButtonInputState::PRESSED ||

				keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_ALT) == Input::ButtonInputState::HELD ||
				keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_ALT) == Input::ButtonInputState::PRESSED)
			{
				movementSpeed = mMovementSpeed / 10.0f;
			}
			else if (mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_SIDE_2) == Input::ButtonInputState::HELD ||
				     mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_SIDE_2) == Input::ButtonInputState::PRESSED ||
				
					 keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_CONTROL) == Input::ButtonInputState::HELD || 
					 keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_CONTROL) == Input::ButtonInputState::PRESSED)
			{
				movementSpeed = mMovementSpeed * 10.0f;
			}
		}

		if (keyboard)
		{
			if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::D) == Input::ButtonInputState::HELD ||
				keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::D) == Input::ButtonInputState::PRESSED)
			{
				mPosition -= mRight * deltaTime * movementSpeed;
			}			
			else if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::A) == Input::ButtonInputState::HELD || 
				     keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::A) == Input::ButtonInputState::PRESSED)
			{
				mPosition += mRight * deltaTime * movementSpeed;
			}

			if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::W) == Input::ButtonInputState::HELD ||
				keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::W) == Input::ButtonInputState::PRESSED)
			{
				mPosition += mForward * deltaTime * movementSpeed;
			}
			else if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::S) == Input::ButtonInputState::HELD ||
				     keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::S) == Input::ButtonInputState::PRESSED)
			{
				mPosition -= mForward * deltaTime * movementSpeed;
			}

			if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::SPACE) == Input::ButtonInputState::HELD ||
				keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::SPACE) == Input::ButtonInputState::PRESSED)
			{
				mPosition += Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f) * deltaTime * movementSpeed;
			}
			else if (keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_SHIFT) == Input::ButtonInputState::HELD ||
				     keyboard->QueryInput(Input::KeyboardInput::KeyboardKeys::LEFT_SHIFT) == Input::ButtonInputState::PRESSED)
			{
				mPosition -= Maths::Vector::Vector3D<float>(0.0f, 1.0f, 0.0f) * deltaTime * movementSpeed;
			}
		}

		// ---------------------------

		if (mouse)
		{
			float lookSpeed   = 0.4f;
			bool  lookChanged = false;

			// Check to see if the right mouse button has been pressed
			if (mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_RIGHT) == Input::ButtonInputState::HELD ||
				mouse->QueryInput(Input::MouseInput::MouseInputBitfield::MOUSE_RIGHT) == Input::ButtonInputState::PRESSED)
			{
				Maths::Vector::Vector2D<float> movementDelta = mouse->GetMousePositionDelta();

				// If looking left/right
				if (movementDelta.x != 0.0)
				{
					mAngleFromVertical -= deltaTime * movementDelta.x * lookSpeed;

					lookChanged = true;
				}
				
				// If looking up/down
				if (movementDelta.y != 0.0f)
				{
					mAngleFromHorizonal -= deltaTime * movementDelta.y * lookSpeed;

					lookChanged = true;
				}
			}

			// Now calculate the new forward vector
			if (lookChanged)
			{
				UpdateLookDirection();
			}
		}

		// ---------------------------
	}

	// -------------------------------------------------------------------- //

	void DebugCamera::UpdateLookDirection()
	{
		// Rotation around Y (left/right looking)
		float cosTheta = cos(mAngleFromVertical);
		float sinTheta = sin(mAngleFromVertical);

		Maths::Matrix::Matrix3X3 yRotation;
		yRotation.SetFromArray({  cosTheta,  0.0f,  sinTheta,
									0.0f,      1.0f,  0.0f,
									-sinTheta,  0.0f,  cosTheta });


		// Rotation around X (up/down looking)
		cosTheta = cos(mAngleFromHorizonal);
		sinTheta = sin(mAngleFromHorizonal);

		Maths::Matrix::Matrix3X3 xRotation;
		xRotation.SetFromArray({ 1.0f, 0.0f, 0.0f,
									0.0f,  cosTheta, -sinTheta,
									0.0f,  sinTheta,  cosTheta });

		Maths::Matrix::Matrix3X3 combinedMatrix = yRotation * xRotation;

		mForward = combinedMatrix.MultiplyBy(mStartForward);
		mRight   = combinedMatrix.MultiplyBy(mStartRight);
		mUp      = combinedMatrix.MultiplyBy(mStartUp);
	}

	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //
	
	OrthographicCamera::OrthographicCamera(float nearDistance,
										   float farDistance,
										   float width,
										   float height,
										   Maths::Vector::Vector3D<float> up,
										   Maths::Vector::Vector3D<float> forward,
										   Maths::Vector::Vector3D<float> position)
		: Camera(nearDistance, farDistance, width, height, 16.0f / 9.0f, 90.0f, up, forward, position, false)
		, mAngleFromHorizonal(0.0f)
		, mAngleFromVertical(0.0f)
	{

	}

	// ---------------------------

	OrthographicCamera::~OrthographicCamera()
	{

	}

	// ---------------------------

	glm::mat4 OrthographicCamera::GetPerspectiveMatrix()
	{
		float halfWidth  = mWidth / 2.0f;
		float halfHeight = mHeight / 2.0f;
		return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfWidth, mNearDistance, mFarDistance);
	}

	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //
	// -------------------------------------------------------------------- //
}