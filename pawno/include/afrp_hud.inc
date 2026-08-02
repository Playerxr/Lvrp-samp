/*
	afrp_hud.inc - HUD joueur : panneau Vie / Armure / Faim / Soif + ping

	A inclure dans LVRP.pwn apres PlayerInfo (lit pFaim et pSoif).

	=== LAYOUT (depuis 2026-08) ===
	Le pack fourni par la proprietaire remplace l'ancien bloc barres+textes.
	Deux plaques arrondies en bas a gauche, a droite du radar (X 142..223) :

	     [ (+) coeur ][ bouclier ][ burger ][ bouteille ]   <- pastilles, Y 389
	     [   98%     ][    0%    ][   92%  ][    87%    ]   <- pourcentages, Y 420

	Chaque pastille est un carre de couleur (indices 12/14/15/16) recouvert
	d'un cache sombre (13/17/18/19). On retrecit le cache : la couleur
	reapparait proportionnellement a la stat, c'est la jauge. Meme technique
	de mise a l'echelle que l'ancienne barre (largeur * valeur / 100).

	Le radar de GTA occupe environ X 25..135 : rien n'est masque.

	Ce module remplace les 7 anciennes TextDraws du HUD ET les 6 TextDraws
	d'etat de LVRP.pwn (TextVie / TextArmure / TextFaim / TextSoif /
	BarUpdateFaim / BarUpdateSoif). UpdateStatsTextsPlayers appelle desormais
	HUD_Update et ne gere plus que la logique metier (blessure, gilet).
	Endurance et Maladie gardent leurs TextDraws dans LVRP.pwn : elles y sont
	creees mais n'ont jamais ete affichees, rien n'est donc perdu ici.

	Le ping reste inchange, en haut a droite, hors du panneau.

	Hooks a brancher :
	- OnPlayerConnect    -> HUD_OnConnect(playerid)
	- OnPlayerDisconnect -> HUD_OnDisconnect(playerid)
	- OnPlayerSpawn      -> HUD_OnSpawn(playerid)  (re-show apres spawn)
	- Optionnel : commande /hud pour toggle
*/

#if defined AFRP_HUD_INCLUDED
	#endinput
#endif
#define AFRP_HUD_INCLUDED

#define HUD_TD_COUNT     43

// Pastilles : carre de couleur 19.5x24, cache sombre 17.5x22 pose dessus.
// Retrecir le cache = remplir la jauge.
#define HUD_FILL_W_MAX   17.5
#define HUD_FILL_H       22.0

// Index des caches sombres et des pourcentages, dans l'ordre Vie/Armure/Faim/Soif.
new hudFillIdx[4] = {13, 17, 18, 19};
new hudPctIdx[4]  = {39, 40, 41, 42};

// --- Ping (inchange, haut droite) ---
#define HUD_PING_X       540.0
#define HUD_PING_Y       100.0

new PlayerText:hudTD[MAX_PLAYERS][HUD_TD_COUNT];
new PlayerText:hudPing[MAX_PLAYERS];
new hudVisible[MAX_PLAYERS];

stock HUD_Create(playerid)
{
	if(hudTD[playerid][0] != PlayerText:INVALID_TEXT_DRAW) return 1;

	// === Pack fourni par la proprietaire (43 elements) ===
	// Les PlayerTextDrawShow d'origine ont ete retires : l'affichage passe
	// par HUD_Show / HUD_Hide pour que /hud et /trailer puissent le cacher.
	hudTD[playerid][0] = CreatePlayerTextDraw(playerid, 142.000000, 418.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][0], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][0], 13.500000, 17.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][0], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][0], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][0], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][0], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][0], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][0], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][0], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][0], 0);

	hudTD[playerid][1] = CreatePlayerTextDraw(playerid, 210.000000, 414.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][1], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][1], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][1], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][1], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][1], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][1], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][1], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][1], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][1], 0);

	hudTD[playerid][2] = CreatePlayerTextDraw(playerid, 210.000000, 418.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][2], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][2], 13.500000, 17.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][2], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][2], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][2], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][2], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][2], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][2], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][2], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][2], 0);

	hudTD[playerid][3] = CreatePlayerTextDraw(playerid, 210.000000, 416.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][3], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][3], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][3], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][3], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][3], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][3], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][3], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][3], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][3], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][3], 0);

	hudTD[playerid][4] = CreatePlayerTextDraw(playerid, 142.000000, 416.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][4], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][4], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][4], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][4], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][4], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][4], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][4], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][4], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][4], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][4], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][4], 0);

	hudTD[playerid][5] = CreatePlayerTextDraw(playerid, 149.000000, 390.398010, "ld_dual:white");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][5], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][5], 67.000000, 22.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][5], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][5], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][5], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][5], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][5], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][5], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][5], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][5], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][5], 0);

	hudTD[playerid][6] = CreatePlayerTextDraw(playerid, 142.000000, 387.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][6], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][6], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][6], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][6], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][6], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][6], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][6], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][6], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][6], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][6], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][6], 0);

	hudTD[playerid][7] = CreatePlayerTextDraw(playerid, 142.000000, 397.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][7], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][7], 13.500000, 18.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][7], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][7], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][7], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][7], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][7], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][7], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][7], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][7], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][7], 0);

	hudTD[playerid][8] = CreatePlayerTextDraw(playerid, 210.000000, 387.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][8], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][8], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][8], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][8], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][8], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][8], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][8], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][8], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][8], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][8], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][8], 0);

	hudTD[playerid][9] = CreatePlayerTextDraw(playerid, 210.000000, 397.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][9], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][9], 13.500000, 18.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][9], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][9], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][9], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][9], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][9], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][9], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][9], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][9], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][9], 0);

	hudTD[playerid][10] = CreatePlayerTextDraw(playerid, 144.098007, 395.398010, "ld_dual:white");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][10], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][10], 77.000000, 11.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][10], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][10], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][10], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][10], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][10], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][10], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][10], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][10], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][10], 0);

	hudTD[playerid][11] = CreatePlayerTextDraw(playerid, 142.000000, 414.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][11], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][11], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][11], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][11], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][11], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][11], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][11], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][11], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][11], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][11], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][11], 0);

	hudTD[playerid][12] = CreatePlayerTextDraw(playerid, 146.000000, 389.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][12], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][12], 19.500000, 24.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][12], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][12], 0x800000FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][12], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][12], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][12], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][12], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][12], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][12], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][12], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][12], 0);

	hudTD[playerid][13] = CreatePlayerTextDraw(playerid, 147.000000, 390.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][13], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][13], 17.500000, 22.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][13], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][13], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][13], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][13], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][13], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][13], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][13], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][13], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][13], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][13], 0);

	hudTD[playerid][14] = CreatePlayerTextDraw(playerid, 164.000000, 389.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][14], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][14], 19.500000, 24.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][14], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][14], 0xFFFFFFFF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][14], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][14], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][14], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][14], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][14], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][14], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][14], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][14], 0);

	hudTD[playerid][15] = CreatePlayerTextDraw(playerid, 181.000000, 389.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][15], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][15], 19.500000, 24.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][15], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][15], 0xFF7F50FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][15], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][15], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][15], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][15], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][15], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][15], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][15], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][15], 0);

	hudTD[playerid][16] = CreatePlayerTextDraw(playerid, 199.000000, 389.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][16], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][16], 19.500000, 24.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][16], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][16], 0x1E90FFFF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][16], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][16], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][16], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][16], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][16], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][16], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][16], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][16], 0);

	hudTD[playerid][17] = CreatePlayerTextDraw(playerid, 165.000000, 390.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][17], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][17], 17.500000, 22.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][17], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][17], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][17], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][17], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][17], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][17], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][17], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][17], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][17], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][17], 0);

	hudTD[playerid][18] = CreatePlayerTextDraw(playerid, 182.000000, 390.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][18], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][18], 17.500000, 22.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][18], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][18], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][18], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][18], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][18], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][18], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][18], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][18], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][18], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][18], 0);

	hudTD[playerid][19] = CreatePlayerTextDraw(playerid, 200.000000, 390.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][19], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][19], 17.500000, 22.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][19], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][19], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][19], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][19], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][19], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][19], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][19], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][19], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][19], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][19], 0);

	hudTD[playerid][20] = CreatePlayerTextDraw(playerid, 174.500000, 391.000000, "-");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][20], 0.128000, 1.699000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][20], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][20], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][20], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][20], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][20], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][20], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][20], 0x00000096);
	PlayerTextDrawFont(playerid, hudTD[playerid][20], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][20], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][20], 0);

	hudTD[playerid][21] = CreatePlayerTextDraw(playerid, 171.500000, 378.000000, "-");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][21], 0.140000, 4.098000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][21], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][21], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][21], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][21], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][21], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][21], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][21], 0x00000096);
	PlayerTextDrawFont(playerid, hudTD[playerid][21], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][21], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][21], 0);

	hudTD[playerid][22] = CreatePlayerTextDraw(playerid, 206.500000, 397.000000, "LD_BEAT:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][22], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][22], 4.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][22], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][22], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][22], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][22], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][22], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][22], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][22], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][22], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][22], 0);

	hudTD[playerid][23] = CreatePlayerTextDraw(playerid, 153.500000, 395.000000, "+");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][23], 0.259000, 1.398000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][23], 1282.000000, 1283.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][23], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][23], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][23], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][23], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][23], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][23], 0x00000096);
	PlayerTextDrawFont(playerid, hudTD[playerid][23], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][23], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][23], 0);

	hudTD[playerid][24] = CreatePlayerTextDraw(playerid, 205.500000, 399.000000, "LD_BEAT:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][24], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][24], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][24], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][24], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][24], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][24], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][24], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][24], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][24], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][24], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][24], 0);

	hudTD[playerid][25] = CreatePlayerTextDraw(playerid, 207.500000, 396.000000, "LD_BEAT:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][25], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][25], 2.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][25], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][25], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][25], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][25], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][25], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][25], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][25], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][25], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][25], 0);

	hudTD[playerid][26] = CreatePlayerTextDraw(playerid, 205.500000, 402.000000, "LD_BEAT:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][26], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][26], 6.000000, 4.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][26], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][26], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][26], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][26], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][26], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][26], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][26], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][26], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][26], 0);

	hudTD[playerid][27] = CreatePlayerTextDraw(playerid, 187.000000, 397.000000, "LD_BEAT:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][27], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][27], 7.000000, 5.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][27], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][27], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][27], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][27], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][27], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][27], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][27], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][27], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][27], 0);

	hudTD[playerid][28] = CreatePlayerTextDraw(playerid, 188.000000, 399.000000, "LD_BUM:blkdot");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][28], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][28], 5.000000, 2.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][28], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][28], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][28], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][28], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][28], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][28], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][28], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][28], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][28], 0);

	hudTD[playerid][29] = CreatePlayerTextDraw(playerid, 188.000000, 402.000000, "LD_BUM:blkdot");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][29], 0.479000, 1.120000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][29], 5.000000, 2.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][29], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][29], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][29], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][29], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][29], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][29], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][29], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][29], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][29], 0);

	hudTD[playerid][30] = CreatePlayerTextDraw(playerid, 149.000000, 417.398010, "ld_dual:white");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][30], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][30], 67.000000, 15.500000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][30], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][30], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][30], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][30], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][30], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][30], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][30], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][30], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][30], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][30], 0);

	hudTD[playerid][31] = CreatePlayerTextDraw(playerid, 142.000000, 414.500000, "ld_beat:chit");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][31], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][31], 13.500000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][31], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][31], 0x121212FF);
	PlayerTextDrawUseBox(playerid, hudTD[playerid][31], 1);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][31], 0x00000032);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][31], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][31], 1);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][31], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][31], 4);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][31], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][31], 0);

	hudTD[playerid][32] = CreatePlayerTextDraw(playerid, 176.500000, 397.000000, "/");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][32], -0.216000, 0.298000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][32], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][32], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][32], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][32], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][32], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][32], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][32], 0x000000C8);
	PlayerTextDrawFont(playerid, hudTD[playerid][32], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][32], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][32], 0);

	hudTD[playerid][33] = CreatePlayerTextDraw(playerid, 173.500000, 401.000000, "/");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][33], -0.165000, 0.497000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][33], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][33], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][33], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][33], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][33], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][33], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][33], 0x000000C8);
	PlayerTextDrawFont(playerid, hudTD[playerid][33], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][33], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][33], 0);

	hudTD[playerid][34] = CreatePlayerTextDraw(playerid, 170.500000, 397.000000, "/");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][34], 0.221000, 0.298000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][34], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][34], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][34], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][34], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][34], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][34], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][34], 0x00000096);
	PlayerTextDrawFont(playerid, hudTD[playerid][34], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][34], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][34], 0);

	hudTD[playerid][35] = CreatePlayerTextDraw(playerid, 173.500000, 401.000000, "/");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][35], 0.170000, 0.497000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][35], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][35], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][35], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][35], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][35], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][35], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][35], 0x000000C8);
	PlayerTextDrawFont(playerid, hudTD[playerid][35], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][35], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][35], 0);

	hudTD[playerid][36] = CreatePlayerTextDraw(playerid, 174.500000, 393.000000, "-");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][36], -0.136000, 0.898000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][36], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][36], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][36], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][36], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][36], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][36], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][36], 0x000000C8);
	PlayerTextDrawFont(playerid, hudTD[playerid][36], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][36], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][36], 0);

	hudTD[playerid][37] = CreatePlayerTextDraw(playerid, 174.500000, 400.000000, "-");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][37], -0.136000, 0.898000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][37], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][37], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][37], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][37], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][37], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][37], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][37], 0x000000C8);
	PlayerTextDrawFont(playerid, hudTD[playerid][37], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][37], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][37], 0);

	hudTD[playerid][38] = CreatePlayerTextDraw(playerid, 175.500000, 391.000000, "-");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][38], -0.128000, 1.699000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][38], 1280.000000, 1280.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][38], 1);
	PlayerTextDrawColor(playerid, hudTD[playerid][38], 0xFFFFFFC8);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][38], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][38], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][38], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][38], 0x00000096);
	PlayerTextDrawFont(playerid, hudTD[playerid][38], 1);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][38], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][38], 0);

	hudTD[playerid][39] = CreatePlayerTextDraw(playerid, 156.000000, 420.000000, "98%");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][39], 0.119000, 1.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][39], 400.000000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][39], 2);
	PlayerTextDrawColor(playerid, hudTD[playerid][39], 0xFFFFFFFF);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][39], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][39], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][39], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][39], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][39], 2);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][39], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][39], 0);

	hudTD[playerid][40] = CreatePlayerTextDraw(playerid, 174.000000, 420.000000, "0%");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][40], 0.119000, 1.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][40], 400.000000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][40], 2);
	PlayerTextDrawColor(playerid, hudTD[playerid][40], 0xFFFFFFFF);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][40], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][40], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][40], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][40], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][40], 2);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][40], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][40], 0);

	hudTD[playerid][41] = CreatePlayerTextDraw(playerid, 191.000000, 420.000000, "92%");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][41], 0.119000, 1.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][41], 400.000000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][41], 2);
	PlayerTextDrawColor(playerid, hudTD[playerid][41], 0xFFFFFFFF);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][41], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][41], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][41], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][41], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][41], 2);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][41], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][41], 0);

	hudTD[playerid][42] = CreatePlayerTextDraw(playerid, 209.000000, 420.000000, "87%");
	PlayerTextDrawLetterSize(playerid, hudTD[playerid][42], 0.119000, 1.000000);
	PlayerTextDrawTextSize(playerid, hudTD[playerid][42], 400.000000, 17.000000);
	PlayerTextDrawAlignment(playerid, hudTD[playerid][42], 2);
	PlayerTextDrawColor(playerid, hudTD[playerid][42], 0xFFFFFFFF);
	PlayerTextDrawBoxColor(playerid, hudTD[playerid][42], 0x80808080);
	PlayerTextDrawSetShadow(playerid, hudTD[playerid][42], 0);
	PlayerTextDrawSetOutline(playerid, hudTD[playerid][42], 0);
	PlayerTextDrawBackgroundColor(playerid, hudTD[playerid][42], 0x000000FF);
	PlayerTextDrawFont(playerid, hudTD[playerid][42], 2);
	PlayerTextDrawSetProportional(playerid, hudTD[playerid][42], 1);
	PlayerTextDrawSetSelectable(playerid, hudTD[playerid][42], 0);

	// === Ping (hors panneau) ===
	hudPing[playerid] = CreatePlayerTextDraw(playerid, HUD_PING_X, HUD_PING_Y, "Ping: ---");
	PlayerTextDrawFont(playerid, hudPing[playerid], 2);
	PlayerTextDrawLetterSize(playerid, hudPing[playerid], 0.20, 1.0);
	PlayerTextDrawColor(playerid, hudPing[playerid], 0x00FF00FF);
	PlayerTextDrawSetShadow(playerid, hudPing[playerid], 1);
	PlayerTextDrawSetOutline(playerid, hudPing[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, hudPing[playerid], 0x000000FF);

	return 1;
}

// Une seule jauge : cache sombre retreci a proportion inverse de la valeur.
stock HUD_SetGauge(playerid, slot, Float:valeur)
{
	if(valeur < 0.0) valeur = 0.0;
	if(valeur > 100.0) valeur = 100.0;

	new Float:cache = HUD_FILL_W_MAX * (1.0 - (valeur / 100.0));
	if(cache < 0.5) cache = 0.5; // 0 exact fait disparaitre le sprite

	PlayerTextDrawTextSize(playerid, hudTD[playerid][hudFillIdx[slot]], cache, HUD_FILL_H);
	PlayerTextDrawShow(playerid, hudTD[playerid][hudFillIdx[slot]]);

	new pct[8];
	format(pct, sizeof(pct), "%d%%", floatround(valeur));
	PlayerTextDrawSetString(playerid, hudTD[playerid][hudPctIdx[slot]], pct);
	return 1;
}

// Rafraichit les 4 jauges. Appele par le timer 1s ET par
// UpdateStatsTextsPlayers, pour que faim/soif reagissent immediatement.
stock HUD_Update(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(hudVisible[playerid] != 1) return 0;
	if(hudTD[playerid][0] == PlayerText:INVALID_TEXT_DRAW) return 0;

	new Float:hp, Float:ar;
	GetPlayerHealth(playerid, hp);
	GetPlayerArmour(playerid, ar);

	HUD_SetGauge(playerid, 0, hp);
	HUD_SetGauge(playerid, 1, ar);
	HUD_SetGauge(playerid, 2, float(PlayerInfo[playerid][pFaim]));
	HUD_SetGauge(playerid, 3, float(PlayerInfo[playerid][pSoif]));

	// Ping : update avec couleur selon valeur
	new ping = GetPlayerPing(playerid);
	new pingStr[20];
	format(pingStr, sizeof(pingStr), "Ping: %d", ping);
	PlayerTextDrawSetString(playerid, hudPing[playerid], pingStr);
	new pingColor;
	if(ping < 100)      pingColor = 0x00FF00FF;
	else if(ping < 200) pingColor = 0xFFAA00FF;
	else                pingColor = 0xFF3333FF;
	PlayerTextDrawColor(playerid, hudPing[playerid], pingColor);
	PlayerTextDrawShow(playerid, hudPing[playerid]);

	return 1;
}

stock HUD_Show(playerid)
{
	HUD_Create(playerid);
	for(new k = 0; k < HUD_TD_COUNT; k++) PlayerTextDrawShow(playerid, hudTD[playerid][k]);
	PlayerTextDrawShow(playerid, hudPing[playerid]);
	hudVisible[playerid] = 1;
	// Sinon le panneau reste sur les valeurs du pack (98% / 0% / 92% / 87%)
	// jusqu'au prochain changement de stat.
	HUD_Update(playerid);
	return 1;
}

stock HUD_Hide(playerid)
{
	if(hudTD[playerid][0] == PlayerText:INVALID_TEXT_DRAW) return 0;
	for(new k = 0; k < HUD_TD_COUNT; k++) PlayerTextDrawHide(playerid, hudTD[playerid][k]);
	PlayerTextDrawHide(playerid, hudPing[playerid]);
	hudVisible[playerid] = 0;
	return 1;
}

stock HUD_OnConnect(playerid)
{
	for(new k = 0; k < HUD_TD_COUNT; k++) hudTD[playerid][k] = PlayerText:INVALID_TEXT_DRAW;
	hudPing[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudVisible[playerid] = 0;
	return 1;
}

stock HUD_OnDisconnect(playerid)
{
	for(new k = 0; k < HUD_TD_COUNT; k++)
	{
		if(hudTD[playerid][k] == PlayerText:INVALID_TEXT_DRAW) continue;
		PlayerTextDrawDestroy(playerid, hudTD[playerid][k]);
	}
	if(hudPing[playerid] != PlayerText:INVALID_TEXT_DRAW)
		PlayerTextDrawDestroy(playerid, hudPing[playerid]);
	HUD_OnConnect(playerid); // reset les vars
	return 1;
}

stock HUD_OnSpawn(playerid)
{
	HUD_Show(playerid);
	return 1;
}

// Timer global qui refresh tous les HUD une fois par seconde
forward HUD_GlobalTick();
public HUD_GlobalTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		HUD_Update(i);
	}
	return 1;
}

stock HUD_Init()
{
	SetTimer("HUD_GlobalTick", 1000, true);
	return 1;
}

// Toggle commande /hud
stock HUD_CmdToggle(playerid)
{
	if(hudVisible[playerid] == 1)
	{
		HUD_Hide(playerid);
		SendClientMessage(playerid, 0xAAAAAAFF, "{AAAAAA}[HUD]{FFFFFF} Cache. /hud pour reafficher.");
	}
	else
	{
		HUD_Show(playerid);
		SendClientMessage(playerid, 0xAAAAAAFF, "{AAAAAA}[HUD]{FFFFFF} Affiche.");
	}
	return 1;
}
