/*
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "ulduar.h"

/* ScriptData
SDName: boss_kologarn
SD%Complete: 90
SDComment: TODO: Achievements
SDCategory: Ulduar
EndScriptData */

#define SPELL_ARM_DEAD_DAMAGE   RAID_MODE(63629, 63979)
#define SPELL_TWO_ARM_SMASH     RAID_MODE(63356, 64003)
#define SPELL_ONE_ARM_SMASH     RAID_MODE(63573, 64006)
#define SPELL_ARM_SWEEP         RAID_MODE(63766, 63983)
#define SPELL_STONE_SHOUT       RAID_MODE(63716, 64005)
#define SPELL_PETRIFY_BREATH    RAID_MODE(62030, 63980)
#define SPELL_STONE_GRIP        RAID_MODE(62166, 63981)
#define SPELL_STONE_GRIP_CANCEL 65594
#define SPELL_SUMMON_RUBBLE     63633
#define SPELL_FALLING_RUBBLE    63821
#define SPELL_ARM_ENTER_VEHICLE 65343
#define SPELL_ARM_ENTER_VISUAL  64753
#define SPELL_SHOCKWAVE         63783
#define SPELL_SHOCKWAVE_VISUAL  63788

#define SPELL_SUMMON_FOCUSED_EYEBEAM        63342
#define SPELL_FOCUSED_EYEBEAM_PERIODIC      RAID_MODE(63347, 63977)
#define SPELL_FOCUSED_EYEBEAM_VISUAL        63369
#define SPELL_FOCUSED_EYEBEAM_VISUAL_LEFT   63676
#define SPELL_FOCUSED_EYEBEAM_VISUAL_RIGHT  63702

// Passive
#define SPELL_KOLOGARN_REDUCE_PARRY 64651
#define SPELL_KOLOGARN_PACIFY       63726
#define SPELL_KOLOGARN_UNK_0        65219   // Not found in DBC

#define SPELL_BERSERK               47008 // guess

#define NPC_RUBBLE_STALKER          33809
#define NPC_ARM_SWEEP_STALKER       33661

#define EMOTE_EYEBEAM           "Kologarn focusing his eyes on you"
#define EMOTE_LEFT              "The Left Arm has regrown!"
#define EMOTE_RIGHT             "The Right Arm has regrown!"
#define EMOTE_STONE             "Kologarn casts Stone Grip!"

enum Events
{
    EVENT_NONE = 0,
    EVENT_INSTALL_ACCESSORIES,
    EVENT_MELEE_CHECK,
    EVENT_SMASH,
    EVENT_SWEEP,
    EVENT_SHOCKWAVE,
    EVENT_STONE_SHOUT,
    EVENT_STONE_GRIP,
    EVENT_FOCUSED_EYEBEAM,
    EVENT_RESPAWN_LEFT_ARM,
    EVENT_RESPAWN_RIGHT_ARM,
    EVENT_ENRAGE,
};

enum Yells
{
    SAY_AGGRO                                   = -1603230,
    SAY_SLAY_1                                  = -1603231,
    SAY_SLAY_2                                  = -1603232,
    SAY_LEFT_ARM_GONE                           = -1603233,
    SAY_RIGHT_ARM_GONE                          = -1603234,
    SAY_SHOCKWAVE                               = -1603235,
    SAY_GRAB_PLAYER                             = -1603236,
    SAY_DEATH                                   = -1603237,
    SAY_BERSERK                                 = -1603238,
};

enum
{
    ACHIEV_DISARMED_START_EVENT                   = 21687,
};

class boss_kologarn : public CreatureScript
{
    public:
        boss_kologarn() : CreatureScript("boss_kologarn") { }

        CreatureAI* GetAI(Creature* pCreature) const
        {
            return new boss_kologarnAI (pCreature);
        }

        struct boss_kologarnAI : public BossAI
        {
            boss_kologarnAI(Creature *pCreature) : BossAI(pCreature, TYPE_KOLOGARN), vehicle(pCreature->GetVehicleKit()),
                left(false), right(false)
            {
                ASSERT(vehicle);

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);

                DoCast(SPELL_KOLOGARN_REDUCE_PARRY);
                SetCombatMovement(false);
                Reset();

                /* should be triggered on caster? */
                SpellEntry* tempSpell;
                tempSpell = GET_SPELL(SPELL_STONE_SHOUT);
                if (tempSpell)
                    tempSpell->EffectImplicitTargetA[0] = 1;
            }

            Vehicle* vehicle;
            bool left, right;
            uint64 eyebeamTarget;

            void EnterCombat(Unit* /*who*/)
            {
                DoScriptText(SAY_AGGRO, me);

                events.ScheduleEvent(EVENT_MELEE_CHECK, 6000);
                events.ScheduleEvent(EVENT_SMASH, 5000);
                events.ScheduleEvent(EVENT_SWEEP, 19000);
                events.ScheduleEvent(EVENT_STONE_GRIP, 25000);
                events.ScheduleEvent(EVENT_FOCUSED_EYEBEAM, 21000);
				events.ScheduleEvent(EVENT_SHOCKWAVE, 12000);
                events.ScheduleEvent(EVENT_ENRAGE, 600000);

                for (uint8 i = 0; i < 2; ++i)
                    if (Unit* arm = vehicle->GetPassenger(i))
                        arm->ToCreature()->SetInCombatWithZone();

                _EnterCombat();
                me->SetReactState(REACT_AGGRESSIVE);
            }

            void Reset()
            {
                _Reset();

                eyebeamTarget = 0;
                me->SetReactState(REACT_DEFENSIVE);
            }

            void JustDied(Unit * /*victim*/)
            {
                DoScriptText(SAY_DEATH, me);
                DoCast(SPELL_KOLOGARN_PACIFY);
                me->GetMotionMaster()->MoveTargetedHome();

                for (uint8 i = 0; i < 2; ++i)
                    if (Unit* arm = vehicle->GetPassenger(i))
                        arm->ExitVehicle();

                _JustDied();
            }

            void KilledUnit(Unit* /*who*/)
            {
                DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
            }

            void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
            {
                bool isEncounterInProgress = instance->GetBossState(TYPE_KOLOGARN) == IN_PROGRESS;
                if (who->GetEntry() == NPC_LEFT_ARM)
                {
                    left = apply;
                    if (!apply && isEncounterInProgress)
                    {
                        DoScriptText(SAY_LEFT_ARM_GONE, me);
                        events.ScheduleEvent(EVENT_RESPAWN_LEFT_ARM, 40000);
                    }
                    else
                        instance->SetData64(DATA_LEFT_ARM, who->GetGUID());
                }

                else if (who->GetEntry() == NPC_RIGHT_ARM)
                {
                    right = apply;
                    if (!apply && isEncounterInProgress)
                    {
                        DoScriptText(SAY_RIGHT_ARM_GONE, me);
                        events.ScheduleEvent(EVENT_RESPAWN_RIGHT_ARM, 40000);
                    }
                    else
                        instance->SetData64(DATA_RIGHT_ARM, who->GetGUID());
                }

                if (!isEncounterInProgress)
                    return;

                if (!apply)
                {
                    who->CastSpell(me, SPELL_ARM_DEAD_DAMAGE, true);

                    if (Creature* rubbleStalker = who->FindNearestCreature(NPC_RUBBLE_STALKER, 70.0f))
                    {
                        if (rubbleStalker)
                        {
                            rubbleStalker->CastSpell(rubbleStalker, SPELL_FALLING_RUBBLE, true);
                            rubbleStalker->CastSpell(rubbleStalker, SPELL_SUMMON_RUBBLE, true);
                        }
                    }

                    if (!right && !left)
                        events.ScheduleEvent(EVENT_STONE_SHOUT, 5000);

                    if (instance)
                        instance->DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_DISARMED_START_EVENT);
                }
                else
                {
                    events.CancelEvent(EVENT_STONE_SHOUT);
                    who->ToCreature()->SetInCombatWithZone();
                }
            }

            void JustSummoned(Creature* summon)
            {
                switch (summon->GetEntry())
                {
                    case NPC_FOCUSED_EYEBEAM:
                        summon->CastSpell(me, SPELL_FOCUSED_EYEBEAM_VISUAL_LEFT, true);
                        break;
                    case NPC_FOCUSED_EYEBEAM_RIGHT:
                        summon->CastSpell(me, SPELL_FOCUSED_EYEBEAM_VISUAL_RIGHT, true);
                        break;
                    case NPC_RUBBLE:
                        summons.push_back(summon->GetGUID());
                        summon->SetInCombatWithZone();
                        // absence of break intended
                    default:
                        return;
                }

                summon->CastSpell(summon, SPELL_FOCUSED_EYEBEAM_PERIODIC, true);
                summon->CastSpell(summon, SPELL_FOCUSED_EYEBEAM_VISUAL, true);
                summon->SetReactState(REACT_PASSIVE);
                summon->SetFlag(UNIT_FIELD_ATTACK_POWER, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                // One of the above spells is a channeled spell, we need to clear this unit state for MoveChase to work
                summon->ClearUnitState(UNIT_STAT_CASTING);

                // Victim gets 67351
                if (eyebeamTarget)
                {
                    if (Unit* target = Unit::GetUnit(*summon, eyebeamTarget))
                    {
                        summon->Attack(target, false);
                        summon->GetMotionMaster()->MoveChase(target);
                    }
                }
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                switch (events.GetEvent())
                {
                    case EVENT_MELEE_CHECK:
                        if (!me->IsWithinMeleeRange(me->getVictim()))
                            DoCast(SPELL_PETRIFY_BREATH);
                        events.RepeatEvent(1000);
                        break;
                    case EVENT_SWEEP:
                        if (left)
                            DoCast(me->FindNearestCreature(NPC_ARM_SWEEP_STALKER, 500.0f, true), SPELL_ARM_SWEEP, true);
                        events.RepeatEvent(25000);
                        break;
                    case EVENT_SMASH:
                        if (left && right)
                            DoCastVictim(SPELL_TWO_ARM_SMASH);
                        else if (left || right)
                            DoCastVictim(SPELL_ONE_ARM_SMASH);
                        events.RepeatEvent(urand(8000, 10000));
                        break;
                    case EVENT_STONE_SHOUT:
                        DoCast(SPELL_STONE_SHOUT);
                        events.RepeatEvent(2000);
                        break;
                    case EVENT_ENRAGE:
                        DoCast(SPELL_BERSERK);
                        DoScriptText(SAY_BERSERK, me);
                        events.CancelEvent(EVENT_ENRAGE);
                        break;
                    case EVENT_RESPAWN_LEFT_ARM:
                    {
                        RespawnArm(NPC_LEFT_ARM);
						me->MonsterTextEmote(EMOTE_LEFT, 0, true);
                        events.CancelEvent(EVENT_RESPAWN_LEFT_ARM);
                        break;
                    }
                    case EVENT_RESPAWN_RIGHT_ARM:
                    {
                        RespawnArm(NPC_RIGHT_ARM);
						me->MonsterTextEmote(EMOTE_RIGHT, 0, true);
                        events.CancelEvent(EVENT_RESPAWN_RIGHT_ARM);
                        break;
                    }
                    case EVENT_STONE_GRIP:
                    {
                        if (right)
                        {
							me->MonsterTextEmote(EMOTE_STONE, 0, true);
                            DoCast(SPELL_STONE_GRIP);
                            DoScriptText(SAY_GRAB_PLAYER, me);
                        }
                        events.RepeatEvent(25000);
                    }
                    break;
					case EVENT_SHOCKWAVE:
						if (left)
						{
							DoScriptText(SAY_SHOCKWAVE, me);
							DoCastAOE(SPELL_SHOCKWAVE, true);
							DoCastAOE(SPELL_SHOCKWAVE_VISUAL, true);
						}
						events.RepeatEvent(urand(15000, 25000));
						break;
                    case EVENT_FOCUSED_EYEBEAM:
                        Unit* eyebeamTargetUnit = SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true);
                        if (eyebeamTargetUnit)
                        {
                            eyebeamTarget = eyebeamTargetUnit->GetGUID();
                            me->MonsterWhisper(EMOTE_EYEBEAM, eyebeamTarget, true);
                            DoCast(SPELL_SUMMON_FOCUSED_EYEBEAM);
                        }
                        events.RepeatEvent(urand(15000, 35000));
                        break;
                }

                DoMeleeAttackIfReady();
            }

            void RespawnArm(uint32 entry)
            {
                /* no way to get arms by guid as they got unsummoned in Unit::_ExitVehicle.
                    temporary spawn them here */
                if (Creature* arm = me->SummonCreature(entry, *me))
                {
                    arm->AddUnitTypeMask(UNIT_MASK_ACCESSORY);
                    // HACK: We should send spell SPELL_ARM_ENTER_VEHICLE here, but this will not work, because
                    // the aura system will not allow it to stack from two different casters
                    int32 seatId = entry == NPC_LEFT_ARM ? 0 : 1;
                    arm->CastCustomSpell(SPELL_ARM_ENTER_VEHICLE, SPELLVALUE_BASE_POINT0, seatId+1, me, true);
                    arm->CastSpell(arm, SPELL_ARM_ENTER_VISUAL, true);
                }
            }
        };
};

class npc_kologarn_arm : public CreatureScript
{
public:
    npc_kologarn_arm() : CreatureScript("npc_kologarn_arm") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_kologarn_armAI(creature);
    }

    struct npc_kologarn_armAI : public ScriptedAI
    {
        npc_kologarn_armAI(Creature* c) : ScriptedAI(c) { }

        void Reset()
        {
            me->SetReactState(REACT_DEFENSIVE);
        }

        void EnterCombat(Unit* who)
        {
            me->SetReactState(REACT_AGGRESSIVE);

            Creature* kologarn = me->GetVehicleCreatureBase();
            if (kologarn && !kologarn->isInCombat())
                kologarn->AI()->AttackStart(who);
        }
        
        void UpdateAI(uint32 const diff) { }
    };
};

class spell_ulduar_rubble_summon : public SpellScriptLoader
{
    public:
        spell_ulduar_rubble_summon() : SpellScriptLoader("spell_ulduar_rubble_summon") { }

        class spell_ulduar_rubble_summonSpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ulduar_rubble_summonSpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                uint64 originalCaster = caster->GetInstanceScript() ? caster->GetInstanceScript()->GetData64(TYPE_KOLOGARN) : 0;
                uint32 spellId = GetEffectValue();
                for (uint8 i = 0; i < 5; ++i)
                    caster->CastSpell(caster, spellId, true, NULL, NULL, originalCaster);
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_ulduar_rubble_summonSpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ulduar_rubble_summonSpellScript();
        }
};

// predicate function to select non main tank target
class StoneGripTargetSelector : public std::unary_function<Unit *, bool>
{
    public:
        StoneGripTargetSelector(Creature* me, const Unit* victim) : _me(me), _victim(victim) {}

        bool operator() (Unit* pTarget)
        {
            if (pTarget == _victim && _me->getThreatManager().getThreatList().size() > 1)
                return true;

            if (pTarget->GetTypeId() != TYPEID_PLAYER)
                return true;

            return false;
        }

        Creature* _me;
        Unit const* _victim;
};

class spell_ulduar_stone_grip_cast_target : public SpellScriptLoader
{
    public:
        spell_ulduar_stone_grip_cast_target() : SpellScriptLoader("spell_ulduar_stone_grip_cast_target") { }

        class spell_ulduar_stone_grip_cast_target_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ulduar_stone_grip_cast_target_SpellScript);

            bool Load()
            {
                if (GetCaster()->GetTypeId() != TYPEID_UNIT)
                    return false;
                return true;
            }

            void FilterTargetsInitial(std::list<Unit*>& unitList)
            {
                // Remove "main tank" and non-player targets
                unitList.remove_if(StoneGripTargetSelector(GetCaster()->ToCreature(), GetCaster()->getVictim()));
                // Maximum affected targets per difficulty mode
                uint32 maxTargets = 1;
                if (GetSpellInfo()->Id == 63981)
                    maxTargets = 3;

                // Return a random amount of targets based on maxTargets
                while (maxTargets < unitList.size())
                {
                    std::list<Unit*>::iterator itr = unitList.begin();
                    advance(itr, urand(0, unitList.size()-1));
                    unitList.erase(itr);
                }

                // For subsequent effects
                m_unitList = unitList;
            }

            void FillTargetsSubsequential(std::list<Unit*>& unitList)
            {
                unitList = m_unitList;
            }

            void HandleForceCast(SpellEffIndex i)
            {
                Player * plr = GetHitPlayer();
                if (!plr)
                    return;

                plr->CastSpell(GetTargetUnit(), GetSpellInfo()->EffectTriggerSpell[i], true);     // Don't send m_originalCasterGUID param here or underlying
                PreventHitEffect(i);                                                                   // AureEffect::HandleAuraControlVehicle will fail on caster == target
            }

            void Register()
            {
                OnUnitTargetSelect += SpellUnitTargetFn(spell_ulduar_stone_grip_cast_target_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_AREA_ENEMY_SRC);
                OnEffect += SpellEffectFn(spell_ulduar_stone_grip_cast_target_SpellScript::HandleForceCast, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
                OnUnitTargetSelect += SpellUnitTargetFn(spell_ulduar_stone_grip_cast_target_SpellScript::FillTargetsSubsequential, EFFECT_1, TARGET_UNIT_AREA_ENEMY_SRC);
                OnUnitTargetSelect += SpellUnitTargetFn(spell_ulduar_stone_grip_cast_target_SpellScript::FillTargetsSubsequential, EFFECT_2, TARGET_UNIT_AREA_ENEMY_SRC);
            }

            // Shared between effects
            std::list<Unit*> m_unitList;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ulduar_stone_grip_cast_target_SpellScript();
        }
};

class spell_ulduar_cancel_stone_grip : public SpellScriptLoader
{
    public:
        spell_ulduar_cancel_stone_grip() : SpellScriptLoader("spell_ulduar_cancel_stone_grip") { }

        class spell_ulduar_cancel_stone_gripSpellScript : public SpellScript
        {
            PrepareSpellScript(spell_ulduar_cancel_stone_gripSpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitPlayer();
                if (!target)
                    return;

                if (!target->GetVehicle())
                    return;

                switch (target->GetMap()->GetDifficulty())
                {
                    case RAID_DIFFICULTY_10MAN_NORMAL:
                        target->RemoveAura(SpellMgr::CalculateSpellEffectAmount(GetSpellInfo(), EFFECT_0));
                        break;
                    case RAID_DIFFICULTY_25MAN_NORMAL:
                        target->RemoveAura(SpellMgr::CalculateSpellEffectAmount(GetSpellInfo(), EFFECT_1));
                        break;
                    default:
                        break;
                }
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_ulduar_cancel_stone_gripSpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_ulduar_cancel_stone_gripSpellScript();
        }
};

class spell_ulduar_squeezed_lifeless : public SpellScriptLoader
{
public:
    spell_ulduar_squeezed_lifeless() : SpellScriptLoader("spell_ulduar_squeezed_lifeless") { }

    class spell_ulduar_squeezed_lifeless_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_ulduar_squeezed_lifeless_SpellScript);

        void MoveCorpse()
        {
            GetHitUnit()->ExitVehicle();
            GetHitUnit()->GetMotionMaster()->MoveJump(1756.25f + irand(-3, 3), -8.3f + irand(-3, 3), 449.0f, 10, 10);
        }

        void Register()
        {
            AfterHit += SpellHitFn(spell_ulduar_squeezed_lifeless_SpellScript::MoveCorpse);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_ulduar_squeezed_lifeless_SpellScript();
    }
};

class spell_ulduar_stone_grip_absorb : public SpellScriptLoader
{
    public:
        spell_ulduar_stone_grip_absorb() : SpellScriptLoader("spell_ulduar_stone_grip_absorb") { }

        class spell_ulduar_stone_grip_absorb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ulduar_stone_grip_absorb_AuraScript);

            //! This will be called when Right Arm (vehicle) has sustained a specific amount of damage depending on instance mode
            //! What we do here is remove all harmful aura's related and teleport to safe spot.
            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() !=  AURA_REMOVE_BY_ENEMY_SPELL)
                    return;

                if (!GetOwner()->ToCreature())
                    return;

                if (Vehicle* vehicle = GetOwner()->ToCreature()->GetVehicleKit())
                    for (uint8 i = 0; i < 3; ++i)
                        if (Unit* passenger = vehicle->GetPassenger(i))
                            passenger->ExitVehicle();
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_ulduar_stone_grip_absorb_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ulduar_stone_grip_absorb_AuraScript();
        }
};

class spell_ulduar_stone_grip : public SpellScriptLoader
{
    public:
        spell_ulduar_stone_grip() : SpellScriptLoader("spell_ulduar_stone_grip") { }

        class spell_ulduar_stone_grip_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_ulduar_stone_grip_AuraScript);

            void OnRemoveStun(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Player* pOwner = GetOwner()->ToPlayer())
                    pOwner->RemoveAurasDueToSpell(aurEff->GetAmount());
            }

            void OnRemoveVehicle(AuraEffect const* /*aurEff*/, AuraEffectHandleModes mode)
            {
                if (!(mode & AURA_EFFECT_HANDLE_REAL))
                    return;

                if (GetOwner()->GetTypeId() != TYPEID_UNIT)
                    return;

                Player* pCaster = GetCaster() ? GetCaster()->ToPlayer() : NULL;
                if (!pCaster || !pCaster->IsOnVehicle(GetOwner()->ToUnit()))
                    return;

                pCaster->RemoveAurasDueToSpell(GetId());
                pCaster->ExitVehicle();
                pCaster->GetMotionMaster()->MoveJump(1756.25f + irand(-3, 3), -8.3f + irand(-3, 3), 448.8f, 5.0f, 5.0f);
                PreventDefaultAction();
            }

            void Register()
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_ulduar_stone_grip_AuraScript::OnRemoveVehicle, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE, AURA_EFFECT_HANDLE_REAL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_ulduar_stone_grip_AuraScript::OnRemoveStun, EFFECT_2, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_ulduar_stone_grip_AuraScript();
        }
};
/*
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` IN (32930);
INSERT INTO `npc_spellclick_spells` (`npc_entry`,`spell_id`,`quest_start`,`quest_start_active`,`quest_end`,`cast_flags`,`aura_required`,`aura_forbidden`,`user_type`) VALUES
(32930,46598,0,0,0,1,0,0,0); -- Kologarn - Arm - Ride Vehicle Hardcoded
*/
void AddSC_boss_kologarn()
{
    new boss_kologarn();
    new npc_kologarn_arm();
    new spell_ulduar_rubble_summon();
    new spell_ulduar_squeezed_lifeless();
    new spell_ulduar_cancel_stone_grip();
    new spell_ulduar_stone_grip_cast_target();
    new spell_ulduar_stone_grip_absorb();
    new spell_ulduar_stone_grip();
}
