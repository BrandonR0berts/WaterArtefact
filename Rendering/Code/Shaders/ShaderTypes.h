#pragma once

namespace Rendering
{
	// -------------------------------------------

	enum class ShaderTypes : unsigned int
	{
		VertexShader   = 0,
		FragmentShader = 1,
		GeometryShader = 2,
		ComputeShader  = 3,

		ShaderTypeCount
	};

	// -------------------------------------------

	enum class VertexShaderTypes : unsigned int
	{
		UI_Video                 = 0,
		GLTF_Instance_Rendering  = 1,
		PickingTextureGeneration = 2,
		Gizmo                    = 3,
		ShadowGenerationShader   = 4,
		Gizmo_Highlight          = 5,
		Skybox                   = 6,

		Text                     = 7,
		UI_Sprite                = 8,

		Particles_Rendering      = 9,

		ConvoluteCubeMap             = 10,
		ConvoluteCubeMap_Reflections = 11,

		DebugRender                  = 12,
		BRDFGeneration               = 13,

		TilingTextures               = 14,

		ShaderCount
	};

	// -------------------------------------------

	enum class FragmentShaderTypes : unsigned int
	{	
		UI_Video                 = 0,
		GLTF_Instance_Rendering  = 1,
		PickingTextureGeneration = 2,
		Gizmo                    = 3,
		ShadowGenerationShader   = 4,
		Gizmo_Highlight          = 5,
		Skybox                   = 6,

		Text                     = 7,
		UI_Sprite                = 8,

		Particles_Rendering      = 9,

		ConvoluteCubeMap             = 10,
		ConvoluteCubeMap_Reflections = 11,

		DebugRender                  = 12,
		BRDFGeneration               = 13,

		TilingTextures               = 14,

		ShaderCount
	};

	// -------------------------------------------

	enum class GeometryShaderTypes : unsigned int
	{
		Gizmo_Translation = 0,
		Gizmo_Scale       = 1,
		Gizmo_Rotation    = 2,

		Gizmo_Highlight   = 3,

		ShaderCount
	};

	// -------------------------------------------

	enum class ComputeShaderTypes : unsigned int
	{
		RayTracing_ComputeShaderVolumeData = 0,
		PerlinNoiseGeneration              = 1,

		RayTracing_ComputeShaderVolumeDataMultiPass_Part1 = 2,
		RayTracing_ComputeShaderVolumeDataMultiPass_Part2 = 3,

		RainParticleSystem = 4,

		ShaderCount
	};

	// -------------------------------------------

	enum class ShaderProgramTypes : unsigned int
	{	
		UI_Video                             = 0,
		
		GLTF_Instance_Program                = 1,
		PickingTextureGeneration             = 2,

		// Level editor gizmos
		Gizmo_Translation_Program            = 3,
		Gizmo_Scale_Program                  = 4,
		Gizmo_Rotation_Program               = 5,

		Shadow_Generation_Program            = 6,

		Gizmo_Highlight_Program              = 7,

		Skybox_Program                       = 8,

		Text_Program                         = 9,
		UI_Sprite                            = 10,

		Rain_Particles_Program               = 11,
		Particle_Rendering_Program           = 12,

		ConvoluteCubeMap_Program             = 13,
		ConvoluteCubeMap_Reflections_Program = 14,

		DebugRenderProgram                   = 15,
		BRDFGenerationProgram                = 16,

		TilingTextures                       = 17,

		ProgramCount
	};

	// -------------------------------------------
}