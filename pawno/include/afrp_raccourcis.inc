/*
	afrp_raccourcis.inc - Menu de raccourcis (simplifie le gameplay)

	Au lieu de retenir /sac, /job, /emotes, /maison et /vehicule separement,
	/raccourcis ouvre un seul dialogue qui les liste. Meme principe que
	afrp_factionmenu.inc (/menu) : on ne reimplemente rien, on rappelle la
	vraie commande via OnPlayerCommandText, donc toutes les verifications
	existantes (sac achete, job en cours, vehicule a proximite...) restent
	telles quelles.

	  /raccourcis (alias /rapide) : ouvre le menu
	  maintenir KEY_LOOK_BEHIND (regarder en arriere, touche C par defaut)
	  a pied ouvre aussi le menu.

	=== POURQUOI UN MAINTIEN ET PAS UN SIMPLE APPUI ===
	KEY_LOOK_BEHIND sert deja a regarder derriere soi (gere par le client,
	le serveur n'y touche pas et ne peut pas le bloquer). Un appui court
	garde donc ce comportement normal. Le menu ne s'ouvre que si la touche
	reste maintenue 600ms - le meme principe que GTA SA distingue deja un
	appui court d'un maintien pour d'autres actions.

	Hooks a brancher dans LVRP.pwn :
	- OnDialogResponse      -> if(Raccourcis_OnDialog(playerid, dialogid, response, listitem)) return 1;
	- OnPlayerKeyStateChange -> Raccourcis_OnKeyStateChange(playerid, newkeys, oldkeys);
	- OnPlayerDisconnect    -> Raccourcis_OnDisconnect(playerid)
	- /raccourcis, /rapide  -> Raccourcis_Ouvrir(playerid)
*/

#if defined AFRP_RACCOURCIS_INCLUDED
	#endinput
#endif
#define AFRP_RACCOURCIS_INCLUDED

#define DIALOG_RACCOURCIS  9654
#define RACCOURCIS_MAINTIEN_MS  600

new bool:raccourcisTenu[MAX_PLAYERS];

stock Raccourcis_Ouvrir(playerid)
{
	new body[200];
	format(body, sizeof(body),
		"{FFFFFF}Sac (inventaire)\nJob (metier)\nEmotes (animations)\nMaison\nVehicule");
	return ShowPlayerDialog(playerid, DIALOG_RACCOURCIS, DIALOG_STYLE_LIST, "{FFD700}Raccourcis", body, "Ouvrir", "Fermer");
}

stock Raccourcis_OnDialog(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_RACCOURCIS) return 0;
	if(!response) return 1;

	switch(listitem)
	{
		case 0: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sac");
		case 1: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/travail");
		case 2: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/emotes");
		case 3: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/maison");
		case 4: CallLocalFunction("OnPlayerCommandText", "is", playerid, "/vehicule");
	}
	return 1;
}

stock Raccourcis_OnKeyStateChange(playerid, newkeys, oldkeys)
{
	// Appui : on arme le maintien, seulement a pied (en vehicule, cette
	// touche sert deja a regarder derriere pour surveiller la route).
	if((newkeys & KEY_LOOK_BEHIND) && !(oldkeys & KEY_LOOK_BEHIND))
	{
		if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return 0;
		raccourcisTenu[playerid] = true;
		SetTimerEx("Raccourcis_KeyCheck", RACCOURCIS_MAINTIEN_MS, false, "i", playerid);
	}
	// Relache avant le delai : simple appui, rien a faire de plus.
	else if(!(newkeys & KEY_LOOK_BEHIND) && (oldkeys & KEY_LOOK_BEHIND))
	{
		raccourcisTenu[playerid] = false;
	}
	return 0;
}

forward Raccourcis_KeyCheck(playerid);
public Raccourcis_KeyCheck(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(!raccourcisTenu[playerid]) return 0;   // relache avant le delai
	raccourcisTenu[playerid] = false;
	if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return 0;   // monte en vehicule entre-temps
	return Raccourcis_Ouvrir(playerid);
}

stock Raccourcis_OnDisconnect(playerid)
{
	raccourcisTenu[playerid] = false;
	return 1;
}
