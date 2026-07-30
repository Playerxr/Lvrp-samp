/*
	afrp_basket.inc - Basket : dribble, paniers et points

	Le serveur avait deja tout le materiel du basket : le ballon s'achete au
	24/7, /basket le fait apparaitre, on le ramasse et on se le passe. Mais la
	partie qui rend le jeu jouable n'a jamais tourne :

	  - la boucle de dribble et de rebond etait ENTIEREMENT commentee dans
	    OnDynamicObjectMoved. Le ballon ne suivait donc pas le joueur ;
	  - il n'y avait aucun panier : impossible de marquer ;
	  - BallDown2 a BallDown5 renvoyaient le ballon a des coordonnees ecrites
	    en dur, heritees d'un autre serveur, et ne servaient a personne.

	Ce module remet le basket en marche, avec des paniers poses en jeu plutot
	qu'ecrits dans le code.

	  /basketpanier 1        enregistre le premier panier sous tes pieds
	  /basketpanier 2        enregistre le second
	  /basketpanier infos    ou en sont les paniers
	  /basketpanier effacer  oublie les deux paniers

	=== POURQUOI LES PANIERS SONT POSES EN JEU ===
	Le code d'origine visait un terrain qui n'existe pas sur ce serveur. Ecrire
	de nouvelles coordonnees a l'aveugle aurait refait la meme erreur : on se
	place sous l'anneau, on tape la commande, et le panier est la ou il est
	vraiment. Le terrain lui-meme se construit avec /mapedit.

	=== TROIS BUGS CORRIGES DANS LE CODE EXISTANT ===
	1. Dans OnPlayerKeyStateChange, la recherche du ballon tenu lisait
	   basket[tmpBall][owner] avec tmpBall encore a -1 : lecture hors tableau,
	   et le ballon n'etait jamais trouve. Tirer ne marchait pas.
	2. BallDown2 a BallDown5 posaient owner a 999 alors que MAX_PLAYERS vaut
	   100 : l'ecriture suivante dans basket_HavingBall[999] sortait du tableau.
	3. La boucle commentee parcourait MAX_BALL+1 ballons pour un tableau de
	   MAX_BALL, et traitait TOUS les ballons a chaque objet deplace sur le
	   serveur, ascenseur et portails compris. Elle ne traite plus que le
	   ballon concerne.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit          -> Bsk_Init()
	- OnPlayerConnect         -> Bsk_OnConnect(playerid)
	- OnPlayerDisconnect      -> Bsk_OnDisconnect(playerid)
	- OnDynamicObjectMoved    -> if(Bsk_OnObjectMoved(objectid)) return 1;
	- KEY_FIRE, ballon en main -> if(Bsk_Tirer(playerid, tmpBall)) return 1;
	- BallDown2..5            -> Bsk_Retour(playerid, ballid, panier, reussi)
	- /basketpanier           -> Bsk_Cmd(playerid, cmdtext)
*/

#if defined AFRP_BASKET_INCLUDED
	#endinput
#endif
#define AFRP_BASKET_INCLUDED

#define BSK_ADMIN_LEVEL     3
#define BSK_FILE            "basket_paniers.cfg"
#define BSK_NB_PANIER       2

// Hauteur de l'anneau au-dessus du sol. Valeur reprise du systeme d'origine,
// qui envoyait le ballon 1.83 m plus haut que le sol du terrain.
#define BSK_HAUTEUR_ANNEAU  1.83

// Portees de tir et chances de reussite. Le code d'origine utilisait
// random(1) pour le tir a 4 m, ce qui renvoie toujours 0 : le tir rentrait
// donc a 100%. Chaque distance a maintenant une vraie chance.
#define BSK_DIST_DUNK       2.0
#define BSK_DIST_PROCHE     4.0
#define BSK_DIST_MOYEN      7.0
#define BSK_DIST_LOIN       10.0
#define BSK_CHANCE_PROCHE   2     // 1 sur 2
#define BSK_CHANCE_MOYEN    3     // 1 sur 3
#define BSK_CHANCE_LOIN     4     // 1 sur 4

new Float:bskPanierX[BSK_NB_PANIER], Float:bskPanierY[BSK_NB_PANIER], Float:bskPanierZ[BSK_NB_PANIER];
new bskPanierPose[BSK_NB_PANIER];

// ============================================================================
// OUTILS
// ============================================================================
stock Bsk_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// owner vaut -1 quand personne ne tient le ballon. Tout le reste doit etre un
// slot joueur reel : MAX_PLAYERS vaut 100 ici, une valeur comme 999 ecrasait
// la memoire au premier acces.
stock Bsk_JoueurValide(o)
{
	if(o < 0 || o >= MAX_PLAYERS) return 0;
	if(!IsPlayerConnected(o)) return 0;
	return 1;
}

stock Bsk_BalleParObjet(objectid)
{
	if(objectid <= 0) return -1;
	for(new i = 0; i < MAX_BALL; i++)
	{
		if(basket[i][used] == 0) continue;
		if(basket[i][object] == objectid) return i;
	}
	return -1;
}

stock Bsk_PanierPret()
{
	for(new p = 0; p < BSK_NB_PANIER; p++) if(bskPanierPose[p] == 1) return 1;
	return 0;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Bsk_Save()
{
	if(fexist(BSK_FILE)) fremove(BSK_FILE);
	new File:f = fopen(BSK_FILE, io_write);
	if(!f) return 0;
	new line[96];
	for(new p = 0; p < BSK_NB_PANIER; p++)
	{
		format(line, sizeof(line), "%d|%.4f|%.4f|%.4f\n", bskPanierPose[p], bskPanierX[p], bskPanierY[p], bskPanierZ[p]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Bsk_Champ(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Bsk_Load()
{
	for(new p = 0; p < BSK_NB_PANIER; p++) bskPanierPose[p] = 0;
	if(!fexist(BSK_FILE)) return 0;
	new File:f = fopen(BSK_FILE, io_read);
	if(!f) return 0;

	new line[96], buf[32], n = 0, p = 0;
	while(fread(f, line) && p < BSK_NB_PANIER)
	{
		if(strlen(line) < 3) { p++; continue; }
		new k = 0;
		Bsk_Champ(line, k, buf, sizeof(buf)); bskPanierPose[p] = strval(buf);
		Bsk_Champ(line, k, buf, sizeof(buf)); bskPanierX[p]    = floatstr(buf);
		Bsk_Champ(line, k, buf, sizeof(buf)); bskPanierY[p]    = floatstr(buf);
		Bsk_Champ(line, k, buf, sizeof(buf)); bskPanierZ[p]    = floatstr(buf);
		if(bskPanierPose[p] == 1) n++;
		p++;
	}
	fclose(f);
	return n;
}

// ============================================================================
// RETOUR DU BALLON APRES UN TIR
// ============================================================================
// Appele par BallDown2 a BallDown5 : le ballon a fini sa course vers l'anneau.
stock Bsk_Retour(playerid, ballid, panier, reussi)
{
	if(ballid < 0 || ballid >= MAX_BALL) return 0;
	if(basket[ballid][used] == 0) return 0;
	if(panier < 0 || panier >= BSK_NB_PANIER) panier = 0;

	new Float:x, Float:y, Float:z;
	if(bskPanierPose[panier] == 1)
	{
		x = bskPanierX[panier];
		y = bskPanierY[panier];
		z = bskPanierZ[panier];
	}
	else
	{
		// Panier efface entre le tir et son arrivee : le ballon retombe la ou
		// il est plutot que de partir a l'autre bout de la carte.
		GetDynamicObjectPos(basket[ballid][object], x, y, z);
	}

	if(!reussi)
	{
		// Un tir rate ne revient pas pile sous l'anneau.
		x += float(random(5)) - 2.0;
		y += float(random(5)) - 2.0;
	}

	MoveDynamicObject(basket[ballid][object], x, y, z - 0.8, 10.0 + random(3));
	basket[ballid][owner]    = -1;
	basket[ballid][shooting] = 0;
	basket[ballid][bounce]   = 1;

	if(Bsk_JoueurValide(playerid))
	{
		if(reussi) GameTextForPlayer(playerid, "~g~Panier !", 3000, 3);
		else       GameTextForPlayer(playerid, "~r~Rate !", 3000, 3);
	}
	return 1;
}

// ============================================================================
// TIR AU PANIER
// ============================================================================
// Renvoie 1 si le joueur a tire : l'appelant ne doit alors pas enchainer sur
// la passe ni sur le lancer en avant.
stock Bsk_Tirer(playerid, ballid)
{
	if(!Bsk_JoueurValide(playerid)) return 0;
	if(ballid < 0 || ballid >= MAX_BALL) return 0;
	if(basket[ballid][used] == 0) return 0;
	if(!Bsk_PanierPret()) return 0;

	for(new p = 0; p < BSK_NB_PANIER; p++)
	{
		if(bskPanierPose[p] != 1) continue;

		new Float:px = bskPanierX[p], Float:py = bskPanierY[p], Float:pz = bskPanierZ[p];
		new Float:ax = px, Float:ay = py, Float:az = pz + BSK_HAUTEUR_ANNEAU;

		// Dunk : au contact, ca rentre toujours.
		if(IsPlayerInRangeOfPoint(playerid, BSK_DIST_DUNK, px, py, pz))
		{
			MoveDynamicObject(basket[ballid][object], ax, ay, az, 7.5);
			ApplyAnimation(playerid, "BSKTBALL", "BBALL_Dnk", 4.0, 1, 0, 0, 0, 0);
			basket_HavingBall[playerid] = 0;
			basket[ballid][shooting] = (p == 0) ? (2) : (3);
			SetTimerEx("ClearAnim", 1100, 0, "d", playerid);
			return 1;
		}

		// Tirs a distance : il faut viser le panier.
		new chance = 0;
		if(IsPlayerInRangeOfPoint(playerid, BSK_DIST_PROCHE, px, py, pz))     chance = BSK_CHANCE_PROCHE;
		else if(IsPlayerInRangeOfPoint(playerid, BSK_DIST_MOYEN, px, py, pz)) chance = BSK_CHANCE_MOYEN;
		else if(IsPlayerInRangeOfPoint(playerid, BSK_DIST_LOIN, px, py, pz))  chance = BSK_CHANCE_LOIN;
		if(chance == 0) continue;
		if(!IsPlayerFacingPoint(playerid, 20.0, px, py, pz)) continue;

		new reussi = (random(chance) == 0);
		ApplyAnimation(playerid, "BSKTBALL", "BBALL_Jump_Shot", 4.0, 0, 0, 0, 0, 0);
		basket_HavingBall[playerid] = 0;

		if(reussi)
		{
			MoveDynamicObject(basket[ballid][object], ax, ay, az, 10.5 + random(4));
			basket[ballid][shooting] = (p == 0) ? (2) : (3);
		}
		else
		{
			// Le ballon tape la planche et repart : il monte a cote de l'anneau.
			MoveDynamicObject(basket[ballid][object], ax + 1.0, ay + 1.0, az + float(random(2)), 12.5 + random(4));
			basket[ballid][shooting] = (p == 0) ? (4) : (5);
		}
		return 1;
	}
	return 0;
}

// ============================================================================
// DRIBBLE ET REBOND
// ============================================================================
// Renvoie 1 si l'objet deplace est un ballon de basket : l'appelant s'arrete.
stock Bsk_OnObjectMoved(objectid)
{
	new i = Bsk_BalleParObjet(objectid);
	if(i == -1) return 0;

	new Float:x, Float:y, Float:z;

	// Un tir en cours d'arrivee prime sur tout le reste.
	switch(basket[i][shooting])
	{
		case 2: { Bsk_Retour(basket[i][owner], i, 0, 1); return 1; }
		case 3: { Bsk_Retour(basket[i][owner], i, 1, 1); return 1; }
		case 4: { Bsk_Retour(basket[i][owner], i, 0, 0); return 1; }
		case 5: { Bsk_Retour(basket[i][owner], i, 1, 0); return 1; }
		case 6:
		{
			// Passe arrivee : le receveur prend le ballon.
			new dest = basket[i][owner];
			if(Bsk_JoueurValide(dest))
			{
				ApplyAnimation(dest, "BSKTBALL", "BBALL_walk", 4.1, 1, 1, 1, 1, 1);
				basket_HavingBall[dest] = 1;
				basket_Anim[dest] = 0;
			}
			basket[i][shooting] = 0;
		}
	}

	// Sequence de rebond : le ballon perd de la hauteur a chaque bond.
	if(basket[i][bounce] >= 1 && basket[i][bounce] <= 8)
	{
		// Les vitesses doivent etre des flottants : Pawn ne convertit pas un
		// entier passe a un parametre Float:, il en reinterprete les bits, et
		// le ballon ne bougerait plus.
		new Float:hauteurs[8] = { 1.2, -1.2, 0.8, -0.8, 0.5, -0.5, 0.2, -0.2 };
		new Float:vitesses[8] = { 4.0, 4.0, 3.0, 3.0, 2.0, 2.0, 1.0, 1.0 };
		new b = basket[i][bounce] - 1;
		GetDynamicObjectPos(basket[i][object], x, y, z);
		MoveDynamicObject(basket[i][object], x, y, z + hauteurs[b], vitesses[b]);
		basket[i][bounce] = (basket[i][bounce] == 8) ? (0) : (basket[i][bounce] + 1);
		return 1;
	}

	// Dribble : le ballon suit le porteur tant qu'il le tient.
	new porteur = basket[i][owner];
	if(!Bsk_JoueurValide(porteur)) return 1;
	if(basket_HavingBall[porteur] != 1) return 1;

	new keys, ud, lr;
	GetPlayerKeys(porteur, keys, ud, lr);
	GetPlayerPos(porteur, x, y, z);

	// Distance devant le joueur et vitesse du ballon selon ce qu'il fait.
	new Float:devant = 1.2, Float:vitesse = 5.0;
	if(basket_Anim[porteur])
	{
		devant = 0.4; vitesse = 5.5;   // immobile, dribble serre
	}
	else if(keys & KEY_SPRINT)
	{
		devant = 1.5; vitesse = 8.0;
		ApplyAnimation(porteur, "BSKTBALL", "BBALL_run", 4.1, 1, 1, 1, 1, 1);
	}
	else
	{
		ApplyAnimation(porteur, "BSKTBALL", "BBALL_walk", 4.1, 1, 1, 1, 1, 1);
	}

	new Float:x2, Float:y2;
	GetXYInFrontOfPlayer(porteur, x2, y2, devant);
	StopDynamicObject(basket[i][object]);

	// Le ballon alterne entre le haut et le bas du rebond.
	if(basket[i][statut] == 0)
	{
		basket[i][statut] = 1;
		MoveDynamicObject(basket[i][object], x2, y2, z + 0.1, vitesse);
	}
	else
	{
		basket[i][statut] = 0;
		MoveDynamicObject(basket[i][object], x2, y2, z - 0.8, vitesse);
	}
	return 1;
}

// ============================================================================
// COMMANDE ADMIN
// ============================================================================
stock Bsk_Infos(playerid)
{
	new s[190];
	for(new p = 0; p < BSK_NB_PANIER; p++)
	{
		if(bskPanierPose[p] == 1)
			format(s, sizeof(s), "{FFFFFF}Panier %d : {00FF00}pose{FFFFFF} en %.1f %.1f %.1f", p + 1, bskPanierX[p], bskPanierY[p], bskPanierZ[p]);
		else
			format(s, sizeof(s), "{FFFFFF}Panier %d : {FF8888}pas encore pose", p + 1);
		Bsk_Msg(playerid, s);
	}
	if(!Bsk_PanierPret())
		Bsk_Msg(playerid, "{AAAAAA}Sans panier, on peut dribbler et se faire des passes, mais pas marquer.");
	return 1;
}

stock Bsk_Cmd(playerid, const cmdline[])
{
	if(PlayerInfo[playerid][pAdmin] < BSK_ADMIN_LEVEL)
	{
		new s[160];
		format(s, sizeof(s), "{FF4444}Reserve aux admins niveau %d (tu es niveau %d).", BSK_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return Bsk_Msg(playerid, s);
	}

	// Un seul argument attendu : on saute la commande et on lit le mot suivant.
	new k = 0, mot[16];
	while(cmdline[k] && cmdline[k] != ' ') k++;
	while(cmdline[k] == ' ') k++;
	new p = 0;
	while(cmdline[k] && cmdline[k] != ' ' && p < 15) mot[p++] = cmdline[k++];
	mot[p] = 0;

	if(!strlen(mot))
	{
		Bsk_Msg(playerid, "{FFFF00}--- Paniers de basket ---");
		Bsk_Msg(playerid, "{FFFFFF}/basketpanier 1 {AAAAAA}: enregistre le premier panier sous tes pieds");
		Bsk_Msg(playerid, "{FFFFFF}/basketpanier 2 {AAAAAA}: enregistre le second");
		Bsk_Msg(playerid, "{FFFFFF}/basketpanier infos | effacer");
		Bsk_Msg(playerid, "{AAAAAA}Place-toi juste sous l'anneau avant d'enregistrer.");
		return Bsk_Infos(playerid);
	}

	if(strcmp(mot, "infos", true) == 0) return Bsk_Infos(playerid);

	if(strcmp(mot, "effacer", true) == 0)
	{
		for(new n = 0; n < BSK_NB_PANIER; n++) bskPanierPose[n] = 0;
		Bsk_Save();
		return Bsk_Msg(playerid, "{FFFF00}Les deux paniers sont oublies. On ne peut plus marquer.");
	}

	new num = strval(mot);
	if(num < 1 || num > BSK_NB_PANIER)
		return Bsk_Msg(playerid, "{FF4444}Usage : /basketpanier 1 | 2 | infos | effacer");

	new idx = num - 1;
	GetPlayerPos(playerid, bskPanierX[idx], bskPanierY[idx], bskPanierZ[idx]);
	bskPanierPose[idx] = 1;
	Bsk_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}Panier %d enregistre en %.1f %.1f %.1f.", num, bskPanierX[idx], bskPanierY[idx], bskPanierZ[idx]);
	Bsk_Msg(playerid, s);
	Bsk_Msg(playerid, "{AAAAAA}L'anneau est place 1.83 m au-dessus de ce point.");
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
// Un joueur qui part en tenant le ballon le laissait bloque sur son slot : le
// suivant a occuper ce slot en aurait herite.
stock Bsk_Liberer(playerid)
{
	for(new i = 0; i < MAX_BALL; i++)
	{
		if(basket[i][used] == 0) continue;
		if(basket[i][owner] != playerid) continue;
		basket[i][owner]    = -1;
		basket[i][shooting] = 0;
		basket[i][statut]   = 0;
	}
	return 1;
}

stock Bsk_OnConnect(playerid)
{
	Bsk_Liberer(playerid);
	basket_HavingBall[playerid] = 0;
	basket_Anim[playerid]       = 0;
	return 1;
}

stock Bsk_OnDisconnect(playerid)
{
	Bsk_Liberer(playerid);
	basket_HavingBall[playerid] = 0;
	basket_Anim[playerid]       = 0;
	return 1;
}

stock Bsk_Init()
{
	new n = Bsk_Load();
	printf("[Basket] %d panier(s) charge(s) sur %d.", n, BSK_NB_PANIER);
	return 1;
}
