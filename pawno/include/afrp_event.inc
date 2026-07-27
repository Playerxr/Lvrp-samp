/*
	afrp_event.inc - Systeme d'evenements administrateur (generique)

	Objectif : donner aux admins un vrai outil pour organiser un event, quel
	qu'il soit, sans que les joueurs partent dans tous les sens pendant la
	preparation.

	Le systeme est volontairement GENERIQUE : la course n'est qu'un des types
	d'event. Tout le reste (inscriptions, teleportation groupee, gel, compte a
	rebours, classement, recompenses, annonces) marche pour n'importe quel type.

	=== DEROULE TYPE ===
	  /ev creer <nom>     l'event est cree a la position de l'admin (= depart)
	  /ev type            choisir le type dans un menu
	  /ev prix <montant>  recompense du gagnant
	  /ev vehicule <mdl>  0 = a pied, sinon un vehicule par participant
	  /ev cp              (course) ajoute un checkpoint a ta position
	  /ev ouvrir          ouvre les inscriptions -> les joueurs font /ev rejoindre
	  /ev tp              teleporte TOUS les inscrits sur la grille de depart
	                      et les GELE : plus personne ne bouge
	  /ev depart          compte a rebours 3-2-1 puis degel : l'event demarre
	  /ev fin             termine l'event et paie le classement
	  /ev annuler         annule tout sans rien payer

	=== LE PROBLEME DES JOUEURS QUI BOUGENT ===
	  /ev gel   et  /ev degel  gelent / degelent tous les participants a tout
	  moment. /ev tp gele automatiquement. Le degel n'a lieu qu'au GO.

	=== PARCOURS REUTILISABLES ===
	  Un parcours de checkpoints se sauvegarde et se recharge par son nom :
	  /ev sauver <nom>  puis  /ev charger <nom>  (ou /ev parcours pour la liste)
	  Les parcours survivent au redemarrage du serveur (events_parcours.cfg),
	  donc une course preparee une fois se relance en 10 secondes.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit              -> Ev_Init()
	- OnPlayerDisconnect          -> Ev_OnDisconnect(playerid)
	- OnPlayerDeath               -> Ev_OnDeath(playerid)
	- OnPlayerEnterRaceCheckpoint -> Ev_OnEnterRaceCP(playerid)
	- OnDialogResponse            -> Ev_OnDialog(playerid, dialogid, response, listitem, inputtext)
	- OnPlayerCommandText         -> Ev_Command(playerid, cmdtext)  sur /ev et /evenement
*/

#if defined AFRP_EVENT_INCLUDED
	#endinput
#endif
#define AFRP_EVENT_INCLUDED

#define EV_ADMIN_LEVEL      2           // niveau admin requis pour organiser
#define EV_MAX_CP           30          // checkpoints max par parcours
#define EV_MAX_ROUTE        20          // parcours sauvegardes
#define EV_FILE_ROUTES      "events_parcours.cfg"
#define EV_CP_SIZE          8.0         // rayon d'un checkpoint

// --- etats de l'event ---
#define EV_OFF              0           // aucun event
#define EV_PREP             1           // en preparation (admin seul)
#define EV_INSCRIT          2           // inscriptions ouvertes
#define EV_ATTENTE          3           // sur la grille, geles
#define EV_ENCOURS          4           // lance

// --- types d'event ---
#define EV_T_COURSE         0
#define EV_T_DM             1
#define EV_T_DERBY          2
#define EV_T_CHASSE         3
#define EV_T_RASSEM         4
#define EV_T_LIBRE          5
#define EV_T_COUNT          6

// --- dialogues (plage 9610-9617, libre : verifiee avec /verif) ---
#define DIALOG_EV_MENU      9610
#define DIALOG_EV_TYPE      9611
#define DIALOG_EV_PART      9612
#define DIALOG_EV_CP        9613
#define DIALOG_EV_PRIX      9614
#define DIALOG_EV_NOM       9615
#define DIALOG_EV_ROUTES    9616
#define DIALOG_EV_SAVENAME  9617
#define DIALOG_EV_TOURS     9618
#define DIALOG_EV_TPALL     9619

new evTypeName[EV_T_COUNT][24] = {
	"Course a checkpoints",
	"Deathmatch",
	"Derby de demolition",
	"Chasse a l'homme",
	"Rassemblement",
	"Libre / RP"
};

// Un type "elimination" : mourir sort le joueur de l'event.
new evTypeElim[EV_T_COUNT] = { 0, 1, 1, 1, 0, 0 };

// ============================================================================
// ETAT GLOBAL
// ============================================================================
new evState = EV_OFF;
new evType  = EV_T_COURSE;
new evName[48];
new evPrize;
new evHost = INVALID_PLAYER_ID;
new evVehModel;                 // 0 = a pied
new evLapTotal = 1;
new evCountdown;
new evCountTimer = -1;
new evRankNext;                 // prochaine place a attribuer (1 = 1er)

new Float:evSpawnX, Float:evSpawnY, Float:evSpawnZ, Float:evSpawnA;
new evSpawnInt, evSpawnVw;

new evCpCount;
new Float:evCpX[EV_MAX_CP], Float:evCpY[EV_MAX_CP], Float:evCpZ[EV_MAX_CP];

// --- par joueur ---
new evIn[MAX_PLAYERS];          // 1 = inscrit
new evAlive[MAX_PLAYERS];       // 0 = elimine
new evNextCp[MAX_PLAYERS];      // prochain checkpoint a franchir
new evLap[MAX_PLAYERS];         // tour en cours
new evRank[MAX_PLAYERS];        // place finale (0 = pas fini)
new evVeh[MAX_PLAYERS];         // vehicule cree pour lui (0 = aucun)

// --- parcours sauvegardes ---
new evRouteName[EV_MAX_ROUTE][24];
new evRouteCount[EV_MAX_ROUTE];
new Float:evRouteX[EV_MAX_ROUTE][EV_MAX_CP];
new Float:evRouteY[EV_MAX_ROUTE][EV_MAX_CP];
new Float:evRouteZ[EV_MAX_ROUTE][EV_MAX_CP];
new evRouteUsed[EV_MAX_ROUTE];
new evRouteMap[MAX_PLAYERS][EV_MAX_ROUTE];  // listitem -> index parcours

// ============================================================================
// OUTILS
// ============================================================================
stock Ev_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= EV_ADMIN_LEVEL);
}

stock Ev_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// Refus d'acces : on affiche le niveau requis ET le niveau du joueur. Sans ca,
// un "reserve aux admins" ne dit pas si le probleme vient du grade ou d'ailleurs.
stock Ev_MsgAdmin(playerid)
{
	new str[144];
	format(str, sizeof(str), "{FF4444}Reserve aux administrateurs (niveau %d requis, tu es niveau %d).",
		EV_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
	return Ev_Msg(playerid, str);
}

stock Ev_Announce(const text[])
{
	SendClientMessageToAll(0xFFCC00FF, text);
	return 1;
}

// Petit tokenizer autonome : evite de dependre de l'index de OnPlayerCommandText.
stock Ev_Tok(const src[], &evPos, dest[], destSize)
{
	new p = 0;
	while(src[evPos] == ' ') evPos++;
	while(src[evPos] && src[evPos] != ' ' && p < destSize - 1)
		dest[p++] = src[evPos++];
	dest[p] = 0;
	return p;
}

// Recupere tout le reste de la ligne (pour un nom avec des espaces).
stock Ev_Rest(const src[], evPos, dest[], destSize)
{
	new p = 0;
	while(src[evPos] == ' ') evPos++;
	while(src[evPos] && p < destSize - 1) dest[p++] = src[evPos++];
	dest[p] = 0;
	return p;
}

stock Ev_CountPart()
{
	new c = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && evIn[i] == 1) c++;
	return c;
}

stock Ev_CountAlive()
{
	new c = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && evIn[i] == 1 && evAlive[i] == 1) c++;
	return c;
}

stock Ev_StateName(st, dest[], destSize)
{
	switch(st)
	{
		case EV_PREP:    format(dest, destSize, "En preparation");
		case EV_INSCRIT: format(dest, destSize, "Inscriptions ouvertes");
		case EV_ATTENTE: format(dest, destSize, "Sur la grille (geles)");
		case EV_ENCOURS: format(dest, destSize, "En cours");
		default:         format(dest, destSize, "Aucun event");
	}
	return 1;
}

// ============================================================================
// GEL / DEGEL - le coeur du probleme "les joueurs bougent dans tous les sens"
// ============================================================================
stock Ev_FreezeAll(freeze)
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || evIn[i] != 1) continue;
		TogglePlayerControllable(i, (freeze == 1) ? false : true);
		n++;
	}
	return n;
}

// Vrai si le joueur doit RESTER gele parce qu'il attend le depart d'un event.
// Utilise par chargement() dans LVRP.pwn : sans ca, le timer de fin de
// chargement d'un spawn precedent degelait un participant sur la grille.
stock Ev_IsFrozen(playerid)
{
	if(evState != EV_ATTENTE) return 0;
	return (evIn[playerid] == 1);
}

// ============================================================================
// RASSEMBLEMENT : tout le serveur au spawn de Los Santos
// ============================================================================
stock Ev_TpAllRun(playerid)
{
	new n = 0, jailed = 0, slot = 0, str[144];
	new Float:baseA = spawn[0][pos][3];
	new Float:cosA  = floatcos(-baseA, degrees);
	new Float:sinA  = floatsin(-baseA, degrees);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		// Un rassemblement ne doit pas faire sortir un detenu de prison.
		if(PlayerInfo[i][pJailed] != 0) { jailed++; continue; }

		// Grille de 6 par rangee pour ne pas empiler tout le monde au meme point.
		new col = slot % 6;
		new row = slot / 6;
		new Float:offX = (float(col) - 2.5) * 3.0;
		new Float:offY = -(float(row) * 3.0);
		new Float:px = spawn[0][pos][0] + (offX * cosA) - (offY * sinA);
		new Float:py = spawn[0][pos][1] + (offX * sinA) + (offY * cosA);

		SafeTeleportPlayer(i, px, py, spawn[0][pos][2], 0, 0, baseA);
		SendClientMessage(i, 0xFFCC00FF, "[RASSEMBLEMENT] Tu as ete teleporte au spawn de Los Santos par un administrateur.");
		slot++;
		n++;
	}

	format(str, sizeof(str), "{00FF00}%d joueur(s) teleporte(s) au spawn de Los Santos.", n);
	Ev_Msg(playerid, str);
	if(jailed > 0)
	{
		format(str, sizeof(str), "{AAAAAA}%d joueur(s) en prison n'ont pas ete deplaces.", jailed);
		Ev_Msg(playerid, str);
	}
	return 1;
}

// SORTIE DE SECOURS. Ev_FreezeAll(0) ne touche QUE les inscrits : si l'event a
// ete termine ou annule, evIn est vide et un joueur reste gele n'a plus aucun
// moyen d'etre libere. Cette commande degele tout le monde, event ou pas.
stock Ev_CmdUnfreezeAll(playerid)
{
	if(!Ev_IsAdmin(playerid)) return Ev_MsgAdmin(playerid);

	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		TogglePlayerControllable(i, true);
		n++;
	}
	// On sort aussi de l'etat "sur la grille", sinon chargement() re-bloquerait.
	if(evState == EV_ATTENTE) evState = EV_ENCOURS;

	new str[128];
	format(str, sizeof(str), "{00FF00}%d joueur(s) degele(s). Plus personne n'est bloque.", n);
	return Ev_Msg(playerid, str);
}

stock Ev_CmdTpAll(playerid)
{
	if(!Ev_IsAdmin(playerid)) return Ev_MsgAdmin(playerid);

	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && !IsPlayerNPC(i) && PlayerInfo[i][pJailed] == 0) n++;

	new body[256];
	format(body, sizeof(body), "{FFFFFF}Teleporter {FFFF00}%d joueur(s){FFFFFF} au spawn de Los Santos ?\n\n{AAAAAA}Tout le monde sera deplace, en jeu comme en interieur.\nLes joueurs en prison ne sont pas concernes.", n);
	return ShowPlayerDialog(playerid, DIALOG_EV_TPALL, DIALOG_STYLE_MSGBOX, "Rassemblement general", body, "Teleporter", "Annuler");
}

// ============================================================================
// CHECKPOINTS
// ============================================================================
stock Ev_GiveCp(playerid)
{
	if(evType != EV_T_COURSE || evCpCount == 0) return 0;

	new cp = evNextCp[playerid];
	if(cp < 0 || cp >= evCpCount) return 0;

	// Dernier checkpoint du dernier tour = arrivee (type 1), sinon normal (0).
	new cpType = 0;
	new Float:nx = evCpX[cp], Float:ny = evCpY[cp], Float:nz = evCpZ[cp];

	if(cp == evCpCount - 1)
	{
		if(evLap[playerid] >= evLapTotal) cpType = 1;   // drapeau d'arrivee
		else { nx = evCpX[0]; ny = evCpY[0]; nz = evCpZ[0]; }
	}
	else
	{
		nx = evCpX[cp + 1]; ny = evCpY[cp + 1]; nz = evCpZ[cp + 1];
	}

	SetPlayerRaceCheckpoint(playerid, cpType, evCpX[cp], evCpY[cp], evCpZ[cp], nx, ny, nz, EV_CP_SIZE);
	return 1;
}

stock Ev_ClearCpAll()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		DisablePlayerRaceCheckpoint(i);
	}
	return 1;
}

// ============================================================================
// PERSISTANCE DES PARCOURS
// ============================================================================
stock Ev_RouteField(const line[], &i, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock Ev_SaveRoutes()
{
	if(fexist(EV_FILE_ROUTES)) fremove(EV_FILE_ROUTES);
	new File:f = fopen(EV_FILE_ROUTES, io_write);
	if(!f) return 0;
	new line[160];
	for(new r = 0; r < EV_MAX_ROUTE; r++)
	{
		if(evRouteUsed[r] != 1) continue;
		// une ligne d'entete par parcours, puis une ligne par checkpoint
		format(line, sizeof(line), "R|%s|%d\n", evRouteName[r], evRouteCount[r]);
		fwrite(f, line);
		for(new c = 0; c < evRouteCount[r]; c++)
		{
			format(line, sizeof(line), "C|%.4f|%.4f|%.4f\n", evRouteX[r][c], evRouteY[r][c], evRouteZ[r][c]);
			fwrite(f, line);
		}
	}
	fclose(f);
	return 1;
}

stock Ev_LoadRoutes()
{
	for(new r = 0; r < EV_MAX_ROUTE; r++) { evRouteUsed[r] = 0; evRouteCount[r] = 0; }
	if(!fexist(EV_FILE_ROUTES)) return 0;
	new File:f = fopen(EV_FILE_ROUTES, io_read);
	if(!f) return 0;

	new line[160], buf[48], cur = -1, loaded = 0;
	while(fread(f, line))
	{
		if(strlen(line) < 3) continue;
		new i = 0;
		Ev_RouteField(line, i, buf, sizeof(buf));
		if(strcmp(buf, "R", true) == 0)
		{
			cur = -1;
			for(new r = 0; r < EV_MAX_ROUTE; r++)
				if(evRouteUsed[r] == 0) { cur = r; break; }
			if(cur == -1) continue;
			Ev_RouteField(line, i, buf, sizeof(buf));
			format(evRouteName[cur], 24, "%s", buf);
			evRouteUsed[cur]  = 1;
			evRouteCount[cur] = 0;
			loaded++;
		}
		else if(strcmp(buf, "C", true) == 0 && cur != -1)
		{
			new n = evRouteCount[cur];
			if(n >= EV_MAX_CP) continue;
			Ev_RouteField(line, i, buf, sizeof(buf)); evRouteX[cur][n] = floatstr(buf);
			Ev_RouteField(line, i, buf, sizeof(buf)); evRouteY[cur][n] = floatstr(buf);
			Ev_RouteField(line, i, buf, sizeof(buf)); evRouteZ[cur][n] = floatstr(buf);
			evRouteCount[cur] = n + 1;
		}
	}
	fclose(f);
	printf("[Event] %d parcours charge(s)", loaded);
	return loaded;
}

stock Ev_FindRoute(const rname[])
{
	for(new r = 0; r < EV_MAX_ROUTE; r++)
		if(evRouteUsed[r] == 1 && strcmp(evRouteName[r], rname, true) == 0) return r;
	return -1;
}

// ============================================================================
// CYCLE DE VIE
// ============================================================================
stock Ev_DestroyVehicles()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(evVeh[i] != 0)
		{
			DestroyVehicle(evVeh[i]);
			evVeh[i] = 0;
		}
	}
	return 1;
}

stock Ev_ResetPlayer(playerid)
{
	evIn[playerid]     = 0;
	evAlive[playerid]  = 0;
	evNextCp[playerid] = 0;
	evLap[playerid]    = 1;
	evRank[playerid]   = 0;
	if(evVeh[playerid] != 0) { DestroyVehicle(evVeh[playerid]); evVeh[playerid] = 0; }
	if(IsPlayerConnected(playerid))
	{
		DisablePlayerRaceCheckpoint(playerid);
		TogglePlayerControllable(playerid, true);
	}
	return 1;
}

stock Ev_Reset()
{
	if(evCountTimer != -1) { KillTimer(evCountTimer); evCountTimer = -1; }
	// On ne "degele" et on ne retire le checkpoint QUE des participants :
	// sinon on annulerait le gel pose par un autre systeme (outils de moderation,
	// intro, etc.) sur des joueurs qui n'ont rien a voir avec l'event.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(evIn[i] == 1) Ev_ResetPlayer(i);
		else if(evVeh[i] != 0) { DestroyVehicle(evVeh[i]); evVeh[i] = 0; }
	}
	evState    = EV_OFF;
	evPrize    = 0;
	evVehModel = 0;
	evCpCount  = 0;
	evLapTotal = 1;
	evRankNext = 1;
	evHost     = INVALID_PLAYER_ID;
	evName[0]  = 0;
	return 1;
}

stock Ev_Create(playerid, const wanted[])
{
	if(evState != EV_OFF) return Ev_Msg(playerid, "{FF4444}Un event existe deja. Termine-le (/ev fin) ou annule-le (/ev annuler).");

	Ev_Reset();
	format(evName, sizeof(evName), "%s", wanted);
	if(!strlen(evName)) format(evName, sizeof(evName), "Event");

	GetPlayerPos(playerid, evSpawnX, evSpawnY, evSpawnZ);
	GetPlayerFacingAngle(playerid, evSpawnA);
	evSpawnInt = GetPlayerInterior(playerid);
	evSpawnVw  = GetPlayerVirtualWorld(playerid);

	evState = EV_PREP;
	evHost  = playerid;
	evType  = EV_T_COURSE;

	new str[144];
	format(str, sizeof(str), "{00FF00}Event \"%s\" cree ici. Position de depart enregistree.", evName);
	Ev_Msg(playerid, str);
	Ev_Msg(playerid, "{FFFF00}Suite : /ev type, /ev prix, /ev vehicule, /ev cp, puis /ev ouvrir.");
	return 1;
}

stock Ev_Open(playerid)
{
	if(evState != EV_PREP) return Ev_Msg(playerid, "{FF4444}Aucun event en preparation.");
	if(evType == EV_T_COURSE && evCpCount == 0)
		return Ev_Msg(playerid, "{FF4444}Une course sans checkpoint ? Ajoute-en avec /ev cp (ou /ev charger <parcours>).");

	evState = EV_INSCRIT;
	new str[160];
	format(str, sizeof(str), "[EVENT] %s - %s | Inscriptions OUVERTES : tape /ev rejoindre", evName, evTypeName[evType]);
	Ev_Announce(str);
	if(evPrize > 0)
	{
		format(str, sizeof(str), "[EVENT] Recompense du gagnant : %d$", evPrize);
		Ev_Announce(str);
	}
	return 1;
}

stock Ev_Close(playerid)
{
	if(evState != EV_INSCRIT) return Ev_Msg(playerid, "{FF4444}Les inscriptions ne sont pas ouvertes.");
	evState = EV_PREP;
	Ev_Announce("[EVENT] Inscriptions fermees.");
	return 1;
}

stock Ev_Join(playerid)
{
	if(evState != EV_INSCRIT) return Ev_Msg(playerid, "{FF4444}Aucune inscription ouverte pour le moment.");
	if(evIn[playerid] == 1)   return Ev_Msg(playerid, "{FF4444}Tu es deja inscrit. /ev quitter pour te desinscrire.");

	evIn[playerid]     = 1;
	evAlive[playerid]  = 1;
	evNextCp[playerid] = 0;
	evLap[playerid]    = 1;
	evRank[playerid]   = 0;

	new str[144];
	format(str, sizeof(str), "{00FF00}Inscrit a l'event \"%s\". Attends la teleportation de l'organisateur.", evName);
	Ev_Msg(playerid, str);
	format(str, sizeof(str), "[EVENT] %s a rejoint (%d participant(s)).", PlayerInfo[playerid][pName], Ev_CountPart());
	if(evHost != INVALID_PLAYER_ID && IsPlayerConnected(evHost)) SendClientMessage(evHost, 0xFFCC00FF, str);
	return 1;
}

stock Ev_Leave(playerid)
{
	if(evIn[playerid] != 1) return Ev_Msg(playerid, "{FF4444}Tu n'es inscrit a aucun event.");
	Ev_ResetPlayer(playerid);
	Ev_Msg(playerid, "{FFFF00}Tu as quitte l'event.");
	return 1;
}

// Grille de depart : 5 par rangee, ecartes, orientes comme l'admin.
stock Ev_TeleportAll(playerid)
{
	if(evState != EV_PREP && evState != EV_INSCRIT)
		return Ev_Msg(playerid, "{FF4444}Rien a teleporter : cree un event et ouvre les inscriptions d'abord.");

	new n = Ev_CountPart();
	if(n == 0) return Ev_Msg(playerid, "{FF4444}Aucun participant inscrit.");

	new slot = 0;
	new Float:cosA = floatcos(-evSpawnA, degrees);
	new Float:sinA = floatsin(-evSpawnA, degrees);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || evIn[i] != 1) continue;

		new col = slot % 5;
		new row = slot / 5;
		new Float:offX = (float(col) - 2.0) * 4.0;   // ecart lateral
		new Float:offY = -(float(row) * 6.0);        // rangees derriere

		new Float:px = evSpawnX + (offX * cosA) - (offY * sinA);
		new Float:py = evSpawnY + (offX * sinA) + (offY * cosA);
		new Float:pz = evSpawnZ;

		if(evVeh[i] != 0) { DestroyVehicle(evVeh[i]); evVeh[i] = 0; }

		if(evVehModel > 0)
		{
			evVeh[i] = CreateVehicle(evVehModel, px, py, pz + 0.5, evSpawnA, -1, -1, 600);
			LinkVehicleToInterior(evVeh[i], evSpawnInt);
			SetVehicleVirtualWorld(evVeh[i], evSpawnVw);
			SetPlayerInterior(i, evSpawnInt);
			SetPlayerVirtualWorld(i, evSpawnVw);
			PutPlayerInVehicle(i, evVeh[i], 0);
		}
		else
		{
			SetPlayerInterior(i, evSpawnInt);
			SetPlayerVirtualWorld(i, evSpawnVw);
			SetPlayerPos(i, px, py, pz);
			SetPlayerFacingAngle(i, evSpawnA);
		}

		evNextCp[i] = 0;
		evLap[i]    = 1;
		evAlive[i]  = 1;
		evRank[i]   = 0;
		DisablePlayerRaceCheckpoint(i);
		TogglePlayerControllable(i, false);     // <-- personne ne bouge
		SendClientMessage(i, 0xFFCC00FF, "[EVENT] Tu es sur la grille de depart. Tu es gele jusqu'au GO.");
		slot++;
	}

	evState    = EV_ATTENTE;
	evRankNext = 1;

	new str[144];
	format(str, sizeof(str), "{00FF00}%d participant(s) teleporte(s) et geles. /ev depart quand tu veux lancer.", n);
	Ev_Msg(playerid, str);
	return 1;
}

forward Ev_CountdownTick();
public Ev_CountdownTick()
{
	new str[32];
	if(evCountdown > 0)
	{
		format(str, sizeof(str), "~r~%d", evCountdown);
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || evIn[i] != 1) continue;
			GameTextForPlayer(i, str, 1100, 3);
			PlayerPlaySound(i, 1056, 0.0, 0.0, 0.0);
		}
		evCountdown--;
		return 1;
	}

	// GO
	if(evCountTimer != -1) { KillTimer(evCountTimer); evCountTimer = -1; }
	evState = EV_ENCOURS;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || evIn[i] != 1) continue;
		GameTextForPlayer(i, "~g~GO !", 1500, 3);
		PlayerPlaySound(i, 1057, 0.0, 0.0, 0.0);
		TogglePlayerControllable(i, true);
		Ev_GiveCp(i);
	}
	Ev_Announce("[EVENT] C'est parti !");
	return 1;
}

stock Ev_Start(playerid)
{
	if(evState != EV_ATTENTE)
		return Ev_Msg(playerid, "{FF4444}Teleporte d'abord les participants sur la grille (/ev tp).");
	if(evCountTimer != -1) return Ev_Msg(playerid, "{FF4444}Un compte a rebours est deja lance.");

	evCountdown  = 3;
	evCountTimer = SetTimer("Ev_CountdownTick", 1000, true);
	Ev_Msg(playerid, "{00FF00}Compte a rebours lance.");
	return 1;
}

// Un joueur franchit la ligne d'arrivee (ou est le dernier survivant).
stock Ev_PlayerFinish(playerid)
{
	if(evRank[playerid] != 0) return 0;

	evRank[playerid] = evRankNext++;
	DisablePlayerRaceCheckpoint(playerid);

	new str[160], gain = 0;
	switch(evRank[playerid])
	{
		case 1: gain = evPrize;
		case 2: gain = evPrize / 2;
		case 3: gain = evPrize / 4;
	}
	if(gain > 0)
	{
		PlayerInfo[playerid][pCash] += gain;
		format(str, sizeof(str), "{00FF00}Tu termines %d e ! Gain : %d$", evRank[playerid], gain);
		Ev_Msg(playerid, str);
	}

	format(str, sizeof(str), "[EVENT] %d e : %s", evRank[playerid], PlayerInfo[playerid][pName]);
	Ev_Announce(str);
	return 1;
}

stock Ev_Finish(playerid)
{
	if(evState == EV_OFF) return Ev_Msg(playerid, "{FF4444}Aucun event en cours.");
	new str[144];
	format(str, sizeof(str), "[EVENT] \"%s\" termine. Merci a tous les participants !", evName);
	Ev_Announce(str);
	Ev_DestroyVehicles();
	Ev_Reset();
	if(playerid != INVALID_PLAYER_ID) Ev_Msg(playerid, "{00FF00}Event termine et nettoye.");
	return 1;
}

stock Ev_Cancel(playerid)
{
	if(evState == EV_OFF) return Ev_Msg(playerid, "{FF4444}Aucun event en cours.");
	Ev_Announce("[EVENT] L'evenement a ete annule par l'organisation.");
	Ev_DestroyVehicles();
	Ev_Reset();
	Ev_Msg(playerid, "{FFFF00}Event annule, aucune recompense versee.");
	return 1;
}

// ============================================================================
// HOOKS
// ============================================================================
stock Ev_OnEnterRaceCP(playerid)
{
	if(evState != EV_ENCOURS) return 0;
	if(evIn[playerid] != 1 || evAlive[playerid] != 1) return 0;
	if(evType != EV_T_COURSE || evCpCount == 0) return 0;

	new last = (evNextCp[playerid] == evCpCount - 1);

	if(last && evLap[playerid] >= evLapTotal)
	{
		Ev_PlayerFinish(playerid);
		// Tout le monde a fini ?
		new remaining = 0;
		for(new i = 0; i < MAX_PLAYERS; i++)
			if(IsPlayerConnected(i) && evIn[i] == 1 && evAlive[i] == 1 && evRank[i] == 0) remaining++;
		if(remaining == 0) Ev_Finish(INVALID_PLAYER_ID);
		return 1;
	}

	if(last)
	{
		evLap[playerid]++;
		evNextCp[playerid] = 0;
		new str[64];
		format(str, sizeof(str), "~w~Tour ~y~%d~w~/%d", evLap[playerid], evLapTotal);
		GameTextForPlayer(playerid, str, 2000, 4);
	}
	else evNextCp[playerid]++;

	Ev_GiveCp(playerid);
	return 1;
}

stock Ev_OnDeath(playerid)
{
	if(evState != EV_ENCOURS) return 0;
	if(evIn[playerid] != 1 || evAlive[playerid] != 1) return 0;
	if(evTypeElim[evType] != 1) return 0;

	evAlive[playerid] = 0;
	DisablePlayerRaceCheckpoint(playerid);
	Ev_Msg(playerid, "{FF4444}Tu es elimine de l'event.");

	new alive = Ev_CountAlive();
	if(alive == 1)
	{
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || evIn[i] != 1 || evAlive[i] != 1) continue;
			Ev_PlayerFinish(i);
			break;
		}
		Ev_Finish(INVALID_PLAYER_ID);
	}
	else
	{
		new str[96];
		format(str, sizeof(str), "[EVENT] %d survivant(s) restant(s).", alive);
		Ev_Announce(str);
	}
	return 1;
}

stock Ev_OnDisconnect(playerid)
{
	if(evIn[playerid] == 1) Ev_ResetPlayer(playerid);
	if(evHost == playerid) evHost = INVALID_PLAYER_ID;
	return 1;
}

// ============================================================================
// MENUS
// ============================================================================
stock Ev_ShowMenu(playerid)
{
	// ATTENTION : en DIALOG_STYLE_LIST chaque ligne est un item selectionnable.
	// Le resume tient donc sur UNE seule ligne = l'item 0, et les actions
	// commencent a l'item 1. Ne jamais ajouter de ligne d'info ici sans
	// decaler les index du handler DIALOG_EV_MENU.
	new body[1400], titre[80], stName[32];
	Ev_StateName(evState, stName, sizeof(stName));

	if(evState == EV_OFF)
	{
		format(titre, sizeof(titre), "Evenement - aucun en cours");
		format(body, sizeof(body), "{AAAAAA}Aucun evenement en cours\n");
	}
	else
	{
		format(titre, sizeof(titre), "Evenement : %s (%s)", evName, stName);
		if(evType == EV_T_COURSE)
			format(body, sizeof(body), "{AAAAAA}%s | %d joueur(s) | %d$ | %d cp | %d tour(s)\n",
				evTypeName[evType], Ev_CountPart(), evPrize, evCpCount, evLapTotal);
		else
			format(body, sizeof(body), "{AAAAAA}%s | %d joueur(s) | %d$\n",
				evTypeName[evType], Ev_CountPart(), evPrize);
	}

	if(!Ev_IsAdmin(playerid))
	{
		if(evState == EV_INSCRIT && evIn[playerid] != 1) strcat(body, "{00FF00}Rejoindre l'event\n");
		else if(evIn[playerid] == 1) strcat(body, "{FF8888}Quitter l'event\n");
		else strcat(body, "{888888}(rien a faire pour l'instant)\n");
		strcat(body, "{AAAAAA}Voir les participants");
		return ShowPlayerDialog(playerid, DIALOG_EV_MENU, DIALOG_STYLE_LIST, titre, body, "Choisir", "Fermer");
	}

	// menu admin
	strcat(body, "{00FF00}Creer un event ici\n");
	strcat(body, "{FFFFFF}Changer le type\n");
	strcat(body, "{FFFFFF}Definir la recompense\n");
	strcat(body, "{FFFFFF}Gerer les checkpoints\n");
	strcat(body, "{FFFF00}Ouvrir les inscriptions\n");
	strcat(body, "{FFFF00}Fermer les inscriptions\n");
	strcat(body, "{00CCFF}Teleporter tout le monde (et geler)\n");
	strcat(body, "{00FF00}Lancer le depart (3-2-1)\n");
	strcat(body, "{FFFFFF}Geler tout le monde\n");
	strcat(body, "{FFFFFF}Degeler tout le monde\n");
	strcat(body, "{AAAAAA}Voir les participants\n");
	strcat(body, "{AAAAAA}Parcours sauvegardes\n");
	strcat(body, "{FF8888}Terminer l'event\n");
	strcat(body, "{FF4444}Annuler l'event");
	return ShowPlayerDialog(playerid, DIALOG_EV_MENU, DIALOG_STYLE_LIST, titre, body, "Choisir", "Fermer");
}

stock Ev_ShowTypes(playerid)
{
	new body[400], str[64];
	body[0] = 0;
	for(new t = 0; t < EV_T_COUNT; t++)
	{
		format(str, sizeof(str), "%s\n", evTypeName[t]);
		strcat(body, str);
	}
	return ShowPlayerDialog(playerid, DIALOG_EV_TYPE, DIALOG_STYLE_LIST, "Type d'evenement", body, "Choisir", "Retour");
}

stock Ev_ShowPart(playerid)
{
	new body[1200], str[128], n = 0;
	body[0] = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || evIn[i] != 1) continue;
		if(evRank[i] != 0)      format(str, sizeof(str), "{AAAAAA}%s - termine (%d e)\n", PlayerInfo[i][pName], evRank[i]);
		else if(evAlive[i] != 1) format(str, sizeof(str), "{FF6666}%s - elimine\n", PlayerInfo[i][pName]);
		else if(evType == EV_T_COURSE && evCpCount > 0)
			format(str, sizeof(str), "{FFFFFF}%s - tour %d/%d, cp %d/%d\n", PlayerInfo[i][pName], evLap[i], evLapTotal, evNextCp[i] + 1, evCpCount);
		else format(str, sizeof(str), "{FFFFFF}%s - en course\n", PlayerInfo[i][pName]);
		strcat(body, str);
		n++;
	}
	if(n == 0) format(body, sizeof(body), "{AAAAAA}Aucun participant inscrit.");
	return ShowPlayerDialog(playerid, DIALOG_EV_PART, DIALOG_STYLE_MSGBOX, "Participants", body, "Fermer", "");
}

stock Ev_ShowCpMenu(playerid)
{
	// Meme regle qu'au menu principal : l'item 0 est la ligne de resume,
	// les actions commencent a l'item 1.
	new body[600], str[128], titre[80];
	format(titre, sizeof(titre), "Parcours : %d checkpoint(s), %d tour(s)", evCpCount, evLapTotal);
	format(body, sizeof(body), "{AAAAAA}%d / %d checkpoints places\n", evCpCount, EV_MAX_CP);
	strcat(body, "{00FF00}Ajouter un checkpoint a ma position\n");
	strcat(body, "{FF8888}Supprimer le dernier checkpoint\n");
	strcat(body, "{FF4444}Effacer tout le parcours\n");
	strcat(body, "{FFFFFF}Sauvegarder ce parcours\n");
	strcat(body, "{FFFFFF}Charger un parcours\n");
	format(str, sizeof(str), "{FFFFFF}Nombre de tours (actuel : %d)", evLapTotal);
	strcat(body, str);
	return ShowPlayerDialog(playerid, DIALOG_EV_CP, DIALOG_STYLE_LIST, titre, body, "Choisir", "Retour");
}

stock Ev_ShowRoutes(playerid)
{
	new body[900], str[96], n = 0;
	body[0] = 0;
	for(new r = 0; r < EV_MAX_ROUTE; r++)
	{
		if(evRouteUsed[r] != 1) continue;
		format(str, sizeof(str), "%s (%d cp)\n", evRouteName[r], evRouteCount[r]);
		strcat(body, str);
		evRouteMap[playerid][n] = r;
		n++;
	}
	if(n == 0)
	{
		Ev_Msg(playerid, "{FF4444}Aucun parcours sauvegarde. Place des checkpoints puis /ev sauver <nom>.");
		return 0;
	}
	return ShowPlayerDialog(playerid, DIALOG_EV_ROUTES, DIALOG_STYLE_LIST, "Charger un parcours", body, "Charger", "Retour");
}

// ============================================================================
// ACTIONS CHECKPOINTS
// ============================================================================
stock Ev_AddCp(playerid)
{
	if(evCpCount >= EV_MAX_CP) return Ev_Msg(playerid, "{FF4444}Nombre maximum de checkpoints atteint.");

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	evCpX[evCpCount] = px;
	evCpY[evCpCount] = py;
	evCpZ[evCpCount] = pz;
	evCpCount++;

	new str[128];
	format(str, sizeof(str), "{00FF00}Checkpoint %d place ici. (%d au total)", evCpCount, evCpCount);
	Ev_Msg(playerid, str);
	return 1;
}

stock Ev_LoadRouteInto(playerid, r)
{
	if(r < 0 || r >= EV_MAX_ROUTE || evRouteUsed[r] != 1) return 0;
	evCpCount = evRouteCount[r];
	for(new c = 0; c < evCpCount; c++)
	{
		evCpX[c] = evRouteX[r][c];
		evCpY[c] = evRouteY[r][c];
		evCpZ[c] = evRouteZ[r][c];
	}
	new str[128];
	format(str, sizeof(str), "{00FF00}Parcours \"%s\" charge : %d checkpoints.", evRouteName[r], evCpCount);
	Ev_Msg(playerid, str);
	return 1;
}

stock Ev_SaveRouteAs(playerid, const rname[])
{
	if(evCpCount == 0) return Ev_Msg(playerid, "{FF4444}Aucun checkpoint a sauvegarder.");
	if(!strlen(rname)) return Ev_Msg(playerid, "{FF4444}Donne un nom au parcours.");

	new r = Ev_FindRoute(rname);
	if(r == -1)
	{
		for(new k = 0; k < EV_MAX_ROUTE; k++)
			if(evRouteUsed[k] == 0) { r = k; break; }
	}
	if(r == -1) return Ev_Msg(playerid, "{FF4444}Plus de place : supprime un parcours avant.");

	format(evRouteName[r], 24, "%s", rname);
	evRouteCount[r] = evCpCount;
	evRouteUsed[r]  = 1;
	for(new c = 0; c < evCpCount; c++)
	{
		evRouteX[r][c] = evCpX[c];
		evRouteY[r][c] = evCpY[c];
		evRouteZ[r][c] = evCpZ[c];
	}
	Ev_SaveRoutes();

	new str[128];
	format(str, sizeof(str), "{00FF00}Parcours \"%s\" sauvegarde (%d checkpoints). Rechargeable avec /ev charger %s", evRouteName[r], evCpCount, evRouteName[r]);
	Ev_Msg(playerid, str);
	return 1;
}

// ============================================================================
// DIALOGUES
// ============================================================================
stock Ev_OnDialog(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
		case DIALOG_EV_MENU:
		{
			if(!response) return 1;
			// item 0 = ligne de resume (voir Ev_ShowMenu), les actions suivent.
			if(!Ev_IsAdmin(playerid))
			{
				switch(listitem)
				{
					case 0: Ev_ShowMenu(playerid);
					case 1:
					{
						if(evState == EV_INSCRIT && evIn[playerid] != 1) Ev_Join(playerid);
						else if(evIn[playerid] == 1) Ev_Leave(playerid);
						else Ev_Msg(playerid, "{FF4444}Aucune inscription ouverte pour le moment.");
					}
					case 2: Ev_ShowPart(playerid);
				}
				return 1;
			}
			switch(listitem)
			{
				case 0:  Ev_ShowMenu(playerid);
				case 1:  ShowPlayerDialog(playerid, DIALOG_EV_NOM, DIALOG_STYLE_INPUT, "Nouvel evenement", "{FFFFFF}Nom de l'evenement :\n\n{AAAAAA}Le depart sera place a ta position actuelle.", "Creer", "Retour");
				case 2:  Ev_ShowTypes(playerid);
				case 3:  ShowPlayerDialog(playerid, DIALOG_EV_PRIX, DIALOG_STYLE_INPUT, "Recompense", "{FFFFFF}Montant pour le gagnant (en $) :\n{AAAAAA}Le 2e recoit la moitie, le 3e le quart.", "Valider", "Retour");
				case 4:  Ev_ShowCpMenu(playerid);
				case 5:  Ev_Open(playerid);
				case 6:  Ev_Close(playerid);
				case 7:  Ev_TeleportAll(playerid);
				case 8:  Ev_Start(playerid);
				case 9:  { new n = Ev_FreezeAll(1); new s2[64]; format(s2, sizeof(s2), "{00FF00}%d joueur(s) gele(s).", n); Ev_Msg(playerid, s2); }
				case 10: { new n = Ev_FreezeAll(0); new s2[64]; format(s2, sizeof(s2), "{00FF00}%d joueur(s) degele(s).", n); Ev_Msg(playerid, s2); }
				case 11: Ev_ShowPart(playerid);
				case 12: Ev_ShowRoutes(playerid);
				case 13: Ev_Finish(playerid);
				case 14: Ev_Cancel(playerid);
			}
			return 1;
		}
		case DIALOG_EV_TYPE:
		{
			if(!response) return Ev_ShowMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			if(listitem < 0 || listitem >= EV_T_COUNT) return 1;
			evType = listitem;
			new str[128];
			format(str, sizeof(str), "{00FF00}Type de l'event : %s", evTypeName[evType]);
			Ev_Msg(playerid, str);
			if(evType == EV_T_COURSE) Ev_Msg(playerid, "{FFFF00}Pense a placer les checkpoints : /ev cp a chaque endroit du parcours.");
			return 1;
		}
		case DIALOG_EV_NOM:
		{
			if(!response) return Ev_ShowMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			Ev_Create(playerid, inputtext);
			return 1;
		}
		case DIALOG_EV_PRIX:
		{
			if(!response) return Ev_ShowMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			new amount = strval(inputtext);
			if(amount < 0) amount = 0;
			evPrize = amount;
			new str[96];
			format(str, sizeof(str), "{00FF00}Recompense du gagnant : %d$", evPrize);
			Ev_Msg(playerid, str);
			return 1;
		}
		case DIALOG_EV_CP:
		{
			if(!response) return Ev_ShowMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			// item 0 = ligne de resume (voir Ev_ShowCpMenu), les actions suivent.
			switch(listitem)
			{
				case 0: Ev_ShowCpMenu(playerid);
				case 1: { Ev_AddCp(playerid); Ev_ShowCpMenu(playerid); }
				case 2:
				{
					if(evCpCount > 0) { evCpCount--; Ev_Msg(playerid, "{FFFF00}Dernier checkpoint supprime."); }
					else Ev_Msg(playerid, "{FF4444}Le parcours est deja vide.");
					Ev_ShowCpMenu(playerid);
				}
				case 3: { evCpCount = 0; Ev_Msg(playerid, "{FFFF00}Parcours efface."); Ev_ShowCpMenu(playerid); }
				case 4: ShowPlayerDialog(playerid, DIALOG_EV_SAVENAME, DIALOG_STYLE_INPUT, "Sauvegarder le parcours", "{FFFFFF}Nom du parcours :\n{AAAAAA}Un nom deja utilise sera ecrase.", "Sauver", "Retour");
				case 5: Ev_ShowRoutes(playerid);
				case 6: ShowPlayerDialog(playerid, DIALOG_EV_TOURS, DIALOG_STYLE_LIST, "Nombre de tours", "1 tour\n2 tours\n3 tours\n4 tours\n5 tours", "Choisir", "Retour");
			}
			return 1;
		}
		case DIALOG_EV_TOURS:
		{
			if(!response) return Ev_ShowCpMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			evLapTotal = listitem + 1;
			new s3[64];
			format(s3, sizeof(s3), "{00FF00}Nombre de tours : %d", evLapTotal);
			Ev_Msg(playerid, s3);
			Ev_ShowCpMenu(playerid);
			return 1;
		}
		case DIALOG_EV_SAVENAME:
		{
			if(!response) return Ev_ShowCpMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			Ev_SaveRouteAs(playerid, inputtext);
			return 1;
		}
		case DIALOG_EV_ROUTES:
		{
			if(!response) return Ev_ShowMenu(playerid), 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			Ev_LoadRouteInto(playerid, evRouteMap[playerid][listitem]);
			return 1;
		}
		case DIALOG_EV_TPALL:
		{
			if(!response) return 1;
			if(!Ev_IsAdmin(playerid)) return 1;
			Ev_TpAllRun(playerid);
			return 1;
		}
		case DIALOG_EV_PART: return 1;
	}
	return 0;
}

// ============================================================================
// COMMANDE /ev
// ============================================================================
stock Ev_Help(playerid)
{
	Ev_Msg(playerid, "{FFFF00}--- Organisation d'evenement ---");
	Ev_Msg(playerid, "{FFFFFF}/ev {AAAAAA}: ouvre le menu (tout se fait au menu)");
	Ev_Msg(playerid, "{FFFFFF}/ev creer <nom> {AAAAAA}: cree l'event a ta position");
	Ev_Msg(playerid, "{FFFFFF}/ev type | /ev prix <$> | /ev vehicule <model|0> | /ev tours <n>");
	Ev_Msg(playerid, "{FFFFFF}/ev cp {AAAAAA}: ajoute un checkpoint ici {FFFFFF}| /ev cpsuppr | /ev cpvider");
	Ev_Msg(playerid, "{FFFFFF}/ev sauver <nom> | /ev charger <nom> | /ev parcours");
	Ev_Msg(playerid, "{FFFFFF}/ev ouvrir | /ev fermer | /ev tp | /ev depart | /ev fin | /ev annuler");
	Ev_Msg(playerid, "{FFFFFF}/ev gel | /ev degel {AAAAAA}: bloque / debloque tous les participants");
	Ev_Msg(playerid, "{FFFFFF}/ev annonce <texte> {AAAAAA}: message a tout le serveur");
	Ev_Msg(playerid, "{FFFFFF}/tpall {AAAAAA}: rassemble TOUT le serveur au spawn de Los Santos");
	return 1;
}

stock Ev_Command(playerid, const cmdline[])
{
	new evPos = 0, sub[24], arg[64];
	Ev_Tok(cmdline, evPos, sub, sizeof(sub));      // consomme "/ev"
	if(Ev_Tok(cmdline, evPos, sub, sizeof(sub)) == 0) return Ev_ShowMenu(playerid);

	// --- accessible a tous ---
	if(strcmp(sub, "rejoindre", true) == 0) return Ev_Join(playerid);
	if(strcmp(sub, "quitter",   true) == 0) return Ev_Leave(playerid);
	if(strcmp(sub, "liste",     true) == 0) return Ev_ShowPart(playerid);
	if(strcmp(sub, "menu",      true) == 0) return Ev_ShowMenu(playerid);

	// --- reserve aux admins ---
	if(!Ev_IsAdmin(playerid))
	{
		Ev_MsgAdmin(playerid);
		return Ev_Msg(playerid, "{AAAAAA}Utilise /ev sans argument pour voir l'event en cours.");
	}

	if(strcmp(sub, "aide", true) == 0 || strcmp(sub, "help", true) == 0) return Ev_Help(playerid);

	if(strcmp(sub, "creer", true) == 0)
	{
		Ev_Rest(cmdline, evPos, arg, sizeof(arg));
		if(!strlen(arg)) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev creer <nom de l'event>");
		return Ev_Create(playerid, arg);
	}
	if(strcmp(sub, "type",    true) == 0) return Ev_ShowTypes(playerid);
	if(strcmp(sub, "ouvrir",  true) == 0) return Ev_Open(playerid);
	if(strcmp(sub, "fermer",  true) == 0) return Ev_Close(playerid);
	if(strcmp(sub, "tp",      true) == 0) return Ev_TeleportAll(playerid);
	if(strcmp(sub, "depart",  true) == 0) return Ev_Start(playerid);
	if(strcmp(sub, "fin",     true) == 0) return Ev_Finish(playerid);
	if(strcmp(sub, "annuler", true) == 0) return Ev_Cancel(playerid);
	if(strcmp(sub, "cp",      true) == 0) return Ev_AddCp(playerid);
	if(strcmp(sub, "parcours",true) == 0) return Ev_ShowRoutes(playerid);
	if(strcmp(sub, "tpall",   true) == 0) return Ev_CmdTpAll(playerid);

	if(strcmp(sub, "gel", true) == 0)
	{
		new n = Ev_FreezeAll(1), s2[64];
		format(s2, sizeof(s2), "{00FF00}%d participant(s) gele(s).", n);
		return Ev_Msg(playerid, s2);
	}
	if(strcmp(sub, "degel", true) == 0)
	{
		new n = Ev_FreezeAll(0), s2[64];
		format(s2, sizeof(s2), "{00FF00}%d participant(s) degele(s).", n);
		return Ev_Msg(playerid, s2);
	}
	if(strcmp(sub, "cpsuppr", true) == 0)
	{
		if(evCpCount == 0) return Ev_Msg(playerid, "{FF4444}Le parcours est deja vide.");
		evCpCount--;
		return Ev_Msg(playerid, "{FFFF00}Dernier checkpoint supprime.");
	}
	if(strcmp(sub, "cpvider", true) == 0)
	{
		evCpCount = 0;
		return Ev_Msg(playerid, "{FFFF00}Parcours efface.");
	}
	if(strcmp(sub, "prix", true) == 0)
	{
		if(Ev_Tok(cmdline, evPos, arg, sizeof(arg)) == 0) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev prix <montant>");
		evPrize = strval(arg);
		if(evPrize < 0) evPrize = 0;
		new s2[96];
		format(s2, sizeof(s2), "{00FF00}Recompense du gagnant : %d$", evPrize);
		return Ev_Msg(playerid, s2);
	}
	if(strcmp(sub, "vehicule", true) == 0)
	{
		if(Ev_Tok(cmdline, evPos, arg, sizeof(arg)) == 0) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev vehicule <model 400-611, ou 0 pour a pied>");
		new mdl = strval(arg);
		if(mdl != 0 && (mdl < 400 || mdl > 611)) return Ev_Msg(playerid, "{FF4444}Model invalide (400-611, ou 0).");
		evVehModel = mdl;
		new s2[96];
		if(mdl == 0) format(s2, sizeof(s2), "{00FF00}Event a pied : aucun vehicule ne sera fourni.");
		else format(s2, sizeof(s2), "{00FF00}Chaque participant recevra un vehicule (model %d).", mdl);
		return Ev_Msg(playerid, s2);
	}
	if(strcmp(sub, "tours", true) == 0)
	{
		if(Ev_Tok(cmdline, evPos, arg, sizeof(arg)) == 0) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev tours <1-10>");
		new n = strval(arg);
		if(n < 1 || n > 10) return Ev_Msg(playerid, "{FF4444}Entre 1 et 10 tours.");
		evLapTotal = n;
		new s2[64];
		format(s2, sizeof(s2), "{00FF00}Nombre de tours : %d", evLapTotal);
		return Ev_Msg(playerid, s2);
	}
	if(strcmp(sub, "sauver", true) == 0)
	{
		Ev_Rest(cmdline, evPos, arg, sizeof(arg));
		if(!strlen(arg)) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev sauver <nom du parcours>");
		return Ev_SaveRouteAs(playerid, arg);
	}
	if(strcmp(sub, "charger", true) == 0)
	{
		Ev_Rest(cmdline, evPos, arg, sizeof(arg));
		if(!strlen(arg)) return Ev_ShowRoutes(playerid);
		new r = Ev_FindRoute(arg);
		if(r == -1) return Ev_Msg(playerid, "{FF4444}Parcours introuvable. /ev parcours pour la liste.");
		return Ev_LoadRouteInto(playerid, r);
	}
	if(strcmp(sub, "annonce", true) == 0)
	{
		new msg[144];
		Ev_Rest(cmdline, evPos, msg, sizeof(msg));
		if(!strlen(msg)) return Ev_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/ev annonce <texte>");
		new s2[200];
		format(s2, sizeof(s2), "[EVENT] %s", msg);
		return Ev_Announce(s2);
	}

	return Ev_Help(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Ev_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		evIn[i] = 0; evAlive[i] = 0; evNextCp[i] = 0;
		evLap[i] = 1; evRank[i] = 0; evVeh[i] = 0;
	}
	evState = EV_OFF;
	evHost  = INVALID_PLAYER_ID;
	Ev_LoadRoutes();
	return 1;
}
