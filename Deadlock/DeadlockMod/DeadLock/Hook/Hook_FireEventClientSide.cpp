#include "Hook_FireEventClientSide.hpp"

#include <DeadlockClient/CDeadlockClient.hpp>

auto Hook_FireEventClientSide( IGameEventManager2* pGameEventManager2 , IGameEvent* pGameEvent ) -> bool
{
	GetDeadlockClient()->OnFireEventClientSide( pGameEvent );

	return FireEventClientSide_o( pGameEventManager2 , pGameEvent );
}
