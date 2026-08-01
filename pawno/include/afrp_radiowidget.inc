/*
	afrp_radiowidget.inc - Icone "en transmission radio" a l'ecran

	Le talkie fourni par l'utilisateur (fichier radinholapa.txt) n'a aucun
	texte : c'est une icone seule. On l'affiche pendant qu'un joueur parle
	en mode radio (talkStats == 3, bascule via /radio), et on la cache des
	qu'il arrete de parler ou repasse en mode local (/local).

	=== POURQUOI SEULEMENT PENDANT LA TRANSMISSION ===
	Un indicatif permanent aurait ete un element de plus en jeu pour rien
	les 95% du temps ou personne ne parle. Il ne coute rien au budget de
	textdraws sauf le court instant ou quelqu'un transmet vraiment.

	=== POURQUOI PAS PENDANT LE SAC OU LE TELEPHONE ===
	L'icone occupe x353-460 / y225-355, une zone qui chevauche largement le
	panneau du sac (x347-402 / y168-339). L'afficher par-dessus aurait
	rendu les deux illisibles. On saute simplement l'affichage si l'un des
	deux est deja ouvert au moment ou le joueur commence a parler.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit  -> RadioWidget_Init()
	- OnPlayerVoiceStart -> RadioWidget_OnVoiceStart(playerid)
	- OnPlayerVoiceStop  -> RadioWidget_OnVoiceStop(playerid)
	- OnPlayerDisconnect -> RadioWidget_OnDisconnect(playerid)
*/

#if defined AFRP_RADIOWIDGET_INCLUDED
	#endinput
#endif
#define AFRP_RADIOWIDGET_INCLUDED

#define RADIOWIDGET_COUNT  23

new Text:radioWidgetTD[RADIOWIDGET_COUNT];
new bool:radioWidgetAffiche[MAX_PLAYERS];

stock RadioWidget_Init()
{
	radioWidgetTD[0] = TextDrawCreate(358.000, 334.000, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[0], 111.000, 148.000);
	TextDrawAlignment(radioWidgetTD[0], 1);
	TextDrawColor(radioWidgetTD[0], 168430335);
	TextDrawSetShadow(radioWidgetTD[0], 0);
	TextDrawSetOutline(radioWidgetTD[0], 0);
	TextDrawBackgroundColor(radioWidgetTD[0], 255);
	TextDrawFont(radioWidgetTD[0], 4);
	TextDrawSetProportional(radioWidgetTD[0], 1);

	radioWidgetTD[1] = TextDrawCreate(353.899, 320.000, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[1], 25.000, 25.000);
	TextDrawAlignment(radioWidgetTD[1], 1);
	TextDrawColor(radioWidgetTD[1], 168430335);
	TextDrawSetShadow(radioWidgetTD[1], 0);
	TextDrawSetOutline(radioWidgetTD[1], 0);
	TextDrawBackgroundColor(radioWidgetTD[1], 255);
	TextDrawFont(radioWidgetTD[1], 4);
	TextDrawSetProportional(radioWidgetTD[1], 1);

	radioWidgetTD[2] = TextDrawCreate(448.000, 320.000, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[2], 25.100, 25.000);
	TextDrawAlignment(radioWidgetTD[2], 1);
	TextDrawColor(radioWidgetTD[2], 168430335);
	TextDrawSetShadow(radioWidgetTD[2], 0);
	TextDrawSetOutline(radioWidgetTD[2], 0);
	TextDrawBackgroundColor(radioWidgetTD[2], 255);
	TextDrawFont(radioWidgetTD[2], 4);
	TextDrawSetProportional(radioWidgetTD[2], 1);

	radioWidgetTD[3] = TextDrawCreate(367.000, 324.398, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[3], 92.000, 148.000);
	TextDrawAlignment(radioWidgetTD[3], 1);
	TextDrawColor(radioWidgetTD[3], 168430335);
	TextDrawSetShadow(radioWidgetTD[3], 0);
	TextDrawSetOutline(radioWidgetTD[3], 0);
	TextDrawBackgroundColor(radioWidgetTD[3], 255);
	TextDrawFont(radioWidgetTD[3], 4);
	TextDrawSetProportional(radioWidgetTD[3], 1);

	radioWidgetTD[4] = TextDrawCreate(367.000, 355.398, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[4], 92.000, 148.000);
	TextDrawAlignment(radioWidgetTD[4], 1);
	TextDrawColor(radioWidgetTD[4], 598910975);
	TextDrawSetShadow(radioWidgetTD[4], 0);
	TextDrawSetOutline(radioWidgetTD[4], 0);
	TextDrawBackgroundColor(radioWidgetTD[4], 255);
	TextDrawFont(radioWidgetTD[4], 4);
	TextDrawSetProportional(radioWidgetTD[4], 1);

	radioWidgetTD[5] = TextDrawCreate(435.799, 338.700, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[5], 28.100, 31.000);
	TextDrawAlignment(radioWidgetTD[5], 1);
	TextDrawColor(radioWidgetTD[5], 598910975);
	TextDrawSetShadow(radioWidgetTD[5], 0);
	TextDrawSetOutline(radioWidgetTD[5], 0);
	TextDrawBackgroundColor(radioWidgetTD[5], 255);
	TextDrawFont(radioWidgetTD[5], 4);
	TextDrawSetProportional(radioWidgetTD[5], 1);

	radioWidgetTD[6] = TextDrawCreate(362.199, 338.700, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[6], 28.100, 31.000);
	TextDrawAlignment(radioWidgetTD[6], 1);
	TextDrawColor(radioWidgetTD[6], 598910975);
	TextDrawSetShadow(radioWidgetTD[6], 0);
	TextDrawSetOutline(radioWidgetTD[6], 0);
	TextDrawBackgroundColor(radioWidgetTD[6], 255);
	TextDrawFont(radioWidgetTD[6], 4);
	TextDrawSetProportional(radioWidgetTD[6], 1);

	radioWidgetTD[7] = TextDrawCreate(375.700, 343.998, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[7], 75.000, 84.000);
	TextDrawAlignment(radioWidgetTD[7], 1);
	TextDrawColor(radioWidgetTD[7], 598910975);
	TextDrawSetShadow(radioWidgetTD[7], 0);
	TextDrawSetOutline(radioWidgetTD[7], 0);
	TextDrawBackgroundColor(radioWidgetTD[7], 255);
	TextDrawFont(radioWidgetTD[7], 4);
	TextDrawSetProportional(radioWidgetTD[7], 1);

	radioWidgetTD[8] = TextDrawCreate(434.000, 313.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[8], 20.000, 13.000);
	TextDrawAlignment(radioWidgetTD[8], 1);
	TextDrawColor(radioWidgetTD[8], 168430335);
	TextDrawSetShadow(radioWidgetTD[8], 0);
	TextDrawSetOutline(radioWidgetTD[8], 0);
	TextDrawBackgroundColor(radioWidgetTD[8], 255);
	TextDrawFont(radioWidgetTD[8], 4);
	TextDrawSetProportional(radioWidgetTD[8], 1);

	radioWidgetTD[9] = TextDrawCreate(428.997, 303.299, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[9], 30.100, 22.000);
	TextDrawAlignment(radioWidgetTD[9], 1);
	TextDrawColor(radioWidgetTD[9], 168430335);
	TextDrawSetShadow(radioWidgetTD[9], 0);
	TextDrawSetOutline(radioWidgetTD[9], 0);
	TextDrawBackgroundColor(radioWidgetTD[9], 255);
	TextDrawFont(radioWidgetTD[9], 4);
	TextDrawSetProportional(radioWidgetTD[9], 1);

	radioWidgetTD[10] = TextDrawCreate(435.200, 238.399, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[10], 17.698, 76.000);
	TextDrawAlignment(radioWidgetTD[10], 1);
	TextDrawColor(radioWidgetTD[10], 168430335);
	TextDrawSetShadow(radioWidgetTD[10], 0);
	TextDrawSetOutline(radioWidgetTD[10], 0);
	TextDrawBackgroundColor(radioWidgetTD[10], 255);
	TextDrawFont(radioWidgetTD[10], 4);
	TextDrawSetProportional(radioWidgetTD[10], 1);

	radioWidgetTD[11] = TextDrawCreate(430.697, 225.600, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[11], 26.500, 24.000);
	TextDrawAlignment(radioWidgetTD[11], 1);
	TextDrawColor(radioWidgetTD[11], 168430335);
	TextDrawSetShadow(radioWidgetTD[11], 0);
	TextDrawSetOutline(radioWidgetTD[11], 0);
	TextDrawBackgroundColor(radioWidgetTD[11], 255);
	TextDrawFont(radioWidgetTD[11], 4);
	TextDrawSetProportional(radioWidgetTD[11], 1);

	radioWidgetTD[12] = TextDrawCreate(365.000, 343.000, "PARTICLE:lamp_shad_64");
	TextDrawTextSize(radioWidgetTD[12], 94.000, 138.000);
	TextDrawAlignment(radioWidgetTD[12], 1);
	TextDrawColor(radioWidgetTD[12], -16777318);
	TextDrawSetShadow(radioWidgetTD[12], 0);
	TextDrawSetOutline(radioWidgetTD[12], 0);
	TextDrawBackgroundColor(radioWidgetTD[12], 255);
	TextDrawFont(radioWidgetTD[12], 4);
	TextDrawSetProportional(radioWidgetTD[12], 1);

	radioWidgetTD[13] = TextDrawCreate(401.000, 308.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[13], 20.000, 18.000);
	TextDrawAlignment(radioWidgetTD[13], 1);
	TextDrawColor(radioWidgetTD[13], 168430335);
	TextDrawSetShadow(radioWidgetTD[13], 0);
	TextDrawSetOutline(radioWidgetTD[13], 0);
	TextDrawBackgroundColor(radioWidgetTD[13], 255);
	TextDrawFont(radioWidgetTD[13], 4);
	TextDrawSetProportional(radioWidgetTD[13], 1);

	radioWidgetTD[14] = TextDrawCreate(395.997, 299.299, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[14], 30.100, 22.000);
	TextDrawAlignment(radioWidgetTD[14], 1);
	TextDrawColor(radioWidgetTD[14], 168430335);
	TextDrawSetShadow(radioWidgetTD[14], 0);
	TextDrawSetOutline(radioWidgetTD[14], 0);
	TextDrawBackgroundColor(radioWidgetTD[14], 255);
	TextDrawFont(radioWidgetTD[14], 4);
	TextDrawSetProportional(radioWidgetTD[14], 1);

	radioWidgetTD[15] = TextDrawCreate(369.000, 319.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[15], 20.000, 13.000);
	TextDrawAlignment(radioWidgetTD[15], 1);
	TextDrawColor(radioWidgetTD[15], 168430335);
	TextDrawSetShadow(radioWidgetTD[15], 0);
	TextDrawSetOutline(radioWidgetTD[15], 0);
	TextDrawBackgroundColor(radioWidgetTD[15], 255);
	TextDrawFont(radioWidgetTD[15], 4);
	TextDrawSetProportional(radioWidgetTD[15], 1);

	radioWidgetTD[16] = TextDrawCreate(363.997, 311.299, "LD_BEAT:chit");
	TextDrawTextSize(radioWidgetTD[16], 30.100, 22.000);
	TextDrawAlignment(radioWidgetTD[16], 1);
	TextDrawColor(radioWidgetTD[16], 168430335);
	TextDrawSetShadow(radioWidgetTD[16], 0);
	TextDrawSetOutline(radioWidgetTD[16], 0);
	TextDrawBackgroundColor(radioWidgetTD[16], 255);
	TextDrawFont(radioWidgetTD[16], 4);
	TextDrawSetProportional(radioWidgetTD[16], 1);

	radioWidgetTD[17] = TextDrawCreate(439.000, 350.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[17], 9.000, 6.000);
	TextDrawAlignment(radioWidgetTD[17], 1);
	TextDrawColor(radioWidgetTD[17], -1);
	TextDrawSetShadow(radioWidgetTD[17], 0);
	TextDrawSetOutline(radioWidgetTD[17], 0);
	TextDrawBackgroundColor(radioWidgetTD[17], 255);
	TextDrawFont(radioWidgetTD[17], 4);
	TextDrawSetProportional(radioWidgetTD[17], 1);

	radioWidgetTD[18] = TextDrawCreate(448.200, 351.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[18], 1.600, 4.000);
	TextDrawAlignment(radioWidgetTD[18], 1);
	TextDrawColor(radioWidgetTD[18], -1);
	TextDrawSetShadow(radioWidgetTD[18], 0);
	TextDrawSetOutline(radioWidgetTD[18], 0);
	TextDrawBackgroundColor(radioWidgetTD[18], 255);
	TextDrawFont(radioWidgetTD[18], 4);
	TextDrawSetProportional(radioWidgetTD[18], 1);

	radioWidgetTD[19] = TextDrawCreate(375.200, 350.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[19], 1.600, 5.000);
	TextDrawAlignment(radioWidgetTD[19], 1);
	TextDrawColor(radioWidgetTD[19], -1);
	TextDrawSetShadow(radioWidgetTD[19], 0);
	TextDrawSetOutline(radioWidgetTD[19], 0);
	TextDrawBackgroundColor(radioWidgetTD[19], 255);
	TextDrawFont(radioWidgetTD[19], 4);
	TextDrawSetProportional(radioWidgetTD[19], 1);

	radioWidgetTD[20] = TextDrawCreate(378.200, 349.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[20], 1.600, 6.000);
	TextDrawAlignment(radioWidgetTD[20], 1);
	TextDrawColor(radioWidgetTD[20], -1);
	TextDrawSetShadow(radioWidgetTD[20], 0);
	TextDrawSetOutline(radioWidgetTD[20], 0);
	TextDrawBackgroundColor(radioWidgetTD[20], 255);
	TextDrawFont(radioWidgetTD[20], 4);
	TextDrawSetProportional(radioWidgetTD[20], 1);

	radioWidgetTD[21] = TextDrawCreate(371.200, 352.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[21], 2.599, 3.000);
	TextDrawAlignment(radioWidgetTD[21], 1);
	TextDrawColor(radioWidgetTD[21], -1);
	TextDrawSetShadow(radioWidgetTD[21], 0);
	TextDrawSetOutline(radioWidgetTD[21], 0);
	TextDrawBackgroundColor(radioWidgetTD[21], 255);
	TextDrawFont(radioWidgetTD[21], 4);
	TextDrawSetProportional(radioWidgetTD[21], 1);

	radioWidgetTD[22] = TextDrawCreate(368.000, 329.500, "LD_SPAC:white");
	TextDrawTextSize(radioWidgetTD[22], 90.000, 4.000);
	TextDrawAlignment(radioWidgetTD[22], 1);
	TextDrawColor(radioWidgetTD[22], 421075455);
	TextDrawSetShadow(radioWidgetTD[22], 0);
	TextDrawSetOutline(radioWidgetTD[22], 0);
	TextDrawBackgroundColor(radioWidgetTD[22], 255);
	TextDrawFont(radioWidgetTD[22], 4);
	TextDrawSetProportional(radioWidgetTD[22], 1);
	return 1;
}

stock RadioWidget_OnVoiceStart(playerid)
{
	if(GetPVarInt(playerid, "talkStats") != 3) return 0;
	// Chevauche le panneau du sac : on ne montre jamais les deux ensemble.
	if(svOuvert[playerid] == 1) return 0;
	if(phone_State[playerid] != PHONE_STATE_CLOSED) return 0;

	radioWidgetAffiche[playerid] = true;
	for(new k = 0; k < RADIOWIDGET_COUNT; k++) TextDrawShowForPlayer(playerid, radioWidgetTD[k]);
	return 1;
}

stock RadioWidget_OnVoiceStop(playerid)
{
	if(!radioWidgetAffiche[playerid]) return 0;
	radioWidgetAffiche[playerid] = false;
	for(new k = 0; k < RADIOWIDGET_COUNT; k++) TextDrawHideForPlayer(playerid, radioWidgetTD[k]);
	return 1;
}

stock RadioWidget_OnDisconnect(playerid)
{
	radioWidgetAffiche[playerid] = false;
	return 1;
}
