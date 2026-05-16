#include "Hook_MouseInputEnabled.hpp"

#include <DeadlockClient/CDeadlockGUI.hpp>

auto Hook_MouseInputEnabled( CCitadelInput* pCCitadelInput ) -> bool
{
	if ( GetDeadlockGUI()->IsVisible() )
		return false;

	return MouseInputEnabled_o( pCCitadelInput );
}
