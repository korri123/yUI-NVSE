#pragma once

#include "TESBoundAnimObject.hpp"
#include "TESActorBaseData.hpp"
#include "TESContainer.hpp"
#include "BGSTouchSpellForm.hpp"
#include "TESSpellList.hpp"
#include "TESAIForm.hpp"
#include "TESHealthForm.hpp"
#include "TESAttributes.hpp"
#include "TESAnimation.hpp"
#include "TESFullName.hpp"
#include "TESModel.hpp"
#include "BGSDestructibleObjectForm.hpp"
#include "TESScriptableForm.hpp"
#include "ActorValueOwner.hpp"

class BGSBodyPartData;
class TESCombatStyle;

class TESActorBase :
	public TESBoundAnimObject, public TESActorBaseData, public TESContainer,
	public BGSTouchSpellForm, public TESSpellList, public TESAIForm, public TESHealthForm, public TESAttributes,
	public TESAnimation, public TESFullName, public TESModel, public TESScriptableForm, public ActorValueOwner, public BGSDestructibleObjectForm {
public:
	TESActorBase();
	virtual ~TESActorBase();

	virtual BGSBodyPartData*	GetBodyPartData(void);
	virtual void				Fn_61(void* arg);
	virtual TESCombatStyle*		GetCombatStyle(void);	// Result saved as ZNAM GetCombatStyle
	virtual void				SetCombatStyle(TESCombatStyle* combatStyle);
	virtual void				SetAttr(UInt32 idx, float value);	// calls Fn65
	virtual void				SetAttr(UInt32 idx, UInt32 value);
	virtual void				ModActorValue(UInt32 actorValueCode, float value);
	virtual void				Fn_67(void* arg0, UInt32 arg1);	// mod actor value?
};

ASSERT_SIZE(TESActorBase, 0x10C);