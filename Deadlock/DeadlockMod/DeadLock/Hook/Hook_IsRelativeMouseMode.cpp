#include "Hook_IsRelativeMouseMode.hpp"

#include <DeadlockClient/CDeadlockGUI.hpp>

auto Hook_IsRelativeMouseMode( CInputSystem* pInputSystem , bool Active ) -> void
{
	GetDeadlockGUI()->m_bMainActive = Active;

	if ( GetDeadlockGUI()->IsVisible() )
		return IsRelativeMouseMode_o( pInputSystem , false );

	return IsRelativeMouseMode_o( pInputSystem , Active );
}
