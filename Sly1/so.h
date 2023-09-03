#pragma once
#include "alo.h"

// Static Object
struct SO
{
	public:
		ALO ALocalObject;
		DLE dleRoot;
		DL dlPhys;
		DLE dlePhys;
		glm::mat3 momintLocal;
		glm::mat3 momintInvLocal;
		glm::vec3 dvGravity;
		float gBuoyancy;
		float gViscosity;
		float m;
		glm::vec3 posWorldPrev;

		void so(CID cid, SW* psw, ALO* paloParent, OID oid);
};

void InitSo(SO *pso); // NOT FINISHED