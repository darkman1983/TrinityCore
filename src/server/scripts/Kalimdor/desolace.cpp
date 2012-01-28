/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Desolace
SD%Complete: 100
SDComment: Quest support: 5561
SDCategory: Desolace
EndScriptData */

/* ContentData
npc_aged_dying_ancient_kodo
go_iruxos
npc_dalinda_malem
go_demon_portal
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"

enum eDyingKodo
{
    // signed for 9999
    SAY_SMEED_HOME_1                = -1000348,
    SAY_SMEED_HOME_2                = -1000349,
    SAY_SMEED_HOME_3                = -1000350,

    QUEST_KODO                      = 5561,

    NPC_SMEED                       = 11596,
    NPC_AGED_KODO                   = 4700,
    NPC_DYING_KODO                  = 4701,
    NPC_ANCIENT_KODO                = 4702,
    NPC_TAMED_KODO                  = 11627,

    SPELL_KODO_KOMBO_ITEM           = 18153,
    SPELL_KODO_KOMBO_PLAYER_BUFF    = 18172,                //spells here have unclear function, but using them at least for visual parts and checks
    SPELL_KODO_KOMBO_DESPAWN_BUFF   = 18377,
    SPELL_KODO_KOMBO_GOSSIP         = 18362

};

class npc_aged_dying_ancient_kodo : public CreatureScript
{
public:
    npc_aged_dying_ancient_kodo() : CreatureScript("npc_aged_dying_ancient_kodo") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) && creature->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
        {
            //the expected quest objective
            player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());

            player->RemoveAurasDueToSpell(SPELL_KODO_KOMBO_PLAYER_BUFF);
            
            creature->GetMotionMaster()->MoveIdle();
            creature->RemoveAllAuras();
            creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            creature->CastSpell(creature, SPELL_KODO_KOMBO_GOSSIP, true);
            
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
		    return true;
            
        }
        else return false;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_aged_dying_ancient_kodoAI(creature);
    }

    struct npc_aged_dying_ancient_kodoAI : public ScriptedAI
    {
        npc_aged_dying_ancient_kodoAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

        uint32 m_uiDespawnTimer;
        uint32 m_uiTimeoutTimer;
        bool m_uiDespawn;
        bool m_uiTimeout;

        void Reset()
        {
            m_uiDespawnTimer = 60000;
            m_uiTimeoutTimer = 300000;
            m_uiDespawn = false;
            m_uiTimeout = false;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (who->GetEntry() == NPC_SMEED)
            {
                if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                    return;

                if (me->IsWithinDistInMap(who, 10.0f))
                {
                    DoScriptText(RAND(SAY_SMEED_HOME_1, SAY_SMEED_HOME_2, SAY_SMEED_HOME_3), who);
					me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }
        }

        void SpellHit(Unit* pCaster, SpellInfo const* pSpell)
        {
        	
            if (pSpell->Id == SPELL_KODO_KOMBO_GOSSIP)
            {
                m_uiDespawn = true;
                m_uiTimeout = false;
            }
            else if(pSpell->Id == SPELL_KODO_KOMBO_ITEM)
            {
            	
            	if (!pCaster->HasAura(SPELL_KODO_KOMBO_PLAYER_BUFF) && !me->HasAura(SPELL_KODO_KOMBO_DESPAWN_BUFF))
            	{
            		
            		pCaster->CastSpell(pCaster, SPELL_KODO_KOMBO_PLAYER_BUFF, true);

                	me->UpdateEntry(NPC_TAMED_KODO);
                	me->CastSpell(me, SPELL_KODO_KOMBO_DESPAWN_BUFF, false);
                	m_uiTimeout = true;
                	me->CombatStop(true);
                	me->DeleteThreatList();
                	me->GetMotionMaster()->MoveFollow(pCaster, PET_FOLLOW_DIST, me->GetFollowAngle());
            		
            	}
            	
            }
        }

        void UpdateAI(const uint32 diff)
        {
            
            if (m_uiDespawn == true && m_uiDespawnTimer <= diff)
            {
                Reset();
                me->ForcedDespawn(0);
                return;
            } else if (m_uiDespawn == true) m_uiDespawnTimer -= diff;
			
	    	if (m_uiTimeout == true && m_uiTimeoutTimer <= diff)
		    {
				Reset();
				me->ForcedDespawn(0);
				return;
			}
			else if(m_uiTimeout == true) m_uiTimeoutTimer -= diff;
			
            if (!UpdateVictim())
                return;
			
	    	DoMeleeAttackIfReady();
	    
        }
    };

};

/*######
## go_iruxos
## Hand of Iruxos
######*/

enum 
{
    QUEST_HAND_IRUXOS   = 5381,
    NPC_DEMON_SPIRIT    = 11876,
};

class go_iruxos : public GameObjectScript
{
    public:
        go_iruxos() : GameObjectScript("go_iruxos") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            if (player->GetQuestStatus(QUEST_HAND_IRUXOS) == QUEST_STATUS_INCOMPLETE && !go->FindNearestCreature(NPC_DEMON_SPIRIT, 25.0f, true))
                player->SummonCreature(NPC_DEMON_SPIRIT, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 10000);

            return true;
        }
};

/*######
## npc_dalinda_malem. Quest 1440
######*/

#define QUEST_RETURN_TO_VAHLARRIEL     1440

class npc_dalinda : public CreatureScript
{
public:
    npc_dalinda() : CreatureScript("npc_dalinda") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_RETURN_TO_VAHLARRIEL)
       {
            if (npc_escortAI* pEscortAI = CAST_AI(npc_dalinda::npc_dalindaAI, creature->AI()))
            {
                pEscortAI->Start(true, false, player->GetGUID());
                creature->setFaction(113);
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_dalindaAI(creature);
    }

    struct npc_dalindaAI : public npc_escortAI
    {
        npc_dalindaAI(Creature* creature) : npc_escortAI(creature) { }

        void WaypointReached(uint32 i)
        {
            Player* player = GetPlayerForEscort();
            switch (i)
            {
                case 1:
                    me->IsStandState();
                    break;
                case 15:
                    if (player)
                    player->GroupEventHappens(QUEST_RETURN_TO_VAHLARRIEL, me);
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/) { }

        void Reset() {}

        void JustDied(Unit* /*killer*/)
        {
            Player* player = GetPlayerForEscort();
            if (player)
                player->FailQuest(QUEST_RETURN_TO_VAHLARRIEL);
            return;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);
            if (!UpdateVictim())
                return;
            DoMeleeAttackIfReady();
        }
    };
};

/*######
## go_demon_portal
######*/

enum DemonPortal
{
    NPC_DEMON_GUARDIAN          = 11937,

    QUEST_PORTAL_OF_THE_LEGION  = 5581,
};

class go_demon_portal : public GameObjectScript
{
    public:
        go_demon_portal() : GameObjectScript("go_demon_portal") { }

        bool OnGossipHello(Player* player, GameObject* go)
        {
            if (player->GetQuestStatus(QUEST_PORTAL_OF_THE_LEGION) == QUEST_STATUS_INCOMPLETE && !go->FindNearestCreature(NPC_DEMON_GUARDIAN, 5.0f, true))
            {
                if (Creature* guardian = player->SummonCreature(NPC_DEMON_GUARDIAN, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(), 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
                    guardian->AI()->AttackStart(player);
            }

            return true;
        }
};

void AddSC_desolace()
{
    new npc_aged_dying_ancient_kodo();
    new go_iruxos();
    new npc_dalinda();
    new go_demon_portal();
}
