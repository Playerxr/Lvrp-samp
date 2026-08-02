/*
	afrp_prison.inc - Prison : vraie map interieure (fournie par le proprio)
	+ enceinte fixe (legacy, dormante) + createur de prison custom + barreaux

	[VRAIE PRISON, aout 2026] Le proprietaire a fourni une map interieure
	faite a la main (525 objets, voir afrp_prison_map.inc) pour remplacer
	l'ancienne enceinte a ciel ouvert generee proceduralement. C'est
	desormais CA la vraie prison : /a jail y teleporte le joueur (monde
	PRISON_MAP_WORLD / interieur PRISON_MAP_INTERIOR, voir afrp_prison_map.inc),
	Prison_CreateMap() la construit une fois au demarrage (OnGameModeInit).

	[POURQUOI GARDER LE RESTE] Rien ci-dessous n'est plus le chemin par
	defaut de /a jail, mais rien n'a ete supprime : le proprietaire n'a pas
	demande a perdre ces outils, seulement a utiliser la vraie map.
	1) ENCEINTE FIXE (Prison_CreateCompound) : mur/miradors/portail autour du
	   point "Prison acceuil" deja existant dans init_Mapping(). Deja
	   desactivee (appel commente dans OnGameModeInit) AVANT ce changement -
	   code mort laisse en place tel quel, rien touche ici.
	2) CREATEUR CUSTOM (/a creer prison) : construit la MEME enceinte centree
	   sur la position de l'admin + orientee selon l'angle ou il regarde.
	   Relancer la commande detruit l'ancienne prison custom et reconstruit.
	   Reste utilisable tel quel (ex: prison RP secondaire, evenement) mais
	   /a jail ne s'y teleporte plus automatiquement (voir Prison_GetCellPos).
	3) BARREAUX (/a barreau) : grille de cellule (kb_cagedoor) posee a la
	   position de l'admin pour fermer une cellule piece par piece. Toujours
	   utile pour amenager l'enceinte custom ci-dessus.

	[FIX HAUTEUR] Le modele tall_fence (974) a son pivot proche du CENTRE du
	mesh, pas a sa base -> place "a plat" sur le sol (Z du joueur/ancre), il
	semble flotter au-dessus du sol. FENCE_Z_OFFSET le redescend pour que sa
	base touche reellement le sol.
	[MUR INFRANCHISSABLE] Chaque "panneau" de mur est en fait 2 grillages
	empiles (rez-de-sol + un autre par-dessus) -> hauteur doublee, impossible
	de sauter par-dessus en RP normal.
	[GRANDE PRISON] Dimensions doublees par rapport a la version precedente.
*/

#if defined AFRP_PRISON_INCLUDED
	#endinput
#endif
#define AFRP_PRISON_INCLUDED

// La vraie map (objets + PRISON_MAP_WORLD/PRISON_MAP_INTERIOR).
#include <afrp_prison_map>

// Point d'ancrage de l'enceinte FIXE : identique a l'acteur "Prison acceuil"
// deja place dans init_Mapping() (CreateDynamicLvrpActor(71, 919.7765, -2370.7874, 13.2474, ...))
#define PRISON_ANCHOR_X    919.7765
#define PRISON_ANCHOR_Y    -2370.7874
#define PRISON_ANCHOR_Z    13.2474

// [FIX HAUTEUR] Negatif = redescend le grillage pour que sa base touche le sol
// (le pivot du modele 974 est proche du centre du mesh, pas a sa base).
// Si encore trop haut/bas chez toi, ajuste cette seule valeur.
#define PRISON_FENCE_Z_OFFSET   -1.5
// Hauteur d'un grillage empile (mesuree empiriquement sur ce modele) : decalage
// vertical entre l'etage du bas et celui du dessus pour doubler la hauteur.
#define PRISON_FENCE_STACK_H     3.0

// [GRANDE PRISON] Dimensions doublees
#define PRISON_WIDTH       60.0   // Largeur (axe X, mur Est-Ouest)
#define PRISON_DEPTH       45.0   // Profondeur (axe Y, vers le nord = -Y)
#define PRISON_PANEL_LEN   3.0    // Largeur d'un panneau de grillage (mesuree dans le fichier source)

#define MODEL_PRISON_FENCE   974   // tall_fence
#define MODEL_PRISON_GATE    969   // Electricgate
#define MODEL_PRISON_TOWER   3279  // a51_spottower
#define MODEL_PRISON_PYLON   16335 // des_transtower
#define MODEL_PRISON_BAR     1499  // kb_cagedoor (grille/barreaux de cellule)

// Point de detention dans la VRAIE map (afrp_prison_map.inc). Choisi au
// centre de la piece d'accueil du fichier fourni (repere : cluster de portes
// cr_door_03/gen_doorint04 et de murs cj_sports_wall entre x=-1175/-1164,
// y=-990/-1000 ; meubles dunc_dinning/kb_couch06/plant_pot autour), a bonne
// distance (>=2 unites) de chaque porte, mur et meuble releve dans le fichier
// source -> aucun risque de spawn coince dans un mur, mais PAS verifie en jeu
// (pas de client dispo ici) : a confirmer par le proprietaire (joueur au sol,
// pas dans le decor, texture/collision correctes).
#define PRISON_CELL_X       -1170.0
#define PRISON_CELL_Y       -995.5
#define PRISON_CELL_Z        128.9
// Petite grille (3x3, 1.2 unite d'ecart) autour du point ci-dessus pour que
// plusieurs joueurs jailes en meme temps ne s'empilent pas exactement au
// meme endroit, tout en restant dans la meme zone dagagee.
#define PRISON_CELL_SPACING   1.2

#define MAX_PRISON_CUSTOM_OBJ  260
#define MAX_PRISON_BARS        100

new prisonCustomObjects[MAX_PRISON_CUSTOM_OBJ];
new prisonCustomCount = 0;

new prisonBarObjects[MAX_PRISON_BARS];
new bool:prisonBarUsed[MAX_PRISON_BARS];
new prisonBarLast = -1; // index du dernier barreau pose (pour /a barreau undo)

// Enceinte custom la plus recente construite via /a creer prison : memorisee
// pour que /a jail (prison OOC) teleporte desormais DANS cette enceinte au
// lieu de l'ancienne enceinte fixe (desactivee, sur l'eau).
new bool:prisonCustomExists = false;
new Float:prisonCustomAnchorX, Float:prisonCustomAnchorY, Float:prisonCustomAnchorZ, Float:prisonCustomAngle;

// Tourne un offset local (lx,ly) selon l'angle GTA (sens horaire depuis le
// nord, meme convention que floatsin(-angle)/floatcos(-angle) utilise pour
// les barreaux) -> permet de placer murs/tours/cellules en suivant vraiment
// la direction ou l'admin regardait au moment de la construction, au lieu
// de rester bloque sur un rectangle aligne Nord-Sud/Est-Ouest.
stock Prison_RotateOffset(Float:lx, Float:ly, Float:angle, &Float:outDX, &Float:outDY)
{
	outDX = (lx * floatcos(angle, degrees)) - (ly * floatsin(angle, degrees));
	outDY = (lx * floatsin(angle, degrees)) + (ly * floatcos(angle, degrees));
	return 1;
}

// Place un panneau de mur DOUBLE HAUTEUR (2 grillages empiles) -> infranchissable.
// Compteur passe par reference pour pouvoir l'utiliser aussi bien dans le
// tableau fixe (compteur factice) que dans le tableau custom (avec limite).
stock Prison_PlaceWallPanel(Float:px, Float:py, Float:pz, Float:rx, Float:ry, Float:rz, bool:tracked, &count)
{
	new Float:baseZ = pz + PRISON_FENCE_Z_OFFSET;
	if(tracked)
	{
		if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ)
			prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_FENCE, px, py, baseZ, rx, ry, rz, -1, -1, -1, STREAM_DISTANCE);
		if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ)
			prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_FENCE, px, py, baseZ + PRISON_FENCE_STACK_H, rx, ry, rz, -1, -1, -1, STREAM_DISTANCE);
	}
	else
	{
		CreateDynamicObject(MODEL_PRISON_FENCE, px, py, baseZ, rx, ry, rz, -1, -1, -1, STREAM_DISTANCE);
		CreateDynamicObject(MODEL_PRISON_FENCE, px, py, baseZ + PRISON_FENCE_STACK_H, rx, ry, rz, -1, -1, -1, STREAM_DISTANCE);
	}
	count += 2;
	return 1;
}

// ==================== ENCEINTE FIXE ====================

stock Prison_CreateCompound()
{
	new Float:fx = PRISON_ANCHOR_X;
	new Float:fy = PRISON_ANCHOR_Y;
	new Float:fz = PRISON_ANCHOR_Z;
	new Float:halfW = PRISON_WIDTH / 2.0;
	new count = 0;

	new nbPanelsSouth = floatround(PRISON_WIDTH / PRISON_PANEL_LEN, floatround_floor);
	new gateIdx = (nbPanelsSouth / 2) - 1;
	for(new i = 0; i < nbPanelsSouth; i++)
	{
		new Float:px = fx - halfW + (float(i) * PRISON_PANEL_LEN) + (PRISON_PANEL_LEN / 2.0);
		if(i == gateIdx)
		{
			// Portail decoratif UNIQUEMENT : ce modele ne bloque pas le passage
			// (rendu en position ouverte), donc on pose AUSSI un panneau de mur
			// plein derriere pour fermer reellement l'enceinte.
			CreateDynamicObject(MODEL_PRISON_GATE, px, fy, fz + PRISON_FENCE_Z_OFFSET, 0.0, 0.0, 90.0, -1, -1, -1, STREAM_DISTANCE);
			count++;
		}
		Prison_PlaceWallPanel(px, fy, fz, 0.0, 0.0, 90.0, false, count);
	}

	new Float:northY = fy - PRISON_DEPTH;
	for(new i = 0; i < nbPanelsSouth; i++)
	{
		new Float:px = fx - halfW + (float(i) * PRISON_PANEL_LEN) + (PRISON_PANEL_LEN / 2.0);
		Prison_PlaceWallPanel(px, northY, fz, 0.0, 0.0, 90.0, false, count);
	}

	new nbPanelsSide = floatround(PRISON_DEPTH / PRISON_PANEL_LEN, floatround_floor);
	for(new i = 0; i < nbPanelsSide; i++)
	{
		new Float:py = fy - (float(i) * PRISON_PANEL_LEN) - (PRISON_PANEL_LEN / 2.0);
		Prison_PlaceWallPanel(fx + halfW, py, fz, 0.0, 0.0, 0.0, false, count);
		Prison_PlaceWallPanel(fx - halfW, py, fz, 0.0, 0.0, 0.0, false, count);
	}

	new Float:towerZ = fz + PRISON_FENCE_Z_OFFSET - 2.3;
	CreateDynamicObject(MODEL_PRISON_TOWER, fx - halfW, fy,     towerZ, 0.0, 0.0, 0.0, -1, -1, -1, STREAM_DISTANCE);
	CreateDynamicObject(MODEL_PRISON_TOWER, fx + halfW, fy,     towerZ, 0.0, 0.0, 0.0, -1, -1, -1, STREAM_DISTANCE);
	CreateDynamicObject(MODEL_PRISON_TOWER, fx - halfW, northY, towerZ, 0.0, 0.0, 0.0, -1, -1, -1, STREAM_DISTANCE);
	CreateDynamicObject(MODEL_PRISON_TOWER, fx + halfW, northY, towerZ, 0.0, 0.0, 0.0, -1, -1, -1, STREAM_DISTANCE);
	count += 4;

	CreateDynamicObject(MODEL_PRISON_PYLON, fx, northY + 4.0, fz + PRISON_FENCE_Z_OFFSET + 1.5, 0.0, 0.0, 0.0, -1, -1, -1, STREAM_DISTANCE);
	count++;

	CreateDynamic3DTextLabel("{FF0000}PRISON D'ETAT\n{FFFFFF}Zone reservee aux forces de l'ordre",
		0xFFFFFFFF, fx, fy + 1.5, fz + 3.0, 30.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, -1, -1, -1, 100.0);

	printf("[PRISON] Enceinte exterieure creee : %d objets (mur double hauteur, miradors, portail).", count);
	return 1;
}

// Position d'un joueur jail OOC (/a jail) DANS LA VRAIE MAP (afrp_prison_map.inc).
// [SIMPLIFICATION] Avant la vraie map, cette fonction branchait entre
// l'enceinte custom (/a creer prison) et l'enceinte fixe selon
// prisonCustomExists -> avec la vraie map ce serait un 3e branchement
// (fixe/custom/vraie map) sans plus de raison d'etre : la vraie map est
// maintenant LA prison, un point c'est tout. La branche custom est retiree
// d'ICI uniquement ; /a creer prison et /a barreau restent intacts plus bas
// pour qui voudrait batir une enceinte custom a part (voir entete fichier).
// L'appelant doit aussi placer le joueur sur PRISON_MAP_WORLD/PRISON_MAP_INTERIOR
// (voir afrp_prison_map.inc) : cette fonction ne renvoie que x/y/z.
stock Prison_GetCellPos(playerid, &Float:outX, &Float:outY, &Float:outZ)
{
	new col = playerid % 3;
	new row = (playerid / 3) % 3;
	outX = PRISON_CELL_X + (float(col) - 1.0) * PRISON_CELL_SPACING;
	outY = PRISON_CELL_Y + (float(row) - 1.0) * PRISON_CELL_SPACING;
	outZ = PRISON_CELL_Z;
	return 1;
}

// ==================== /a creer prison (a la position du joueur) ====================

stock Prison_DestroyCustom()
{
	for(new i = 0; i < prisonCustomCount; i++)
	{
		if(prisonCustomObjects[i] != 0)
			DestroyDynamicObject(prisonCustomObjects[i]);
	}
	prisonCustomCount = 0;
	return 1;
}

stock Prison_CmdCreateAt(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 3)
	{
		SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 3+.");
		return 1;
	}
	Prison_DestroyCustom();

	new Float:fx, Float:fy, Float:fz, Float:fangle;
	GetPlayerPos(playerid, fx, fy, fz);
	GetPlayerFacingAngle(playerid, fangle);
	new Float:halfW = PRISON_WIDTH / 2.0;
	new dummyCount = 0;
	new Float:dx, Float:dy;

	// Espace local : ly=0 = mur d'entree (cote admin), ly=+PRISON_DEPTH = mur
	// du fond ; lx de -halfW a +halfW. Tout est tourne par Prison_RotateOffset
	// selon fangle pour que l'enceinte suive vraiment la direction du regard.
	new nbPanelsSouth = floatround(PRISON_WIDTH / PRISON_PANEL_LEN, floatround_floor);
	new gateIdx = (nbPanelsSouth / 2) - 1;
	for(new i = 0; i < nbPanelsSouth; i++)
	{
		new Float:lx = -halfW + (float(i) * PRISON_PANEL_LEN) + (PRISON_PANEL_LEN / 2.0);
		Prison_RotateOffset(lx, 0.0, fangle, dx, dy);
		if(i == gateIdx)
		{
			// Portail decoratif UNIQUEMENT : ce modele ne bloque pas le passage
			// (rendu en position ouverte), donc on pose AUSSI un panneau de mur
			// plein derriere pour fermer reellement l'enceinte.
			if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ)
				prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_GATE, fx + dx, fy + dy, fz + PRISON_FENCE_Z_OFFSET, 0.0, 0.0, fangle + 90.0, -1, -1, -1, STREAM_DISTANCE);
		}
		Prison_PlaceWallPanel(fx + dx, fy + dy, fz, 0.0, 0.0, fangle + 90.0, true, dummyCount);
	}

	for(new i = 0; i < nbPanelsSouth; i++)
	{
		new Float:lx = -halfW + (float(i) * PRISON_PANEL_LEN) + (PRISON_PANEL_LEN / 2.0);
		Prison_RotateOffset(lx, PRISON_DEPTH, fangle, dx, dy);
		Prison_PlaceWallPanel(fx + dx, fy + dy, fz, 0.0, 0.0, fangle + 90.0, true, dummyCount);
	}

	new nbPanelsSide = floatround(PRISON_DEPTH / PRISON_PANEL_LEN, floatround_floor);
	for(new i = 0; i < nbPanelsSide; i++)
	{
		new Float:ly = (float(i) * PRISON_PANEL_LEN) + (PRISON_PANEL_LEN / 2.0);
		Prison_RotateOffset(halfW, ly, fangle, dx, dy);
		Prison_PlaceWallPanel(fx + dx, fy + dy, fz, 0.0, 0.0, fangle, true, dummyCount);
		Prison_RotateOffset(-halfW, ly, fangle, dx, dy);
		Prison_PlaceWallPanel(fx + dx, fy + dy, fz, 0.0, 0.0, fangle, true, dummyCount);
	}

	new Float:towerZ = fz + PRISON_FENCE_Z_OFFSET - 2.3;
	Prison_RotateOffset(-halfW, 0.0, fangle, dx, dy);
	if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ) prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_TOWER, fx + dx, fy + dy, towerZ, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);
	Prison_RotateOffset(halfW, 0.0, fangle, dx, dy);
	if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ) prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_TOWER, fx + dx, fy + dy, towerZ, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);
	Prison_RotateOffset(-halfW, PRISON_DEPTH, fangle, dx, dy);
	if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ) prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_TOWER, fx + dx, fy + dy, towerZ, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);
	Prison_RotateOffset(halfW, PRISON_DEPTH, fangle, dx, dy);
	if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ) prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_TOWER, fx + dx, fy + dy, towerZ, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);

	Prison_RotateOffset(0.0, PRISON_DEPTH - 4.0, fangle, dx, dy);
	if(prisonCustomCount < MAX_PRISON_CUSTOM_OBJ)
		prisonCustomObjects[prisonCustomCount++] = CreateDynamicObject(MODEL_PRISON_PYLON, fx + dx, fy + dy, fz + PRISON_FENCE_Z_OFFSET + 1.5, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);

	// Memorise cette enceinte pour que /a jail teleporte desormais dedans
	prisonCustomAnchorX = fx;
	prisonCustomAnchorY = fy;
	prisonCustomAnchorZ = fz;
	prisonCustomAngle = fangle;
	prisonCustomExists = true;

	new msg[180];
	format(msg, sizeof(msg),
		"{00FF00}[Prison]{FFFFFF} Grande enceinte construite a votre position (%d objets, mur double hauteur). {FFFF00}/a barreau{FFFFFF} pour fermer les cellules.",
		prisonCustomCount);
	SendClientMessage(playerid, -1, msg);
	return 1;
}

// ==================== /a barreau (grilles de cellule) ====================

stock Prison_CmdAddBar(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 2)
	{
		SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 2+.");
		return 1;
	}
	new freeSlot = -1;
	for(new i = 0; i < MAX_PRISON_BARS; i++)
	{
		if(!prisonBarUsed[i]) { freeSlot = i; break; }
	}
	if(freeSlot == -1)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Prison]{FFFFFF} Limite de barreaux atteinte (100). Retirez-en avant d'en ajouter.");
		return 1;
	}

	new Float:fx, Float:fy, Float:fz, Float:fangle;
	GetPlayerPos(playerid, fx, fy, fz);
	GetPlayerFacingAngle(playerid, fangle);
	fx += 1.0 * floatsin(-fangle, degrees);
	fy += 1.0 * floatcos(-fangle, degrees);

	prisonBarObjects[freeSlot] = CreateDynamicObject(MODEL_PRISON_BAR, fx, fy, fz - 1.0, 0.0, 0.0, fangle, -1, -1, -1, STREAM_DISTANCE);
	prisonBarUsed[freeSlot] = true;
	prisonBarLast = freeSlot;

	new msg[140];
	format(msg, sizeof(msg), "{00FF00}[Prison]{FFFFFF} Barreau pose (id {FFFF00}%d{FFFFFF}). {FFFF00}/a barreau undo{FFFFFF} pour annuler.", freeSlot);
	SendClientMessage(playerid, -1, msg);
	return 1;
}

stock Prison_CmdUndoBar(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 2)
	{
		SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 2+.");
		return 1;
	}
	if(prisonBarLast == -1 || !prisonBarUsed[prisonBarLast])
	{
		SendClientMessage(playerid, -1, "{FF4444}[Prison]{FFFFFF} Aucun barreau a annuler.");
		return 1;
	}
	DestroyDynamicObject(prisonBarObjects[prisonBarLast]);
	prisonBarUsed[prisonBarLast] = false;
	SendClientMessage(playerid, -1, "{00FF00}[Prison]{FFFFFF} Dernier barreau retire.");
	prisonBarLast = -1;
	return 1;
}

stock Prison_CmdDelBar(playerid, id)
{
	if(PlayerInfo[playerid][pAdmin] < 2)
	{
		SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 2+.");
		return 1;
	}
	if(id < 0 || id >= MAX_PRISON_BARS || !prisonBarUsed[id])
	{
		SendClientMessage(playerid, -1, "{FF4444}[Prison]{FFFFFF} ID de barreau invalide ou deja retire.");
		return 1;
	}
	DestroyDynamicObject(prisonBarObjects[id]);
	prisonBarUsed[id] = false;
	if(prisonBarLast == id) prisonBarLast = -1;
	SendClientMessage(playerid, -1, "{00FF00}[Prison]{FFFFFF} Barreau retire.");
	return 1;
}

stock Prison_CmdListBars(playerid)
{
	new count = 0;
	for(new i = 0; i < MAX_PRISON_BARS; i++)
	{
		if(prisonBarUsed[i]) count++;
	}
	new msg[100];
	format(msg, sizeof(msg), "{FFAA00}[Prison]{FFFFFF} %d barreau(x) actif(s) sur %d max.", count, MAX_PRISON_BARS);
	SendClientMessage(playerid, -1, msg);
	return 1;
}
