#include "transition.h"

void CTransition::Set(char* pchzWorld, OID oidWarp, OID oidWarpContext, GRFTRANS grftrans)
{
	if (m_fPending == 0) 
	{
		grftrans = grftrans;
		m_fPending = 1;
		m_pchzWorld = pchzWorld;
		m_oidWarp = oidWarp;
		m_oidWarpContext = oidWarpContext;
	}
}

void CTransition::Execute(std::string file)
{
	// Setting the global game state to load because where loading a file
	SetPhase(PHASE_Load);
	ResetClock(&g_clock, 0.0);
	// THIS IS HERE TEMPORARILY I PLAN ON REWRITING THE WAY THE GAME LOADS FILES
	CBinaryInputStream pbis{file};

	if (pbis.file.is_open() == false)
	{
		std::cout << "Error opening file\n";
		m_fPending = 0;
		return;
	}
	
	g_psw = nullptr;
	// Reading data thats not needed
	pbis.S32Read();
	// Reading data thats not needed
	pbis.S32Read();
	// Initializing parent SW object
	g_psw = (SW*)PloNew(CID::CID_SW, nullptr, nullptr, OID::OID__WORLD, -1);
	// Loads parent static world from binary file.
	g_psw->pvtlo->pfnLoadLoFromBrx(g_psw, &pbis);
	SetClockRate(1.0);

	// delete file out of memory
	pbis.file.close();
	// Setting the level pending flag to 0
	m_fPending = 0;
}