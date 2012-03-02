/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

/* ScriptData
SDName: Hodir
SD%Complete: 100
SDComments:
EndScriptData */

#include "ScriptPCH.h"
#include "ulduar.h"

enum Spells
{
    // Hodir
    SPELL_FROZEN_BLOWS                          = 62478,
    SPELL_FLASH_FREEZE                          = 61968,
    SPELL_FLASH_FREEZE_VISUAL                   = 62148,
    SPELL_BITING_COLD                           = 62038,
    SPELL_BITING_COLD_TRIGGERED                 = 62039,
    SPELL_BITING_COLD_DAMAGE                    = 62188,
    SPELL_FREEZE                                = 62469,
    SPELL_ICICLE                                = 62234,
    SPELL_ICICLE_SNOWDRIFT                      = 62462,
    SPELL_BLOCK_OF_ICE                          = 61969,
    SPELL_BLOCK_OF_ICE_NPC                      = 61990,
    SPELL_FROZEN_KILL                           = 62226,
    SPELL_ICICLE_FALL                           = 62453, // 69428,
    SPELL_FALL_DAMAGE                           = 62236,
    SPELL_FALL_SNOWDRIFT                        = 62460,
    SPELL_BERSERK                               = 47008,

    // Druids
    SPELL_WRATH                                 = 62793,
    SPELL_STARLIGHT                             = 62807,
    // Shamans
    SPELL_LAVA_BURST                            = 61924,
    SPELL_STORM_CLOUD                           = 65123,
    SPELL_STORM_POWER_10                        = 63711,
    SPELL_STORM_POWER_25                        = 65134,
    // Mages
    SPELL_FIREBALL                              = 61909,
    SPELL_CONJURE_FIRE                          = 62823,
    SPELL_MELT_ICE                              = 64528,
    SPELL_SINGED                                = 62821,
    // Priests
    SPELL_SMITE                                 = 61923,
    SPELL_GREATER_HEAL                          = 62809,
    SPELL_DISPEL_MAGIC                          = 63499
};

// Achievements
#define ACHIEVEMENT_CHEESE_THE_FREEZE           RAID_MODE(2961, 2962)
#define ACHIEVEMENT_COLD_IN_HERE                RAID_MODE(2967, 2968)
#define ACHIEVEMENT_THIS_CACHE_WAS_RARE         RAID_MODE(3182, 3184)
#define ACHIEVEMENT_COOLEST_FRIENDS             RAID_MODE(2963, 2965)

#define ACTION_FAILED_COOLEST_FRIENDS           1

enum Entrys
{
    ENTRY_NPC_FLASH_FREEZE_PRE                        = 32926,
    ENTRY_NPC_FLASH_FREEZE                            = 32938,
    ENTRY_NPC_ICICLE_TARGET                           = 33174,
    ENTRY_NPC_HODIR                                   = 32845
};

enum Events
{
    EVENT_NONE,
    EVENT_FREEZE,
    EVENT_FLASH_CAST,
    EVENT_FLASH_EFFECT,
    EVENT_ICICLE,
    EVENT_BLOWS,
    EVENT_RARE_CACHE,
    EVENT_BERSERK
};

enum Yells
{
    SAY_AGGRO                                   = -1603210,
    SAY_SLAY_1                                  = -1603211,
    SAY_SLAY_2                                  = -1603212,
    SAY_FLASH_FREEZE                            = -1603213,
    SAY_STALACTITE                              = -1603214,
    SAY_DEATH                                   = -1603215,
    SAY_BERSERK                                 = -1603216,
    SAY_YS_HELP                                 = -1603217,
    SAY_HARD_MODE_MISSED                        = -1603218
};

#define EMOTE_FREEZE                            "Hodir begins to cast Flash Freeze!"
#define EMOTE_BLOWS                             "Hodir gains Frozen Blows!"

struct SummonLocation
{
    float x, y, z, o;
    uint32 entry;
};

SummonLocation addLocations[]=
{
    {1983.75f, -243.36f, 432.767f, 1.57f, 32897}, // Priest 1
    {1999.90f, -230.49f, 432.767f, 1.57f, 33325}, // Druid 1
    {2010.06f, -243.45f, 432.767f, 1.57f, 33328}, // Shaman 1
    {2021.12f, -236.65f, 432.767f, 1.57f, 32893}, // Mage 1
    {2028.10f, -244.66f, 432.767f, 1.57f, 33326}, // Priest 2
    {2014.18f, -232.80f, 432.767f, 1.57f, 32901}, // Druid 2
    {1992.90f, -237.54f, 432.767f, 1.57f, 32900}, // Shaman 2
    {1976.60f, -233.53f, 432.767f, 1.57f, 33327}  // Mage 2
};

#define NORMAL_COUNT 4
#define RAID_COUNT 8


class boss_hodir : public CreatureScript
{
    public:
        boss_hodir() : CreatureScript("boss_hodir") { }

        struct boss_hodirAI : public BossAI
        {
            boss_hodirAI(Creature* creature) : BossAI(creature, TYPE_HODIR)
            {
                me->ApplySpellImmune(0, IMMUNITY_ID, 65280, true); // Singed
            }

            void Reset()
            {
                _Reset();
                me->SetReactState(REACT_PASSIVE);

                // Spawn NPC Helpers
                for (uint8 i = 0; i < RAID_MODE<uint8>(NORMAL_COUNT, RAID_COUNT); ++i)
                {
                    if (Creature* helper = me->SummonCreature(addLocations[i].entry, addLocations[i].x, addLocations[i].y, addLocations[i].z, addLocations[i].o))
                        if (Creature* iceBlock = helper->SummonCreature(ENTRY_NPC_FLASH_FREEZE_PRE, addLocations[i].x, addLocations[i].y, addLocations[i].z, addLocations[i].o))
                            helper->AddThreat(me, 5000000.0f);
                }
            }

            void KilledUnit(Unit* /*victim*/)
            {
                if (!(rand()%5))
                    DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
            }

            void JustDied(Unit* /*victim*/)
            {
                DoScriptText(SAY_DEATH, me);
                me->setFaction(35);

                if (instance)
                {
                    // Kill credit
                    instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, 64899);
                    // Getting Cold in Here
                    if (!_moreThanTwoIntenseCold)
                        instance->DoCompleteAchievement(ACHIEVEMENT_COLD_IN_HERE);
                    // Cheese the Freeze
                    if (_cheeseTheFreeze)
                        instance->DoCompleteAchievement(ACHIEVEMENT_CHEESE_THE_FREEZE);
                    // I Have the Coolest Friends
                    if (_coolestFriends)
                        instance->DoCompleteAchievement(ACHIEVEMENT_COOLEST_FRIENDS);
                    // I Could Say That This Cache Was Rare
                    if (_rareCache)
                    {
                        instance->DoCompleteAchievement(ACHIEVEMENT_THIS_CACHE_WAS_RARE);
                        instance->SetData(DATA_HODIR_RARE_CHEST, GO_STATE_READY);
                    }
                }
                _JustDied();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                DoScriptText(SAY_AGGRO, me);
                me->SetReactState(REACT_AGGRESSIVE);
                DoCast(me, SPELL_BITING_COLD, false);
                events.ScheduleEvent(EVENT_ICICLE, 2000);
                events.ScheduleEvent(EVENT_FREEZE, 25000);
                events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
                events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
                events.ScheduleEvent(EVENT_RARE_CACHE, 180000);
                events.ScheduleEvent(EVENT_BERSERK, 480000);
                _checkIntenseColdTimer = 2000;
                _moreThanTwoIntenseCold = false;
                _cheeseTheFreeze = true;
                _coolestFriends = true;
                _rareCache = true;
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim())
                    return;

                if (me->getVictim() && !me->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
                    me->Kill(me->getVictim());

                events.Update(diff);
                _DoAggroPulse(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (_checkIntenseColdTimer < diff && !_moreThanTwoIntenseCold)
                {
                    std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                    for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                    {
                        Unit* target = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                        if (!target || target->GetTypeId() != TYPEID_PLAYER)
                            continue;

                        Aura* intenseCold = target->GetAura(SPELL_BITING_COLD_TRIGGERED);
                        if (intenseCold && intenseCold->GetStackAmount() > 2)
                        {
                            _moreThanTwoIntenseCold = true;
                            break;
                        }
                    }
                    _checkIntenseColdTimer = 2000;
                }
                else
                    _checkIntenseColdTimer -= diff;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FREEZE:
                            DoCastAOE(SPELL_FREEZE);
                            events.ScheduleEvent(EVENT_FREEZE, urand(30000, 35000));
                            break;
                        case EVENT_ICICLE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                DoCast(target, SPELL_ICICLE);
                            events.ScheduleEvent(EVENT_ICICLE, 2000);
                            break;
                        case EVENT_FLASH_CAST:
                            DoScriptText(SAY_FLASH_FREEZE, me);
                            me->MonsterTextEmote(EMOTE_FREEZE, 0, true);
                            for (uint8 i = 0; i < RAID_MODE(2, 3); ++i)
                            {
                                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                    target->CastSpell(target, SPELL_ICICLE_SNOWDRIFT, true);
                            }
                            DoCast(SPELL_FLASH_FREEZE);
                            events.RescheduleEvent(EVENT_ICICLE, 15000);
                            events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
                            events.ScheduleEvent(EVENT_FLASH_EFFECT, 9000);
                            break;
                        case EVENT_FLASH_EFFECT:
                            DoCast(SPELL_FLASH_FREEZE_VISUAL);
                            FlashFreeze();
                            events.CancelEvent(EVENT_FLASH_EFFECT);
                            break;
                        case EVENT_BLOWS:
                            DoScriptText(SAY_STALACTITE, me);
                            me->MonsterTextEmote(EMOTE_BLOWS, 0, true);
                            DoCast(me, SPELL_FROZEN_BLOWS);
                            events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
                            break;
                        case EVENT_RARE_CACHE:
                            DoScriptText(SAY_HARD_MODE_MISSED, me);
                            _rareCache = false;
                            //instance->SetData(DATA_HODIR_RARE_CACHE, 0);
                            events.CancelEvent(EVENT_RARE_CACHE);
                            break;
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK, true);
                            DoScriptText(SAY_BERSERK, me);
                            events.CancelEvent(EVENT_BERSERK);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            void FlashFreeze()
            {
                DoZoneInCombat();
                std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                {
                    if (Unit* target = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                    {
                        if (!target->ToPlayer())
                            continue;

                        if (target->HasAura(SPELL_BLOCK_OF_ICE))
                        {
                            DoCast(target, SPELL_FROZEN_KILL);
                            continue;
                        }
                        else
                        {
                            if (GetClosestCreatureWithEntry(target, ENTRY_NPC_ICICLE_TARGET, 5.0f))
                                continue;
                            else if (Creature* iceBlock = target->SummonCreature(ENTRY_NPC_FLASH_FREEZE, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 105000))
                            {
                                if (target->GetTypeId() == TYPEID_PLAYER)
                                    _cheeseTheFreeze = false;
                            }
                        }
                    }
                }
            }

            void DoAction(int32 const action)
            {
                switch (action)
                {
                    case ACTION_FAILED_COOLEST_FRIENDS:
                        _coolestFriends = false;
                        break;
                }
            }

        private:
            uint32 _checkIntenseColdTimer;
            bool _moreThanTwoIntenseCold;
            bool _cheeseTheFreeze;
            bool _coolestFriends;
            bool _rareCache;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new boss_hodirAI(creature);
        }
};


class npc_icicle : public CreatureScript
{
    public:
        npc_icicle() : CreatureScript("npc_icicle") { }

        struct npc_icicleAI : public Scripted_NoMovementAI
        {
            npc_icicleAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(28470);
            }

            void Reset()
            {
                _icicleTimer = 2000;
            }

            void UpdateAI(uint32 const diff)
            {
                if (_icicleTimer <= diff)
                {
                    DoCast(me, SPELL_FALL_DAMAGE);
                    DoCast(me, SPELL_ICICLE_FALL);
                    _icicleTimer = 10000;
                }
                else
                    _icicleTimer -= diff;
            }

        private:
            uint32 _icicleTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_icicleAI(creature);
        }
};

class npc_icicle_snowdrift : public CreatureScript
{
    public:
        npc_icicle_snowdrift() : CreatureScript("npc_icicle_snowdrift") { }

        struct npc_icicle_snowdriftAI : public Scripted_NoMovementAI
        {
            npc_icicle_snowdriftAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(28470);
            }

            void Reset()
            {
                _icicleTimer = 2000;
            }

            void UpdateAI(uint32 const diff)
            {
                if (_icicleTimer <= diff)
                {
                    DoCast(me, SPELL_FALL_SNOWDRIFT);
                    DoCast(me, SPELL_ICICLE_FALL);
                    _icicleTimer = 10000;
                }
                else
                    _icicleTimer -= diff;
            }

        private:
            uint32 _icicleTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_icicle_snowdriftAI(creature);
        }
};

class npc_snowpacked_icicle : public CreatureScript
{
    public:
        npc_snowpacked_icicle() : CreatureScript("npc_snowpacked_icicle") { }

        struct npc_snowpacked_icicleAI : public Scripted_NoMovementAI
        {
            npc_snowpacked_icicleAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(15880);
            }

            void Reset()
            {
                _despawnTimer = 12000;
            }

            void UpdateAI(uint32 const diff)
            {
                if (_despawnTimer <= diff)
                {
                    if (GameObject* snowdrift = me->FindNearestGameObject(194173, 2.0f))
                        me->RemoveGameObject(snowdrift, true);
                    me->DespawnOrUnsummon();
                }
                else _despawnTimer -= diff;
            }

        private:
            uint32 _despawnTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_snowpacked_icicleAI(creature);
        }
};

class npc_hodir_priest : public CreatureScript
{
    public:
        npc_hodir_priest() : CreatureScript("npc_hodir_priest") { }

        struct npc_hodir_priestAI : public ScriptedAI
        {
            npc_hodir_priestAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = creature->GetInstanceScript();
                me->setFaction(1665);
            }

            void Reset()
            {
                _healTimer = urand(4000, 8000);
                _dispelTimer = urand(20000, 30000);
            }

            void AttackStart(Unit* who)
            {
                AttackStartCaster(who, 20.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                if (HealthBelowPct(35))
                    DoCastAOE(SPELL_GREATER_HEAL, true);

                if (_healTimer <= diff)
                {
                    DoCastAOE(SPELL_GREATER_HEAL, true);
                    _healTimer = urand(12000, 14000);
                }
                else
                    _healTimer -= diff;

                if (_dispelTimer < diff)
                {
                    std::list<Player*> players;
                    Trinity::AnyPlayerInObjectRangeCheck checker(me, 30.0f);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                    me->VisitNearbyWorldObject(30.0f, searcher);
                    if (!players.empty())
                    {
                        for (std::list<Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
                        {
                            if ((*iter)->HasAura(SPELL_FREEZE))
                            {
                                DoCast((*iter), SPELL_DISPEL_MAGIC, true);
                                _dispelTimer = urand(25000, 30000);
                                return;
                            }
                        }
                    }
                    _dispelTimer = 5000;
                }
                else
                    _dispelTimer -= diff;

                DoSpellAttackIfReady(SPELL_SMITE);
            }

            void JustDied(Unit* /*victim*/)
            {
                // I Have the Coolest Friends
                if (Creature* hodir = me->FindNearestCreature(NPC_HODIR, 250.0f, true))
                    hodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
            }

        private:
            InstanceScript* _instance;
            uint32 _healTimer;
            uint32 _dispelTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_hodir_priestAI(creature);
        }
};

class npc_hodir_shaman : public CreatureScript
{
    public:
        npc_hodir_shaman() : CreatureScript("npc_hodir_shaman") { }

        struct npc_hodir_shamanAI : public ScriptedAI
        {
            npc_hodir_shamanAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = creature->GetInstanceScript();
                me->setFaction(1665);
            }

            void Reset()
            {
                _stormTimer = urand(15000, 20000);
            }

            void AttackStart(Unit* who)
            {
                AttackStartCaster(who, 20.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                if (_stormTimer <= diff)
                {
                    std::list<Player*> players;
                    Trinity::AnyPlayerInObjectRangeCheck checker(me, 30.0f);
                    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                    me->VisitNearbyWorldObject(30.0f, searcher);
                    if (!players.empty())
                    {
                        std::list<Player*>::iterator iter = players.begin();
                        DoCast((*iter), SPELL_STORM_CLOUD, true);
                    }
                    _stormTimer = urand(15000, 20000);
                }
                else
                    _stormTimer -= diff;

                DoSpellAttackIfReady(SPELL_LAVA_BURST);
            }

            void JustDied(Unit* /*victim*/)
            {
                // I Have the Coolest Friends
                if (Creature* hodir = me->FindNearestCreature(NPC_HODIR, 250.0f, true))
                    hodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
            }

        private:
            InstanceScript* _instance;
            uint32 _stormTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_hodir_shamanAI(creature);
        }
};

class npc_hodir_druid : public CreatureScript
{
    public:
        npc_hodir_druid() : CreatureScript("npc_hodir_druid") { }

        struct npc_hodir_druidAI : public ScriptedAI
        {
            npc_hodir_druidAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = creature->GetInstanceScript();
                me->setFaction(1665);
            }

            void Reset()
            {
                _starlightTimer = urand(10000, 15000);
            }

            void AttackStart(Unit* who)
            {
                AttackStartCaster(who, 20.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                if (_starlightTimer <= diff)
                {
                    DoCast(me, SPELL_STARLIGHT, true);
                    _starlightTimer = urand(20000, 25000);
                }
                else
                    _starlightTimer -= diff;

                DoSpellAttackIfReady(SPELL_WRATH);
            }

            void JustDied(Unit* /*victim*/)
            {
                // I Have the Coolest Friends
                if (Creature* hodir = me->FindNearestCreature(NPC_HODIR, 250.0f, true))
                    hodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
            }

        private:
            InstanceScript* _instance;
            uint32 _starlightTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_hodir_druidAI(creature);
        }
};

class npc_hodir_mage : public CreatureScript
{
    public:
        npc_hodir_mage() : CreatureScript("npc_hodir_mage") { }

        struct npc_hodir_mageAI : public ScriptedAI
        {
            npc_hodir_mageAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = creature->GetInstanceScript();
                me->setFaction(1665);
            }

            void Reset()
            {
                _fireTimer = urand(15000, 20000);
                _meltIceTimer = 5000;
            }

            void AttackStart(Unit* who)
            {
                AttackStartCaster(who, 20.0f);
            }

            void UpdateAI(uint32 const diff)
            {
                if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                if (_fireTimer <= diff)
                {
                    DoCast(me, SPELL_CONJURE_FIRE, true);
                    _fireTimer = urand(25000, 30000);
                }
                else
                    _fireTimer -= diff;

                if (_meltIceTimer < diff)
                {
                    if (Creature* shard = me->FindNearestCreature(ENTRY_NPC_FLASH_FREEZE, 50.0f, true))
                    {
                        DoCast(shard, SPELL_MELT_ICE, true);
                        _meltIceTimer = urand(5000,10000);
                    }
                    _meltIceTimer = 5000;
                }
                else
                    _meltIceTimer -= diff;

                DoSpellAttackIfReady(SPELL_FIREBALL);
            }

            void JustDied(Unit* /*victim*/)
            {
                // I Have the Coolest Friends
                if (Creature* hodir = me->FindNearestCreature(NPC_HODIR, 250.0f, true))
                    hodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
            }

        private:
            InstanceScript* _instance;
            uint32 _fireTimer;
            uint32 _meltIceTimer;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_hodir_mageAI(creature);
        }
};

class npc_toasty_fire : public CreatureScript
{
    public:
        npc_toasty_fire() : CreatureScript("npc_toasty_fire") { }

        struct npc_toasty_fireAI : public Scripted_NoMovementAI
        {
            npc_toasty_fireAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(15880);
            }

            void Reset()
            {
                DoCast(me, SPELL_SINGED, true);
            }

            void SpellHit(Unit* /*who*/, SpellInfo const* spell)
            {
                // Toasty fire can be extinguished by falling ice or Flash Freeze
                if (spell->Id == SPELL_BLOCK_OF_ICE || spell->Id == 62457 || spell->Id == 65370)
                {
                    if (GameObject* fire = me->FindNearestGameObject(194300, 4.0f))
                        me->RemoveGameObject(fire, true);
                    me->DespawnOrUnsummon();
                }
            }

            void JustDied(Unit* /*victim*/)
            {
                if (GameObject* fire = me->FindNearestGameObject(194300, 4.0f))
                    me->RemoveGameObject(fire, true);
            }

        private:
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_toasty_fireAI(creature);
        }
};

class npc_flash_freeze : public CreatureScript
{
    public:
        npc_flash_freeze() : CreatureScript("npc_flash_freeze") { }

        struct npc_flash_freezeAI : public Scripted_NoMovementAI
        {
            npc_flash_freezeAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_FLAG_PACIFIED);
                me->SetReactState(REACT_PASSIVE);
                me->SetDisplayId(me->GetCreatureTemplate()->Modelid2);
                me->setFaction(14);
            }

            void Reset()
            {
                if (me->ToTempSummon()->GetSummoner())
                {
                    if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE)
                    {
                        DoCast(SPELL_BLOCK_OF_ICE_NPC);
                        me->ToTempSummon()->GetSummoner()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    }
                    else
                        DoCast(SPELL_BLOCK_OF_ICE);

                    me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (!me->ToTempSummon()->GetSummoner())
                {
                    me->DisappearAndDie();
                    return;
                }

                if (me->ToTempSummon()->GetSummoner()->isDead())
                {
                    me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
                    me->DisappearAndDie();
                    return;
                }

                if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE && !me->ToTempSummon()->GetSummoner()->HasAura(SPELL_BLOCK_OF_ICE_NPC))
                {
                    DoCast(SPELL_BLOCK_OF_ICE_NPC);
                    me->ToTempSummon()->GetSummoner()->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
            }

            void DamageTaken(Unit* attacker, uint32 &damage)
            {
                if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE)
                {
                    if (attacker && attacker->GetTypeId() == TYPEID_PLAYER)
                        if (Creature* hodir = me->FindNearestCreature(NPC_HODIR, 200.0f, true))
                            if (!hodir->isInCombat())
                                hodir->AI()->AttackStart(attacker);

                    me->ToTempSummon()->GetSummoner()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                }
            }

            void JustDied(Unit * /*victim*/)
            {
                if (me->ToTempSummon()->GetSummoner())
                    me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
            }

        private:
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_flash_freezeAI(creature);
        }
};

class spell_biting_cold : public SpellScriptLoader
{
    public:
        spell_biting_cold() : SpellScriptLoader("spell_biting_cold") { }

        class spell_biting_cold_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_biting_cold_AuraScript)

            void HandlePeriodicDummy(AuraEffect const* aurEff)
            {
                PreventDefaultAction();
                if (Unit* trigger = GetTarget())
                {
                    if (aurEff->GetSpellInfo()->Id == SPELL_BITING_COLD)
                    {
                        if (trigger->ToPlayer())
                            if (!trigger->HasAura(62821)) // Not Triggered if in Toasty Fire
                                trigger->CastSpell(trigger, SPELL_BITING_COLD_TRIGGERED, true, 0, 0, GetCasterGUID());
                    }
                    else if (aurEff->GetSpellInfo()->Id == SPELL_BITING_COLD_TRIGGERED)
                    {
                        int32 damage = trigger->GetAuraCount(SPELL_BITING_COLD_TRIGGERED);
                        damage *= 400;
                        trigger->CastCustomSpell(trigger, SPELL_BITING_COLD_DAMAGE, &damage, NULL, NULL, true, 0, 0, GetCasterGUID());
                    }
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_biting_cold_AuraScript::HandlePeriodicDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_biting_cold_AuraScript();
        }
};

// TODO: fixme
class spell_hodir_toasty_fire : public SpellScriptLoader
{
    public:
        spell_hodir_toasty_fire() : SpellScriptLoader("spell_hodir_toasty_fire") { }

        class spell_hodir_toasty_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hodir_toasty_fire_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target || !target->ToPlayer())
                    return;

                target->ToPlayer()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, GetId());
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_hodir_toasty_fire_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_STAT, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hodir_toasty_fire_AuraScript();
        }
};

// TODO: fixme
class spell_hodir_starlight : public SpellScriptLoader
{
    public:
        spell_hodir_starlight() : SpellScriptLoader("spell_hodir_starlight") { }

        class spell_hodir_starlight_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hodir_starlight_AuraScript);

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                Unit* target = GetTarget();
                if (!target || !target->ToPlayer())
                    return;

                target->ToPlayer()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, GetId());
            }

            void Register()
            {
                OnEffectApply += AuraEffectApplyFn(spell_hodir_starlight_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MELEE_SLOW, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_hodir_starlight_AuraScript();
        }
};

class achievement_staying_buffed_all_winter : public AchievementCriteriaScript
{
   public:
       achievement_staying_buffed_all_winter() : AchievementCriteriaScript("achievement_staying_buffed_all_winter") { }

       bool OnCheck(Player* player, Unit* /*target*/)
       {
           if (player->HasAura(SPELL_SINGED) && player->HasAura(SPELL_STARLIGHT) && (player->HasAura(SPELL_STORM_POWER_10) || player->HasAura(SPELL_STORM_POWER_25)))
               return true;

           return false;
       }
};

/*
-- Hodir
UPDATE `creature_template` SET `mechanic_immune_mask` = 650854239, `flags_extra` = 1, `ScriptName` = 'boss_hodir' WHERE `entry` = 32845;
UPDATE `creature_template` SET `equipment_id` = 1843, `mechanic_immune_mask` = 650854239, `flags_extra` = 1 WHERE `entry` = 32846;

-- Hodir npcs
UPDATE `creature_template` SET `ScriptName` = 'npc_hodir_priest' WHERE `entry` IN (32897, 33326, 32948, 33330);
UPDATE `creature_template` SET `ScriptName` = 'npc_hodir_shaman' WHERE `entry` IN (33328, 32901, 33332, 32950);
UPDATE `creature_template` SET `ScriptName` = 'npc_hodir_druid' WHERE `entry` IN (33325, 32900, 32941, 33333);
UPDATE `creature_template` SET `ScriptName` = 'npc_hodir_mage' WHERE `entry` IN (32893, 33327, 33331, 32946);
UPDATE `creature_template` SET `ScriptName` = 'npc_toasty_fire' WHERE `entry` = 33342;
UPDATE `creature_template` SET `ScriptName` = 'npc_icicle' WHERE `entry` = 33169;
UPDATE `creature_template` SET `ScriptName` = 'npc_icicle_snowdrift' WHERE `entry` = 33173;
UPDATE `creature_template` SET `ScriptName` = 'npc_snowpacked_icicle' WHERE `entry` = 33174;
UPDATE `creature_template` SET `difficulty_entry_1` = 33352, `mechanic_immune_mask` = 612597599, `ScriptName` = 'npc_flash_freeze' WHERE `entry` = 32926;
UPDATE `creature_template` SET `difficulty_entry_1` = 33353, `mechanic_immune_mask` = 612597599, `ScriptName` = 'npc_flash_freeze' WHERE `entry` = 32938;
UPDATE `creature_template` SET `mechanic_immune_mask` = 612597599 WHERE `entry` IN (33352, 33353);
UPDATE `gameobject_template` SET `flags` = 4 WHERE `entry` = 194173;

DELETE FROM spell_script_names WHERE spell_id IN (62038,62039);
INSERT INTO spell_script_names (spell_id,Scriptname)
VALUES
(62038,'spell_biting_cold'),
(62039,'spell_biting_cold');

-- Cleanup
DELETE FROM `creature` WHERE `id` IN (32950, 32941, 32948, 32946, 32938);
*/
void AddSC_boss_hodir()
{
    new boss_hodir();
    new npc_icicle();
    new npc_icicle_snowdrift();
    new npc_snowpacked_icicle();
    new npc_hodir_priest();
    new npc_hodir_shaman();
    new npc_hodir_druid();
    new npc_hodir_mage();
    new npc_toasty_fire();
    new npc_flash_freeze();
    new spell_biting_cold();
    new spell_hodir_starlight();
    new spell_hodir_toasty_fire();
    new achievement_staying_buffed_all_winter();
}
