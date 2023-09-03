#include "main.h"

int main(int cphzArgs, char* aphzArgs[])
{
	// Initializing all things needed for game to be started
	Startup();

	while (true)
	{
		// If level pending flag is set to other than zero load up level
		if (g_transition.m_fPending != 0)
		{
			// Loads level
			g_transition.Execute();
		}

	}
	return 0;
}

void Startup()
{
	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
	SetPhase(PHASE_Startup);
	StartupBrx();
	//InitGL();

	g_transition.m_fPending = 1; // GOTTA CHANGE THIS
}