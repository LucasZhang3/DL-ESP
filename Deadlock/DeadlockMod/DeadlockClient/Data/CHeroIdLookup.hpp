#pragma once

/*
 * Partial HeroID -> display name table (external research). Incomplete by design;
 * game updates may add heroes, retire IDs, or renumber — update this file as needed.
 * Unknown IDs fall back to numeric text at the call site (see FormatEnemyHeroEspLabel).
 */

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace HeroIdLookup
{

struct HeroIdNameEntry_t
{
	uint32_t m_uHeroId;
	std::string_view m_szName;
};

// Sorted by m_uHeroId for binary search.
inline constexpr std::array<HeroIdNameEntry_t , 38> g_HeroIdNameTable =
{{
	{ 1u , "Infernus" } ,
	{ 2u , "Seven" } ,
	{ 3u , "Vindicta" } ,
	{ 4u , "LadyGeist" } ,
	{ 6u , "Abrams" } ,
	{ 7u , "Wraith" } ,
	{ 8u , "McGinnis" } ,
	{ 10u , "Paradox" } ,
	{ 11u , "Dynamo" } ,
	{ 12u , "Kelvin" } ,
	{ 13u , "Haze" } ,
	{ 14u , "Holliday" } ,
	{ 15u , "Bebop" } ,
	{ 16u , "Calico" } ,
	{ 17u , "GreyTalon" } ,
	{ 18u , "MoAndKrill" } ,
	{ 19u , "Shiv" } ,
	{ 20u , "Ivy" } ,
	{ 25u , "Warden" } ,
	{ 27u , "Yamato" } ,
	{ 31u , "Lash" } ,
	{ 35u , "Viscous" } ,
	{ 50u , "Pocket" } ,
	{ 52u , "Mirage" } ,
	{ 58u , "Vyper" } ,
	{ 60u , "Sinclair" } ,
	{ 63u , "Mina" } ,
	{ 64u , "Drifter" } ,
	{ 65u , "Venator" } ,
	{ 66u , "Victor" } ,
	{ 67u , "Paige" } ,
	{ 69u , "Doorman" } ,
	{ 72u , "Billy" } ,
	{ 76u , "Graves" } ,
	{ 77u , "Apollo" } ,
	{ 79u , "Rem" } ,
	{ 80u , "Silver" } ,
	{ 81u , "Celeste" } ,
}};

[[nodiscard]] inline auto TryLookupHeroName( const uint32_t uHeroId ) -> std::string_view
{
	auto Left = size_t{ 0 };
	auto Right = g_HeroIdNameTable.size();

	while ( Left < Right )
	{
		const auto Mid = Left + ( Right - Left ) / 2;
		const auto Id = g_HeroIdNameTable[Mid].m_uHeroId;

		if ( Id == uHeroId )
			return g_HeroIdNameTable[Mid].m_szName;

		if ( Id < uHeroId )
			Left = Mid + 1;
		else
			Right = Mid;
	}

	return {};
}

inline auto FormatEnemyHeroEspLabel( const uint32_t uHeroId , char* pszBuf , const size_t uBufLen ) -> void
{
	if ( uBufLen == 0u || pszBuf == nullptr )
		return;

	if ( const auto SvName = TryLookupHeroName( uHeroId ); !SvName.empty() )
	{
		const size_t nMaxCopy = uBufLen - 1u;
		const size_t nLen = SvName.size();
		const size_t nCopy = ( nLen < nMaxCopy ) ? nLen : nMaxCopy;

		if ( nCopy > 0u )
			std::memcpy( pszBuf , SvName.data() , nCopy );

		pszBuf[nCopy] = '\0';
	}
	else
		std::snprintf( pszBuf , uBufLen , "Unknown (%u)" , static_cast<unsigned>( uHeroId ) );
}

} // namespace HeroIdLookup
