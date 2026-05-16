#include "Hook_OnRemoveEntity.hpp"

#include <DeadlockClient/CDeadlockClient.hpp>

auto Hook_OnRemoveEntity( CGameEntitySystem* pCGameEntitySystem , CEntityInstance* pInst , CHandle handle ) -> void
{
	GetDeadlockClient()->OnRemoveEntity( pInst , handle );

	return OnRemoveEntity_o( pCGameEntitySystem , pInst , handle );
}
