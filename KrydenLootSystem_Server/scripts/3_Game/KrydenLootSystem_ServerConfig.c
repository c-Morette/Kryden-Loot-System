// ---------------------------------------------------------------------
// KrydenLootSystem (Server/3_Game): caminhos, log e o schema dos JSON do admin (D-19).
//
// Os nomes de campo sao contrato com o JSON em disco: renomear um deles faz o JsonFileLoader cair no
// default, em silencio. O JsonFileLoader grava TODO membro declarado: nao guardar estado aqui.
// ---------------------------------------------------------------------
class KLS_ServerConst
{
	static const string MKDIR_KRYDEN = "$profile:Kryden";
	static const string MKDIR_ROOT   = "$profile:Kryden/KrydenLootSystem";
	static const string MKDIR_POOLS  = "$profile:Kryden/KrydenLootSystem/Pools";

	static const string DIR_POOLS     = "$profile:Kryden/KrydenLootSystem/Pools/";
	static const string FILE_SETTINGS = "$profile:Kryden/KrydenLootSystem/Settings.json";
	static const string FILE_STORAGES = "$profile:Kryden/KrydenLootSystem/Storages.json";
	static const string FILE_KITS     = "$profile:Kryden/KrydenLootSystem/Kits.json";

	// Ate que nivel o kit do Kits.json abre Attachments/Cargo: arma (0) > luneta (1) > pilha (2, so dano e
	// quantidade), como o template gerava antes do Kits.json. Grupo escrito na propria linha nao tem limite.
	static const int KIT_GROUP_DEPTH = 2;

	// Padrao do vanilla para o dano do loot (LootDamageMin/Max do globals.xml).
	static const float DEFAULT_DAMAGE_MIN = 0.0;
	static const float DEFAULT_DAMAGE_MAX = 0.82;

	// Teto padrao de armas de fogo por storage (Settings.MaxGunsPerStorage).
	static const int DEFAULT_MAX_GUNS = 2;

	// Teto padrao de copias do mesmo item por storage (Settings.MaxSameItemPerStorage).
	static const int DEFAULT_MAX_SAME_ITEM = 2;
}

// DayZ 1.30 (DZEXP-134): FindFile deixou de resolver "$profile:" com as flags normais e devolve nada, sem erro.
// As pools sumiam e o servidor gravava as de exemplo por cima (16/10/2026). Mesmo contorno do CF 1.5.9
// (CF::FindFileEx): testa uma vez num arquivo que existe; se a busca nao o acha, passa -1 nas flags, que resolve o
// prefixo. No 1.29, ou quando a Bohemia corrigir, o teste passa e nada muda.
class KLS_Files
{
	protected static bool s_Tested;
	protected static bool s_ResolveWithMinusOne;

	static FindFileHandle Find(string pattern, out string fileName, out FileAttr fileAttributes)
	{
		if (!s_Tested) DetectProfileListing();

		FindFileFlags flags = FindFileFlags.DIRECTORIES;
		if (s_ResolveWithMinusOne) flags = -1;

		return FindFile(pattern, fileName, fileAttributes, flags);
	}

	protected static void DetectProfileListing()
	{
		// Sem o arquivo de teste ainda (primeiro boot antes do Settings.json), testa na proxima chamada.
		if (!FileExist(KLS_ServerConst.FILE_SETTINGS)) return;

		s_Tested = true;

		string testName;
		FileAttr testAttr;
		FindFileHandle handle = FindFile(KLS_ServerConst.FILE_SETTINGS, testName, testAttr, FindFileFlags.DIRECTORIES);
		if (!handle || testName != "Settings.json") s_ResolveWithMinusOne = true;
		if (handle) CloseFindFile(handle);

		if (s_ResolveWithMinusOne) KLS_Log.Info("FindFile ignores $profile: on this DayZ version (DZEXP-134): listing folders with the workaround.");
	}
}

class KLS_Log
{
	static bool s_Debug;

	static void Debug(string message)
	{
		if (!s_Debug) return;
		Print(KrydenLootSystemConst.LOG_TAG + "[DEBUG] " + message);
	}

	static void Info(string message)
	{
		Print(KrydenLootSystemConst.LOG_TAG + message);
	}

	static void Warn(string message)
	{
		Print(KrydenLootSystemConst.LOG_TAG + "[WARN] " + message);
	}

	static void Error(string message)
	{
		Print(KrydenLootSystemConst.LOG_TAG + "[ERROR] " + message);
	}
}

// Settings.json
class KLS_Settings
{
	// Segundos segurando F (D-11).
	float SearchTime;

	// [min, max] de DANO (0 = pristine) para item da pool que nao tem "Damage". Vazio = padrao vanilla.
	ref array<float> DefaultDamage;

	// Uma linha por vasculhada com o que saiu (RF-26).
	bool DebugLog;

	// Teto de armas de fogo (CfgWeapons) por storage. A mesma arma nunca sai duas vezes no mesmo storage.
	int MaxGunsPerStorage;

	// Teto de copias do MESMO item (que nao e arma) por storage. Se a pool for pequena demais, a quantidade
	// vem antes e o teto e ignorado. 0 = sem teto.
	int MaxSameItemPerStorage;

	// 1 = grava Export/items.json no boot, para a ferramenta do dono do servidor gerar o template do mapa (D-31).
	bool ExportItems;

	void KLS_Settings()
	{
		SearchTime        = KrydenLootSystemConst.DEFAULT_SEARCH_TIME;
		DefaultDamage     = new array<float>();
		DebugLog          = false;
		MaxGunsPerStorage = KLS_ServerConst.DEFAULT_MAX_GUNS;
		MaxSameItemPerStorage = KLS_ServerConst.DEFAULT_MAX_SAME_ITEM;
		ExportItems       = false;
	}
}

// Uma entrada de pool. Recursiva: Attachments e Cargo sao grupos de entradas, no modelo do
// cfgspawnabletypes vanilla (o grupo sai com Chance e dele sai UM item, sorteado por Weight).
// Formato enxuto (D-29): a linha so precisa de Item e Weight; cada campo do kit (Damage, Quantity,
// Attachments, Cargo) que ela NAO trouxer vem do Kits.json. O que ela trouxer vence o kit.
class KLS_PoolItem
{
	string Item;
	float Weight;

	// Categoria para o filtro do storage (KLS_StorageDef.Categories). O template grava: guns, ammo,
	// attachments, medical, tools, food, clothes, containers, explosives. Vazio = sem categoria.
	string Category;

	// [min, max] de DANO (0 = pristine, 1 = ruined), igual ao <damage> do cfgspawnabletypes.
	ref array<float> Damage;

	// [min, max] de 0 a 1. Carregador e pilha de municao: fracao da capacidade.
	ref array<float> Quantity;

	ref array<ref KLS_PoolGroup> Attachments;
	ref array<ref KLS_PoolGroup> Cargo;

	void KLS_PoolItem()
	{
		Item        = "";
		Weight      = 1.0;
		Category    = "";
		Damage      = new array<float>();
		Quantity    = new array<float>();
		Attachments = new array<ref KLS_PoolGroup>();
		Cargo       = new array<ref KLS_PoolGroup>();
	}
}

class KLS_PoolGroup
{
	float Chance;
	ref array<ref KLS_PoolItem> Items;

	void KLS_PoolGroup()
	{
		Chance = 1.0;
		Items  = new array<ref KLS_PoolItem>();
	}
}

// Um arquivo em Pools/. O nome da pool e o nome do arquivo sem ".json".
class KLS_PoolFile
{
	ref array<ref KLS_PoolItem> Items;

	void KLS_PoolFile()
	{
		Items = new array<ref KLS_PoolItem>();
	}
}

// Kits.json (D-29): o kit padrao de cada item, como o cfgspawnabletypes vanilla. Escrito uma vez, vale para
// toda pool; os itens de dentro dos grupos tambem buscam o proprio kit aqui (ate KIT_GROUP_DEPTH).
class KLS_ItemKit
{
	string Item;
	ref array<float> Damage;
	ref array<float> Quantity;
	ref array<ref KLS_PoolGroup> Attachments;
	ref array<ref KLS_PoolGroup> Cargo;

	void KLS_ItemKit()
	{
		Item        = "";
		Damage      = new array<float>();
		Quantity    = new array<float>();
		Attachments = new array<ref KLS_PoolGroup>();
		Cargo       = new array<ref KLS_PoolGroup>();
	}
}

class KLS_KitsFile
{
	ref array<ref KLS_ItemKit> Kits;

	void KLS_KitsFile()
	{
		Kits = new array<ref KLS_ItemKit>();
	}
}

// Storages.json
class KLS_StoragePool
{
	string Pool;
	float Weight;

	void KLS_StoragePool()
	{
		Pool   = "";
		Weight = 1.0;
	}
}

// Sorteio garantido de uma pool: sai de Min a Max itens dela, antes dos sorteios livres (04/10/2026).
// Categories (opcional) filtra a garantia: "1 comida" da pool da cidade, por exemplo.
class KLS_StorageGuarantee
{
	string Pool;
	int Min;
	int Max;
	ref array<string> Categories;

	void KLS_StorageGuarantee()
	{
		Pool       = "";
		Min        = 1;
		Max        = 1;
		Categories = new array<string>();
	}
}

class KLS_StorageDef
{
	// Classe do storage. Vale para ela e para as filhas (KLS_WoodenCrate cobre os _T1.._T4), a nao
	// ser que a filha tenha entrada propria.
	string Storage;
	int Min;
	int Max;
	ref array<ref KLS_StoragePool> Pools;

	// Storage tematico (04/10/2026). Categories filtra os sorteios livres (vazio = tudo); Guaranteed sai
	// antes deles e conta no lote (Min/Max); AmmoPerGun = carregadores/municao da propria arma por arma
	// criada, fora do lote, com chance AmmoChance (0 a 1) por arma.
	ref array<string> Categories;
	ref array<ref KLS_StorageGuarantee> Guaranteed;
	int AmmoPerGun;
	float AmmoChance;

	void KLS_StorageDef()
	{
		Storage    = "";
		Min        = 1;
		Max        = 3;
		Pools      = new array<ref KLS_StoragePool>();
		Categories = new array<string>();
		Guaranteed = new array<ref KLS_StorageGuarantee>();
		AmmoPerGun = 0;
		AmmoChance = 1.0;
	}
}

class KLS_StoragesFile
{
	ref array<ref KLS_StorageDef> Storages;

	void KLS_StoragesFile()
	{
		Storages = new array<ref KLS_StorageDef>();
	}
}
