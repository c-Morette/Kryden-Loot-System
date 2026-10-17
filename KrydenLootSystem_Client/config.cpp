class CfgPatches
{
	class KrydenLootSystem_Client
	{
		units[] =
		{
			"KLS_WoodenCrate",
			"KLS_WoodenCrate_T1",
			"KLS_WoodenCrate_T2",
			"KLS_WoodenCrate_T3",
			"KLS_WoodenCrate_T4",
			"KLS_WoodenCrate_Military",
			"KLS_WoodenCrate_Military_T1",
			"KLS_WoodenCrate_Military_T2",
			"KLS_WoodenCrate_Military_T3",
			"KLS_WoodenCrate_Military_T4",
			"KLS_WoodenCrate_Police",
			"KLS_WoodenCrate_Police_T1",
			"KLS_WoodenCrate_Police_T2",
			"KLS_WoodenCrate_Police_T3",
			"KLS_WoodenCrate_Police_T4",
			"KLS_WoodenCrate_Toxic",
			"KLS_WoodenCrate_Toxic_T1",
			"KLS_WoodenCrate_Toxic_T2",
			"KLS_WoodenCrate_Toxic_T3",
			"KLS_WoodenCrate_Toxic_T4",
			"KLS_HuntingBag",
			"KLS_HuntingBag_T1",
			"KLS_HuntingBag_T2",
			"KLS_HuntingBag_T3",
			"KLS_HuntingBag_T4",
			"KLS_MountainBag",
			"KLS_MountainBag_T1",
			"KLS_MountainBag_T2",
			"KLS_MountainBag_T3",
			"KLS_MountainBag_T4",
			"KLS_ChildBag",
			"KLS_ChildBag_T1",
			"KLS_ChildBag_T2",
			"KLS_ChildBag_T3",
			"KLS_ChildBag_T4",
			"KLS_CoyoteBag",
			"KLS_CoyoteBag_T1",
			"KLS_CoyoteBag_T2",
			"KLS_CoyoteBag_T3",
			"KLS_CoyoteBag_T4",
			"KLS_AssaultBag",
			"KLS_AssaultBag_T1",
			"KLS_AssaultBag_T2",
			"KLS_AssaultBag_T3",
			"KLS_AssaultBag_T4",
			"KLS_TTsKOJacket",
			"KLS_TTsKOJacket_T1",
			"KLS_TTsKOJacket_T2",
			"KLS_TTsKOJacket_T3",
			"KLS_TTsKOJacket_T4",
			"KLS_QuiltedJacket",
			"KLS_QuiltedJacket_T1",
			"KLS_QuiltedJacket_T2",
			"KLS_QuiltedJacket_T3",
			"KLS_QuiltedJacket_T4",
			"KLS_ParamedicJacket",
			"KLS_ParamedicJacket_T1",
			"KLS_ParamedicJacket_T2",
			"KLS_ParamedicJacket_T3",
			"KLS_ParamedicJacket_T4",
			"KLS_PoliceJacket",
			"KLS_PoliceJacket_T1",
			"KLS_PoliceJacket_T2",
			"KLS_PoliceJacket_T3",
			"KLS_PoliceJacket_T4",
			"KLS_WeaponCase",
			"KLS_WeaponCase_T1",
			"KLS_WeaponCase_T2",
			"KLS_WeaponCase_T3",
			"KLS_WeaponCase_T4",
			"KLS_WeaponCase_Police",
			"KLS_WeaponCase_Police_T1",
			"KLS_WeaponCase_Police_T2",
			"KLS_WeaponCase_Police_T3",
			"KLS_WeaponCase_Police_T4",
			"KLS_WeaponCase_Hunting",
			"KLS_WeaponCase_Hunting_T1",
			"KLS_WeaponCase_Hunting_T2",
			"KLS_WeaponCase_Hunting_T3",
			"KLS_WeaponCase_Hunting_T4",
			"KLS_MedKit",
			"KLS_MedKit_T1",
			"KLS_MedKit_T2",
			"KLS_MedKit_T3",
			"KLS_MedKit_T4",
			"KLS_MilitarySupply",
			"KLS_MilitarySupply_T1",
			"KLS_MilitarySupply_T2",
			"KLS_MilitarySupply_T3",
			"KLS_MilitarySupply_T4",
			"KLS_FoodSack",
			"KLS_FoodSack_T1",
			"KLS_FoodSack_T2",
			"KLS_FoodSack_T3",
			"KLS_FoodSack_T4",
			"KLS_ToolChest",
			"KLS_ToolChest_T1",
			"KLS_ToolChest_T2",
			"KLS_ToolChest_T3",
			"KLS_ToolChest_T4"
		};
		weapons[] = {};
		requiredVersion = 0.1;
		// As tres DZ_* sao donas dos modelos, texturas e rvmats usados abaixo.
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Camping", "DZ_Characters_Backpacks", "DZ_Characters_Tops", "DZ_Gear_Containers"};
	};
};

class CfgMods
{
	class KrydenLootSystem_Client
	{
		dir = "Kryden/KrydenLootSystem/KrydenLootSystem_Client";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 1;
		name = "KrydenLootSystem - Client";
		credits = "";
		author = "Kryden";
		authorID = "0";
		version = "0.1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		// O prefixo do PBO e "Kryden\KrydenLootSystem\KrydenLootSystem_Client" ($PBOPREFIX$): todo caminho aqui usa ele.
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Client/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Client/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Kryden/KrydenLootSystem/KrydenLootSystem_Client/scripts/5_Mission"};
			};
		};
	};
};

// ---------------------------------------------------------------------
// Storages (D-06, D-18). Classe NOVA derivada da nossa base, que herda Container_Base: do item vanilla
// so vem o modelo de chao (_g), as texturas e o rvmat de dano (copiados do config vanilla). Nada de
// config de roupa/mochila: o storage nao e item, nao vai para slot e nao cabe em cargo.
//
// O script de todas e o KLS_Storage_Base (4_World/KLS_Storage.c): a engine sobe o config ate achar
// uma classe de script com o nome.
//
// Tier (D-13): _T1.._T4 sao so config, para o types.xml por cada variante na zona certa. A classe sem
// sufixo e a variante "sem tier".
// ---------------------------------------------------------------------
class CfgVehicles
{
	class Container_Base;

	class KLS_Storage_Base: Container_Base
	{
		scope = 0;
		descriptionShort = "$STR_KLS_Storage_Desc";
		weight = 1000;
		// Grande demais para caber em qualquer cargo.
		itemSize[] = {10, 10};
		itemBehaviour = 0;
		canBeDigged = 0;
		carveNavmesh = 1;
		// Todo storage com cargo 10x10 (03/10/2026, Morette): cabe mochila grande e arma em qualquer um, sem
		// sorteio desperdicado. O modelo nao precisa bater com o cargo (o vanilla tambem nao bate).
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
	};

	// WoodenCrate
	class KLS_WoodenCrate: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_WoodenCrate";
		rotationFlags = 2;
		model="\DZ\gear\camping\wooden_case.p3d";
		hiddenSelections[]=
		{
			"camoGround"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\gear\camping\data\wooden_case_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=400;
					transferToAttachmentsCoef=0.059999999;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\gear\camping\data\wooden_case.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\gear\camping\data\wooden_case.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\gear\camping\data\wooden_case_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\gear\camping\data\wooden_case_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\gear\camping\data\wooden_case_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class FragGrenade
				{
					class Health
					{
						damage=8;
					};
					class Blood
					{
						damage=8;
					};
					class Shock
					{
						damage=8;
					};
				};
			};
		};
	};
	class KLS_WoodenCrate_T1: KLS_WoodenCrate {};
	class KLS_WoodenCrate_T2: KLS_WoodenCrate {};
	class KLS_WoodenCrate_T3: KLS_WoodenCrate {};
	class KLS_WoodenCrate_T4: KLS_WoodenCrate {};
	// Familias do caixote (04/10/2026): o caixote serve usos de loot diferentes, e cada familia tem a usage
	// dela no types. O CE, que sabe o uso do ponto, escolhe a variante; o Storages.json da a pool da familia.
	class KLS_WoodenCrate_Military: KLS_WoodenCrate {};
	class KLS_WoodenCrate_Military_T1: KLS_WoodenCrate_Military {};
	class KLS_WoodenCrate_Military_T2: KLS_WoodenCrate_Military {};
	class KLS_WoodenCrate_Military_T3: KLS_WoodenCrate_Military {};
	class KLS_WoodenCrate_Military_T4: KLS_WoodenCrate_Military {};
	class KLS_WoodenCrate_Police: KLS_WoodenCrate {};
	class KLS_WoodenCrate_Police_T1: KLS_WoodenCrate_Police {};
	class KLS_WoodenCrate_Police_T2: KLS_WoodenCrate_Police {};
	class KLS_WoodenCrate_Police_T3: KLS_WoodenCrate_Police {};
	class KLS_WoodenCrate_Police_T4: KLS_WoodenCrate_Police {};
	class KLS_WoodenCrate_Toxic: KLS_WoodenCrate {};
	class KLS_WoodenCrate_Toxic_T1: KLS_WoodenCrate_Toxic {};
	class KLS_WoodenCrate_Toxic_T2: KLS_WoodenCrate_Toxic {};
	class KLS_WoodenCrate_Toxic_T3: KLS_WoodenCrate_Toxic {};
	class KLS_WoodenCrate_Toxic_T4: KLS_WoodenCrate_Toxic {};

	// HuntingBag
	class KLS_HuntingBag: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Backpack";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 16;
		model="\dz\characters\backpacks\hunting_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\backpacks\data\hunting_co.paa",
			"\dz\characters\backpacks\data\hunting_co.paa",
			"\dz\characters\backpacks\data\hunting_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=160;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\Characters\backpacks\data\hunting.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\Characters\backpacks\data\hunting.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\Characters\backpacks\data\hunting_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\Characters\backpacks\data\hunting_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\Characters\backpacks\data\hunting_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_HuntingBag_T1: KLS_HuntingBag {};
	class KLS_HuntingBag_T2: KLS_HuntingBag {};
	class KLS_HuntingBag_T3: KLS_HuntingBag {};
	class KLS_HuntingBag_T4: KLS_HuntingBag {};

	// MountainBag_Green
	class KLS_MountainBag: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Backpack";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 16;
		model="\dz\characters\backpacks\mountain_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"DZ\characters\backpacks\data\Mountain_Green_co.paa",
			"DZ\characters\backpacks\data\Mountain_Green_co.paa",
			"DZ\characters\backpacks\data\Mountain_Green_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=160;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\Characters\backpacks\data\mountain.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\Characters\backpacks\data\mountain.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\Characters\backpacks\data\mountain_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\Characters\backpacks\data\mountain_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\Characters\backpacks\data\mountain_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_MountainBag_T1: KLS_MountainBag {};
	class KLS_MountainBag_T2: KLS_MountainBag {};
	class KLS_MountainBag_T3: KLS_MountainBag {};
	class KLS_MountainBag_T4: KLS_MountainBag {};

	// ChildBag_Red
	class KLS_ChildBag: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Backpack";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 16;
		model="\dz\characters\backpacks\childs_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"DZ\characters\backpacks\data\childs_Red_co.paa",
			"DZ\characters\backpacks\data\childs_Red_co.paa",
			"DZ\characters\backpacks\data\childs_Red_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=120;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\Characters\backpacks\data\childs.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\Characters\backpacks\data\childs.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\Characters\backpacks\data\childs_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\Characters\backpacks\data\childs_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\Characters\backpacks\data\childs_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_ChildBag_T1: KLS_ChildBag {};
	class KLS_ChildBag_T2: KLS_ChildBag {};
	class KLS_ChildBag_T3: KLS_ChildBag {};
	class KLS_ChildBag_T4: KLS_ChildBag {};

	// CoyoteBag_Brown
	class KLS_CoyoteBag: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Backpack";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 16;
		model="\dz\characters\backpacks\CoyotesBP_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\backpacks\data\CoyoteBP_co.paa",
			"\dz\characters\backpacks\data\CoyoteBP_co.paa",
			"\dz\characters\backpacks\data\CoyoteBP_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=190;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\Characters\backpacks\data\CoyotesBP.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\Characters\backpacks\data\CoyotesBP.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\Characters\backpacks\data\CoyotesBP_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\Characters\backpacks\data\CoyotesBP_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\Characters\backpacks\data\CoyotesBP_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_CoyoteBag_T1: KLS_CoyoteBag {};
	class KLS_CoyoteBag_T2: KLS_CoyoteBag {};
	class KLS_CoyoteBag_T3: KLS_CoyoteBag {};
	class KLS_CoyoteBag_T4: KLS_CoyoteBag {};

	// AssaultBag_Ttsko
	class KLS_AssaultBag: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Backpack";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 16;
		model="\dz\characters\backpacks\boulder_m53_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\backpacks\data\boulder_m53_ttsko_co.paa",
			"\dz\characters\backpacks\data\boulder_m53_ttsko_co.paa",
			"\dz\characters\backpacks\data\boulder_m53_ttsko_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=170;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\Characters\backpacks\data\boulder_m53.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\Characters\backpacks\data\boulder_m53.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\Characters\backpacks\data\boulder_m53_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\Characters\backpacks\data\boulder_m53_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\Characters\backpacks\data\boulder_m53_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_AssaultBag_T1: KLS_AssaultBag {};
	class KLS_AssaultBag_T2: KLS_AssaultBag {};
	class KLS_AssaultBag_T3: KLS_AssaultBag {};
	class KLS_AssaultBag_T4: KLS_AssaultBag {};

	// TTsKOJacket_Camo
	class KLS_TTsKOJacket: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Jacket";
		model="\DZ\characters\tops\TTsKO_Jacket_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\tops\data\TTsKO_Jacket_g_camo_co.paa",
			"\dz\characters\tops\data\TTsKO_Jacket_camo_co.paa",
			"\dz\characters\tops\data\TTsKO_Jacket_camo_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=180;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\characters\tops\Data\TTsKO_Jacket.rvmat",
								"DZ\characters\tops\Data\TTsKO_Jacket_g.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\characters\tops\Data\TTsKO_Jacket.rvmat",
								"DZ\characters\tops\Data\TTsKO_Jacket_g.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\characters\tops\Data\TTsKO_Jacket_damage.rvmat",
								"DZ\characters\tops\Data\TTsKO_Jacket_g_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\characters\tops\Data\TTsKO_Jacket_damage.rvmat",
								"DZ\characters\tops\Data\TTsKO_Jacket_g_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\characters\tops\Data\TTsKO_Jacket_destruct.rvmat",
								"DZ\characters\tops\Data\TTsKO_Jacket_g_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class Melee
				{
					class Health
					{
						damage=0.89999998;
					};
					class Blood
					{
						damage=0.89999998;
					};
					class Shock
					{
						damage=1;
					};
				};
				class Infected
				{
					class Health
					{
						damage=0.89999998;
					};
					class Blood
					{
						damage=0.89999998;
					};
					class Shock
					{
						damage=1;
					};
				};
			};
		};
	};
	class KLS_TTsKOJacket_T1: KLS_TTsKOJacket {};
	class KLS_TTsKOJacket_T2: KLS_TTsKOJacket {};
	class KLS_TTsKOJacket_T3: KLS_TTsKOJacket {};
	class KLS_TTsKOJacket_T4: KLS_TTsKOJacket {};

	// QuiltedJacket_Black
	class KLS_QuiltedJacket: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Jacket";
		// Igual ao vanilla: orientacao no chao (sem isto a mochila nascia em pe, 03/10/2026).
		rotationFlags = 34;
		model="\DZ\characters\tops\QuiltedJacket_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\tops\data\QuiltedJacket_black_co.paa",
			"\dz\characters\tops\data\QuiltedJacket_black_co.paa",
			"\dz\characters\tops\data\QuiltedJacket_black_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=120;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\characters\tops\Data\QuiltedJacket.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\characters\tops\Data\QuiltedJacket.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\characters\tops\Data\QuiltedJacket_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\characters\tops\Data\QuiltedJacket_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\characters\tops\Data\QuiltedJacket_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class Melee
				{
					class Health
					{
						damage=0.94999999;
					};
					class Blood
					{
						damage=1;
					};
					class Shock
					{
						damage=1;
					};
				};
				class Infected
				{
					class Health
					{
						damage=0.94999999;
					};
					class Blood
					{
						damage=1;
					};
					class Shock
					{
						damage=1;
					};
				};
			};
		};
	};
	class KLS_QuiltedJacket_T1: KLS_QuiltedJacket {};
	class KLS_QuiltedJacket_T2: KLS_QuiltedJacket {};
	class KLS_QuiltedJacket_T3: KLS_QuiltedJacket {};
	class KLS_QuiltedJacket_T4: KLS_QuiltedJacket {};

	// ParamedicJacket_Blue
	class KLS_ParamedicJacket: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Jacket";
		model="\DZ\characters\tops\ParaMedic_Jacket_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\tops\data\Paramedic_Jacket_g_blue_co.paa",
			"\dz\characters\tops\data\Paramedic_Jacket_blue_co.paa",
			"\dz\characters\tops\data\Paramedic_Jacket_blue_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=160;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\characters\tops\Data\Paramedic_Jacket.rvmat",
								"DZ\characters\tops\Data\Paramedic_Jacket_g.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\characters\tops\Data\Paramedic_Jacket.rvmat",
								"DZ\characters\tops\Data\Paramedic_Jacket_g.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\characters\tops\Data\Paramedic_Jacket_damage.rvmat",
								"DZ\characters\tops\Data\Paramedic_Jacket_g_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\characters\tops\Data\Paramedic_Jacket_damage.rvmat",
								"DZ\characters\tops\Data\Paramedic_Jacket_g_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\characters\tops\Data\Paramedic_Jacket_destruct.rvmat",
								"DZ\characters\tops\Data\Paramedic_Jacket_g_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class Melee
				{
					class Health
					{
						damage=0.89999998;
					};
					class Blood
					{
						damage=0.89999998;
					};
					class Shock
					{
						damage=1;
					};
				};
				class Infected
				{
					class Health
					{
						damage=0.89999998;
					};
					class Blood
					{
						damage=0.89999998;
					};
					class Shock
					{
						damage=1;
					};
				};
			};
		};
	};
	class KLS_ParamedicJacket_T1: KLS_ParamedicJacket {};
	class KLS_ParamedicJacket_T2: KLS_ParamedicJacket {};
	class KLS_ParamedicJacket_T3: KLS_ParamedicJacket {};
	class KLS_ParamedicJacket_T4: KLS_ParamedicJacket {};

	// PoliceJacket
	class KLS_PoliceJacket: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_Jacket";
		model="\DZ\characters\tops\PoliceJacket_g.p3d";
		hiddenSelections[]=
		{
			"camoGround",
			"camoMale",
			"camoFemale"
		};
		hiddenSelectionsTextures[]=
		{
			"\dz\characters\tops\data\PoliceJacket_g_co.paa",
			"\dz\characters\tops\data\PoliceJacket_co.paa",
			"\dz\characters\tops\data\PoliceJacket_co.paa"
		};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=120;
					healthLevels[]=
					{
				
						{
							1,
					
							{
								"DZ\characters\tops\Data\PoliceJacket.rvmat",
								"DZ\characters\tops\Data\PoliceJacket_g.rvmat"
							}
						},
				
						{
							0.69999999,
					
							{
								"DZ\characters\tops\Data\PoliceJacket.rvmat",
								"DZ\characters\tops\Data\PoliceJacket_g.rvmat"
							}
						},
				
						{
							0.5,
					
							{
								"DZ\characters\tops\Data\PoliceJacket_damage.rvmat",
								"DZ\characters\tops\Data\PoliceJacket_g_damage.rvmat"
							}
						},
				
						{
							0.30000001,
					
							{
								"DZ\characters\tops\Data\PoliceJacket_damage.rvmat",
								"DZ\characters\tops\Data\PoliceJacket_g_damage.rvmat"
							}
						},
				
						{
							0,
					
							{
								"DZ\characters\tops\Data\PoliceJacket_destruct.rvmat",
								"DZ\characters\tops\Data\PoliceJacket_g_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class Melee
				{
					class Health
					{
						damage=0.94999999;
					};
					class Blood
					{
						damage=1;
					};
					class Shock
					{
						damage=1;
					};
				};
				class Infected
				{
					class Health
					{
						damage=0.94999999;
					};
					class Blood
					{
						damage=1;
					};
					class Shock
					{
						damage=1;
					};
				};
			};
		};
	};
	class KLS_PoliceJacket_T1: KLS_PoliceJacket {};
	class KLS_PoliceJacket_T2: KLS_PoliceJacket {};
	class KLS_PoliceJacket_T3: KLS_PoliceJacket {};
	class KLS_PoliceJacket_T4: KLS_PoliceJacket {};

	// ===== Storages tematicos (C, 04/10/2026): modelo vanilla proprio por tema, para o jogador reconhecer de
	// longe. O que sai (garantia, categoria) fica no Storages.json. =====
	// Maleta de armas: caixote vanilla pintado de verde militar (04/10/2026). Era o SmallProtectorCase, mas a
	// maleta vanilla tambem sai no loot e pode ser pega, e o modelo dela nao aceita textura propria.
	// 1 arma longa garantida + municao dela. A classe base e a familia militar.
	class KLS_WeaponCase: KLS_WoodenCrate
	{
		displayName = "$STR_KLS_WeaponCase";
		hiddenSelectionsTextures[] = {"Kryden\KrydenLootSystem\KrydenLootSystem_Client\data\kls_weaponcase_co.paa"};
	};
	class KLS_WeaponCase_T1: KLS_WeaponCase {};
	class KLS_WeaponCase_T2: KLS_WeaponCase {};
	class KLS_WeaponCase_T3: KLS_WeaponCase {};
	class KLS_WeaponCase_T4: KLS_WeaponCase {};
	class KLS_WeaponCase_Police: KLS_WeaponCase {};
	class KLS_WeaponCase_Police_T1: KLS_WeaponCase_Police {};
	class KLS_WeaponCase_Police_T2: KLS_WeaponCase_Police {};
	class KLS_WeaponCase_Police_T3: KLS_WeaponCase_Police {};
	class KLS_WeaponCase_Police_T4: KLS_WeaponCase_Police {};
	class KLS_WeaponCase_Hunting: KLS_WeaponCase {};
	class KLS_WeaponCase_Hunting_T1: KLS_WeaponCase_Hunting {};
	class KLS_WeaponCase_Hunting_T2: KLS_WeaponCase_Hunting {};
	class KLS_WeaponCase_Hunting_T3: KLS_WeaponCase_Hunting {};
	class KLS_WeaponCase_Hunting_T4: KLS_WeaponCase_Hunting {};
	// Kit medico (FirstAidKit): so item medico.
	class KLS_MedKit: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_MedKit";
		rotationFlags = 17;
		model = "\dz\gear\containers\FirstAidKit.p3d";
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=80;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"DZ\gear\containers\data\FirsAidKit.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"DZ\gear\containers\data\FirsAidKit.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"DZ\gear\containers\data\FirsAidKit_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"DZ\gear\containers\data\FirsAidKit_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"DZ\gear\containers\data\FirsAidKit_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_MedKit_T1: KLS_MedKit {};
	class KLS_MedKit_T2: KLS_MedKit {};
	class KLS_MedKit_T3: KLS_MedKit {};
	class KLS_MedKit_T4: KLS_MedKit {};
	// Suprimento militar (AliceBag verde): comida e municao.
	class KLS_MilitarySupply: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_MilitarySupply";
		rotationFlags = 16;
		model = "\dz\characters\backpacks\alicebackpack_g.p3d";
		hiddenSelections[] = {"camoGround", "camoMale", "camoFemale"};
		hiddenSelectionsTextures[] = {"\dz\characters\backpacks\data\alicebackpack_co.paa", "\dz\characters\backpacks\data\alicebackpack_co.paa", "\dz\characters\backpacks\data\alicebackpack_co.paa"};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=160;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"DZ\Characters\backpacks\data\Alice_backpack.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"DZ\Characters\backpacks\data\Alice_backpack.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"DZ\Characters\backpacks\data\Alice_backpack_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"DZ\Characters\backpacks\data\Alice_backpack_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"DZ\Characters\backpacks\data\Alice_backpack_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_MilitarySupply_T1: KLS_MilitarySupply {};
	class KLS_MilitarySupply_T2: KLS_MilitarySupply {};
	class KLS_MilitarySupply_T3: KLS_MilitarySupply {};
	class KLS_MilitarySupply_T4: KLS_MilitarySupply {};
	// Saco de comida (CanvasBag oliva): so comida.
	class KLS_FoodSack: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_FoodSack";
		rotationFlags = 2;
		model = "\dz\characters\backpacks\CanvasBag_g.p3d";
		hiddenSelections[] = {"camoGround", "camoMale", "camoFemale"};
		hiddenSelectionsTextures[] = {"\dz\characters\backpacks\data\CanvasBag_olive_CO.paa", "\dz\characters\backpacks\data\CanvasBag_olive_CO.paa", "\dz\characters\backpacks\data\CanvasBag_olive_CO.paa"};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=140;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"DZ\Characters\backpacks\data\CanvasBag.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"DZ\Characters\backpacks\data\CanvasBag.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"DZ\Characters\backpacks\data\CanvasBag_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"DZ\Characters\backpacks\data\CanvasBag_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"DZ\Characters\backpacks\data\CanvasBag_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class KLS_FoodSack_T1: KLS_FoodSack {};
	class KLS_FoodSack_T2: KLS_FoodSack {};
	class KLS_FoodSack_T3: KLS_FoodSack {};
	class KLS_FoodSack_T4: KLS_FoodSack {};
	// Bau de ferramentas (SeaChest PINTADO de amarelo: o SeaChest vanilla tambem nasce no chao e pode ser
	// pego; a cor evita confundir). So ferramenta e material de construcao.
	class KLS_ToolChest: KLS_Storage_Base
	{
		scope = 2;
		displayName = "$STR_KLS_ToolChest";
		rotationFlags = 2;
		model = "\DZ\gear\camping\sea_chest.p3d";
		hiddenSelections[] = {"camoGround"};
		hiddenSelectionsTextures[] = {"Kryden\KrydenLootSystem\KrydenLootSystem_Client\data\kls_toolchest_co.paa"};
		class Cargo
		{
			itemsCargoSize[] = {10, 10};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=600;
					transferToAttachmentsCoef=0.059999999;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"DZ\gear\camping\data\sea_chest.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"DZ\gear\camping\data\sea_chest.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"DZ\gear\camping\data\sea_chest_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"DZ\gear\camping\data\sea_chest_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"DZ\gear\camping\data\sea_chest_destruct.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class FragGrenade
				{
					class Health
					{
						damage=8;
					};
					class Blood
					{
						damage=8;
					};
					class Shock
					{
						damage=8;
					};
				};
			};
		};
	};
	class KLS_ToolChest_T1: KLS_ToolChest {};
	class KLS_ToolChest_T2: KLS_ToolChest {};
	class KLS_ToolChest_T3: KLS_ToolChest {};
	class KLS_ToolChest_T4: KLS_ToolChest {};
};
