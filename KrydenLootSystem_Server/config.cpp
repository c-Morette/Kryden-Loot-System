class CfgPatches
{
	class KrydenLootSystem_Server
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		// Carrega depois do Client: os scripts daqui podem estender classes de la.
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "KrydenLootSystem_Client"};
	};
};

class CfgMods
{
	class KrydenLootSystem_Server
	{
		dir = "Kryden/KrydenLootSystem/KrydenLootSystem_Server";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 1;
		name = "KrydenLootSystem - Server";
		credits = "";
		author = "Kryden";
		authorID = "0";
		version = "0.1.0";
		extra = 0;
		// type = "mod" nos DOIS lados (servermod quebra o cliente que carregar o PBO).
		// A separacao e logica: guardas GetGame().IsServer() + hooks, nao empacotamento.
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Server/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Server/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Server/scripts/5_Mission"};
			};
		};
	};
};