// ---------------------------------------------------------------------
// Exportacao dos dados de item para a ferramenta do dono do servidor (D-31).
//
// Com ExportItems = 1 no Settings.json, o servidor grava no boot Export/items.json: todo item scope 2 com itemSize
// da config CARREGADA (vanilla e os mods do servidor), com a raiz do config, o tamanho no inventario e a cadeia de
// classes base. E o que o gerador de template lia dos configs extraidos em P:\DZ, que o dono do servidor nao tem.
// ---------------------------------------------------------------------
class KLS_ExportedItem
{
	string Item;
	string Root;
	ref array<int> Size;
	ref array<string> Bases;

	void KLS_ExportedItem()
	{
		Item  = "";
		Root  = "";
		Size  = new array<int>();
		Bases = new array<string>();
	}
}

class KLS_ItemExportFile
{
	string GameVersion;
	ref array<string> Mods;
	ref array<ref KLS_ExportedItem> Items;

	void KLS_ItemExportFile()
	{
		GameVersion = "";
		Mods        = new array<string>();
		Items       = new array<ref KLS_ExportedItem>();
	}
}

class KLS_ItemExport
{
	static const string MKDIR_EXPORT = "$profile:Kryden/KrydenLootSystem/Export";
	static const string FILE_ITEMS   = "$profile:Kryden/KrydenLootSystem/Export/items.json";

	// Teto da cadeia de classes base: config circular nao trava o boot.
	static const int MAX_BASES = 32;

	static void Run()
	{
		ref KLS_ItemExportFile file = new KLS_ItemExportFile();

		string version;
		GetGame().GetVersion(version);
		file.GameVersion = version;

		array<ref ModInfo> mods = new array<ref ModInfo>();
		GetGame().GetModInfos(mods);
		foreach (ModInfo mod : mods)
		{
			if (mod) file.Mods.Insert(mod.GetName());
		}

		ExportRoot("CfgVehicles", file.Items);
		ExportRoot("CfgWeapons", file.Items);
		ExportRoot("CfgMagazines", file.Items);

		MakeDirectory(MKDIR_EXPORT);
		string errorMessage;
		if (JsonFileLoader<KLS_ItemExportFile>.SaveFile(FILE_ITEMS, file, errorMessage))
			KLS_Log.Info(string.Format("Exported %1 item(s) to %2 (ExportItems = 1 in Settings.json).", file.Items.Count(), FILE_ITEMS));
		else
			KLS_Log.Error("Could not write " + FILE_ITEMS + ": " + errorMessage);
	}

	protected static void ExportRoot(string root, array<ref KLS_ExportedItem> items)
	{
		int count = GetGame().ConfigGetChildrenCount(root);
		for (int index = 0; index < count; index++)
		{
			string className;
			if (!GetGame().ConfigGetChildName(root, index, className)) continue;

			string path = root + " " + className;
			if (GetGame().ConfigGetInt(path + " scope") != 2) continue;
			if (!GetGame().ConfigIsExisting(path + " itemSize")) continue;

			TIntArray size = new TIntArray();
			GetGame().ConfigGetIntArray(path + " itemSize", size);

			KLS_ExportedItem exported = new KLS_ExportedItem();
			exported.Item = className;
			exported.Root = root;
			exported.Size = size;
			AddBases(root, className, exported.Bases);
			items.Insert(exported);
		}
	}

	protected static void AddBases(string root, string className, array<string> bases)
	{
		string current = className;
		for (int depth = 0; depth < MAX_BASES; depth++)
		{
			string baseName;
			if (!GetGame().ConfigGetBaseName(root + " " + current, baseName)) break;
			if (baseName == "" || baseName == current) break;

			bases.Insert(baseName);
			current = baseName;
		}
	}
}
