// KrydenLootSystem — constantes compartilhadas. 3_Game carrega antes de 4_World e 5_Mission,
// e este PBO (Client) carrega em cliente E servidor: o que esta aqui vale para os dois.
class KrydenLootSystemConst
{
	static const string MOD_NAME = "KrydenLootSystem";
	static const string VERSION  = "0.1.0";
	static const string LOG_TAG  = "[KrydenLootSystem] ";

	// Tempo do Vasculhar quando o servidor ainda nao mandou o do Settings.json (D-11).
	static const float DEFAULT_SEARCH_TIME = 3.0;

	// Som do Vasculhar pela categoria da animacao de craft (ActionBase.Start -> SetSoundCategoryHash,
	// actionbase.c:754): "Rags_craft" e o rasgar roupa em trapo do vanilla (craftrag.c) - pedido do Morette.
	static const string SEARCH_SOUND_CATEGORY = "Rags_craft";

	// Loop tocado NO STORAGE (estado sincronizado), em todo cliente proximo. A animacao em pe nao dispara a
	// categoria acima (provado in-game 03/10/2026), entao o som vem daqui. O Cloth_Back_* (som de roupa do
	// personagem andando) nao saiu num objeto; o de rasgar roupa em trapo e som de craft comum.
	static const string SEARCH_SOUNDSET = "Rags_craft_SoundSet";
}
