#include "Hook_OnAddEntity.hpp"

#include <DeadlockClient/CDeadlockClient.hpp>

auto Hook_OnAddEntity( CGameEntitySystem* pCGameEntitySystem , CEntityInstance* pInst , CHandle handle ) -> void
{
	GetDeadlockClient()->OnAddEntity( pInst , handle );

	return OnAddEntity_o( pCGameEntitySystem , pInst , handle );
}
