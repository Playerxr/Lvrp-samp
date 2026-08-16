/*
	afrp_raccourcis.inc - Menu de raccourcis (simplifie le gameplay)

	Au lieu de retenir toutes les commandes separement, /raccourcis ouvre un
	seul dialogue paginable qui liste les commandes les plus utilisees par
	les joueurs. Meme principe que afrp_factionmenu.inc (/menu) : on ne
	reimplemente rien, on rappelle la vraie commande via OnPlayerCommandText,
	donc toutes les verifications existantes (sac achete, job en cours,
	vehicule a proximite...) restent telles quelles.

	  /raccourcis (alias /rapide) : ouvre le menu
	  maintenir KEY_LOOK_BEHIND (regarder en arriere, touche C par defaut)
	  a pied ouvre aussi le menu.
	  Egalement accessible depuis le panneau touche 'Y' (voir
	  afrp_menurapide.inc, bouton "Raccourcis" ex-"Toys").

	=== PAGINATION ===
	[SIMPLIFICATION] La demande initiale (5 commandes) a grandi avec le
	temps ; DIALOG_STYLE_LIST scrolle deja tout seul en SA-MP, mais un
	systeme "Page suivante" explicite reste plus lisible sur mobile/petit
	ecran et correspond a ce qui a ete demande. RACCOURCIS_PER_PAGE
	commandes par page, derniere ligne = navigation ("Page suivante >>" ou
	"<< Retour au debut" sur la derniere page). Le bouton droit du dialogue
	("Quitter") ferme sans rien faire, comme n'importe quel Echap.

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
#define RACCOURCIS_PER_PAGE  7

new bool:raccourcisTenu[MAX_PLAYERS];
new raccourcisPage[MAX_PLAYERS];

// [SIMPLIFICATION] Liste des commandes les plus utilisees par les joueurs,
// verifiees une a une dans LVRP.pwn (aucune commande inventee ici). Ajouter
// une commande = ajouter une ligne aux DEUX tableaux, meme index.
stock const raccourcisLabel[][32] = {
	"Sac (inventaire)",
	"Job (metier)",
	"Emotes (animations)",
	"Maison",
	"Vehicule",
	"Telephone (mobile)",
	"Annuaire (contacts)",
	"Police (911)",
	"Medecin (sos)",
	"Pompiers (feu)",
	"Signaler un joueur",
	"Reglement",
	"Menu Faction",
	"VIP",
	"Statistiques",
	"Guide complet"
};
stock const raccourcisCmd[][16] = {
	"/sac",
	"/travail",
	"/emotes",
	"/maison",
	"/v",
	"/mobile",
	"/annuaire",
	"/911",
	"/sos",
	"/feu",
	"/report",
	"/regles",
	"/menu",
	"/vip",
	"/stats",
	"/aide"
};
#define RACCOURCIS_TOTAL 16

stock Raccourcis_Ouvrir(playerid, page = 0)
{
	new debut = page * RACCOURCIS_PER_PAGE;
	if(debut >= RACCOURCIS_TOTAL) { page = 0; debut = 0; } // page invalide -> retour au debut

	new fin = debut + RACCOURCIS_PER_PAGE;
	if(fin > RACCOURCIS_TOTAL) fin = RACCOURCIS_TOTAL;

	raccourcisPage[playerid] = page;

	new body[600];
	body[0] = 0;
	for(new i = debut; i < fin; i++)
	{
		strcat(body, "{FFFFFF}");
		strcat(body, raccourcisLabel[i]);
		strcat(body, "\n");
	}

	// Ligne de navigation : uniquement s'il reste d'autres commandes a voir.
	if(fin < RACCOURCIS_TOTAL)
		{strcat(body, "{FFD700}Page suivante >>");}
	else if(page > 0)
		{strcat(body, "{8C8C8C}<< Retour au debut");}

	new titre[48];
	format(titre, sizeof(titre), "{FFD700}Raccourcis (page %d)", page + 1);
	return ShowPlayerDialog(playerid, DIALOG_RACCOURCIS, DIALOG_STYLE_LIST, titre, body, "Choisir", "Quitter");
}

stock Raccourcis_OnDialog(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_RACCOURCIS) return 0;
	if(!response) return 1; // "Quitter" ou Echap

	new page = raccourcisPage[playerid];
	new debut = page * RACCOURCIS_PER_PAGE;
	new fin = debut + RACCOURCIS_PER_PAGE;
	if(fin > RACCOURCIS_TOTAL) fin = RACCOURCIS_TOTAL;
	new nbAffiches = fin - debut;

	// La derniere ligne affichee est la navigation, pas une commande.
	if(listitem == nbAffiches)
	{
		if(fin < RACCOURCIS_TOTAL)
			{return Raccourcis_Ouvrir(playerid, page + 1);}
		return Raccourcis_Ouvrir(playerid, 0);
	}

	new idx = debut + listitem;
	if(idx < 0 || idx >= RACCOURCIS_TOTAL) return 1; // securite, ne devrait jamais arriver
	CallLocalFunction("OnPlayerCommandText", "is", playerid, raccourcisCmd[idx]);
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
	raccourcisPage[playerid] = 0;
	return 1;
}
