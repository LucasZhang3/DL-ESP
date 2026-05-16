#pragma once

#include <Common/Common.hpp>
#include <ImGui/imgui.h>

class CDeadlockMenu final
{
public:
	auto OnRenderMenu() -> void;

private:
	auto RenderLeftChild() -> void;
	auto RenderRightChild() -> void;

private:
	auto RenderConfigTabContent() -> void;
	auto RenderVisualTabContent() -> void;
	auto RenderMiscTabContent() -> void;

private:
	auto ButtonIcon( const char8_t* szIcon , const char* szText , ImVec2 Size ) -> bool;

private:
	auto RenderInputText( const char* szTitle , const char* szStrID , char* szBuffer , int BufferSize , float left_padding = 0.f , float max_width = -1.f ) -> bool;
	auto RenderCheckBox( const char* szTitle , const char* szStrID , bool& SettingsItem ) -> bool;
	auto RenderComboBox( const char* szTitle , const char* szStrID , int& v , const char* Items[] , int ItemsCount , float left_padding = 90.f ) -> bool;
	auto RenderSliderInt( const char* szTitle , const char* szStrID , int& v , int min , int max , float left_padding = 90.f ) -> void;
	auto RenderSliderFloat( const char* szTitle , const char* szStrID , float& v , float min , float max , float left_padding = 90.f ) -> void;

private:
	auto RenderRandomBorderColor() -> void;

private:
	float m_flMainMenuColorStepH = 0.0f;
	float m_flMainMenuColorStepS = 0.5f;

	static constexpr auto g_MainWindowSizeX = 700.f;
	static constexpr auto g_MainWindowSizeY = 540.f;

	static constexpr auto g_ChildSizeX = 220.f;
	static constexpr auto g_ButtonSizeY = 25.f;

public:
	inline auto SetConfigSelected( uint32_t Index ) -> void
	{
		m_nConfigSelected = Index;
	}

private:
	uint32_t m_nConfigSelected = 0;
	char m_szNewConfigFileName[32] = { 0 };
	ImColor m_MainMenuBorderColor;
};

auto GetDeadlockMenu() -> CDeadlockMenu*;
