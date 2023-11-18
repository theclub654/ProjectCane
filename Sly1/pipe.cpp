#include "pipe.h"

void InitPipe(PIPE* ppipe)
{
	//std::cout << "PIPE Size: " << sizeof(PIPE) << "\n";
	InitShape(ppipe);
}

void OnPipeAdd(PIPE* ppipe)
{
	OnLoAdd(ppipe);
}

void OnPipeRemove(PIPE* ppipe)
{
	OnLoRemove(ppipe);
}

void DeletePipe(LO* plo)
{
	delete(PIPE*)plo;
}
