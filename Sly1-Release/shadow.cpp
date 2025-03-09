#include "shadow.h"

void InitSwShadowDl(SW* psw)
{
	InitDl(&psw->dlShadow, offsetof(SHADOW, dle));
}
