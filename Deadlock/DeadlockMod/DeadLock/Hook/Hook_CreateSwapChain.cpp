#include "Hook_CreateSwapChain.hpp"

#include <DeadlockClient/CDeadlockGUI.hpp>

auto WINAPI Hook_CreateSwapChain( IDXGIFactory* pFactory , IUnknown* pDevice , DXGI_SWAP_CHAIN_DESC* pDesc , IDXGISwapChain** ppSwapChain )->HRESULT
{
	GetDeadlockGUI()->ClearRenderTargetView();

	return CreateSwapChain_o( pFactory , pDevice , pDesc , ppSwapChain );
}
