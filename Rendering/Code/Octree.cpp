#include "Octree.h"

#include "Engine/Code/AssertMsg.h"
#include "GLTFModel.h"
#include "window.h"

#include "Camera.h"
#include "Shaders/ShaderProgram.h"
#include "Shaders/ShaderStore.h"

#ifdef _DEBUG_BUILD
	#include "Include/imgui/imgui.h"
	#include "Include/imgui/imgui_impl_glfw.h"
	#include "Include/imgui/imgui_impl_opengl3.h"

	#include "Rendering/Code/RenderingResourceTracking.h"
#endif

namespace Rendering
{
	// --------------------------------------

#ifdef _DEBUG_BUILD
	void LeafNode::WriteOutImGuiData(unsigned int depth)
	{
		std::string displayData;
		std::string baseDisplayData;

		for (unsigned int i = 0; i < depth; i++)
		{
			baseDisplayData += "|----";
		}

		// Display that this is a leaf node
		displayData = baseDisplayData + "Leaf Node";
		ImGui::Text(displayData.c_str());

		// Say the name of the model
		displayData = baseDisplayData + "Model Name : " + mModel->GetModelName();
		ImGui::Text(displayData.c_str());

		// Display the position
		displayData = baseDisplayData + "Position : " + std::to_string(mCentrePosition.x) + ", " + std::to_string(mCentrePosition.y) + ", " + std::to_string(mCentrePosition.z);
		ImGui::Text(displayData.c_str());

		// Display the bounding area
		/*Maths::Vector::Vector3D<float> dimensions = mModel->GetBoundingBox();
		displayData = baseDisplayData + "Model Dimensions : " + std::to_string(dimensions.x) + ", " + std::to_string(dimensions.y) + ", " + std::to_string(dimensions.z);
		ImGui::Text(displayData.c_str());*/
	}
#endif

	// --------------------------------------
	// --------------------------------------
	// --------------------------------------

	ParentNode::ParentNode(Maths::Vector::Vector3D<float> centre, Maths::Vector::Vector3D<float> dimensions)
		: mBoundingCentre(centre)
		, mBoundingDimensions(dimensions)
		, mChildNodes()
	{
		ClearUpChildren();

		mLeafNode = false;
	}

	// --------------------------------------

	ParentNode::~ParentNode()
	{
		ClearUpChildren();

		mBoundingCentre     = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);
		mBoundingDimensions = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);
	}

	// --------------------------------------

	void ParentNode::AddChildNode(GLTFModel* model, Maths::Vector::Vector3D<float> instancePosition)
	{
		// First check to see if the position of the object is beyond the extents of the octree - this is not likely but possible
		if (instancePosition.x > mBoundingCentre.x + mBoundingDimensions.x ||
			instancePosition.x < mBoundingCentre.x - mBoundingDimensions.x ||

			instancePosition.y > mBoundingCentre.y + mBoundingDimensions.y || 
			instancePosition.y < mBoundingCentre.y - mBoundingDimensions.y ||

			instancePosition.x > mBoundingCentre.x + mBoundingDimensions.x ||
			instancePosition.z < mBoundingCentre.z - mBoundingDimensions.z
			)
		{
			ASSERTFAIL("Tried to add a model instance to a position outside of the octree of the world!");

			return;
		}

		unsigned int nodeToAddTo = 0;

		// We know it is in the bounding scope, so now find which octant
		if (instancePosition.x > mBoundingCentre.x)
		{
			if (instancePosition.y > mBoundingCentre.y)
			{
				if (instancePosition.z > mBoundingCentre.z)
				{
					// + + +
					nodeToAddTo = 0;
				}
				else
				{
					// + + -
					nodeToAddTo = 1;
				}
			}
			else
			{
				if (instancePosition.z > mBoundingCentre.z)
				{
					// + - +
					nodeToAddTo = 2;
				}
				else
				{
					// + - -
					nodeToAddTo = 3;
				}
			}
		}
		else
		{
			if (instancePosition.y > mBoundingCentre.y)
			{
				if (instancePosition.z > mBoundingCentre.z)
				{
					// - + +
					nodeToAddTo = 4;
				}
				else
				{
					// - + -
					nodeToAddTo = 5;
				}
			}
			else
			{
				if (instancePosition.z > mBoundingCentre.z)
				{
					// - - +
					nodeToAddTo = 6;
				}
				else
				{
					// - - -
					nodeToAddTo = 7;
				}
			}
		}

		if (mChildNodes[nodeToAddTo])
		{
			// The node does exist, so we need to check if it is a parent or leaf node
			if (mChildNodes[nodeToAddTo]->mLeafNode)
			{
				// As this is a leaf node we need to remove this leaf and replace it with a parent node
				GLTFModel*                     modelHeldInLeaf         = ((LeafNode*)mChildNodes[nodeToAddTo])->mModel;
				Maths::Vector::Vector3D<float> instancePositionOfChild = ((LeafNode*)mChildNodes[nodeToAddTo])->mCentrePosition;

				// Clear up the node
				delete mChildNodes[nodeToAddTo];
				mChildNodes[nodeToAddTo] = nullptr;

				// Create a new parent node to replace it - passing in the centre of the octant we are currently in, and half the dimensions
				Maths::Vector::Vector3D<float> halfDimensions = mBoundingDimensions / 2.0f;
				Maths::Vector::Vector3D<float> newCentre; 

				switch (nodeToAddTo)
				{
				case 0:
					newCentre = mBoundingCentre + halfDimensions;
				break;

				case 1:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x + halfDimensions.x, 
															   mBoundingCentre.y + halfDimensions.y, 
															   mBoundingCentre.z - halfDimensions.z);
				break;

				case 2:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x + halfDimensions.x, 
															   mBoundingCentre.y - halfDimensions.y, 
															   mBoundingCentre.z + halfDimensions.z);
				break;

				case 3:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x + halfDimensions.x, 
															   mBoundingCentre.y - halfDimensions.y, 
															   mBoundingCentre.z - halfDimensions.z);
				break;

				case 4:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x - halfDimensions.x, 
															   mBoundingCentre.y + halfDimensions.y, 
															   mBoundingCentre.z + halfDimensions.z);
				break;

				case 5:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x - halfDimensions.x, 
															   mBoundingCentre.y + halfDimensions.y, 
															   mBoundingCentre.z - halfDimensions.z);
				break;

				case 6:
					newCentre = Maths::Vector::Vector3D<float>(mBoundingCentre.x - halfDimensions.x, 
															   mBoundingCentre.y - halfDimensions.y, 
															   mBoundingCentre.z + halfDimensions.z);
				break;

				case 7:
					newCentre = mBoundingCentre - halfDimensions;
				break;

				default:
					ASSERTFAIL("In an invalid octant ID somehow!");
				break;
				}

				mChildNodes[nodeToAddTo] = new ParentNode(newCentre, halfDimensions);

				// Re-add the model that was held in the last leaf
				((ParentNode*)mChildNodes[nodeToAddTo])->AddChildNode(modelHeldInLeaf, instancePositionOfChild);
			}

			// Now add the new model we came into this function for
			((ParentNode*)mChildNodes[nodeToAddTo])->AddChildNode(model, instancePosition);
		}
		else
		{
			// Child node does not exist, so for now add a leaf node there to hold the model instance
			mChildNodes[nodeToAddTo] = new LeafNode(model, instancePosition);
		}
	}

	// -------------------------------------------------------

	void ParentNode::ClearUpChildren()
	{
		for (unsigned int i = 0; i < 8; i++)
		{
			if(mChildNodes[i])
				delete mChildNodes[i];

			mChildNodes[i] = nullptr;
		}
	}

	// -------------------------------------------------------

#ifdef _DEBUG_BUILD
	void ParentNode::WriteOutImGuiData(unsigned int depth)
	{
		std::string displayData;
		std::string baseDisplayData;

		for (unsigned int i = 0; i < depth; i++)
		{
			baseDisplayData += "|----";
		}

		// Display that this is a parent node
		displayData = baseDisplayData + "Parent Node";
		ImGui::Text(displayData.c_str());

		// Display the centre
		displayData = baseDisplayData + "Centre : " + std::to_string(mBoundingCentre.x) + ", " + std::to_string(mBoundingCentre.y) + ", " + std::to_string(mBoundingCentre.z);
		ImGui::Text(displayData.c_str());

		// Display the bounding area
		displayData = baseDisplayData + "Bounding Dimensions : " + std::to_string(mBoundingDimensions.x) + ", " + std::to_string(mBoundingDimensions.y) + ", " + std::to_string(mBoundingDimensions.z);
		ImGui::Text(displayData.c_str());

		for (unsigned int i = 0; i < 8; i++)
		{
			if (mChildNodes[i])
			{
				mChildNodes[i]->WriteOutImGuiData(depth + 1);
			}
		}
	}
#endif

	// -------------------------------------------------------

	std::vector<InstanceModelData> ParentNode::GetAllModelsInCube(std::vector<Maths::Vector::Vector3D<float>>& verticies, RenderModelsCollection& modelsCollection)
	{
		std::vector<InstanceModelData> returnData;

		// If the cube is not properly formed then return now as the maths wont work
		if (verticies.size() != 8)
		{
			return returnData;
		}

		// Check to see if the cube overlaps this bounding box at all

		// Now we know that it overlaps at least a little bit, loop through all children and determine if it hits them as well
		for(unsigned int i = 0; i < 8; i++)
		{
			if (mChildNodes[i])
			{
				if (mChildNodes[i]->mLeafNode)
				{
					// Grab the position of the model in the world
					Maths::Vector::Vector3D<float> worldPos = ((LeafNode*)mChildNodes[i])->mCentrePosition;

					// Now find the ID of the model in the list of models
					int modelID = modelsCollection.GetModelIndex(((LeafNode*)mChildNodes[i])->mModel);

					// Add the model to the list
					returnData.push_back(InstanceModelData { modelID, worldPos});
				}
				else
				{
					// Get all models from the child node - this is where the recursion is
					std::vector<InstanceModelData> modelsInChild = ((ParentNode*)mChildNodes[i])->GetAllModelsInCube(verticies, modelsCollection);

					// Add the found models to the running total
					returnData.insert(returnData.end(), modelsInChild.begin(), modelsInChild.end());
				}
			}
		}

		// Return back up the list
		return returnData;
	}

	// -------------------------------------------------------
	// -------------------------------------------------------
	// -------------------------------------------------------

	Octree::Octree(RenderModelsCollection& modelsCollection)
		: mModelsCollection(modelsCollection)
		, mRootNode(nullptr)
		, mSize(0)
	{
		// Setup the octree to have a massive bounding area so that all models we will add into it will be in-bounds
		// this could be setup through the tree re-sizing itself on additions, but that gets really complex and computation heavy
		// Dimensions set to 100 million in each positive and negative axis - if something goes beyond this then we have biggers problems
		mRootNode = new ParentNode(Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f), Maths::Vector::Vector3D<float>(100000000.0f, 100000000.0f, 100000000.0f));

		// Octree buffer data SSBO
		Window::GetBufferStore().CreateSSBO(nullptr, 0, GL_STATIC_DRAW, "OctreeData_SSBO");
	}

	// -------------------------------------------------------

	Octree::~Octree()
	{
		delete mRootNode;
		mRootNode = nullptr;

		mSize = 0;
	}

	// -------------------------------------------------------

	void Octree::AddModel(GLTFModel* model, Maths::Vector::Vector3D<float> positionInWorld)
	{
		if (!model)
			return;

		// If there is no root then we can quickly add one and leave the function
		if (!mRootNode)
		{
			ASSERTFAIL("Octree root node does not exist for some reason!");

			float startDimensions = 100000000.0f;
			mRootNode = new ParentNode(Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f), Maths::Vector::Vector3D<float>(startDimensions, startDimensions, startDimensions));
		}

		// Add the child to the root node, which cascades down the children
		mRootNode->AddChildNode(model, positionInWorld);

		mSize++;
	}

	// -------------------------------------------------------

	void Octree::RemoveModel(GLTFModel* model)
	{
		if (!mRootNode)
		{
			ASSERTFAIL("Octree root node does not exist for some reason!");
			return;
		}

		mSize--;
	}

	// -------------------------------------------------------

#ifdef _DEBUG_BUILD
	void Octree::RenderDataToImGuiDisplay()
	{
		ImGui::Begin("Octree Data Display");

		if (!mRootNode)
		{
			ImGui::End();
			return;
		}
		
		// This function call recursively displays the data
		mRootNode->WriteOutImGuiData(0);

		ImGui::End();
	}
#endif

	// -------------------------------------------------------

	std::vector<InstanceModelData> Octree::FindAllWithinCube(std::vector<Maths::Vector::Vector3D<float>> verticies)
	{
		std::vector<InstanceModelData> returnData;

		// Make sure that all 8 verticies have been passed in
		if (verticies.size() != 8)
		{
			return returnData;
		}

		if (mRootNode)
		{
			returnData = mRootNode->GetAllModelsInCube(verticies, mModelsCollection);
		}

		return returnData;
	}

	// -------------------------------------------------------

	void Octree::UpdateBufferData(Camera* worldCamera)
	{
		if (!worldCamera)
		{
			return;
		}

		// ----------------------------------------------------------------

		// Get the buffer to be filling
		Buffers::ShaderStorageBufferObject* ModelDataSSBO = Window::GetBufferStore().GetSSBO("OctreeData_SSBO");

		if (!ModelDataSSBO)
		{
			ASSERTFAIL("Octree storage SSBO does not exist!");
			return;
		}

		// ----------------------------------------------------------------

		// Clear the existing memory
		ModelDataSSBO->ClearAllDataInBuffer(GL_STATIC_DRAW);

		// ----------------------------------------------------------------

		// Find all visible models in the world
		std::vector<InstanceModelData> visibleModelData = FindAllWithinCube(worldCamera->GetRoughFrustumBox());

		// Now create the new buffer of data to pass to the GPU
		unsigned int modelCount       = (unsigned int)visibleModelData.size();
		unsigned int floatsInNewData  = unsigned int(sizeof(InstanceModelData) / 4.0f) * modelCount;
		float*       newBufferData    = new float[floatsInNewData];
		unsigned int charsInData      = floatsInNewData * 4;

		unsigned int currentIndex = 0;
		unsigned int visibleModelDataCount = (unsigned int)visibleModelData.size();
		for (unsigned int i = 0; i < visibleModelDataCount; i++)
		{
			currentIndex = i * 4;
			newBufferData[currentIndex]     = (float)visibleModelData[i].mModelID;

			newBufferData[currentIndex + 1] = visibleModelData[i].mWorldPosition.x;
			newBufferData[currentIndex + 2] = visibleModelData[i].mWorldPosition.y;
			newBufferData[currentIndex + 3] = visibleModelData[i].mWorldPosition.z;
		}

		// ----------------------------------------------------------------

		// Pass the new buffer data to the GPU
		ModelDataSSBO->SetBufferData((GLvoid*)newBufferData, charsInData, GL_STATIC_DRAW);

		// ----------------------------------------------------------------

		// Delete this version of the data as it is now stored on the GPU, and we dont need it in RAM
		delete[] newBufferData;
		newBufferData = nullptr;

		// ----------------------------------------------------------------
	}

	// -------------------------------------------------------

	void Octree::Clear()
	{
		if (mRootNode)
		{
			delete mRootNode;
			mRootNode = nullptr;
		}

		mSize = 0;
	}

	// -------------------------------------------------------
}