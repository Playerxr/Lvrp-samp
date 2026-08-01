/*
	afrp_speedo2.inc - Tableau de bord moderne (habillage alternatif)

	Le serveur avait deja un compteur fonctionnel : vitesse, etat du vehicule,
	kilometrage, jauge d'essence, moteur, phares, clignotants. Ce qui manquait
	n'etait pas l'information mais l'allure. Un pack de 29 textdraws fourni par
	le proprietaire du serveur redessine le tableau de bord avec un apercu du
	vehicule en 3D.

	Ce module ne remplace donc pas la logique : il en refait l'affichage, en
	lisant exactement les memes donnees que l'ancien compteur.

	  /compteur   bascule entre le tableau de bord moderne et le classique

	=== DEUX CORRECTIONS SUR LE PACK D'ORIGINE ===
	1. Le pack declarait ses 29 elements en textdraws GLOBAUX. Un textdraw
	   global n'a qu'un seul contenu pour tout le serveur : le premier joueur
	   a monter en voiture aurait impose sa vitesse et son vehicule a tous les
	   autres. Tout est converti en textdraws par joueur.
	2. Les quatre apercus 3D etaient restes sur le modele 2153, un meuble de
	   cuisine laisse par l'editeur de textdraws. Ils affichent maintenant le
	   vehicule reellement conduit.

	=== LISIBILITE ===
	Le pack dessinait le vehicule QUATRE fois : deux grands apercus superposes a
	quelques pixels d'ecart, et deux minuscules laisses par l'editeur. Le
	panneau paraissait flou. Un seul apercu est conserve. Les chiffres de la
	vitesse, le libelle d'essence et les temoins etaient en police fine sur fond
	sombre : leurs tailles et leurs fonds ont ete revus pour etre lus en roulant.

	=== UN ELEMENT AJOUTE ===
	Le pack n'avait pas de place pour l'etat du vehicule ni le kilometrage, que
	l'ancien compteur affichait. Les perdre en gagnant du style aurait ete un
	mauvais echange : une ligne a ete ajoutee en bas du panneau.

	=== LES ELEMENTS NE VIVENT QUE PENDANT LA CONDUITE ===
	Ils sont crees en montant dans un vehicule et detruits en descendant. Un
	client SA-MP n'accepte que 256 textdraws par joueur ; le telephone en tient
	deja 169 en permanence. Les garder en reserve a pied aurait pu empecher un
	autre panneau de s'afficher, sans message d'erreur pour l'expliquer.

	=== CE QUI RESTE A L'ANCIEN COMPTEUR ===
	Les clignotants (speedometer_Blinking) et l'alerte de bridage continuent de
	passer par le code existant. Ce module ne touche pas a la conduite.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Sp2_Init()
	- OnPlayerConnect    -> Sp2_OnConnect(playerid)
	- OnPlayerDisconnect -> Sp2_OnDisconnect(playerid)
	- speedometer_Show   -> if(Sp2_Show(playerid)) return 1;   EN PREMIERE LIGNE
	- speedometer_Hide   -> Sp2_Hide(playerid);                EN PREMIERE LIGNE
	- /compteur          -> Sp2_Cmd(playerid)
*/

#if defined AFRP_SPEEDO2_INCLUDED
	#endinput
#endif
#define AFRP_SPEEDO2_INCLUDED

#define SP2_NB_TD        30      // 29 du pack + la ligne etat / kilometrage
#define SP2_TD_INFO      29
#define SP2_TICK_MS      300     // 3 rafraichissements par seconde

// Elements du pack qui recoivent une valeur a chaque rafraichissement.
#define SP2_TD_VITESSE   28
#define SP2_TD_JAUGE     19     // barre de remplissage (largeur variable)
#define SP2_TD_ESSENCE   20     // texte au-dessus de la barre
#define SP2_TD_LOCK      21
#define SP2_TD_ENGINE    22
#define SP2_TD_LIGHT     23

#define SP2_JAUGE_X      518.0017
#define SP2_JAUGE_W_MAX  57.0    // largeur de la barre pleine, cf. le pack

// Elements decoratifs du pack qui ne sont jamais affiches : le pack posait
// DEUX grands apercus 3D l'un sur l'autre, decales de quelques pixels, plus
// deux minuscules. Le vehicule etait donc dessine quatre fois, ce qui rendait
// le panneau flou et illisible. On n'en garde qu'un, le plus grand.
#define SP2_TD_APERCU    3

new PlayerText:spTD[MAX_PLAYERS][SP2_NB_TD];
new spCree[MAX_PLAYERS];        // 1 = textdraws crees pour ce joueur
new spVisible[MAX_PLAYERS];     // 1 = panneau affiche
new spClassique[MAX_PLAYERS];   // 1 = le joueur prefere l'ancien compteur
new spDernierModel[MAX_PLAYERS];// evite de rappeler SetPreviewModel pour rien
new spTimer = -1;

forward Sp2_Tick();

// ============================================================================
// CREATION
// ============================================================================
// 1 = element du pack volontairement laisse de cote.
stock Sp2_Decoratif(k)
{
	if(k == 1)  return 1;   // second grand apercu, superpose au premier
	if(k == 12) return 1;   // apercu minuscule
	if(k == 16) return 1;   // apercu minuscule, retourne
	return 0;
}

stock Sp2_Creer(playerid)
{
	if(spCree[playerid] == 1) return 0;

	spTD[playerid][0] = CreatePlayerTextDraw(playerid, 520.1669, 371.6667, "particle:lamp_shad_64");
	PlayerTextDrawTextSize(playerid, spTD[playerid][0], 76.0000, 33.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][0], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][0], 673720351);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][0], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][0], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][0], 0);
	spTD[playerid][1] = CreatePlayerTextDraw(playerid, 578.8068, 349.2500, "_");
	PlayerTextDrawTextSize(playerid, spTD[playerid][1], 69.0000, 51.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][1], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][1], -40817153);
	PlayerTextDrawFont(playerid, spTD[playerid][1], 5);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][1], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][1], 0);
	PlayerTextDrawSetPreviewModel(playerid, spTD[playerid][1], 2153);
	PlayerTextDrawSetPreviewRot(playerid, spTD[playerid][1], 0.0000, -15.0000, 90.0000, 1.0000);
	spTD[playerid][2] = CreatePlayerTextDraw(playerid, 598.5870, 386.5832, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][2], 20.0000, 20.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][2], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][2], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][2], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][2], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][2], 0);
	spTD[playerid][3] = CreatePlayerTextDraw(playerid, 560.0659, 345.7499, "_");
	PlayerTextDrawTextSize(playerid, spTD[playerid][3], 67.0000, 55.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][3], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][3], -40817153);
	PlayerTextDrawFont(playerid, spTD[playerid][3], 5);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][3], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][3], 0);
	PlayerTextDrawSetPreviewModel(playerid, spTD[playerid][3], 2153);
	PlayerTextDrawSetPreviewRot(playerid, spTD[playerid][3], 0.0000, -15.0000, 90.0000, 1.0000);
	spTD[playerid][4] = CreatePlayerTextDraw(playerid, 584.4331, 392.4166, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][4], 24.0000, 11.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][4], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][4], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][4], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][4], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][4], 0);
	spTD[playerid][5] = CreatePlayerTextDraw(playerid, 576.0000, 387.0000, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][5], 20.0000, 20.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][5], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][5], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][5], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][5], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][5], 0);
	spTD[playerid][6] = CreatePlayerTextDraw(playerid, 600.3627, 346.3332, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][6], 24.0000, 11.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][6], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][6], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][6], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][6], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][6], 0);
	spTD[playerid][7] = CreatePlayerTextDraw(playerid, 590.6226, 342.8333, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][7], 20.0000, 20.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][7], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][7], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][7], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][7], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][7], 0);
	spTD[playerid][8] = CreatePlayerTextDraw(playerid, 597.5516, 353.3333, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][8], 17.0000, 44.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][8], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][8], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][8], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][8], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][8], 0);
	spTD[playerid][9] = CreatePlayerTextDraw(playerid, 614.5171, 342.8333, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][9], 20.0000, 20.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][9], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][9], -40817153);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][9], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][9], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][9], 0);
	spTD[playerid][10] = CreatePlayerTextDraw(playerid, 625.1155, 413.0834, "particle:lamp_shad_64");
	PlayerTextDrawTextSize(playerid, spTD[playerid][10], -131.0000, -67.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][10], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][10], 673720436);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][10], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][10], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][10], 0);
	spTD[playerid][11] = CreatePlayerTextDraw(playerid, 570.0070, 366.1667, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][11], 15.0000, 15.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][11], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][11], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][11], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][11], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][11], 0);
	spTD[playerid][12] = CreatePlayerTextDraw(playerid, 575.0589, 374.3332, "_");
	PlayerTextDrawTextSize(playerid, spTD[playerid][12], 10.0000, 9.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][12], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][12], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][12], 336860160);
	PlayerTextDrawFont(playerid, spTD[playerid][12], 5);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][12], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][12], 0);
	PlayerTextDrawSetPreviewModel(playerid, spTD[playerid][12], 2153);
	PlayerTextDrawSetPreviewRot(playerid, spTD[playerid][12], 0.0000, -15.0000, 90.0000, 1.0000);
	spTD[playerid][13] = CreatePlayerTextDraw(playerid, 568.1330, 373.1668, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][13], 15.0000, 15.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][13], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][13], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][13], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][13], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][13], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][13], 0);
	spTD[playerid][14] = CreatePlayerTextDraw(playerid, 517.4339, 368.5000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][14], 59.0000, 17.5000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][14], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][14], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][14], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][14], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][14], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][14], 0);
	spTD[playerid][15] = CreatePlayerTextDraw(playerid, 511.4421, 366.1667, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][15], 15.0000, 15.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][15], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][15], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][15], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][15], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][15], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][15], 0);
	spTD[playerid][16] = CreatePlayerTextDraw(playerid, 519.3051, 380.1664, "_");
	PlayerTextDrawTextSize(playerid, spTD[playerid][16], -10.0000, -9.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][16], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][16], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][16], 336860160);
	PlayerTextDrawFont(playerid, spTD[playerid][16], 5);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][16], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][16], 0);
	PlayerTextDrawSetPreviewModel(playerid, spTD[playerid][16], 2153);
	PlayerTextDrawSetPreviewRot(playerid, spTD[playerid][16], 0.0000, -15.0000, 90.0000, 1.0000);
	spTD[playerid][17] = CreatePlayerTextDraw(playerid, 509.0996, 373.1667, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][17], 15.0000, 15.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][17], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][17], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][17], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][17], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][17], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][17], 0);
	spTD[playerid][18] = CreatePlayerTextDraw(playerid, 518.0017, 379.0000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][18], 57.0000, 3.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][18], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][18], -1166921985);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][18], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][18], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][18], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][18], 0);
	spTD[playerid][19] = CreatePlayerTextDraw(playerid, 518.0017, 379.0000, "LD_SPAC:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][19], 31.0000, 3.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][19], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][19], -124373249);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][19], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][19], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][19], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][19], 0);
	spTD[playerid][20] = CreatePlayerTextDraw(playerid, 535.9739, 367.9167, "FILL_146");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][20], 0.1151, 1.0808);
	PlayerTextDrawAlignment(playerid, spTD[playerid][20], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][20], -1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][20], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][20], 2);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][20], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][20], 0);
	spTD[playerid][21] = CreatePlayerTextDraw(playerid, 516.0000, 351.0000, "LOCK");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][21], 0.1802, 1.1916);
	PlayerTextDrawAlignment(playerid, spTD[playerid][21], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][21], -459199745);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][21], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][21], 1);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][21], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][21], 0);
	spTD[playerid][22] = CreatePlayerTextDraw(playerid, 538.0000, 351.0000, "ENGINE");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][22], 0.1802, 1.1916);
	PlayerTextDrawAlignment(playerid, spTD[playerid][22], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][22], -1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][22], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][22], 1);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][22], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][22], 0);
	spTD[playerid][23] = CreatePlayerTextDraw(playerid, 566.0000, 351.0000, "LIGHT");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][23], 0.1802, 1.1916);
	PlayerTextDrawAlignment(playerid, spTD[playerid][23], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][23], -1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][23], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][23], 1);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][23], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][23], 0);
	spTD[playerid][24] = CreatePlayerTextDraw(playerid, 576.0000, 386.5000, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][24], 20.0000, 20.5000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][24], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][24], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][24], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][24], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][24], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][24], 0);
	spTD[playerid][25] = CreatePlayerTextDraw(playerid, 589.0000, 391.0000, "KM/H");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][25], 0.1287, 1.1566);
	PlayerTextDrawAlignment(playerid, spTD[playerid][25], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][25], -1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][25], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][25], 2);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][25], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][25], 0);
	spTD[playerid][26] = CreatePlayerTextDraw(playerid, 598.5870, 386.5832, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, spTD[playerid][26], 20.0000, 20.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][26], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][26], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][26], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][26], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][26], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][26], 0);
	spTD[playerid][27] = CreatePlayerTextDraw(playerid, 585.4685, 390.0833, "ld_spac:white");
	PlayerTextDrawTextSize(playerid, spTD[playerid][27], 23.0000, 13.5000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][27], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][27], 336860415);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][27], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][27], 4);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][27], 0);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][27], 0);
	spTD[playerid][28] = CreatePlayerTextDraw(playerid, 597.0000, 364.0000, "255");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][28], 0.2734, 1.5008);
	PlayerTextDrawTextSize(playerid, spTD[playerid][28], -22.0000, 0.0000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][28], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][28], -1);
	PlayerTextDrawSetOutline(playerid, spTD[playerid][28], 1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][28], 134);
	PlayerTextDrawFont(playerid, spTD[playerid][28], 3);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][28], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][28], 0);

	// --- LISIBILITE ---
	// Les tailles du pack etaient calibrees pour faire joli sur la capture de
	// son auteur, pas pour etre lues en conduisant : les chiffres de la vitesse
	// et le libelle d'essence sortaient en police fine sur fond sombre.
	PlayerTextDrawLetterSize(playerid, spTD[playerid][SP2_TD_VITESSE], 0.4200, 2.2000);
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_VITESSE], 0xFFFFFFFF);
	PlayerTextDrawSetOutline(playerid, spTD[playerid][SP2_TD_VITESSE], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][SP2_TD_VITESSE], 0);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][SP2_TD_VITESSE], 0x000000CC);

	PlayerTextDrawLetterSize(playerid, spTD[playerid][SP2_TD_ESSENCE], 0.2000, 1.1000);
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_ESSENCE], 0xFFFFFFFF);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][SP2_TD_ESSENCE], 0x000000CC);

	PlayerTextDrawLetterSize(playerid, spTD[playerid][25], 0.1900, 1.1000);   // KM/H
	PlayerTextDrawColor(playerid, spTD[playerid][25], 0xFFFFFFFF);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][25], 0x000000CC);

	// Temoins moteur / phares / portieres : police trop fine pour etre lue.
	for(new k = SP2_TD_LOCK; k <= SP2_TD_LIGHT; k++)
	{
		PlayerTextDrawLetterSize(playerid, spTD[playerid][k], 0.1900, 1.1000);
		PlayerTextDrawBackgroundColor(playerid, spTD[playerid][k], 0x000000CC);
	}

	// [AJOUT] Etat du vehicule et kilometrage : absents du pack, presents dans
	// l'ancien compteur. Places sous le panneau, dans l'espace libre.
	spTD[playerid][SP2_TD_INFO] = CreatePlayerTextDraw(playerid, 516.0000, 403.5000, "_");
	PlayerTextDrawLetterSize(playerid, spTD[playerid][SP2_TD_INFO], 0.1900, 1.1000);
	PlayerTextDrawAlignment(playerid, spTD[playerid][SP2_TD_INFO], 1);
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_INFO], -1);
	PlayerTextDrawBackgroundColor(playerid, spTD[playerid][SP2_TD_INFO], 255);
	PlayerTextDrawFont(playerid, spTD[playerid][SP2_TD_INFO], 2);
	PlayerTextDrawSetProportional(playerid, spTD[playerid][SP2_TD_INFO], 1);
	PlayerTextDrawSetShadow(playerid, spTD[playerid][SP2_TD_INFO], 0);

	spCree[playerid] = 1;
	return 1;
}

stock Sp2_Detruire(playerid)
{
	if(spCree[playerid] != 1) return 0;
	for(new k = 0; k < SP2_NB_TD; k++) PlayerTextDrawDestroy(playerid, spTD[playerid][k]);
	spCree[playerid]    = 0;
	spVisible[playerid] = 0;
	return 1;
}

// ============================================================================
// MISE A JOUR
// ============================================================================
stock Sp2_Rafraichir(playerid)
{
	if(spVisible[playerid] != 1) return 0;

	new carid = GetPlayerVehicleID(playerid);
	if(carid == 0) return 0;

	new s[64];

	// Vitesse
	format(s, sizeof(s), "%d", GetVehicleSpeed(carid));
	PlayerTextDrawSetString(playerid, spTD[playerid][SP2_TD_VITESSE], s);

	// Apercu 3D du vehicule. SetPreviewModel force le client a recharger le
	// modele : on ne le rappelle que si le vehicule a change, sinon l'apercu
	// clignote trois fois par seconde.
	new model = GetVehicleModel(carid);
	if(model >= 400 && model <= 611 && spDernierModel[playerid] != model)
	{
		spDernierModel[playerid] = model;
		PlayerTextDrawSetPreviewModel(playerid, spTD[playerid][SP2_TD_APERCU], model);
		PlayerTextDrawShow(playerid, spTD[playerid][SP2_TD_APERCU]);
	}

	// Essence. La valeur n'est pas toujours plafonnee a la source (mecano,
	// jerrican) : on borne pour l'affichage sans toucher a la donnee.
	new gas = vehicle[carid][cGas];
	if(gas > 100) gas = 100;
	else if(gas < 0) gas = 0;

	// La barre est une boite : on ne peut pas la deplacer apres creation, mais
	// on peut changer sa largeur.
	new Float:larg = (SP2_JAUGE_W_MAX * float(gas)) / 100.0;
	if(larg < 1.0) larg = 1.0;
	PlayerTextDrawTextSize(playerid, spTD[playerid][SP2_TD_JAUGE], SP2_JAUGE_X + larg, 3.0);
	if(gas > 45)      PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_JAUGE], 0x66FF66FF);
	else if(gas > 15) PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_JAUGE], 0xFF8000FF);
	else              PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_JAUGE], 0xFF0000FF);
	PlayerTextDrawShow(playerid, spTD[playerid][SP2_TD_JAUGE]);

	format(s, sizeof(s), "ESSENCE %d%%", gas);
	PlayerTextDrawSetString(playerid, spTD[playerid][SP2_TD_ESSENCE], s);

	// Moteur, phares, portieres
	new engine, lights, alarm, doors, bonnet, boot, objective;
	GetVehicleParamsEx(carid, engine, lights, alarm, doors, bonnet, boot, objective);
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_ENGINE], (engine == 1) ? (0x66FF66FF) : (0x666666FF));
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_LIGHT],  (lights == 1) ? (0xFFFF66FF) : (0x666666FF));
	PlayerTextDrawColor(playerid, spTD[playerid][SP2_TD_LOCK],   (doors  == 1) ? (0xFF4444FF) : (0x666666FF));
	PlayerTextDrawShow(playerid, spTD[playerid][SP2_TD_ENGINE]);
	PlayerTextDrawShow(playerid, spTD[playerid][SP2_TD_LIGHT]);
	PlayerTextDrawShow(playerid, spTD[playerid][SP2_TD_LOCK]);

	// Etat du vehicule + kilometrage
	new Float:hp;
	GetVehicleHealth(carid, hp);
	new etat = floatround(((hp - 250.0) / 750.0) * 100.0);
	if(etat > 100) etat = 100;
	else if(etat < 0) etat = 0;
	format(s, sizeof(s), "ETAT %d%%   MILES %d.%d", etat, vehicle[carid][cKiloMeter], vehicle[carid][cMeter]);
	PlayerTextDrawSetString(playerid, spTD[playerid][SP2_TD_INFO], s);

	return 1;
}

public Sp2_Tick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(spVisible[i] != 1) continue;
		if(!IsPlayerConnected(i) || GetPlayerState(i) != PLAYER_STATE_DRIVER)
		{
			Sp2_Hide(i);
			continue;
		}
		Sp2_Rafraichir(i);
	}
	return 1;
}

// ============================================================================
// AFFICHER / MASQUER
// ============================================================================
// Renvoie 1 si ce module prend l'affichage en charge : l'appelant doit alors
// s'arreter et ne pas montrer l'ancien compteur par-dessus.
stock Sp2_Show(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(spClassique[playerid] == 1) return 0;

	Sp2_Creer(playerid);
	spDernierModel[playerid] = 0;
	for(new k = 0; k < SP2_NB_TD; k++)
	{
		if(Sp2_Decoratif(k)) continue;
		PlayerTextDrawShow(playerid, spTD[playerid][k]);
	}
	spVisible[playerid] = 1;
	Sp2_Rafraichir(playerid);
	return 1;
}

stock Sp2_Hide(playerid)
{
	if(spCree[playerid] != 1) return 0;
	if(spVisible[playerid] != 1) return 0;
	for(new k = 0; k < SP2_NB_TD; k++) PlayerTextDrawHide(playerid, spTD[playerid][k]);
	spVisible[playerid] = 0;

	// Les 30 elements sont detruits en sortant du vehicule, pas seulement
	// masques. Un client SA-MP n'en accepte que 256 par joueur et le telephone
	// en tient deja 169 en permanence : garder le tableau de bord en reserve
	// pendant qu'on est a pied aurait pu empecher un autre panneau, le sac par
	// exemple, de s'afficher du tout, sans aucun message d'erreur.
	Sp2_Detruire(playerid);
	return 1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Sp2_Cmd(playerid)
{
	new etaitVisible = spVisible[playerid];
	Sp2_Hide(playerid);

	if(spClassique[playerid] == 1)
	{
		spClassique[playerid] = 0;
		SendClientMessage(playerid, 0xFFFFFFFF, "{00FF00}Tableau de bord moderne active.");
		// L'ancien compteur est peut-etre affiche : on le masque avant.
		if(etaitVisible || GetPlayerState(playerid) == PLAYER_STATE_DRIVER) speedometer_Hide(playerid);
		if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER) speedometer_Show(playerid);
	}
	else
	{
		spClassique[playerid] = 1;
		SendClientMessage(playerid, 0xFFFFFFFF, "{FFFF00}Compteur classique active.");
		if(GetPlayerState(playerid) == PLAYER_STATE_DRIVER) speedometer_Show(playerid);
	}
	SendClientMessage(playerid, 0xFFFFFFFF, "{AAAAAA}Le choix vaut pour cette session : il n'est pas encore enregistre sur ton compte.");
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
stock Sp2_OnConnect(playerid)
{
	// Un nouveau joueur herite du slot d'un autre : sans remise a zero il
	// heriterait de sa preference et de ses textdraws.
	spCree[playerid]         = 0;
	spVisible[playerid]      = 0;
	spClassique[playerid]    = 0;
	spDernierModel[playerid] = 0;
	return 1;
}

stock Sp2_OnDisconnect(playerid)
{
	Sp2_Detruire(playerid);
	spClassique[playerid]    = 0;
	spDernierModel[playerid] = 0;
	return 1;
}

stock Sp2_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		spCree[i] = 0; spVisible[i] = 0; spClassique[i] = 0; spDernierModel[i] = 0;
	}
	if(spTimer != -1) KillTimer(spTimer);
	spTimer = SetTimer("Sp2_Tick", SP2_TICK_MS, true);
	return 1;
}
