// ---------------------------------------------------------------------
// Storage vasculhavel (D-06, D-08, D-20).
//
// O unico estado e a VIDA: nao-Ruined = fechado (cargo oculto, acao Vasculhar disponivel);
// Ruined = vasculhado (cargo liberado no TAB, acao some). Nao ha flag propria nem OnStoreSave: a
// vida persiste sozinha, entao um restart devolve o storage ja aberto e sem gerar loot de novo.
//
// Esconder o cargo e o par do vanilla em tentbase.c:520/551 (LockInventory/UnlockInventory com
// HIDE_INV_FROM_SCRIPT), o mesmo do bau do ZombieHordeEvent. A constante e da engine: escrita solta.
// O lock roda dos dois lados porque o cliente nao recebe o estado do servidor; cada lado recalcula a
// partir de IsRuined().
//
// A geracao do loot mora no PBO de servidor: KLS_OnSearchedServer e so a ponte, vazia aqui.
//
// Todos os storages do config.cpp derivam da classe de config KLS_Storage_Base, entao usam este
// script (a engine sobe o config ate achar uma classe de script com o nome).
// ---------------------------------------------------------------------
class KLS_Storage_Base extends Container_Base
{
	// Tempo do Vasculhar (Settings.json do servidor). Sincronizado: a barra de progresso do cliente
	// precisa terminar junto com a do servidor.
	protected float m_KLS_SearchTime;

	// Alguem esta vasculhando agora: liga o loop de som em todo cliente proximo.
	protected bool m_KLS_BeingSearched;
	protected EffectSound m_KLS_SearchSound;

	// Janela em que o SERVIDOR pode por item no cargo (a geracao). Nao e sincronizada de proposito:
	// no cliente fica sempre false, e e isso que barra o arrastar na tela de inventario.
	protected bool m_KLS_AllowSystemInsert;

	void KLS_Storage_Base()
	{
		m_KLS_SearchTime        = KrydenLootSystemConst.DEFAULT_SEARCH_TIME;
		m_KLS_AllowSystemInsert = false;

		RegisterNetSyncVariableFloat("m_KLS_SearchTime", 0.0, 600.0, 1);
		RegisterNetSyncVariableBool("m_KLS_BeingSearched");
		SetTakeable(false);
	}

	override void EEInit()
	{
		super.EEInit();
		KLS_ApplyHideState();
	}

	// O CE aplica LootDamageMin/Max no que espalha; o storage nasce inteiro (RF-03).
	override void EEOnCECreate()
	{
		super.EEOnCECreate();
		SetHealthMax("", "");
	}

	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);
		KLS_ApplyHideState();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();
		KLS_ApplyHideState();
		KLS_UpdateSearchSound();
	}

	override void EEDelete(EntityAI parent)
	{
		StopSoundSet(m_KLS_SearchSound);
		super.EEDelete(parent);
	}

	// -----------------------------------------------------------------
	// Som do Vasculhar (cliente)
	// -----------------------------------------------------------------
	protected void KLS_UpdateSearchSound()
	{
		if (GetGame().IsDedicatedServer()) return;

		if (m_KLS_BeingSearched)
		{
			if (!m_KLS_SearchSound && KrydenLootSystemConst.SEARCH_SOUNDSET != "")
				PlaySoundSetLoop(m_KLS_SearchSound, KrydenLootSystemConst.SEARCH_SOUNDSET, 0.2, 0.3);
		}
		else
		{
			StopSoundSet(m_KLS_SearchSound);
		}
	}

	// Servidor: a acao liga no inicio e desliga no fim (terminou ou foi cancelada).
	void KLS_SetBeingSearched(bool searching)
	{
		if (!GetGame().IsServer()) return;
		if (m_KLS_BeingSearched == searching) return;

		m_KLS_BeingSearched = searching;
		SetSynchDirty();
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		KLS_ApplyHideState();

		// Ruined e vazio no load: sobra de antes do restart, apaga (RF-09). Com folga, para o cargo
		// terminar de carregar antes; o TryDelete ainda reconfere IsPrepareToDelete na hora.
		if (GetGame().IsServer()) GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckForDestroy, 10000, false);
	}

	// -----------------------------------------------------------------
	// Fechado / aberto
	// -----------------------------------------------------------------
	void KLS_ApplyHideState()
	{
		if (IsRuined())
		{
			GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
		}
		else
		{
			GetInventory().LockInventory(HIDE_INV_FROM_SCRIPT);
		}
	}

	override bool CanDisplayCargo()
	{
		return IsRuined();
	}

	// -----------------------------------------------------------------
	// Vasculhar
	// -----------------------------------------------------------------
	bool KLS_CanBeSearched()
	{
		return !IsRuined();
	}

	float KLS_GetSearchTime()
	{
		if (m_KLS_SearchTime <= 0) return KrydenLootSystemConst.DEFAULT_SEARCH_TIME;
		return m_KLS_SearchTime;
	}

	// Ponte para o PBO de servidor, que gera o loot. Vazia aqui de proposito.
	void KLS_OnSearchedServer(PlayerBase player) {}

	// Servidor: liga/desliga a janela de insercao durante a geracao.
	void KLS_SetAllowSystemInsert(bool allow)
	{
		if (!GetGame().IsServer()) return;
		m_KLS_AllowSystemInsert = allow;
	}

	bool KLS_IsSystemInsertAllowed()
	{
		return m_KLS_AllowSystemInsert;
	}

	// Servidor: vira "vasculhado". Destrava explicito, sem esperar o IsRuined() atualizar.
	void KLS_MarkSearched()
	{
		if (!GetGame().IsServer()) return;

		SetHealth("", "", 0);
		GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);
		SetSynchDirty();
	}

	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionKLS_Search);
	}

	// -----------------------------------------------------------------
	// Nao e item: nao pega, nao veste, nao arrasta (RF-02)
	// -----------------------------------------------------------------
	override bool IsTakeable()
	{
		return false;
	}

	override bool CanPutInCargo(EntityAI parent)
	{
		return false;
	}

	override bool CanPutIntoHands(EntityAI parent)
	{
		return false;
	}

	override bool CanPutAsAttachment(EntityAI parent)
	{
		return false;
	}

	override bool CanSwapEntities(EntityAI otherItem, InventoryLocation otherDestination, InventoryLocation destination)
	{
		return false;
	}

	// -----------------------------------------------------------------
	// Nao aceita nada (RF-06). O load do restart usa CanLoadItemIntoCargo, que fica livre de
	// proposito (entityai.c:1573): sem isso a sobra de um storage vasculhado sumiria no restart.
	// -----------------------------------------------------------------
	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (!m_KLS_AllowSystemInsert) return false;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanSwapItemInCargo(EntityAI child_entity, EntityAI new_entity)
	{
		if (!m_KLS_AllowSystemInsert) return false;
		return super.CanSwapItemInCargo(child_entity, new_entity);
	}

	override bool CanReceiveAttachment(EntityAI attachment, int slotId)
	{
		return false;
	}

	// -----------------------------------------------------------------
	// Some quando vasculhado e vazio (RF-08). Padrao da fogueira (fireplacebase.c:648-660):
	// CheckForDestroy -> TryDelete, que espera se alguem esta mexendo no item.
	// -----------------------------------------------------------------
	override bool IsPrepareToDelete()
	{
		return IsRuined() && IsEmpty();
	}

	override void OnChildItemRemoved(InventoryItem item)
	{
		super.OnChildItemRemoved(item);
		if (GetGame().IsServer()) CheckForDestroy();
	}
}
