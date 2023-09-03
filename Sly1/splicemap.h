#pragma once

struct SW;

#include "sw.h"
#include "serialize.h"

void LoadSwSpliceFromBrx(SW *psw, CBinaryInputStream *pbis);