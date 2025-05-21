#include "letterbox.h"

void StartupLetterbox(LETTERBOX *pletterbox)
{
	pletterbox->pvtletterbox = &g_vtletterbox;
}

void DrawLetterBox(LETTERBOX* pletterbox)
{

}

LETTERBOX g_letterbox;