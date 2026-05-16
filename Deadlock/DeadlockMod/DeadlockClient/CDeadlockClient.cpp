#include "CDeadlockClient.hpp"
#include "CDeadlockGUI.hpp"

#include "Fonts/CFontManager.hpp"

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>

#include <DeadlockClient/Features/CVisual/CVisual.hpp>

#include <DeadlockClient/GUI/CDeadlockMenu.hpp>
#include <DeadlockClient/Settings/CSettingsJson.hpp>
#include <DeadlockClient/Render/CRenderStackSystem.hpp>

static CDeadlockClient g_CDeadlockClient{};

auto CDeadlockClient::OnInit() -> void
{
	GetDeadlockMenu()->SetConfigSelected( GetSettingsJson()->GetConfigLoadedIndex() );
}

auto CDeadlockClient::OnFireEventClientSide( IGameEvent* pGameEvent ) -> void
{

}

auto CDeadlockClient::OnAddEntity( CEntityInstance* pInst , CHandle handle ) -> void
{
	GetEntityCache()->OnAddEntity( pInst , handle );
}

auto CDeadlockClient::OnRemoveEntity( CEntityInstance* pInst , CHandle handle ) -> void
{
	GetEntityCache()->OnRemoveEntity( pInst , handle );
}

auto CDeadlockClient::OnStartSound( const Vector3& Pos , const int SourceEntityIndex , const char* szSoundName ) -> void
{
	GetVisual()->OnStartSound( Pos , SourceEntityIndex , szSoundName );
}

auto CDeadlockClient::OnClientOutput() -> void
{
	if ( SDK::Interfaces::EngineToClient()->IsInGame() )
		GetVisual()->OnClientOutput();
}

auto CDeadlockClient::OnRender() -> void
{
	if ( GetDeadlockGUI()->IsVisible() )
		GetDeadlockMenu()->OnRenderMenu();

	GetFontManager()->FirstInitFonts();
	GetFontManager()->m_VerdanaFont.DrawString( 1 , 1 , ImColor( 1.f , 1.f , 0.f ) , FW1_LEFT , XorStr( CHEAT_NAME ) );

	GetRenderStackSystem()->OnRenderStack();
}

auto CDeadlockClient::OnCreateMove( CCitadelInput* pCitadelInput , CUserCmd* pUserCmd ) -> void
{

}

auto GetDeadlockClient() -> CDeadlockClient*
{
	return &g_CDeadlockClient;
}
