/*
	afrp_raccourcis.inc - Menu de raccourcis (simplifie le gameplay)

	Au lieu de retenir /sac, /job, /emotes, /maison et /vehicule separement,
	/raccourcis ouvre un seul dialogue qui les liste. Meme principe que
	afrp_factionmenu.inc (/menu) : on ne reimplemente rien, on rappelle la
	vraie commande via OnPlayerCommandText, donc toutes les verifications
	existantes (sac achete, job en cours, vehicule a proximite...) restent
	telles quelles.

	  /raccourcis (alias /rapide) : ouvre le menu

	Hooks a brancher dans LVRP.pwn :
	- OnDialogResponse -> if(Raccourcis_OnDialog(playerid, dialogid, response, listitem)) return 1;
	- /raccourcis, /rapide -> Raccourcis_Ouvrir(playerid)
*/

#if defined AFRP_RACCOURCIS_INCLUDED
	#endinput
#endif
#define AFRP_RACCOURCIS_INCLUDED

#define DIALOG_RACCOURCIS  9654

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
