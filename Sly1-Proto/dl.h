#pragma once
#include <iostream>

// Data List
// Stores ptr to first entry in list and ptr to last entry in list
struct DL
{
	union
	{
		// Stores the first ptr to data
		void* pvFirst;
		class LO* ploFirst;
		class ALO* paloFirst;
		class SO* psoFirst;
		/*class ASEGA* pasegaFirst;
		class ACTSEG* pactsegFirst;
		class AMB* pambFirst;
		class EXC* pexcFirst;
		class RIP* pripFirst;
		class BLIP* pblipFirst;
		class BLIPG* pblipgFirst;
		class DZ* pdzFirst;
		class TARGET* ptargetFirst;
		class HND* phndFirst;
		class ACT* pactFirst;
		class IKH* pikhFirst;*/
		class LIGHT* plightFirst;
		/*class SHADOW* pshadowFirst;
		class SHAPE* pshapeFirst;
		class SMA* psmaFirst;
		class PXR* ppxrFirst;
		class HSHAPE* phshapeFirst;
		class HPNT* phpntFirst;
		class HBSK* phbskFirst;
		class PIPE* ppipeFirst;
		class PATHZONE* ppathzoneFirst;
		class FLY* pflyFirst;
		class DPRIZE* pdprizeFirst;
		class RAT* pratFirst;
		class RATHOLE* pratholeFirst;
		class ROC* procFirst;
		class ROH* prohFirst;
		class ROST* prostFirst;
		class DART* pdartFirst;
		class SPIRE* pspireFirst;
		class RAIL* prailFirst;
		class LANDING* plandingFirst;
		class LASEN* plasenFirst;
		class FE* pfeFirst;
		class EXPLSTE* pexplsteFirst;
		class FADER* pfaderFirst;
		class CRFOD* pcrfodFirst;
		class JLOVOL* pjlovolFirst;
		class EMITTER* pemitterFirst;
		class MGCO* pmgcoFirst;
		class JMT* pjmtFirst;*/
	};

	union
	{
		// Stores the last ptr to the last data in list
		void* pvLast;
		class LO* ploLast;
		class ALO* paloLast;
		/*class SO* psoLast;
		class ASEGA* pasegaLast;
		class ACTSEG* pactsegLast;
		class AMB* pambLast;
		class EXC* pexcLast;
		class RIP* pripLast;
		class BLIP* pblipLast;
		class BLIPG* pblipgLast;
		class DZ* pdzLast;
		class TARGET* ptargetLast;
		class HND* phndLast;
		class ACT* pactLast;
		class IKH* pikhLast;
		class LIGHT* plightLast;
		class SHADOW* pshadowLast;
		class SHAPE* pshapeLast;
		class SMA* psmaLast;
		class PXR* ppxrLast;
		class HSHAPE* phshapeLast;
		class HPNT* phpntLast;
		class HBSK* phbskLast;
		class PIPE* ppipeLast;
		class PATHZONE* ppathzoneLast;
		class FLY* pflyLast;
		class DPRIZE* pdprizeLast;
		class RAT* pratLast;
		class RATHOLE* pratholeLast;
		class ROC* procLast;
		class ROH* prohLast;
		class ROST* prostLast;
		class DART* pdartLast;
		class SPIRE* pspireLast;
		class RAIL* prailLast;
		class LANDING* plandingLast;
		class LASEN* plasenLast;
		class FE* pfeLast;
		class EXPLSTE* pexplsteLast;
		class FADER* pfaderLast;
		class CRFOD* pcrfodLast;
		class JLOVOL* pjlovolLast;
		class EMITTER* pemitterLast;
		class MGCO* pmgcoLast;
		class JMT* pjmtLast;*/
	};

	// Base offset to DL entry (DLE)
	uint64_t ibDle;
};

// Data List Element
// DLE stores the next and previous ptr to data
// Linked list data structure
// Entry is just another word for linked list
struct DLE
{
	union
	{
		// Stores ptr to next data
		void* pvNext;
		class LO* ploNext;
		class ALO* paloNext;
		class SO* psoNext;
		class ASEGA* pasegaNext;
		class ACTSEG* pactsegNext;
		class AMB* pambNext;
		class EXC* pexcNext;
		class RIP* pripNext;
		class BLIP* pblipNext;
		class BLIPG* pblipgNext;
		class DZ* pdzNext;
		class TARGET* ptargetNext;
		class HND* phndNext;
		class ACT* pactNext;
		class IKH* pikhNext;
		class LIGHT* plightNext;
		class SHADOW* pshadowNext;
		class SHAPE* pshapeNext;
		class SMA* psmaNext;
		class PXR* ppxrNext;
		class HSHAPE* phshapeNext;
		class HPNT* phpntNext;
		class HBSK* phbskNext;
		class PIPE* ppipeNext;
		class PATHZONE* ppathzoneNext;
		class FLY* pflyNext;
		class DPRIZE* pdprizeNext;
		class RAT* pratNext;
		class RATHOLE* pratholeNext;
		class ROC* procNext;
		class ROH* prohNext;
		class ROST* prostNext;
		class DART* pdartNext;
		class SPIRE* pspireNext;
		class RAIL* prailNext;
		class LANDING* plandingNext;
		class LASEN* plasenNext;
		class FE* pfeNext;
		class EXPLSTE* pexplsteNext;
		class FADER* pfaderNext;
		class CRFOD* pcrfodNext;
		class JLOVOL* pjlovolNext;
		class EMITTER* pemitterNext;
		class MGCO* pmgcoNext;
		class JMT* pjmtNext;
	};

	union
	{
		// Stores ptr to previous data
		void* pvPrev;
		class LO* ploPrev;
		class ALO* paloPrev;
		class SO* psoPrev;
		class ASEGA* pasegaPrev;
		class ACTSEG* pactsegPrev;
		class AMB* pambPrev;
		class EXC* pexcPrev;
		class RIP* pripPrev;
		class BLIP* pblipPrev;
		class BLIPG* pblipgPrev;
		class DZ* pdzPrev;
		class TARGET* ptargetPrev;
		class HND* phndPrev;
		class ACT* pactPrev;
		class IKH* pikhPrev;
		class LIGHT* plightPrev;
		class SHADOW* pshadowPrev;
		class SHAPE* pshapePrev;
		class SMA* psmaPrev;
		class PXR* ppxrPrev;
		class HSHAPE* phshapePrev;
		class HPNT* phpntPrev;
		class HBSK* phbskPrev;
		class PIPE* ppipePrev;
		class PATHZONE* ppathzonePrev;
		class FLY* pflyPrev;
		class DPRIZE* pdprizePrev;
		class RAT* pratPrev;
		class RATHOLE* pratholePrev;
		class ROC* procPrev;
		class ROH* prohPrev;
		class ROST* prostPrev;
		class DART* pdartPrev;
		class SPIRE* pspirePrev;
		class RAIL* prailPrev;
		class LANDING* plandingPrev;
		class LASEN* plasenPrev;
		class FE* pfePrev;
		class EXPLSTE* pexplstePrev;
		class FADER* pfaderPrev;
		class CRFOD* pcrfodPrev;
		class JLOVOL* pjlovolPrev;
		class EMITTER* pemitterPrev;
		class MGCO* pmgcoPrev;
		class JMT* pjmtPrev;
	};
};

// Data list index
struct DLI
{
	// List address value
	DL* m_pdl;
	// Parent pointer value
	void** m_ppv;
	// Used for base offset to entry for parent object
	uint64_t m_ibDle;
	// Pointer to next parent object
	DLI* m_pdliNext;
};

// Initializing list
void InitDl(DL *pdl, int ibDle);
// Clearing DL list
void ClearDl(DL *pdl);
// Clearing entry linked list
void ClearDle(DLE *pdle);
// Returns entry from DL
DLE* PdleFromDlEntry(DL *pdl, void *pv);
// Appending data to end of DL
void AppendDlEntry(DL *pdl, void *pv);
// Inserts data at the front of DL list
void PrependDlEntry(DL *pdl, void *pv);
// Remove Entry from DL list
void RemoveDlEntry(DL *pdl, void *pv);
// Returns whether entry was found in DL list or not
int FFindDlEntry(DL *pdl, void *pv);
// Returns whether DL list is empty or not
int FIsDlEmpty(DL *pdl);

// Used to keep track of head node
extern DLI* s_pdliFirst;