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
	Un client SA-MP n'accepte que 256 textdraws par joueur. Longtemps ce
	gamemode en gardait pres de 200 en permanence, et la grille avait du etre
	bridee a 51 elements pour tenir. Ce n'est plus vrai : depuis que le
	telephone cree ses 165 textdraws d'apps a l'ouverture au lieu de les garder
	toute la session, le total resident est de 98 (4 notifications du
	telephone, 50 pour init_PlayerTexts, 44 pour le HUD). Il reste 158 places.
	La grille peut donc reprendre la composition complete du pack fourni :
	121 elements (5 par case x 20, 5 d'habillage, 2 par bouton x 8), soit un
	pic de 219 sur 256 quand le sac est ouvert.
	Ils restent crees a l'ouverture et detruits a la fermeture : hors du sac,
	le module ne coute rien.

	=== L'INTERFACE VIENT DU PACK "NOPIXEL" FOURNI (2026) ===
	La proprietaire a fourni un pack d'inventaire NoPixel complet, d'un autre
	gamemode, et prefere son interface a la notre. C'est donc la MISE EN PAGE
	qui est reprise, pas seulement la palette comme au premier essai :
	- grille 5 colonnes au pas de 39 px (x=125/164/203/242/281), 4 lignes au
	  pas de 52 px (y=137/189/241/293), cases de 37x49 ;
	- 5 elements par case comme dans le pack : la tuile de fond (INDEXTD, qui
	  est aussi le seul element cliquable et change de couleur a la
	  selection), l'apercu 3D (MODELTD), le nom (NAMETD), la quantite
	  (AMOUNTTD) et le trait d'accent sous la case (GARISBAWAH) ;
	- le bandeau d'entete du pack : fond, filet, titre a gauche, compteur a
	  droite ;
	- les boutons a la maniere du pack : un pave sprite + son libelle centre.
	Le pack n'a que 4 boutons (Jumlah/Gunakan/Berikan/Tutup). Les 8 du gamemode
	sont conserves et habilles dans le meme langage visuel : en supprimer un
	aurait supprime une fonction.
	CE QUI N'EST PAS REPRIS, le stockage : le pack range les objets dans
	InventoryData[][MAX_INVENTORY] avec des cles texte et sa propre table
	MySQL, incompatible avec PlayerInfo[][pInvModel/pInvQuant] que des dizaines
	d'endroits du gamemode lisent et ecrivent deja (metiers, dealer, magasins,
	/craft, armes). Ses Inventory_Add/Set/Remove sont donc ignores.
	Son NOTIFBOX (bulle "objet recu") ne l'est pas non plus : SacV_Msg dit deja
	tout dans le chat, ou le joueur garde l'historique, alors qu'une bulle
	disparait. Cinq textdraws pour dire moins bien la meme chose.

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

// Geometrie reprise telle quelle du pack fourni (CreatePlayerInv) : panneau
// principal 118,114 sur 206x237, grille 5x4 de cases 37x49 aux pas 39/52.
// Le pack, dessine a la main dans un editeur, a quelques pixels de jeu entre
// ses lignes (137/189/241/294) ; on prend le pas regulier de 52.
#define SV_PANEL_X        118.0
#define SV_PANEL_Y        114.0
#define SV_PANEL_W        206.0
#define SV_PANEL_H        237.0
#define SV_CASE_X0        125.0
#define SV_CASE_Y0        137.0
#define SV_CASE_W         37.0
#define SV_CASE_H         49.0
#define SV_COL_PAS        39.0
#define SV_LIGNE_PAS      52.0

// Panneau d'actions : le pack le cale a droite de la grille (330..418) pour
// 4 boutons. Il en faut 8 ici, donc il descend jusqu'au bas du panneau
// principal (351) au lieu de s'arreter a 286 ; le pas passe de 23 a 26.
// Bornes verifiees : dernier bouton 319..341, sous le bas du fond a 351, et
// x=330 commence apres la grille qui finit a 318 : rien ne se recouvre.
#define SV_ACT_X          330.0
#define SV_ACT_Y          130.0
#define SV_ACT_W          88.0
#define SV_ACT_H          221.0
#define SV_BTN_X          334.0
#define SV_BTN_Y0         137.0
#define SV_BTN_W          79.0
#define SV_BTN_H          22.0
#define SV_BTN_PAS        26.0

// Palette reprise du pack NoPixel (INDEXTD/JUMLAH/panneaux) : ardoise foncee
// pour les cases et boutons au repos, or pour la selection, quasi-noir pour
// les fonds de panneau. Coordonnees RGBA (ordre SA-MP), pas ARGB.
#define SV_COUL_CASE      960053503     // case/bouton au repos (ardoise)
#define SV_COUL_CASE_SEL  0xFFD70091    // case selectionnee (or, ex-cyan)
#define SV_COUL_TRAIT     16777215      // trait d'accent du pack (cyan opaque)
#define SV_COUL_TRAIT_SEL 0xFFD700FF    // le meme trait sous la case selectionnee
#define SV_COUL_FOND      151655679     // fond des panneaux (quasi-noir)
#define SV_DIST_DONNER    3.0

// Largeur d'un caractere, en pixels, pour une taille de lettre donnee : sert a
// centrer les libelles des boutons et a couper les noms trop longs. Releve sur
// les textdraws existants du gamemode (0.119 tient ~15 caracteres en 39 px).
#define SV_CHAR_PX(%0)    ((%0) * 21.8)

// Boutons du panneau de droite.
#define SV_BTN_DETAILS    0
#define SV_BTN_UTILISER   1
#define SV_BTN_DONNER     2
#define SV_BTN_JETER      3
#define SV_BTN_DEPLACER   4
#define SV_BTN_SAC        5
#define SV_BTN_PAGE       6
#define SV_BTN_FERMER     7

// Habillage : 0 fond principal, 1 filet de l'entete, 2 titre, 3 compteur.
new PlayerText:svFondP[MAX_PLAYERS][4];
new PlayerText:svPanneau[MAX_PLAYERS];              // fond du panneau d'actions
// Les cinq elements par case du pack fourni. svCase est le seul cliquable :
// les quatre autres se dessinent par-dessus sans intercepter le clic, exactement
// comme INDEXTD/MODELTD/NAMETD/AMOUNTTD/GARISBAWAH dans le pack.
new PlayerText:svCase[MAX_PLAYERS][SV_CASES];       // tuile de fond (INDEXTD)
new PlayerText:svModele[MAX_PLAYERS][SV_CASES];     // apercu 3D (MODELTD)
new PlayerText:svNom[MAX_PLAYERS][SV_CASES];        // nom (NAMETD)
new PlayerText:svQte[MAX_PLAYERS][SV_CASES];        // quantite (AMOUNTTD)
new PlayerText:svTrait[MAX_PLAYERS][SV_CASES];      // trait d'accent (GARISBAWAH)
new PlayerText:svBtn[MAX_PLAYERS][SV_BOUTONS];      // pave du bouton (cliquable)
new PlayerText:svBtnTxt[MAX_PLAYERS][SV_BOUTONS];   // libelle pose dessus

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
// L'ordre de creation suit l'ordre d'affichage (fonds, puis tuiles, puis
// contenu des cases, puis boutons) : sur SA-MP c'est le dernier montre qui se
// dessine par-dessus, et garder les deux ordres identiques evite de se
// demander lequel des deux compte le jour ou l'on ajoute un element.
stock SacV_Creer(playerid)
{
	new Float:x, Float:y;

	// --- fond principal du pack (INVNAME[0]) ---
	svFondP[playerid][0] = CreatePlayerTextDraw(playerid, SV_PANEL_X, SV_PANEL_Y, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svFondP[playerid][0], SV_PANEL_W, SV_PANEL_H);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][0], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][0], SV_COUL_FOND);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][0], 255);
	PlayerTextDrawFont(playerid, svFondP[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][0], 1);

	// --- fond du panneau d'actions (INVNAME[4] du pack) ---
	svPanneau[playerid] = CreatePlayerTextDraw(playerid, SV_ACT_X, SV_ACT_Y, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svPanneau[playerid], SV_ACT_W, SV_ACT_H);
	PlayerTextDrawAlignment(playerid, svPanneau[playerid], 1);
	PlayerTextDrawColor(playerid, svPanneau[playerid], SV_COUL_FOND);
	PlayerTextDrawSetShadow(playerid, svPanneau[playerid], 0);
	PlayerTextDrawSetOutline(playerid, svPanneau[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, svPanneau[playerid], 255);
	PlayerTextDrawFont(playerid, svPanneau[playerid], 4);
	PlayerTextDrawSetProportional(playerid, svPanneau[playerid], 1);

	// --- bandeau d'entete (INVNAME[1..3]) : filet, titre, compteur ---
	svFondP[playerid][1] = CreatePlayerTextDraw(playerid, 125.0, 130.0, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, svFondP[playerid][1], 193.0, 4.0);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][1], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][1], SV_COUL_TRAIT);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][1], 255);
	PlayerTextDrawFont(playerid, svFondP[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][1], 1);

	svFondP[playerid][2] = CreatePlayerTextDraw(playerid, 125.0, 119.0, "Sac");
	PlayerTextDrawLetterSize(playerid, svFondP[playerid][2], 0.209, 0.999);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][2], 1);
	PlayerTextDrawColor(playerid, svFondP[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][2], 150);
	PlayerTextDrawFont(playerid, svFondP[playerid][2], 1);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][2], 1);

	// Compteur cale a droite du bandeau (le pack le pose a gauche en x=277,
	// mais notre texte varie de longueur : aligne a droite, il ne peut pas
	// venir mordre le titre ni deborder du fond a x=324).
	svFondP[playerid][3] = CreatePlayerTextDraw(playerid, 318.0, 121.0, "0/0");
	PlayerTextDrawLetterSize(playerid, svFondP[playerid][3], 0.189, 0.799);
	PlayerTextDrawAlignment(playerid, svFondP[playerid][3], 3);
	PlayerTextDrawColor(playerid, svFondP[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, svFondP[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, svFondP[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, svFondP[playerid][3], 150);
	PlayerTextDrawFont(playerid, svFondP[playerid][3], 1);
	PlayerTextDrawSetProportional(playerid, svFondP[playerid][3], 1);

	// --- les 20 cases, cinq elements chacune comme dans le pack ---
	for(new k = 0; k < SV_CASES; k++)
	{
		x = SV_CASE_X0 + (float(k % SV_COLS) * SV_COL_PAS);
		y = SV_CASE_Y0 + (float(k / SV_COLS) * SV_LIGNE_PAS);

		// La tuile (INDEXTD) porte le clic ET la couleur de selection. Les quatre
		// elements suivants se posent dessus sans etre selectionnables : SA-MP ne
		// laisse intercepter un clic qu'aux textdraws selectionnables, les autres
		// ne bloquent rien. C'est exactement le montage du pack.
		svCase[playerid][k] = CreatePlayerTextDraw(playerid, x, y, "LD_SPAC:white");
		PlayerTextDrawTextSize(playerid, svCase[playerid][k], SV_CASE_W, SV_CASE_H);
		PlayerTextDrawAlignment(playerid, svCase[playerid][k], 1);
		PlayerTextDrawColor(playerid, svCase[playerid][k], SV_COUL_CASE);
		PlayerTextDrawSetShadow(playerid, svCase[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svCase[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svCase[playerid][k], 255);
		PlayerTextDrawFont(playerid, svCase[playerid][k], 4);
		PlayerTextDrawSetProportional(playerid, svCase[playerid][k], 1);
		PlayerTextDrawSetSelectable(playerid, svCase[playerid][k], 1);

		svModele[playerid][k] = CreatePlayerTextDraw(playerid, x, y + 2.0, "_");
		PlayerTextDrawTextSize(playerid, svModele[playerid][k], SV_CASE_W, 46.0);
		PlayerTextDrawAlignment(playerid, svModele[playerid][k], 1);
		PlayerTextDrawColor(playerid, svModele[playerid][k], -1);
		PlayerTextDrawSetShadow(playerid, svModele[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svModele[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svModele[playerid][k], 0);
		PlayerTextDrawFont(playerid, svModele[playerid][k], 5);
		PlayerTextDrawSetProportional(playerid, svModele[playerid][k], 0);
		PlayerTextDrawSetPreviewModel(playerid, svModele[playerid][k], 1212);
		PlayerTextDrawSetPreviewRot(playerid, svModele[playerid][k], 0.0, 0.0, 0.0, 1.0);

		svNom[playerid][k] = CreatePlayerTextDraw(playerid, x + 2.0, y + 2.0, "_");
		PlayerTextDrawLetterSize(playerid, svNom[playerid][k], 0.159, 0.898);
		PlayerTextDrawAlignment(playerid, svNom[playerid][k], 1);
		PlayerTextDrawColor(playerid, svNom[playerid][k], -1);
		PlayerTextDrawSetShadow(playerid, svNom[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svNom[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svNom[playerid][k], 150);
		PlayerTextDrawFont(playerid, svNom[playerid][k], 1);
		PlayerTextDrawSetProportional(playerid, svNom[playerid][k], 1);

		svQte[playerid][k] = CreatePlayerTextDraw(playerid, x + 2.0, y + 40.0, "_");
		PlayerTextDrawLetterSize(playerid, svQte[playerid][k], 0.150, 0.799);
		PlayerTextDrawAlignment(playerid, svQte[playerid][k], 1);
		PlayerTextDrawColor(playerid, svQte[playerid][k], -1);
		PlayerTextDrawSetShadow(playerid, svQte[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svQte[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svQte[playerid][k], 150);
		PlayerTextDrawFont(playerid, svQte[playerid][k], 1);
		PlayerTextDrawSetProportional(playerid, svQte[playerid][k], 1);

		svTrait[playerid][k] = CreatePlayerTextDraw(playerid, x, y + 48.0, "LD_SPAC:white");
		PlayerTextDrawTextSize(playerid, svTrait[playerid][k], SV_CASE_W, 2.0);
		PlayerTextDrawAlignment(playerid, svTrait[playerid][k], 1);
		PlayerTextDrawColor(playerid, svTrait[playerid][k], SV_COUL_TRAIT);
		PlayerTextDrawSetShadow(playerid, svTrait[playerid][k], 0);
		PlayerTextDrawSetOutline(playerid, svTrait[playerid][k], 0);
		PlayerTextDrawBackgroundColor(playerid, svTrait[playerid][k], 255);
		PlayerTextDrawFont(playerid, svTrait[playerid][k], 4);
		PlayerTextDrawSetProportional(playerid, svTrait[playerid][k], 1);
	}

	// --- les 8 boutons : pave sprite + libelle, comme JUMLAH + INVNAME[5] ---
	// Le pack cale ses libelles a des x tapes a la main ; ici le libelle est
	// centre par calcul, sinon "Sac" et "Deplacer" ne pourraient pas partager
	// la meme abscisse sans que l'un des deux soit de travers.
	new libelles[SV_BOUTONS][12] = { "Details", "Utiliser", "Donner", "Jeter", "Deplacer", "Sac", "Page 1/1", "Fermer" };
	for(new b = 0; b < SV_BOUTONS; b++)
	{
		y = SV_BTN_Y0 + (float(b) * SV_BTN_PAS);

		svBtn[playerid][b] = CreatePlayerTextDraw(playerid, SV_BTN_X, y, "LD_SPAC:white");
		PlayerTextDrawTextSize(playerid, svBtn[playerid][b], SV_BTN_W, SV_BTN_H);
		PlayerTextDrawAlignment(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawColor(playerid, svBtn[playerid][b], SV_COUL_CASE);
		PlayerTextDrawSetShadow(playerid, svBtn[playerid][b], 0);
		PlayerTextDrawSetOutline(playerid, svBtn[playerid][b], 0);
		PlayerTextDrawBackgroundColor(playerid, svBtn[playerid][b], 255);
		PlayerTextDrawFont(playerid, svBtn[playerid][b], 4);
		PlayerTextDrawSetProportional(playerid, svBtn[playerid][b], 1);
		PlayerTextDrawSetSelectable(playerid, svBtn[playerid][b], 1);

		x = SV_BTN_X + ((SV_BTN_W - (float(strlen(libelles[b])) * SV_CHAR_PX(0.30))) / 2.0);
		svBtnTxt[playerid][b] = CreatePlayerTextDraw(playerid, x, y + 6.0, libelles[b]);
		PlayerTextDrawLetterSize(playerid, svBtnTxt[playerid][b], 0.30, 0.999);
		PlayerTextDrawAlignment(playerid, svBtnTxt[playerid][b], 1);
		PlayerTextDrawColor(playerid, svBtnTxt[playerid][b], -1);
		PlayerTextDrawSetShadow(playerid, svBtnTxt[playerid][b], 0);
		PlayerTextDrawSetOutline(playerid, svBtnTxt[playerid][b], 0);
		PlayerTextDrawBackgroundColor(playerid, svBtnTxt[playerid][b], 150);
		PlayerTextDrawFont(playerid, svBtnTxt[playerid][b], 1);
		PlayerTextDrawSetProportional(playerid, svBtnTxt[playerid][b], 1);
	}
	return 1;
}

stock SacV_Detruire(playerid)
{
	for(new k = 0; k < 4; k++) PlayerTextDrawDestroy(playerid, svFondP[playerid][k]);
	PlayerTextDrawDestroy(playerid, svPanneau[playerid]);
	for(new k = 0; k < SV_CASES; k++)
	{
		PlayerTextDrawDestroy(playerid, svCase[playerid][k]);
		PlayerTextDrawDestroy(playerid, svModele[playerid][k]);
		PlayerTextDrawDestroy(playerid, svNom[playerid][k]);
		PlayerTextDrawDestroy(playerid, svQte[playerid][k]);
		PlayerTextDrawDestroy(playerid, svTrait[playerid][k]);
	}
	for(new b = 0; b < SV_BOUTONS; b++)
	{
		PlayerTextDrawDestroy(playerid, svBtn[playerid][b]);
		PlayerTextDrawDestroy(playerid, svBtnTxt[playerid][b]);
	}
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

	// Les cases ne se chevauchent pas entre elles (37 de large au pas de 39,
	// 49 de haut au pas de 52) : seul l'ordre A L'INTERIEUR d'une case compte.
	// La tuile passe donc en premier a chaque fois, sans quoi elle recouvrirait
	// l'apercu et le nom au reaffichage qui suit un changement de selection.
	for(new k = 0; k < SV_CASES; k++)
	{
		new slot = svSlotDe[playerid][k];
		new selectionnee = (svSel[playerid] == k && slot != -1);

		PlayerTextDrawColor(playerid, svCase[playerid][k],
			(selectionnee) ? (SV_COUL_CASE_SEL) : (SV_COUL_CASE));
		PlayerTextDrawShow(playerid, svCase[playerid][k]);

		if(slot == -1)
		{
			// Case vide : la tuile reste (et reste cliquable, pour servir de
			// destination a un deplacement), tout le contenu part. L'apercu est
			// masque plutot que mis au modele 0, un modele invalide affichant
			// parfois un objet au hasard cote client.
			PlayerTextDrawHide(playerid, svModele[playerid][k]);
			PlayerTextDrawHide(playerid, svNom[playerid][k]);
			PlayerTextDrawHide(playerid, svQte[playerid][k]);
			PlayerTextDrawHide(playerid, svTrait[playerid][k]);
			continue;
		}

		new modelid = PlayerInfo[playerid][pInvModel][slot];
		format(nom, sizeof(nom), "%s", GetItemName(modelid));
		// Un nom trop long depasserait de la case : 37 px a 0.159 de largeur de
		// lettre tiennent une dizaine de caracteres.
		if(strlen(nom) > 10) nom[10] = 0;

		PlayerTextDrawSetPreviewModel(playerid, svModele[playerid][k], SacV_ModeleAffiche(modelid));
		PlayerTextDrawShow(playerid, svModele[playerid][k]);

		PlayerTextDrawSetString(playerid, svNom[playerid][k], nom);
		PlayerTextDrawShow(playerid, svNom[playerid][k]);

		format(s, sizeof(s), "x%d", PlayerInfo[playerid][pInvQuant][slot]);
		PlayerTextDrawSetString(playerid, svQte[playerid][k], s);
		PlayerTextDrawShow(playerid, svQte[playerid][k]);

		// Le trait du pack est statique ; le passer en or sous la case
		// selectionnee rend le choix lisible meme quand l'objet couvre la tuile.
		PlayerTextDrawColor(playerid, svTrait[playerid][k],
			(selectionnee) ? (SV_COUL_TRAIT_SEL) : (SV_COUL_TRAIT));
		PlayerTextDrawShow(playerid, svTrait[playerid][k]);
	}

	format(s, sizeof(s), "%d/%d objets", SacV_NbObjets(playerid), gPlayerInventory[playerid]);
	PlayerTextDrawSetString(playerid, svFondP[playerid][3], s);
	PlayerTextDrawShow(playerid, svFondP[playerid][3]);

	// La pagination s'affiche sur son propre bouton : le bandeau reste lisible
	// et ca evite un textdraw de plus rien que pour "page 1/2".
	format(s, sizeof(s), "Page %d/%d", svPage[playerid] + 1, SacV_NbPages(playerid));
	PlayerTextDrawSetString(playerid, svBtnTxt[playerid][SV_BTN_PAGE], s);
	PlayerTextDrawShow(playerid, svBtnTxt[playerid][SV_BTN_PAGE]);
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
	// textdraws du client : 234 pour le smartphone ouvert, plus 121 pour la
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

	// [ORDRE D'AFFICHAGE] Sur SA-MP, un textdraw montre plus tard se dessine
	// PAR-DESSUS les precedents : c'est exactement le bug qu'avait /craft, dont
	// le fond etait affiche en dernier et masquait ses propres boutons. Les
	// deux fonds partent donc en premier, le contenu ensuite.
	PlayerTextDrawShow(playerid, svFondP[playerid][0]);
	PlayerTextDrawShow(playerid, svPanneau[playerid]);

	new titre[40];
	format(titre, sizeof(titre), "Sac - %s", PlayerInfo[playerid][pName]);
	PlayerTextDrawSetString(playerid, svFondP[playerid][2], titre);
	PlayerTextDrawShow(playerid, svFondP[playerid][1]);
	PlayerTextDrawShow(playerid, svFondP[playerid][2]);

	for(new b = 0; b < SV_BOUTONS; b++)
	{
		PlayerTextDrawShow(playerid, svBtn[playerid][b]);
		PlayerTextDrawShow(playerid, svBtnTxt[playerid][b]);
	}
	SacV_Remplir(playerid);          // cases + compteur + libelle de pagination
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

// [LIEN REEL] /utiliser aliment et /utiliser drogue <nom> existent deja dans
// le gamemode (LVRP.pwn ~70372) et consomment exactement ces memes modeles
// depuis le meme inventaire : on les appelle au lieu de refaire la
// consommation ici. En inventer une deuxieme aurait fait deux chemins pour la
// meme action (doses en cours, ingredients requis...), avec le risque qu'ils
// divergent - c'est pour ca que le bouton "Utiliser" ne faisait jusqu'ici que
// les aliments : personne n'avait fini le reste de ce branchement.
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

	// [DROGUES] memes modeles que /utiliser drogue <nom>, voir GetItemName().
	switch(modelid)
	{
		case 1575: // Weed brute -> se roule en joint (consomme Feuilles + Tabac)
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser drogue weed rouler");
		}
		case 1485: // Joint deja roule -> s'allume et se fume (consomme un Briquet)
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser drogue weed utiliser");
		}
		case 1579: // Heroine -> injection directe
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser drogue heroine");
		}
		case 1578: // Cocaine -> sniff direct
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser drogue cocaine");
		}
		case 1580: // Ecstasy -> avale directement
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/utiliser drogue ecstasy");
		}
		case 19897: // Cigarettes -> meme commande que /fumer (consomme un Briquet)
		{
			SacV_Fermer(playerid);
			return CallLocalFunction("OnPlayerCommandText", "is", playerid, "/fumer");
		}
		case 328, 2751, 2351: // Feuilles, Tabac, Briquet : ingredients, pas d'usage seul
		{
			return SacV_Msg(playerid, "{FF8888}Cet objet sert d'ingr\xe9dient (roulage ou allumage) : il s'utilise tout seul au bon moment, pas depuis ce bouton.");
		}
		case 1950: // Morphine -> matiere premiere de la Mafia (/mafia drogue hero synt), pas un usage simple
		{
			return SacV_Msg(playerid, "{FF8888}La morphine est une mati\xe8re premi\xe8re de fabrication (voir {FFFFFF}/mafia drogue heroine synth\xe9tiser{FF8888}), pas un objet \xe0 utiliser directement.");
		}
		case 2966: // Talkie Walkie -> radio vocale, deplacee dans le menu Y (touche Y, bouton Phone)
		{
			return SacV_Msg(playerid, "{8B8B00}Talkie Walkie {FFFFFF}: ouvre le menu Y (touche Y) puis {FFFF00}Phone{FFFFFF} pour rejoindre une radio.");
		}
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

	// C'est la tuile de fond qui porte le clic (comme INDEXTD dans le pack) :
	// l'apercu 3D pose dessus n'est pas selectionnable et ne l'intercepte pas.
	for(new k = 0; k < SV_CASES; k++)
	{
		if(playertextid != svCase[playerid][k]) continue;
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
