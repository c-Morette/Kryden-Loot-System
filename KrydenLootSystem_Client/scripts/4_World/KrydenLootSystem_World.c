// ---------------------------------------------------------------------
// Registros globais e furos de insercao (RNF-03: o minimo de modded sobre o vanilla).
// ---------------------------------------------------------------------

// O AddAction do storage diz "a acao aparece aqui"; este registro diz "a acao existe". Sem ele a
// acao nunca aparece e o log fica limpo (dayz-gotchas, acoes-e-ciclo-de-vida).
modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionKLS_Search);
	}
}

// Item que esta DENTRO de um storage (em qualquer nivel) nao recebe nada: sem isto o jogador
// guardaria coisa num kit medico do storage, ou juntaria municao numa pilha de dentro, e nenhum dos
// dois passa pelo CanReceiveItemIntoCargo do storage. A geracao do servidor abre a janela
// (KLS_SetAllowSystemInsert) e por isso passa. O load do restart usa CanLoad*, que fica livre.
modded class ItemBase
{
	protected bool KLS_IsLockedInsideStorage()
	{
		KLS_Storage_Base storage;
		if (!Class.CastTo(storage, GetHierarchyRoot())) return false;
		if (storage == this) return false;

		return !storage.KLS_IsSystemInsertAllowed();
	}

	override bool CanReceiveItemIntoCargo(EntityAI item)
	{
		if (KLS_IsLockedInsideStorage()) return false;
		return super.CanReceiveItemIntoCargo(item);
	}

	override bool CanReceiveAttachment(EntityAI attachment, int slotId)
	{
		if (KLS_IsLockedInsideStorage()) return false;
		return super.CanReceiveAttachment(attachment, slotId);
	}

	// Quem recebe o combine e o "this" (receiver_item.CanBeCombined(item), handscontainer.c:508).
	override bool CanBeCombined(EntityAI other_item, bool reservation_check = true, bool stack_max_limit = false)
	{
		if (KLS_IsLockedInsideStorage()) return false;
		return super.CanBeCombined(other_item, reservation_check, stack_max_limit);
	}
}

// Roupa com item dentro so entra em cargo de jogador (clothing_base.c:44). Excecao: o cargo do storage,
// para a mochila/jaqueta da pool sair cheia. O storage so aceita insercao na janela da geracao
// (KLS_Storage_Base.CanReceiveItemIntoCargo), entao o jogador nao ganha um bau para roupa cheia.
modded class Clothing
{
	override bool CanPutInCargoClothingConditions(EntityAI parent)
	{
		if (parent && parent.IsInherited(KLS_Storage_Base)) return true;
		return super.CanPutInCargoClothingConditions(parent);
	}
}
