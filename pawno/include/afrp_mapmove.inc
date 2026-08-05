/*
	afrp_mapmove.inc - Deplacer une map entiere en jeu (/a deplacer)

	POURQUOI CE MODULE EXISTE : plusieurs maps communautaires ajoutees ces
	dernieres semaines tombent sur du decor GTA deja present. Personne ici ne
	peut voir ces collisions - il n'y a pas de client SA-MP dans
	l'environnement de developpement - et le proprietaire, lui, les voit mais
	ne sait pas donner des coordonnees. Lui livrer un outil qu'elle manipule
	sur place est la seule facon de regler le probleme sans faire dix
	allers-retours a l'aveugle.

	  /a deplacer                  liste les maps deplacables et leur etat
	  /a deplacer <nom>            repose la map sur sa position et son angle
	  /a deplacer reset <nom>      remet la map ou son auteur l'avait mise
	  /a deplacer aller <nom>      teleporte sur la map et dit si elle existe
	  /a deplacer hauteur <n> <v>  remonte (v>0) ou descend (v<0) la map
	  /a deplacer nettoyer <n> <r> regle le rayon de retrait auto des batiments
	                                GTA du nouvel endroit (0 = desactive)
	(admin niveau 3, comme les autres /a de creation/edition)

	COMMENT UNE MAP EST DEPLACEE : chaque objet garde sa position d'origine
	dans le code. On ne reecrit jamais les donnees. A la creation, chaque
	coordonnee passe par MM_Point() qui la reexprime par rapport a un point de
	reference de la map (son "pivot", la position d'origine approximative de
	son rez-de-chaussee), la fait tourner de l'angle demande, puis la repose
	autour du nouveau point d'ancrage. Tant qu'aucun deplacement n'est
	enregistre, ancrage = pivot et angle = 0 : le calcul rend exactement la
	coordonnee d'origine, donc rien ne bouge et rien ne peut casser.

	La rotation reprend volontairement la meme formule que
	Prison_RotateOffset() (afrp_prison.inc) : deux fonctions de rotation avec
	des conventions differentes dans le meme gamemode, c'est la garantie qu'un
	jour l'une des deux tournera dans le mauvais sens. afrp_prison.inc n'est
	pas modifie pour autant, la formule est simplement recopiee ici.

	CE QUI SUIT LA MAP, ET POURQUOI IL FALLAIT Y PENSER : deplacer les objets
	ne suffit jamais. Tout ce qui est accroche a la map par des coordonnees
	ecrites ailleurs reste sur place et casse en silence.
	  - Hopital : la zone qui autorise /duty hopital et /heal etait figee sur
	    l'ancien emplacement. Elle est desormais lue depuis l'ancrage courant
	    (MM_AnchorX/Y/Z), donc elle suit.
	  - Concession : les cinq vehicules de vitrine sont repositionnes, et le
	    pickup 'N' de la concession (qui vit en base, table
	    lvrp_server_dealership) est deplace ET resauvegarde.
	  - Project_1 : les deux points d'entree/sortie passent par MM_Point a
	    chaque test, donc ils suivent sans etat a synchroniser.

	CE QUI NE SUIT PAS TOUT SEUL, ET COMMENT C'EST CORRIGE : les
	RemoveBuildingForPlayer d'origine (Hopital_MapRemoveBuildings,
	MapPack_Retraits...) visent des batiments GTA precis, a leurs coordonnees
	REELLES et fixes dans le jeu - ils ne bougent pas avec la map. Deplacer la
	map ailleurs laisse donc le(s) batiment(s) GTA du NOUVEL endroit bien
	debout, solides, par-dessus/autour du nouveau decor : la map deplacee
	"semble invisible" alors qu'elle est bien creee (confirme par
	MM_LogEtat - Hopital et Coffee Shop ont ete reconstruits avec leur plein
	compte d'objets apres un deplacement, le 5 aout).
	[NETTOYAGE AUTO] Comme on ne connait jamais a l'avance quels batiments
	occupent un nouvel emplacement, MM_NettoyerBatiments() appelle
	RemoveBuildingForPlayer(playerid, -1, ancre, rayon) sur chaque map
	DEPLACEE : modelid -1 retire TOUS les batiments GTA dans le rayon autour
	du point d'ancrage, sans avoir besoin de connaitre leur modelid un par un
	(confirme par la doc officielle open.mp/SA-MP 0.3.7 : "-1 pour le modelid
	retire tous les objets dans le rayon donne"). Rayon par defaut
	MM_CLEAR_RADIUS_DEFAUT, ajustable par map via /a deplacer nettoyer <nom>
	<rayon>.
	[POURQUOI FUSIONNER LES ZONES QUI SE CHEVAUCHENT] La doc previent aussi :
	retirer DEUX FOIS le meme objet fait planter le client. Si deux maps
	deplacees tombent pres l'une de l'autre (c'est arrive : Hopital et Coffee
	Shop n'etaient qu'a 1 m d'ecart le 5 aout), leurs deux cercles de
	nettoyage se recouperaient et cibleraient les memes vrais batiments deux
	fois. MM_NettoyerBatiments() fusionne ces cas en UN seul appel qui
	couvre les deux zones, au lieu de faire les appels independamment.
	Toujours vrai malgre tout ca : SA-MP ne retire un batiment qu'a la
	connexion du joueur, donc rien de tout ceci ne change quoi que ce soit
	pour un joueur deja connecte - seule une reconnexion applique un nouveau
	nettoyage.

	PERSISTANCE : meme principe que MapPack_Save/MapPack_Load
	(afrp_mappack.inc) - un fichier texte dans scriptfiles, relu au demarrage
	AVANT que les maps soient creees, une ligne par map deplacee.
*/

#if defined AFRP_MAPMOVE_INCLUDED
	#endinput
#endif
#define AFRP_MAPMOVE_INCLUDED

#define MM_ADMIN_LEVEL   3
#define MM_FILE          "mapdeplace.cfg"

#define MM_COFFEE        0
#define MM_HOPITAL       1
#define MM_CONCESSION    2
#define MM_MARKET247     3
#define MM_MINIMARKET    4
#define MM_VETEMENTS     5
#define MM_PROJET1       6
#define MM_MAX_MAPS      7

// La prison est volontairement absente de cette liste : elle vit dans son
// propre monde virtuel et son propre interieur (871 / 14), la deplacer n'a
// aucun sens et casserait /a jail qui y teleporte les joueurs.

// La plus grosse map suivie ici est l'hopital (272 objets). Les quatre
// commerces ont deja leur propre tableau dans afrp_commerces_map.inc et le
// Coffee Shop celui du pack de mappings : ils ne consomment rien ici.
#define MM_MAX_OBJ       290

// Index du Coffee Shop dans le pack de mappings. Ecrit en dur parce que ce
// fichier est inclus AVANT afrp_mappack_data.inc (ou MP_COFFEE est defini) :
// le preprocesseur ne connaitrait pas encore la constante. MM_Init() verifie
// au demarrage que les deux concordent toujours et le dit sinon.
#define MM_COFFEE_MP     1

new mmNom[MM_MAX_MAPS][40] = {
	"Coffee Shop",
	"Hopital (QG faction)",
	"Concession d'occasion (LV)",
	"Supermarche 24/7",
	"Mini market",
	"Magasin de vetements",
	"Project_1"
};

// Mot-clef tape par l'admin. Court, sans accent, sans espace.
new mmCle[MM_MAX_MAPS][16] = {
	"coffee",
	"hopital",
	"concession",
	"247",
	"minimarket",
	"vetements",
	"project1"
};

// Pivot = position d'origine de reference de la map. X/Y au centre approximatif,
// Z au niveau du sol tel que l'auteur l'a construit (estime a partir de la
// hauteur la plus frequente des objets poses au sol). Si une map deplacee
// s'enfonce ou flotte de facon constante, c'est ce Z-la qu'il faut corriger,
// et lui seul.
new const Float:mmPivotX[MM_MAX_MAPS] = { 1016.6318, 394.0000, 1025.0000, 1357.0000, 1925.0000, 1674.0000, 2337.0000 };
new const Float:mmPivotY[MM_MAX_MAPS] = { -1332.1266, -1816.0000, -1958.0000, -1768.0000, -2071.0000, -1751.0000, -1245.0000 };
new const Float:mmPivotZ[MM_MAX_MAPS] = { 12.4500, 6.8000, 12.0500, 12.5500, 12.4500, 12.7500, 21.5000 };

// Ancrage courant. Initialise au pivot : tant que rien n'est deplace, le
// calcul est l'identite.
new Float:mmAncX[MM_MAX_MAPS];
new Float:mmAncY[MM_MAX_MAPS];
new Float:mmAncZ[MM_MAX_MAPS];
new Float:mmAncRot[MM_MAX_MAPS];
new mmDeplacee[MM_MAX_MAPS];

// [NETTOYAGE AUTO] Rayon de retrait "-1" (voir en-tete) autour de l'ancrage
// d'une map DEPLACEE. Point de depart raisonnable, pas une mesure : a
// ajuster au vu du jeu avec /a deplacer nettoyer <nom> <rayon>. N'a aucun
// effet tant que la map n'est pas deplacee (mmDeplacee[m] == 0).
#define MM_CLEAR_RADIUS_DEFAUT   45.0
new Float:mmClearRadius[MM_MAX_MAPS] = {MM_CLEAR_RADIUS_DEFAUT, ...};

// Carte en cours de construction (MM_Begin/MM_End) et suivi de ses objets.
new mmCourante = -1;
new bool:mmSuivi = false;
new mmObj[MM_MAX_MAPS][MM_MAX_OBJ];
new mmNb[MM_MAX_MAPS];

// ============================================================================
// GEOMETRIE
// ============================================================================
// Meme convention que Prison_RotateOffset() dans afrp_prison.inc (non modifie).
stock MM_RotateOffset(Float:lx, Float:ly, Float:angle, &Float:outDX, &Float:outDY)
{
	outDX = (lx * floatcos(angle, degrees)) - (ly * floatsin(angle, degrees));
	outDY = (lx * floatsin(angle, degrees)) + (ly * floatcos(angle, degrees));
	return 1;
}

// Reexprime un point ecrit dans les donnees d'origine vers sa position reelle
// actuelle. C'est le seul endroit qui connait la transformation : tout ce qui
// doit suivre une map passe par ici plutot que de recopier le calcul.
stock MM_Point(map, Float:x, Float:y, Float:z, &Float:outX, &Float:outY, &Float:outZ)
{
	if(map < 0 || map >= MM_MAX_MAPS || mmDeplacee[map] == 0)
	{
		outX = x; outY = y; outZ = z;
		return 0;
	}
	new Float:dx, Float:dy;
	MM_RotateOffset(x - mmPivotX[map], y - mmPivotY[map], mmAncRot[map], dx, dy);
	outX = mmAncX[map] + dx;
	outY = mmAncY[map] + dy;
	outZ = mmAncZ[map] + (z - mmPivotZ[map]);
	return 1;
}

stock Float:MM_Angle(map, Float:a)
{
	if(map < 0 || map >= MM_MAX_MAPS || mmDeplacee[map] == 0) return a;
	new Float:r = a + mmAncRot[map];
	while(r >= 360.0) r -= 360.0;
	while(r < 0.0) r += 360.0;
	return r;
}

// Position d'ancrage courante d'une map. Utilise par tout ce qui doit rester
// centre dessus (zone de duty hopital, teleportation "aller voir").
stock Float:MM_AnchorX(map) { return (map < 0 || map >= MM_MAX_MAPS) ? (0.0) : ((mmDeplacee[map] == 1) ? (mmAncX[map]) : (mmPivotX[map])); }
stock Float:MM_AnchorY(map) { return (map < 0 || map >= MM_MAX_MAPS) ? (0.0) : ((mmDeplacee[map] == 1) ? (mmAncY[map]) : (mmPivotY[map])); }
stock Float:MM_AnchorZ(map) { return (map < 0 || map >= MM_MAX_MAPS) ? (0.0) : ((mmDeplacee[map] == 1) ? (mmAncZ[map]) : (mmPivotZ[map])); }

// ============================================================================
// CREATION DES OBJETS
// ============================================================================
// Encadre la construction d'une map. Toute la transformation est portee par
// ces deux appels : les fonctions de map n'ont pas a savoir qu'elles peuvent
// etre deplacees.
stock MM_Begin(map, bool:suivi = true)
{
	mmCourante = map;
	mmSuivi = suivi;
	if(suivi && map >= 0 && map < MM_MAX_MAPS) mmNb[map] = 0;
	return 1;
}

stock MM_End()
{
	mmCourante = -1;
	mmSuivi = false;
	return 1;
}

// Remplace CreateDynamicObject dans les fichiers de map. Meme signature, donc
// le portage des donnees brutes est un simple changement de nom de fonction :
// aucune coordonnee n'a ete retouchee dans les fichiers de map.
stock MM_Obj(model, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, wid = -1, iid = -1, pid = -1, Float:sd = 300.0, Float:dd = 300.0)
{
	new Float:nx = x, Float:ny = y, Float:nz = z, Float:nrz = rz;
	if(mmCourante >= 0)
	{
		MM_Point(mmCourante, x, y, z, nx, ny, nz);
		nrz = MM_Angle(mmCourante, rz);
	}
	new objectid = CreateDynamicObject(model, nx, ny, nz, rx, ry, nrz, wid, iid, pid, sd, dd);
	if(mmSuivi && mmCourante >= 0 && mmCourante < MM_MAX_MAPS && mmNb[mmCourante] < MM_MAX_OBJ)
		mmObj[mmCourante][mmNb[mmCourante]++] = objectid;
	return objectid;
}

stock MM_Detruire(map)
{
	if(map < 0 || map >= MM_MAX_MAPS) return 0;
	new n = 0;
	for(new k = 0; k < mmNb[map]; k++)
	{
		if(mmObj[map][k] > 0 && IsValidDynamicObject(mmObj[map][k]))
		{
			DestroyDynamicObject(mmObj[map][k]);
			n++;
		}
		mmObj[map][k] = 0;
	}
	mmNb[map] = 0;
	return n;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock MM_Save()
{
	if(fexist(MM_FILE)) fremove(MM_FILE);
	new File:f = fopen(MM_FILE, io_write);
	if(!f) return 0;
	new line[160];
	for(new m = 0; m < MM_MAX_MAPS; m++)
	{
		if(mmDeplacee[m] != 1) continue;
		format(line, sizeof(line), "%d|%f|%f|%f|%f|%f\n", m, mmAncX[m], mmAncY[m], mmAncZ[m], mmAncRot[m], mmClearRadius[m]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

// Decoupe "champ|champ|..." sans dependre d'un include externe : le fichier
// est ecrit par MM_Save juste au-dessus, le format ne bougera pas.
stock MM_Champ(const line[], num, sortie[], taille)
{
	// Noms prefixes : "pos" est deja une constante d'enumeration globale dans
	// LVRP.pwn (dealerShip[i][pos]...), une variable locale du meme nom ne
	// compile pas.
	new mmI = 0, mmChamp = 0, mmK = 0;
	sortie[0] = 0;
	while(line[mmI] && line[mmI] != '\r' && line[mmI] != '\n')
	{
		if(line[mmI] == '|') { mmChamp++; mmI++; continue; }
		if(mmChamp == num && mmK < taille - 1) sortie[mmK++] = line[mmI];
		mmI++;
	}
	sortie[mmK] = 0;
	return mmK;
}

stock MM_Load()
{
	for(new m = 0; m < MM_MAX_MAPS; m++)
	{
		mmDeplacee[m] = 0;
		mmAncX[m] = mmPivotX[m];
		mmAncY[m] = mmPivotY[m];
		mmAncZ[m] = mmPivotZ[m];
		mmAncRot[m] = 0.0;
		mmClearRadius[m] = MM_CLEAR_RADIUS_DEFAUT;
		mmNb[m] = 0;
	}
	if(!fexist(MM_FILE)) return 0;
	new File:f = fopen(MM_FILE, io_read);
	if(!f) return 0;

	new line[160], buf[32], n = 0;
	while(fread(f, line))
	{
		MM_Champ(line, 0, buf, sizeof(buf));
		new m = strval(buf);
		if(m < 0 || m >= MM_MAX_MAPS) continue;
		MM_Champ(line, 1, buf, sizeof(buf)); mmAncX[m] = floatstr(buf);
		MM_Champ(line, 2, buf, sizeof(buf)); mmAncY[m] = floatstr(buf);
		MM_Champ(line, 3, buf, sizeof(buf)); mmAncZ[m] = floatstr(buf);
		MM_Champ(line, 4, buf, sizeof(buf)); mmAncRot[m] = floatstr(buf);
		// [COMPAT] Fichiers ecrits avant l'ajout du rayon de nettoyage (5e champ) :
		// MM_Champ rend une chaine vide, floatstr("") = 0.0, qui DESACTIVERAIT le
		// nettoyage pour une map deja deplacee. On garde le defaut dans ce cas.
		mmClearRadius[m] = MM_CLEAR_RADIUS_DEFAUT;
		if(MM_Champ(line, 5, buf, sizeof(buf)) > 0) mmClearRadius[m] = floatstr(buf);
		mmDeplacee[m] = 1;
		n++;
	}
	fclose(f);
	return n;
}

// ============================================================================
// RECONSTRUCTION
// ============================================================================
// Detruit puis recree une map avec l'ancrage courant. Le "quoi detruire" et le
// "quoi recreer" different d'une map a l'autre parce qu'elles n'ont pas toutes
// le meme proprietaire : les commerces ont leur tableau dans
// afrp_commerces_map.inc, le Coffee Shop appartient au pack de mappings.
// Renvoie le nombre d'objets recrees, ou -1 si la map n'est pas posee.
stock MM_Reconstruire(map)
{
	switch(map)
	{
		case MM_COFFEE:
		{
			// Le Coffee Shop est une carte du pack : c'est le pack qui compte
			// ses objets et qui sait si elle est allumee. On lui delegue les
			// deux operations plutot que de tenir un deuxieme inventaire qui
			// finirait par diverger du sien.
			if(!MapPack_EstAllumee(MM_COFFEE_MP)) return -1;
			MapPack_Detruire(MM_COFFEE_MP);
			return MapPack_Creer(MM_COFFEE_MP);
		}
		case MM_HOPITAL:
		{
			MM_Detruire(MM_HOPITAL);
			Hopital_CreateMap();
			return mmNb[MM_HOPITAL];
		}
		case MM_CONCESSION:
		{
			MM_Detruire(MM_CONCESSION);
			Concession_CreateMap();
			Concession_MoveExpo();      // les vehicules de vitrine suivent
			Concession_MovePickup();    // le pickup 'N' aussi (et il est resauvegarde)
			return mmNb[MM_CONCESSION];
		}
		// Les quatre commerces sont delegues a leur propre module : ce fichier
		// est inclus avant afrp_commerces_map.inc, donc ses constantes CMM_*
		// n'existent pas encore ici (le preprocesseur les lit plus loin).
		case MM_MARKET247, MM_MINIMARKET, MM_VETEMENTS, MM_PROJET1:
			return Cmm_Reconstruire(map);
	}
	return -1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock MM_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock MM_Trouver(const cle[])
{
	for(new m = 0; m < MM_MAX_MAPS; m++)
		if(strcmp(cle, mmCle[m], true) == 0) return m;
	return -1;
}

stock MM_Liste(playerid)
{
	MM_Msg(playerid, "{FF6347}\xbb Deplacer une map \xab{FFFFFF} /a deplacer <nom> pose la map a tes pieds, dans la direction ou tu regardes.");
	new s[190];
	for(new m = 0; m < MM_MAX_MAPS; m++)
	{
		if(mmDeplacee[m] == 1)
			format(s, sizeof(s), "{FFFF00}%s{FFFFFF} - %s : deplacee en %.1f / %.1f / %.1f (angle %.0f)", mmCle[m], mmNom[m], mmAncX[m], mmAncY[m], mmAncZ[m], mmAncRot[m]);
		else
			format(s, sizeof(s), "{FFFF00}%s{FFFFFF} - %s : {888888}position d'origine", mmCle[m], mmNom[m]);
		MM_Msg(playerid, s);
	}
	MM_Msg(playerid, "{AAAAAA}/a deplacer reset <nom> remet une map la ou son auteur l'avait mise.");
	MM_Msg(playerid, "{AAAAAA}La prison n'est pas dans la liste : elle vit dans son propre monde, la deplacer casserait /a jail.");
	return 1;
}

// arg1 = mot-clef de la map, ou "reset" ; arg2 = mot-clef si arg1 vaut "reset".
stock MM_Cmd(playerid, const arg1[], const arg2[], const arg3[] = "")
{
	if(PlayerInfo[playerid][pAdmin] < MM_ADMIN_LEVEL)
	{
		new err[160];
		format(err, sizeof(err), "{FF0069}\xbb Erreur \xab{FFFFFF} Reserve aux admins niveau %d (tu es niveau %d).", MM_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return MM_Msg(playerid, err);
	}
	if(!strlen(arg1)) return MM_Liste(playerid);

	// [DIAGNOSTIC MAP INVISIBLE] "aller" teleporte exactement au point d'ancrage
	// courant de la map et dit combien d'objets y ont ete crees. C'est ce qui
	// permet de trancher sans voir le jeu : s'il y a des objets et qu'on ne voit
	// rien sur place, le probleme est la hauteur (voir "hauteur" ci-dessous) ;
	// s'il n'y en a aucun, c'est la reconstruction qui a echoue.
	if(strcmp(arg1, "aller", true) == 0)
	{
		new mapA = MM_Trouver(arg2);
		if(mapA == -1) { MM_Msg(playerid, "{FF4444}Nom de map inconnu."); return MM_Liste(playerid); }

		SetPlayerPos(playerid, MM_AnchorX(mapA), MM_AnchorY(mapA), MM_AnchorZ(mapA) + 3.0);
		SetPlayerInterior(playerid, 0);
		SetPlayerVirtualWorld(playerid, 0);

		new sa[190];
		format(sa, sizeof(sa), "{00FF00}Sur le point d'ancrage de %s : %.1f / %.1f / %.1f", mmNom[mapA], MM_AnchorX(mapA), MM_AnchorY(mapA), MM_AnchorZ(mapA));
		MM_Msg(playerid, sa);
		format(sa, sizeof(sa), "{FFFF00}Objets suivis pour cette map : %d{FFFFFF} (0 = la map n'a pas ete reconstruite ici).", mmNb[mapA]);
		MM_Msg(playerid, sa);
		MM_Msg(playerid, "{AAAAAA}Tu ne vois rien mais tu te cognes ? La map est enterree : /a deplacer hauteur <nom> <valeur> pour la remonter.");
		return 1;
	}

	// [FIX MAP ENTERREE] Le point de reference en hauteur de chaque map a ete
	// ESTIME (hauteur la plus frequente de ses objets), jamais mesure en jeu.
	// Si cette estimation tombe sur le milieu des murs plutot que sur le sol,
	// la map se pose trop bas : le rez-de-chaussee passe sous le terrain, on ne
	// voit plus rien mais on se cogne toujours dedans - exactement le symptome
	// remonte. Impossible de corriger a l'aveugle depuis ici : cette commande
	// donne la molette a celle qui voit le jeu.
	if(strcmp(arg1, "hauteur", true) == 0)
	{
		new mapH = MM_Trouver(arg2);
		if(mapH == -1 || !strlen(arg3))
		{
			MM_Msg(playerid, "{FF4444}Usage : {FFFFFF}/a deplacer hauteur <nom> <valeur>");
			MM_Msg(playerid, "{AAAAAA}Valeur positive = remonter la map, negative = la descendre. Ex : 2.5 ou -1");
			return MM_Liste(playerid);
		}

		new Float:delta = floatstr(arg3);
		if(delta > -0.001 && delta < 0.001)
			return MM_Msg(playerid, "{FF4444}Valeur nulle : indique de combien remonter (ex 2.5) ou descendre (ex -1).");

		// Une map jamais deplacee a ancrage == pivot : on l'inscrit comme
		// deplacee pour que la correction de hauteur soit conservee elle aussi.
		if(mmDeplacee[mapH] == 0)
		{
			mmAncX[mapH] = mmPivotX[mapH];
			mmAncY[mapH] = mmPivotY[mapH];
			mmAncRot[mapH] = 0.0;
			mmDeplacee[mapH] = 1;
		}
		mmAncZ[mapH] += delta;

		new nH = MM_Reconstruire(mapH);
		MM_Save();

		new sh[190];
		if(nH == -1)
			format(sh, sizeof(sh), "{FFFF00}%s : hauteur enregistree, mais la carte est eteinte (/mappack).", mmNom[mapH]);
		else
			format(sh, sizeof(sh), "{00FF00}%s %s de %.2f m (%d objets reposes). Nouvelle hauteur : %.2f", mmNom[mapH], (delta > 0.0) ? ("remontee") : ("descendue"), floatabs(delta), nH, mmAncZ[mapH]);
		MM_Msg(playerid, sh);
		MM_Msg(playerid, "{AAAAAA}Repete la commande jusqu'a ce que le sol de la map affleure. /a deplacer reset <nom> annule tout.");
		return 1;
	}

	// [NETTOYAGE AUTO] Regle le rayon de retrait des batiments GTA autour de
	// l'ancrage actuel (voir MM_NettoyerBatiments, en-tete du fichier). 0 =
	// desactive le nettoyage pour cette map (les batiments GTA du nouvel
	// endroit resteront visibles).
	if(strcmp(arg1, "nettoyer", true) == 0)
	{
		new mapN = MM_Trouver(arg2);
		if(mapN == -1 || !strlen(arg3))
		{
			MM_Msg(playerid, "{FF4444}Usage : {FFFFFF}/a deplacer nettoyer <nom> <rayon>");
			MM_Msg(playerid, "{AAAAAA}Rayon en metres autour du point d'ancrage. 0 desactive le nettoyage pour cette map.");
			return MM_Liste(playerid);
		}
		new Float:rayonN = floatstr(arg3);
		if(rayonN < 0.0) return MM_Msg(playerid, "{FF4444}Le rayon ne peut pas etre negatif.");
		mmClearRadius[mapN] = rayonN;
		MM_Save();

		new sn[190];
		if(rayonN <= 0.0)
			format(sn, sizeof(sn), "{00FF00}%s : nettoyage automatique desactive.", mmNom[mapN]);
		else
			format(sn, sizeof(sn), "{00FF00}%s : rayon de nettoyage regle a %.0f m.", mmNom[mapN], rayonN);
		MM_Msg(playerid, sn);
		MM_Msg(playerid, "{AAAAAA}Reconnecte-toi pour appliquer le nouveau rayon (SA-MP ne retire les batiments qu'a la connexion).");
		return 1;
	}

	new bool:reset = (strcmp(arg1, "reset", true) == 0 || strcmp(arg1, "annuler", true) == 0);
	new map = reset ? MM_Trouver(arg2) : MM_Trouver(arg1);
	if(map == -1)
	{
		MM_Msg(playerid, "{FF4444}Nom de map inconnu.");
		return MM_Liste(playerid);
	}

	new s[190];
	if(reset)
	{
		if(mmDeplacee[map] == 0)
		{
			format(s, sizeof(s), "{FFFF00}%s est deja a sa position d'origine.", mmNom[map]);
			return MM_Msg(playerid, s);
		}
		mmDeplacee[map] = 0;
		mmAncX[map] = mmPivotX[map];
		mmAncY[map] = mmPivotY[map];
		mmAncZ[map] = mmPivotZ[map];
		mmAncRot[map] = 0.0;
	}
	else
	{
		new Float:px, Float:py, Float:pz, Float:pa;
		GetPlayerPos(playerid, px, py, pz);
		GetPlayerFacingAngle(playerid, pa);
		mmAncX[map] = px;
		mmAncY[map] = py;
		// GetPlayerPos rend le centre du joueur, environ un metre au-dessus du
		// sol. Sans cette correction chaque deplacement ferait monter la map
		// d'un metre, et deux deplacements de deux metres.
		mmAncZ[map] = pz - 1.0;
		mmAncRot[map] = pa;
		mmDeplacee[map] = 1;
	}

	new n = MM_Reconstruire(map);
	MM_Save();

	if(n == -1)
	{
		format(s, sizeof(s), "{FFFF00}%s : position enregistree, mais la carte est eteinte (/mappack). Elle sera posee au bon endroit en l'allumant.", mmNom[map]);
		return MM_Msg(playerid, s);
	}
	if(reset)
		format(s, sizeof(s), "{00FF00}%s remise a sa position d'origine (%d objets recrees).", mmNom[map], n);
	else
		format(s, sizeof(s), "{00FF00}%s deplacee ici (%d objets recrees, angle %.0f).", mmNom[map], n, mmAncRot[map]);
	MM_Msg(playerid, s);
	if(reset)
	{
		MM_Msg(playerid, "{AAAAAA}Reconnecte-toi pour revoir les batiments GTA d'origine a cette adresse (SA-MP ne les remet qu'a la connexion).");
	}
	else
	{
		format(s, sizeof(s), "{AAAAAA}Les batiments GTA du nouvel endroit seront retires automatiquement (rayon %.0f m) a la PROCHAINE connexion. Pas assez ou trop retire ? /a deplacer nettoyer %s <rayon>.", mmClearRadius[map], mmCle[map]);
		MM_Msg(playerid, s);
	}
	return 1;
}

// ============================================================================
// NETTOYAGE AUTOMATIQUE DES BATIMENTS GTA (voir en-tete du fichier)
// ============================================================================
// A appeler a OnPlayerConnect, apres Hopital_MapRemoveBuildings et
// Commerces_MapRemoveBuildings (leurs listes precises, a l'ancienne adresse,
// restent utiles et ne font pas double emploi : elles visent l'ANCIEN
// emplacement, celui-ci vise le NOUVEL emplacement de chaque map DEPLACEE).
stock MM_NettoyerBatiments(playerid)
{
	new bool:mmZoneFaite[MM_MAX_MAPS];
	new n = 0;
	for(new m = 0; m < MM_MAX_MAPS; m++)
	{
		if(mmDeplacee[m] != 1 || mmClearRadius[m] <= 0.0 || mmZoneFaite[m]) continue;

		new Float:cx = MM_AnchorX(m), Float:cy = MM_AnchorY(m), Float:cz = MM_AnchorZ(m);
		new Float:cr = mmClearRadius[m];

		// [ANTI CRASH] Fusionne avec toute autre map deplacee dont le cercle de
		// nettoyage chevauche celui-ci : appeler RemoveBuildingForPlayer(-1,...)
		// deux fois sur une zone qui se recoupe reciblerait les memes vrais
		// batiments deux fois, ce que la doc SA-MP dit explicitement faire
		// planter le client. Cas reel constate le 5 aout (Hopital et Coffee
		// Shop a environ 1 m l'un de l'autre).
		for(new m2 = m + 1; m2 < MM_MAX_MAPS; m2++)
		{
			if(mmDeplacee[m2] != 1 || mmClearRadius[m2] <= 0.0 || mmZoneFaite[m2]) continue;
			new Float:dx = MM_AnchorX(m2) - cx, Float:dy = MM_AnchorY(m2) - cy;
			new Float:dist = floatsqroot(dx * dx + dy * dy);
			if(dist < cr + mmClearRadius[m2])
			{
				new Float:besoin = dist + mmClearRadius[m2];
				if(besoin > cr) cr = besoin;
				mmZoneFaite[m2] = true;
			}
		}

		RemoveBuildingForPlayer(playerid, -1, cx, cy, cz, cr);
		mmZoneFaite[m] = true;
		n++;
	}
	return n;
}

// ============================================================================
// INIT
// ============================================================================
// A appeler AVANT les _CreateMap() d'OnGameModeInit : les maps doivent naitre
// directement au bon endroit, pas etre posees puis deplacees.
stock MM_Init()
{
	new n = MM_Load();
	printf("[DEPLACER MAP] %d map(s) deplacee(s) sur %d.", n, MM_MAX_MAPS);
	return n;
}

// [DIAGNOSTIC MAP INVISIBLE] A appeler a la FIN d'OnGameModeInit, une fois
// toutes les maps creees (y compris le Coffee Shop, qui nait avec le pack de
// mappings, plus tard que les autres).
// Raison d'etre : le symptome "la map est deplacee mais on ne la voit pas" ne
// peut pas etre tranche a distance. Ce recapitulatif ecrit dans server_log.txt
// l'ancrage reel ET le nombre d'objets reellement construits pour chaque map,
// donc un simple envoi de log suffit a savoir si les objets existent (donc
// probleme de hauteur/emplacement) ou si la reconstruction a echoue (0 objet).
// Cela evite de faire taper une commande a quelqu'un qui joue deja.
stock MM_LogEtat()
{
	print("[DEPLACER MAP] --- Etat des maps deplacables ---");
	for(new m = 0; m < MM_MAX_MAPS; m++)
	{
		// Le Coffee Shop est compte par le pack de mappings, pas ici : son
		// mmNb reste a 0 par construction (MM_Begin(..., false)), ne pas le
		// lire comme un echec.
		if(m == MM_COFFEE)
		{
			printf("[DEPLACER MAP] %-28s %s | ancre %.1f / %.1f / %.1f | objets comptes par /mappack",
				mmNom[m], (mmDeplacee[m] == 1) ? ("DEPLACEE") : ("origine  "),
				MM_AnchorX(m), MM_AnchorY(m), MM_AnchorZ(m));
			continue;
		}
		printf("[DEPLACER MAP] %-28s %s | ancre %.1f / %.1f / %.1f | %d objet(s) crees",
			mmNom[m], (mmDeplacee[m] == 1) ? ("DEPLACEE") : ("origine  "),
			MM_AnchorX(m), MM_AnchorY(m), MM_AnchorZ(m), mmNb[m]);
	}
	print("[DEPLACER MAP] --- 0 objet sur une map DEPLACEE = reconstruction ratee ---");
	return 1;
}
