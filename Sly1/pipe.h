#pragma once
#include "shape.h"

class PIPE : public SHAPE
{
	public:
};

void InitPipe(PIPE* ppipe);
void OnPipeAdd(PIPE* ppipe);
void OnPipeRemove(PIPE* ppipe);