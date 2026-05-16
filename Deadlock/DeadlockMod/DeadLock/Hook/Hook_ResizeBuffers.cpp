#include "Hook_ResizeBuffers.hpp"

#include <DeadlockClient/CDeadlockGUI.hpp>

auto Hook_ResizeBuffers( IDXGISwapChain* pSwapChain , UINT BufferCount , UINT Width , UINT Height , DXGI_FORMAT NewFormat , UINT SwapChainFlags ) -> HRESULT
{
	GetDeadlockGUI()->OnResizeBuffers( pSwapChain );

	return ResizeBuffers_o( pSwapChain , BufferCount , Width , Height , NewFormat , SwapChainFlags );
}
