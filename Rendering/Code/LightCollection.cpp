#include "LightCollection.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"

	#include "Rendering/Code/RenderingResourceTracking.h"
#endif

#include "window.h"
#include "Engine/Code/AssertMsg.h"

namespace Rendering
{
	// -----------------------------------------------------

	LightCollection::LightCollection() 
		: mDirectionalLights()
		, mPointLights()
	{
		// Create the SSBO that will be used to pass the light data to the ray tracer
		Window::GetBufferStore().CreateSSBO(nullptr, 0, GL_STATIC_DRAW, "LightData_SSBO");
	}

	// -----------------------------------------------------

	LightCollection::~LightCollection()
	{
		//ClearAllLights();
	}

	// -----------------------------------------------------

	void LightCollection::AddDirectionalLight(Maths::Vector::Vector3D<float> direction, Maths::Vector::Vector3D<float> colour, Maths::Vector::Vector3D<float> position)
	{
		mDirectionalLights.push_back(DirectionalLight(colour, direction, position));
	}

	// -----------------------------------------------------

	void LightCollection::AddPointLight(Maths::Vector::Vector3D<float> position, Maths::Vector::Vector3D<float> colour, float intensity)
	{
		mPointLights.push_back(PointLight(position, colour, intensity));
	}

	// -----------------------------------------------------

	// If directional is true then it checks the direction,
	// If it is false then it checks position
	void LightCollection::RemoveLight(Maths::Vector::Vector3D<float> dataToCheck, bool directional)
	{
		if (directional)
		{
			unsigned int directionalLightCount = (unsigned int)mDirectionalLights.size();
			for (unsigned int i = 0; i < directionalLightCount; i++)
			{
				if (mDirectionalLights[i].mDirection == dataToCheck)
				{					
					mDirectionalLights.erase(mDirectionalLights.begin() + i);

					return;
				}
			}
		}
		else
		{
			unsigned int pointLightCount = (unsigned int)mPointLights.size();
			for (unsigned int i = 0; i < pointLightCount; i++)
			{
				if (mPointLights[i].mPosition == dataToCheck)
				{
					mPointLights.erase(mPointLights.begin() + i);

					return;
				}
			}
		}
	}

	// -----------------------------------------------------

	void LightCollection::RemoveLight(unsigned int index, bool directional)
	{
		if (directional)
		{			
			mDirectionalLights.erase(mDirectionalLights.begin() + index);
		}
		else
		{
			mPointLights.erase(mPointLights.begin() + index);
		}
	}

	// -----------------------------------------------------

#ifdef _DEBUG_BUILD
	void LightCollection::RenderImGuiData()
	{
		ImGui::Begin("Light Data Table Overlay");

		ImGui::Text(std::string("Lights in level: ").append(std::to_string(mDirectionalLights.size() + mPointLights.size())).c_str());

		ImGuiTableFlags localFlags = ImGuiTableFlags_Resizable + ImGuiTableFlags_Borders;
		if (ImGui::BeginTable("Lights Data Table", 5, localFlags, ImVec2(0, 0), 0))
		{
			ImGui::TableSetupColumn("Light Type");
			ImGui::TableSetupColumn("Position");
			ImGui::TableSetupColumn("Direction");
			ImGui::TableSetupColumn("Colour");
			ImGui::TableSetupColumn("Intensity");
			ImGui::TableHeadersRow();

			// First do the directional lights
			unsigned int directionalLightCount = (unsigned int)mDirectionalLights.size();
			for (unsigned int i = 0; i < directionalLightCount; i++)
			{
				ImGui::TableNextColumn();

				// Light type
				ImGui::Text("Directional");

				ImGui::TableNextColumn();

				// Position
				std::string positionData = "";
							positionData += std::to_string(mDirectionalLights[i].mPosition.x) + ", " + std::to_string(mDirectionalLights[i].mPosition.y) + ", " + std::to_string(mDirectionalLights[i].mPosition.z);
				ImGui::Text(positionData.c_str());
				ImGui::TableNextColumn();

				// Direction data					
				std::string directionData = "";
					        directionData += std::to_string(mDirectionalLights[i].mDirection.x) + ", " + std::to_string(mDirectionalLights[i].mDirection.y) + ", " + std::to_string(mDirectionalLights[i].mDirection.z);
				ImGui::Text(directionData.c_str());				
				ImGui::TableNextColumn();

				// Colour data
				std::string colourData = "";
					        colourData += std::to_string(mDirectionalLights[i].mColour.x) + ", " + std::to_string(mDirectionalLights[i].mColour.y) + ", " + std::to_string(mDirectionalLights[i].mColour.z);
				ImGui::Text(colourData.c_str());
				ImGui::TableNextColumn();

				// Intensity data	
				ImGui::Text("N/A");
			}

			// Now do the point lights
			unsigned int pointLightCount = (unsigned int)mPointLights.size();
			for (unsigned int i = 0; i < pointLightCount; i++)
			{
				ImGui::TableNextColumn();

				ImGui::Text("Point");
				ImGui::TableNextColumn();

				// Position
				std::string positionData = "";
						    positionData += std::to_string(mPointLights[i].mPosition.x) + ", " + std::to_string(mPointLights[i].mPosition.y) + ", " + std::to_string(mPointLights[i].mPosition.z);
				ImGui::Text(positionData.c_str());
				ImGui::TableNextColumn();

				// Directional data
				ImGui::Text("N/A");
				ImGui::TableNextColumn();

				// Colour data
				std::string colourData = "";
				            colourData += std::to_string(mPointLights[i].mColour.x) + ", " + std::to_string(mPointLights[i].mColour.y) + ", " + std::to_string(mPointLights[i].mColour.z);
				ImGui::Text(colourData.c_str());
				ImGui::TableNextColumn();

				// Intensity data
				ImGui::Text(std::to_string(mPointLights[i].mIntensity).c_str());
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}
#endif

	// -----------------------------------------------------

	void LightCollection::UpdateLightBufferData()
	{
		Buffers::ShaderStorageBufferObject* LightDataSSBO = Window::GetBufferStore().GetSSBO("LightData_SSBO");	

		if (!LightDataSSBO)
		{
			ASSERTFAIL("Light storage SSBO does not exist!");
			return;
		}

		// ----------------------------------------------------------------

		// Clear the existing memory
		LightDataSSBO->ClearAllDataInBuffer(GL_STATIC_DRAW);

		// ----------------------------------------------------------------

		// Calculate how many floats in the new buffer
		unsigned int bufferLength = ((unsigned int)mDirectionalLights.size() * SizeOfDirectionalLightOnGPU) + ((unsigned int)mPointLights.size() * SizeOfPointLightOnGPU);

		// Now add on the counts at the start - 2, one for each type of light
		bufferLength += 2;
		float* newBufferData = new float[bufferLength];

		// ----------------------------------------------------------------

		// Add the amount of each type of lights to the start of the buffer
		newBufferData[0] = (float)mDirectionalLights.size();
		newBufferData[1] = (float)mPointLights.size();

		// ----------------------------------------------------------------

		// Now loop through the lights and copy across the data

		// Directional first
		unsigned int nextPos = 2;
		unsigned int directionalLightCount = (unsigned int)mDirectionalLights.size();
		for (unsigned int i = 0; i < directionalLightCount; i++)
		{
			// Colour
			newBufferData[nextPos]     = mDirectionalLights[i].mColour.x;
			newBufferData[nextPos + 1] = mDirectionalLights[i].mColour.y;
			newBufferData[nextPos + 2] = mDirectionalLights[i].mColour.z;

			// Direction
			newBufferData[nextPos + 3] = mDirectionalLights[i].mDirection.x;
			newBufferData[nextPos + 4] = mDirectionalLights[i].mDirection.y;
			newBufferData[nextPos + 5] = mDirectionalLights[i].mDirection.z;

			// Move onto next one
			nextPos += SizeOfDirectionalLightOnGPU;
		}

		// Now for point lights
		unsigned int pointLightCount = (unsigned int)mPointLights.size();
		for (unsigned int i = 0; i < pointLightCount; i++)
		{
			// First position
			newBufferData[nextPos]     = mPointLights[i].mPosition.x;
			newBufferData[nextPos + 1] = mPointLights[i].mPosition.y;
			newBufferData[nextPos + 2] = mPointLights[i].mPosition.z;

			// Colour
			newBufferData[nextPos + 3] = mPointLights[i].mColour.x;
			newBufferData[nextPos + 4] = mPointLights[i].mColour.y;
			newBufferData[nextPos + 5] = mPointLights[i].mColour.z;

			// Range
			newBufferData[nextPos + 6] = mPointLights[i].mIntensity;

			// Move onto next one
			nextPos += SizeOfPointLightOnGPU;
		}

		// ----------------------------------------------------------------

		// Pass the new buffer data to the GPU
		unsigned int bufferLengthBytes = bufferLength * 4;
		LightDataSSBO->SetBufferData((GLvoid*)newBufferData, bufferLengthBytes, GL_STATIC_DRAW);

		// ----------------------------------------------------------------

		// Delete this version of the data
		delete[] newBufferData;
		newBufferData = nullptr;

		// ----------------------------------------------------------------
	}

	// -----------------------------------------------------

	void LightCollection::Update(const float deltaTime)
	{
		/*static float timer   = 0.0;
		static bool  goingUp = true;

		timer += deltaTime;

		if (goingUp)
		{
			mPointLights[0].mPosition.y += deltaTime;
			mPointLights[0].mPosition.z += deltaTime;
		}
		else
		{
			mPointLights[0].mPosition.y -= deltaTime;
			mPointLights[0].mPosition.z += deltaTime;
		}


		if (timer > 2.0)
		{
			goingUp = !goingUp;
			timer   = 0.0f;
		}

		UpdateLightBufferData();*/
	}

	// -----------------------------------------------------

	void LightCollection::ClearAllLights()
	{
		mPointLights.clear();
		mDirectionalLights.clear();

		UpdateLightBufferData();
	}

	// -----------------------------------------------------

	unsigned int LightCollection::size()
	{
		return (unsigned int)(mPointLights.size() + mDirectionalLights.size());
	}

	// -----------------------------------------------------
}