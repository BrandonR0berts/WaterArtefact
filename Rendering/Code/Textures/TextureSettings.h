#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Rendering
{
	struct TextureMinMagFilters
	{
		// ------------------------------------------------------------------

		TextureMinMagFilters()
			: mMinFilter(GL_LINEAR)
			, mMagFilter(GL_NEAREST)
		{ }

		TextureMinMagFilters(GLenum minFilter, GLenum magFilter)
			: mMinFilter(minFilter)
			, mMagFilter(magFilter)
		{ }

		TextureMinMagFilters(const TextureMinMagFilters& other)
			: mMinFilter(other.mMinFilter)
			, mMagFilter(other.mMagFilter)
		{ }

		~TextureMinMagFilters() { ; }

		// ------------------------------------------------------------------

		GLenum mMinFilter;
		GLenum mMagFilter;

		// ------------------------------------------------------------------
	};

	struct TextureWrappingSettings
	{
		// ------------------------------------------------------------------

		TextureWrappingSettings()
			: mSSetting(GL_REPEAT)
			, mTSetting(GL_REPEAT)
			, mRSetting(GL_REPEAT)
		{ }

		TextureWrappingSettings(GLenum sSetting, GLenum tSetting, GLenum rSetting)
			: mSSetting(sSetting)
			, mTSetting(tSetting)
			, mRSetting(rSetting)
		{ }

		TextureWrappingSettings(const TextureWrappingSettings& other)
			: mSSetting(other.mSSetting)
			, mTSetting(other.mTSetting)
			, mRSetting(other.mRSetting)
		{ }

		~TextureWrappingSettings() { ; }

		// ------------------------------------------------------------------

		GLenum mSSetting;
		GLenum mTSetting;
		GLenum mRSetting;

		// ------------------------------------------------------------------
	};
}