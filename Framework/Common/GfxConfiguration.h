#pragma once
#include <cstdint>
#include <iostream>
#include <cwchar>

namespace My {
	struct GfxConfiguration
	{
		uint32_t redBits; ///< red color channel depth in bits
		uint32_t greenBits; ///< green color channel depth in bits
		uint32_t blueBits; ///< blue color channel depth in bits
		uint32_t alphaBits; ///< alpha color channel depth in bits
		uint32_t depthBits; ///< depth buffer depth in bits
		uint32_t stencilBits; ///< stencil buffer depth in bits
		uint32_t msaaSamples; ///< MSAA samples
		uint32_t screenWidth;
		uint32_t screenHeight;
		const wchar_t* appName;

		GfxConfiguration(uint32_t r = 8,uint32_t g = 8,
			uint32_t b = 8,uint32_t a = 8,
			uint32_t d=24,uint32_t s=0,uint32_t msaa=0,
			uint32_t width=1920,uint32_t height=1080,const wchar_t* app_name=L"TQEngine"):
			redBits(r),greenBits(g),blueBits(b),alphaBits(a),
			depthBits(d), stencilBits(s), msaaSamples(msaa),
			screenWidth(width), screenHeight(height), appName(app_name)
		{}

		friend std::ostream& operator<<(std::ostream& out, const GfxConfiguration& conf)
		{
			out << "App Name:" << conf.appName << std::endl;
			out<<"GfxConfiguration:"<<
				" R:" << conf.redBits <<
				" G:" << conf.greenBits <<
				" B:" << conf.blueBits <<
				" A:" << conf.alphaBits <<
				" D:" << conf.depthBits <<
				" S:" << conf.stencilBits <<
				" M:" << conf.msaaSamples <<
				" W:" << conf.screenWidth <<
				" H:" << conf.screenHeight <<
				std::endl;
			return out;
		}
	};
}