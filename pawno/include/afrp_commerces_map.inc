/*
	afrp_commerces_map.inc - Decors des trois commerces communautaires
	(supermarche 24/7, mini market, magasin de vetements) plus le batiment
	"Project_1" rendu accessible.

	POURQUOI UN FICHIER A PART, ET UN SEUL : meme convention que
	afrp_prison_map.inc / afrp_concession_map.inc / afrp_hopital_map.inc -
	les milliers d'appels CreateDynamicObject bruts n'ont rien a faire au
	milieu de la logique de jeu. Les quatre maps sont regroupees ici parce
	qu'elles arrivent ensemble et se ressemblent (decor pur, monde ouvert),
	mais chacune garde SON tableau d'objets suivi separement : c'est ce qui
	permet de les detruire/recreer une par une (voir /a deplacer).

	CE FICHIER NE CREE AUCUN COMMERCE JOUABLE : il pose le decor. Les
	boutiques elles-memes se creent EN JEU avec le systeme bizz deja
	existant (/a creer bizz a l'emplacement de la porte, puis /bizz edit
	pour le type). Type 1 = "24/7", type 2 = "Binco" (fringues). Il n'y a
	donc aucune ligne SQL a inserer a la main pour ces trois commerces, et
	surtout aucun deuxieme systeme de boutique a ecrire.

	REBRANDING (demande explicite du proprietaire) : seules les enseignes 3D
	reellement visibles en jeu (SetDynamicObjectMaterialText) ont ete
	touchees, jamais une position/un modele/une texture. Les enseignes qui
	citaient l'auteur d'origine ou une marque etrangere sont remplacees par
	des libelles francais de longueur equivalente - un mot plus long ou plus
	court de plusieurs caracteres casse la mise en page des enseignes
	decoupees lettre par lettre sur plusieurs objets.
	  "WISE MAPPEACOES" -> "SUPERETTE AFRP"      (24/7)
	  "MARKET"          -> "MARCHE"              (24/7)
	  "24/7"            -> inchange              (lisible tel quel, pas une marque tierce)
	  "A" + "CTION"     -> "A" + "LLURE"         (vetements, enseigne "ALLURE")
	  "M" + "ODAS"      -> "M" + "ODES"          (vetements, enseigne "MODES")
	  "WISE MAP 5 ANOS" -> "AFRP VETEMENTS"      (vetements)
	Les commentaires d'en-tete des fichiers source creditent leurs auteurs
	(NINJAx_MAPPER, WISE_ENTERPRISE, Ayoub, Wikennn) : ils ne s'affichent
	pas en jeu, les retirer n'apporterait rien et serait impoli, ils sont
	rappeles au-dessus de chaque fonction.

	LIGNES SOURCE ECARTEES (magasin de vetements uniquement, 5 sur 229) :
	deux objets de modele 0 (modele inexistant) poses a 2320/944 et
	2311/953, et trois objets 19362 dont le y contient visiblement une
	valeur de z (3113 / 2750 / 3079) et le z une valeur de y (-1755 /
	-1754) - bug de l'exportateur. Elles ont ete ECARTEES et non
	"corrigees" : on ne sait pas ou l'auteur voulait les mettre, et
	inventer des coordonnees serait pire que de les omettre. Toutes les
	autres lignes des quatre fichiers sont portees telles quelles.

	RemoveBuildingForPlayer ISOLE DANS SA PROPRE FONCTION : SA-MP n'accepte
	de retirer un batiment que pour UN joueur, a sa connexion. Impossible
	donc de le faire une fois pour toutes depuis OnGameModeInit (qui n'a pas
	de playerid) - meme raison que Hopital_MapRemoveBuildings et
	MapPack_Retraits. Appelee depuis OnPlayerConnect (LVRP.pwn).

	PROJECT_1 : batiment ferme, sans enseigne, que le proprietaire veut
	rendre accessible. Les deux points d'entree/sortie ci-dessous ont ete
	deduits de la geometrie brute (deux battants de porte modele 1649 face
	a face a y=-1232.2, seul endroit du mur nord sans panneau de mur 19454),
	PAS d'une observation en jeu : IL N'Y A PAS DE CLIENT SA-MP ICI. Ils
	sont donc a verifier sur place, et a corriger ici si l'un des deux
	tombe dans un mur.

	A VERIFIER EN JEU (non verifiable depuis ce sandbox) : collision avec la
	geometrie GTA existante aux quatre emplacements, et position exacte des
	portes ou poser les pickups bizz.

	ATTENTION DOUBLON : le pack de mappings (/mappack) contient deja des
	versions "a allumer/eteindre" de deux de ces maps ("Magasin de vetements
	Wise" et "Mini market"). Elles sont eteintes par defaut et le systeme
	refuse desormais de les allumer (voir afrp_mappack.inc), sinon les
	objets se superposeraient exactement a ceux d'ici.
*/

#if defined AFRP_COMMERCES_MAP_INCLUDED
	#endinput
#endif
#define AFRP_COMMERCES_MAP_INCLUDED

#define CMM_MARKET247   0
#define CMM_MINIMARKET  1
#define CMM_VETEMENTS   2
#define CMM_PROJET1     3
#define CMM_MAX_CARTES  4

#define CMM_MAX_OBJ     261   // la plus grosse des quatre en compte 251

// Position d'observation de chaque map (pour /a deplacer et les rapports).
new Float:cmmVoirX[CMM_MAX_CARTES] = { 1357.0000, 1925.0000, 1674.0000, 2333.9000 };
new Float:cmmVoirY[CMM_MAX_CARTES] = { -1768.0000, -2071.0000, -1751.0000, -1228.5000 };
new Float:cmmVoirZ[CMM_MAX_CARTES] = { 13.5000, 14.0000, 14.0000, 21.6000 };

new cmmObj[CMM_MAX_CARTES][CMM_MAX_OBJ];
new cmmNb[CMM_MAX_CARTES];

// Cree un objet ET l'enregistre pour sa carte. C'est le seul point de passage :
// c'est ce qui rend chaque map destructible/recreable individuellement sans
// toucher aux 700 lignes de donnees brutes ci-dessous.
stock Cmm_Obj(carte, model, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, wid = -1, iid = -1, pid = -1, Float:sd = 300.0, Float:dd = 300.0)
{
	new objectid = CreateDynamicObject(model, x, y, z, rx, ry, rz, wid, iid, pid, sd, dd);
	if(carte >= 0 && carte < CMM_MAX_CARTES && cmmNb[carte] < CMM_MAX_OBJ)
		cmmObj[carte][cmmNb[carte]++] = objectid;
	return objectid;
}

stock Cmm_Detruire(carte)
{
	if(carte < 0 || carte >= CMM_MAX_CARTES) return 0;
	new n = 0;
	for(new k = 0; k < cmmNb[carte]; k++)
	{
		if(cmmObj[carte][k] > 0 && IsValidDynamicObject(cmmObj[carte][k]))
		{
			DestroyDynamicObject(cmmObj[carte][k]);
			n++;
		}
		cmmObj[carte][k] = 0;
	}
	cmmNb[carte] = 0;
	return n;
}

// ============================================================================
// SUPERMARCHE 24/7 - Los Santos, x~1333-1375 / y~-1743 a -1785
// Map communautaire creditee NINJAx_MAPPER (export Texture Studio).
// Porte a poser en bizz type 1 ("24/7") : portes cote ouest, x~1354.5 / y~-1770.
// ============================================================================
stock Market247_CreateMap()
{
	new tmpobjid;
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18981,1348.736,-1771.932,12.055,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-40-percent", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18981,1351.609,-1771.952,12.055,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-40-percent", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1361.360,-1770.882,14.185,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1366.089,-1762.848,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1354.521,-1776.897,11.361,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.496,-1764.271,14.185,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1363.976,-1764.285,14.185,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1356.145,-1764.368,16.413,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19376,1359.817,-1764.329,15.852,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampwhite", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19376,1359.683,-1764.352,12.473,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_stationfloor", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19376,1359.684,-1773.986,12.473,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_stationfloor", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19376,1359.777,-1773.924,15.852,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampwhite", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1499,1354.532,-1772.090,12.553,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 19325, "lsmall_shops", "lsmall_window01", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 2, 19325, "lsmall_shops", "lsmall_window01", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 3, 19325, "lsmall_shops", "lsmall_window01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19466,1354.521,-1770.214,15.908,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1356.925,-1770.579,17.117,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1356.922,-1764.453,17.117,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1361.914,-1764.455,17.116,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1356.925,-1770.579,16.264,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1356.922,-1764.453,16.264,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1362.620,-1771.989,17.114,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1361.914,-1764.455,16.264,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1361.909,-1772.002,17.108,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1362.620,-1771.989,16.264,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1361.909,-1772.002,16.264,-90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1344.003,-1752.098,9.487,58.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-40-percent", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1353.886,-1772.142,10.810,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1349.652,-1775.565,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1333.521,-1775.398,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1340.158,-1775.563,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.495,-1770.686,14.185,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1363.976,-1770.778,14.185,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1359.247,-1759.551,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1360.859,-1756.197,11.577,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.689,-1759.528,11.577,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.634,-1772.155,10.822,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.689,-1768.998,10.822,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.745,-1765.797,10.822,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1336.768,-1762.462,10.822,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1360.733,-1753.326,10.762,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1353.855,-1765.892,10.810,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18762,1353.848,-1768.532,10.862,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18762,1353.851,-1766.652,10.862,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18762,1353.847,-1767.616,10.862,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19386,1362.881,-1770.879,14.282,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1499,1354.512,-1769.089,12.553,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 19325, "lsmall_shops", "lsmall_window01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1359.245,-1775.438,14.185,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2949,1363.595,-1770.826,12.532,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19356,1361.362,-1761.179,14.179,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19356,1341.411,-1757.837,11.572,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19386,1361.360,-1764.507,14.282,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "orange", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.831,-1770.900,14.015,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.831,-1770.878,14.015,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1364.489,-1768.055,13.105,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.352,-1771.557,16.539,0.000,-90.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.352,-1768.925,16.841,0.000,-90.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.352,-1768.925,16.237,0.000,-90.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.352,-1768.925,16.540,0.000,-90.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1364.356,-1772.373,16.237,0.000,-90.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2163,1364.029,-1766.337,14.385,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1744,1364.118,-1763.813,13.791,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1354.520,-1764.285,11.361,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1356.144,-1764.347,16.012,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1356.140,-1767.007,16.012,0.000,-90.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1355.211,-1764.746,16.741,0.000,90.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1355.185,-1763.175,17.055,0.000,90.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1355.211,-1761.289,16.741,0.000,90.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19327,1354.375,-1773.659,16.698,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000}24/7", 140, "Ariel", 199, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1355.211,-1761.289,17.357,0.000,90.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19426,1355.211,-1761.289,17.055,0.000,90.000,90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1353.838,-1762.608,10.810,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1360.716,-1750.144,10.768,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18835, "mickytextures", "smileyface1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1361.598,-1758.756,12.018,90.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 10101, "2notherbuildsfe", "Bow_church_grass_alt", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19356,1354.522,-1761.190,14.179,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "breezewall", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19356,1346.597,-1757.870,11.572,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1344.004,-1761.080,12.054,90.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-40-percent", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18766,1351.655,-1758.753,12.012,90.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 10101, "2notherbuildsfe", "Bow_church_grass_alt", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1351.312,-1759.551,11.577,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19447,1351.314,-1756.202,11.577,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "shingles2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 673,1357.874,-1758.006,12.398,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19327,1361.265,-1774.869,15.698,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000}SUPERETTE AFRP", 140, "Ariel", 25, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19327,1361.265,-1769.099,15.188,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000}MARCHE", 140, "Ariel", 145, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19327,1361.265,-1771.229,15.188,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000}24/7", 140, "Ariel", 145, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1372,1365.490,-1774.643,12.664,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1368.370,-1767.877,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1221,1367.230,-1767.974,13.185,3.141,0.000,3.054,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.505,-1762.712,14.812,-180.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.525,-1765.793,14.694,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19325,1354.513,-1775.452,13.986,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.505,-1762.712,14.031,-180.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.538,-1772.185,13.672,-180.000,0.000,180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.526,-1765.605,14.031,-180.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.513,-1772.184,14.694,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.538,-1770.145,14.996,90.000,0.000,-180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.513,-1768.971,13.672,-180.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19325,1354.563,-1765.740,13.790,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1499,1361.333,-1763.728,12.553,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.512,-1769.161,14.694,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1897,1354.514,-1770.949,14.996,90.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19466,1354.523,-1771.018,15.908,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19450,1365.000,-1777.372,11.474,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1447,1340.930,-1784.471,13.650,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1338,1342.348,-1776.607,13.170,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1447,1369.480,-1767.611,13.650,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1219,1370.769,-1768.447,12.532,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1368.865,-1773.871,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1221,1342.682,-1781.905,12.984,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1367.201,-1769.025,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1347,1368.872,-1775.000,12.900,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1364.478,-1770.427,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1364.845,-1771.372,13.105,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2675,1365.370,-1769.312,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1368.355,-1771.164,12.772,0.000,0.000,-62.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2671,1374.185,-1768.834,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1367.264,-1768.677,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1372,1339.538,-1776.097,12.664,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1370.198,-1767.782,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1368.055,-1774.967,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1368.867,-1772.757,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2949,1361.647,-1762.961,12.496,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2949,1364.081,-1775.331,12.496,0.000,0.000,180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1367.964,-1771.322,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1367.382,-1773.826,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2677,1366.005,-1771.176,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2677,1366.057,-1774.010,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1366.433,-1774.902,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1221,1365.566,-1773.107,12.984,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1341.603,-1781.704,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1338,1365.728,-1767.923,13.170,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1219,1341.918,-1777.711,12.532,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1369.877,-1770.051,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2677,1370.768,-1768.453,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1370.879,-1768.968,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1373.817,-1767.758,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1338.739,-1781.273,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1372.834,-1768.871,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1984,1357.409,-1761.753,12.561,0.000,0.000,-180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2578,1356.523,-1759.722,14.016,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2593,1358.245,-1759.962,13.412,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1354.867,-1763.142,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1354.917,-1768.833,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2579,1355.375,-1759.722,14.016,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2596,1354.916,-1760.593,14.630,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2598,1358.245,-1759.942,13.412,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2690,1358.800,-1759.692,13.606,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1988,1360.524,-1760.152,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1357.509,-1759.822,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1981,1360.772,-1767.482,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1991,1360.768,-1773.364,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1996,1360.769,-1774.362,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1361.196,-1765.626,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1844,1358.045,-1774.854,12.560,0.000,0.000,180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1991,1360.776,-1766.509,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1996,1360.774,-1768.466,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1361.098,-1769.396,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 955,1360.944,-1761.392,12.863,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1363.532,-1763.167,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 956,1355.627,-1774.968,12.957,0.000,0.000,180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1360.075,-1775.221,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1354.971,-1772.253,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1844,1360.917,-1771.275,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1987,1359.544,-1760.151,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2533,1357.987,-1769.511,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2543,1357.998,-1770.437,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2543,1358.001,-1771.419,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2542,1356.984,-1771.418,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2542,1356.982,-1770.447,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2542,1356.975,-1769.460,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2533,1356.983,-1766.618,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2536,1357.921,-1766.614,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2539,1357.915,-1768.521,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2541,1356.973,-1768.470,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2538,1357.911,-1767.542,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2537,1357.923,-1765.645,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2541,1356.977,-1767.577,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2531,1356.987,-1765.632,12.560,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2599,1352.483,-1766.581,12.957,0.000,0.000,-33.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1350.851,-1766.564,13.097,0.000,0.000,-180.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2010,1361.131,-1763.490,12.560,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1716,1356.608,-1760.576,12.561,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2872,1360.803,-1762.653,12.559,0.000,0.000,-62.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1737,1363.435,-1767.260,12.561,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1789,1363.481,-1769.121,13.182,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1810,1363.388,-1764.791,12.561,0.000,0.000,-62.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1808,1354.818,-1762.528,12.560,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19562,1357.097,-1761.937,13.495,0.000,0.000,50.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19568,1357.530,-1761.525,13.494,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19572,1357.039,-1761.566,13.495,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19565,1356.727,-1761.842,13.495,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19560,1357.623,-1761.854,13.495,0.000,0.000,50.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19592,1358.150,-1761.729,13.061,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19563,1357.322,-1761.747,13.495,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2163,1364.037,-1769.109,14.385,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 11729,1361.711,-1770.448,12.561,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1220,1361.785,-1769.126,12.762,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2332,1363.473,-1765.270,12.964,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1810,1363.182,-1766.555,12.561,0.000,0.000,47.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1650,1363.719,-1765.035,13.733,0.000,0.000,-40.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19835,1363.723,-1766.990,13.440,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1808,1363.728,-1768.032,12.561,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1778,1362.993,-1770.291,12.561,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19622,1363.214,-1768.906,13.270,0.000,0.000,91.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1716,1361.984,-1769.534,12.561,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1487,1363.756,-1767.598,13.518,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19807,1363.145,-1765.994,13.410,0.000,0.000,-52.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2222,1363.520,-1767.356,13.422,0.000,0.000,-47.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 18868,1363.126,-1767.157,13.356,0.000,0.000,-76.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 11713,1361.449,-1768.994,13.801,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1220,1363.489,-1770.265,12.762,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1362.932,-1763.958,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1362.643,-1769.367,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 11743,1363.686,-1765.993,13.355,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1886,1363.359,-1766.664,13.997,0.000,0.000,4.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 19627,1363.198,-1766.980,13.356,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2690,1363.830,-1763.637,12.808,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2596,1363.449,-1763.244,14.702,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1486,1363.819,-1763.860,14.265,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1487,1363.723,-1763.757,14.308,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1370,1361.812,-1768.251,13.064,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1338.772,-1779.951,13.097,0.000,0.000,-98.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1447,1369.427,-1772.880,13.650,0.000,0.000,90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 673,1347.627,-1758.223,12.398,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 673,1362.385,-1757.994,12.398,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 673,1352.480,-1758.169,12.398,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1371.565,-1769.773,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1340.416,-1777.967,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 2676,1340.540,-1784.626,12.772,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1374.771,-1768.734,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1338,1372.881,-1768.003,13.170,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1352.212,-1768.294,13.097,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1221,1374.895,-1767.851,12.984,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1372,1368.927,-1771.532,12.664,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1219,1367.639,-1768.459,12.532,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1342.670,-1783.595,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1349,1341.142,-1783.999,13.097,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1372,1342.718,-1780.038,12.664,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1338.841,-1778.747,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1341.589,-1778.078,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1338,1339.326,-1783.349,13.170,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1265,1340.031,-1780.567,12.968,356.858,0.000,3.141,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1337.022,-1759.261,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1363.731,-1743.059,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1375.723,-1767.546,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1356.222,-1742.473,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1355.594,-1758.157,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1349.814,-1758.130,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1360.906,-1758.182,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1232,1340.825,-1759.213,15.090,0.000,0.000,0.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1359.213,-1769.367,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1359.213,-1762.627,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1356.622,-1762.627,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	tmpobjid = Cmm_Obj(CMM_MARKET247, 1893,1356.622,-1769.197,16.186,0.000,0.000,-90.000,-1,-1,-1,300.000,300.000);
	#pragma unused tmpobjid
	return 251;
}

// ============================================================================
// MINI MARKET - Los Santos, x~1919-1932 / y~-2066 a -2076
// Map communautaire creditee "Ayoub ( Dev )".
// Deux fichiers ont ete fournis pour cette map (market_1 et DOC20250807WA0074).
// Un diff ligne a ligne montre qu'ils sont identiques a la ligne de credit pres :
// memes 200 objets, memes coordonnees. On porte market_1 et on garde le credit
// que seul l'autre portait : il n'y avait rien d'autre a arbitrer.
// Aucune enseigne 3D dans cette map : rien a rebrander.
// ============================================================================
stock MiniMarket_CreateMap()
{
	new tmpobjid;
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1928.599609, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1924.599609, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1925.799438, -2066.051757, 14.346878, 0.000000, 0.000000, 0.000034, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1921.599609, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.799560, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.799560, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1920.799560, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1919.899536, -2066.052734, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1929.899536, -2076.491210, 15.806878, -0.000002, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1927.799560, -2066.051757, 15.546876, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1932.899536, -2076.491210, 15.806878, -0.000002, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1912.819824, -2067.274169, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2067.273681, 12.477608, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2067.273681, 12.477608, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2067.273681, 12.477608, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2067.273681, 12.477608, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2071.364746, 12.468608, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1928.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959716, -2075.684082, 14.346878, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1932.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2071.364746, 12.468608, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1924.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1928.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1932.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1924.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1921.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1923.876464, -2076.078613, 11.565093, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1921.599609, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1923.876464, -2076.078613, 11.565093, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1922.887207, -2070.808349, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1929.627197, -2070.918457, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1922.887207, -2070.808349, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1929.627197, -2070.918457, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2068.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2070.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2073.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2068.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2069.273681, 12.478610, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2070.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2073.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2069.273681, 12.478610, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1918.156372, -2071.288085, 11.565094, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1934.366333, -2071.288085, 11.565094, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2071.364746, 12.468608, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1927.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1920.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.949707, -2066.051757, 14.346878, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19368, 1931.871582, -2066.017578, 14.246875, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1649, "wglass", "carshowwin2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1925.799438, -2066.051757, 14.346878, 0.000000, 0.000000, 0.000034, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19368, 1923.741333, -2066.017578, 14.246875, 0.000000, 0.000000, 90.000030, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1649, "wglass", "carshowwin2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1929.799560, -2066.051757, 14.346878, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1499, 1929.304199, -2066.081787, 12.546875, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 18065, "ab_sfammumain", "shelf_glas", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1499, 1926.304199, -2066.101806, 12.546875, 0.000000, 0.000000, 360.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 18065, "ab_sfammumain", "shelf_glas", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.566528, -2066.047607, 14.346878, -0.000000, -0.000000, -179.999938, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19368, 1920.619506, -2066.011718, 14.246875, -0.000000, 0.000000, -89.999931, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1649, "wglass", "carshowwin2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1927.799560, -2066.051757, 15.546876, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1927.799560, -2066.051757, 16.346878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.799560, -2066.051757, 16.346878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.799560, -2066.051757, 16.346878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1920.799560, -2066.051757, 16.346878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2069.273681, 12.478610, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2069.273681, 12.478610, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1918.156372, -2071.288085, 11.565094, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1934.366333, -2071.288085, 11.565094, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959716, -2075.684082, 14.346878, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.556518, -2075.677734, 14.346878, -0.000000, -0.000000, -179.999938, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1929.198242, -2071.364746, 12.468608, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19380, 1923.318237, -2071.364746, 12.468608, 0.000000, 90.042007, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18065, "ab_sfammumain", "gun_floor1", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1927.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1920.799560, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1923.876464, -2076.078613, 11.565093, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1928.876464, -2076.076904, 11.565093, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1922.887207, -2070.918457, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1922.887207, -2070.808349, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1929.627197, -2070.918457, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1929.627197, -2070.808349, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2068.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2071.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2070.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2073.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2068.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2071.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2070.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2073.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1921.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1924.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19382, 1928.876464, -2076.076904, 11.565093, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1926.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1929.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1928.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1919.899536, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1934.899658, -2067.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1917.619506, -2067.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1934.899658, -2071.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1934.899658, -2074.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1917.619506, -2071.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1917.619506, -2074.151367, 15.806878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1919.619628, -2076.491210, 15.806878, -0.000002, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.619628, -2076.491210, 15.806878, -0.000002, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1926.619628, -2076.491210, 15.806878, -0.000002, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1922.887207, -2070.918457, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19375, 1929.627197, -2070.808349, 16.764549, 0.000000, 90.000000, 450.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2068.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2071.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2070.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1933.959472, -2073.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1918.559570, -2071.551757, 16.346878, 0.000000, 90.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1921.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1924.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1923.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1926.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1929.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1928.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1931.059570, -2075.681396, 16.346878, -0.000001, 90.000000, -179.999984, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 18762, 1919.899536, -2065.151855, 15.806878, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightblue2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1912.819824, -2073.284179, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1915.819824, -2080.983886, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1925.819824, -2080.983886, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1928.819824, -2080.983886, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19379, 1915.819824, -2090.583496, 12.486873, 0.000000, 90.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19428, 1935.266113, -2066.004638, 13.566875, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 955, 1932.851318, -2075.449707, 12.964545, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 955, "cj_ext_vend", "CJ_SPRUNK_DIRTY", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1850, 1918.710571, -2068.369140, 12.566548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2363, 1920.319091, -2073.682617, 12.564809, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7650, "vgnusedcar", "lightgreen2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2643, 1927.742065, -2064.709472, 16.046875, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14668, "711c", "CJ_7_11_win", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2643, 1927.742065, -2066.509277, 15.846874, 0.000000, 0.000000, 360.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14668, "711c", "CJ_7_11_win", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1280, 1935.633300, -2064.037109, 12.963547, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14668, "711c", "CJ_CHIP_M2", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 7650, "vgnusedcar", "lightgreen2_32", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19397, 1919.437866, -2095.313232, 14.188793, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1491, 1920.189819, -2095.312500, 12.495794, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2064.557617, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2068.657714, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2072.757812, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2420, 1929.918945, -2066.604492, 12.564547, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14668, "711c", "CJ_7_11_win", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2076.857910, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2080.857421, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2084.957519, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2089.057617, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1915.016113, -2093.157714, 13.046874, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 970, 1912.926147, -2062.497558, 13.046874, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1932.705566, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1934.365600, -2064.200439, 11.786874, 90.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1929.705566, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1926.705566, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1923.705566, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1920.705566, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1919.805419, -2062.540771, 11.786874, 90.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19430, 1918.105590, -2064.340576, 11.786874, 90.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1843, 1926.431762, -2075.409912, 12.465821, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1887, 1929.439086, -2075.398193, 12.464547, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1884, 1924.316528, -2071.747314, 12.464387, 0.000000, 0.000000, 65.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 955, 1931.593994, -2075.451416, 12.964548, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1844, 1923.442504, -2075.406494, 12.464172, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1889, 1929.365844, -2072.309326, 12.464547, 0.000000, 0.000000, 60.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1842, 1930.935668, -2069.675537, 12.944546, 0.000000, 0.000000, 60.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1885, 1925.062133, -2070.139648, 12.564547, 0.000000, 0.000000, -25.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1885, 1925.262084, -2069.639648, 12.564547, 0.000000, 0.000000, -25.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19622, 1920.484252, -2073.858398, 13.266386, 13.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2413, 1920.531738, -2068.228515, 12.565092, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2413, 1920.531738, -2069.628417, 12.565092, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1959, 1920.616088, -2067.668945, 13.595563, 0.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1845, 1920.499877, -2075.594238, 12.566272, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1847, 1933.743896, -2072.446289, 12.464547, 0.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1209, 1933.875000, -2068.121093, 12.564547, 0.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 956, 1933.839843, -2069.290039, 12.964549, 0.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 2681, 1933.796875, -2067.072998, 12.564547, 0.000000, 0.000000, 270.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 3660, 1913.669921, -2072.372558, 14.572811, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 3660, 1913.669921, -2085.512451, 14.572811, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19428, 1917.061035, -2095.322021, 14.187811, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19428, 1915.460937, -2095.322021, 14.187811, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 19428, 1913.861083, -2095.322021, 14.187811, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1594, 1919.631225, -2077.913818, 13.072813, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1594, 1918.031616, -2080.913818, 13.072813, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1432, 1919.424438, -2083.768310, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1432, 1918.224731, -2086.768310, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1223, 1920.542846, -2081.361328, 12.175712, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1223, 1920.542846, -2086.361328, 12.175712, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1223, 1920.542846, -2091.361328, 12.175712, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1927.429199, -2069.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1931.429199, -2069.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1922.429199, -2069.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1927.429199, -2073.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1931.429199, -2073.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1893, 1922.429199, -2073.378173, 16.664548, 0.000000, 0.000000, 90.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1778, 1918.498168, -2075.289306, 12.565609, 0.000000, 0.000000, 180.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 638, 1918.697143, -2071.550781, 13.166406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 638, 1918.697143, -2073.550781, 13.166406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1215, 1918.549072, -2062.594726, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1215, 1919.549072, -2062.594726, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1215, 1934.148925, -2062.594726, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 1215, 1933.148925, -2062.594726, 12.672811, 0.000000, 0.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 639, 1920.929931, -2080.355712, 14.772811, 0.000000, 3.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 639, 1920.929931, -2083.355712, 14.772811, 0.000000, 3.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 639, 1920.929931, -2088.355712, 14.772811, 0.000000, 3.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	tmpobjid = Cmm_Obj(CMM_MINIMARKET, 639, 1920.929931, -2092.355712, 14.772811, 0.000000, 3.000000, 0.000000, -1, -1, -1, 200.00, 200.00);
	#pragma unused tmpobjid
	return 200;
}

// ============================================================================
// MAGASIN DE VETEMENTS - Los Santos, x~1669-1681 / y~-1742 a -1760
// Map communautaire creditee WISE_ENTERPRISE (export Texture Studio).
// A poser en bizz type 2 ("Binco") : facade est, x~1678 / y~-1757.
// ============================================================================
stock Vetements_CreateMap()
{
	new tmpobjid;
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1670.738403, -1750.135864, 14.203300, -40.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1674.193603, -1744.849609, 15.180000, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1670.741333, -1751.009033, 14.521400, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1670.742797, -1749.263549, 13.888330, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 3438, 1672.759887, -1750.355834, 15.338230, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19597, "lsbeachside", "ceilingtiles4-128x128", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1679.173095, -1757.809814, 12.420900, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1671.982421, -1744.195800, 12.379199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18765, 1674.617309, -1754.846191, 10.423600, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3988, "civic06_lan", "LAcityhwal2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18765, 1674.637817, -1749.536499, 10.425600, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3988, "civic06_lan", "LAcityhwal2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1669.964599, -1754.836669, 14.880200, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1673.992431, -1744.093139, 12.379199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1675.564575, -1759.695190, 12.377200, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1673.928710, -1744.196289, 12.377200, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1669.966186, -1749.533569, 14.882160, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1670.027709, -1754.813110, 14.872200, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1671.965209, -1744.095214, 12.379199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1671.994628, -1759.696289, 12.377200, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1671.984619, -1759.784912, 12.379199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1675.655151, -1759.783569, 12.379199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.002441, -1750.653442, 14.875100, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1675.998046, -1745.184936, 14.877099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.001831, -1746.179809, 14.877099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1671.781494, -1743.921386, 17.623600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2412, 1675.756835, -1749.149169, 12.931500, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19076, "xmastree1", "goldplate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.002807, -1751.649169, 14.877099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1670.026245, -1749.535034, 14.872200, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1673.984741, -1748.608032, 17.649999, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19597, "lsbeachside", "tiles7-128x128", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18766, 1671.984375, -1748.617919, 17.673997, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19597, "lsbeachside", "tiles7-128x128", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.528442, -1752.506347, 16.660589, 0.000000, 90.000000, 298.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.640625, -1756.985351, 16.657569, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.752441, -1758.100708, 17.641599, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.992431, -1756.049438, 17.605600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1675.579833, -1756.643432, 17.673599, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1673.004028, -1759.751831, 17.623600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.528442, -1752.506347, 17.647899, 0.000000, 90.000000, 298.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.640625, -1756.985351, 17.641599, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.221313, -1749.376220, 17.641599, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1675.641601, -1759.753906, 17.623600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1675.643920, -1760.304199, 17.617599, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.222534, -1745.962036, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.777221, -1745.955688, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.753784, -1750.947631, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1676.574707, -1752.297241, 17.643600, 0.000000, 90.000000, 118.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1674.208251, -1743.961547, 17.623600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1671.737304, -1760.302368, 17.615600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.752807, -1755.913208, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.665527, -1758.310302, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.682861, -1756.766601, 17.643600, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1669.994262, -1757.759399, 17.607599, 0.000000, 90.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1673.623657, -1755.508056, 17.663600, 0.000000, 90.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1672.906005, -1756.637573, 17.667600, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1673.622802, -1757.066162, 17.673601, 0.000000, 90.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1680.659790, -1744.504394, 12.674110, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1680.694335, -1760.848022, 12.674110, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18765, 1674.606567, -1755.274902, 10.421600, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3988, "civic06_lan", "LAcityhwal2", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1678.001953, -1760.177368, 12.634960, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1680.632568, -1754.750122, 12.674110, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 3438, 1672.966552, -1749.330444, 15.338230, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19597, "lsbeachside", "ceilingtiles4-128x128", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 3438, 1672.832519, -1749.884399, 15.338230, 0.000000, 0.000000, 69.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 19597, "lsbeachside", "ceilingtiles4-128x128", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.091552, -1742.432983, 12.674099, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1677.129028, -1745.005249, 12.420900, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1679.162353, -1747.010375, 12.420900, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1679.164184, -1751.945556, 12.420900, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1679.169433, -1756.909912, 12.420900, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1760.202392, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1670.741455, -1748.277465, 13.888330, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1671.679931, -1744.890014, 15.182040, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1672.684204, -1744.871093, 15.182040, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19939, 1673.685302, -1744.852539, 15.182040, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18996, "mattextures", "sampblack", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18762, 1675.988037, -1745.034790, 14.877099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 6094, "canalsg_law", "sw_brewbrick01", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1679.522338, -1758.506469, 12.634960, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1678.001953, -1760.177368, 12.634960, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1756.851806, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1679.522338, -1755.136230, 12.634960, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1751.609985, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1679.532348, -1753.105712, 12.634960, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1753.620727, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1679.532348, -1746.145263, 12.634960, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1747.619140, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 18880, 1679.500366, -1744.659301, 14.395001, 180.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 2, 3314, "ce_burbhouse", "sw_wallbrick_06", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19369, 1677.811279, -1744.624511, 12.634960, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 3820, "boxhses_sfsx", "ws_ irongate", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2722, 1678.214111, -1758.294311, 17.609838, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFFFF}A", 50, "Ariel", 90, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2722, 1678.214111, -1757.234252, 17.609838, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFFFF}LLURE", 50, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2722, 1678.164062, -1757.863891, 16.659824, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFFFF}M", 50, "Ariel", 90, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2722, 1678.184082, -1756.723999, 16.659837, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFFFF}ODES", 50, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2722, 1669.244873, -1743.493408, 18.019834, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFFFF}AFRP VETEMENTS", 120, "Ariel", 30, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1679.303955, -1744.691528, 11.706899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.976196, -1746.010009, 14.953100, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1677.953125, -1758.603881, 11.706899, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1676.001708, -1746.593139, 13.987460, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.979370, -1750.242553, 13.877400, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1676.006591, -1747.702148, 15.878060, 90.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1671.964233, -1745.676391, 12.748299, 0.000000, 163.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1679.331298, -1752.643066, 11.706899, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1676.330688, -1754.112670, 11.706899, 0.000000, 0.000000, -140.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1677.942138, -1756.054199, 11.706899, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.067626, -1759.192138, 16.286979, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.999389, -1746.994018, 13.969659, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1737, 1671.674560, -1746.207641, 13.428199, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1670.743164, -1745.865234, 12.718299, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1737, 1670.967651, -1746.203613, 13.047200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1737, 1670.969970, -1745.881591, 13.338199, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1676.163452, -1749.092163, 15.743080, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2719, 1675.274780, -1758.981567, 14.133700, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2369, 1671.118286, -1746.114624, 13.818799, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2390, 1670.852050, -1752.174072, 14.012109, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2406, 1674.515502, -1759.088867, 14.988499, 26.000000, -90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2391, 1670.871704, -1752.818969, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2399, 1675.300903, -1751.937377, 14.012100, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2398, 1670.839965, -1753.438598, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2396, 1672.950805, -1758.824584, 14.415100, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2389, 1670.678833, -1755.989868, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1674.246093, -1756.516723, 12.712699, 40.000000, 0.000000, -271.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2412, 1675.740356, -1746.920532, 12.931500, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2387, 1673.554687, -1755.078857, 11.886230, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1672.518432, -1756.489135, 12.704700, 40.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1672.524658, -1755.385375, 12.704700, 40.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1674.259277, -1755.438842, 12.712699, 40.000000, 0.000000, -271.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2376, 1673.676635, -1757.137207, 12.642800, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2386, 1670.607177, -1751.248535, 14.624699, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2387, 1673.526489, -1756.112792, 11.886230, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2387, 1673.505737, -1757.146240, 11.886230, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2386, 1673.376586, -1755.680175, 13.623700, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2386, 1673.347900, -1756.684814, 13.623700, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2384, 1673.332153, -1756.195556, 13.623700, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2399, 1670.807006, -1756.674682, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2372, 1674.220336, -1758.538818, 12.841899, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2399, 1670.808959, -1757.349243, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2392, 1676.831909, -1754.182983, 13.548999, 0.000000, 0.000000, 40.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2381, 1675.831909, -1758.729736, 13.548999, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2372, 1677.702514, -1754.815429, 12.841899, 0.000000, 0.000000, 41.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2381, 1673.272827, -1752.828735, 13.548999, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1676.751953, -1751.969726, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1673.679565, -1756.464721, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2071, 1676.479614, -1753.540527, 14.093899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2071, 1672.156127, -1759.099853, 14.093899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2071, 1677.336059, -1758.870483, 14.093899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2371, 1672.905151, -1753.385620, 12.841899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2372, 1678.046630, -1758.041748, 12.841899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2381, 1677.881469, -1757.411132, 13.548999, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2372, 1676.474487, -1758.547973, 12.841899, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2382, 1672.597900, -1753.336059, 13.512999, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2392, 1672.589721, -1752.433349, 13.548999, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2406, 1670.628662, -1754.894409, 14.988499, 26.000000, -90.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2396, 1670.734375, -1755.291137, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2396, 1676.583007, -1758.909912, 14.415100, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2394, 1673.565917, -1758.738769, 13.545700, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2399, 1676.008056, -1758.896484, 14.415100, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2729, 1673.860229, -1759.140625, 14.033709, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2730, 1670.599731, -1748.948242, 14.235710, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2707, 1674.279541, -1744.861450, 15.155400, 0.000000, 0.000000, 78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1673.646484, -1745.119506, 14.923399, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2387, 1673.107788, -1749.633422, 12.878549, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2071, 1673.007812, -1751.630249, 14.093899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2384, 1673.388793, -1755.207275, 13.623700, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2731, 1670.599487, -1751.096923, 13.731749, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2384, 1670.690307, -1748.108764, 13.996700, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2384, 1670.616210, -1750.782592, 14.624699, 0.000000, 0.000000, 88.854301, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2401, 1670.808471, -1754.059082, 14.012100, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2401, 1675.246704, -1751.366210, 14.012100, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2690, 1675.427368, -1750.260498, 13.124899, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1716, 1671.780761, -1745.062500, 12.872280, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2247, 1671.263549, -1746.456298, 14.144100, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2698, 1674.760620, -1746.200683, 13.814800, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1670.721069, -1749.111816, 14.089099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2698, 1677.355346, -1754.961669, 12.562190, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1673.317260, -1756.203369, 14.138500, 0.000000, 0.000000, -1.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1671.756225, -1758.735595, 13.093400, 0.000000, 0.000000, 105.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2698, 1671.350830, -1758.357788, 13.821900, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2698, 1670.954711, -1754.862304, 12.562190, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1677.493652, -1757.428588, 13.093400, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1670.871215, -1755.964599, 13.093400, 0.000000, 0.000000, -4.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1674.392944, -1745.161743, 13.093400, 0.000000, 0.000000, -112.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1670.894531, -1757.459838, 13.093400, 0.000000, 0.000000, 13.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1676.014892, -1752.791259, 13.093400, 0.000000, 0.000000, -135.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2654, 1675.306762, -1746.176025, 13.093400, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2852, 1672.041259, -1746.233276, 13.841600, 0.000000, 0.000000, 113.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19325, 1676.860229, -1753.442260, 12.919199, 90.000000, 0.000000, 36.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.057006, -1755.112304, 16.224300, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19325, 1678.070312, -1757.164794, 12.919199, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.069580, -1759.192138, 13.882149, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1675.915161, -1752.145263, 16.247100, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.057006, -1755.112304, 13.882149, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1675.915161, -1752.145263, 13.882149, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.979370, -1750.242553, 16.099899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1676.001708, -1746.593139, 16.097839, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.999389, -1746.994018, 16.104900, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.977905, -1749.795166, 16.099399, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.982910, -1749.573974, 15.878310, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.980590, -1750.780029, 14.899100, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1676.006591, -1747.702148, 17.020990, 90.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.982910, -1749.573974, 17.020170, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1676.192138, -1747.754394, 15.734880, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1676.745117, -1744.877563, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19325, 1675.338134, -1756.865356, 17.828819, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19325, 1671.647583, -1756.770751, 17.828819, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1670.746582, -1747.310913, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1676.399902, -1746.931396, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1897, 1675.977905, -1749.795166, 13.913319, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 14411, 1681.671142, -1749.612304, 9.722399, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1675.996215, -1745.966674, 12.283300, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1676.023437, -1749.541381, 16.965099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1675.996215, -1745.966674, 14.213500, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1675.984008, -1750.836059, 13.279800, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1676.021850, -1748.198730, 16.965099, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1675.996215, -1745.966674, 16.141899, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19466, 1675.984008, -1750.836059, 15.209289, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1674.371582, -1747.076782, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1671.603881, -1746.911987, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1671.354125, -1751.347045, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1673.990600, -1751.493286, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 1734, 1673.066406, -1750.434448, 17.222200, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2707, 1671.518432, -1744.922485, 15.155400, 0.000000, 0.000000, 78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2707, 1672.125244, -1744.903686, 15.155400, 0.000000, 0.000000, 78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2707, 1672.836059, -1744.881591, 15.155400, 0.000000, 0.000000, 78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2345, 1672.228027, -1745.184570, 14.905400, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 2707, 1673.451049, -1744.862426, 15.155400, 0.000000, 0.000000, 78.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1676.416259, -1749.899047, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1675.266479, -1750.896118, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 948, 1672.781250, -1746.318847, 12.862469, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.089599, -1759.232177, 13.882149, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.089599, -1759.232177, 16.102148, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.089599, -1759.262207, 16.102148, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_VETEMENTS, 19087, 1678.089599, -1759.262207, 13.732148, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	#pragma unused tmpobjid
	return 224;
}

// ============================================================================
// PROJECT_1 - batiment ferme rendu accessible, x~2312-2360 / y~-1216 a -1275
// Map communautaire creditee Wikennn (export Texture Studio).
// Aucune enseigne dans la source : rien a rebrander ici.
// ============================================================================
stock Projet1_CreateMap()
{
	new tmpobjid;
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19438, 2326.119873, -1230.291015, 23.272817, -90.000000, 0.000000, 89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19440, 2313.248046, -1218.454101, 26.943765, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19464, 2360.892822, -1274.573730, 23.662118, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19464, 2360.892822, -1268.675170, 23.662118, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1768, 2338.078857, -1260.443603, 21.500000, 0.000000, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2063, 2329.812255, -1257.635864, 22.000001, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2329.369628, -1235.117919, 23.000005, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2329.369628, -1244.715576, 23.000005, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2329.369628, -1254.313232, 23.000005, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2329.369628, -1263.910888, 23.000005, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2324.665039, -1230.319091, 23.000005, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2334.274414, -1261.111572, 23.100006, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1216.022583, 23.000005, -0.000001, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2329.369628, -1273.508544, 23.000005, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2343.783691, -1264.410766, 23.100006, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19391, 2339.023437, -1262.748291, 23.200691, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1235.217895, 23.000005, 0.000000, -0.000001, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1244.815551, 23.000005, 0.000000, -0.000001, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1254.413208, 23.000005, 0.000000, -0.000001, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1264.010864, 23.000005, 0.000000, -0.000001, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1225.620239, 23.000005, -0.000001, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2345.285156, -1271.209106, 23.000005, 0.000000, -0.000004, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1097, 2330.101074, -1256.925781, 21.900001, 0.000000, -20.000011, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19815, 2329.479980, -1257.644042, 23.300006, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19899, 2333.557373, -1260.404174, 21.500000, -0.000004, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19899, 2330.954833, -1260.404174, 21.500000, -0.000004, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19872, 2334.019531, -1253.753173, 19.799993, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19872, 2334.019531, -1248.854370, 19.799993, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19817, 2335.468505, -1242.804443, 19.799993, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19817, 2335.468505, -1237.405761, 19.799993, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1649, 2331.680419, -1232.184448, 23.000005, 0.000000, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1649, 2336.084716, -1232.184448, 23.000005, 0.000000, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19866, 2335.794189, -1232.177856, 21.500000, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19866, 2335.794189, -1232.377807, 21.500000, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19866, 2330.789306, -1232.177856, 21.500000, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19866, 2330.879394, -1232.377807, 21.500000, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1649, 2336.084716, -1232.284423, 23.000005, 0.000000, 0.000001, 0.000044, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1649, 2331.680419, -1232.284423, 23.000005, 0.000000, 0.000001, 0.000044, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1096, 2330.230712, -1258.291137, 22.000001, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 3096, 2312.982910, -1218.400634, 26.980983, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1723, 2344.421386, -1266.977172, 21.500000, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19922, 2342.118896, -1268.054443, 21.403104, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19922, 2342.118896, -1271.953491, 21.403104, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2413, 2331.761962, -1265.328125, 21.500211, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19903, 2331.800292, -1251.913696, 21.500000, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19903, 2331.886474, -1247.125366, 21.500000, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19903, 2330.912109, -1241.088500, 21.500000, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19903, 2330.654296, -1235.631469, 21.500000, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2340.380371, -1275.408081, 23.000005, 0.000000, -0.000003, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 19454, 2330.770996, -1275.408081, 23.000005, 0.000000, -0.000003, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2375, 2330.157226, -1270.321411, 21.500000, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1723, 2344.421386, -1270.976196, 21.500000, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2375, 2330.157226, -1266.822265, 21.500000, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2368, 2331.772949, -1263.655395, 21.500000, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 2375, 2340.952392, -1264.536499, 21.503772, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1845, 2332.998046, -1274.577392, 21.500000, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1849, 2337.302978, -1274.903320, 21.500000, 0.000000, 0.000000, -179.999984, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 3850, 2336.558105, -1255.626220, 22.000001, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 3850, 2333.054687, -1255.626220, 22.000001, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 3850, 2329.551269, -1255.626220, 22.000001, -0.000001, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1523, 2339.099609, -1263.475341, 21.500000, 0.000000, 0.000000, 90.401062, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1568, 2338.545898, -1232.457275, 19.199991, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = Cmm_Obj(CMM_PROJET1, 1568, 2338.545898, -1232.457275, 19.199991, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	#pragma unused tmpobjid
	return 63;
}

// ============================================================================
// BATIMENTS GTA D'ORIGINE REMPLACES
// ============================================================================
// Un seul point d'appel pour les trois maps qui en ont besoin. A appeler depuis
// OnPlayerConnect : SA-MP refuse de retirer un batiment autrement (voir en-tete).
stock Commerces_MapRemoveBuildings(playerid)
{
	// Supermarche 24/7 (7 batiments)
	RemoveBuildingForPlayer(playerid, 4191, 1353.257, -1764.531, 15.593, 0.250);
	RemoveBuildingForPlayer(playerid, 1372, 1337.695, -1774.734, 12.664, 0.250);
	RemoveBuildingForPlayer(playerid, 1265, 1338.789, -1775.320, 12.968, 0.250);
	RemoveBuildingForPlayer(playerid, 1265, 1337.007, -1773.867, 13.000, 0.250);
	RemoveBuildingForPlayer(playerid, 4022, 1353.257, -1764.531, 15.593, 0.250);
	RemoveBuildingForPlayer(playerid, 1532, 1353.132, -1759.656, 12.500, 0.250);
	RemoveBuildingForPlayer(playerid, 1226, 1341.453, -1755.484, 16.421, 0.250);
	// Mini market (2 batiments)
	RemoveBuildingForPlayer(playerid, 1413, 1935.130, -2074.870, 13.726, 0.250);
	RemoveBuildingForPlayer(playerid, 1372, 1920.479, -2088.169, 12.687, 0.250);
	// Magasin de vetements (2 batiments). Ces deux-la sont a 2362/1023, soit a
	// des kilometres de la map : ce sont visiblement des restes d'un autre
	// chantier de l'auteur. Portes fidelement quand meme - retirer un batiment
	// que personne ne regarde ne casse rien, et on ne devine pas a sa place.
	RemoveBuildingForPlayer(playerid, 8677, 2362.320, 1023.179, 7.820, 0.250);
	RemoveBuildingForPlayer(playerid, 8752, 2362.320, 1023.179, 7.820, 0.250);
	return 1;
}

// ============================================================================
// PROJECT_1 : ENTREE / SORTIE
// ============================================================================
// Le batiment est clos. Le seul passage identifiable dans les donnees brutes
// est au nord : deux battants (modele 1649) a x=2331.7 et x=2336.1, y=-1232.2,
// et c'est le seul segment du mur nord sans panneau 19454. On pose donc le lien
// de part et d'autre de cette ouverture, centre sur x=2333.9.
//
// CES DEUX POINTS SONT DEDUITS, PAS VERIFIES : sans client SA-MP ici, rien ne
// garantit qu'ils tombent sur du sol libre. Si l'un des deux coince, ce sont
// les six nombres ci-dessous qu'il faut corriger, rien d'autre.
#define PROJET1_EXT_X   2333.9000
#define PROJET1_EXT_Y   -1228.5000
#define PROJET1_EXT_Z   21.6000
#define PROJET1_EXT_A   180.0000
#define PROJET1_INT_X   2333.9000
#define PROJET1_INT_Y   -1236.5000
#define PROJET1_INT_Z   21.6000
#define PROJET1_INT_A   0.0000

// Appelee depuis player_CheckEnter (LVRP.pwn), comme tous les autres liens.
// Interieur 0 / monde 0 des deux cotes : la map est en monde ouvert, et
// server_Link n'autorise le retour que si le monde virtuel du joueur est egal
// a celui declare cote entree (donc -1 ne marcherait pas ici).
stock Projet1_CheckEnter(playerid)
{
	return server_Link(playerid, 1.5,
		PROJET1_EXT_X, PROJET1_EXT_Y, PROJET1_EXT_Z, PROJET1_EXT_A, 0, 0, 0, "~w~Entree",
		PROJET1_INT_X, PROJET1_INT_Y, PROJET1_INT_Z, PROJET1_INT_A, 0, 0, 0, "~w~Sortie", 1);
}

// ============================================================================
// INIT
// ============================================================================
// Appelee une seule fois depuis OnGameModeInit, a cote des autres _CreateMap().
stock Commerces_CreateMaps()
{
	new n = 0;
	n += Market247_CreateMap();
	n += MiniMarket_CreateMap();
	n += Vetements_CreateMap();
	n += Projet1_CreateMap();
	printf("[COMMERCES] %d objet(s) crees (24/7 %d, mini market %d, vetements %d, Project_1 %d).",
		n, cmmNb[CMM_MARKET247], cmmNb[CMM_MINIMARKET], cmmNb[CMM_VETEMENTS], cmmNb[CMM_PROJET1]);
	return n;
}
