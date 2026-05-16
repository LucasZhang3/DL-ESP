#pragma once

#include <Common/Common.hpp>

namespace Settings
{
	namespace Visual
	{
		inline auto Active = false;

		inline auto SoundStepEsp = false;
		inline auto BonesEsp = false;
		inline auto EnemyEsp = false;
		inline auto TeamEsp = false;
		inline auto ShowHeroName = false;
		inline auto ShowHealth = false;
		inline auto ShowHealthBar = false;
	}
	namespace Misc
	{
		inline auto MenuAlpha = 200;
		inline auto MenuStyle = 0;
	}
	namespace Colors
	{
		namespace Visual
		{
			inline float SoundStepEsp[3] = { 0.f , 0.f , 0.f };
		}
	}
}
