/*
	afrp_sacvisuel.inc - Le sac en images plutot qu'en liste de texte

	Le sac du serveur fonctionne : 55 emplacements, sauvegarde MySQL, sac
	posable a terre, armes rangees dedans. Ce qui manquait, c'est de le VOIR.
	/sac voir affichait une liste de texte dans un dialogue, ou un joueur qui
	transporte vingt objets ne retrouve rien.

	Ce module dessine une grille de 20 cases avec l'objet en 3D dans chacune,
	son nom et sa quantite, et un panneau d'actions a droite.

	  /sac              ouvre la grille (avant, cette commande sans argument
	                    ne faisait qu'afficher un message d'aide)
	  /sac voir         garde la liste de texte, pour qui la prefere
	  /sac arme / poser / prendre / afficher : inchanges
	  /sac ramasser     nouveau : reprend un objet jete au sol (bouton Jeter)

	Boutons du panneau : Details, Utiliser, Donner, Jeter (fait tomber l'objet
	au sol, ramassable par n'importe qui via /sac ramasser), Deplacer (echange
	deux cases), Sac (pose/reprend le sac entier, rappelle /sac poser|prendre),
	Page, Fermer.

	=== IL NE TOUCHE PAS AUX DONNEES ===
	La grille lit PlayerInfo[playerid][pInvModel] et [pInvQuant], les memes
	tableaux que le reste du gamemode, et passe par inventory_UpdateItem pour
	toute modification. Aucune sauvegarde n'est reecrite, aucun format de
	stockage n'est change : si la grille deplaisait, la supprimer ne ferait
	perdre aucun objet a personne.

	=== LE BUDGET DE TEXTDRAWS ===
	Un client SA-MP n'accepte que 256 textdraws par joueur, et ce gamemode en
	tient deja pres de 200 en permanence : 169 pour le telephone, plus le HUD,
	le panneau du top du jour et les objectifs. Une grille bavarde ne se serait
	pas affichee du tout, sans message d'erreur.
	Deux mesures : la grille se limite a 51 elements (un apercu et un libelle
	par case, au lieu de quatre elements comme dans le pack fourni), et ils sont
	crees a l'ouverture puis detruits a la fermeture.

	=== LES ARMES NE SONT PAS DANS LA GRILLE ===
	Elles sont rangees a part dans le gamemode (pInvWeapon / pInvAmmo, avec un
	nombre de slots qui depend du sac) et restent sur /sac arme. Les melanger
	aurait demande de toucher a la logique des armes, ce que ce module evite.

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect              -> SacV_OnConnect(playerid)
	- OnPlayerDisconnect           -> SacV_OnDisconnect(playerid)
	- OnPlayerClickPlayerTextDraw  -> if(SacV_OnClick(playerid, playertextid)) return 1;
	- OnPlayerClickTextDraw        -> if(SacV_OnClickHorsZone(playerid, clickedid)) return 1;
	- OnDialogResponse             -> if(SacV_OnDialog(playerid, dialogid, response, inputtext)) return 1;
	- /sac sans argument           -> SacV_Ouvrir(playerid)
*/

#if defined AFRP_SACVISUEL_INCLUDED
	#endinput
#endif
#define AFRP_SACVISUEL_INCLUDED

#define SV_CASES          20        // 5 colonnes x 4 lignes
#define SV_COLS           5
#define SV_LIGNES         4
#define SV_BOUTONS        8
#define DIALOG_SV_JETER   9639
#define DIALOG_SV_DONNER  9640

// Objets jetes et retrouves au sol : 55055-55152 dans LVRP.pwn gere deja un
// objet pose/repris pour le SAC ENTIER (/sac poser, /sac prendre). Ceci est
// different : un OBJET a l'interieur du sac, depose au sol individuellement,
// que n'importe qui peut ramasser avec /sac ramasser.
#define SV_DROP_MAX         40       // objets simultanement au sol sur tout le serveur
#define SV_DROP_DIST        2.5

// Coordonnees reprises du pack de textdraws fourni.
#define SV_PANEL_X        118.0
#define SV_PANEL_Y        96.0
#define SV_CASE_W         39.0
#define SV_CASE_H         51.0
#define SV_COL_PAS        40.0
#define SV_LIGNE_PAS      56.0
#define SV_CASE_X0        125.0
#define SV_CASE_Y0        120.0

#define SV_COUL_CASE      859394047
#define SV_COUL_CASE_SEL  0x00C8FFAA   // case selectionnee
#define SV_DIST_DONNER    3.0

// Boutons du panneau de droite.
#define SV_BTN_DETAILS    0
#define SV_BTN_UTILISER   1
#define SV_BTN_DONNER     2
#define SV_BTN_JETER      3
#define SV_BTN_DEPLACER   4
#define SV_BTN_SAC        5
#define SV_BTN_PAGE       6
#define SV_BTN_FERMER     7

new PlayerText:svFondP[MAX_PLAYERS][5];
new PlayerText:svModele[MAX_PLAYERS][SV_CASES];
new PlayerText:svNom[MAX_PLAYERS][SV_CASES];
new PlayerText:svBtn[MAX_PLAYERS][SV_BOUTONS];
new PlayerText:svPanneau[MAX_PLAYERS];

new svOuvert[MAX_PLAYERS];          // 1 = grille affichee
new svPage[MAX_PLAYERS];            // page courante
new svSel[MAX_PLAYERS];             // case selectionnee (0..19), -1 si aucune
new svSlotDe[MAX_PLAYERS][SV_CASES];// case -> emplacement reel dans le sac
new svDeplaceSrc[MAX_PLAYERS];      // case en attente d'echange, -1 si aucune

// --- objets deposes au sol (tableaux globaux, pas par joueur) ---
new svDropUtilise[SV_DROP_MAX];             // 1 = case occupee
new svDropModel[SV_DROP_MAX];
new svDropQuant[SV_DROP_MAX];
new Float:svDropX[SV_DROP_MAX];
new Float:svDropY[SV_DROP_MAX];
new Float:svDropZ[SV_DROP_MAX];
new svDropVW[SV_DROP_MAX];
new svDropInt[SV_DROP_MAX];
new svDropObjet[SV_DROP_MAX];
new Text3D:svDropLabel[SV_DROP_MAX];

// Objets consommables directement depuis le sac (boissons et aliments vendus
// a l'epicerie). Verifie contre /utiliser aliment, la seule commande du
// gamemode qui consomme deja ces modeles precis.
stock SacV_EstAlimentBoisson(modelid)
{
	switch(modelid)
	{
		case 1668, 2601, 19569, 19563, 1486:  return 1;  // eau, soda, lait, jus, biere
		case 19811, 19571, 2769, 2663, 2821:  return 1;  // burger, pizza, sandwich, chips, cereales
	}
	return 0;
}

// Les objets ranges dans le sac sont de vrais modeles GTA : ils s'affichent
// donc directement en 3D. Les armes, elles, sont stockees par leur numero
// d'arme (1 a 46), qui n'est pas un modele. Cette table donne le modele de
// l'objet correspondant, releve dans la section weap de default.ide.
new svModeleArme[47] = {
	0,
	331, 333, 334, 335, 336, 337, 338, 339, 341,   //  1-9  armes blanches
	321, 322, 323, 324, 325, 326,                  // 10-15 objets
	342, 343, 344,                                 // 16-18 grenades
	0, 0, 0,                                       // 19-21 inutilises
	346, 347, 348, 349, 350, 351, 352, 353,        // 22-29
	355, 356, 372, 357, 358,                       // 30-34
	359, 360, 361, 362, 363, 364,                  // 35-40
	365, 366, 367, 368, 369, 371                   // 41-46
};

// ============================================================================
// OUTILS
// ============================================================================
stock SacV_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// Modele a afficher en 3D pour un objet du sac.
stock SacV_ModeleAffiche(modelid)
{
	if(modelid >= 1 && modelid <= 46) return svModeleArme[modelid];
	if(modelid >= 300 && modelid <= 19999) return modelid;
	return 0;
}

// inventory_UpdateItem ne signale pas l'echec : si le sac du destinataire est
// plein, l'ajout ne se fait pas, alors que le retrait chez le donneur a deja
// eu lieu. L'objet disparaitrait. On verifie donc la place AVANT de transferer.
stock SacV_APlace(cible, modelid)
{
	for(new i = 0; i < gPlayerInventory[cible]; i++)
	{
		if(PlayerInfo[cible][pInvModel][i] == modelid) return 1;
		if(PlayerInfo[cible][pInvModel][i] == -1) return 1;
	}
	return 0;
}

// Nombre d'objets reellement presents dans le sac.
stock SacV_NbObjets(playerid)
{
	new n = 0;
	for(new i = 0; i < gPlayerInventory[playerid]; i++)
	{
		if(PlayerInfo[playerid][pInvModel][i] == -1) continue;
		if(PlayerInfo[playerid][pInvQuant][i] <= 0) continue;
		n++;
	}
	return n;
}

stock SacV_NbPages(playerid)
{
	new n = SacV_NbObjets(playerid);
	if(n <= 0) return 1;
	return ((n - 1) / SV_CASES) + 1;
}

// ============================================================================
// CREATION DES TEXTDRAWS
// ============================================================================
stock SacV_Creer(playerid)
{
	new Float:x, Float:y;

	// --- fond et entete ---
	svFondP[playerid][0] = CreatePlayerTextDraw(playerid, SV_PANEL_X, SV_PANEL_Y, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svFondP[playerid][0], 213.0, 253.0);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][0], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][0], 690964479);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][0], 255);
	PlayerTextDrawFont(playerid, svFondP[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][0], 1);

	svFondP[playerid][1] = CreatePlayerTextDraw(playerid, 125.0, 115.0, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svFondP[playerid][1], 199.0, 3.0);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][1], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][1], 255);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][1], 255);
	PlayerTextDrawFont(playerid, svFondP[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][1], 1);

	svFondP[playerid][2] = CreatePlayerTextDraw(playerid, 126.0, 115.0, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svFondP[playerid][2], 165.0, 3.0);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][2], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][2], 12582911);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][2], 255);
	PlayerTextDrawFont(playerid, svFondP[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][2], 1);

	svFondP[playerid][3] = CreatePlayerTextDraw(playerid, 126.0, 105.0, "Sac");
	PlayerTextDrawLetterSize(playerid, svFondP[playerid][3], 0.14, 0.898);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][3], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][3], 150);
	PlayerTextDrawFont(playerid, svFondP[playerid][3], 1);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][3], 1);

	svFondP[playerid][4] = CreatePlayerTextDraw(playerid, 324.0, 105.0, "0/0");
	PlayerTextDrawLetterSize(playerid, svFondP[playerid][4], 0.14, 0.699);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][4], 3);
	PlayerTextDrawColor(playerid, svFondP[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][4], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][4], 150);
	PlayerTextDrawFont(playerid, svFondP[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][4], 1);

	// --- les 20 cases ---
	// Un client SA-MP n'accepte que 256 textdraws par joueur, et ce gamemode en
	// tient deja pres de 200 en permanence (telephone, HUD, panneaux). La grille
	// se limite donc a deux elements par case, apercu et libelle, au lieu des
	// quatre du pack d'origine : au-dela, la grille ne s'affichait tout
	// simplement pas pour un joueur qui a un telephone.
	for(new k = 0; k < SV_CASES; k++)
	{
		x = SV_CASE_X0 + (float(k % SV_COLS) * SV_COL_PAS);
		y = SV_CASE_Y0 + (float(k / SV_COLS) * SV_LIGNE_PAS);

		// L'apercu 3D est l'element cliquable : il couvre le centre de la case.
		svModele[playerid][k] = CreatePlayerTextDraw(playerid, x + 4.0, y + 9.0, "_");
		PlayerTextDrawTextSize(playerid, svModele[playerid][k], 30.0, 35.0);
		PlayerTextDrawAlignment(playerid, svModele[playerid][k], 1);
		PlayerTextDrawColor(playerid, svModele[playerid][k], -1);
		PlayerTextDrawSetShadow(playerid, svModele[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svModele[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svModele[playerid][k], 0);
		PlayerTextDrawFont(playerid, svModele[playerid][k], 5);
		PlayerTextDrawSetProportional(playerid, svModele[playerid][k], 0);
		PlayerTextDrawSetPreviewModel(playerid, svModele[playerid][k], 1212);
		PlayerTextDrawSetPreviewRot(playerid, svModele[playerid][k], 0.0, 0.0, 0.0, 1.0);
		PlayerTextDrawSetSelectable(playerid, svModele[playerid][k], 1);

		// Nom et quantite sur la meme ligne, avec un fond de case dessine par la
		// boite de ce textdraw : la selection change sa couleur.
		svNom[playerid][k] = CreatePlayerTextDraw(playerid, x + 2.0, y + 1.0, "_");
		PlayerTextDrawLetterSize(playerid, svNom[playerid][k], 0.119, 0.598);
		PlayerTextDrawAlignment(playerid, svNom[playerid][k], 1);
		PlayerTextDrawColor(playerid, svNom[playerid][k], -1);
		PlayerTextDrawSetShadow(playerid, svNom[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svNom[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svNom[playerid][k], 0);
		PlayerTextDrawFont(playerid, svNom[playerid][k], 1);
		PlayerTextDrawSetProportional(playerid, svNom[playerid][k], 1);
		PlayerTextDrawUseBox(playerid, svNom[playerid][k], 1);
		PlayerTextDrawBoxColor(playerid, svNom[playerid][k], SV_COUL_CASE);
		PlayerTextDrawTextSize(playerid, svNom[playerid][k], x + SV_CASE_W, y + SV_CASE_H);
	}

	// --- panneau d'actions ---
	// Hauteur alignee sur le bas de la grille de cases (y=339, cf SV_CASE_Y0 +
	// 3*SV_LIGNE_PAS + SV_CASE_H) : la colonne de droite peut ainsi accueillir
	// 8 boutons sans deborder sur une zone d'ecran deja utilisee ailleurs.
	svPanneau[playerid] = CreatePlayerTextDraw(playerid, 347.0, 168.0, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svPanneau[playerid], 402.0, 339.0);
	PlayerTextDrawAlignment(playerid, svPanneau[playerid], 1);
	PlayerTextDrawColor(playerid, svPanneau[playerid], 690964479);
	PlayerTextDrawSetShadow(playerid, svPanneau[playerid], 0);
	PlayerTextDrawSetOutline(playerid, svPanneau[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, svPanneau[playerid], 255);
	PlayerTextDrawFont(playerid, svPanneau[playerid], 4);
	PlayerTextDrawSetProportional(playerid, svPanneau[playerid], 1);

	new libelles[SV_BOUTONS][12] = { "Details", "Utiliser", "Donner", "Jeter", "Deplacer", "Sac", "Page", "Fermer" };
	for(new b = 0; b < SV_BOUTONS; b++)
	{
		y = 174.0 + (float(b) * 21.0);

		// Texte et fond dans le meme textdraw (font 1 + UseBox) : deux elements
		// par bouton auraient coute cinq slots de plus pour rien.
		svBtn[playerid][b] = CreatePlayerTextDraw(playerid, 356.0, y + 4.0, libelles[b]);
		PlayerTextDrawLetterSize(playerid, svBtn[playerid][b], 0.15, 0.898);
		PlayerTextDrawAlignment(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawColor(playerid, svBtn[playerid][b], -1);
		PlayerTextDrawSetShadow(playerid, svBtn[playerid][b], 0);
		PlayerTextDrawSetOutline(playerid, svBtn[playerid][b], 0);
		PlayerTextDrawBackgroundColor(playerid, svBtn[playerid][b], 0);
		PlayerTextDrawFont(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawSetProportional(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawUseBox(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawBoxColor(playerid, svBtn[playerid][b], 859394047);
		PlayerTextDrawTextSize(playerid, svBtn[playerid][b], 397.0, y + 18.0);
		PlayerTextDrawSetSelectable(playerid, svBtn[playerid][b], 1);
	}
	return 1;
}

stock SacV_Detruire(playerid)
{
	for(new k = 0; k < 5; k++) PlayerTextDrawDestroy(playerid, svFondP[playerid][k]);
	for(new k = 0; k < SV_CASES; k++)
	{
		PlayerTextDrawDestroy(playerid, svModele[playerid][k]);
		PlayerTextDrawDestroy(playerid, svNom[playerid][k]);
	}
	for(new b = 0; b < SV_BOUTONS; b++) PlayerTextDrawDestroy(playerid, svBtn[playerid][b]);
	PlayerTextDrawDestroy(playerid, svPanneau[playerid]);
	return 1;
}

// ============================================================================
// REMPLISSAGE
// ============================================================================
stock SacV_Remplir(playerid)
{
	new s[64], nom[32];
	new debut = svPage[playerid] * SV_CASES;
	new vu = 0, place = 0;

	for(new k = 0; k < SV_CASES; k++) svSlotDe[playerid][k] = -1;

	for(new i = 0; i < gPlayerInventory[playerid] && place < SV_CASES; i++)
	{
		if(PlayerInfo[playerid][pInvModel][i] == -1) continue;
		if(PlayerInfo[playerid][pInvQuant][i] <= 0) continue;
		if(vu++ < debut) continue;           // objets des pages precedentes
		svSlotDe[playerid][place++] = i;
	}

	for(new k = 0; k < SV_CASES; k++)
	{
		new slot = svSlotDe[playerid][k];
		if(slot == -1)
		{
			// Case vide : le cadre reste, le contenu part. L'apercu est masque
			// plutot que mis au modele 0, un modele invalide affichant parfois
			// un objet au hasard cote client.
			PlayerTextDrawSetString(playerid, svNom[playerid][k], "_");
			PlayerTextDrawBoxColor(playerid, svNom[playerid][k], SV_COUL_CASE);
			PlayerTextDrawShow(playerid, svNom[playerid][k]);
			PlayerTextDrawHide(playerid, svModele[playerid][k]);
			continue;
		}

		new modelid = PlayerInfo[playerid][pInvModel][slot];
		format(nom, sizeof(nom), "%s", GetItemName(modelid));
		// Un nom trop long depasse de la case : on le coupe.
		if(strlen(nom) > 10) nom[10] = 0;
		format(s, sizeof(s), "%s x%d", nom, PlayerInfo[playerid][pInvQuant][slot]);
		PlayerTextDrawSetString(playerid, svNom[playerid][k], s);
		PlayerTextDrawBoxColor(playerid, svNom[playerid][k],
			(svSel[playerid] == k) ? (SV_COUL_CASE_SEL) : (SV_COUL_CASE));

		PlayerTextDrawSetPreviewModel(playerid, svModele[playerid][k], SacV_ModeleAffiche(modelid));
		PlayerTextDrawShow(playerid, svNom[playerid][k]);
		PlayerTextDrawShow(playerid, svModele[playerid][k]);
	}

	format(s, sizeof(s), "%d/%d objets - page %d/%d", SacV_NbObjets(playerid),
		gPlayerInventory[playerid], svPage[playerid] + 1, SacV_NbPages(playerid));
	PlayerTextDrawSetString(playerid, svFondP[playerid][4], s);
	PlayerTextDrawShow(playerid, svFondP[playerid][4]);
	return 1;
}

// ============================================================================
// OUVRIR / FERMER
// ============================================================================
stock SacV_Ouvrir(playerid)
{
	if(PlayerInfo[playerid][pBag] == 0)
		return SacV_Msg(playerid, "{FF4444}Tu n'as pas de sac. Achetes-en un dans un magasin de vetements.");
	if(IsPlayerInAnyVehicle(playerid))
		return SacV_Msg(playerid, "{FF4444}Impossible dans un vehicule.");
	if(PlayerInfo[playerid][pJailed] != 0)
		return SacV_Msg(playerid, "{FF4444}Ton sac a ete vide a l'entree en prison.");

	// Sac pose a terre : il faut etre a cote, comme pour /sac voir.
	if(bag[playerid][statut] == 1)
	{
		if(!PlayerToPoint(1.5, playerid, bag[playerid][pos][0], bag[playerid][pos][1], bag[playerid][pos][2]))
			return SacV_Msg(playerid, "{FF4444}Tu n'es pas a cote de ton sac.");
	}

	// Le telephone et le sac ne peuvent pas tenir ensemble dans le budget de
	// textdraws du client : 234 pour le smartphone ouvert, plus 51 pour la
	// grille, depasse les 256 autorises et la grille ne s'afficherait pas.
	if(phone_State[playerid] != PHONE_STATE_CLOSED) phone_Close(playerid);
	if(usingPhone[playerid] == 1)
		return SacV_Msg(playerid, "{FF8888}Ferme d'abord ton telephone.");
	// [TABLECRAFT] meme raison de budget : le panneau de craft partage lui
	// aussi l'ecran avec la grille du sac.
	TC_Fermer(playerid);

	if(svOuvert[playerid] == 1) SacV_Fermer(playerid);

	SacV_Creer(playerid);
	svOuvert[playerid] = 1;
	svPage[playerid]   = 0;
	svSel[playerid]    = -1;
	svDeplaceSrc[playerid] = -1;

	for(new k = 0; k < 5; k++) PlayerTextDrawShow(playerid, svFondP[playerid][k]);
	PlayerTextDrawShow(playerid, svPanneau[playerid]);
	for(new b = 0; b < SV_BOUTONS; b++) PlayerTextDrawShow(playerid, svBtn[playerid][b]);
	SacV_Remplir(playerid);
	SelectTextDraw(playerid, 0x00C8FFAA);

	SacV_Msg(playerid, "{AAAAAA}Clique un objet, puis une action a droite. Les armes restent sur /sac arme.");
	return 1;
}

stock SacV_Fermer(playerid)
{
	if(svOuvert[playerid] != 1) return 0;
	svOuvert[playerid] = 0;
	svSel[playerid]    = -1;
	svDeplaceSrc[playerid] = -1;
	SacV_Detruire(playerid);
	CancelSelectTextDraw(playerid);
	return 1;
}

// ============================================================================
// ACTIONS
// ============================================================================
// Emplacement reel du sac correspondant a la case selectionnee, -1 si rien.
stock SacV_SlotSel(playerid)
{
	if(svSel[playerid] < 0 || svSel[playerid] >= SV_CASES) return -1;
	new slot = svSlotDe[playerid][svSel[playerid]];
	if(slot < 0 || slot >= gPlayerInventory[playerid]) return -1;
	if(PlayerInfo[playerid][pInvModel][slot] == -1) return -1;
	return slot;
}

stock SacV_Details(playerid)
{
	new slot = SacV_SlotSel(playerid);
	if(slot == -1) return SacV_Msg(playerid, "{FF8888}Clique d'abord un objet.");
	new s[144], modelid = PlayerInfo[playerid][pInvModel][slot];
	format(s, sizeof(s), "{FFFF00}%s{FFFFFF} : %d en stock (modele %d).", GetItemName(modelid), PlayerInfo[playerid][pInvQuant][slot], modelid);
	return SacV_Msg(playerid, s);
}

// [LIEN REEL] /utiliser aliment existe deja dans le gamemode (dialogue 147)
// et consomme exactement ces memes modeles depuis le meme inventaire : on
// l'appelle au lieu de refaire la consommation ici. Pour tout le reste
// (drogues, outils...), chaque objet a deja sa propre commande dediee dans le
// gamemode ; en inventer une nouvelle ici aurait fait deux chemins pour la
// meme action, avec le risque qu'ils divergent.
stock SacV_Utiliser(playerid)
{
	new slot = SacV_SlotSel(playerid);
	if(slot == -1) return SacV_Msg(playerid, "{FF8888}Clique d'abord un objet.");
	new modelid = PlayerInfo[playerid][pInvModel][slot];

	if(SacV_EstAlimentBoisson(modelid))
	{
		SacV_Fermer(playerid);
		return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser aliment");
	}

	new s[190];
	format(s, sizeof(s), "{FF8888}%s ne se consomme pas depuis le sac. Tape {FFFFFF}/utiliser{FF8888} pour voir les commandes dediees.", GetItemName(modelid));
	return SacV_Msg(playerid, s);
}

// Reorganiser le sac : selectionner un objet, presser Deplacer, puis cliquer
// la case avec laquelle l'echanger.
stock SacV_Deplacer(playerid)
{
	new slot = SacV_SlotSel(playerid);
	if(slot == -1) return SacV_Msg(playerid, "{FF8888}Clique d'abord un objet a deplacer.");
	svDeplaceSrc[playerid] = svSel[playerid];
	return SacV_Msg(playerid, "{FFFF00}Clique maintenant la case ou tu veux le mettre.");
}

stock SacV_TerminerDeplacement(playerid, k)
{
	new srcCase = svDeplaceSrc[playerid];
	svDeplaceSrc[playerid] = -1;

	if(k == srcCase)
		return SacV_Msg(playerid, "{FFFF00}Deplacement annule.");
	if(svSlotDe[playerid][k] == -1)
		return SacV_Msg(playerid, "{FF8888}Cette case est vide, rien a echanger. Deplacement annule.");

	new slotA = svSlotDe[playerid][srcCase];
	new slotB = svSlotDe[playerid][k];

	new tmpModel = PlayerInfo[playerid][pInvModel][slotA];
	new tmpQuant = PlayerInfo[playerid][pInvQuant][slotA];
	PlayerInfo[playerid][pInvModel][slotA] = PlayerInfo[playerid][pInvModel][slotB];
	PlayerInfo[playerid][pInvQuant][slotA] = PlayerInfo[playerid][pInvQuant][slotB];
	PlayerInfo[playerid][pInvModel][slotB] = tmpModel;
	PlayerInfo[playerid][pInvQuant][slotB] = tmpQuant;

	svSel[playerid] = k;
	SacV_Remplir(playerid);
	return SacV_Msg(playerid, "{00FF00}Objets echanges.");
}

stock SacV_DemanderJeter(playerid)
{
	new slot = SacV_SlotSel(playerid);
	if(slot == -1) return SacV_Msg(playerid, "{FF8888}Clique d'abord un objet.");
	new body[190];
	format(body, sizeof(body), "{FFFFFF}Combien de {FFFF00}%s{FFFFFF} veux-tu jeter ?\n{AAAAAA}Tu en as %d. Ce qui est jete tombe au sol : n'importe qui peut le ramasser avec /sac ramasser.",
		GetItemName(PlayerInfo[playerid][pInvModel][slot]), PlayerInfo[playerid][pInvQuant][slot]);
	return ShowPlayerDialog(playerid, DIALOG_SV_JETER, DIALOG_STYLE_INPUT, "Jeter un objet", body, "Jeter", "Annuler");
}

// --- objets deposes/ramasses au sol ---
stock SacV_DropSlotLibre()
{
	for(new i = 0; i < SV_DROP_MAX; i++) if(!svDropUtilise[i]) return i;
	return -1;
}

// Fait vraiment tomber l'objet au sol (visuel + label), plutot que de le
// detruire : n'importe qui, y compris le joueur lui-meme, peut le ramasser
// ensuite avec /sac ramasser.
stock SacV_DeposerAuSol(playerid, modelid, qte)
{
	new i = SacV_DropSlotLibre();
	if(i == -1)
	{
		SacV_Msg(playerid, "{FF8888}Trop d'objets deja au sol sur le serveur, impossible d'en jeter un de plus pour l'instant.");
		return 0;
	}

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);

	svDropUtilise[i] = 1;
	svDropModel[i] = modelid;
	svDropQuant[i] = qte;
	svDropX[i] = x;
	svDropY[i] = y;
	svDropZ[i] = z;
	svDropVW[i] = GetPlayerVirtualWorld(playerid);
	svDropInt[i] = GetPlayerInterior(playerid);

	new visuel = SacV_ModeleAffiche(modelid);
	if(visuel == 0) visuel = 1211;   // modele generique de secours, jamais cense arriver
	svDropObjet[i] = CreateDynamicObject(visuel, x, y, z - 0.9, 0.0, 0.0, 0.0, svDropVW[i], svDropInt[i], -1, STREAM_DISTANCE);

	new svLibelle[80];
	format(svLibelle, sizeof(svLibelle), "{FFFF00}%s x%d\n{AAAAAA}/sac ramasser", GetItemName(modelid), qte);
	svDropLabel[i] = CreateDynamic3DTextLabel(svLibelle, 0xFFFF00F6, x, y, z + 0.3, LABEL_STREAM_DISTANCE, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, svDropVW[i], svDropInt[i], -1, LABEL_STREAM_DISTANCE);
	return 1;
}

stock SacV_LibererDrop(i)
{
	if(IsValidDynamicObject(svDropObjet[i])) DestroyDynamicObject(svDropObjet[i]);
	if(IsValidDynamic3DTextLabel(svDropLabel[i])) DestroyDynamic3DTextLabel(svDropLabel[i]);
	svDropUtilise[i] = 0;
	return 1;
}

// /sac ramasser : prend l'objet au sol le plus proche, s'il y a la place.
stock SacV_Ramasser(playerid)
{
	new Float:d, Float:meilleure = SV_DROP_DIST + 1.0, meilleurI = -1;
	new vw = GetPlayerVirtualWorld(playerid), itr = GetPlayerInterior(playerid);

	for(new i = 0; i < SV_DROP_MAX; i++)
	{
		if(!svDropUtilise[i]) continue;
		if(svDropVW[i] != vw || svDropInt[i] != itr) continue;
		d = GetPlayerDistanceFromPoint(playerid, svDropX[i], svDropY[i], svDropZ[i]);
		if(d > SV_DROP_DIST || d >= meilleure) continue;
		meilleure = d; meilleurI = i;
	}

	if(meilleurI == -1)
		return SacV_Msg(playerid, "{FF8888}Rien a ramasser a moins de 2.5 metres.");
	if(PlayerInfo[playerid][pBag] == 0)
		return SacV_Msg(playerid, "{FF8888}Tu n'as pas de sac pour ramasser quoi que ce soit.");
	if(!SacV_APlace(playerid, svDropModel[meilleurI]))
		return SacV_Msg(playerid, "{FF8888}Ton sac est plein.");

	new modelid = svDropModel[meilleurI], qte = svDropQuant[meilleurI];
	inventory_UpdateItem(playerid, modelid, qte);
	SacV_LibererDrop(meilleurI);

	new s[144];
	format(s, sizeof(s), "{00FF00}Ramasse : %d %s.", qte, GetItemName(modelid));
	SacV_Msg(playerid, s);
	if(svOuvert[playerid] == 1) SacV_Remplir(playerid);
	return 1;
}

// Bouton "Sac" : pose ou reprend le sac ENTIER, en rappelant les commandes
// existantes /sac poser et /sac prendre (deja fonctionnelles) plutot que de
// dupliquer leur logique.
stock SacV_ToggleSac(playerid)
{
	if(bag[playerid][statut] == 1)
	{
		SacV_Fermer(playerid);
		return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sac prendre");
	}
	SacV_Fermer(playerid);
	return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/sac poser");
}

stock SacV_DemanderDonner(playerid)
{
	new slot = SacV_SlotSel(playerid);
	if(slot == -1) return SacV_Msg(playerid, "{FF8888}Clique d'abord un objet.");

	// Un seul destinataire possible : le joueur le plus proche. Demander un
	// identifiant a taper aurait laisse donner un objet a l'autre bout de la
	// carte, ce qui n'a pas de sens et se prete au transfert entre comptes.
	new cible = -1, Float:meilleure = SV_DIST_DONNER + 1.0;
	new Float:x, Float:y, Float:z, Float:d;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pBag] == 0) continue;
		GetPlayerPos(i, x, y, z);
		d = GetPlayerDistanceFromPoint(playerid, x, y, z);
		if(d > SV_DIST_DONNER || d >= meilleure) continue;
		meilleure = d; cible = i;
	}
	if(cible == -1)
		return SacV_Msg(playerid, "{FF8888}Personne avec un sac a moins de 3 metres.");

	new body[190];
	format(body, sizeof(body), "{FFFFFF}Combien de {FFFF00}%s{FFFFFF} donner a {FFFF00}%s{FFFFFF} ?\n{AAAAAA}Tu en as %d.",
		GetItemName(PlayerInfo[playerid][pInvModel][slot]), PlayerInfo[cible][pName], PlayerInfo[playerid][pInvQuant][slot]);
	return ShowPlayerDialog(playerid, DIALOG_SV_DONNER, DIALOG_STYLE_INPUT, "Donner un objet", body, "Donner", "Annuler");
}

stock SacV_OnDialog(playerid, dialogid, response, const inputtext[])
{
	if(dialogid != DIALOG_SV_JETER && dialogid != DIALOG_SV_DONNER) return 0;

	if(!response)
	{
		if(svOuvert[playerid] == 1) SelectTextDraw(playerid, 0x00C8FFAA);
		return 1;
	}

	new slot = SacV_SlotSel(playerid);
	if(slot == -1)
	{
		SacV_Msg(playerid, "{FF8888}L'objet n'est plus la.");
		return 1;
	}

	new modelid = PlayerInfo[playerid][pInvModel][slot];
	new dispo   = PlayerInfo[playerid][pInvQuant][slot];
	new qte     = strval(inputtext);
	if(qte < 1 || qte > dispo)
	{
		new s[144];
		format(s, sizeof(s), "{FF4444}Quantite invalide : entre 1 et %d.", dispo);
		SacV_Msg(playerid, s);
		if(svOuvert[playerid] == 1) SelectTextDraw(playerid, 0x00C8FFAA);
		return 1;
	}

	new s[190];
	if(dialogid == DIALOG_SV_JETER)
	{
		inventory_UpdateItem(playerid, modelid, -qte);
		if(SacV_DeposerAuSol(playerid, modelid, qte))
			format(s, sizeof(s), "{FFFF00}Jete au sol : %d %s. (/sac ramasser pour le reprendre)", qte, GetItemName(modelid));
		else
			format(s, sizeof(s), "{FFFF00}Jete : %d %s.", qte, GetItemName(modelid));
		SacV_Msg(playerid, s);
	}
	else
	{
		// On refait la recherche du plus proche : le joueur a pu s'eloigner
		// pendant que la boite de dialogue etait ouverte.
		new cible = -1, Float:meilleure = SV_DIST_DONNER + 1.0;
		new Float:x, Float:y, Float:z, Float:d;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(i == playerid || !IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			if(PlayerInfo[i][pBag] == 0) continue;
			GetPlayerPos(i, x, y, z);
			d = GetPlayerDistanceFromPoint(playerid, x, y, z);
			if(d > SV_DIST_DONNER || d >= meilleure) continue;
			meilleure = d; cible = i;
		}
		if(cible == -1)
		{
			SacV_Msg(playerid, "{FF8888}Il n'y a plus personne a cote de toi.");
			if(svOuvert[playerid] == 1) SelectTextDraw(playerid, 0x00C8FFAA);
			return 1;
		}
		if(!server_IsValidItem(modelid))
		{
			SacV_Msg(playerid, "{FF8888}Cet objet ne peut pas etre transfere.");
			if(svOuvert[playerid] == 1) SelectTextDraw(playerid, 0x00C8FFAA);
			return 1;
		}
		if(!SacV_APlace(cible, modelid))
		{
			new t[144];
			format(t, sizeof(t), "{FF8888}Le sac de %s est plein.", PlayerInfo[cible][pName]);
			SacV_Msg(playerid, t);
			if(svOuvert[playerid] == 1) SelectTextDraw(playerid, 0x00C8FFAA);
			return 1;
		}
		inventory_UpdateItem(playerid, modelid, -qte);
		inventory_UpdateItem(cible, modelid, qte);
		format(s, sizeof(s), "{00FF00}Donne : %d %s a %s.", qte, GetItemName(modelid), PlayerInfo[cible][pName]);
		SacV_Msg(playerid, s);
		format(s, sizeof(s), "{00FF00}%s t'a donne %d %s.", PlayerInfo[playerid][pName], qte, GetItemName(modelid));
		SacV_Msg(cible, s);
	}

	svSel[playerid] = -1;
	if(svOuvert[playerid] == 1)
	{
		// La page a pu se vider : on recule si elle n'existe plus.
		if(svPage[playerid] >= SacV_NbPages(playerid)) svPage[playerid] = SacV_NbPages(playerid) - 1;
		SacV_Remplir(playerid);
		SelectTextDraw(playerid, 0x00C8FFAA);
	}
	return 1;
}

// ============================================================================
// CLICS
// ============================================================================
stock SacV_OnClick(playerid, PlayerText:playertextid)
{
	if(svOuvert[playerid] != 1) return 0;

	for(new k = 0; k < SV_CASES; k++)
	{
		if(playertextid != svModele[playerid][k]) continue;
		if(svDeplaceSrc[playerid] != -1) return SacV_TerminerDeplacement(playerid, k);
		if(svSlotDe[playerid][k] == -1) return 1;   // case vide : clic ignore
		svSel[playerid] = k;
		SacV_Remplir(playerid);
		return 1;
	}

	for(new b = 0; b < SV_BOUTONS; b++)
	{
		if(playertextid != svBtn[playerid][b]) continue;
		switch(b)
		{
			case SV_BTN_DETAILS:  SacV_Details(playerid);
			case SV_BTN_UTILISER: SacV_Utiliser(playerid);
			case SV_BTN_DONNER:   SacV_DemanderDonner(playerid);
			case SV_BTN_JETER:    SacV_DemanderJeter(playerid);
			case SV_BTN_DEPLACER: SacV_Deplacer(playerid);
			case SV_BTN_SAC:      SacV_ToggleSac(playerid);
			case SV_BTN_PAGE:
			{
				svPage[playerid]++;
				if(svPage[playerid] >= SacV_NbPages(playerid)) svPage[playerid] = 0;
				svSel[playerid] = -1;
				SacV_Remplir(playerid);
			}
			case SV_BTN_FERMER: SacV_Fermer(playerid);
		}
		return 1;
	}
	return 0;
}

// Echap pendant la selection : SA-MP previent via OnPlayerClickTextDraw avec
// un identifiant invalide. Sans ca, la grille resterait a l'ecran.
stock SacV_OnClickHorsZone(playerid, Text:clickedid)
{
	if(svOuvert[playerid] != 1) return 0;
	if(clickedid != Text:INVALID_TEXT_DRAW) return 0;
	SacV_Fermer(playerid);
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
stock SacV_OnConnect(playerid)
{
	// Le slot a pu servir a quelqu'un d'autre : sans remise a zero, la grille
	// se croirait ouverte et tenterait de detruire des textdraws inexistants.
	svOuvert[playerid] = 0;
	svPage[playerid]   = 0;
	svSel[playerid]    = -1;
	svDeplaceSrc[playerid] = -1;
	for(new k = 0; k < SV_CASES; k++) svSlotDe[playerid][k] = -1;
	return 1;
}

stock SacV_OnDisconnect(playerid)
{
	if(svOuvert[playerid] == 1) SacV_Detruire(playerid);
	svOuvert[playerid] = 0;
	svSel[playerid]    = -1;
	svPage[playerid]   = 0;
	svDeplaceSrc[playerid] = -1;
	return 1;
}
