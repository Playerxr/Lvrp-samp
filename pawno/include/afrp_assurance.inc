/*
	afrp_assurance.inc - Assurance vehicule

	L'assurance existait pour les maisons, pas pour les vehicules. Le joueur
	assure sa voiture pour une semaine et n'a plus a payer le prix fort quand
	elle finit en fourriere ou en carcasse.

	  /assurance   -> menu (souscrire, voir ses contrats, resilier)

	=== CE QUE COUVRE LE CONTRAT ===
	  1. Fourriere : le supplement de 25.000$ tombe a une franchise de 5.000$.
	  2. Epave : un vehicule assure detruit ressort REPARE a son prochain
	     spawn, au lieu d'etre "Hors Service" jusqu'a ce qu'un mecanicien
	     passe. Sans assurance, sous 450 points de coque le moteur refuse
	     de demarrer.

	=== POURQUOI AUCUNE INDEMNITE EN ARGENT ===
	C'est le piege evident de ce genre de systeme. Ici un vehicule detruit
	n'est PAS perdu : il revient avec /parking. Verser une indemnite pour une
	epave reviendrait donc a payer le joueur pour casser sa propre voiture,
	encore et encore, avec un benefice net a chaque fois. L'assurance ne fait
	que reduire des COUTS : jamais un centime ne sort de l'assureur vers le
	joueur, la boucle a argent est impossible par construction.

	=== TARIF ===
	Prime = 8% de la valeur du vehicule (minimum 5.000$), pour 7 jours.
	Un contrat deja en cours peut etre prolonge : les jours s'ajoutent.

	=== PERSISTANCE ===
	assurances.cfg, indexe sur le SQLID du vehicule : le contrat survit aux
	respawns, aux passages en fourriere et aux redemarrages du serveur.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit   -> Assu_Init()
	- OnVehicleSpawn   -> Assu_OnVehicleSpawn(vehicleid)
	- OnDialogResponse -> Assu_OnDialog(playerid, dialogid, response, listitem)
	- /assurance       -> Assu_Cmd(playerid)
	et dans afrp_parkingpayant.inc : Assu_SupplementFourriere(carSqlid)
	remplace PP_FEE_IMPOUND.
*/

#if defined AFRP_ASSURANCE_INCLUDED
	#endinput
#endif
#define AFRP_ASSURANCE_INCLUDED

#define ASSU_MAX           300         // contrats simultanes
#define ASSU_FILE          "assurances.cfg"
#define ASSU_TAUX          8           // prime = 8% de la valeur
#define ASSU_PRIME_MIN     5000        // prime plancher
#define ASSU_DUREE_JOURS   7
#define ASSU_FRANCHISE     5000        // remplace les 25.000$ de fourriere

// Un joueur possede au plus 6 vehicules (PlayerInfo[][pCar][0..5]) : dimensionner
// la table de correspondance sur ASSU_MAX gaspillait plus d'un Mo de memoire.
#define ASSU_MAP_MAX       24

#define DIALOG_ASSU_MENU   9623
#define DIALOG_ASSU_LISTE  9624
#define DIALOG_ASSU_MES    9625

new assuCarSqlid[ASSU_MAX];
new assuOwnerSqlid[ASSU_MAX];
new assuExpire[ASSU_MAX];              // timestamp de fin
new assuPris[ASSU_MAX];

new assuMap[MAX_PLAYERS][ASSU_MAP_MAX];  // listitem -> index dans vehicle[]

// ============================================================================
// OUTILS
// ============================================================================
stock Assu_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Assu_Find(carSqlid)
{
	if(carSqlid <= 0) return -1;
	for(new k = 0; k < ASSU_MAX; k++)
		if(assuPris[k] == 1 && assuCarSqlid[k] == carSqlid) return k;
	return -1;
}

// Vrai si le vehicule est couvert MAINTENANT.
stock Assu_EstAssure(carSqlid)
{
	new k = Assu_Find(carSqlid);
	if(k == -1) return 0;
	if(assuExpire[k] <= gettime()) return 0;
	return 1;
}

// Supplement a facturer pour sortir de fourriere. Appele par afrp_parkingpayant
// a la place de PP_FEE_IMPOUND : c'est le seul endroit ou le tarif est decide.
stock Assu_SupplementFourriere(carSqlid)
{
	if(Assu_EstAssure(carSqlid)) return ASSU_FRANCHISE;
	return PP_FEE_IMPOUND;
}

stock Assu_Prime(valeur)
{
	new p = (valeur * ASSU_TAUX) / 100;
	if(p < ASSU_PRIME_MIN) p = ASSU_PRIME_MIN;
	return p;
}

// Index dans vehicle[] a partir du SQLID.
stock Assu_VidxParSqlid(carSqlid)
{
	if(carSqlid <= 0) return -1;
	for(new v = 1; v < MAX_CAR; v++)
		if(vehicle[v][used] == 1 && vehicle[v][SQLID] == carSqlid) return v;
	return -1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Assu_Field(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Assu_Save()
{
	if(fexist(ASSU_FILE)) fremove(ASSU_FILE);
	new File:f = fopen(ASSU_FILE, io_write);
	if(!f) return 0;
	new line[96];
	for(new k = 0; k < ASSU_MAX; k++)
	{
		if(assuPris[k] != 1) continue;
		// Un contrat expire libere sa place : sinon le registre se remplissait
		// sans jamais se vider et finissait par refuser toute souscription.
		if(assuExpire[k] <= gettime()) { assuPris[k] = 0; continue; }
		format(line, sizeof(line), "%d|%d|%d\n", assuCarSqlid[k], assuOwnerSqlid[k], assuExpire[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Assu_Load()
{
	for(new k = 0; k < ASSU_MAX; k++) assuPris[k] = 0;
	if(!fexist(ASSU_FILE)) return 0;
	new File:f = fopen(ASSU_FILE, io_read);
	if(!f) return 0;

	new line[96], buf[32], n = 0;
	while(fread(f, line) && n < ASSU_MAX)
	{
		if(strlen(line) < 3) continue;
		new k = 0;
		Assu_Field(line, k, buf, sizeof(buf)); assuCarSqlid[n]   = strval(buf);
		Assu_Field(line, k, buf, sizeof(buf)); assuOwnerSqlid[n] = strval(buf);
		Assu_Field(line, k, buf, sizeof(buf)); assuExpire[n]     = strval(buf);
		if(assuCarSqlid[n] <= 0) continue;
		if(assuExpire[n] <= gettime()) continue;   // deja expire au chargement
		assuPris[n] = 1;
		n++;
	}
	fclose(f);
	printf("[Assurance] %d contrat(s) en cours", n);
	return n;
}

// ============================================================================
// SOUSCRIPTION
// ============================================================================
stock Assu_Souscrire(playerid, vidx)
{
	if(vidx < 1 || vidx >= MAX_CAR || vehicle[vidx][used] != 1)
		return Assu_Msg(playerid, "{FF4444}Vehicule introuvable.");

	new carSqlid = vehicle[vidx][SQLID];
	if(carSqlid <= 0)
		return Assu_Msg(playerid, "{FF4444}Ce vehicule ne peut pas etre assure.");
	if(vehicle[vidx][ownerSQLID] != PlayerInfo[playerid][pSQLID])
		return Assu_Msg(playerid, "{FF4444}Ce vehicule ne t'appartient pas.");

	new valeur = vehicle[vidx][cValue];
	if(valeur <= 0) valeur = ASSU_PRIME_MIN * 10;   // valeur inconnue : tarif plancher
	new prime = Assu_Prime(valeur);

	if(PlayerInfo[playerid][pCash] < prime)
	{
		new s1[176];
		format(s1, sizeof(s1), "{FF4444}Prime de %d$ (8%% de %d$). Tu as %d$.", prime, valeur, PlayerInfo[playerid][pCash]);
		return Assu_Msg(playerid, s1);
	}

	new k = Assu_Find(carSqlid);
	if(k == -1)
	{
		for(new n = 0; n < ASSU_MAX; n++)
			if(assuPris[n] == 0) { k = n; break; }
		if(k == -1) return Assu_Msg(playerid, "{FF4444}Le registre des assurances est plein, previens un admin.");
		assuPris[k]     = 1;
		assuCarSqlid[k] = carSqlid;
		assuExpire[k]   = gettime();
	}
	// Contrat deja en cours : les jours s'ajoutent au lieu de repartir a zero.
	if(assuExpire[k] < gettime()) assuExpire[k] = gettime();
	assuExpire[k]     += ASSU_DUREE_JOURS * 86400;
	assuOwnerSqlid[k]  = PlayerInfo[playerid][pSQLID];

	SafeGivePlayerMoney(playerid, -prime, "Prime d'assurance vehicule");
	Assu_Save();

	new s2[200], jours = (assuExpire[k] - gettime()) / 86400;
	format(s2, sizeof(s2), "{00FF00}Vehicule assure pour %d jour(s). Prime : %d$.", jours, prime);
	Assu_Msg(playerid, s2);
	Assu_Msg(playerid, "{AAAAAA}Fourriere a 5.000$ au lieu de 25.000$, et ta voiture ressort reparee si elle est detruite.");
	return 1;
}

// ============================================================================
// EPAVE REPAREE AU RESPAWN
// ============================================================================
stock Assu_OnVehicleSpawn(vehicleid)
{
	if(vehicleid < 1 || vehicleid >= MAX_CAR) return 0;
	if(vehicle[vehicleid][used] != 1) return 0;
	if(!Assu_EstAssure(vehicle[vehicleid][SQLID])) return 0;

	// Sous 450 points de coque le moteur refuse de demarrer (car_Engine) et il
	// faut un mecanicien. C'est precisement ce que le contrat evite.
	new Float:hp;
	GetVehicleHealth(vehicleid, hp);
	if(hp >= 1000.0) return 0;

	SetVehicleHealth(vehicleid, 1000.0);
	vehicle[vehicleid][cHealth] = 1000.0;

	new pid = INVALID_PLAYER_ID;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pSQLID] == vehicle[vehicleid][ownerSQLID]) { pid = i; break; }
	}
	if(pid != INVALID_PLAYER_ID)
		msg_Client(pid, COLOR_WHITE, "{42AAFF} Assurance {FFFFFF} Ton vehicule assure a ete remis en etat par l'assureur.");
	return 1;
}

// ============================================================================
// MENUS
// ============================================================================
stock Assu_ShowMenu(playerid)
{
	new body[512];
	// Pawn ne concatene pas les chaines sur plusieurs lignes : tout tient ici.
	format(body, sizeof(body), "{AAAAAA}Prime : %d%% de la valeur du vehicule (min %d$) pour %d jours\n{00FF00}Assurer un de mes vehicules\n{FFFFFF}Mes contrats en cours\n{AAAAAA}Ce que couvre le contrat", ASSU_TAUX, ASSU_PRIME_MIN, ASSU_DUREE_JOURS);
	return ShowPlayerDialog(playerid, DIALOG_ASSU_MENU, DIALOG_STYLE_LIST, "Assurance vehicule", body, "Choisir", "Fermer");
}

stock Assu_ShowVehicules(playerid)
{
	new body[1400], str[190], n = 0;
	body[0] = 0;

	for(new v = 1; v < MAX_CAR; v++)
	{
		if(vehicle[v][used] != 1) continue;
		if(vehicle[v][SQLID] <= 0) continue;
		if(vehicle[v][ownerSQLID] != PlayerInfo[playerid][pSQLID]) continue;
		if(n >= ASSU_MAP_MAX) break;

		// Un modele hors 400-611 donnerait un index negatif dans vehName[].
		new mdl = GetVehicleModel(v);
		if(mdl < 400 || mdl > 611) continue;

		new valeur = vehicle[v][cValue];
		if(valeur <= 0) valeur = ASSU_PRIME_MIN * 10;
		new prime = Assu_Prime(valeur);
		new nom[32];
		format(nom, sizeof(nom), "%s", vehName[mdl - 400]);

		if(Assu_EstAssure(vehicle[v][SQLID]))
		{
			new k = Assu_Find(vehicle[v][SQLID]);
			new jours = (assuExpire[k] - gettime()) / 86400;
			new heures = ((assuExpire[k] - gettime()) % 86400) / 3600;
			format(str, sizeof(str), "{00FF00}%s{FFFFFF} - assure (%dj %dh) - prolonger : {FFFF00}%d${FFFFFF}\n", nom, jours, heures, prime);
		}
		else format(str, sizeof(str), "{FFFFFF}%s - non assure - prime : {FFFF00}%d${FFFFFF}\n", nom, prime);

		strcat(body, str);
		assuMap[playerid][n] = v;
		n++;
	}

	if(n == 0)
	{
		Assu_Msg(playerid, "{FF4444}Tu n'as aucun vehicule assurable.");
		return 0;
	}
	return ShowPlayerDialog(playerid, DIALOG_ASSU_LISTE, DIALOG_STYLE_LIST, "Assurer un vehicule", body, "Assurer", "Retour");
}

stock Assu_ShowMesContrats(playerid)
{
	new body[1200], str[190], n = 0;
	body[0] = 0;

	for(new k = 0; k < ASSU_MAX; k++)
	{
		if(assuPris[k] != 1) continue;
		if(assuOwnerSqlid[k] != PlayerInfo[playerid][pSQLID]) continue;
		if(assuExpire[k] <= gettime()) continue;

		new vidx = Assu_VidxParSqlid(assuCarSqlid[k]);
		new nom[32] = "Vehicule";
		if(vidx != -1)
		{
			new mdl2 = GetVehicleModel(vidx);
			if(mdl2 >= 400 && mdl2 <= 611) format(nom, sizeof(nom), "%s", vehName[mdl2 - 400]);
		}

		new jours = (assuExpire[k] - gettime()) / 86400;
		new heures = ((assuExpire[k] - gettime()) % 86400) / 3600;
		format(str, sizeof(str), "{FFFFFF}%s{AAAAAA} - encore %d jour(s) %d h\n", nom, jours, heures);
		strcat(body, str);
		n++;
	}

	if(n == 0) format(body, sizeof(body), "{AAAAAA}Tu n'as aucun contrat en cours.");
	return ShowPlayerDialog(playerid, DIALOG_ASSU_MES, DIALOG_STYLE_MSGBOX, "Mes contrats", body, "Fermer", "");
}

stock Assu_ShowInfo(playerid)
{
	new body[700];
	format(body, sizeof(body), "{FFFFFF}Ce que couvre le contrat :\n\n{00FF00}1. Fourriere{FFFFFF}\nLe supplement passe de {FFFF00}%d${FFFFFF} a une franchise de {FFFF00}%d${FFFFFF}.\n\n{00FF00}2. Epave{FFFFFF}\nUn vehicule assure detruit ressort {FFFF00}repare{FFFFFF} a son prochain spawn, au lieu de rester Hors Service en attendant un mecanicien.\n\n{AAAAAA}Aucune indemnite n'est versee en argent : l'assurance reduit des couts, elle ne paye jamais le joueur.\n{AAAAAA}Prime : %d%% de la valeur, minimum %d$, pour %d jours.", PP_FEE_IMPOUND, ASSU_FRANCHISE, ASSU_TAUX, ASSU_PRIME_MIN, ASSU_DUREE_JOURS);
	return ShowPlayerDialog(playerid, DIALOG_ASSU_MES, DIALOG_STYLE_MSGBOX, "Assurance - couverture", body, "Fermer", "");
}

stock Assu_OnDialog(playerid, dialogid, response, listitem)
{
	switch(dialogid)
	{
		case DIALOG_ASSU_MENU:
		{
			if(!response) return 1;
			// item 0 = ligne de tarif, les actions commencent a 1.
			switch(listitem)
			{
				case 0: Assu_ShowMenu(playerid);
				case 1: Assu_ShowVehicules(playerid);
				case 2: Assu_ShowMesContrats(playerid);
				case 3: Assu_ShowInfo(playerid);
			}
			return 1;
		}
		case DIALOG_ASSU_LISTE:
		{
			if(!response) return Assu_ShowMenu(playerid), 1;
			if(listitem < 0 || listitem >= ASSU_MAP_MAX) return 1;
			Assu_Souscrire(playerid, assuMap[playerid][listitem]);
			return 1;
		}
		case DIALOG_ASSU_MES: return 1;
	}
	return 0;
}

stock Assu_Cmd(playerid)
{
	return Assu_ShowMenu(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Assu_Init()
{
	Assu_Load();
	return 1;
}
