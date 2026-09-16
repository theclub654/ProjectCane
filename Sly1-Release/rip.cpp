#include "rip.h"
#include "droplet.h"
#include "bublet.h"
#include "ripple.h"
#include "puff.h"
#include "dablet.h"
#include "flake.h"
#include "spark.h"
#include "burst.h"
#include "trail.h"
#include "fireball.h"
#include "smokecloud.h"
#include "smoketrail.h"
#include "debris.h"
#include "orbit.h"
#include "smack.h"
#include "ray.h"
#include "rose.h"
#include "flying.h"
#include "stuck.h"
#include "leaf.h"
#include "flame.h"
#include "bullet.h"
#include "shrapnel.h"
#include "glint.h"
#include "match.h"
#include "remit.h"
#include <array>
#include <algorithm>
#include <memory>


namespace
{
	constexpr size_t RIP_SLOT_HEAD = 256;
	constexpr size_t RIP_SLOT_TAIL = 256;
	constexpr size_t RIP_SLOT_SIZE = std::max({
		sizeof(RIP), sizeof(DROPLET), sizeof(BUBLET), sizeof(RIPPLE), sizeof(PUFF),
		sizeof(DABLET), sizeof(FLAKE), sizeof(SPARK), sizeof(BURST), sizeof(TRAIL),
		sizeof(FIREBALL), sizeof(SMOKECLOUD), sizeof(SMOKETRAIL), sizeof(DEBRIS),
		sizeof(ORBIT), sizeof(SMACK), sizeof(RAY), sizeof(ROSE), sizeof(FLYING),
		sizeof(STUCK), sizeof(LEAF), sizeof(FLAME), sizeof(BULLET), sizeof(SHRAPNEL),
		sizeof(GLINT), sizeof(MATCH), sizeof(REMIT) });
	constexpr size_t RIP_SLOT_ALIGNMENT = std::max({
		alignof(RIP), alignof(DROPLET), alignof(BUBLET), alignof(RIPPLE), alignof(PUFF),
		alignof(DABLET), alignof(FLAKE), alignof(SPARK), alignof(BURST), alignof(TRAIL),
		alignof(FIREBALL), alignof(SMOKECLOUD), alignof(SMOKETRAIL), alignof(DEBRIS),
		alignof(ORBIT), alignof(SMACK), alignof(RAY), alignof(ROSE), alignof(FLYING),
		alignof(STUCK), alignof(LEAF), alignof(FLAME), alignof(BULLET), alignof(SHRAPNEL),
		alignof(GLINT), alignof(MATCH), alignof(REMIT) });

	static_assert((RIP_SLOT_HEAD % RIP_SLOT_ALIGNMENT) == 0);
}

class RipPool
{
	struct alignas(RIP_SLOT_ALIGNMENT) SlotStorage
	{
		std::array<std::byte, RIP_SLOT_HEAD + RIP_SLOT_SIZE + RIP_SLOT_TAIL> bytes{};
	};

	std::array<SlotStorage, RIP_SLOT_COUNT> m_slots{};
	std::array<uint16_t, RIP_SLOT_COUNT> m_free{};
	std::array<uint32_t, RIP_SLOT_COUNT> m_generation{};
	std::array<bool, RIP_SLOT_COUNT> m_occupied{};
	size_t m_freeCount = RIP_SLOT_COUNT;

	template<class T>
	T* Construct(size_t index)
	{
		void* storage = m_slots[index].bytes.data() + RIP_SLOT_HEAD;
		return std::construct_at(reinterpret_cast<T*>(storage));
	}

	template<class T>
	static void DestroyAs(RIP* rip)
	{
		std::destroy_at(static_cast<T*>(rip));
	}

public:
	RipPool()
	{
		for (size_t i = 0; i < RIP_SLOT_COUNT; ++i)
			m_free[i] = static_cast<uint16_t>(RIP_SLOT_COUNT - 1 - i);
	}

	~RipPool()
	{
		for (size_t i = 0; i < RIP_SLOT_COUNT; ++i)
		{
			if (m_occupied[i])
				Free(reinterpret_cast<RIP*>(m_slots[i].bytes.data() + RIP_SLOT_HEAD));
		}
	}

	RIP* Allocate(RIPT ript)
	{
		if (m_freeCount == 0)
			return nullptr;

		const size_t index = m_free[--m_freeCount];
		RIP* rip = nullptr;

		switch (ript)
		{
		case RIPT_Rip:        rip = Construct<RIP>(index);        rip->pvtrip = &g_vtrip; break;
		case RIPT_Droplet:    rip = Construct<DROPLET>(index);    rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtdroplet); break;
		case RIPT_Bublet:     rip = Construct<BUBLET>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtbublet); break;
		case RIPT_Ripple:     rip = Construct<RIPPLE>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtripple); break;
		case RIPT_Puff:       rip = Construct<PUFF>(index);       rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtpuff); break;
		case RIPT_Dablet:     rip = Construct<DABLET>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtdablet); break;
		case RIPT_Flake:      rip = Construct<FLAKE>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtflake); break;
		case RIPT_Spark:      rip = Construct<SPARK>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtspark); break;
		case RIPT_Burst:      rip = Construct<BURST>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtburst); break;
		case RIPT_Trail:      rip = Construct<TRAIL>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vttrail); break;
		case RIPT_Fireball:   rip = Construct<FIREBALL>(index);   rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtfireball); break;
		case RIPT_SmokeCloud: rip = Construct<SMOKECLOUD>(index); rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtsmokecloud); break;
		case RIPT_SmokeTrail: rip = Construct<SMOKETRAIL>(index); rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtsmoketrail); break;
		case RIPT_Debris:     rip = Construct<DEBRIS>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtdebris); break;
		case RIPT_Orbit:      rip = Construct<ORBIT>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtorbit); break;
		case RIPT_Smack:      rip = Construct<SMACK>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtsmack); break;
		case RIPT_Ray:        rip = Construct<RAY>(index);        rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtray); break;
		case RIPT_Rose:       rip = Construct<ROSE>(index);       rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtrose); break;
		case RIPT_Flying:     rip = Construct<FLYING>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtflying); break;
		case RIPT_Stuck:      rip = Construct<STUCK>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtstuck); break;
		case RIPT_Leaf:       rip = Construct<LEAF>(index);       rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtleaf); break;
		case RIPT_Flame:      rip = Construct<FLAME>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtflame); break;
		case RIPT_Bullet:     rip = Construct<BULLET>(index);     rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtbullet); break;
		case RIPT_Shrapnel:   rip = Construct<SHRAPNEL>(index);   rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtshrapnel); break;
		case RIPT_Glint:      rip = Construct<GLINT>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtglint); break;
		case RIPT_Match:      rip = Construct<MATCH>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtmatch); break;
		case RIPT_Emitter:    rip = Construct<REMIT>(index);      rip->pvtrip = reinterpret_cast<VTRIP*>(&g_vtremit); break;
		default:
			m_free[m_freeCount++] = static_cast<uint16_t>(index);
			return nullptr;
		}

		rip->ript = ript;
		m_occupied[index] = true;
		++m_generation[index];
		return rip;
	}

	bool Locate(const RIP* rip, size_t* indexOut = nullptr) const
	{
		if (rip == nullptr)
			return false;

		const uintptr_t address = reinterpret_cast<uintptr_t>(rip);
		const uintptr_t firstStorage = reinterpret_cast<uintptr_t>(m_slots.data());
		if (address < firstStorage + RIP_SLOT_HEAD)
			return false;

		const uintptr_t storageAddress = address - RIP_SLOT_HEAD;
		const uintptr_t delta = storageAddress - firstStorage;
		if ((delta % sizeof(SlotStorage)) != 0)
			return false;

		const size_t index = static_cast<size_t>(delta / sizeof(SlotStorage));
		if (index >= RIP_SLOT_COUNT ||
			address != reinterpret_cast<uintptr_t>(m_slots[index].bytes.data() + RIP_SLOT_HEAD))
			return false;

		if (indexOut != nullptr)
			*indexOut = index;
		return m_occupied[index];
	}

	uint32_t Generation(const RIP* rip) const
	{
		size_t index = 0;
		return Locate(rip, &index) ? m_generation[index] : 0;
	}

	bool IsAlive(const RIP* rip, uint32_t generation) const
	{
		size_t index = 0;
		return Locate(rip, &index) && m_generation[index] == generation;
	}

	void Free(RIP* rip)
	{
		size_t index = 0;
		if (!Locate(rip, &index))
			return;

		switch (rip->ript)
		{
		case RIPT_Droplet: DestroyAs<DROPLET>(rip); break;
		case RIPT_Bublet: DestroyAs<BUBLET>(rip); break;
		case RIPT_Ripple: DestroyAs<RIPPLE>(rip); break;
		case RIPT_Puff: DestroyAs<PUFF>(rip); break;
		case RIPT_Dablet: DestroyAs<DABLET>(rip); break;
		case RIPT_Flake: DestroyAs<FLAKE>(rip); break;
		case RIPT_Spark: DestroyAs<SPARK>(rip); break;
		case RIPT_Burst: DestroyAs<BURST>(rip); break;
		case RIPT_Trail: DestroyAs<TRAIL>(rip); break;
		case RIPT_Fireball: DestroyAs<FIREBALL>(rip); break;
		case RIPT_SmokeCloud: DestroyAs<SMOKECLOUD>(rip); break;
		case RIPT_SmokeTrail: DestroyAs<SMOKETRAIL>(rip); break;
		case RIPT_Debris: DestroyAs<DEBRIS>(rip); break;
		case RIPT_Orbit: DestroyAs<ORBIT>(rip); break;
		case RIPT_Smack: DestroyAs<SMACK>(rip); break;
		case RIPT_Ray: DestroyAs<RAY>(rip); break;
		case RIPT_Rose: DestroyAs<ROSE>(rip); break;
		case RIPT_Flying: DestroyAs<FLYING>(rip); break;
		case RIPT_Stuck: DestroyAs<STUCK>(rip); break;
		case RIPT_Leaf: DestroyAs<LEAF>(rip); break;
		case RIPT_Flame: DestroyAs<FLAME>(rip); break;
		case RIPT_Bullet: DestroyAs<BULLET>(rip); break;
		case RIPT_Shrapnel: DestroyAs<SHRAPNEL>(rip); break;
		case RIPT_Glint: DestroyAs<GLINT>(rip); break;
		case RIPT_Match: DestroyAs<MATCH>(rip); break;
		case RIPT_Emitter: DestroyAs<REMIT>(rip); break;
		default: DestroyAs<RIP>(rip); break;
		}

		m_occupied[index] = false;
		++m_generation[index];
		m_free[m_freeCount++] = static_cast<uint16_t>(index);
	}

	template<class Fn>
	void ForEach(Fn&& fn) const
	{
		for (size_t i = 0; i < RIP_SLOT_COUNT; ++i)
			if (m_occupied[i])
				fn(reinterpret_cast<RIP*>(const_cast<std::byte*>(m_slots[i].bytes.data()) + RIP_SLOT_HEAD), m_generation[i]);
	}
};

namespace
{
	struct RipSnapshotEntry { RIP* rip = nullptr; uint32_t generation = 0; };
	struct RipSnapshot
	{
		std::array<RipSnapshotEntry, RIP_SLOT_COUNT> owners{};
		size_t count = 0;

		auto begin() const { return owners.begin(); }
		auto end() const { return owners.begin() + count; }
	};

	RipSnapshot SnapshotRipgRips(RIPG* pripg)
	{
		RipSnapshot snapshot;

		if (pripg == nullptr || pripg->psw == nullptr)
			return snapshot;

		if (pripg->psw->pripPool != nullptr)
			pripg->psw->pripPool->ForEach([&](RIP* rip, uint32_t generation)
			{
				if (rip->pripg == pripg)
					snapshot.owners[snapshot.count++] = { rip, generation };
			});

		return snapshot;
	}

	bool FRipStillLinked(RIPG* pripg, RIP* prip)
	{
		return pripg != nullptr && prip != nullptr &&
			(pripg->dlRip.pripFirst == prip || prip->dle.pripPrev != nullptr || prip->dle.pripNext != nullptr);
	}

	bool FRipSnapshotAlive(RIPG* pripg, const RipSnapshotEntry& entry)
	{
		return pripg != nullptr && pripg->psw != nullptr && pripg->psw->pripPool != nullptr &&
			pripg->psw->pripPool->IsAlive(entry.rip, entry.generation) &&
			FRipStillLinked(pripg, entry.rip);
	}
}

void ResetSwRipPool(SW* psw)
{
	if (psw == nullptr)
		return;
	delete psw->pripPool;
	psw->pripPool = new RipPool();
}

void DeleteSwRipPool(SW* psw)
{
	if (psw == nullptr)
		return;
	delete psw->pripPool;
	psw->pripPool = nullptr;
}

void InitRip(RIP* prip, glm::vec3* ppos, float scale, SO* psoTouch)
{
	prip->psoTouch = psoTouch;
	prip->tCreated = g_clock.t;
	prip->dtLifetime = 1.0f;

	prip->clqScale = CLQ{ scale, 0.0f, 0.0f, 0.0f };
	prip->clqAlpha = g_clqAlphaRip;

	prip->pos = *ppos;
	prip->posPrev = *ppos;
	prip->mat = glm::mat3(1.0f);

	if (prip->pripg != nullptr)
	{
		RIPG* pripg = prip->pripg;
		const float radius = glm::length(prip->pos - pripg->xf.posWorld);

		if (radius > pripg->sRadiusRenderSelf)
		{
			pripg->sRadiusRenderSelf = radius;
			pripg->sRadiusAll = radius;
			pripg->sRadiusSelf = radius;
			pripg->sRadiusRenderAll = radius;
		}
	}
}

RIPG* PripgNew(SW* psw, RIPGT ripgt)
{
	if (ripgt == RIPGT_Default && psw->pripgDefault != nullptr)
		return psw->pripgDefault;

	RIPG* pripg = psw->pripgFree;

	if (pripg == nullptr)
	{
		// The original world supplied a pool of RIPG objects. Some converted
		// worlds do not, so create one through the normal LO factory instead
		// of dropping every effect that requests the default group.
		pripg = static_cast<RIPG*>(PloNew(CID_RIPG, psw, nullptr, OID_Nil, -1));

		if (pripg == nullptr)
			return nullptr;

	}
	else
	{
		psw->pripgFree = pripg->pripgNext;
	}

	pripg->ripgt = ripgt;
	pripg->pripgNext = nullptr;

	// PloNew only constructs and registers an LO; it does not place it in the
	// world's child/update/render lists.  Newly allocated groups must be added
	// just like groups taken from the free list, otherwise their RIPs are
	// created successfully but are never updated or rendered.
	pripg->pvtlo->pfnAddLo(pripg);

	if (ripgt == RIPGT_Default)
		psw->pripgDefault = pripg;

	return pripg;
}

RIPG* NewRipg()
{
	return new RIPG{};
}

RIP* PripNewRipg(RIPT ript, RIPG* pripg)
{
	if (pripg == nullptr)
	{
		pripg = PripgNew(g_psw, RIPGT_Default);

		if (pripg == nullptr)
			return nullptr;
	}

	SW* psw = pripg->psw;

	if (psw == nullptr)
		return nullptr;

	if (psw->pripPool == nullptr)
		ResetSwRipPool(psw);

	RIP* prip = psw->pripPool->Allocate(ript);
	if (prip == nullptr)
		return nullptr;

	prip->pripg = pripg;
	prip->cref = 1;
	prip->pmqFirst = nullptr;

	AppendDlEntry(&pripg->dlRip, prip);

	return prip;
}

void InitRipg(RIPG *pripg)
{
	InitSo(pripg);
	InitDl(&pripg->dlRip, offsetof(RIP, dle));

    pripg->fNoXpsAll = true;
    pripg->fNoGravity = true;
    pripg->sMRD = 1.0e10f;
    pripg->mrds = 2;
    pripg->fNoFreeze = true;
}

int GetRipgSize()
{
	return sizeof(RIPG);
}

void CloneRipg(RIPG* pripg, RIPG* pripgBase)
{
	CloneSo(pripg, pripgBase);

	pripg->ripgt = pripgBase->ripgt;
	pripg->sExpand = pripgBase->sExpand;

	pripg->dlRip = pripgBase->dlRip;

	pripg->pripgNext = pripgBase->pripgNext;
}

void SetRipgEmitb(RIPG* pripg, EMITB* pemitb)
{
    pripg->sExpand = (pemitb->emitp).emitrip.sExpand;
}

void OnRipgRemove(RIPG* pripg)
{
	while (pripg->dlRip.ploFirst != nullptr) 
	{
		RIP* prip = pripg->dlRip.pripFirst;
		RemoveRip(prip);
	}

	pripg->pripgNext = pripg->psw->pripgFree;
	pripg->psw->pripgFree = pripg;

	if (pripg == pripg->psw->pripgDefault)
		pripg->psw->pripgDefault = nullptr;

	OnSoRemove(pripg);
}

void ProjectRipgTransform(RIPG* pripg, float dt, int fForce)
{
	pripg->posMin = glm::vec3(FLT_MAX);
	pripg->posMax = glm::vec3(-FLT_MAX);
	bool fHaveBounds = false;

	for (const RipSnapshotEntry& entry : SnapshotRipgRips(pripg))
	{
		RIP* prip = entry.rip;

		if (!FRipSnapshotAlive(pripg, entry))
			continue;

		if (prip->tCreated <= g_clock.t)
			prip->pvtrip->pfnProjectRipTransform(prip, dt);

		if (!FRipSnapshotAlive(pripg, entry))
			continue;

		pripg->posMin = glm::min(pripg->posMin, glm::min(prip->posPrev, prip->pos));
		pripg->posMax = glm::max(pripg->posMax, glm::max(prip->posPrev, prip->pos));
		fHaveBounds = true;
	}

	if (!fHaveBounds)
		return;

	if (pripg->sExpand != 0.0f) 
	{
		glm::vec3 expand(pripg->sExpand);

		pripg->posMin -= expand;
		pripg->posMax += expand;
	}

	glm::vec3 posCenter = (pripg->posMin + pripg->posMax) * 0.5f;
	pripg->pvtalo->pfnTranslateAloToPos(pripg, &posCenter);

	float radius = glm::length(pripg->posMax - posCenter);

	pripg->sRadiusRenderSelf = radius;
	pripg->sRadiusAll = radius;
	pripg->sRadiusSelf = radius;
	pripg->sRadiusRenderAll = radius;

}

void UpdateRipg(RIPG* pripg, float dt)
{
	for (const RipSnapshotEntry& entry : SnapshotRipgRips(pripg))
	{
		RIP* prip = entry.rip;

		if (!FRipSnapshotAlive(pripg, entry))
			continue;

		if (prip->tCreated < g_clock.t)
			prip->pvtrip->pfnUpdateRip(prip, dt);
	}

	if (pripg->fFrozen || FIsDlEmpty(&pripg->dlRip))
		pripg->pvtlo->pfnRemoveLo(pripg);
	else
		BounceRipgRips(pripg);

	ResolveAlo(pripg);

}

void BounceRipgRips(RIPG* pripg)
{
	if (pripg == nullptr || pripg->ripgt != RIPGT_Bounce)
		return;

	DL* pdlRip = &pripg->dlRip;
	const RipSnapshot ripSnapshot = SnapshotRipgRips(pripg);

	if (pripg->poxa == nullptr)
		return;

	for (OX* pox = pripg->poxa->pox; pox != nullptr; pox = pox->poxNext)
	{
		SO* psoRoot = pox->psoOther;

		if (psoRoot == nullptr)
			continue;

		DLI dliPhys;
		dliPhys.m_pdl = &psoRoot->dlPhys;
		dliPhys.m_ibDle = psoRoot->dlPhys.ibDle;
		dliPhys.m_ppv = reinterpret_cast<void**>(&psoRoot->dlPhys);
		dliPhys.m_pdliNext = s_pdliFirst;

		if (!psoRoot->fNoXpsAll)
		{
			SO* pso = psoRoot->dlPhys.psoFirst;

			if (pso != nullptr)
				dliPhys.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pso) + dliPhys.m_ibDle);

			s_pdliFirst = &dliPhys;

			while (pso != nullptr)
			{
				const bool isWater = (pso->pvtlo->grfcid & 0x80U) != 0;

				if (!pso->fNoXpsSelf || isWater)
				{
					BSP* pbspPruned = nullptr;
					bool checkRips = false;
					std::vector<BSP> bspPruned;

					if (pso->bspc.cbsp < 1)
						checkRips = true;
					else
					{
						bspPruned.resize(static_cast<size_t>(pso->bspc.cbspFull));
						pbspPruned = bspPruned.data();

						const PRUNE prune = PruneBsp(pso->bspc.absp.data(), &pripg->xf.posWorld, pripg->sRadiusAll, &pbspPruned);

						if (prune == PRUNE_Mixed)
						{
							--pbspPruned;
							checkRips = true;
						}
					}

					if (checkRips)
					{
						for (const RipSnapshotEntry& entry : ripSnapshot)
						{
							RIP* prip = entry.rip;

							if (!FRipSnapshotAlive(pripg, entry))
								continue;

							if (prip->tCreated < g_clock.t)
							{
								LSG alsg[4]{};

								const int clsg = ClsgClipEdgeToObjectPruned(pso, pbspPruned, &prip->posPrev, &prip->pos, 4, alsg);

								for (int ilsg = 0; ilsg < clsg; ++ilsg)
								{
									if (alsg[ilsg].au[0] == 0.0f)
										continue;

									if (prip->pvtrip->pfnFBounceRip(prip, pso, &alsg[ilsg].apos[0], &alsg[ilsg].anormal[0]))
										break;
								}
							}
						}
					}
				}

				pso = static_cast<SO*>(*dliPhys.m_ppv);

				if (pso != nullptr)
					dliPhys.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pso) + dliPhys.m_ibDle);
			}
		}

		s_pdliFirst = dliPhys.m_pdliNext;
	}

	if (FIsDlEmpty(pdlRip))
		pripg->pvtlo->pfnRemoveLo(pripg);
}

void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro)
{
	for (const RipSnapshotEntry& entry : SnapshotRipgRips(pripg))
	{
		RIP* prip = entry.rip;

		if (!FRipSnapshotAlive(pripg, entry))
			continue;

		if (prip->tCreated < g_clock.t)
			prip->pvtrip->pfnRenderRip(prip, pcm);
	}
}

void RemoveRip(RIP* prip)
{
	RemoveDlEntry(&prip->pripg->dlRip, prip);
	ReleaseRip(prip);
}

void ReleaseRip(RIP* prip)
{
	if (prip == nullptr || prip->pripg == nullptr || prip->pripg->psw == nullptr)
		return;

	SW* psw = prip->pripg->psw;
	if (psw->pripPool == nullptr || !psw->pripPool->Locate(prip))
		return;

	// Match the original slot-heap lifetime contract.  A recursive release
	// during a removal callback sees cref == 0 and must not process the RIP a
	// second time.
	--prip->cref;
	if (prip->cref != 0)
		return;

	if (prip->pvtrip != nullptr && prip->pvtrip->pfnOnRipRemove != nullptr)
		prip->pvtrip->pfnOnRipRemove(prip);

	std::shared_ptr<MQ> pmqFirst;
	auto itMq = psw->ripMqOwners.find(prip);

	if (itMq != psw->ripMqOwners.end())
	{
		pmqFirst = std::move(itMq->second);
		psw->ripMqOwners.erase(itMq);
	}

	prip->pmqFirst = nullptr;

	for (std::shared_ptr<MQ> pmq = pmqFirst; pmq != nullptr;)
	{
		std::shared_ptr <MQ> pmqNext = pmq->pmqNext;

		if (pmq->pfnmq != nullptr)
			pmq->pfnmq((LO*)pmq->pvContext, MSGID_rip_removed, prip);

		pmq = std::move(pmqNext);
	}

	if (pmqFirst != nullptr)
		FreeSwMqList(psw, pmqFirst);

	// Return the object to the shared fixed-capacity slot heap only after all
	// removal callbacks and message subscribers have finished using it.
	psw->pripPool->Free(prip);
}

void TouchRip(RIP* prip, int fTouching)
{
	if (fTouching != 0) 
		RemoveRip(prip);
}

void ForceRipFade(RIP* prip, float dtFade)
{
	float dtRemaining = prip->dtLifetime - (g_clock.t - prip->tCreated);

	if (dtFade >= dtRemaining)
		return;

	float uLifetime = dtRemaining / prip->dtLifetime;

	prip->dtLifetime = dtFade / uLifetime;
	prip->tCreated = g_clock.t - (1.0f - uLifetime) * prip->dtLifetime;
}

int FBounceRip(RIP* prip, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal)
{
	if (prip == nullptr || psoOther == nullptr || ppos == nullptr || pnormal == nullptr)
		return 0;

	float radius;
	float elasticity;
	float friction;

	if (prip->ript == RIPT_Shrapnel)
	{
		SHRAPNEL* pshrapnel = static_cast<SHRAPNEL*>(prip);

		radius = pshrapnel->sRadius;
		elasticity = pshrapnel->elas;
		friction = pshrapnel->mu;
	}
	else
	{
		if (prip->paloRender == nullptr)
			return 0;

		radius = prip->paloRender->sRadiusRenderAll;
		elasticity = 0.7f;
		friction = 0.8f;
	}

	float lifetimeRatio = 0.0f;

	if (prip->dtLifetime > 0.0f)
		lifetimeRatio = (g_clock.t - prip->tCreated) / prip->dtLifetime;

	const float scale = prip->clqScale.g0 + lifetimeRatio * (prip->clqScale.g1 + lifetimeRatio * prip->clqScale.g2);

	radius *= scale;

	// Non-water collision.
	if ((psoOther->pvtlo->grfcid & 0x80U) == 0)
	{
		const glm::vec3 collisionNormal = *pnormal;
		glm::vec3 contactOffset;

		/*
		 * Simple render objects use a spherical contact offset.
		 * Mesh-based render objects find the vertex furthest into the
		 * collision plane.
		 */
		if ((prip->paloRender->pvtlo->grfcid & 0x02U) == 0)
			contactOffset = collisionNormal * -radius;
		else
		{
			const glm::vec3 normalLocal = glm::transpose(prip->mat) * collisionNormal;

			glm::vec3 supportPoint(0.0f);
			float smallestProjection = std::numeric_limits<float>::max();
			SO* paloRender = (SO*)prip->paloRender;

			const GEOM& geometry = paloRender->geomLocal;

			for (int i = 0; i < geometry.cpos; ++i)
			{
				const glm::vec3& vertex = geometry.apos[i];
				const float projection = glm::dot(normalLocal, vertex);

				if (projection < smallestProjection)
				{
					smallestProjection = projection;
					supportPoint = vertex;
				}
			}

			contactOffset = prip->mat * supportPoint;
		}

		glm::vec3 surfaceVelocity = contactOffset;
		CalculateAloMovement(static_cast<ALO*>(psoOther), nullptr, *ppos, &surfaceVelocity, nullptr, nullptr, nullptr);

		const float effectiveRadius = glm::max(radius, 10.0f);
		const float inverseInertia = 2.5f / (effectiveRadius * effectiveRadius);

		const glm::vec3 contactAngularVelocity = glm::cross(prip->w, contactOffset);
		const glm::vec3 relativeVelocity = prip->v + contactAngularVelocity - surfaceVelocity;

		const glm::vec3 normalTorque = glm::cross(contactOffset, collisionNormal);
		const glm::vec3 normalAngularResponse = glm::cross(collisionNormal, normalTorque);
		const glm::vec3 normalResponse = collisionNormal + normalAngularResponse * inverseInertia;

		const float normalDenominator = glm::dot(normalResponse, collisionNormal);

		float normalImpulse = 0.0f;

		if (glm::abs(normalDenominator) > 0.0001f)
			normalImpulse = glm::dot(relativeVelocity, collisionNormal) * (-1.0f - elasticity) / normalDenominator;

		const glm::vec3 velocityAfterNormal = relativeVelocity + normalResponse * normalImpulse;
		const glm::vec3 tangentVelocity = velocityAfterNormal - collisionNormal * glm::dot(velocityAfterNormal, collisionNormal);

		glm::vec3 tangentDirection = g_normalX;

		const float tangentLength = glm::length(tangentVelocity);

		if (tangentLength >= 0.0001f)
			tangentDirection = tangentVelocity / tangentLength;

		const glm::vec3 frictionDirection = collisionNormal - tangentDirection * friction;
		const glm::vec3 frictionTorque = glm::cross(contactOffset, frictionDirection);
		const glm::vec3 frictionAngularResponse = glm::cross(frictionDirection, frictionTorque);
		const glm::vec3 frictionResponse = frictionDirection + frictionAngularResponse * inverseInertia;

		const float frictionDenominator = glm::dot(frictionResponse, collisionNormal);

		float frictionImpulse = 0.0f;

		if (glm::abs(frictionDenominator) > 0.0001f)
			frictionImpulse = glm::dot(relativeVelocity, collisionNormal) * (-1.0f - elasticity) / frictionDenominator;

		const glm::vec3 velocityAfterFriction = relativeVelocity + frictionResponse * frictionImpulse;

		/*
		 * Find the point on the segment between the frictionless and
		 * full-friction responses that is closest to zero velocity.
		 */
		const glm::vec3 impulseSegment = velocityAfterFriction - velocityAfterNormal;
		const float impulseSegmentLengthSq = glm::dot(impulseSegment, impulseSegment);

		float frictionAmount = 0.0f;

		if (impulseSegmentLengthSq > 0.0001f)
			frictionAmount = glm::clamp(-glm::dot(velocityAfterNormal, impulseSegment) / impulseSegmentLengthSq, 0.0f, 1.0f);

		const float appliedNormalImpulse = (1.0f - frictionAmount) * normalImpulse;
		const float appliedFrictionImpulse = frictionAmount * frictionImpulse;

		prip->v += collisionNormal * appliedNormalImpulse + frictionDirection * appliedFrictionImpulse;
		prip->w += (normalTorque * appliedNormalImpulse + frictionTorque * appliedFrictionImpulse) * inverseInertia;

		if (prip->v.z < SV_RipBounceMin)
		{
			prip->v.z = SV_RipBounceMin;
			ForceRipFade(prip, DT_RipBounceFade);
		}

		const float penetration = glm::max(glm::dot(*ppos - prip->pos, collisionNormal), 0.0f);
		prip->pos += collisionNormal * (penetration + 1.0f);
	}
	else
	{
		glm::vec3 waterCurrent;
		CalculateWaterCurrent((WATER*)psoOther, ppos, &waterCurrent, nullptr);

		prip->v = glm::mix(prip->v, waterCurrent, R_RipImpactWater);
		prip->emitdv.dv = (waterCurrent - prip->v) / DT_RipFadeWater;

		ForceRipFade(prip, DT_RipFadeWater);
		StockSplashSmall(ppos, radius * R_RipSplashRadius, psoOther);
	}

	return 1;
}
void ProjectRipTransform(RIP* prip, float dt)
{
	if (prip == nullptr)
		return;

	// Update angular velocity and orientation.
	if (!FIsZeroW(prip->w))
	{
		// rwDamping is presumably zero or negative.
		prip->w *= 1.0f + prip->rwDamping * dt;

		const float angularSpeed = glm::length(prip->w);

		if (angularSpeed > 0.0001f)
		{
			const glm::vec3 axis = prip->w / angularSpeed;
			const float angle = angularSpeed * dt;

			const glm::mat3 rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));

			// The original applies the rotation to each existing basis vector.
			prip->mat = rotation * prip->mat;
		}
	}

	// Preserve the position before advancing the RIP.
	prip->posPrev = prip->pos;

	if (!FIsZeroV(prip->v) || !FIsZeroDv(prip->emitdv.dv))
	{
		glm::mat4 velocityMatrix{ 1.0f };

		CalculateEmitdvMatrix(&prip->emitdv, dt, &velocityMatrix);

		// CalculateEmitdvMatrix produces an affine velocity transform.
		prip->v = glm::vec3(velocityMatrix * glm::vec4(prip->v, 1.0f));

		prip->pos += prip->v * dt;
	}
}

void UpdateRipgBounds(RIPG* pripg)
{

}

void UpdateRip(RIP* prip, float dt)
{
	if (prip == nullptr)
		return;

	if (g_clock.t - prip->tCreated > prip->dtLifetime)
	{
		RemoveRip(prip);
		return;
	}

	if (prip->pripg->ripgt == RIPGT_Bounce)
		return;

	SO* const psoTouch = prip->psoTouch;

	if (psoTouch == nullptr || psoTouch->bspc.absp.size() == 0)
		return;

	bool fStillTouching = false;

	if (FIsLoInWorld(psoTouch))
		fStillTouching = PbspPointInBspQuick(&prip->pos, psoTouch->bspc.absp.data()) != nullptr;

	if (prip->pvtrip->pfnTouchRip != nullptr)
		prip->pvtrip->pfnTouchRip(prip, fStillTouching);
}

int FRenderRipPosMat(RIP* prip, CM* pcm, glm::vec3* ppos, glm::mat3* pmat)
{
	if (prip == nullptr || pcm == nullptr || ppos == nullptr || pmat == nullptr)
		return 0;

	ALO* const palo = prip->paloRender;

	if (palo == nullptr || prip->dtLifetime <= 0.0f)
		return 0;

	const float uLifetime = (g_clock.t - prip->tCreated) / prip->dtLifetime;

	const float uAlpha = glm::clamp(prip->clqAlpha.g0 + uLifetime * (prip->clqAlpha.g1 + uLifetime * prip->clqAlpha.g2), 0.0f, 1.0f);
	const float scale = prip->clqScale.g0 + uLifetime * (prip->clqScale.g1 + uLifetime * prip->clqScale.g2);
	const float radius = palo->sRadiusRenderAll * scale;
	if (!SphereInFrustum(pcm->frustum, *ppos, radius))
		return 0;

	const glm::vec3 dpos = *ppos - pcm->pos;

	float uAlphaCm = 1.0f;

	if (!FInsideCmMrd(pcm, dpos, radius, palo->sMRD, uAlphaCm))
		return 0;

	if (uAlpha < 0.0001f && scale < 0.0001f)
		return 1;

	RO ro{};

	glm::vec3 vecScale(scale);
	LoadMatrixFromPosRotScale(ppos, pmat, &vecScale, &ro.model);

	ro.uAlpha = uAlpha * uAlphaCm;
	ro.uAlphaCelBorder = 1.0f;

	if (palo->pvtalo->pfnRenderAloGlobset != nullptr)
		palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);

	const float originalShadowRadius = palo->sFastShadowRadius;

	if (originalShadowRadius > 0.0f)
	{
		palo->sFastShadowRadius = originalShadowRadius * scale;
		RenderFastShadow(palo, pcm, &ro);
		palo->sFastShadowRadius = originalShadowRadius;
	}

	return 1;
}

void RenderRip(RIP* prip, CM* pcm)
{
	if (prip == nullptr || pcm == nullptr)
		return;

	glm::vec3* ppos = &prip->pos;
	glm::mat3* pmat = &prip->mat;

	glm::vec3 posWarp{};
	glm::mat3 matWarp{ 1.0f };

	if (prip->pwr != nullptr)
	{
		WarpWrTransform(prip->pwr, 50.0f, &prip->pos, &prip->mat, &posWarp, &matWarp, nullptr);

		ppos = &posWarp;
		pmat = &matWarp;
	}

	if (!FRenderRipPosMat(prip, pcm, ppos, pmat))
		RemoveRip(prip);
}

void SubscribeRipObject(RIP* prip, LO* ploTarget)
{
	if (prip == nullptr || prip->pripg == nullptr || prip->pripg->psw == nullptr || ploTarget == nullptr)
		return;

	SW* psw = prip->pripg->psw;
	std::shared_ptr<MQ>& pmqOwner = psw->ripMqOwners[prip];
	SubscribeSwPpmqStruct(psw, &pmqOwner, (PFNMQ)ploTarget->pvtlo->pfnHandleLoMessage, ploTarget);
	prip->pmqFirst = pmqOwner.get();
}

void SubscribeRipStruct(RIP* prip, PFNMQ pfnmq, void* pvContext)
{
	if (prip == nullptr || prip->pripg == nullptr || prip->pripg->psw == nullptr)
		return;

	SW* psw = prip->pripg->psw;
	std::shared_ptr<MQ>& pmqOwner = psw->ripMqOwners[prip];
	SubscribeSwPpmqStruct(psw, &pmqOwner, pfnmq, pvContext);
	prip->pmqFirst = pmqOwner.get();
}

void UnsubscribeRipStruct(RIP* prip, PFNMQ pfnmq, void* pvContext)
{
	if (prip == nullptr || prip->pripg == nullptr || prip->pripg->psw == nullptr)
		return;

	SW* psw = prip->pripg->psw;
	auto it = psw->ripMqOwners.find(prip);

	if (it == psw->ripMqOwners.end())
	{
		prip->pmqFirst = nullptr;
		return;
	}

	UnsubscribeSwPpmqStruct(psw, &it->second, pfnmq, pvContext);
	prip->pmqFirst = it->second.get();

	if (it->second == nullptr)
		psw->ripMqOwners.erase(it);
}

void EmitRips(EMITB* pemitb, EMITG* pemitg, int crip, glm::vec3* apos, glm::vec3* av, float* atCreated, float* atDestroy)
{
	ALO* paloReference = pemitb->emito.paloReference;
	EMITRIP& emitrip = pemitb->emitp.emitrip;
	RIPG* pripg = nullptr;
	if (pemitg && pemitg->ppripg)
		pripg = *pemitg->ppripg;

	if (!pripg)
	{
		pripg = PripgNew(g_psw, emitrip.ripgt);

		if (!pripg)
		{
			return;
		}

		if (pemitg && pemitg->ppripg)
			*pemitg->ppripg = pripg;

		SetRipgEmitb(pripg, pemitb);

		if (pemitg && pemitg->ploSubscribe)
			pripg->pvtlo->pfnSubscribeLoObject(pripg, pemitg->ploSubscribe);
	}

	for (int irip = 0; irip < crip; ++irip)
	{
		RIP* prip = PripNewRipg(emitrip.ript, pripg);

		if (!prip)
		{
			return;
		}

		const float gScale = GRandInRange(emitrip.lmGScale.gMin, emitrip.lmGScale.gMax);

		prip->pvtrip->pfnInitRip(prip, &apos[irip], gScale, emitrip.psoTouch);

		if (emitrip.clqScale.g1 != 0.0f || emitrip.clqScale.g2 != 0.0f)
		{
			prip->clqScale.g1 = emitrip.clqScale.g1;
			prip->clqScale.g2 = emitrip.clqScale.g2;
		}

		prip->v = av[irip];
		prip->tCreated = atCreated[irip];
		prip->dtLifetime = atDestroy[irip] - atCreated[irip];
		prip->clqAlpha = pemitb->emitv.clqAlpha;

		SetEmitdvEmitb(&prip->emitdv, pemitb);

		if (emitrip.paloRender)
			prip->paloRender = emitrip.paloRender;

		if (paloReference)
			prip->mat = glm::mat3(paloReference->xf.matWorld);

		if (emitrip.riptTrail != RIPT_Nil)
		{
			TRAIL* ptrail = static_cast<TRAIL*>(PripNewRipg(emitrip.riptTrail, nullptr));

			if (!ptrail)
				return;

			const float trailScale = prip->clqScale.g0 * 0.5f;

			ptrail->pvtrip->pfnInitRip(ptrail, &prip->pos, trailScale, nullptr);
			SetTrailTrls(ptrail, TRLS_TrackingRip, prip);
		}

		if (prip->pvtrip->pfnPostRipEmit)
			prip->pvtrip->pfnPostRipEmit(prip, pemitb);
	}

}

void DeleteRipg(RIPG* pripg)
{
	delete pripg;
}

float DT_RipFadeWater = 1.0;
float R_RipSplashRadius = 1.5;
float R_RipImpactWater = 0.5;
float SV_RipBounceMin = 200.0;
float DT_RipBounceFade = 0.25;
CLQ g_clqAlphaRip = { 1.666667, -2.666667 , 0.0};
VTRIP g_vtrip;
