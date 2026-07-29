/*
	afrp_mapedit.inc - Editeur de mapping en jeu

	Poser des objets depuis le code sans voir le resultat, c'est prendre le
	risque d'un mur au milieu d'une route ou d'un batiment enterre : ca ne se
	voit qu'en jeu. Cet editeur laisse l'admin mapper lui-meme, a la souris,
	et sauvegarde tout seul. Aucune coordonnee n'est jamais ecrite a la main.

	  /mapedit modeles         catalogue de modeles range par theme
	  /mapedit objet <model>   pose un objet devant toi et ouvre l'edition
	  /mapedit select          clique un objet pose pour le selectionner
	  /mapedit editer          rouvre l'edition souris sur le selectionne
	  /mapedit copier          duplique le selectionne, decale de 2m
	  /mapedit suppr           supprime le selectionne
	  /mapedit liste           les objets a moins de 30m
	  /mapedit aller <n>       te teleporte sur l'objet n
	  /mapedit sauver          force la sauvegarde (elle est deja automatique)
	  /mapedit aide

	=== SAUVEGARDE AUTOMATIQUE ===
	Chaque creation, deplacement et suppression ecrit mapping.cfg. Un mapping
	de plusieurs heures perdu parce que le serveur a plante avant un /sauver
	manuel serait la pire facon de decouvrir cet editeur.

	=== L'INTERIEUR ET LE MONDE SONT ENREGISTRES ===
	Un objet pose dans un interieur ne doit pas flotter dans le monde exterieur
	a la meme position. L'interieur et le monde virtuel du createur sont donc
	retenus et reappliques au chargement.

	=== EDITION ANNULEE ===
	Si l'admin annule l'edition (clic droit), l'objet revient a sa position
	precedente. Un objet neuf annule est supprime : on ne laisse pas trainer
	un objet pose par erreur au milieu de la route.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit                -> MapEd_Init()
	- OnPlayerConnect               -> MapEd_OnConnect(playerid)
	- OnPlayerEditDynamicObject     -> MapEd_OnEdit(...)   EN PREMIER
	- OnPlayerSelectDynamicObject   -> MapEd_OnSelect(...) EN PREMIER
	- OnDialogResponse              -> MapEd_OnDialog(playerid, ...)
- /mapedit                      -> MapEd_Command(playerid, cmdtext)
*/

#if defined AFRP_MAPEDIT_INCLUDED
	#endinput
#endif
#define AFRP_MAPEDIT_INCLUDED

#define ME_MAX_OBJ      1000
#define ME_FILE         "mapping.cfg"
#define ME_ADMIN_LEVEL  3
#define ME_STREAM_DIST  300.0
#define ME_LISTE_DIST   30.0
#define DIALOG_ME_LISTE 9634

new meModel[ME_MAX_OBJ];
new Float:meX[ME_MAX_OBJ], Float:meY[ME_MAX_OBJ], Float:meZ[ME_MAX_OBJ];
new Float:meRX[ME_MAX_OBJ], Float:meRY[ME_MAX_OBJ], Float:meRZ[ME_MAX_OBJ];
new meInt[ME_MAX_OBJ], meVw[ME_MAX_OBJ];
new meObj[ME_MAX_OBJ];              // id streamer, -1 si non cree
new mePris[ME_MAX_OBJ];

new meSel[MAX_PLAYERS];             // index selectionne, -1 si aucun
new meNeuf[MAX_PLAYERS];            // 1 = objet cree a l'instant, pas encore valide

// ============================================================================
// OUTILS
// ============================================================================
stock MapEd_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock MapEd_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= ME_ADMIN_LEVEL);
}

stock MapEd_Count()
{
	new n = 0;
	for(new k = 0; k < ME_MAX_OBJ; k++) if(mePris[k] == 1) n++;
	return n;
}

// Index a partir de l'id streamer, -1 si l'objet n'est pas a nous.
stock MapEd_ParObjectId(objectid)
{
	if(objectid <= 0) return -1;
	for(new k = 0; k < ME_MAX_OBJ; k++)
		if(mePris[k] == 1 && meObj[k] == objectid) return k;
	return -1;
}

stock MapEd_Creer(k)
{
	if(k < 0 || k >= ME_MAX_OBJ || mePris[k] != 1) return 0;
	if(meObj[k] != -1 && IsValidDynamicObject(meObj[k])) DestroyDynamicObject(meObj[k]);
	meObj[k] = CreateDynamicObject(meModel[k], meX[k], meY[k], meZ[k],
		meRX[k], meRY[k], meRZ[k], meVw[k], meInt[k], -1, ME_STREAM_DIST);
	return 1;
}

stock MapEd_Detruire(k)
{
	if(k < 0 || k >= ME_MAX_OBJ) return 0;
	if(meObj[k] != -1 && IsValidDynamicObject(meObj[k])) DestroyDynamicObject(meObj[k]);
	meObj[k] = -1;
	return 1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock MapEd_Field(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock MapEd_Save()
{
	if(fexist(ME_FILE)) fremove(ME_FILE);
	new File:f = fopen(ME_FILE, io_write);
	if(!f) return 0;
	new line[190];
	for(new k = 0; k < ME_MAX_OBJ; k++)
	{
		if(mePris[k] != 1) continue;
		format(line, sizeof(line), "%d|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%d|%d\n",
			meModel[k], meX[k], meY[k], meZ[k], meRX[k], meRY[k], meRZ[k], meInt[k], meVw[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock MapEd_Load()
{
	for(new k = 0; k < ME_MAX_OBJ; k++) { mePris[k] = 0; meObj[k] = -1; }
	if(!fexist(ME_FILE)) return 0;
	new File:f = fopen(ME_FILE, io_read);
	if(!f) return 0;

	new line[190], buf[32], n = 0;
	while(fread(f, line) && n < ME_MAX_OBJ)
	{
		if(strlen(line) < 9) continue;
		new k = 0;
		MapEd_Field(line, k, buf, sizeof(buf)); meModel[n] = strval(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meX[n]     = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meY[n]     = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meZ[n]     = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meRX[n]    = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meRY[n]    = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meRZ[n]    = floatstr(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meInt[n]   = strval(buf);
		MapEd_Field(line, k, buf, sizeof(buf)); meVw[n]    = strval(buf);
		if(meModel[n] < 300 || meModel[n] > 20000) continue;
		mePris[n] = 1;
		meObj[n]  = -1;
		MapEd_Creer(n);
		n++;
	}
	fclose(f);
	printf("[MapEdit] %d objet(s) de mapping charge(s)", n);
	return n;
}

// ============================================================================
// CREATION / EDITION
// ============================================================================
stock MapEd_Nouveau(playerid, model)
{
	if(!MapEd_IsAdmin(playerid)) return MapEd_Msg(playerid, "{FF4444}Reserve aux administrateurs niveau 3.");
	if(model < 300 || model > 20000)
		return MapEd_Msg(playerid, "{FF4444}Model invalide. Les objets GTA vont de 300 a 19999.");

	new k = -1;
	for(new n = 0; n < ME_MAX_OBJ; n++) if(mePris[n] == 0) { k = n; break; }
	if(k == -1) return MapEd_Msg(playerid, "{FF4444}Limite d'objets atteinte.");

	// Pose 3 metres devant l'admin plutot que sur lui : sinon l'objet apparait
	// dans son personnage et la poignee d'edition est inattrapable.
	new Float:px, Float:py, Float:pz, Float:pa;
	GetPlayerPos(playerid, px, py, pz);
	GetPlayerFacingAngle(playerid, pa);
	meX[k] = px + (3.0 * floatsin(-pa, degrees));
	meY[k] = py + (3.0 * floatcos(-pa, degrees));
	meZ[k] = pz;
	meRX[k] = 0.0; meRY[k] = 0.0; meRZ[k] = 0.0;
	meModel[k] = model;
	meInt[k]   = GetPlayerInterior(playerid);
	meVw[k]    = GetPlayerVirtualWorld(playerid);
	mePris[k]  = 1;
	meObj[k]   = -1;
	MapEd_Creer(k);

	meSel[playerid]  = k;
	meNeuf[playerid] = 1;

	new s[190];
	format(s, sizeof(s), "{00FF00}Objet %d (model %d) pose. Place-le a la souris puis valide.", k + 1, model);
	MapEd_Msg(playerid, s);
	MapEd_Msg(playerid, "{AAAAAA}Clic droit annule et supprime l'objet.");
	EditDynamicObject(playerid, meObj[k]);
	return 1;
}

stock MapEd_Editer(playerid)
{
	new k = meSel[playerid];
	if(k == -1 || mePris[k] != 1) return MapEd_Msg(playerid, "{FF4444}Aucun objet selectionne. /mapedit select");
	if(meObj[k] == -1 || !IsValidDynamicObject(meObj[k])) return MapEd_Msg(playerid, "{FF4444}Objet introuvable en jeu.");
	meNeuf[playerid] = 0;
	MapEd_Msg(playerid, "{AAAAAA}Edition ouverte. Clic droit pour annuler et revenir a la position d'avant.");
	EditDynamicObject(playerid, meObj[k]);
	return 1;
}

stock MapEd_Supprimer(playerid)
{
	new k = meSel[playerid];
	if(k == -1 || mePris[k] != 1) return MapEd_Msg(playerid, "{FF4444}Aucun objet selectionne.");
	MapEd_Detruire(k);
	mePris[k] = 0;
	meSel[playerid] = -1;
	MapEd_Save();
	new s[128];
	format(s, sizeof(s), "{FFFF00}Objet %d supprime. Il reste %d objet(s).", k + 1, MapEd_Count());
	return MapEd_Msg(playerid, s);
}

stock MapEd_Copier(playerid)
{
	new src = meSel[playerid];
	if(src == -1 || mePris[src] != 1) return MapEd_Msg(playerid, "{FF4444}Aucun objet selectionne.");

	new k = -1;
	for(new n = 0; n < ME_MAX_OBJ; n++) if(mePris[n] == 0) { k = n; break; }
	if(k == -1) return MapEd_Msg(playerid, "{FF4444}Limite d'objets atteinte.");

	meModel[k] = meModel[src];
	meX[k] = meX[src] + 2.0;    // decale, sinon la copie est invisible dans l'original
	meY[k] = meY[src];
	meZ[k] = meZ[src];
	meRX[k] = meRX[src]; meRY[k] = meRY[src]; meRZ[k] = meRZ[src];
	meInt[k] = meInt[src]; meVw[k] = meVw[src];
	mePris[k] = 1; meObj[k] = -1;
	MapEd_Creer(k);
	MapEd_Save();

	meSel[playerid]  = k;
	meNeuf[playerid] = 0;
	new s[160];
	format(s, sizeof(s), "{00FF00}Copie creee (objet %d), decalee de 2m. Elle est selectionnee.", k + 1);
	MapEd_Msg(playerid, s);
	EditDynamicObject(playerid, meObj[k]);
	return 1;
}

stock MapEd_Aller(playerid, num)
{
	new k = num - 1;
	if(k < 0 || k >= ME_MAX_OBJ || mePris[k] != 1) return MapEd_Msg(playerid, "{FF4444}Numero invalide. /mapedit liste");
	SafeTeleportPlayer(playerid, meX[k], meY[k], meZ[k] + 2.0, meInt[k], meVw[k]);
	meSel[playerid] = k;
	new s[128];
	format(s, sizeof(s), "{00FF00}Teleporte sur l'objet %d (model %d). Il est selectionne.", num, meModel[k]);
	return MapEd_Msg(playerid, s);
}

stock MapEd_Liste(playerid)
{
	new body[1400], str[144], n = 0;
	body[0] = 0;
	for(new k = 0; k < ME_MAX_OBJ; k++)
	{
		if(mePris[k] != 1) continue;
		if(!IsPlayerInRangeOfPoint(playerid, ME_LISTE_DIST, meX[k], meY[k], meZ[k])) continue;
		if(n >= 30) break;
		format(str, sizeof(str), "{FFFFFF}%d - model %d%s\n", k + 1, meModel[k],
			(meSel[playerid] == k) ? (" {00FF00}(selectionne)") : (""));
		strcat(body, str);
		n++;
	}
	if(n == 0)
	{
		new s[144];
		format(s, sizeof(s), "{AAAAAA}Aucun objet a moins de %d metres. Total sur le serveur : %d.", floatround(ME_LISTE_DIST), MapEd_Count());
		return MapEd_Msg(playerid, s);
	}
	strcat(body, "\n{AAAAAA}/mapedit aller <n> pour t'y rendre, /mapedit select pour cliquer un objet.");
	return ShowPlayerDialog(playerid, DIALOG_ME_LISTE, DIALOG_STYLE_MSGBOX, "Objets de mapping a proximite", body, "Fermer", "");
}

// ============================================================================
// HOOKS D'EDITION
// ============================================================================
// Renvoie 1 si l'objet edite nous appartient, pour que LVRP.pwn s'arrete la.
stock MapEd_OnEdit(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
	new k = MapEd_ParObjectId(objectid);
	if(k == -1) return 0;

	// EDIT_RESPONSE_UPDATE est emis EN CONTINU pendant que l'admin deplace
	// l'objet a la souris. Sans ce filtre, on recreait l'objet et on
	// reecrivait mapping.cfg plusieurs fois par seconde pendant tout le
	// glisser : le disque prenait cher pour rien. On ne traite que la
	// validation et l'annulation, et on renvoie 1 pour que le reste du
	// gamemode ne bouge pas non plus notre objet.
	if(response != EDIT_RESPONSE_FINAL && response != EDIT_RESPONSE_CANCEL) return 1;

	if(response == EDIT_RESPONSE_CANCEL)
	{
		if(meNeuf[playerid] == 1)
		{
			// Objet tout neuf annule : on ne laisse pas un objet pose par
			// erreur au milieu de la route.
			MapEd_Detruire(k);
			mePris[k] = 0;
			meSel[playerid] = -1;
			meNeuf[playerid] = 0;
			MapEd_Save();
			MapEd_Msg(playerid, "{FFFF00}Annule : l'objet a ete supprime.");
			return 1;
		}
		// Objet existant : on le remet ou il etait.
		MapEd_Creer(k);
		MapEd_Msg(playerid, "{FFFF00}Annule : l'objet est revenu a sa position precedente.");
		return 1;
	}

	meX[k] = x;   meY[k] = y;   meZ[k] = z;
	meRX[k] = rx; meRY[k] = ry; meRZ[k] = rz;
	MapEd_Creer(k);
	meNeuf[playerid] = 0;
	MapEd_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}Objet %d enregistre : %.2f %.2f %.2f. Sauvegarde faite.", k + 1, x, y, z);
	MapEd_Msg(playerid, s);
	return 1;
}

stock MapEd_OnSelect(playerid, objectid, modelid, Float:x, Float:y, Float:z)
{
	#pragma unused x
	#pragma unused y
	#pragma unused z
	new k = MapEd_ParObjectId(objectid);
	if(k == -1) return 0;

	meSel[playerid]  = k;
	meNeuf[playerid] = 0;
	CancelEdit(playerid);

	new s[176];
	format(s, sizeof(s), "{00FF00}Objet %d selectionne (model %d). /mapedit editer, copier ou suppr.", k + 1, modelid);
	MapEd_Msg(playerid, s);
	return 1;
}

// ============================================================================
// CATALOGUE DE MODELES
//
// Retenir des numeros de modeles a cinq chiffres et les taper a l'aveugle est
// le vrai obstacle du mapping, pas la pose de l'objet. Le catalogue range les
// modeles utiles par theme : on choisit dans une liste et l'objet est pose.
//
// Tous les identifiants viennent des fichiers .ide de GTA San Andreas et de
// SAMP.ide : ce sont les modeles reellement presents dans le jeu.
// ============================================================================
#define DIALOG_ME_CAT_THEME 9635
#define DIALOG_ME_CAT_MODEL 9636

#define ME_CAT_THEMES 9

new meCatTheme[ME_CAT_THEMES][40] = {
	"Prison : murs, grilles, portails",
	"Prison : cellules et securite",
	"Postes de garde et barrieres",
	"Logements prets : exterieurs",
	"Logements prets : interieurs",
	"Construction : murs, dalles, escaliers",
	"Portes et fenetres",
	"Meubles : chambre et salon",
	"Meubles : cuisine et salle de bain"
};

new meCatGrp[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

new meCatModel[] = {
	19794, 19795, 19796, 14883, 4099, 19312, 19313, 19913, 3936, 3937,
	8147, 987, 974, 19868, 19869, 19641, 19870, 19912, 18762, 18766,
	14856, 19301, 19302, 19303, 19304, 2602, 2603, 1771, 19305, 19804,
	19810, 19834, 19419, 19420, 19618, 19619, 19799, 19805, 19832, 19942,
	8168, 5837, 16095, 16096, 966, 967, 969, 985, 986, 990,
	993, 19467, 19871, 2920, 1237, 19425, 1215, 19489, 19491, 19493,
	19495, 19497, 19499, 19501, 19503, 19505, 19507, 19509, 19511, 19595,
	19854, 19849, 19865, 19866, 19867, 19490, 19492, 19494, 19496, 19498,
	19500, 19502, 19504, 19506, 19508, 19510, 19512, 19855, 19856, 19857,
	19858, 19859, 19860, 19861, 19862, 18762, 18763, 18764, 18765, 18766,
	18980, 18981, 18767, 19789, 19790, 19791, 19353, 19354, 19355, 19356,
	19357, 19358, 19359, 19360, 19361, 19362, 19843, 19844, 19845, 19846,
	19531, 19545, 19943, 1491, 1492, 1494, 1502, 1499, 1495, 1497,
	1498, 1500, 1501, 1504, 1505, 1506, 1507, 19802, 19875, 19466,
	19325, 1493, 1496, 1800, 1793, 1804, 1798, 1771, 2331, 1763,
	1766, 1753, 1735, 2096, 1810, 1594, 1781, 1792, 19787, 19786,
	2313, 1567, 1742, 2562, 19806, 19807, 19826, 2170, 2147, 2336,
	2149, 1208, 19922, 19923, 19924, 19925, 19926, 19932, 19933, 19934,
	19935, 19938, 19830, 19831, 2514, 2521, 2517, 2520, 2516, 2519,
	2515, 2518, 19873, 19874
};

new meCatNom[][33] = {
	"Mur de prison de Los Santos",
	"Portail de prison (est)",
	"Portail de prison (sud)",
	"Grille de prison",
	"Grillage de prison (LS)",
	"Cloture militaire haute",
	"Cloture militaire (retour)",
	"Grande cloture SA-MP",
	"Cloture barbelee",
	"Cloture barbelee (variante)",
	"Grillage renforce barbele",
	"Cloture electrifiee",
	"Cloture haute",
	"Grillage souple 1",
	"Grillage souple 2",
	"Section de cloture",
	"Portail metallique",
	"Grand portail metallique",
	"Bloc beton 1x1x5",
	"Mur beton 10x1x5",
	"Bloc de cellules",
	"Commissariat SF (sans cellules)",
	"Porte de cellule (grille)",
	"Porte de cellule (barreaux)",
	"Haut de porte de cellule",
	"Toilettes de cellule",
	"Lit de cellule",
	"Lits superposes",
	"Clavier a code",
	"Cadenas",
	"Panneau Personnel autorise",
	"Rubalise de police",
	"Gyrophare de police 1",
	"Gyrophare de police 2",
	"Coffre-fort",
	"Porte de coffre-fort",
	"Porte blindee de Caligula",
	"Tableau blanc",
	"Caisse de munitions",
	"Radio de police",
	"Guerite de garde (Vegas)",
	"Guerite de garde (LS)",
	"Poste de garde militaire 1",
	"Poste de garde militaire 2",
	"Barriere levante",
	"Boitier de barriere",
	"Portail electrique",
	"Portail automatique (droite)",
	"Portail automatique (gauche)",
	"Barriere de chantier",
	"Barriere de rue",
	"Barriere pour vehicules",
	"Poteau de cordon",
	"Barriere de police",
	"Barriere de rue (petite)",
	"Ralentisseur",
	"Borne lumineuse",
	"Maison SF 1",
	"Maison SF 2",
	"Maison SF 3",
	"Maison SF 4",
	"Maison LV 1",
	"Maison LV 2",
	"Maison LV 3",
	"Maison LV 4",
	"Maison LS 1",
	"Maison LS 2",
	"Maison LS 3",
	"Maison LS 4",
	"Immeuble d'appartements LS",
	"Maison de campagne (exterieur)",
	"Terrain de la maison de campagne",
	"Cloture en bois",
	"Cloture en blocs",
	"Boite aux lettres",
	"Interieur maison SF 1",
	"Interieur maison SF 2",
	"Interieur maison SF 3",
	"Interieur maison SF 4",
	"Interieur maison LV 1",
	"Interieur maison LV 2",
	"Interieur maison LV 3",
	"Interieur maison LV 4",
	"Interieur maison LS 1",
	"Interieur maison LS 2",
	"Interieur maison LS 3",
	"Interieur maison LS 4",
	"Interieur maison de campagne",
	"Cloisons interieures",
	"Porte de maison 1",
	"Porte de maison 2",
	"Porte de maison 3",
	"Porte de maison 4",
	"Porte de garage 1",
	"Porte de garage 2",
	"Bloc beton 1x1x5",
	"Bloc beton 3x3x5",
	"Bloc beton 5x5x5",
	"Bloc beton 10x10x5",
	"Bloc beton 10x1x5",
	"Poutre beton 1x1x25",
	"Panneau beton 1x25x25",
	"Escalier en beton",
	"Cube 1x1",
	"Cube 5x5",
	"Cube 10x10",
	"Mur 001",
	"Mur 002",
	"Mur 003",
	"Mur 004",
	"Mur 005",
	"Mur 006",
	"Mur 007",
	"Mur 008",
	"Mur 009",
	"Mur 010",
	"Panneau metal 1",
	"Panneau metal 2",
	"Panneau metal 3",
	"Panneau metal 4",
	"Dalle beton 125x125",
	"Dalle beton 62x15",
	"Pilier de pierre",
	"Porte interieure 1",
	"Porte interieure 2",
	"Porte interieure 3",
	"Porte interieure 4",
	"Porte interieure 5",
	"Porte exterieure 1",
	"Porte exterieure 2",
	"Porte exterieure 3",
	"Porte exterieure 4",
	"Porte exterieure 5",
	"Porte exterieure 6",
	"Porte exterieure 7",
	"Porte exterieure 8",
	"Porte exterieure 9",
	"Porte interieure fixe",
	"Porte de bureau",
	"Fenetre",
	"Vitrine de magasin",
	"Porte de boutique 1",
	"Porte de boutique 2",
	"Lit simple 1",
	"Lit simple 2",
	"Lit double 1",
	"Lit double de luxe",
	"Lits superposes",
	"Lit avec table de nuit",
	"Canape 1",
	"Canape 2",
	"Canape de luxe",
	"Fauteuil",
	"Rocking-chair",
	"Chaise pliante",
	"Table et chaises",
	"Television 1",
	"Television de luxe",
	"Television ecran plat",
	"Grande television ecran plat",
	"Meuble TV",
	"Armoire",
	"Bibliotheque",
	"Commode",
	"Lustre",
	"Telephone fixe",
	"Interrupteur",
	"Cuisiniere",
	"Refrigerateur",
	"Evier de cuisine",
	"Micro-ondes",
	"Machine a laver",
	"Table de cuisine",
	"Ilot de cuisson",
	"Hotte aspirante",
	"Plan de travail 1",
	"Plan de travail 2",
	"Meuble a four",
	"Four mural",
	"Placard de cuisine 1",
	"Placard de cuisine 2",
	"Etagere de cuisine",
	"Mixeur",
	"Barbecue",
	"Toilettes 1",
	"Toilettes 2",
	"Douche 1",
	"Douche 2",
	"Baignoire 1",
	"Baignoire 2",
	"Lavabo 1",
	"Lavabo 2",
	"Rouleau de papier toilette",
	"Savon"
};

new meCatSel[MAX_PLAYERS];   // theme ouvert par le joueur, -1 si aucun


stock MapEd_CatNbTheme(theme)
{
	new n = 0;
	for(new i = 0; i < sizeof(meCatModel); i++) if(meCatGrp[i] == theme) n++;
	return n;
}

stock MapEd_CatThemes(playerid)
{
	if(!MapEd_IsAdmin(playerid)) return MapEd_Msg(playerid, "{FF4444}Reserve aux administrateurs niveau 3.");

	new body[900], str[80];
	body[0] = 0;
	for(new t = 0; t < ME_CAT_THEMES; t++)
	{
		format(str, sizeof(str), "%s {AAAAAA}(%d)\n", meCatTheme[t], MapEd_CatNbTheme(t));
		strcat(body, str);
	}
	meCatSel[playerid] = -1;
	return ShowPlayerDialog(playerid, DIALOG_ME_CAT_THEME, DIALOG_STYLE_LIST, "Catalogue de modeles", body, "Ouvrir", "Fermer");
}

stock MapEd_CatModeles(playerid, theme)
{
	if(theme < 0 || theme >= ME_CAT_THEMES) return MapEd_CatThemes(playerid);

	// Dans un dialogue LIST, TOUTE ligne est cliquable : une ligne de titre
	// decalerait les index. La premiere ligne est donc une vraie action.
	new body[2600], str[80];
	format(body, sizeof(body), "{FFFF00}<< Retour aux themes\n");
	for(new i = 0; i < sizeof(meCatModel); i++)
	{
		if(meCatGrp[i] != theme) continue;
		format(str, sizeof(str), "{FFFFFF}%s {AAAAAA}[%d]\n", meCatNom[i], meCatModel[i]);
		strcat(body, str);
	}
	meCatSel[playerid] = theme;
	return ShowPlayerDialog(playerid, DIALOG_ME_CAT_MODEL, DIALOG_STYLE_LIST, meCatTheme[theme], body, "Poser", "Retour");
}

stock MapEd_OnDialog(playerid, dialogid, response, listitem)
{
	// Un client modifie peut repondre a un dialogue qu'il n'a jamais ouvert :
	// on revalide le niveau admin ici, pas seulement a l'ouverture.
	if(dialogid == DIALOG_ME_CAT_THEME || dialogid == DIALOG_ME_CAT_MODEL)
	{
		if(!MapEd_IsAdmin(playerid)) return 1;
	}

	if(dialogid == DIALOG_ME_CAT_THEME)
	{
		if(!response) return 1;
		return MapEd_CatModeles(playerid, listitem);
	}

	if(dialogid == DIALOG_ME_CAT_MODEL)
	{
		if(!response) return MapEd_CatThemes(playerid);
		if(listitem == 0) return MapEd_CatThemes(playerid);

		new theme = meCatSel[playerid];
		if(theme < 0 || theme >= ME_CAT_THEMES) return MapEd_CatThemes(playerid);

		// listitem 1 = premier modele du theme : on reparcourt le catalogue
		// dans le meme ordre que l'affichage.
		new rang = 0;
		for(new i = 0; i < sizeof(meCatModel); i++)
		{
			if(meCatGrp[i] != theme) continue;
			rang++;
			if(rang == listitem) return MapEd_Nouveau(playerid, meCatModel[i]);
		}
		return MapEd_CatThemes(playerid);
	}
	return 0;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock MapEd_Aide(playerid)
{
	MapEd_Msg(playerid, "{FFFF00}--- Editeur de mapping ---");
	MapEd_Msg(playerid, "{FFFFFF}/mapedit modeles {AAAAAA}: catalogue range par theme (prison, logement, meubles...)");
	MapEd_Msg(playerid, "{FFFFFF}/mapedit objet <model> {AAAAAA}: pose un objet devant toi et ouvre l'edition souris");
	MapEd_Msg(playerid, "{FFFFFF}/mapedit select {AAAAAA}: clique un objet deja pose pour le selectionner");
	MapEd_Msg(playerid, "{FFFFFF}/mapedit editer | copier | suppr {AAAAAA}: agit sur le selectionne");
	MapEd_Msg(playerid, "{FFFFFF}/mapedit liste | aller <n> {AAAAAA}: objets proches, et s'y teleporter");
	MapEd_Msg(playerid, "{AAAAAA}Tout est sauvegarde automatiquement a chaque changement.");
	return 1;
}

stock MapEd_Tok(const src[], &mePos, dest[], destSize)
{
	new p = 0;
	while(src[mePos] == ' ') mePos++;
	while(src[mePos] && src[mePos] != ' ' && p < destSize - 1)
		dest[p++] = src[mePos++];
	dest[p] = 0;
	return p;
}

stock MapEd_Command(playerid, const cmdline[])
{
	if(!MapEd_IsAdmin(playerid))
	{
		new s[160];
		format(s, sizeof(s), "{FF4444}Reserve aux admins niveau %d (tu es niveau %d).", ME_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return MapEd_Msg(playerid, s);
	}

	new mePos = 0, sub[24], arg[24];
	MapEd_Tok(cmdline, mePos, sub, sizeof(sub));        // consomme "/mapedit"
	if(MapEd_Tok(cmdline, mePos, sub, sizeof(sub)) == 0) return MapEd_Aide(playerid);

	if(strcmp(sub, "aide",   true) == 0) return MapEd_Aide(playerid);
	if(strcmp(sub, "editer", true) == 0) return MapEd_Editer(playerid);
	if(strcmp(sub, "suppr",  true) == 0) return MapEd_Supprimer(playerid);
	if(strcmp(sub, "copier", true) == 0) return MapEd_Copier(playerid);
	if(strcmp(sub, "liste",  true) == 0) return MapEd_Liste(playerid);
	if(strcmp(sub, "modeles",   true) == 0) return MapEd_CatThemes(playerid);
	if(strcmp(sub, "catalogue", true) == 0) return MapEd_CatThemes(playerid);
	if(strcmp(sub, "sauver", true) == 0)
	{
		MapEd_Save();
		new s[128];
		format(s, sizeof(s), "{00FF00}%d objet(s) sauvegardes dans %s.", MapEd_Count(), ME_FILE);
		return MapEd_Msg(playerid, s);
	}
	if(strcmp(sub, "select", true) == 0)
	{
		MapEd_Msg(playerid, "{AAAAAA}Clique un objet de mapping pour le selectionner (Echap pour annuler).");
		SelectObject(playerid);
		return 1;
	}
	if(strcmp(sub, "objet", true) == 0)
	{
		if(MapEd_Tok(cmdline, mePos, arg, sizeof(arg)) == 0)
			return MapEd_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/mapedit objet <model>");
		return MapEd_Nouveau(playerid, strval(arg));
	}
	if(strcmp(sub, "aller", true) == 0)
	{
		if(MapEd_Tok(cmdline, mePos, arg, sizeof(arg)) == 0)
			return MapEd_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/mapedit aller <numero>");
		return MapEd_Aller(playerid, strval(arg));
	}
	return MapEd_Aide(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock MapEd_OnConnect(playerid)
{
	meSel[playerid]  = -1;
	meNeuf[playerid] = 0;
	meCatSel[playerid] = -1;
	return 1;
}

stock MapEd_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++) { meSel[i] = -1; meNeuf[i] = 0; meCatSel[i] = -1; }
	MapEd_Load();
	return 1;
}
