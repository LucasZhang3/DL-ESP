#include "CDeadlockMenu.hpp"

#include <string>

#include <DeadlockClient/CDeadlockGUI.hpp>
#include <DeadlockClient/Settings/Settings.hpp>
#include <DeadlockClient/Settings/CSettingsJson.hpp>
#include <DeadlockClient/Fonts/FontAwesomeIcon.hpp>

static CDeadlockMenu g_CDeadlockMenu{};

auto CDeadlockMenu::OnRenderMenu() -> void
{
	const float MenuAlpha = static_cast<float>( Settings::Misc::MenuAlpha ) / 255.f;

	ImGui::PushStyleVar( ImGuiStyleVar_Alpha , MenuAlpha );
	ImGui::SetNextWindowSize( ImVec2( g_MainWindowSizeX , g_MainWindowSizeY ) , ImGuiCond_Always );

	if ( ImGui::Begin( XorStr( CHEAT_NAME ) , 0 , ImGuiWindowFlags_NoResize ) )
	{
		RenderRandomBorderColor();

		ImGui::BeginGroup();
		RenderLeftChild();
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		RenderRightChild();
		ImGui::EndGroup();
	}

	ImGui::End();

	ImGui::PopStyleVar();
}

auto CDeadlockMenu::RenderLeftChild() -> void
{
	ImGui::BeginChild( XorStr( "DeadlockChildLeft" ) , ImVec2( g_ChildSizeX , -1.f ) , true );

	ImGui::TextUnformatted( XorStr( "Configs" ) );
	ImGui::Separator();

	const auto ConfigList = GetSettingsJson()->GetConfigList();
	const auto ConfigLoaded = GetSettingsJson()->GetConfigLoadedIndex();

	for ( auto idx = 0u; idx < ConfigList.size(); idx++ )
	{
		auto ConfigName = ConfigList[idx];
		auto ConfigColor = ImColor( 255 , 255 , 255 );

		if ( ConfigLoaded == idx && m_nConfigSelected == idx )
		{
			ConfigName += XorStr( " Loaded+" );
			ConfigColor = ImColor( 0 , 255 , 0 );
		}
		else if ( ConfigLoaded == idx )
		{
			ConfigName += XorStr( " Loaded" );
			ConfigColor = ImColor( 0 , 255 , 0 );
		}
		else if ( m_nConfigSelected == idx )
		{
			ConfigName += XorStr( " Selected" );
			ConfigColor = ImColor( 255 , 140 , 0 );
		}

		ImGui::PushStyleVar( ImGuiStyleVar_ButtonTextAlign , ImVec2( 0.f , 0.5f ) );
		ImGui::PushStyleColor( ImGuiCol_Text , ConfigColor.operator ImVec4() );

		if ( ImGui::Button( ConfigName.c_str() , ImVec2( -1.f , 0.f ) ) )
			m_nConfigSelected = idx;

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	ImGui::EndChild();
}

auto CDeadlockMenu::RenderRightChild() -> void
{
	ImGui::BeginChild( XorStr( "DeadlockChildRight" ) , ImVec2( ImGui::GetContentRegionAvail().x , ImGui::GetContentRegionAvail().y - 2.f ) , true );

	if ( ImGui::BeginTabBar( XorStr( "##MainSettingsTabBar" ) , ImGuiTabBarFlags_None ) )
	{
		if ( ImGui::BeginTabItem( XorStr( "Config##tab_config" ) ) )
		{
			RenderConfigTabContent();
			ImGui::EndTabItem();
		}

		if ( ImGui::BeginTabItem( XorStr( "Visual##tab_visual" ) ) )
		{
			RenderVisualTabContent();
			ImGui::EndTabItem();
		}

		if ( ImGui::BeginTabItem( XorStr( "Misc##tab_misc" ) ) )
		{
			RenderMiscTabContent();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::EndChild();
}

auto CDeadlockMenu::RenderConfigTabContent() -> void
{
	ImGui::BeginChild( XorStr( "##ConfigTabScroll" ) , ImVec2( -1.f , 0.f ) , ImGuiChildFlags_Borders );

	const auto& ConfigList = GetSettingsJson()->GetConfigList();
	const auto LoadedIdx = GetSettingsJson()->GetConfigLoadedIndex();

	if ( ConfigList.empty() )
		ImGui::TextUnformatted( XorStr( "Loaded: (no .json files)" ) );
	else if ( LoadedIdx < ConfigList.size() )
		ImGui::Text( XorStr( "Loaded: %s" ) , ConfigList[LoadedIdx].c_str() );
	else
		ImGui::TextUnformatted( XorStr( "Loaded: (unknown)" ) );

	ImGui::Spacing();
	ImGui::TextWrapped( XorStr( "Select a config in the left list, then use the actions below." ) );
	ImGui::Separator();

	RenderInputText( XorStr( "New file name" ) , XorStr( "##NewConfigFileName" ) , m_szNewConfigFileName , 32 , 0.f , -1.f );

	if ( ButtonIcon( ICON_FA_FILE_ALT , XorStr( "Create##CreateAndSaveNewConfig" ) , ImVec2( -1.f , g_ButtonSizeY ) ) )
	{
		const std::string ConfigFileName = m_szNewConfigFileName;

		if ( !ConfigFileName.empty() )
		{
			GetSettingsJson()->SaveConfig( ConfigFileName + XorStr( ".json" ) );
			GetSettingsJson()->UpdateConfigList();
		}
	}

	const auto ConfigExist = ConfigList.empty() ? false : true;

	if ( ButtonIcon( ICON_FA_DOWNLOAD , XorStr( "Load##LoadSelectedConfig" ) , ImVec2( -1.f , g_ButtonSizeY ) ) && ConfigExist )
	{
		GetSettingsJson()->LoadConfig( ConfigList[m_nConfigSelected] );
		GetDeadlockGUI()->UpdateStyle();
	}

	if ( ButtonIcon( ICON_FA_SAVE , XorStr( "Save##SaveSelectedConfig" ) , ImVec2( -1.f , g_ButtonSizeY ) ) && ConfigExist )
	{
		GetSettingsJson()->SaveConfig( ConfigList[m_nConfigSelected] );
	}

	if ( ButtonIcon( ICON_FA_CUT , XorStr( "Delete##DeleteSelectedConfig" ) , ImVec2( -1.f , g_ButtonSizeY ) ) && ConfigExist )
	{
		GetSettingsJson()->DeleteConfig( ConfigList[m_nConfigSelected] );
		GetSettingsJson()->UpdateConfigList();
	}

	if ( ButtonIcon( ICON_FA_UNDO , XorStr( "Refresh list##RefreshConfigList" ) , ImVec2( -1.f , g_ButtonSizeY ) ) )
	{
		GetSettingsJson()->UpdateConfigList();
	}

	ImGui::Separator();
	ImGui::Text( XorStr( "%s  v%s" ) , CHEAT_NAME , CHEAT_VERSION );
	ImGui::Text( XorStr( "Build: %s %s" ) , __DATE__ , __TIME__ );

	ImGui::EndChild();
}

auto CDeadlockMenu::RenderVisualTabContent() -> void
{
	ImGui::BeginChild( XorStr( "##DeadlockVisualTabPanel" ) , ImVec2( -1.f , 0.f ) , ImGuiChildFlags_Borders );

	RenderCheckBox( XorStr( "Player ESP" ) , XorStr( "##Settings.Visual.Active" ) , Settings::Visual::Active );

	ImGui::Separator();

	RenderCheckBox( XorStr( "Enemy ESP" ) , XorStr( "##Settings.Visual.EnemyEsp" ) , Settings::Visual::EnemyEsp );
	RenderCheckBox( XorStr( "Team ESP" ) , XorStr( "##Settings.Visual.TeamEsp" ) , Settings::Visual::TeamEsp );

	ImGui::Separator();

	RenderCheckBox( XorStr( "Show Hero Name" ) , XorStr( "##Settings.Visual.ShowHeroName" ) , Settings::Visual::ShowHeroName );
	RenderCheckBox( XorStr( "Show Health" ) , XorStr( "##Settings.Visual.ShowHealth" ) , Settings::Visual::ShowHealth );
	RenderCheckBox( XorStr( "Show Health Bar" ) , XorStr( "##Settings.Visual.ShowHealthBar" ) , Settings::Visual::ShowHealthBar );

	ImGui::Separator();

	RenderCheckBox( XorStr( "Footstep ESP" ) , XorStr( "##Settings.Visual.SoundStepEsp" ) , Settings::Visual::SoundStepEsp );

	ImGui::Separator();

	RenderCheckBox( XorStr( "Bones ESP" ) , XorStr( "##Settings.Visual.BonesEsp" ) , Settings::Visual::BonesEsp );

	ImGui::EndChild();
}

auto CDeadlockMenu::RenderMiscTabContent() -> void
{
	ImGui::BeginChild( XorStr( "##DeadlockMiscTabPanel" ) , ImVec2( -1.f , 0.f ) , ImGuiChildFlags_Borders );

	const char* szMenuStyle[] =
	{
		"Indigo",
		"Vermillion",
		"Classic Steam",
		"Charcoal"
	};

	RenderSliderInt( XorStr( "Menu alpha" ) , XorStr( "##Settings.Misc.MenuAlpha" ) , Settings::Misc::MenuAlpha , 100 , 255 , 150.f );

	if ( RenderComboBox( XorStr( "Menu style" ) , XorStr( "##Settings.Misc.MenuStyle" ) , Settings::Misc::MenuStyle , szMenuStyle , IM_ARRAYSIZE( szMenuStyle ) , 150.f ) )
		GetDeadlockGUI()->UpdateStyle();

	ImGui::EndChild();
}

auto CDeadlockMenu::ButtonIcon( const char8_t* szIcon , const char* szText , ImVec2 Size ) -> bool
{
	ImGui::PushFont( GetDeadlockGUI()->m_pFontAwesomeIcons );
	ImGui::Text( XorStr( "%s" ) , szIcon );
	ImGui::PopFont();

	ImGui::SameLine( 40.f );

	return ImGui::Button( szText , Size );
}

auto CDeadlockMenu::RenderInputText( const char* szTitle , const char* szStrID , char* szBuffer , int BufferSize , float left_padding , float max_width ) -> bool
{
	const auto TextSize = ImGui::CalcTextSize( szTitle ).x + 10.f;

	if ( szTitle )
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text( szTitle );

		if ( left_padding <= 0.f )
			ImGui::SameLine( TextSize );
		else
			ImGui::SameLine( left_padding );
	}

	if ( max_width != -1.f )
		ImGui::PushItemWidth( max_width - TextSize );
	else
		ImGui::PushItemWidth( -1.f );

	const auto Ret = ImGui::InputText( szStrID , szBuffer , BufferSize , 0 );

	ImGui::PopItemWidth();

	return Ret;
}

auto CDeadlockMenu::RenderCheckBox( const char* szTitle , const char* szStrID , bool& SettingsItem ) -> bool
{
	if ( szTitle )
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text( szTitle );
		ImGui::SameLine( ImGui::CalcTextSize( szTitle ).x + 10.f );
	}

	const auto LeftPadding = ImGui::GetStyle().FramePadding.x;

	ImGui::Dummy( ImVec2( ImGui::GetContentRegionAvail().x - 27.f - LeftPadding , 0.f ) );
	ImGui::SameLine();

	return ImGui::Checkbox( szStrID , &SettingsItem );
}

auto CDeadlockMenu::RenderComboBox( const char* szTitle , const char* szStrID , int& v , const char* Items[] , int ItemsCount , float left_padding ) -> bool
{
	if ( szTitle )
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text( szTitle );

		if ( left_padding <= 0.f )
			ImGui::SameLine( ImGui::CalcTextSize( szTitle ).x + 10.f );
		else
			ImGui::SameLine( left_padding );
	}

	ImGui::PushItemWidth( -1.f );
	const auto Ret = ImGui::Combo( szStrID , &v , Items , ItemsCount );
	ImGui::PopItemWidth();

	return Ret;
}

auto CDeadlockMenu::RenderSliderInt( const char* szTitle , const char* szStrID , int& v , int min , int max , float left_padding ) -> void
{
	if ( szTitle )
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text( szTitle );

		if ( left_padding <= 0.f )
			ImGui::SameLine( ImGui::CalcTextSize( szTitle ).x + 10.f );
		else
			ImGui::SameLine( left_padding );
	}

	ImGui::PushItemWidth( -1.f );
	ImGui::SliderInt( szStrID , &v , min , max , "%d" , ImGuiSliderFlags_AlwaysClamp );
	ImGui::PopItemWidth();
}

auto CDeadlockMenu::RenderSliderFloat( const char* szTitle , const char* szStrID , float& v , float min , float max , float left_padding ) -> void
{
	if ( szTitle )
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text( szTitle );

		if ( left_padding <= 0.f )
			ImGui::SameLine( ImGui::CalcTextSize( szTitle ).x + 10.f );
		else
			ImGui::SameLine( left_padding );
	}

	ImGui::PushItemWidth( -1.f );
	ImGui::SliderFloat( szStrID , &v , min , max , "%.3f" , ImGuiSliderFlags_AlwaysClamp );
	ImGui::PopItemWidth();
}

auto CDeadlockMenu::RenderRandomBorderColor() -> void
{
	auto WindowPos = ImGui::GetWindowPos() - ImVec2( 1.f , 1.f );
	auto WindowSize = ImGui::GetWindowSize() + ImVec2( 2.f , 2.f );

	auto pBackGround = ImGui::GetBackgroundDrawList();

	if ( m_flMainMenuColorStepH > 1.f )
		m_flMainMenuColorStepH = 0.f;
	else
		m_flMainMenuColorStepH += 0.01f;

	if ( m_flMainMenuColorStepS > 1.f )
		m_flMainMenuColorStepS = 0.f;
	else
		m_flMainMenuColorStepS += 0.01f;

	m_MainMenuBorderColor.SetHSV( m_flMainMenuColorStepH , m_flMainMenuColorStepS , 1.f );

	pBackGround->AddRect( WindowPos , WindowPos + WindowSize , m_MainMenuBorderColor , 1.f , 0 , 1.f );
}

auto GetDeadlockMenu() -> CDeadlockMenu*
{
	return &g_CDeadlockMenu;
}
