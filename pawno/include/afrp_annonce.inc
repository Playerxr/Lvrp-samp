/*
	afrp_annonce.inc - Annonce visuelle a l'ecran (textdraw), pas dans le chat

	Le chat defile : une annonce importante y disparait en trois lignes. Ce
	module affiche un bandeau centre a l'ecran, qui s'efface tout seul.

	  Annonce_All("EVENEMENT", "Course de nuit : /ev rejoindre", 0xFFD700FF, 8000);
	  Annonce_Show(playerid, "CONVOI", "Ton convoi est arrive.", 0x00FF00FF, 5000);

	=== POURQUOI UN COMPTEUR DE GENERATION ===
	Chaque annonce arme un timer d'effacement. Si une seconde annonce arrive
	avant la fin de la premiere, le timer de la premiere effacerait la seconde
	en plein milieu. Chaque annonce porte donc un numero, et un timer n'efface
	que si le numero correspond toujours. Sans ca, deux annonces rapprochees
	donnent un bandeau qui clignote et disparait trop tot.

	=== PLACEMENT ===
	Bandeau centre en X (170..470), sous le viseur, a Y 132..170. Il ne touche
	ni le panneau du top du jour (X 500, Y 26..71), ni celui des objectifs
	(X 496, Y 125), ni le bloc vie/faim passe en bas a gauche.

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect    -> Annonce_OnConnect(playerid)
	- OnPlayerDisconnect -> Annonce_OnDisconnect(playerid)
*/

#if defined AFRP_ANNONCE_INCLUDED
	#endinput
#endif
#define AFRP_ANNONCE_INCLUDED

#define ANN_BOX_X1     170.0
#define ANN_BOX_X2     470.0
#define ANN_BOX_Y      132.0
#define ANN_CENTRE_X   320.0
#define ANN_TITRE_Y    135.0
#define ANN_TEXTE_Y    152.0
#define ANN_DUREE_DEF  7000        // duree par defaut en millisecondes

// --- Bandeau PERMANENT (haut de l'ecran) ---
// Different du bandeau du milieu : celui-ci n'a pas de minuteur, il reste
// affiche tant qu'on ne l'efface pas explicitement. Sert a signaler ce qui
// est EN COURS et qu'on peut encore aller chercher (event du monde, etc.).
// Place en Y 4..20 : au-dessus de la premiere ligne de chat (~Y 22), a droite
// du logo AFRP (X 5) et a gauche de la barre d'endurance (X 542).
#define ANNP_BOX_X1    180.0
#define ANNP_BOX_X2    460.0
#define ANNP_BOX_Y     4.0
#define ANNP_TXT_Y     5.0

// Plusieurs systemes peuvent vouloir ce bandeau en meme temps (un braquage
// pendant un event du monde, par exemple). Chacun a son emplacement : sans ca,
// le premier qui termine effacerait le bandeau de l'autre, encore actif.
// L'emplacement le plus prioritaire (indice le plus bas) est celui affiche.
#define ANNP_SRC_BRAQUAGE  0
#define ANNP_SRC_EVENT     1
#define ANNP_SRC_LIBRE     2
#define ANNP_MAX_SRC       3

new PlayerText:annPermBox[MAX_PLAYERS];
new PlayerText:annPermTxt[MAX_PLAYERS];
new annPermOn[ANNP_MAX_SRC];
new annPermTxtSrc[ANNP_MAX_SRC][144];
new annPermColSrc[ANNP_MAX_SRC];
new annPermActif = 0;                  // 1 = au moins une source est active
new annPermTexte[144];                 // texte reellement affiche
new annPermCouleur = 0xFFD700FF;

new PlayerText:annBox[MAX_PLAYERS];
new PlayerText:annTitre[MAX_PLAYERS];
new PlayerText:annTexte[MAX_PLAYERS];
new annGen[MAX_PLAYERS];
new annVisible[MAX_PLAYERS];

stock Annonce_Create(playerid)
{
	if(annBox[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	// Fond : aligne a gauche avec une largeur explicite, plus previsible qu'une
	// boite centree dont SA-MP calcule mal les bords.
	annBox[playerid] = CreatePlayerTextDraw(playerid, ANN_BOX_X1, ANN_BOX_Y, "_");
	PlayerTextDrawFont(playerid, annBox[playerid], 1);
	PlayerTextDrawLetterSize(playerid, annBox[playerid], 0.0, 3.6);
	PlayerTextDrawColor(playerid, annBox[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, annBox[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, annBox[playerid], ANN_BOX_X2, 0.0);
	PlayerTextDrawUseBox(playerid, annBox[playerid], 1);
	PlayerTextDrawBoxColor(playerid, annBox[playerid], 0x000000A0);

	// Titre : centre (alignement 2) sur le milieu de l'ecran.
	annTitre[playerid] = CreatePlayerTextDraw(playerid, ANN_CENTRE_X, ANN_TITRE_Y, " ");
	PlayerTextDrawFont(playerid, annTitre[playerid], 1);
	PlayerTextDrawAlignment(playerid, annTitre[playerid], 2);
	PlayerTextDrawLetterSize(playerid, annTitre[playerid], 0.34, 1.5);
	PlayerTextDrawColor(playerid, annTitre[playerid], 0xFFD700FF);
	PlayerTextDrawSetShadow(playerid, annTitre[playerid], 0);
	PlayerTextDrawSetOutline(playerid, annTitre[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, annTitre[playerid], 0x000000FF);
	PlayerTextDrawSetProportional(playerid, annTitre[playerid], 1);

	annTexte[playerid] = CreatePlayerTextDraw(playerid, ANN_CENTRE_X, ANN_TEXTE_Y, " ");
	PlayerTextDrawFont(playerid, annTexte[playerid], 1);
	PlayerTextDrawAlignment(playerid, annTexte[playerid], 2);
	PlayerTextDrawLetterSize(playerid, annTexte[playerid], 0.22, 1.1);
	PlayerTextDrawColor(playerid, annTexte[playerid], 0xFFFFFFFF);
	PlayerTextDrawSetShadow(playerid, annTexte[playerid], 0);
	PlayerTextDrawSetOutline(playerid, annTexte[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, annTexte[playerid], 0x000000FF);
	PlayerTextDrawSetProportional(playerid, annTexte[playerid], 1);

	annVisible[playerid] = 0;
	return 1;
}

stock Annonce_HideNow(playerid)
{
	if(annBox[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;
	PlayerTextDrawHide(playerid, annBox[playerid]);
	PlayerTextDrawHide(playerid, annTitre[playerid]);
	PlayerTextDrawHide(playerid, annTexte[playerid]);
	annVisible[playerid] = 0;
	return 1;
}

// N'efface que si aucune annonce plus recente n'a pris la place (voir en-tete).
forward Annonce_Expire(playerid, gen);
public Annonce_Expire(playerid, gen)
{
	if(!IsPlayerConnected(playerid)) return 1;
	if(annGen[playerid] != gen) return 1;
	Annonce_HideNow(playerid);
	return 1;
}

// Pas de 'const' sur titre/texte : le natif PlayerTextDrawSetString attend une
// chaine modifiable et refuse un tableau const (erreur 035).
stock Annonce_Show(playerid, titre[], texte[], couleur = 0xFFD700FF, duree = ANN_DUREE_DEF)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	Annonce_Create(playerid);

	PlayerTextDrawSetString(playerid, annTitre[playerid], titre);
	PlayerTextDrawSetString(playerid, annTexte[playerid], texte);
	PlayerTextDrawColor(playerid, annTitre[playerid], couleur);

	PlayerTextDrawShow(playerid, annBox[playerid]);
	PlayerTextDrawShow(playerid, annTitre[playerid]);
	PlayerTextDrawShow(playerid, annTexte[playerid]);
	annVisible[playerid] = 1;

	annGen[playerid]++;
	SetTimerEx("Annonce_Expire", duree, false, "ii", playerid, annGen[playerid]);
	return 1;
}

stock Annonce_All(titre[], texte[], couleur = 0xFFD700FF, duree = ANN_DUREE_DEF)
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		Annonce_Show(i, titre, texte, couleur, duree);
		n++;
	}
	return n;
}

// ============================================================================
// BANDEAU PERMANENT
// ============================================================================
stock Annonce_PermCreate(playerid)
{
	if(annPermBox[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	annPermBox[playerid] = CreatePlayerTextDraw(playerid, ANNP_BOX_X1, ANNP_BOX_Y, "_");
	PlayerTextDrawFont(playerid, annPermBox[playerid], 1);
	PlayerTextDrawLetterSize(playerid, annPermBox[playerid], 0.0, 1.6);
	PlayerTextDrawColor(playerid, annPermBox[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, annPermBox[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, annPermBox[playerid], ANNP_BOX_X2, 0.0);
	PlayerTextDrawUseBox(playerid, annPermBox[playerid], 1);
	PlayerTextDrawBoxColor(playerid, annPermBox[playerid], 0x000000A0);

	annPermTxt[playerid] = CreatePlayerTextDraw(playerid, 320.0, ANNP_TXT_Y, " ");
	PlayerTextDrawFont(playerid, annPermTxt[playerid], 1);
	PlayerTextDrawAlignment(playerid, annPermTxt[playerid], 2);
	PlayerTextDrawLetterSize(playerid, annPermTxt[playerid], 0.24, 1.2);
	PlayerTextDrawColor(playerid, annPermTxt[playerid], annPermCouleur);
	PlayerTextDrawSetShadow(playerid, annPermTxt[playerid], 0);
	PlayerTextDrawSetOutline(playerid, annPermTxt[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, annPermTxt[playerid], 0x000000FF);
	PlayerTextDrawSetProportional(playerid, annPermTxt[playerid], 1);
	return 1;
}

stock Annonce_PermShow(playerid)
{
	if(annPermActif != 1) return 0;
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	Annonce_PermCreate(playerid);
	PlayerTextDrawSetString(playerid, annPermTxt[playerid], annPermTexte);
	PlayerTextDrawColor(playerid, annPermTxt[playerid], annPermCouleur);
	PlayerTextDrawShow(playerid, annPermBox[playerid]);
	PlayerTextDrawShow(playerid, annPermTxt[playerid]);
	return 1;
}

stock Annonce_PermHide(playerid)
{
	if(annPermBox[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;
	PlayerTextDrawHide(playerid, annPermBox[playerid]);
	PlayerTextDrawHide(playerid, annPermTxt[playerid]);
	return 1;
}

// Recalcule ce qui doit etre affiche et le pousse a tout le monde.
stock Annonce_PermRefresh()
{
	new src = -1;
	for(new k = 0; k < ANNP_MAX_SRC; k++)
		if(annPermOn[k] == 1) { src = k; break; }

	if(src == -1)
	{
		annPermActif = 0;
		annPermTexte[0] = 0;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			Annonce_PermHide(i);
		}
		return 1;
	}

	annPermActif   = 1;
	annPermCouleur = annPermColSrc[src];
	format(annPermTexte, sizeof(annPermTexte), "%s", annPermTxtSrc[src]);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		Annonce_PermShow(i);
	}
	return 1;
}

// src : ANNP_SRC_BRAQUAGE, ANNP_SRC_EVENT ou ANNP_SRC_LIBRE.
stock Annonce_PermSet(src, texte[], couleur = 0xFFD700FF)
{
	if(src < 0 || src >= ANNP_MAX_SRC) return 0;
	format(annPermTxtSrc[src], 144, "%s", texte);
	annPermColSrc[src] = couleur;
	annPermOn[src]     = 1;
	return Annonce_PermRefresh();
}

// N'efface QUE sa propre source : un systeme ne peut pas effacer le bandeau
// d'un autre. Si une autre source reste active, elle reprend l'affichage.
stock Annonce_PermClear(src)
{
	if(src < 0 || src >= ANNP_MAX_SRC) return 0;
	if(annPermOn[src] == 0) return 0;
	annPermOn[src] = 0;
	annPermTxtSrc[src][0] = 0;
	return Annonce_PermRefresh();
}

// Un joueur qui arrive alors qu'un event est deja en cours doit le voir : sans
// ca, seuls les joueurs presents au declenchement auraient le bandeau.
stock Annonce_OnSpawn(playerid)
{
	return Annonce_PermShow(playerid);
}

stock Annonce_OnConnect(playerid)
{
	annPermBox[playerid] = PlayerText:INVALID_TEXT_DRAW;
	annPermTxt[playerid] = PlayerText:INVALID_TEXT_DRAW;
	annBox[playerid]     = PlayerText:INVALID_TEXT_DRAW;
	annTitre[playerid]   = PlayerText:INVALID_TEXT_DRAW;
	annTexte[playerid]   = PlayerText:INVALID_TEXT_DRAW;
	annGen[playerid]     = 0;
	annVisible[playerid] = 0;
	return 1;
}

stock Annonce_OnDisconnect(playerid)
{
	if(annBox[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, annBox[playerid]);
		PlayerTextDrawDestroy(playerid, annTitre[playerid]);
		PlayerTextDrawDestroy(playerid, annTexte[playerid]);
	}
	if(annPermBox[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, annPermBox[playerid]);
		PlayerTextDrawDestroy(playerid, annPermTxt[playerid]);
	}
	Annonce_OnConnect(playerid);
	return 1;
}
