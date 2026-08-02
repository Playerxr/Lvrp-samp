/*
	afrp_announce.inc - Bandeau visuel pour /a an (Flash News admin)

	=== POURQUOI CE MODULE EXISTE ===
	/a an <texte> existait deja (ligne ~65074 de LVRP.pwn) mais n'etait qu'un
	message OOC (msg_OOC) : facile a manquer dans le flux du chat. La
	proprietaire a fourni Announnce.pwn (bandeau "Announce / By Admin : ...")
	pour lui donner un vrai impact visuel. On garde le message OOC existant
	(utile pour le scroll-back) et on ajoute ce bandeau par-dessus.

	=== POURQUOI DES Text: GLOBAUX (PAS DES PlayerText:) ===
	Le contenu est identique pour tout le monde au meme instant (c'est une
	diffusion), donc pas besoin d'une copie par joueur : on cree les 13
	textdraws une seule fois, on met a jour le texte dynamique avant de les
	montrer a chaque joueur connecte.

	=== DEUX ELEMENTS DYNAMIQUES ===
	PublicTD[3] (a l'origine un "A A A A..." de remplissage) reçoit le
	message ; PublicTD[6] (a l'origine "Https_Kinani", un nom d'exemple)
	reçoit le nom reel de l'admin.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit -> Announce_Init()
	- /a an <texte>  -> Announce_Show(playerid, result) en plus du msg_OOC existant
*/

#if defined AFRP_ANNOUNCE_INCLUDED
	#endinput
#endif
#define AFRP_ANNOUNCE_INCLUDED

#define ANNOUNCE_DURATION_MS  8000

new Text:announceTD[13];
new announceHideTimer = -1;

stock Announce_Init()
{
	announceTD[0] = TextDrawCreate(220.000, 169.000, "ld_dual:white");
	TextDrawFont(announceTD[0], 4);
	TextDrawLetterSize(announceTD[0], 0.600, 2.000);
	TextDrawTextSize(announceTD[0], 185.500, 100.500);
	TextDrawSetOutline(announceTD[0], 1);
	TextDrawSetShadow(announceTD[0], 0);
	TextDrawAlignment(announceTD[0], 1);
	TextDrawColor(announceTD[0], -1);
	TextDrawBackgroundColor(announceTD[0], 255);
	TextDrawBoxColor(announceTD[0], 50);
	TextDrawUseBox(announceTD[0], 1);
	TextDrawSetProportional(announceTD[0], 1);

	announceTD[1] = TextDrawCreate(220.000, 168.000, "ld_dual:white");
	TextDrawFont(announceTD[1], 4);
	TextDrawLetterSize(announceTD[1], 0.600, 2.000);
	TextDrawTextSize(announceTD[1], 185.000, 100.000);
	TextDrawSetOutline(announceTD[1], 1);
	TextDrawSetShadow(announceTD[1], 0);
	TextDrawAlignment(announceTD[1], 1);
	TextDrawColor(announceTD[1], -16776961);
	TextDrawBackgroundColor(announceTD[1], 255);
	TextDrawBoxColor(announceTD[1], 50);
	TextDrawUseBox(announceTD[1], 1);
	TextDrawSetProportional(announceTD[1], 1);

	announceTD[2] = TextDrawCreate(271.000, 168.000, "Announce");
	TextDrawFont(announceTD[2], 1);
	TextDrawLetterSize(announceTD[2], 0.600, 2.000);
	TextDrawTextSize(announceTD[2], 400.000, 17.000);
	TextDrawAlignment(announceTD[2], 1);
	TextDrawColor(announceTD[2], -1);
	TextDrawBackgroundColor(announceTD[2], 255);
	TextDrawSetProportional(announceTD[2], 1);

	// [DYNAMIQUE] message de l'annonce - texte de depart juste pour reserver l'espace
	announceTD[3] = TextDrawCreate(233.000, 204.000, " ");
	TextDrawFont(announceTD[3], 1);
	TextDrawLetterSize(announceTD[3], 0.341666, 1.350);
	TextDrawTextSize(announceTD[3], 400.000, 17.000);
	TextDrawAlignment(announceTD[3], 1);
	TextDrawColor(announceTD[3], -1);
	TextDrawBackgroundColor(announceTD[3], 255);
	TextDrawSetProportional(announceTD[3], 1);

	announceTD[4] = TextDrawCreate(263.000, 188.000, "ld_dual:white");
	TextDrawFont(announceTD[4], 4);
	TextDrawLetterSize(announceTD[4], 0.600, 2.000);
	TextDrawTextSize(announceTD[4], 96.500, 0.500);
	TextDrawSetOutline(announceTD[4], 1);
	TextDrawSetShadow(announceTD[4], 0);
	TextDrawAlignment(announceTD[4], 1);
	TextDrawColor(announceTD[4], -1);
	TextDrawBackgroundColor(announceTD[4], 255);
	TextDrawBoxColor(announceTD[4], 50);
	TextDrawUseBox(announceTD[4], 1);
	TextDrawSetProportional(announceTD[4], 1);

	announceTD[5] = TextDrawCreate(221.000, 260.000, "By Admin :");
	TextDrawFont(announceTD[5], 2);
	TextDrawLetterSize(announceTD[5], 0.116666, 0.850);
	TextDrawTextSize(announceTD[5], 400.000, 17.000);
	TextDrawAlignment(announceTD[5], 1);
	TextDrawColor(announceTD[5], -1);
	TextDrawBackgroundColor(announceTD[5], 255);
	TextDrawSetProportional(announceTD[5], 1);

	// [DYNAMIQUE] nom de l'admin - texte de depart juste pour reserver l'espace
	announceTD[6] = TextDrawCreate(246.000, 260.000, " ");
	TextDrawFont(announceTD[6], 2);
	TextDrawLetterSize(announceTD[6], 0.116666, 0.850);
	TextDrawTextSize(announceTD[6], 400.000, 17.000);
	TextDrawAlignment(announceTD[6], 1);
	TextDrawColor(announceTD[6], -1);
	TextDrawBackgroundColor(announceTD[6], 255);
	TextDrawSetProportional(announceTD[6], 1);

	announceTD[7] = TextDrawCreate(221.000, 159.000, "ld_beat:chit");
	TextDrawFont(announceTD[7], 4);
	TextDrawLetterSize(announceTD[7], 0.600, 2.000);
	TextDrawTextSize(announceTD[7], 7.000, 7.000);
	TextDrawSetOutline(announceTD[7], 1);
	TextDrawSetShadow(announceTD[7], 0);
	TextDrawAlignment(announceTD[7], 1);
	TextDrawColor(announceTD[7], -1);
	TextDrawBackgroundColor(announceTD[7], 255);
	TextDrawBoxColor(announceTD[7], 50);
	TextDrawUseBox(announceTD[7], 1);
	TextDrawSetProportional(announceTD[7], 1);

	announceTD[8] = TextDrawCreate(213.000, 158.000, "ld_beat:chit");
	TextDrawFont(announceTD[8], 4);
	TextDrawLetterSize(announceTD[8], 0.600, 2.000);
	TextDrawTextSize(announceTD[8], 9.000, 9.000);
	TextDrawSetOutline(announceTD[8], 1);
	TextDrawSetShadow(announceTD[8], 0);
	TextDrawAlignment(announceTD[8], 1);
	TextDrawColor(announceTD[8], -16776961);
	TextDrawBackgroundColor(announceTD[8], 255);
	TextDrawBoxColor(announceTD[8], 50);
	TextDrawUseBox(announceTD[8], 1);
	TextDrawSetProportional(announceTD[8], 1);

	announceTD[9] = TextDrawCreate(215.000, 169.000, "ld_beat:chit");
	TextDrawFont(announceTD[9], 4);
	TextDrawLetterSize(announceTD[9], 0.600, 2.000);
	TextDrawTextSize(announceTD[9], 4.000, 4.000);
	TextDrawSetOutline(announceTD[9], 1);
	TextDrawSetShadow(announceTD[9], 0);
	TextDrawAlignment(announceTD[9], 1);
	TextDrawColor(announceTD[9], -1);
	TextDrawBackgroundColor(announceTD[9], 255);
	TextDrawBoxColor(announceTD[9], 50);
	TextDrawUseBox(announceTD[9], 1);
	TextDrawSetProportional(announceTD[9], 1);

	announceTD[10] = TextDrawCreate(399.000, 271.000, "ld_beat:chit");
	TextDrawFont(announceTD[10], 4);
	TextDrawLetterSize(announceTD[10], 0.600, 2.000);
	TextDrawTextSize(announceTD[10], 5.500, 5.500);
	TextDrawSetOutline(announceTD[10], 1);
	TextDrawSetShadow(announceTD[10], 0);
	TextDrawAlignment(announceTD[10], 1);
	TextDrawColor(announceTD[10], -16776961);
	TextDrawBackgroundColor(announceTD[10], 255);
	TextDrawBoxColor(announceTD[10], 50);
	TextDrawUseBox(announceTD[10], 1);
	TextDrawSetProportional(announceTD[10], 1);

	announceTD[11] = TextDrawCreate(406.000, 263.000, "ld_beat:chit");
	TextDrawFont(announceTD[11], 4);
	TextDrawLetterSize(announceTD[11], 0.600, 2.000);
	TextDrawTextSize(announceTD[11], 7.000, 7.000);
	TextDrawSetOutline(announceTD[11], 1);
	TextDrawSetShadow(announceTD[11], 0);
	TextDrawAlignment(announceTD[11], 1);
	TextDrawColor(announceTD[11], -16776961);
	TextDrawBackgroundColor(announceTD[11], 255);
	TextDrawBoxColor(announceTD[11], 50);
	TextDrawUseBox(announceTD[11], 1);
	TextDrawSetProportional(announceTD[11], 1);

	announceTD[12] = TextDrawCreate(404.000, 270.000, "ld_beat:chit");
	TextDrawFont(announceTD[12], 4);
	TextDrawLetterSize(announceTD[12], 0.600, 2.000);
	TextDrawTextSize(announceTD[12], 9.000, 9.000);
	TextDrawSetOutline(announceTD[12], 1);
	TextDrawSetShadow(announceTD[12], 0);
	TextDrawAlignment(announceTD[12], 1);
	TextDrawColor(announceTD[12], -1);
	TextDrawBackgroundColor(announceTD[12], 255);
	TextDrawSetProportional(announceTD[12], 1);
	return 1;
}

// Appele en plus du msg_OOC existant dans /a an. adminName = nom reel de
// l'admin, message = texte tape apres /a an.
stock Announce_Show(adminName[], message[])
{
	TextDrawSetString(announceTD[3], message);
	TextDrawSetString(announceTD[6], adminName);

	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		for(new k = 0; k < 13; k++) TextDrawShowForPlayer(i, announceTD[k]);
	}

	if(announceHideTimer != -1) KillTimer(announceHideTimer);
	announceHideTimer = SetTimer("Announce_HideTick", ANNOUNCE_DURATION_MS, 0);
	return 1;
}

forward Announce_HideTick();
public Announce_HideTick()
{
	announceHideTimer = -1;
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		for(new k = 0; k < 13; k++) TextDrawHideForPlayer(i, announceTD[k]);
	}
	return 1;
}
