/*
	afrp_topjour.inc - Classement des meilleurs joueurs du jour

	Chaque joueur accumule des POINTS D'ACTIVITE pendant la journee. A minuit,
	le podium est annonce a tout le serveur et les trois premiers sont payes.

	=== COMMENT ON GAGNE DES POINTS ===
	  +1  par minute reellement jouee (voir anti-AFK plus bas)
	  +25 par objectif quotidien accompli

	=== ANTI-AFK ===
	Une minute ne compte QUE si les trois conditions sont reunies :
	  1. le client envoie toujours des donnees (un joueur en pause / alt-tab
	     arrete d'appeler OnPlayerUpdate : il est detecte et ne marque plus) ;
	  2. le joueur a parcouru au moins 3 metres depuis la minute precedente,
	     OU a parle / tape une commande dans les 2 dernieres minutes ;
	  3. il est connecte et identifie.
	Sans ca, il suffirait de laisser son PC allume la nuit pour rafler 200k.

	=== RECOMPENSES (a minuit pile, heure du serveur) ===
	  1er : 200.000$   2e : 100.000$   3e : 50.000$
	Un gagnant hors ligne a minuit n'est pas lese : son gain est mis de cote
	et lui est verse automatiquement a sa prochaine connexion.

	=== AFFICHAGE ===
	  - Panneau textdraw en haut a droite : top 3 en direct + ta propre place.
	    (La place est libre depuis que le bloc vie/faim est passe en bas a gauche.)
	  - /top  : classement complet (20 premiers) + explication des points.
	  - /toptd: masquer / reafficher le panneau.

	=== PERSISTANCE ===
	topjour.cfg garde la date du jour, les points et les gains en attente. Le
	classement survit donc a un redemarrage du serveur en pleine journee, et si
	le serveur etait eteint a minuit la remise des prix se fait au demarrage
	suivant (le changement de date est detecte, pas l'heure exacte).

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit      -> TopJour_Init()
	- OnPlayerConnect     -> TopJour_OnConnect(playerid)
	- OnPlayerDisconnect  -> TopJour_OnDisconnect(playerid)
	- apres le login      -> TopJour_OnLogin(playerid)
	- OnPlayerUpdate      -> TopJour_OnUpdate(playerid)
	- OnPlayerText        -> TopJour_OnAction(playerid)
	- OnPlayerCommandText -> TopJour_OnAction(playerid)
	- OnDialogResponse    -> TopJour_OnDialog(playerid, dialogid, response, listitem)
*/

#if defined AFRP_TOPJOUR_INCLUDED
	#endinput
#endif
#define AFRP_TOPJOUR_INCLUDED

#define TJ_MAX_ENTRY     200        // joueurs classes dans une journee
#define TJ_MAX_PENDING   50         // gains en attente (gagnants hors ligne)
#define TJ_RANK_KEEP     20         // taille du classement affiche dans /top
#define TJ_FILE          "topjour.cfg"

// --- Recompenses ---
#define TJ_PRIZE_1       200000
#define TJ_PRIZE_2       100000
#define TJ_PRIZE_3       50000

// --- Points ---
#define TJ_PTS_MINUTE    1
#define TJ_PTS_OBJECTIF  25

// --- Anti-AFK ---
#define TJ_PAUSE_MS      60000      // aucun paquet client depuis 60s = en pause
#define TJ_MOVE_MIN      3.0        // metres a parcourir dans la minute
#define TJ_ACTION_SEC    120        // ou avoir agi dans les 2 dernieres minutes

// --- Panneau (haut droite, place liberee par le deplacement du HUD) ---
// Y=26 et pas plus haut : la barre d'endurance occupe deja Y 7..17.
// En dessous, le ping est a Y=100 et le panneau des objectifs a Y=125 :
// les 5 lignes (26..71) tiennent sans toucher ni l'un ni l'autre.
#define TJ_TD_X          500.0
#define TJ_TD_Y          26.0
#define TJ_TD_LINE_H     9.0

#define DIALOG_TJ_TOP    9620

// ============================================================================
// DONNEES
// ============================================================================
new tjSqlid[TJ_MAX_ENTRY];
new tjName[TJ_MAX_ENTRY][MAX_PLAYER_NAME];
new tjPoints[TJ_MAX_ENTRY];
new tjSlotPris[TJ_MAX_ENTRY];

new tjPendSqlid[TJ_MAX_PENDING];
new tjPendCash[TJ_MAX_PENDING];
new tjPendPris[TJ_MAX_PENDING];

new tjRank[TJ_RANK_KEEP];
new tjRankCount;

new tjDay;                          // AAAAMMJJ, change a minuit local

// --- par joueur ---
new tjIdx[MAX_PLAYERS];             // index dans le classement, -1 si aucun
new tjLastUpdate[MAX_PLAYERS];      // GetTickCount du dernier paquet client
new tjLastAction[MAX_PLAYERS];      // gettime() du dernier chat / commande
new Float:tjLastX[MAX_PLAYERS], Float:tjLastY[MAX_PLAYERS], Float:tjLastZ[MAX_PLAYERS];
new tjTdOn[MAX_PLAYERS];
new PlayerText:tjTd[MAX_PLAYERS][5];

// ============================================================================
// OUTILS
// ============================================================================
stock TopJour_Today()
{
	new tjY, tjM, tjD;
	getdate(tjY, tjM, tjD);
	return tjY * 10000 + tjM * 100 + tjD;
}

stock TopJour_Find(tjWanted)
{
	if(tjWanted <= 0) return -1;
	for(new k = 0; k < TJ_MAX_ENTRY; k++)
		if(tjSlotPris[k] == 1 && tjSqlid[k] == tjWanted) return k;
	return -1;
}

stock TopJour_OnlineBySqlid(tjWanted)
{
	if(tjWanted <= 0) return INVALID_PLAYER_ID;
	for(new k = 0; k < MAX_PLAYERS; k++)
	{
		if(!IsPlayerConnected(k) || IsPlayerNPC(k)) continue;
		if(PlayerInfo[k][pSQLID] == tjWanted) return k;
	}
	return INVALID_PLAYER_ID;
}

// Recupere (ou cree) la ligne de classement du joueur.
stock TopJour_Entry(playerid)
{
	if(tjIdx[playerid] != -1) return tjIdx[playerid];

	new tjMy = PlayerInfo[playerid][pSQLID];
	if(tjMy <= 0) return -1;                 // pas encore identifie

	new k = TopJour_Find(tjMy);
	if(k == -1)
	{
		for(new n = 0; n < TJ_MAX_ENTRY; n++)
			if(tjSlotPris[n] == 0) { k = n; break; }
		if(k == -1) return -1;               // classement plein pour aujourd'hui
		tjSlotPris[k] = 1;
		tjSqlid[k]    = tjMy;
		tjPoints[k]   = 0;
	}
	format(tjName[k], MAX_PLAYER_NAME, "%s", PlayerInfo[playerid][pName]);
	tjIdx[playerid] = k;
	return k;
}

// Classement : selectionne les TJ_RANK_KEEP meilleurs.
stock TopJour_BuildRank()
{
	new tjDeja[TJ_MAX_ENTRY];
	tjRankCount = 0;
	for(new r = 0; r < TJ_RANK_KEEP; r++)
	{
		new tjBest = -1;
		for(new k = 0; k < TJ_MAX_ENTRY; k++)
		{
			if(tjSlotPris[k] != 1 || tjDeja[k] == 1) continue;
			if(tjPoints[k] <= 0) continue;
			if(tjBest == -1 || tjPoints[k] > tjPoints[tjBest]) tjBest = k;
		}
		if(tjBest == -1) break;
		tjDeja[tjBest] = 1;
		tjRank[r] = tjBest;
		tjRankCount++;
	}
	return tjRankCount;
}

// Place du joueur (1 = premier), 0 s'il n'a aucun point.
stock TopJour_MyRank(playerid)
{
	new k = tjIdx[playerid];
	if(k == -1 || tjSlotPris[k] != 1 || tjPoints[k] <= 0) return 0;
	new tjBetter = 0;
	for(new n = 0; n < TJ_MAX_ENTRY; n++)
	{
		if(tjSlotPris[n] != 1 || n == k) continue;
		if(tjPoints[n] > tjPoints[k]) tjBetter++;
	}
	return tjBetter + 1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock TopJour_Field(const tjLine[], &k, tjDest[], tjDestSize)
{
	new p = 0;
	while(p < tjDestSize - 1 && tjLine[k] && tjLine[k] != '|' && tjLine[k] != '\r' && tjLine[k] != '\n')
		tjDest[p++] = tjLine[k++];
	tjDest[p] = 0;
	if(tjLine[k] == '|') k++;
	return p;
}

stock TopJour_Save()
{
	if(fexist(TJ_FILE)) fremove(TJ_FILE);
	new File:f = fopen(TJ_FILE, io_write);
	if(!f) return 0;

	new tjLine[128];
	format(tjLine, sizeof(tjLine), "D|%d\n", tjDay);
	fwrite(f, tjLine);
	for(new k = 0; k < TJ_MAX_ENTRY; k++)
	{
		if(tjSlotPris[k] != 1 || tjPoints[k] <= 0) continue;
		format(tjLine, sizeof(tjLine), "P|%d|%d|%s\n", tjSqlid[k], tjPoints[k], tjName[k]);
		fwrite(f, tjLine);
	}
	for(new k = 0; k < TJ_MAX_PENDING; k++)
	{
		if(tjPendPris[k] != 1) continue;
		format(tjLine, sizeof(tjLine), "G|%d|%d\n", tjPendSqlid[k], tjPendCash[k]);
		fwrite(f, tjLine);
	}
	fclose(f);
	return 1;
}

stock TopJour_Load()
{
	for(new k = 0; k < TJ_MAX_ENTRY; k++)  { tjSlotPris[k] = 0; tjPoints[k] = 0; }
	for(new k = 0; k < TJ_MAX_PENDING; k++) tjPendPris[k] = 0;
	tjDay = TopJour_Today();

	if(!fexist(TJ_FILE)) return 0;
	new File:f = fopen(TJ_FILE, io_read);
	if(!f) return 0;

	new tjLine[160], tjBuf[48], nbP = 0, nbG = 0, tjFileDay = 0;
	while(fread(f, tjLine))
	{
		if(strlen(tjLine) < 3) continue;
		new k = 0;
		TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf));

		if(strcmp(tjBuf, "D", true) == 0)
		{
			TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf));
			tjFileDay = strval(tjBuf);
		}
		else if(strcmp(tjBuf, "P", true) == 0)
		{
			if(nbP >= TJ_MAX_ENTRY) continue;
			TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf)); tjSqlid[nbP]  = strval(tjBuf);
			TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf)); tjPoints[nbP] = strval(tjBuf);
			TopJour_Field(tjLine, k, tjName[nbP], MAX_PLAYER_NAME);
			if(tjSqlid[nbP] <= 0 || tjPoints[nbP] <= 0) continue;
			tjSlotPris[nbP] = 1;
			nbP++;
		}
		else if(strcmp(tjBuf, "G", true) == 0)
		{
			if(nbG >= TJ_MAX_PENDING) continue;
			TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf)); tjPendSqlid[nbG] = strval(tjBuf);
			TopJour_Field(tjLine, k, tjBuf, sizeof(tjBuf)); tjPendCash[nbG]  = strval(tjBuf);
			if(tjPendSqlid[nbG] <= 0 || tjPendCash[nbG] <= 0) continue;
			tjPendPris[nbG] = 1;
			nbG++;
		}
	}
	fclose(f);

	printf("[TopJour] %d joueur(s) classe(s), %d gain(s) en attente (fichier du %d)", nbP, nbG, tjFileDay);

	// Le serveur etait eteint au changement de jour : on solde la journee
	// enregistree avant de repartir a zero, personne ne perd sa recompense.
	if(tjFileDay != 0 && tjFileDay != tjDay)
	{
		tjDay = tjFileDay;
		TopJour_Payout();
	}
	return nbP;
}

// ============================================================================
// GAINS EN ATTENTE
// ============================================================================
stock TopJour_AddPending(tjWho, tjCash)
{
	for(new k = 0; k < TJ_MAX_PENDING; k++)
	{
		if(tjPendPris[k] == 1 && tjPendSqlid[k] == tjWho) { tjPendCash[k] += tjCash; return 1; }
	}
	for(new k = 0; k < TJ_MAX_PENDING; k++)
	{
		if(tjPendPris[k] != 0) continue;
		tjPendPris[k]  = 1;
		tjPendSqlid[k] = tjWho;
		tjPendCash[k]  = tjCash;
		return 1;
	}
	return 0;
}

stock TopJour_ClaimPending(playerid)
{
	new tjMy = PlayerInfo[playerid][pSQLID];
	if(tjMy <= 0) return 0;
	for(new k = 0; k < TJ_MAX_PENDING; k++)
	{
		if(tjPendPris[k] != 1 || tjPendSqlid[k] != tjMy) continue;
		new tjCash = tjPendCash[k];
		tjPendPris[k] = 0;
		tjPendCash[k] = 0;
		SafeGivePlayerMoney(playerid, tjCash, "Top du jour");
		new tjStr[160];
		format(tjStr, sizeof(tjStr), "{FFD700} Top du jour {FFFFFF} Tu etais au podium hier : {FFFFB2}+%d${FFFFFF} verses sur toi.", tjCash);
		msg_Client(playerid, COLOR_WHITE, tjStr);
		TopJour_Save();
		return 1;
	}
	return 0;
}

// ============================================================================
// POINTS
// ============================================================================
stock TopJour_AddPoints(playerid, tjNb)
{
	if(tjNb <= 0) return 0;
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	new k = TopJour_Entry(playerid);
	if(k == -1) return 0;
	tjPoints[k] += tjNb;
	TopJour_TdRefresh(playerid);
	return 1;
}

// Vrai si la minute ecoulee doit compter (voir l'anti-AFK en tete de fichier).
stock TopJour_MinuteCompte(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(PlayerInfo[playerid][pSQLID] <= 0) return 0;

	// 1) client en pause / alt-tab : il n'envoie plus rien
	if(GetTickCount() - tjLastUpdate[playerid] > TJ_PAUSE_MS) return 0;

	// 2) deplacement OU action recente
	new Float:tjX, Float:tjY, Float:tjZ;
	GetPlayerPos(playerid, tjX, tjY, tjZ);
	new Float:tjDist = floatsqroot(
		floatpower(tjX - tjLastX[playerid], 2.0) +
		floatpower(tjY - tjLastY[playerid], 2.0) +
		floatpower(tjZ - tjLastZ[playerid], 2.0));
	tjLastX[playerid] = tjX;
	tjLastY[playerid] = tjY;
	tjLastZ[playerid] = tjZ;

	if(tjDist >= TJ_MOVE_MIN) return 1;
	if(gettime() - tjLastAction[playerid] <= TJ_ACTION_SEC) return 1;
	return 0;
}

// ============================================================================
// REMISE DES PRIX
// ============================================================================
stock TopJour_ResetDay()
{
	for(new k = 0; k < TJ_MAX_ENTRY; k++) { tjSlotPris[k] = 0; tjPoints[k] = 0; tjSqlid[k] = 0; }
	for(new k = 0; k < MAX_PLAYERS; k++)  tjIdx[k] = -1;
	tjRankCount = 0;
	tjDay = TopJour_Today();
	TopJour_Save();
	for(new k = 0; k < MAX_PLAYERS; k++)
		if(IsPlayerConnected(k) && !IsPlayerNPC(k)) TopJour_TdRefresh(k);
	return 1;
}

stock TopJour_Payout()
{
	TopJour_BuildRank();
	if(tjRankCount == 0)
	{
		TopJour_ResetDay();
		return 0;
	}

	new tjPrize[3];
	tjPrize[0] = TJ_PRIZE_1;
	tjPrize[1] = TJ_PRIZE_2;
	tjPrize[2] = TJ_PRIZE_3;

	new tjStr[176];
	SendClientMessageToAll(0xFFD700FF, "|_____________ TOP DU JOUR _____________|");
	for(new r = 0; r < 3 && r < tjRankCount; r++)
	{
		new k = tjRank[r];
		format(tjStr, sizeof(tjStr), "   %d. %s  -  %d points  -  %d$", r + 1, tjName[k], tjPoints[k], tjPrize[r]);
		SendClientMessageToAll(0xFFD700FF, tjStr);

		new pid = TopJour_OnlineBySqlid(tjSqlid[k]);
		if(pid != INVALID_PLAYER_ID)
		{
			SafeGivePlayerMoney(pid, tjPrize[r], "Top du jour");
			format(tjStr, sizeof(tjStr), "{FFD700} Top du jour {FFFFFF} Tu finis {FFFFB2}%d{FFFFFF} de la journee : {FFFFB2}+%d${FFFFFF} !", r + 1, tjPrize[r]);
			msg_Client(pid, COLOR_WHITE, tjStr);
			PlayerPlaySound(pid, 1057, 0.0, 0.0, 0.0);
		}
		else
		{
			// Hors ligne a minuit : le gain l'attend a sa prochaine connexion.
			TopJour_AddPending(tjSqlid[k], tjPrize[r]);
		}
	}
	SendClientMessageToAll(0xFFD700FF, "|__ Nouvelle journee : le classement repart a zero (/top) __|");

	TopJour_ResetDay();
	return 1;
}

// ============================================================================
// PANNEAU TEXTDRAW
// ============================================================================
stock TopJour_TdCreate(playerid)
{
	if(tjTd[playerid][0] != PlayerText:INVALID_TEXT_DRAW) return 1;
	for(new l = 0; l < 5; l++)
	{
		tjTd[playerid][l] = CreatePlayerTextDraw(playerid, TJ_TD_X, TJ_TD_Y + (float(l) * TJ_TD_LINE_H), " ");
		PlayerTextDrawFont(playerid, tjTd[playerid][l], 1);
		PlayerTextDrawLetterSize(playerid, tjTd[playerid][l], 0.19, 0.9);
		PlayerTextDrawColor(playerid, tjTd[playerid][l], 0xFFFFFFFF);
		PlayerTextDrawSetShadow(playerid, tjTd[playerid][l], 0);
		PlayerTextDrawSetOutline(playerid, tjTd[playerid][l], 1);
		PlayerTextDrawBackgroundColor(playerid, tjTd[playerid][l], 0x000000CC);
		PlayerTextDrawSetProportional(playerid, tjTd[playerid][l], 1);
	}
	PlayerTextDrawColor(playerid, tjTd[playerid][0], 0xFFD700FF);
	return 1;
}

stock TopJour_TdRefresh(playerid)
{
	if(tjTdOn[playerid] != 1) return 0;
	if(tjTd[playerid][0] == PlayerText:INVALID_TEXT_DRAW) return 0;

	TopJour_BuildRank();
	PlayerTextDrawSetString(playerid, tjTd[playerid][0], "~y~TOP DU JOUR");

	new tjStr[96];
	for(new r = 0; r < 3; r++)
	{
		if(r < tjRankCount)
		{
			new k = tjRank[r];
			format(tjStr, sizeof(tjStr), "%d. %s  %d", r + 1, tjName[k], tjPoints[k]);
		}
		else format(tjStr, sizeof(tjStr), "%d. ---", r + 1);
		PlayerTextDrawSetString(playerid, tjTd[playerid][r + 1], tjStr);
	}

	new tjPlace = TopJour_MyRank(playerid);
	new k2 = tjIdx[playerid];
	new tjMyPts = (k2 != -1 && tjSlotPris[k2] == 1) ? tjPoints[k2] : 0;
	if(tjPlace == 0) format(tjStr, sizeof(tjStr), "~w~Toi: non classe");
	else             format(tjStr, sizeof(tjStr), "~w~Toi: %d  (%d pts)", tjPlace, tjMyPts);
	PlayerTextDrawSetString(playerid, tjTd[playerid][4], tjStr);

	for(new l = 0; l < 5; l++) PlayerTextDrawShow(playerid, tjTd[playerid][l]);
	return 1;
}

stock TopJour_TdShow(playerid)
{
	TopJour_TdCreate(playerid);
	tjTdOn[playerid] = 1;
	TopJour_TdRefresh(playerid);
	return 1;
}

stock TopJour_TdHide(playerid)
{
	tjTdOn[playerid] = 0;
	if(tjTd[playerid][0] == PlayerText:INVALID_TEXT_DRAW) return 0;
	for(new l = 0; l < 5; l++) PlayerTextDrawHide(playerid, tjTd[playerid][l]);
	return 1;
}

stock TopJour_TdToggle(playerid)
{
	if(tjTdOn[playerid] == 1)
	{
		TopJour_TdHide(playerid);
		msg_Client(playerid, COLOR_WHITE, "{FFD700} Top du jour {FFFFFF} Panneau masque. {FFFFB2}/toptd{FFFFFF} pour le reafficher.");
	}
	else
	{
		TopJour_TdShow(playerid);
		msg_Client(playerid, COLOR_WHITE, "{FFD700} Top du jour {FFFFFF} Panneau affiche.");
	}
	return 1;
}

// ============================================================================
// TIMER (1 minute)
// ============================================================================
forward TopJour_Tick();
public TopJour_Tick()
{
	// Changement de jour = remise des prix, ou que l'on soit dans la minute.
	if(TopJour_Today() != tjDay)
	{
		TopJour_Payout();
		return 1;
	}

	for(new k = 0; k < MAX_PLAYERS; k++)
	{
		if(!IsPlayerConnected(k) || IsPlayerNPC(k)) continue;
		if(TopJour_MinuteCompte(k)) TopJour_AddPoints(k, TJ_PTS_MINUTE);
		else TopJour_TdRefresh(k);
	}
	TopJour_Save();
	return 1;
}

// ============================================================================
// HOOKS
// ============================================================================
stock TopJour_OnConnect(playerid)
{
	tjIdx[playerid]        = -1;
	tjLastUpdate[playerid] = GetTickCount();
	tjLastAction[playerid] = gettime();
	tjLastX[playerid]      = 0.0;
	tjLastY[playerid]      = 0.0;
	tjLastZ[playerid]      = 0.0;
	tjTdOn[playerid]       = 0;
	for(new l = 0; l < 5; l++) tjTd[playerid][l] = PlayerText:INVALID_TEXT_DRAW;
	return 1;
}

stock TopJour_OnDisconnect(playerid)
{
	if(tjTd[playerid][0] != PlayerText:INVALID_TEXT_DRAW)
		for(new l = 0; l < 5; l++) PlayerTextDrawDestroy(playerid, tjTd[playerid][l]);
	TopJour_OnConnect(playerid);
	return 1;
}

stock TopJour_OnLogin(playerid)
{
	TopJour_Entry(playerid);
	TopJour_ClaimPending(playerid);
	GetPlayerPos(playerid, tjLastX[playerid], tjLastY[playerid], tjLastZ[playerid]);
	tjLastUpdate[playerid] = GetTickCount();
	tjLastAction[playerid] = gettime();
	TopJour_TdShow(playerid);
	return 1;
}

// Appele depuis OnPlayerUpdate : doit rester le plus leger possible.
stock TopJour_OnUpdate(playerid)
{
	tjLastUpdate[playerid] = GetTickCount();
	return 1;
}

// Appele quand le joueur parle ou tape une commande.
stock TopJour_OnAction(playerid)
{
	tjLastAction[playerid] = gettime();
	return 1;
}

// ============================================================================
// /top
// ============================================================================
stock TopJour_ShowDialog(playerid)
{
	TopJour_BuildRank();

	new tjBody[1600], tjStr[176];
	format(tjBody, sizeof(tjBody), "{FFD700}Classement du jour{FFFFFF} - remise des prix a minuit\n{AAAAAA}1er 200.000$   2e 100.000$   3e 50.000$\n\n");

	if(tjRankCount == 0) strcat(tjBody, "{AAAAAA}Personne n'a encore marque de point aujourd'hui.\n");
	else
	{
		for(new r = 0; r < tjRankCount; r++)
		{
			new k = tjRank[r];
			new tjIsMe = (tjIdx[playerid] == k);
			if(r == 0)      format(tjStr, sizeof(tjStr), "{FFD700}%d. %s{FFFFFF} - %d pts%s\n", r + 1, tjName[k], tjPoints[k], (tjIsMe) ? (" {00FF00}(toi)") : (""));
			else if(r == 1) format(tjStr, sizeof(tjStr), "{C0C0C0}%d. %s{FFFFFF} - %d pts%s\n", r + 1, tjName[k], tjPoints[k], (tjIsMe) ? (" {00FF00}(toi)") : (""));
			else if(r == 2) format(tjStr, sizeof(tjStr), "{CD7F32}%d. %s{FFFFFF} - %d pts%s\n", r + 1, tjName[k], tjPoints[k], (tjIsMe) ? (" {00FF00}(toi)") : (""));
			else            format(tjStr, sizeof(tjStr), "{FFFFFF}%d. %s - %d pts%s\n", r + 1, tjName[k], tjPoints[k], (tjIsMe) ? (" {00FF00}(toi)") : (""));
			strcat(tjBody, tjStr);
		}
	}

	new tjPlace = TopJour_MyRank(playerid);
	new k2 = tjIdx[playerid];
	new tjMyPts = (k2 != -1 && tjSlotPris[k2] == 1) ? tjPoints[k2] : 0;
	if(tjPlace == 0) strcat(tjBody, "\n{AAAAAA}Tu n'as pas encore de point aujourd'hui.\n");
	else
	{
		format(tjStr, sizeof(tjStr), "\n{FFFFFF}Ta place : {FFFF00}%d{FFFFFF}  ({FFFF00}%d{FFFFFF} points)\n", tjPlace, tjMyPts);
		strcat(tjBody, tjStr);
	}

	strcat(tjBody, "\n{AAAAAA}Comment marquer des points :\n");
	strcat(tjBody, "{AAAAAA}  +1 par minute reellement jouee\n");
	strcat(tjBody, "{AAAAAA}  +25 par objectif quotidien accompli\n");
	strcat(tjBody, "{AAAAAA}Rester AFK ne rapporte rien : il faut bouger ou parler.\n");
	strcat(tjBody, "{AAAAAA}/toptd pour masquer le panneau a l'ecran.");

	return ShowPlayerDialog(playerid, DIALOG_TJ_TOP, DIALOG_STYLE_MSGBOX, "Meilleurs joueurs du jour", tjBody, "Fermer", "");
}

stock TopJour_OnDialog(playerid, dialogid, response, listitem)
{
	#pragma unused response
	#pragma unused listitem
	if(dialogid != DIALOG_TJ_TOP) return 0;
	return 1;
}

// Commande admin : force la remise des prix tout de suite (test).
stock TopJour_CmdForce(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 4)
		return msg_Client(playerid, COLOR_WHITE, "{FF4444} Top du jour {FFFFFF} Reserve admin niveau 4+."), 1;
	msg_Client(playerid, COLOR_WHITE, "{FFD700} Top du jour {FFFFFF} Remise des prix forcee.");
	TopJour_Payout();
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
stock TopJour_Init()
{
	for(new k = 0; k < MAX_PLAYERS; k++) TopJour_OnConnect(k);
	TopJour_Load();
	SetTimer("TopJour_Tick", 60000, true);
	return 1;
}
