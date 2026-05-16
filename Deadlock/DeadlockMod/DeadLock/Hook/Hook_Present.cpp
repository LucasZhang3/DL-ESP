#include "Hook_Present.hpp"

#include <DeadlockClient/CDeadlockGUI.hpp>

auto Hook_Present( IDXGISwapChain* pSwapChain , UINT SyncInterval , UINT Flags ) -> HRESULT
{
	GetDeadlockGUI()->OnPresent( pSwapChain );

	return Present_o( pSwapChain , SyncInterval , Flags );
}
