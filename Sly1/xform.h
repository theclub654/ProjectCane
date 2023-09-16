#pragma once
#include "alo.h"
#include "vec.h"

class XFM : public LO
{
	glm::vec3 posLocal;
	glm::mat3 matLocal;
};

class WARP : public XFM
{
	public:
};

class EXIT : public ALO
{
	public:
};

void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis); // GOTTA COME BACK TO THIS
void LoadExitFromBrx(EXIT *pexit, CBinaryInputStream* pbis); // GOTTA COME BACK TO THIS