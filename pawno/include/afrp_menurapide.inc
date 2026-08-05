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

	=== "Phone" : TALKIE OU TELEPHONE ? ===
	Demande de la proprietaire : si le joueur possede un Talkie Walkie (objet
	2966), "Phone" propose d'abord un choix au lieu d'ouvrir /tel directement.
	"Talkie-walkie" ouvre la RADIO VOCALE (canaux VOIP - /radio de faction ou
	/frequency manuelle, deja cables dans afrp_radiovoice.inc), pas l'ancien
	systeme texte (/freq + /ta) : la proprietaire a choisi explicitement la
	radio vocale, celle qui vient d'etre reparee. Comme pour Inventory/Anims,
	on rappelle les vraies commandes (CallLocalFunction) au lieu de dupliquer
	leurs verifications (item possede, plugin/micro, faction legale...).

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect              -> MenuRapide_OnConnect(playerid)
	- OnPlayerDisconnect            -> MenuRapide_OnDisconnect(playerid)
	- OnPlayerClickTextDraw         -> if(MenuRapide_OnClick(playerid, clickedid)) return 1;
	- OnPlayerClickPlayerTextDraw   -> if(MenuRapide_OnClickPlayerText(playerid, playertextid)) return 1;
	- OnPlayerKeyStateChange        -> quand KEY_YES est libre (voir plus haut), MenuRapide_Toggle(playerid)
	- OnDialogResponse              -> if(MenuRapide_OnDialog(playerid, dialogid, response, listitem, inputtext)) return 1;
*/

#if defined AFRP_MENURAPIDE_INCLUDED
	#endinput
#endif
#define AFRP_MENURAPIDE_INCLUDED

// Choix Talkie-walkie / Telephone (DIALOG_STYLE_LIST, 2 lignes).
#define DIALOG_MR_TALKIE_CHOIX   9670
// Sous-menu radio vocale : rejoindre faction / frequence manuelle / quitter.
#define DIALOG_MR_TALKIE_RADIO   9671
// Saisie du numero de frequence a rejoindre (relaie vers /frequency <id>).
#define DIALOG_MR_TALKIE_FREQ    9672

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
	if(playertextid == mrTD[playerid][1]) // Phone -> /tel, ou choix talkie si l'objet est possede
	{
		// Objet 2966 = Talkie Walkie (meme verification que /frequency et /ta).
		if(inventory_GetItemQuantity(playerid, 2966) > 0)
		{
			MenuRapide_Fermer(playerid); // on ouvre un dialogue, pas un ecran a selection
			ShowPlayerDialog(playerid, DIALOG_MR_TALKIE_CHOIX, DIALOG_STYLE_LIST, "{FFD700}Communication",
				"Talkie-walkie (radio vocale)\nT\xe9l\xe9phone", "Choisir", "Annuler");
			return 1;
		}
		// Sans talkie : comportement inchange, le telephone reprend la selection lui-meme.
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

// true = evenement consomme, le dispatcheur OnDialogResponse doit s'arreter la.
// Gere le sous-menu talkie ouvert depuis "Phone" (voir MenuRapide_OnClickPlayerText).
stock MenuRapide_OnDialog(playerid, dialogid, response, listitem, inputtext[])
{
	if(dialogid == DIALOG_MR_TALKIE_CHOIX)
	{
		if(!response) return 1; // "Annuler" ou Echap : on ne fait rien

		if(listitem == 0) // Talkie-walkie -> sous-menu radio vocale
		{
			new mrRadioBody[160];
			// [SOURCE DE VERITE UNIQUE] radioVoiceAttachedTo/isUsingRadioVoip
			// (afrp_radiovoice.inc) refletent deja l'etat reel : on l'affiche au
			// lieu d'un menu toujours identique, pour eviter un "Rejoindre" qui
			// semble ne rien faire quand on est deja connecte.
			if(isUsingRadioVoip[playerid])
				format(mrRadioBody, sizeof(mrRadioBody), "Rejoindre la radio de faction\nRejoindre une fr\xe9quence...\nQuitter le canal actuel ({FF8C00}connect\xe9{FFFFFF})");
			else
				format(mrRadioBody, sizeof(mrRadioBody), "Rejoindre la radio de faction\nRejoindre une fr\xe9quence...\nQuitter le canal actuel");
			ShowPlayerDialog(playerid, DIALOG_MR_TALKIE_RADIO, DIALOG_STYLE_LIST, "{FFD700}Radio vocale", mrRadioBody, "Choisir", "Retour");
			return 1;
		}
		// listitem == 1 : Telephone -> vraie commande /tel (comme sans talkie)
		CallLocalFunction("OnPlayerCommandText", "is", playerid, "/tel");
		return 1;
	}

	if(dialogid == DIALOG_MR_TALKIE_RADIO)
	{
		if(!response) return 1; // "Retour" ou Echap

		switch(listitem)
		{
			case 0: // Radio de faction -> vraie commande /radio (verifs deja dedans)
				CallLocalFunction("OnPlayerCommandText", "is", playerid, "/radio");
			case 1: // Frequence manuelle -> demande le numero, relaie vers /frequency <id>
				ShowPlayerDialog(playerid, DIALOG_MR_TALKIE_FREQ, DIALOG_STYLE_INPUT, "{FFD700}Fr\xe9quence radio",
					"Entrez le num\xe9ro de fr\xe9quence \xe0 rejoindre (1 \xe0 999999) :", "Rejoindre", "Annuler");
			case 2: // Quitter -> vraie commande /frequency 0 (quitte radio ET frequence manuelle)
				CallLocalFunction("OnPlayerCommandText", "is", playerid, "/frequency 0");
		}
		return 1;
	}

	if(dialogid == DIALOG_MR_TALKIE_FREQ)
	{
		if(!response) return 1; // "Annuler"

		new mrFreq = strval(inputtext);
		if(mrFreq < 1 || mrFreq > 999999)
		{
			msg_Client(playerid, COLOR_INFO, "{CF9756}\xbb Info \xab{FFFFFF} Fr\xe9quence invalide (1 \xe0 999999).");
			return 1;
		}
		new mrCmd[24];
		format(mrCmd, sizeof(mrCmd), "/frequency %d", mrFreq);
		CallLocalFunction("OnPlayerCommandText", "is", playerid, mrCmd);
		return 1;
	}

	return 0;
}
