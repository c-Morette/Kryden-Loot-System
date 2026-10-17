// KrydenLootSystem (Server/5_Mission): carrega as pools no boot, para os avisos de classname
// errado sairem no log logo na subida (RF-27), e solta o singleton no fim da missao.
modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		KLS_Loot.Get();
	}

	override void OnMissionFinish()
	{
		KLS_Loot.Destroy();
		super.OnMissionFinish();
	}
}
