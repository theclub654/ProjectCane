#pragma once
#include "serialize.h"
#include "gc.h"

void LoadSwSpliceFromBrx(SW *psw, CBinaryInputStream *pbis);
std::shared_ptr <CFrame> PframeFromIsplice(int isplice, SW* psw);
