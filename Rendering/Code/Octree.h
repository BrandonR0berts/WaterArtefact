#pragma once

#include "Maths/Code/Vector.h"
#include "RenderModelsCollection.h"

#include <vector>

namespace Rendering
{
	class GLTFModel;

	// ----------------------------------------------------------

	struct InstanceModelData
	{
		int                            mModelID;
		Maths::Vector::Vector3D<float> mWorldPosition;
	};

	// ----------------------------------------------------------

	struct TreeNode abstract
	{
	public:
		bool mLeafNode;

#ifdef _DEBUG_BUILD
		virtual void WriteOutImGuiData(unsigned int depth) = 0;
#endif
	};

	// ----------------------------------------------------------

	// A leaf node contains an instance of a voxel model that is present in the world
	struct LeafNode : public TreeNode
	{
	public:

		LeafNode(GLTFModel* model, Maths::Vector::Vector3D<float> instancePosition)
			: mModel(model)
			, mCentrePosition(instancePosition)			
		{
			mLeafNode = true;
		}

		~LeafNode()
		{
			mModel = nullptr;
			mCentrePosition = Maths::Vector::Vector3D<float>(0.0f, 0.0f, 0.0f);
		}

#ifdef _DEBUG_BUILD
		void WriteOutImGuiData(unsigned int depth) override;
#endif

		GLTFModel*                    mModel;
		Maths::Vector::Vector3D<float> mCentrePosition;
	};

	// ----------------------------------------------------------

	// A parent node contains 8 child nodes, 
	// These nodes can either be leaves or other parent nodes that further sub-divide the space into 8
	struct ParentNode : public TreeNode
	{
	public:
		ParentNode(Maths::Vector::Vector3D<float> centre, Maths::Vector::Vector3D<float> dimensions);
		~ParentNode();

		void AddChildNode(GLTFModel* model, Maths::Vector::Vector3D<float> instancePosition);

		std::vector<InstanceModelData> GetAllModelsInCube(std::vector<Maths::Vector::Vector3D<float>>& verticies, RenderModelsCollection& modelsCollection);

#ifdef _DEBUG_BUILD
		void WriteOutImGuiData(unsigned int depth) override;
#endif

	private:
		void ClearUpChildren();

		TreeNode*                      mChildNodes[8];

		Maths::Vector::Vector3D<float> mBoundingDimensions;
		Maths::Vector::Vector3D<float> mBoundingCentre;
	};

	// ----------------------------------------------------------

	class Camera;

	// This container is for models that will never move - for example, the chunks of the world ground, buildings, trees, etc
	// Dynamic objects are contained in a seperate container
	class Octree final
	{
	public:
		Octree(RenderModelsCollection& modelsCollection);
		~Octree();

		void                           AddModel(GLTFModel* model, Maths::Vector::Vector3D<float> positionInWorld);
		void                           RemoveModel(GLTFModel* model);

		void                           UpdateBufferData(Camera* worldCamera);
		unsigned int                   GetSize()          const { return mSize; }

		std::vector<InstanceModelData> FindAllWithinCube(std::vector<Maths::Vector::Vector3D<float>> verticies);

#ifdef _DEBUG_BUILD
		void                           RenderDataToImGuiDisplay();
#endif

		void                           Clear();

	private:
		RenderModelsCollection& mModelsCollection;
		ParentNode*             mRootNode;

		unsigned int            mSize;
	};

	// ----------------------------------------------------------
}