#include "math.h"

void LoadMatrixFromPosRot(glm::vec3 &ppos, glm::mat3 &pmat, glm::mat4 &pmatDst)
{
	pmatDst = pmat;

	pmatDst[3][0] = ppos[0];
	pmatDst[3][1] = ppos[1];
	pmatDst[3][2] = ppos[2];
	pmatDst[3][3] = 1.0;
}

void LoadMatrixFromPosRotScale(glm::vec3 &vecPos, glm::mat3 &matRot, glm::vec3 &vecScale, glm::mat4 &pmat)
{
	pmat[0][0] = matRot[0][0] * vecScale.x;
	pmat[0][1] = matRot[0][1] * vecScale.x;
	pmat[0][2] = matRot[0][2] * vecScale.x;
	pmat[0][3] = 0.0;

	pmat[1][0] = matRot[1][0] * vecScale.y;
	pmat[1][1] = matRot[1][1] * vecScale.y;
	pmat[1][2] = matRot[1][2] * vecScale.y;
	pmat[1][3] = 0.0;

	pmat[2][0] = matRot[2][0] * vecScale.z;
	pmat[2][1] = matRot[2][1] * vecScale.z;
	pmat[2][2] = matRot[2][2] * vecScale.z;
	pmat[2][3] = 0.0;

	pmat[3][0] = vecPos.x;
	pmat[3][1] = vecPos.y;
	pmat[3][2] = vecPos.z;
	pmat[3][3] = 1.0;
}

void LoadMatrixFromPosRotInverse(glm::vec3 &pposSrc, glm::mat3 &pmatSrc, glm::mat4 &pmatDst)
{
	pmatDst[0][0] = pmatSrc[0][0];
	pmatDst[0][1] = pmatSrc[0][1];
	pmatDst[0][2] = pmatSrc[0][2];

	pmatDst[1][0] = pmatSrc[1][0];
	pmatDst[1][1] = pmatSrc[1][1];
	pmatDst[1][2] = pmatSrc[1][2];

	pmatDst[2][0] = pmatSrc[2][0];
	pmatDst[2][1] = pmatSrc[2][1];
	pmatDst[2][2] = pmatSrc[2][2];
}

void BuildRotateVectorsMatrix(glm::vec3 &vec1, glm::vec3 &vec2, glm::mat4 &mat)
{
	glm::vec3 p1 = vec1;
	glm::vec3 p2 = vec2;

	p1 = normalize(p1);
	p2 = normalize(p2);


}

void LoadRotateMatrixRad(float rad, glm::vec3& pnormal, glm::mat4& pmat)
{
	
}