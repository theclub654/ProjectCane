#include "transition.h"

void CTransition::Set(char* pchzWorld, OID oidWarp, OID oidWarpContext, GRFTRANS grftrans)
{
	if (m_fPending == 0) {
		grftrans = grftrans;
		m_fPending = 1;
		m_pchzWorld = pchzWorld;
		m_oidWarp = oidWarp;
		m_oidWarpContext = oidWarpContext;
	}
}

void CTransition::Execute()
{
	// Setting the global game state to load because where loading a file
	SetPhase(PHASE_Load);
	// THIS IS HERE TEMPORARILY I PLAN ON REWRITING THE WAY THE GAME LOADS FILES
	CBinaryInputStream file("jb_intro");
	// Deleting parent SW object
	DeleteSw(g_psw);
	g_psw = nullptr;
	// Reading data thats not needed
	file.S32Read();
	// Reading data thats not needed
	file.S32Read();
	// Initializing parent SW object
	g_psw = (SW*)PloNew(CID::CID_SW, 0, 0, OID::OID__WORLD, -1);
	// Loads parent static world from binary file.
	LoadSwFromBrx(g_psw, &file);
	// Closing binary object
	file.Close();
	// Setting the level pending flag to 0
	m_fPending = 0;
}