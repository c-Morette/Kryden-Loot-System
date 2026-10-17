// ---------------------------------------------------------------------
// KrydenLootSystem (Server/4_World): pools, sorteio e geracao do loot (D-19).
//
// Fluxo: a acao termina no servidor -> KLS_Storage_Base.KLS_OnSearchedServer (ponte, modded aqui)
// -> KLS_Loot.Search -> abre a janela de insercao, sorteia e cria, fecha a janela -> Ruined.
// ---------------------------------------------------------------------

// =====================================================================
// Ponte do storage (o Client declara vazia)
// =====================================================================
modded class KLS_Storage_Base
{
	override void EEInit()
	{
		super.EEInit();

		if (!GetGame().IsServer()) return;

		// Manda o tempo do Settings.json para o cliente, que precisa dele na barra de progresso.
		KLS_Settings settings = KLS_Loot.Get().GetSettings();
		if (!settings) return;

		m_KLS_SearchTime = settings.SearchTime;
		SetSynchDirty();
	}

	// Diagnostico do ciclo de vida (so com DebugLog): quem o CE cria, o que volta do load e quem apaga.
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (!GetGame().IsServer()) return;

		KLS_Loot.Get();
		KLS_Log.Debug(string.Format("CE created %1 at %2", GetType(), GetPosition()));
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();

		if (!GetGame().IsServer()) return;

		// Uma linha de resumo no boot, nao uma por storage (eram ~3 mil linhas com o template, 03/10/2026).
		KLS_Loot.Get().NoteLoadedFromStorage(IsRuined(), KLS_CargoCount());
	}

	override bool TryDelete()
	{
		if (GetGame().IsServer() && IsPrepareToDelete())
		{
			KLS_Log.Debug(string.Format("KLS deleting %1 at %2 (ruined and empty)", GetType(), GetPosition()));
		}

		return super.TryDelete();
	}

	override void EEDelete(EntityAI parent)
	{
		if (GetGame().IsServer())
		{
			KLS_Log.Debug(string.Format("Deleted: %1 at %2 ruined=%3 items=%4 lifetime=%5", GetType(), GetPosition(), IsRuined(), KLS_CargoCount(), GetLifetime()));
		}

		super.EEDelete(parent);
	}

	protected int KLS_CargoCount()
	{
		CargoBase cargo = GetInventory().GetCargo();
		if (!cargo) return 0;
		return cargo.GetItemCount();
	}

	override void KLS_OnSearchedServer(PlayerBase player)
	{
		super.KLS_OnSearchedServer(player);

		if (!GetGame().IsServer()) return;

		// Outro jogador terminou antes: ja e Ruined, nao gera de novo.
		if (!KLS_CanBeSearched()) return;

		KLS_Loot.Get().Search(this, player);
	}
}

// =====================================================================
// Singleton: carrega no boot (MissionServer.OnInit) ou no primeiro uso, o que vier antes.
// =====================================================================
class KLS_Loot
{
	protected static ref KLS_Loot s_Instance;

	protected ref KLS_LootRepository m_Repository;
	protected ref KLS_LootGenerator m_Generator;
	protected ref map<string, bool> m_NoEntryReported;

	static KLS_Loot Get()
	{
		if (!s_Instance)
		{
			s_Instance = new KLS_Loot();
			s_Instance.Load();
		}

		return s_Instance;
	}

	static void Destroy()
	{
		if (s_Instance && GetGame())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(s_Instance.LogLoadSummary);

		s_Instance = null;
	}

	void KLS_Loot()
	{
		m_Repository      = new KLS_LootRepository();
		m_Generator       = new KLS_LootGenerator();
		m_NoEntryReported = new map<string, bool>();
	}

	// Resumo dos storages que voltaram da persistencia: conta durante o load e loga uma vez, 30 s depois.
	protected int m_LoadedTotal;
	protected int m_LoadedRuined;
	protected int m_LoadedWithItems;
	protected bool m_LoadSummaryPending;

	void NoteLoadedFromStorage(bool ruined, int itemCount)
	{
		m_LoadedTotal++;
		if (ruined) m_LoadedRuined++;
		if (itemCount > 0) m_LoadedWithItems++;

		if (m_LoadSummaryPending) return;

		m_LoadSummaryPending = true;
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LogLoadSummary, 30000, false);
	}

	void LogLoadSummary()
	{
		m_LoadSummaryPending = false;
		KLS_Log.Info(string.Format("Storages loaded from persistence: %1 (ruined %2, with items %3)", m_LoadedTotal, m_LoadedRuined, m_LoadedWithItems));

		m_LoadedTotal     = 0;
		m_LoadedRuined    = 0;
		m_LoadedWithItems = 0;
	}

	void Load()
	{
		m_Repository.Load();

		KLS_Settings settings = m_Repository.GetSettings();
		if (settings && settings.ExportItems) KLS_ItemExport.Run();
	}

	KLS_Settings GetSettings()
	{
		return m_Repository.GetSettings();
	}

	void Search(KLS_Storage_Base storage, PlayerBase player)
	{
		if (!storage) return;

		int created = 0;
		string report = "";

		KLS_StorageDef storageDef = m_Repository.FindStorageDef(storage);
		if (storageDef)
		{
			// Cargo trancado recusa insercao e CreateInInventory devolve null sem erro: destrava antes.
			storage.KLS_SetAllowSystemInsert(true);
			storage.GetInventory().UnlockInventory(HIDE_INV_FROM_SCRIPT);

			created = m_Generator.Fill(storage, storageDef, m_Repository, report);

			storage.KLS_SetAllowSystemInsert(false);
		}
		else if (!m_NoEntryReported.Contains(storage.GetType()))
		{
			m_NoEntryReported.Set(storage.GetType(), true);
			KLS_Log.Warn("No entry in Storages.json for " + storage.GetType() + ". It opens empty.");
		}

		storage.KLS_MarkSearched();

		// Lote vazio nao fica "vivo" (RF-10): Ruined e vazio, o CheckForDestroy apaga.
		if (created == 0) storage.CheckForDestroy();

		if (KLS_Log.s_Debug)
		{
			string who = "unknown";
			if (player && player.GetIdentity()) who = player.GetIdentity().GetName();

			KLS_Log.Debug(string.Format("%1 searched %2 at %3: %4 item(s) [%5]", who, storage.GetType(), storage.GetPosition(), created, report));
		}
	}
}

// =====================================================================
// Leitura dos JSON do admin
// =====================================================================
class KLS_LootRepository
{
	protected ref KLS_Settings m_Settings;
	protected ref map<string, ref KLS_PoolFile> m_Pools;
	protected ref array<ref KLS_StorageDef> m_StorageDefs;

	// Classname ja avisado: um aviso por classname no boot, nunca em loop (RF-27).
	protected ref map<string, bool> m_Reported;

	// Maior Weight de cada item nas pools (= nominal vanilla no template). Usado para sortear o carregador da
	// propria arma: o tambor de 75 sai raro como no chao (04/10/2026).
	protected ref map<string, float> m_ItemWeights;

	// Kit padrao de cada item (Kits.json, D-29).
	protected ref map<string, ref KLS_ItemKit> m_Kits;

	void KLS_LootRepository()
	{
		m_Settings    = new KLS_Settings();
		m_Pools       = new map<string, ref KLS_PoolFile>();
		m_StorageDefs = new array<ref KLS_StorageDef>();
		m_Reported    = new map<string, bool>();
		m_ItemWeights = new map<string, float>();
		m_Kits        = new map<string, ref KLS_ItemKit>();
	}

	// null = item sem kit: sai so com o que a propria linha traz.
	KLS_ItemKit GetKit(string className)
	{
		if (!m_Kits.Contains(className)) return null;
		return m_Kits.Get(className);
	}

	// 0 = item fora das pools.
	float GetItemWeight(string className)
	{
		if (!m_ItemWeights.Contains(className)) return 0;
		return m_ItemWeights.Get(className);
	}

	void Load()
	{
		KLS_Defaults.EnsureFiles();

		LoadSettings();
		LoadKits();
		LoadPools();
		LoadStorages();

		KLS_Log.Info(string.Format("Loaded %1 pool(s), %2 kit(s) and %3 storage entry(ies) from %4.", m_Pools.Count(), m_Kits.Count(), m_StorageDefs.Count(), KLS_ServerConst.MKDIR_ROOT));
	}

	KLS_Settings GetSettings()
	{
		return m_Settings;
	}

	KLS_PoolFile GetPool(string poolName)
	{
		if (!m_Pools.Contains(poolName)) return null;
		return m_Pools.Get(poolName);
	}

	// Entrada exata primeiro; depois a primeira cuja classe e mae desta (KLS_WoodenCrate cobre _T3).
	KLS_StorageDef FindStorageDef(EntityAI storage)
	{
		if (!storage) return null;

		string storageType = storage.GetType();

		foreach (KLS_StorageDef exactDef : m_StorageDefs)
		{
			if (exactDef.Storage == storageType) return exactDef;
		}

		foreach (KLS_StorageDef parentDef : m_StorageDefs)
		{
			if (storage.IsKindOf(parentDef.Storage)) return parentDef;
		}

		return null;
	}

	protected void LoadSettings()
	{
		ref KLS_Settings settings = new KLS_Settings();
		string errorMessage;

		if (!JsonFileLoader<KLS_Settings>.LoadFile(KLS_ServerConst.FILE_SETTINGS, settings, errorMessage))
		{
			KLS_Log.Error("Unreadable " + KLS_ServerConst.FILE_SETTINGS + ": " + errorMessage + ". Using defaults.");
			settings = new KLS_Settings();
		}

		if (settings.SearchTime <= 0) settings.SearchTime = KrydenLootSystemConst.DEFAULT_SEARCH_TIME;

		m_Settings = settings;
		KLS_Log.s_Debug = settings.DebugLog;
	}

	// Uma pool por arquivo: um JSON quebrado derruba so a propria pool (padrao do ZombieHordeEvent).
	protected void LoadPools()
	{
		m_Pools.Clear();
		m_ItemWeights.Clear();

		string fileName;
		FileAttr fileAttr;
		FindFileHandle handle = KLS_Files.Find(KLS_ServerConst.DIR_POOLS + "*.json", fileName, fileAttr);
		if (!handle)
		{
			KLS_Log.Warn("No pool in " + KLS_ServerConst.DIR_POOLS + ". Every storage opens empty.");
			return;
		}

		while (fileName != "")
		{
			LoadPoolFile(fileName);
			if (!FindNextFile(handle, fileName, fileAttr)) break;
		}

		CloseFindFile(handle);
	}

	protected void LoadPoolFile(string fileName)
	{
		string filePath = KLS_ServerConst.DIR_POOLS + fileName;
		string poolName = fileName;
		int extPos = fileName.IndexOf(".json");
		if (extPos > 0) poolName = fileName.Substring(0, extPos);

		// Pre-alocar e obrigatorio: o serializer so preenche o que veio no arquivo.
		ref KLS_PoolFile pool = new KLS_PoolFile();
		string errorMessage;

		if (!JsonFileLoader<KLS_PoolFile>.LoadFile(filePath, pool, errorMessage))
		{
			KLS_Log.Error("Unreadable pool " + filePath + ": " + errorMessage + ". The others still load.");
			return;
		}

		PruneItems(pool.Items, "Pool " + poolName);

		if (pool.Items.Count() == 0)
		{
			KLS_Log.Warn("Pool " + poolName + " has no valid item. Skipped.");
			return;
		}

		// Set e nao Insert: Insert nao atualiza chave existente.
		m_Pools.Set(poolName, pool);

		foreach (KLS_PoolItem weighted : pool.Items)
		{
			if (!weighted) continue;
			if (weighted.Weight > GetItemWeight(weighted.Item)) m_ItemWeights.Set(weighted.Item, weighted.Weight);
		}
	}

	// Kits.json e opcional: sem ele, cada item sai so com o que a propria linha da pool traz.
	protected void LoadKits()
	{
		m_Kits.Clear();

		if (!FileExist(KLS_ServerConst.FILE_KITS))
		{
			KLS_Log.Info("No " + KLS_ServerConst.FILE_KITS + ": pool items spawn only with what their own line says.");
			return;
		}

		ref KLS_KitsFile kitsFile = new KLS_KitsFile();
		string errorMessage;

		if (!JsonFileLoader<KLS_KitsFile>.LoadFile(KLS_ServerConst.FILE_KITS, kitsFile, errorMessage))
		{
			KLS_Log.Error("Unreadable " + KLS_ServerConst.FILE_KITS + ": " + errorMessage + ". Items spawn without their default kit.");
			return;
		}

		foreach (KLS_ItemKit kit : kitsFile.Kits)
		{
			if (!kit) continue;

			if (!ClassExists(kit.Item))
			{
				ReportMissing(kit.Item, "Kits.json");
				continue;
			}

			PruneGroups(kit.Attachments, "Kits.json");
			PruneGroups(kit.Cargo, "Kits.json");

			// Set e nao Insert: kit repetido, vale o ultimo.
			m_Kits.Set(kit.Item, kit);
		}
	}

	protected void LoadStorages()
	{
		m_StorageDefs.Clear();

		ref KLS_StoragesFile storagesFile = new KLS_StoragesFile();
		string errorMessage;

		if (!JsonFileLoader<KLS_StoragesFile>.LoadFile(KLS_ServerConst.FILE_STORAGES, storagesFile, errorMessage))
		{
			KLS_Log.Error("Unreadable " + KLS_ServerConst.FILE_STORAGES + ": " + errorMessage + ". Every storage opens empty.");
			return;
		}

		foreach (KLS_StorageDef storageDef : storagesFile.Storages)
		{
			if (!storageDef) continue;

			if (!GetGame().ConfigIsExisting("CfgVehicles " + storageDef.Storage))
			{
				KLS_Log.Warn("Storages.json names a storage that doesn't exist: '" + storageDef.Storage + "'. Skipped.");
				continue;
			}

			if (storageDef.Min < 0) storageDef.Min = 0;
			if (storageDef.Max < storageDef.Min) storageDef.Max = storageDef.Min;

			foreach (KLS_StoragePool poolRef : storageDef.Pools)
			{
				if (!poolRef) continue;
				if (m_Pools.Contains(poolRef.Pool)) continue;

				KLS_Log.Warn(storageDef.Storage + " points to pool '" + poolRef.Pool + "', which doesn't exist or has no valid item.");
			}

			if (storageDef.Guaranteed)
			{
				foreach (KLS_StorageGuarantee guarantee : storageDef.Guaranteed)
				{
					if (!guarantee || m_Pools.Contains(guarantee.Pool)) continue;

					KLS_Log.Warn(storageDef.Storage + " guarantees pool '" + guarantee.Pool + "', which doesn't exist or has no valid item.");
				}
			}

			m_StorageDefs.Insert(storageDef);
		}
	}

	// Tira classname que nao existe, em qualquer nivel, e avisa uma vez (RF-27).
	protected void PruneItems(array<ref KLS_PoolItem> items, string source)
	{
		if (!items) return;

		for (int itemIndex = items.Count() - 1; itemIndex >= 0; itemIndex--)
		{
			KLS_PoolItem entry = items.Get(itemIndex);
			if (!entry)
			{
				items.RemoveOrdered(itemIndex);
				continue;
			}

			if (!ClassExists(entry.Item))
			{
				ReportMissing(entry.Item, source);
				items.RemoveOrdered(itemIndex);
				continue;
			}

			PruneGroups(entry.Attachments, source);
			PruneGroups(entry.Cargo, source);
		}
	}

	protected void ReportMissing(string className, string source)
	{
		if (m_Reported.Contains(className)) return;

		m_Reported.Set(className, true);
		KLS_Log.Warn(source + " names a classname that doesn't exist: '" + className + "'. Skipped.");
	}

	protected void PruneGroups(array<ref KLS_PoolGroup> groups, string source)
	{
		if (!groups) return;

		for (int groupIndex = groups.Count() - 1; groupIndex >= 0; groupIndex--)
		{
			KLS_PoolGroup group = groups.Get(groupIndex);
			if (!group)
			{
				groups.RemoveOrdered(groupIndex);
				continue;
			}

			PruneItems(group.Items, source);
			if (!group.Items || group.Items.Count() == 0) groups.RemoveOrdered(groupIndex);
		}
	}

	// Classe de DayZ vive em tres raizes: so CfgVehicles reprovaria arma e municao.
	static bool ClassExists(string className)
	{
		if (className == "") return false;
		if (GetGame().ConfigIsExisting("CfgVehicles " + className)) return true;
		if (GetGame().ConfigIsExisting("CfgWeapons " + className)) return true;
		if (GetGame().ConfigIsExisting("CfgMagazines " + className)) return true;
		return false;
	}
}

// =====================================================================
// Sorteio e criacao
// =====================================================================
class KLS_LootGenerator
{
	// Cria o lote no cargo do storage. Devolve quantos itens de primeiro nivel sairam.
	int Fill(KLS_Storage_Base storage, KLS_StorageDef storageDef, KLS_LootRepository repository, out string report)
	{
		report = "";
		if (!storage || !storageDef || !repository) return 0;

		int target = Math.RandomIntInclusive(storageDef.Min, storageDef.Max);

		// Item que falhou nesta vasculhada (nao coube, slot errado) nao e sorteado de novo.
		ref array<string> banned = new array<string>();

		// Armas de fogo: a mesma arma sai uma vez so e o total tem teto (03/10/2026: 2 AKM no mesmo storage).
		m_MaxGuns = KLS_ServerConst.DEFAULT_MAX_GUNS;
		KLS_Settings settings = repository.GetSettings();
		if (settings) m_MaxGuns = settings.MaxGunsPerStorage;
		m_GunCount = 0;

		// Repetidos (04/10/2026: 3 iodos num kit, 3 coronhas de FAL num caixote): no maximo N do mesmo item.
		m_MaxSame = KLS_ServerConst.DEFAULT_MAX_SAME_ITEM;
		if (settings) m_MaxSame = settings.MaxSameItemPerStorage;
		m_SameCounts = new map<string, int>();
		m_SameBanned = new array<string>();
		m_SameRelaxed = false;

		int ammoPerGun = storageDef.AmmoPerGun;
		float ammoChance = storageDef.AmmoChance;
		int created = 0;

		// 0) Garantias do storage tematico (04/10/2026): saem antes e contam no lote. As tentativas ficam na
		//    MESMA pool, para a garantia nao virar item de outra pool.
		if (storageDef.Guaranteed)
		{
			foreach (KLS_StorageGuarantee guarantee : storageDef.Guaranteed)
			{
				if (!guarantee) continue;

				KLS_PoolFile guaranteedPool = repository.GetPool(guarantee.Pool);
				if (!guaranteedPool) continue;

				int guaranteedMax = guarantee.Max;
				if (guaranteedMax < guarantee.Min) guaranteedMax = guarantee.Min;
				int guaranteedCount = Math.RandomIntInclusive(guarantee.Min, guaranteedMax);

				for (int guaranteedIndex = 0; guaranteedIndex < guaranteedCount; guaranteedIndex++)
				{
					for (int guaranteedTry = 0; guaranteedTry < 5; guaranteedTry++)
					{
						KLS_PoolItem guaranteed = PickItem(guaranteedPool.Items, banned, guarantee.Categories);
						if (!guaranteed) break;

						NoteGunPicked(guaranteed, banned);
						NoteSamePicked(guaranteed);

						string guaranteedLine = TrySpawnTopLevel(storage, guaranteed, repository, banned, ammoPerGun, ammoChance);
						if (guaranteedLine == "")
						{
							if (IsGun(guaranteed.Item)) m_GunCount--;
							continue;
						}

						created++;
						report = report + guaranteedLine;
						break;
					}
				}
			}
		}

		// 1) Sorteia o resto do lote e cria do MAIOR para o menor: rifle comprido sorteado por ultimo nao
		//    achava mais linha livre no cargo (Mosin 10x3 no caixote 10x5, 03/10/2026).
		ref array<KLS_PoolItem> picks = new array<KLS_PoolItem>();
		for (int pickIndex = created; pickIndex < target; pickIndex++)
		{
			KLS_PoolItem rolled = PickFree(storageDef, repository, banned);
			if (!rolled) break;

			NoteGunPicked(rolled, banned);
			NoteSamePicked(rolled);
			picks.Insert(rolled);
		}
		SortBySizeDescending(picks);

		foreach (KLS_PoolItem pick : picks)
		{
			string pickedLine = TrySpawnTopLevel(storage, pick, repository, banned, ammoPerGun, ammoChance);
			if (pickedLine == "")
			{
				// Arma que nao coube libera a vaga dela para o sorteio de reposicao.
				if (IsGun(pick.Item)) m_GunCount--;
				continue;
			}

			created++;
			report = report + pickedLine;
		}

		// 2) O que falhou e trocado por outro sorteio, ate fechar o lote ou esgotar as tentativas.
		int attempts = 0;
		int maxAttempts = target * 5;
		while (created < target && attempts < maxAttempts)
		{
			attempts++;

			KLS_PoolItem retry = PickFree(storageDef, repository, banned);
			if (!retry) break;

			NoteGunPicked(retry, banned);
			NoteSamePicked(retry);

			string retryLine = TrySpawnTopLevel(storage, retry, repository, banned, ammoPerGun, ammoChance);
			if (retryLine == "")
			{
				if (IsGun(retry.Item)) m_GunCount--;
				continue;
			}

			created++;
			report = report + retryLine;
		}

		// Os tetos valem so dentro desta vasculhada.
		m_GunCount = 0;
		m_MaxGuns = -1;
		m_SameCounts = null;
		m_SameBanned = null;
		m_SameRelaxed = false;

		return created;
	}

	// Estado do teto de repetidos durante um Fill.
	protected int m_MaxSame;
	protected ref map<string, int> m_SameCounts;
	protected ref array<string> m_SameBanned;
	protected bool m_SameRelaxed;

	protected void NoteSamePicked(KLS_PoolItem entry)
	{
		if (!entry || !m_SameCounts || m_MaxSame <= 0 || IsGun(entry.Item)) return;

		int count = 1;
		if (m_SameCounts.Contains(entry.Item)) count = m_SameCounts.Get(entry.Item) + 1;
		m_SameCounts.Set(entry.Item, count);

		if (count >= m_MaxSame && m_SameBanned.Find(entry.Item) == -1) m_SameBanned.Insert(entry.Item);
	}

	// Sorteio livre com o teto de repetidos; se a pool for pequena demais para ele, a quantidade vem antes
	// (Morette, 04/10/2026: "nao pode diminuir a quantidade que vem").
	protected KLS_PoolItem PickFree(KLS_StorageDef storageDef, KLS_LootRepository repository, array<string> banned)
	{
		KLS_PoolItem rolled = PickFromStorage(storageDef, repository, banned);
		if (rolled || m_SameRelaxed || !m_SameBanned || m_SameBanned.Count() == 0) return rolled;

		m_SameRelaxed = true;
		return PickFromStorage(storageDef, repository, banned);
	}

	// Estado do teto de armas durante um Fill. m_MaxGuns < 0 = sem teto (fora do Fill).
	protected int m_GunCount;
	protected int m_MaxGuns = -1;
	protected ref map<string, bool> m_GunCache;

	// Arma de fogo = classe em CfgWeapons (a categoria "weapons" do types mistura municao e acessorio).
	protected bool IsGun(string className)
	{
		if (!m_GunCache) m_GunCache = new map<string, bool>();
		if (m_GunCache.Contains(className)) return m_GunCache.Get(className);

		bool isGun = GetGame().ConfigIsExisting("CfgWeapons " + className);
		m_GunCache.Set(className, isGun);
		return isGun;
	}

	// A arma sorteada nao sai de novo neste storage (vai para os banidos) e conta para o teto.
	protected void NoteGunPicked(KLS_PoolItem entry, array<string> banned)
	{
		if (!entry || !IsGun(entry.Item)) return;

		if (banned.Find(entry.Item) == -1) banned.Insert(entry.Item);
		m_GunCount++;
	}

	// Devolve o trecho do log do item criado, ou "" se nao coube (e o item vai para a lista de banidos).
	protected string TrySpawnTopLevel(KLS_Storage_Base storage, KLS_PoolItem entry, KLS_LootRepository repository, array<string> banned, int ammoPerGun, float ammoChance)
	{
		EntityAI spawned = SpawnItem(storage, entry, true, repository, 0);
		if (!spawned)
		{
			banned.Insert(entry.Item);
			return "";
		}

		// Lifetime no log: prova que o override parcial (nominal=0) manteve o lifetime do types (S5).
		string line = string.Format("%1(%2) ", entry.Item, spawned.GetLifetime());
		// Chance por arma (04/10/2026, Morette): arma sempre com o carregador dela ficava "roubado".
		if (ammoPerGun > 0 && IsGun(entry.Item) && Roll(ammoChance)) line = line + SpawnGunAmmo(storage, entry.Item, ammoPerGun, repository);

		return line;
	}

	// Carregador ou municao da PROPRIA arma: magazines[] da arma; sem carregador destacavel (Mosin, SKS,
	// espingarda), as balas de chamberableFrom[]. Fora do lote, entao nao conta no Min/Max.
	protected string SpawnGunAmmo(KLS_Storage_Base storage, string gunClass, int count, KLS_LootRepository repository)
	{
		TStringArray candidates = new TStringArray();
		GetGame().ConfigGetTextArray("CfgWeapons " + gunClass + " magazines", candidates);
		KeepSpawnableMagazines(candidates);

		if (candidates.Count() == 0)
		{
			GetGame().ConfigGetTextArray("CfgWeapons " + gunClass + " chamberableFrom", candidates);
			KeepSpawnableMagazines(candidates);
		}

		if (candidates.Count() == 0) return "";

		string line = "";
		for (int ammoIndex = 0; ammoIndex < count; ammoIndex++)
		{
			string ammoClass = PickWeightedName(candidates, repository);

			Magazine magazine;
			if (!Class.CastTo(magazine, storage.GetInventory().CreateInInventory(ammoClass))) continue;

			int maxAmmo = magazine.GetAmmoMax();
			int minAmmo = maxAmmo / 3;
			if (minAmmo < 1) minAmmo = 1;
			magazine.ServerSetAmmoCount(Math.RandomIntInclusive(minAmmo, maxAmmo));

			line = line + "+" + ammoClass + " ";
		}

		return line;
	}

	// Pelo maior peso do item nas pools (nominal vanilla da zona); fora das pools = peso 1. Perto dos pesos do
	// template (x1000, D-29), o que o vanilla nao poe no chao (tambor _TEST, variantes de cor, clip) quase nunca sai.
	protected string PickWeightedName(TStringArray names, KLS_LootRepository repository)
	{
		float total = 0;
		foreach (string countName : names)
		{
			total += NameWeight(countName, repository);
		}

		float roll = Math.RandomFloat01() * total;
		float running = 0;
		foreach (string pickName : names)
		{
			running += NameWeight(pickName, repository);
			if (roll < running) return pickName;
		}

		return names.Get(names.Count() - 1);
	}

	protected float NameWeight(string className, KLS_LootRepository repository)
	{
		float weight = repository.GetItemWeight(className);
		if (weight <= 0) weight = 1.0;
		return weight;
	}

	// So carregador/municao que existe e e de loot (scope 2). Remove nao preserva a ordem: tudo bem aqui.
	protected void KeepSpawnableMagazines(TStringArray names)
	{
		for (int index = names.Count() - 1; index >= 0; index--)
		{
			string path = "CfgMagazines " + names.Get(index);
			if (!GetGame().ConfigIsExisting(path) || GetGame().ConfigGetInt(path + " scope") != 2) names.Remove(index);
		}
	}

	// Lote e pequeno (ate ~6): insercao simples, sem alocar nada.
	protected void SortBySizeDescending(array<KLS_PoolItem> picks)
	{
		for (int outer = 1; outer < picks.Count(); outer++)
		{
			KLS_PoolItem current = picks.Get(outer);
			int currentArea = ItemArea(current.Item);
			int inner = outer - 1;
			while (inner >= 0 && ItemArea(picks.Get(inner).Item) < currentArea)
			{
				picks.Set(inner + 1, picks.Get(inner));
				inner--;
			}
			picks.Set(inner + 1, current);
		}
	}

	// Area no inventario (itemSize w x h), lida do config e guardada.
	protected ref map<string, int> m_AreaCache;

	protected int ItemArea(string className)
	{
		if (!m_AreaCache) m_AreaCache = new map<string, int>();
		if (m_AreaCache.Contains(className)) return m_AreaCache.Get(className);

		int area = 1;
		string path = "CfgVehicles " + className + " itemSize";
		if (!GetGame().ConfigIsExisting(path)) path = "CfgWeapons " + className + " itemSize";
		if (!GetGame().ConfigIsExisting(path)) path = "CfgMagazines " + className + " itemSize";

		if (GetGame().ConfigIsExisting(path))
		{
			TIntArray size = new TIntArray();
			GetGame().ConfigGetIntArray(path, size);
			if (size.Count() >= 2) area = size.Get(0) * size.Get(1);
		}

		m_AreaCache.Set(className, area);
		return area;
	}

	// Primeiro a pool, pelo peso dela no Storages.json; depois o item, pelo Weight dentro da pool.
	protected KLS_PoolItem PickFromStorage(KLS_StorageDef storageDef, KLS_LootRepository repository, array<string> banned)
	{
		float totalWeight = 0;

		foreach (KLS_StoragePool countRef : storageDef.Pools)
		{
			if (!countRef || countRef.Weight <= 0) continue;

			KLS_PoolFile countPool = repository.GetPool(countRef.Pool);
			if (!countPool || !HasEligible(countPool.Items, banned, storageDef.Categories)) continue;

			totalWeight += countRef.Weight;
		}

		if (totalWeight <= 0) return null;

		float roll = Math.RandomFloat01() * totalWeight;
		float running = 0;
		KLS_PoolFile lastEligible = null;

		foreach (KLS_StoragePool pickRef : storageDef.Pools)
		{
			if (!pickRef || pickRef.Weight <= 0) continue;

			KLS_PoolFile pickPool = repository.GetPool(pickRef.Pool);
			if (!pickPool || !HasEligible(pickPool.Items, banned, storageDef.Categories)) continue;

			lastEligible = pickPool;
			running += pickRef.Weight;
			if (roll < running) return PickItem(pickPool.Items, banned, storageDef.Categories);
		}

		// Arredondamento de float no ultimo passo.
		if (lastEligible) return PickItem(lastEligible.Items, banned, storageDef.Categories);
		return null;
	}

	protected KLS_PoolItem PickItem(array<ref KLS_PoolItem> items, array<string> banned, array<string> categories)
	{
		if (!items) return null;

		float totalWeight = 0;
		foreach (KLS_PoolItem countEntry : items)
		{
			if (IsEligible(countEntry, banned, categories)) totalWeight += countEntry.Weight;
		}

		if (totalWeight <= 0) return null;

		float roll = Math.RandomFloat01() * totalWeight;
		float running = 0;
		KLS_PoolItem lastEligible = null;

		foreach (KLS_PoolItem pickEntry : items)
		{
			if (!IsEligible(pickEntry, banned, categories)) continue;

			lastEligible = pickEntry;
			running += pickEntry.Weight;
			if (roll < running) return pickEntry;
		}

		return lastEligible;
	}

	protected bool HasEligible(array<ref KLS_PoolItem> items, array<string> banned, array<string> categories)
	{
		if (!items) return false;

		foreach (KLS_PoolItem entry : items)
		{
			if (IsEligible(entry, banned, categories)) return true;
		}

		return false;
	}

	protected bool IsEligible(KLS_PoolItem entry, array<string> banned, array<string> categories)
	{
		if (!entry) return false;
		if (entry.Weight <= 0) return false;
		if (banned && banned.Find(entry.Item) != -1) return false;
		// Teto de repetidos: so no primeiro nivel (aninhado passa banned null).
		if (banned && !m_SameRelaxed && m_SameBanned && m_SameBanned.Find(entry.Item) != -1) return false;
		// Filtro do storage tematico: so nos sorteios livres de primeiro nivel (aninhado passa null).
		if (categories && categories.Count() > 0 && categories.Find(entry.Category) == -1) return false;
		if (m_MaxGuns >= 0 && m_GunCount >= m_MaxGuns && IsGun(entry.Item)) return false;
		return true;
	}

	// Cria o item e, recursivamente, os attachments e o cargo dele. depth = nivel do item (0 = solto no
	// storage). Cada campo que a linha nao trouxer vem do kit do item no Kits.json (D-29).
	protected EntityAI SpawnItem(EntityAI parent, KLS_PoolItem entry, bool intoCargo, KLS_LootRepository repository, int depth)
	{
		if (!parent || !entry || !repository) return null;

		KLS_ItemKit kit = repository.GetKit(entry.Item);
		bool kitOpensGroups = kit && depth < KLS_ServerConst.KIT_GROUP_DEPTH;

		array<float> damage = entry.Damage;
		if (!HasRange(damage) && kit) damage = kit.Damage;

		array<float> quantity = entry.Quantity;
		if (!HasRange(quantity) && kit) quantity = kit.Quantity;

		array<ref KLS_PoolGroup> attachments = entry.Attachments;
		if (!HasGroups(attachments) && kitOpensGroups) attachments = kit.Attachments;

		array<ref KLS_PoolGroup> cargo = entry.Cargo;
		if (!HasGroups(cargo) && kitOpensGroups) cargo = kit.Cargo;

		EntityAI item;
		Weapon_Base weapon;

		// Container dentro de cargo nao recebe nada (Container_Base.CanReceiveItemIntoCargo ->
		// AreChildrenAccessible, container_base.c:25): kit medico criado no storage nascia vazio
		// (03/10/2026). Item com Cargo nasce no mundo, e preenchido ali e depois entra no cargo, como o
		// vanilla faz ao enterrar no stash (actiondiginstash.c:177).
		// Roupa cheia tambem vem por aqui: o Client libera a entrada dela no storage (modded Clothing).
		bool fillInWorld = intoCargo && HasGroups(cargo);

		if (!intoCargo && Class.CastTo(weapon, parent) && IsWeaponMagazine(entry.Item))
		{
			// Carregador na arma: o caminho do vanilla, que acerta camara e o estado da arma.
			int flags = WeaponWithAmmoFlags.CHAMBER;
			if (!HasRange(quantity)) flags = flags | WeaponWithAmmoFlags.QUANTITY_RNG;

			item = weapon.SpawnAttachedMagazine(entry.Item, flags);
		}
		else if (fillInWorld)
		{
			item = EntityAI.Cast(GetGame().CreateObjectEx(entry.Item, parent.GetPosition() + Vector(0, 0.3, 0), ECE_KEEPHEIGHT));
		}
		else if (intoCargo)
		{
			item = parent.GetInventory().CreateInInventory(entry.Item);
		}
		else
		{
			item = parent.GetInventory().CreateAttachment(entry.Item);
		}

		if (!item)
		{
			// Nao e necessariamente erro: cargo cheio ou slot incompativel caem aqui.
			KLS_Log.Debug("Could not create " + entry.Item + " in " + parent.GetType() + " (no room or no matching slot).");
			return null;
		}

		ApplyDamage(item, damage, repository.GetSettings());
		ApplyQuantity(item, quantity);
		ApplyNeutralTemperature(item);

		SpawnGroups(item, attachments, false, repository, depth + 1);
		SpawnGroups(item, cargo, true, repository, depth + 1);

		if (fillInWorld && !parent.ServerTakeEntityToTargetCargo(parent, item))
		{
			KLS_Log.Debug("Could not move " + entry.Item + " into " + parent.GetType() + " (no room).");
			GetGame().ObjectDelete(item);
			return null;
		}

		return item;
	}

	protected bool HasGroups(array<ref KLS_PoolGroup> groups)
	{
		return groups && groups.Count() > 0;
	}

	// Cada grupo sai com Chance; dele sai UM item, sorteado por Weight (como o cfgspawnabletypes).
	protected void SpawnGroups(EntityAI parent, array<ref KLS_PoolGroup> groups, bool intoCargo, KLS_LootRepository repository, int depth)
	{
		if (!parent || !groups) return;

		foreach (KLS_PoolGroup group : groups)
		{
			if (!group) continue;
			if (!Roll(group.Chance)) continue;

			KLS_PoolItem child = PickItem(group.Items, null, null);
			if (child) SpawnItem(parent, child, intoCargo, repository, depth);
		}
	}

	protected void ApplyDamage(EntityAI item, array<float> damage, KLS_Settings settings)
	{
		float minDamage = KLS_ServerConst.DEFAULT_DAMAGE_MIN;
		float maxDamage = KLS_ServerConst.DEFAULT_DAMAGE_MAX;

		if (HasRange(damage))
		{
			minDamage = damage.Get(0);
			maxDamage = damage.Get(1);
		}
		else if (settings && HasRange(settings.DefaultDamage))
		{
			minDamage = settings.DefaultDamage.Get(0);
			maxDamage = settings.DefaultDamage.Get(1);
		}

		if (maxDamage < minDamage) maxDamage = minDamage;

		// Nunca ruined: quem fica ruined e o storage, nao o loot.
		float health = Math.Clamp(1.0 - Math.RandomFloatInclusive(minDamage, maxDamage), 0.05, 1.0);
		item.SetHealth01("", "", health);
	}

	// Temperatura neutra, como o vanilla faz no item criado por script (SetupSpawnedItem, itembase.c:4943): sem
	// isto o item nascia na temperatura do ambiente e a comida vinha "chilly" a noite (04/10/2026).
	protected void ApplyNeutralTemperature(EntityAI item)
	{
		if (!item.CanHaveTemperature()) return;

		item.SetTemperatureDirect(GameConstants.ITEM_TEMPERATURE_NEUTRAL_ZONE_MIDDLE);
		if (item.CanFreeze()) item.SetFrozen(false);
	}

	protected void ApplyQuantity(EntityAI item, array<float> quantity)
	{
		if (!HasRange(quantity)) return;

		float minQuantity = Math.Clamp(quantity.Get(0), 0.0, 1.0);
		float maxQuantity = Math.Clamp(quantity.Get(1), 0.0, 1.0);
		if (maxQuantity < minQuantity) maxQuantity = minQuantity;

		float rolled = Math.RandomFloatInclusive(minQuantity, maxQuantity);

		// Carregador e pilha de municao contam balas, nao quantity.
		Magazine magazine;
		if (Class.CastTo(magazine, item))
		{
			int ammoCount = Math.Round(magazine.GetAmmoMax() * rolled);
			if (magazine.IsAmmoPile() && ammoCount < 1) ammoCount = 1;

			magazine.ServerSetAmmoCount(ammoCount);
			return;
		}

		if (item.HasQuantity()) item.SetQuantityNormalized(rolled, false);
	}

	protected bool IsWeaponMagazine(string className)
	{
		if (!GetGame().ConfigIsExisting("CfgMagazines " + className)) return false;
		return !GetGame().IsKindOf(className, "Ammunition_Base");
	}

	protected bool HasRange(array<float> range)
	{
		return range && range.Count() >= 2;
	}

	// Os extremos nao sorteiam: chance 0 nunca sai, chance 1 sempre sai.
	protected bool Roll(float chance)
	{
		if (chance <= 0.0) return false;
		if (chance >= 1.0) return true;
		return Math.RandomFloat01() < chance;
	}
}
