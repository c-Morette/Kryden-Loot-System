// ---------------------------------------------------------------------
// Acao "Vasculhar" (D-11): segurar F no storage, ~3 s, com som.
//
// Molde: ActionPackTent (sem item na mao, alvo no mundo, ContinuousInteractActionInput). A animacao
// e a de craft EM PE (CMD_ACTIONMOD_CRAFTING, loop de parte de cima do corpo, "erc,cro" em dayzplayer.c:757).
// Descartadas em 03/10/2026: a de craft de corpo inteiro agacha; a de interacao (CMD_ACTIONFB_INTERACT) tem
// som de galho embutido; a de tirar mordaca parece esvaziar panela. O som vem da categoria da acao
// (GetSoundCategory = "Rags_craft"), que a animacao toca em todo cliente.
//
// O cliente so decide se o prompt aparece; quem gera o loot e o servidor, em OnFinishProgressServer.
// Dois jogadores juntos: o primeiro que termina deixa Ruined, e o ActionConditionContinue do outro
// corta a barra no meio.
// ---------------------------------------------------------------------
class ActionKLS_SearchCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		float searchTime = KrydenLootSystemConst.DEFAULT_SEARCH_TIME;

		KLS_Storage_Base storage;
		if (m_ActionData && m_ActionData.m_Target && Class.CastTo(storage, m_ActionData.m_Target.GetObject()))
		{
			searchTime = storage.KLS_GetSearchTime();
		}

		m_ActionData.m_ActionComponent = new CAContinuousTime(searchTime);
	}
}

class ActionKLS_Search : ActionContinuousBase
{
	void ActionKLS_Search()
	{
		m_CallbackClass = ActionKLS_SearchCB;
		m_CommandUID    = DayZPlayerConstants.CMD_ACTIONMOD_CRAFTING;
		m_FullBody      = false;
		m_StanceMask    = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text          = "#STR_KLS_Action_Search";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem   = new CCINone();
		m_ConditionTarget = new CCTCursor(UAMaxDistances.DEFAULT);
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasTarget()
	{
		return true;
	}

	override bool UseMainItem()
	{
		return false;
	}

	override bool CanBeUsedInVehicle()
	{
		return false;
	}

	override string GetSoundCategory(ActionData action_data)
	{
		return KrydenLootSystemConst.SEARCH_SOUND_CATEGORY;
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		KLS_Storage_Base storage;
		if (action_data && action_data.m_Target && Class.CastTo(storage, action_data.m_Target.GetObject()))
			storage.KLS_SetBeingSearched(true);
	}

	override void OnEndServer(ActionData action_data)
	{
		KLS_Storage_Base storage;
		if (action_data && action_data.m_Target && Class.CastTo(storage, action_data.m_Target.GetObject()))
			storage.KLS_SetBeingSearched(false);

		super.OnEndServer(action_data);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (!target) return false;

		KLS_Storage_Base storage;
		if (!Class.CastTo(storage, target.GetObject())) return false;

		return storage.KLS_CanBeSearched();
	}

	override bool ActionConditionContinue(ActionData action_data)
	{
		if (!action_data || !action_data.m_Target) return false;

		KLS_Storage_Base storage;
		if (!Class.CastTo(storage, action_data.m_Target.GetObject())) return false;

		return storage.KLS_CanBeSearched();
	}

	override void OnFinishProgressServer(ActionData action_data)
	{
		super.OnFinishProgressServer(action_data);

		if (!action_data || !action_data.m_Target) return;

		KLS_Storage_Base storage;
		if (!Class.CastTo(storage, action_data.m_Target.GetObject())) return;

		storage.KLS_OnSearchedServer(action_data.m_Player);
	}
}
