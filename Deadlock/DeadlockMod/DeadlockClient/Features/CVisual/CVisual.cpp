#include "CVisual.hpp"

#include <algorithm>

#include <DeadLock/SDK/SDK.hpp>
#include <DeadLock/SDK/Math/Math.hpp>
#include <DeadLock/SDK/Types/CEntityData.hpp>
#include <DeadLock/SDK/Interface/CGameEntitySystem.hpp>
#include <DeadLock/SDK/Interface/IEngineToClient.hpp>

#include <GameClient/CEntityCache/CEntityCache.hpp>

#include <GameClient/CL_CitadelPlayerPawn.hpp>
#include <GameClient/CL_CitadelPlayerController.hpp>
#include <GameClient/CL_Bones.hpp>

#include <DeadlockClient/Settings/Settings.hpp>
#include <DeadlockClient/Data/CHeroIdLookup.hpp>
#include <DeadlockClient/Fonts/CFontManager.hpp>
#include <DeadlockClient/Render/CRenderStackSystem.hpp>

namespace
{
[[nodiscard]] auto GetCitadelPlayerEspColor( CCitadelPlayerController* pCCitadelPlayerController , CCitadelPlayerController* pLocalCtrl ) -> ImColor
{
	if ( !pLocalCtrl )
		return ImColor( 255 , 255 , 255 );

	if ( pCCitadelPlayerController == pLocalCtrl )
		return ImColor( 255 , 255 , 255 );

	if ( pCCitadelPlayerController->m_iTeamNum() == pLocalCtrl->m_iTeamNum() )
		return ImColor( 0 , 0 , 255 );

	return ImColor( 255 , 0 , 0 );
}
}

static CVisual g_CVisual{};

auto CVisual::OnRender() -> void
{
	if ( !Settings::Visual::Active )
		return;

	const auto CachedVec = GetEntityCache()->GetCachedEntity();

	std::scoped_lock Lock( GetEntityCache()->GetLock() );

	auto* const pCachedLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();

	for ( const auto& CachedEntity : *CachedVec )
	{
		auto pEntity = CachedEntity.m_Handle.Get();

		if ( !pEntity )
			continue;

		auto hEntity = pEntity->pEntityIdentity()->Handle();

		if ( hEntity != CachedEntity.m_Handle )
			continue;

		switch ( CachedEntity.m_Type )
		{
			case CachedEntity_t::CITADEL_PLAYER_CONTROLLER:
			{
				auto* pCCitadelPlayerController = reinterpret_cast<CCitadelPlayerController*>( pEntity );
				auto* pC_CitadelPlayerPawn = pCCitadelPlayerController->m_hHeroPawn().Get<C_CitadelPlayerPawn>();

				if ( pC_CitadelPlayerPawn )
				{
					if ( pCachedLocalCtrl && pCCitadelPlayerController == pCachedLocalCtrl )
						continue;

					if ( pCachedLocalCtrl )
					{
						const bool sameTeam = pCCitadelPlayerController->m_iTeamNum() == pCachedLocalCtrl->m_iTeamNum();

						if ( sameTeam && !Settings::Visual::TeamEsp )
							continue;

						if ( !sameTeam && !Settings::Visual::EnemyEsp )
							continue;
					}

					/*static bool DumpHitBox = true;

					if ( DumpHitBox )
					{
						if ( auto pLocalCitadelPlayerPawn = GetCL_CitadelPlayerPawn()->GetLocal(); pLocalCitadelPlayerPawn )
						{
							auto* pHitBoxSet = pLocalCitadelPlayerPawn->GetHitBoxSet();

							if ( pHitBoxSet )
							{
								for ( auto HitBoxIndex = 0; HitBoxIndex < pHitBoxSet->m_HitBoxes().Count(); HitBoxIndex++ )
								{
									DEV_LOG( "%i , %s -> %s\n" , HitBoxIndex ,
											 pHitBoxSet->m_HitBoxes()[HitBoxIndex].m_name().Get() ,
											 pHitBoxSet->m_HitBoxes()[HitBoxIndex].m_sBoneName().Get() );
								}

								DumpHitBox = false;
							}
						}
					}*/

					/*static bool DumpBones = true;

					if ( DumpBones )
					{
						if ( auto pLocalCitadelPlayerPawn = GetCL_CitadelPlayerPawn()->GetLocal(); pLocalCitadelPlayerPawn )
						{
							auto& hModel = pLocalCitadelPlayerPawn->m_pGameSceneNode()->GetSkeletonInstance()->m_modelState().m_hModel();

							if ( hModel.is_valid() )
							{
								for ( auto BoneIndex = 0u; BoneIndex < hModel->m_nBoneCount; BoneIndex++ )
								{
									DEV_LOG( "%i -> %s\n" , BoneIndex , hModel->m_szBoneNames[BoneIndex] );
								}

								DumpBones = false;
							}
						}
					}*/

					const auto vOrigin = pC_CitadelPlayerPawn->m_vOldOrigin();
					Vector3 vHeadPos;
					
					if ( pC_CitadelPlayerPawn->m_pGameSceneNode()->GetBonePosition( pC_CitadelPlayerPawn->GetBoneIdByName( XorStr( "head" ) ) , vHeadPos ) )
					{
						if ( !vOrigin.IsZero() && !vHeadPos.IsZero() )
						{
							ImVec2 OriginScreen , HeadScreen;

							if ( Math::WorldToScreen( vOrigin , OriginScreen ) && Math::WorldToScreen( vHeadPos , HeadScreen ) )
							{
								const auto BoxHeight = floor( OriginScreen.y - HeadScreen.y );
								const auto BoxWidth = floor( BoxHeight / 2.f );

								Rect_t Rect;

								Rect.x = floor( HeadScreen.x - BoxWidth / 2.f );
								Rect.y = floor( HeadScreen.y );
								Rect.w = floor( HeadScreen.x + BoxWidth / 2.f );
								Rect.h = floor( OriginScreen.y );

								OnRenderPlayerEsp( pCCitadelPlayerController , Rect , pCachedLocalCtrl );
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}

	if ( Settings::Visual::SoundStepEsp )
		OnRenderSound();
}

auto CVisual::OnStartSound( const Vector3& Pos , const int SourceEntityIndex , const char* szSoundName ) -> void
{
	if ( !Settings::Visual::EnemyEsp )
		return;

	if ( strstr( szSoundName , XorStr( "Footstep" ) ) )
	{
		if ( auto* pBaseEntity = SDK::Interfaces::GameEntitySystem()->GetBaseEntity( SourceEntityIndex ); pBaseEntity )
		{
			if ( auto* pLocalCitadelPlayerController = GetCL_CitadelPlayerController()->GetLocal(); pLocalCitadelPlayerController )
			{
				if ( auto* pLocalPawn = pLocalCitadelPlayerController->m_hHeroPawn().Get<C_CitadelPlayerPawn>(); pLocalPawn )
				{
					if ( static_cast<C_BaseEntity*>( pLocalPawn ) == pBaseEntity )
						return;
				}

				if ( pLocalCitadelPlayerController->m_iTeamNum() != pBaseEntity->m_iTeamNum() )
				{
					if ( pBaseEntity->IsCitadelPlayerPawn() )
					{
						std::scoped_lock m_Lock( m_SoundLock );

						m_SoundList.emplace_back( GetTickCount64() , Pos );
					}
				}
			}
		}
	}
}

auto CVisual::OnClientOutput() -> void
{
	GetRenderStackSystem()->EnsureUpdateCapacity( 768u );

	OnRender();

	if ( Settings::Visual::BonesEsp )
	{
		auto* const pCachedLocalCtrl = GetCL_CitadelPlayerController()->GetLocal();

		FOR_EACH_ENTITY( idx )
		{
			auto pBaseEntity = SDK::Interfaces::GameEntitySystem()->GetBaseEntity( idx );

			if ( pBaseEntity && pBaseEntity->IsCitadelPlayerController() )
			{
				auto* pCCitadelPlayerController = reinterpret_cast<CCitadelPlayerController*>( pBaseEntity );

				if ( pCachedLocalCtrl && pCCitadelPlayerController == pCachedLocalCtrl )
					continue;

				if ( pCachedLocalCtrl )
				{
					const bool sameTeam = pCCitadelPlayerController->m_iTeamNum() == pCachedLocalCtrl->m_iTeamNum();

					if ( sameTeam && !Settings::Visual::TeamEsp )
						continue;

					if ( !sameTeam && !Settings::Visual::EnemyEsp )
						continue;
				}

				if ( !pCCitadelPlayerController->m_PlayerDataGlobal().m_bAlive() )
					continue;

				auto* pPawn = pCCitadelPlayerController->m_hHeroPawn().Get<C_CitadelPlayerPawn>();

				if ( !pPawn || !pPawn->IsCitadelPlayerPawn() )
					continue;

				OnRenderSkeleton( pPawn , GetCitadelPlayerEspColor( pCCitadelPlayerController , pCachedLocalCtrl ) );
			}
		}
	}
}

auto CVisual::OnRenderSound() -> void
{
	std::scoped_lock m_Lock( m_SoundLock );

	if ( !Settings::Visual::EnemyEsp )
	{
		m_SoundList.clear();
		return;
	}

	auto NewEnd = std::remove_if( m_SoundList.begin() , m_SoundList.end() , []( const SoundData_t& Sound )
	{
		return GetTickCount64() - Sound.dwTime >= g_SoundShowTime;
	} );

	m_SoundList.erase( NewEnd , m_SoundList.end() );

	for ( const auto& Sound : m_SoundList )
	{
		auto Ratio = static_cast<float>( GetTickCount64() - Sound.dwTime ) / static_cast<float>( g_SoundShowTime );
		auto Alpha = std::lerp( 1.f , 0.f , Ratio );

		ImVec2 Screen;

		if ( Math::WorldToScreen( Sound.Pos , Screen ) )
		{
			constexpr static auto SoundSize = 20.f;
			auto Radius = std::lerp( SoundSize , 0.f , Ratio );

			GetRenderStackSystem()->DrawCircle3D( Sound.Pos , Radius , ImColor( 1.f , 1.f , 0.f , Alpha ) );
		}
	}
}

auto CVisual::OnRenderPlayerEsp( CCitadelPlayerController* pCCitadelPlayerController , const Rect_t& Box , CCitadelPlayerController* pCachedLocalCtrl ) -> void
{
	if ( pCachedLocalCtrl && pCCitadelPlayerController == pCachedLocalCtrl )
		return;

	if ( !pCCitadelPlayerController->m_PlayerDataGlobal().m_bAlive() )
		return;

	const ImVec2 min = { Box.x, Box.y };
	const ImVec2 max = { Box.w, Box.h };

	const auto PlayerColor = GetCitadelPlayerEspColor( pCCitadelPlayerController , pCachedLocalCtrl );

	GetRenderStackSystem()->DrawOutlineCoalBox( min , max , PlayerColor );

	if ( Settings::Visual::ShowHeroName && pCachedLocalCtrl )
	{
		char szHeroLabel[96];
		const auto uHeroId = pCCitadelPlayerController->m_PlayerDataGlobal().m_nHeroID().m_Value;

		HeroIdLookup::FormatEnemyHeroEspLabel( uHeroId , szHeroLabel , sizeof( szHeroLabel ) );

		const auto nCenterX = static_cast<int>( floor( ( min.x + max.x ) * 0.5f ) );
		const auto nLabelY = static_cast<int>( floor( min.y ) ) - 14;

		GetRenderStackSystem()->DrawString( &GetFontManager()->m_VerdanaFont , nCenterX , nLabelY , FW1_CENTER , PlayerColor , XorStr( "%s" ) , szHeroLabel );
	}

	if ( Settings::Visual::ShowHealth || Settings::Visual::ShowHealthBar )
	{
		auto& PlayerData = pCCitadelPlayerController->m_PlayerDataGlobal();

		int iCurrentHealth = PlayerData.m_iHealth();
		int iMaxHealth = PlayerData.m_iHealthMax();

		if ( iCurrentHealth < 0 )
			iCurrentHealth = 0;

		if ( iMaxHealth > 0 && iCurrentHealth > iMaxHealth )
			iCurrentHealth = iMaxHealth;

		const auto nCenterX = static_cast<int>( floor( ( min.x + max.x ) * 0.5f ) );
		auto flYOffset = max.y + 2.f;

		if ( Settings::Visual::ShowHealthBar && iMaxHealth > 0 )
		{
			constexpr auto flBarHeight = 3.f;
			const auto flBarWidth = max.x - min.x;
			const auto flFillRatio = static_cast<float>( iCurrentHealth ) / static_cast<float>( iMaxHealth );

			const ImVec2 BarMin( min.x , flYOffset );
			const ImVec2 BarMax( max.x , flYOffset + flBarHeight );

			GetRenderStackSystem()->DrawFillBox( BarMin , BarMax , ImColor( 30 , 30 , 30 , 220 ) );

			if ( flFillRatio > 0.f )
			{
				const ImVec2 FillMax( min.x + flBarWidth * flFillRatio , flYOffset + flBarHeight );
				GetRenderStackSystem()->DrawFillBox( BarMin , FillMax , PlayerColor );
			}

			flYOffset += flBarHeight + 2.f;
		}

		if ( Settings::Visual::ShowHealth )
		{
			char szHealthText[32];

			if ( iMaxHealth > 0 )
				snprintf( szHealthText , sizeof( szHealthText ) , "%d/%d" , iCurrentHealth , iMaxHealth );
			else
				snprintf( szHealthText , sizeof( szHealthText ) , "HP: ?" );

			const auto nTextY = static_cast<int>( floor( flYOffset ) );

			GetRenderStackSystem()->DrawString( &GetFontManager()->m_VerdanaFont , nCenterX , nTextY , FW1_CENTER , PlayerColor , XorStr( "%s" ) , szHealthText );
		}
	}
}

auto CVisual::OnRenderSkeleton( C_CitadelPlayerPawn* pC_CitadelPlayerPawn , const ImColor& BoneColor ) -> void
{
	Vector3 BonePosStart , BonePosEnd;

	for ( const auto& Bones : g_AllSkeletonPairBones )
	{
		const auto& [Start , End] = Bones;

		BonePosStart = GetCL_Bones()->GetBonePositionByName( pC_CitadelPlayerPawn , Start.c_str() );
		BonePosEnd = GetCL_Bones()->GetBonePositionByName( pC_CitadelPlayerPawn , End.c_str() );

		ImVec2 ScreenStart , ScreenEnd;

		if ( !BonePosStart.IsZero() && !BonePosEnd.IsZero() &&
			 Math::WorldToScreen( BonePosStart , ScreenStart ) &&
			 Math::WorldToScreen( BonePosEnd , ScreenEnd ) )
		{
			if ( !BonePosStart.IsZero() && !BonePosEnd.IsZero() )
			{
				GetRenderStackSystem()->DrawLine( ScreenStart , ScreenEnd ,
												  BoneColor ,
												  2.f );
			}
		}
	}
}

auto GetVisual() -> CVisual*
{
	return &g_CVisual;
}
