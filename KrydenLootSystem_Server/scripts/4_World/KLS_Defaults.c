// ---------------------------------------------------------------------
// Arquivos de exemplo gravados no primeiro boot, so o que FALTA (RNF-04: config do admin nunca e
// sobrescrita). Sao para o mod funcionar de cara e para o admin ver o formato; as pools de verdade
// vem no template do mapa (D-09), geradas do types/cfgspawnabletypes vanilla.
// ---------------------------------------------------------------------
class KLS_Defaults
{
	static void EnsureFiles()
	{
		MakeDirectory(KLS_ServerConst.MKDIR_KRYDEN);
		MakeDirectory(KLS_ServerConst.MKDIR_ROOT);
		MakeDirectory(KLS_ServerConst.MKDIR_POOLS);

		string errorMessage;

		if (!FileExist(KLS_ServerConst.FILE_SETTINGS))
		{
			ref KLS_Settings settings = new KLS_Settings();
			settings.DefaultDamage.Insert(KLS_ServerConst.DEFAULT_DAMAGE_MIN);
			settings.DefaultDamage.Insert(KLS_ServerConst.DEFAULT_DAMAGE_MAX);

			if (JsonFileLoader<KLS_Settings>.SaveFile(KLS_ServerConst.FILE_SETTINGS, settings, errorMessage))
				KLS_Log.Info("Created " + KLS_ServerConst.FILE_SETTINGS);
			else
				KLS_Log.Error("Could not create " + KLS_ServerConst.FILE_SETTINGS + ": " + errorMessage);
		}

		if (!FileExist(KLS_ServerConst.FILE_STORAGES))
		{
			ref KLS_StoragesFile storages = BuildStorages();

			if (JsonFileLoader<KLS_StoragesFile>.SaveFile(KLS_ServerConst.FILE_STORAGES, storages, errorMessage))
				KLS_Log.Info("Created " + KLS_ServerConst.FILE_STORAGES);
			else
				KLS_Log.Error("Could not create " + KLS_ServerConst.FILE_STORAGES + ": " + errorMessage);
		}

		if (!HasAnyPool())
		{
			WritePool("Food_Canned", BuildFoodCanned());
			WritePool("Tools", BuildTools());
			WritePool("Medical", BuildMedical());
			WritePool("Clothes", BuildClothes());
			WritePool("Military_001", BuildMilitary001());
		}
	}

	protected static bool HasAnyPool()
	{
		string fileName;
		FileAttr fileAttr;
		FindFileHandle handle = KLS_Files.Find(KLS_ServerConst.DIR_POOLS + "*.json", fileName, fileAttr);
		if (!handle) return false;

		CloseFindFile(handle);
		return fileName != "";
	}

	protected static void WritePool(string poolName, KLS_PoolFile pool)
	{
		string filePath = KLS_ServerConst.DIR_POOLS + poolName + ".json";
		string errorMessage;

		if (JsonFileLoader<KLS_PoolFile>.SaveFile(filePath, pool, errorMessage))
			KLS_Log.Info("Created example pool " + filePath);
		else
			KLS_Log.Error("Could not create " + filePath + ": " + errorMessage);
	}

	// -----------------------------------------------------------------
	// Montadores
	// -----------------------------------------------------------------
	protected static KLS_PoolItem MakeItem(string className, float weight)
	{
		KLS_PoolItem entry = new KLS_PoolItem();
		entry.Item   = className;
		entry.Weight = weight;
		return entry;
	}

	// NAO devolver o proprio argumento: "return entry" com entry vindo de um temporario
	// (WithX(MakeItem(...))) corrompeu o heap no free (0xc0000374, crash de 03/10/2026). Por isso
	// estes sao void e trabalham sobre uma variavel nomeada.
	protected static void SetDamage(KLS_PoolItem entry, float minDamage, float maxDamage)
	{
		entry.Damage.Insert(minDamage);
		entry.Damage.Insert(maxDamage);
	}

	protected static void SetQuantity(KLS_PoolItem entry, float minQuantity, float maxQuantity)
	{
		entry.Quantity.Insert(minQuantity);
		entry.Quantity.Insert(maxQuantity);
	}

	// Recebe nomes e cria os itens aqui dentro: nenhum objeto temporario entra como argumento.
	protected static KLS_PoolGroup MakeGroup(float chance, string firstItem, float firstWeight = 1, string secondItem = "", float secondWeight = 1)
	{
		KLS_PoolGroup group = new KLS_PoolGroup();
		group.Chance = chance;

		KLS_PoolItem first = MakeItem(firstItem, firstWeight);
		group.Items.Insert(first);

		if (secondItem != "")
		{
			KLS_PoolItem second = MakeItem(secondItem, secondWeight);
			group.Items.Insert(second);
		}

		return group;
	}

	protected static KLS_StorageDef MakeStorage(string className, int minItems, int maxItems)
	{
		KLS_StorageDef storageDef = new KLS_StorageDef();
		storageDef.Storage = className;
		storageDef.Min     = minItems;
		storageDef.Max     = maxItems;
		return storageDef;
	}

	protected static void AddPool(KLS_StorageDef storageDef, string poolName, float weight)
	{
		KLS_StoragePool poolRef = new KLS_StoragePool();
		poolRef.Pool   = poolName;
		poolRef.Weight = weight;
		storageDef.Pools.Insert(poolRef);
	}

	// -----------------------------------------------------------------
	// Storages.json de exemplo: tematico, como o template vai vir (o admin decide - D-19).
	// -----------------------------------------------------------------
	protected static KLS_StoragesFile BuildStorages()
	{
		KLS_StoragesFile file = new KLS_StoragesFile();
		KLS_StorageDef storageDef;

		storageDef = MakeStorage("KLS_WoodenCrate", 2, 4);
		AddPool(storageDef, "Tools", 50);
		AddPool(storageDef, "Food_Canned", 30);
		AddPool(storageDef, "Military_001", 20);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_HuntingBag", 1, 3);
		AddPool(storageDef, "Food_Canned", 40);
		AddPool(storageDef, "Tools", 30);
		AddPool(storageDef, "Clothes", 30);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_MountainBag", 1, 3);
		AddPool(storageDef, "Food_Canned", 40);
		AddPool(storageDef, "Clothes", 30);
		AddPool(storageDef, "Medical", 15);
		AddPool(storageDef, "Tools", 15);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_ChildBag", 1, 2);
		AddPool(storageDef, "Food_Canned", 50);
		AddPool(storageDef, "Clothes", 30);
		AddPool(storageDef, "Medical", 20);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_CoyoteBag", 1, 3);
		AddPool(storageDef, "Food_Canned", 40);
		AddPool(storageDef, "Tools", 35);
		AddPool(storageDef, "Clothes", 25);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_AssaultBag", 1, 3);
		AddPool(storageDef, "Military_001", 60);
		AddPool(storageDef, "Medical", 20);
		AddPool(storageDef, "Food_Canned", 20);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_TTsKOJacket", 1, 2);
		AddPool(storageDef, "Military_001", 60);
		AddPool(storageDef, "Medical", 20);
		AddPool(storageDef, "Food_Canned", 20);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_QuiltedJacket", 1, 2);
		AddPool(storageDef, "Food_Canned", 50);
		AddPool(storageDef, "Clothes", 25);
		AddPool(storageDef, "Medical", 25);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_ParamedicJacket", 1, 2);
		AddPool(storageDef, "Medical", 80);
		AddPool(storageDef, "Food_Canned", 20);
		file.Storages.Insert(storageDef);

		storageDef = MakeStorage("KLS_PoliceJacket", 1, 2);
		AddPool(storageDef, "Military_001", 30);
		AddPool(storageDef, "Medical", 30);
		AddPool(storageDef, "Food_Canned", 40);
		file.Storages.Insert(storageDef);

		return file;
	}

	// -----------------------------------------------------------------
	// Pools de exemplo (classnames vanilla conferidos em P:\DZ)
	// -----------------------------------------------------------------
	protected static KLS_PoolFile BuildFoodCanned()
	{
		KLS_PoolFile pool = new KLS_PoolFile();
		pool.Items.Insert(MakeItem("BakedBeansCan", 10));
		pool.Items.Insert(MakeItem("SardinesCan", 10));
		pool.Items.Insert(MakeItem("PeachesCan", 8));
		pool.Items.Insert(MakeItem("TunaCan", 8));
		pool.Items.Insert(MakeItem("SpaghettiCan", 8));
		pool.Items.Insert(MakeItem("TacticalBaconCan", 4));
		return pool;
	}

	protected static KLS_PoolFile BuildTools()
	{
		KLS_PoolFile pool = new KLS_PoolFile();
		pool.Items.Insert(MakeItem("Matchbox", 10));
		pool.Items.Insert(MakeItem("CanOpener", 8));
		pool.Items.Insert(MakeItem("Screwdriver", 8));
		pool.Items.Insert(MakeItem("Pliers", 6));
		pool.Items.Insert(MakeItem("Hammer", 6));
		pool.Items.Insert(MakeItem("DuctTape", 6));
		pool.Items.Insert(MakeItem("Wrench", 5));
		pool.Items.Insert(MakeItem("Rope", 4));
		pool.Items.Insert(MakeItem("Hatchet", 3));
		return pool;
	}

	protected static KLS_PoolFile BuildMedical()
	{
		KLS_PoolFile pool = new KLS_PoolFile();
		KLS_PoolItem rag = MakeItem("Rag", 12);
		SetQuantity(rag, 0.2, 1.0);
		pool.Items.Insert(rag);
		pool.Items.Insert(MakeItem("BandageDressing", 10));
		pool.Items.Insert(MakeItem("VitaminBottle", 6));
		pool.Items.Insert(MakeItem("PainkillerTablets", 5));
		pool.Items.Insert(MakeItem("CharcoalTablets", 5));
		KLS_PoolItem disinfectant = MakeItem("DisinfectantSpray", 4);
		SetQuantity(disinfectant, 0.3, 1.0);
		pool.Items.Insert(disinfectant);
		pool.Items.Insert(MakeItem("TetracyclineAntibiotics", 3));

		// Kit com coisa dentro: Cargo usa os mesmos grupos dos attachments.
		KLS_PoolItem firstAidKit = MakeItem("FirstAidKit", 2);
		firstAidKit.Cargo.Insert(MakeGroup(0.8, "BandageDressing", 1));
		firstAidKit.Cargo.Insert(MakeGroup(0.5, "PainkillerTablets", 1, "CharcoalTablets", 1));
		pool.Items.Insert(firstAidKit);
		return pool;
	}

	protected static KLS_PoolFile BuildClothes()
	{
		KLS_PoolFile pool = new KLS_PoolFile();
		pool.Items.Insert(MakeItem("TShirt_White", 8));
		pool.Items.Insert(MakeItem("Jeans_Blue", 6));
		pool.Items.Insert(MakeItem("BeanieHat_Black", 6));
		pool.Items.Insert(MakeItem("Sneakers_Black", 5));
		pool.Items.Insert(MakeItem("WorkingGloves_Black", 4));
		return pool;
	}

	protected static KLS_PoolFile BuildMilitary001()
	{
		KLS_PoolFile pool = new KLS_PoolFile();

		// AKM com os mesmos grupos do cfgspawnabletypes vanilla do Chernarus.
		KLS_PoolItem akm = MakeItem("AKM", 2);
		SetDamage(akm, 0.45, 0.85);
		akm.Attachments.Insert(MakeGroup(0.25, "AK_PlasticBttstck", 1));
		akm.Attachments.Insert(MakeGroup(1.00, "AK_PlasticHndgrd", 1));
		akm.Attachments.Insert(MakeGroup(0.50, "KashtanOptic", 30, "PSO11Optic", 20));
		akm.Attachments.Insert(MakeGroup(0.05, "AK_Suppressor", 1));
		akm.Attachments.Insert(MakeGroup(0.40, "Mag_AKM_30Rnd", 1));
		pool.Items.Insert(akm);

		KLS_PoolItem sks = MakeItem("SKS", 3);
		SetDamage(sks, 0.45, 0.85);
		pool.Items.Insert(sks);

		KLS_PoolItem magazine = MakeItem("Mag_AKM_30Rnd", 6);
		SetQuantity(magazine, 0.0, 1.0);
		pool.Items.Insert(magazine);

		KLS_PoolItem ammo = MakeItem("Ammo_762x39", 8);
		SetQuantity(ammo, 0.2, 1.0);
		pool.Items.Insert(ammo);

		KLS_PoolItem radio = MakeItem("PersonalRadio", 3);
		radio.Attachments.Insert(MakeGroup(0.30, "Battery9V", 1));
		pool.Items.Insert(radio);

		KLS_PoolItem flashlight = MakeItem("Flashlight", 4);
		flashlight.Attachments.Insert(MakeGroup(0.30, "Battery9V", 1));
		pool.Items.Insert(flashlight);

		return pool;
	}
}
