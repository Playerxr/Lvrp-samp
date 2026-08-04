/*
	afrp_menurapide.inc - Petit panneau de raccourcis (Inventory/Phone/Anims/Toys)

	Ne pas confondre avec afrp_raccourcis.inc (/raccourcis, maintien de
	KEY_LOOK_BEHIND, menu en DIALOG_STYLE_LIST) : c'est un module different,
	demande separement par la proprietaire a partir d'un export
	panelHtrap7mar.txt (4 boutons + croix de fermeture, en PlayerText: donc
	un jeu de textdraws par joueur, comme afrp_sacvisuel.inc/afrp_tablecraft.inc).

	=== POURQUOI NE RIEN REIMPLEMENTER ===
	Meme principe que /raccourcis : "Inventory" et "Phone" rappellent les
	vraies commandes /sac et /tel existantes (CallLocalFunction), et "Anims"
	appelle directement Emotes_ShowCategories() (le systeme de ~70 emotes
	deja construit dans afrp_emotes.inc). Le fichier vactions.pwn envoye par
	la proprietaire pour "le systeme d'anim" recoupe presque entierement ce
	qui existe deja dans afrp_emotes.inc (memes idees : manger, fumer, danser,
	saluer...) - on ne le duplique pas ici, voir le message renvoye a la
	proprietaire pour la question de fond (fusionner les quelques commandes
	en plus, ou laisser tel quel).

	=== "TOYS" ===
	Reste indefini au moment d'ecrire ce module (aucun systeme de props/jouets
	n'existe dans le codebase, et vactions.pwn n'en contient pas non plus,
	seulement des animations). Le bouton est present et cliquable comme prevu
	dans l'export, mais affiche un message d'attente plutot que de forcer une
	fonctionnalite inventee - a cabler des que la proprietaire precise ce que
	"Toys" doit faire.

	=== TOUCHE Y (KEY_YES) ===
	La proprietaire a choisi H ou Y. Y (KEY_YES) est deja utilise ailleurs
	dans OnPlayerKeyStateChange, mais seulement quand job_Start[playerid]==1
	(ramassage colis pendant un job) ou police_Duty[playerid]==1 (demande de
	renfort police) - en dehors de ces deux cas, Y ne fait rien du tout
	actuellement. Le menu ne s'ouvre donc que quand Y est "libre", pour ne
	rien casser des jobs ou de la police.

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect              -> MenuRapide_OnConnect(playerid)
	- OnPlayerDisconnect            -> MenuRapide_OnDisconnect(playerid)
	- OnPlayerClickTextDraw         -> if(MenuRapide_OnClick(playerid, clickedid)) return 1;
	- OnPlayerClickPlayerTextDraw   -> if(MenuRapide_OnClickPlayerText(playerid, playertextid)) return 1;
	- OnPlayerKeyStateChange        -> quand KEY_YES est libre (voir plus haut), MenuRapide_Toggle(playerid)
*/

#if defined AFRP_MENURAPIDE_INCLUDED
	#endinput
#endif
#define AFRP_MENURAPIDE_INCLUDED

new PlayerText:mrTD[MAX_PLAYERS][10];
new bool:mrOuvert[MAX_PLAYERS];

stock MenuRapide_OnConnect(playerid)
{
	mrOuvert[playerid] = false;

	mrTD[playerid][0] = CreatePlayerTextDraw(playerid, 160.000, 186.000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, mrTD[playerid][0], 76.000, 30.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][0], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][0], -1306385665);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][0], 255);
	PlayerTextDrawFont(playerid, mrTD[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][0], 1);
	PlayerTextDrawSetSelectable(playerid, mrTD[playerid][0], 1);

	mrTD[playerid][1] = CreatePlayerTextDraw(playerid, 160.000, 220.000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, mrTD[playerid][1], 76.000, 30.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][1], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][1], -1306385665);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][1], 255);
	PlayerTextDrawFont(playerid, mrTD[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][1], 1);
	PlayerTextDrawSetSelectable(playerid, mrTD[playerid][1], 1);

	mrTD[playerid][2] = CreatePlayerTextDraw(playerid, 161.000, 255.000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, mrTD[playerid][2], 76.000, 30.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][2], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][2], -1306385665);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][2], 255);
	PlayerTextDrawFont(playerid, mrTD[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][2], 1);
	PlayerTextDrawSetSelectable(playerid, mrTD[playerid][2], 1);

	mrTD[playerid][3] = CreatePlayerTextDraw(playerid, 161.000, 289.000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, mrTD[playerid][3], 76.000, 30.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][3], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][3], -1306385665);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][3], 255);
	PlayerTextDrawFont(playerid, mrTD[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][3], 1);
	PlayerTextDrawSetSelectable(playerid, mrTD[playerid][3], 1);

	mrTD[playerid][4] = CreatePlayerTextDraw(playerid, 140.000, 173.000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, mrTD[playerid][4], 16.000, 13.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][4], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][4], -1306385665);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][4], 0);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][4], 255);
	PlayerTextDrawFont(playerid, mrTD[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][4], 1);
	PlayerTextDrawSetSelectable(playerid, mrTD[playerid][4], 1);

	mrTD[playerid][5] = CreatePlayerTextDraw(playerid, 175.000, 193.000, "Inventory");
	PlayerTextDrawLetterSize(playerid, mrTD[playerid][5], 0.300, 1.500);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][5], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][5], 1);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][5], 1);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][5], 0);
	PlayerTextDrawFont(playerid, mrTD[playerid][5], 1);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][5], 1);

	mrTD[playerid][6] = CreatePlayerTextDraw(playerid, 180.000, 226.000, "Phone");
	PlayerTextDrawLetterSize(playerid, mrTD[playerid][6], 0.300, 1.500);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][6], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][6], 1);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][6], 1);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][6], 0);
	PlayerTextDrawFont(playerid, mrTD[playerid][6], 1);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][6], 1);

	mrTD[playerid][7] = CreatePlayerTextDraw(playerid, 180.000, 262.000, "Anims");
	PlayerTextDrawLetterSize(playerid, mrTD[playerid][7], 0.300, 1.500);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][7], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][7], 1);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][7], 1);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][7], 0);
	PlayerTextDrawFont(playerid, mrTD[playerid][7], 1);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][7], 1);

	mrTD[playerid][8] = CreatePlayerTextDraw(playerid, 182.000, 294.000, "Toys");
	PlayerTextDrawLetterSize(playerid, mrTD[playerid][8], 0.300, 1.500);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][8], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][8], 1);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][8], 1);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][8], 0);
	PlayerTextDrawFont(playerid, mrTD[playerid][8], 1);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][8], 1);

	mrTD[playerid][9] = CreatePlayerTextDraw(playerid, 144.000, 172.000, "X");
	PlayerTextDrawLetterSize(playerid, mrTD[playerid][9], 0.300, 1.500);
	PlayerTextDrawTextSize(playerid, mrTD[playerid][9], 5.000, -9.000);
	PlayerTextDrawAlignment(playerid, mrTD[playerid][9], 1);
	PlayerTextDrawColor(playerid, mrTD[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, mrTD[playerid][9], 1);
	PlayerTextDrawSetOutline(playerid, mrTD[playerid][9], 1);
	PlayerTextDrawBackgroundColor(playerid, mrTD[playerid][9], 0);
	PlayerTextDrawFont(playerid, mrTD[playerid][9], 1);
	PlayerTextDrawSetProportional(playerid, mrTD[playerid][9], 1);
	return 1;
}

stock MenuRapide_OnDisconnect(playerid)
{
	mrOuvert[playerid] = false;
	for(new k = 0; k < 10; k++)
	{
		if(mrTD[playerid][k] != PlayerText:INVALID_TEXT_DRAW)
			{PlayerTextDrawDestroy(playerid, mrTD[playerid][k]);}
	}
	return 1;
}

stock MenuRapide_Ouvrir(playerid)
{
	if(mrOuvert[playerid]) return 1;
	mrOuvert[playerid] = true;
	for(new k = 0; k < 10; k++) PlayerTextDrawShow(playerid, mrTD[playerid][k]);
	SelectTextDraw(playerid, 0xCCCCCCFF);
	return 1;
}

// [FIX INVENTAIRE QUI DISPARAIT SEUL] CancelSelectTextDraw() ne se contente
// pas de sortir du mode selection : il declenche lui-meme
// OnPlayerClickTextDraw avec INVALID_TEXT_DRAW. Or c'est exactement le signal
// que le sac (SacV_OnClickHorsZone) et le telephone interpretent comme "le
// joueur a appuye sur Echap, on ferme".
// Enchainement observe en jeu : clic sur "Inventory" -> on fermait le menu
// avec CancelSelectTextDraw -> /sac ouvrait la grille -> le INVALID_TEXT_DRAW
// de NOTRE annulation arrivait juste apres et refermait la grille aussitot.
// D'ou "l'inventaire disparait tout seul".
// Les ecrans qui reprennent la main (sac, telephone) appellent deja
// SelectTextDraw eux-memes : on leur passe la main sans annuler. On n'annule
// que quand rien ne prend le relais (croix, Echap, dialogue, message).
stock MenuRapide_Fermer(playerid, bool:annulerSelection = true)
{
	if(!mrOuvert[playerid]) return 1;
	mrOuvert[playerid] = false;
	for(new k = 0; k < 10; k++) PlayerTextDrawHide(playerid, mrTD[playerid][k]);
	if(annulerSelection) CancelSelectTextDraw(playerid);
	return 1;
}

stock MenuRapide_Toggle(playerid)
{
	if(mrOuvert[playerid]) return MenuRapide_Fermer(playerid);
	return MenuRapide_Ouvrir(playerid);
}

// SA-MP separe les clics sur textdraws globaux (OnPlayerClickTextDraw, ici
// seulement pour intercepter l'Echap/INVALID_TEXT_DRAW) des clics sur
// PlayerText: (OnPlayerClickPlayerTextDraw, ci-dessous pour les vrais
// boutons) - meme decoupage que SacV_OnClickHorsZone / SacV_OnClick.

// true = evenement consomme, le dispatcheur OnPlayerClickTextDraw doit s'arreter la
stock MenuRapide_OnClick(playerid, Text:clickedid)
{
	if(!mrOuvert[playerid]) return 0;
	if(clickedid != Text:INVALID_TEXT_DRAW) return 0;
	MenuRapide_Fermer(playerid);
	return 1;
}

// true = evenement consomme, le dispatcheur OnPlayerClickPlayerTextDraw doit s'arreter la
stock MenuRapide_OnClickPlayerText(playerid, PlayerText:playertextid)
{
	if(!mrOuvert[playerid]) return 0;

	if(playertextid == mrTD[playerid][4]) // croix de fermeture
	{
		MenuRapide_Fermer(playerid);
		return 1;
	}
	// false = on n'annule PAS la selection : le sac appelle SelectTextDraw
	// juste apres, et annuler ici refermerait la grille aussitot ouverte.
	if(playertextid == mrTD[playerid][0]) // Inventory -> vraie commande /sac
	{
		MenuRapide_Fermer(playerid, false);
		CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sac");
		return 1;
	}
	// Idem : le telephone reprend la selection lui-meme.
	if(playertextid == mrTD[playerid][1]) // Phone -> vraie commande /tel
	{
		MenuRapide_Fermer(playerid, false);
		CallLocalFunction("OnPlayerCommandText", "is", playerid, "/tel");
		return 1;
	}
	if(playertextid == mrTD[playerid][2]) // Anims -> vrai systeme d'emotes existant
	{
		MenuRapide_Fermer(playerid);
		Emotes_ShowCategories(playerid);
		return 1;
	}
	if(playertextid == mrTD[playerid][3]) // Toys -> pas encore defini, voir en-tete
	{
		MenuRapide_Fermer(playerid);
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}Menu {FFFFFF}\"Toys\" bientot disponible.");
		return 1;
	}
	return 0;
}
