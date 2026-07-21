/*
	afrp_gangcreation.inc - Creation de gang par les joueurs (50.000.000$)

	Jusqu'ici seul un admin pouvait creer une faction (gang/mafia) via
	/a faction creer, avec un long flow de dialogs. Ce module ajoute un
	chemin PAYANT pour les joueurs : /gang creer <nom>.

	Reutilise exactement le meme catalogue de 6 QG "gang" (memes interieurs,
	memes coordonnees de sortie) que /a faction creer, pour rester coherent
	avec l'univers deja construit sur le serveur. Seul le type "Gang" est
	propose ici (pas Mafia, qui reste reservee a l'admin - voir demande).

	Flow :
	  /gang creer <nom>  -> verifs (pas deja en faction, argent, nom libre)
	                     -> dialog liste des 6 QG possibles
	                     -> a la validation : argent debite, faction creee
	                        a la position actuelle du joueur (entree), le
	                        joueur devient Chef (pMember/pLeader/pRank).

	Reutilise les fonctions deja existantes faction_Save() et
	faction_UpdateInfos() (definies dans LVRP.pwn) pour la persistance et
	les visuels (pickups/labels), exactement comme le fait /a faction creer.
*/

#if defined AFRP_GANGCREATION_INCLUDED
	#endinput
#endif
#define AFRP_GANGCREATION_INCLUDED

#define GANG_CREATE_COST    50000000
#define GANG_INTERIOR_COUNT 6
#define GANG_DLG_CHOOSE     9944

new const gangInt_ID[GANG_INTERIOR_COUNT]      = { 5, 3, 5, 8, 5, 2 };
new const Float:gangInt_X[GANG_INTERIOR_COUNT] = { 2352.8064, 2495.9897, 318.5954,  2807.6248, 318.57,   2454.717041 };
new const Float:gangInt_Y[GANG_INTERIOR_COUNT] = { -1180.9540, -1692.0836, 1114.4801, -1174.7563, 1115.21, -1700.871582 };
new const Float:gangInt_Z[GANG_INTERIOR_COUNT] = { 1027.9766, 1014.7422, 1083.8828, 1025.5703, 1083.98, 1013.515197 };

new gc_PendingName[MAX_PLAYERS][32];

stock GangCreate_Cmd(playerid, params[])
{
	if(PlayerInfo[playerid][pMember] >= 200 || PlayerInfo[playerid][pLeader] >= 200)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Tu fais deja partie d'une faction. Quitte-la d'abord.");

	if(strlen(params) < 4 || strlen(params) > 31)
		return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /gang creer <nom> (4 a 31 caracteres)");

	for(new f = 0; f < MAX_FACTION; f++)
	{
		if(FactionInfo[f][fCreate] == 1 && strcmp(FactionInfo[f][fName], params, true) == 0)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Ce nom est deja pris par une autre faction.");
	}

	if(PlayerInfo[playerid][pCash] < GANG_CREATE_COST)
	{
		new msg[128], costStr[24];
		FormatMoney(GANG_CREATE_COST, costStr, sizeof(costStr));
		format(msg, sizeof(msg), "{CF9756} Gang {FFFFFF} Il te faut $%s pour fonder un gang.", costStr);
		return msg_Client(playerid, COLOR_INFO, msg);
	}

	new n = 0; while(n < 31 && params[n]) { gc_PendingName[playerid][n] = params[n]; n++; } gc_PendingName[playerid][n] = 0;

	ShowPlayerDialog(playerid, GANG_DLG_CHOOSE, DIALOG_STYLE_LIST, "{9EC73D}Fonder un gang - Choisis ton QG",
		"Int 1 (Burning Desire House)\nInt 2 (Johnson House)\nInt 3 (The Crack Den)\nInt 4 (Colonel Fuhrberger's House)\nInt 5 (Ryders house)\nInt 6 (Sweet's house)",
		"Choisir", "Annuler");
	return 1;
}

stock GangCreate_Finalize(playerid, interiorChoice, const gname[])
{
	// Re-verifs : le temps a pu passer entre la commande et le choix du QG
	if(PlayerInfo[playerid][pMember] >= 200 || PlayerInfo[playerid][pLeader] >= 200)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Tu fais deja partie d'une faction.");
	if(PlayerInfo[playerid][pCash] < GANG_CREATE_COST)
	{
		new msg[128], costStr[24];
		FormatMoney(GANG_CREATE_COST, costStr, sizeof(costStr));
		format(msg, sizeof(msg), "{CF9756} Gang {FFFFFF} Il te faut $%s pour fonder un gang.", costStr);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	for(new f = 0; f < MAX_FACTION; f++)
	{
		if(FactionInfo[f][fCreate] == 1 && strcmp(FactionInfo[f][fName], gname, true) == 0)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Ce nom est deja pris.");
	}
	if(interiorChoice < 0 || interiorChoice >= GANG_INTERIOR_COUNT)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Choix invalide.");

	new Float:gx, Float:gy, Float:gz;
	GetPlayerPos(playerid, gx, gy, gz);
	new intId       = gangInt_ID[interiorChoice];
	new Float:ex    = gangInt_X[interiorChoice];
	new Float:ey    = gangInt_Y[interiorChoice];
	new Float:ez    = gangInt_Z[interiorChoice];

	// Cherche un slot libre (faction supprimee) parmi les factions existantes,
	// sinon prend un tout nouveau slot (meme logique que /a faction creer).
	new ID = -1;
	for(new fac = 0; fac < totalFactions; fac++)
	{
		if(FactionInfo[fac][fCreate] == 0) { ID = fac; break; }
	}

	SafeGivePlayerMoney(playerid, -GANG_CREATE_COST, "Fondation d'un gang");

	if(ID == -1)
	{
		new rowc = 0;
		mysql_query(MYSQL, "SELECT * FROM lvrp_factions_illegals");
		cache_get_row_count(rowc);
		rowc++;
		if(rowc > MAX_FACTION)
		{
			SafeGivePlayerMoney(playerid, GANG_CREATE_COST, "Remboursement - limite de gangs atteinte");
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Gang {FFFFFF} Nombre maximum de gangs/mafias atteint sur le serveur. Contacte un admin.");
		}
		ID = rowc - 1;
		new insSql[560];
		format(insSql, sizeof(insSql),
			"INSERT INTO lvrp_factions_illegals SET id=%d, Created=1, Owned=1, Pos_x=%f, Pos_y=%f, Pos_z=%f, Exit_x=%f, Exit_y=%f, Exit_z=%f, Safe_x=%f, Safe_y=%f, Safe_z=%f, Interior=%d, Name='%s', Type=1, VW=%d",
			rowc, gx, gy, gz, ex, ey, ez, gx, gy, gz + 2, intId, gname, ID + 200);
		mysql_pquery(MYSQL, insSql);
		totalFactions++;
	}

	FactionInfo[ID][fEntrance][0] = gx;
	FactionInfo[ID][fEntrance][1] = gy;
	FactionInfo[ID][fEntrance][2] = gz;
	FactionInfo[ID][fExit][0] = ex;
	FactionInfo[ID][fExit][1] = ey;
	FactionInfo[ID][fExit][2] = ez;
	FactionInfo[ID][fSafe][0] = gx;
	FactionInfo[ID][fSafe][1] = gy;
	FactionInfo[ID][fSafe][2] = gz + 2;
	FactionInfo[ID][fCash] = 0;
	FactionInfo[ID][FNO_Mats] = 0;
	FactionInfo[ID][FNO_Hero] = 0;
	FactionInfo[ID][FNO_Coca] = 0;
	FactionInfo[ID][FNO_Exta] = 0;
	FactionInfo[ID][fArmour] = 0.0;
	FactionInfo[ID][FNO_Arme1] = 0;  FactionInfo[ID][FNO_Amo1] = 0;
	FactionInfo[ID][FNO_Arme2] = 0;  FactionInfo[ID][FNO_Amo2] = 0;
	FactionInfo[ID][FNO_Arme3] = 0;  FactionInfo[ID][FNO_Amo3] = 0;
	FactionInfo[ID][FNO_Arme4] = 0;  FactionInfo[ID][FNO_Amo4] = 0;
	FactionInfo[ID][FNO_Arme5] = 0;  FactionInfo[ID][FNO_Amo5] = 0;
	FactionInfo[ID][FNO_Arme6] = 0;  FactionInfo[ID][FNO_Amo6] = 0;
	FactionInfo[ID][FNO_Arme7] = 0;  FactionInfo[ID][FNO_Amo7] = 0;
	FactionInfo[ID][FNO_Arme8] = 0;  FactionInfo[ID][FNO_Amo8] = 0;
	FactionInfo[ID][FNO_Arme9] = 0;  FactionInfo[ID][FNO_Amo9] = 0;
	FactionInfo[ID][FNO_Arme10] = 0; FactionInfo[ID][FNO_Amo10] = 0;
	FactionInfo[ID][fInt] = intId;
	new n = 0; while(n < 31 && gname[n]) { FactionInfo[ID][fName][n] = gname[n]; n++; } FactionInfo[ID][fName][n] = 0;
	FactionInfo[ID][fLock] = 1;
	FactionInfo[ID][fOwned] = 1;
	FactionInfo[ID][fType] = 1; // Gang
	FactionInfo[ID][fColor] = 0;
	FactionInfo[ID][fChoseColor] = 0;
	FactionInfo[ID][fWV] = ID + 200;
	strmid(FactionInfo[ID][fRank1], "Nouveau", 0, strlen("Nouveau"), 32);
	strmid(FactionInfo[ID][fRank2], "Membre", 0, strlen("Membre"), 32);
	strmid(FactionInfo[ID][fRank3], "Membre", 0, strlen("Membre"), 32);
	strmid(FactionInfo[ID][fRank4], "Membre", 0, strlen("Membre"), 32);
	strmid(FactionInfo[ID][fRank5], "Sous-Chef", 0, strlen("Sous-Chef"), 32);
	strmid(FactionInfo[ID][fRank6], "Chef", 0, strlen("Chef"), 32);
	FactionInfo[ID][fCreate] = 1;

	// Slot reutilise (existait deja en DB, fCreate remis a 1) : persiste via UPDATE.
	// Slot flambant neuf : deja insere ci-dessus, faction_Save() met a jour le reste.
	faction_Save(ID);
	faction_UpdateInfos(ID);

	// Le fondateur devient Chef de son gang
	PlayerInfo[playerid][pMember] = ID + 200;
	PlayerInfo[playerid][pLeader] = ID + 200;
	PlayerInfo[playerid][pRank] = 6;
	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Member", PlayerInfo[playerid][pMember]);
	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Leader", PlayerInfo[playerid][pLeader]);
	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Rank", PlayerInfo[playerid][pRank]);
	GangTag_Update(playerid); // [GANG TAG] affiche direct le nom du gang au-dessus de la tete

	new okStr[220];
	format(okStr, sizeof(okStr), "{9EC73D}[Gang]{FFFFFF} Felicitations, {FFFFB2}%s{FFFFFF} est fonde ! Tu en es le Chef. QG installe a ta position actuelle (entree).", gname);
	msg_Client(playerid, COLOR_WHITE, okStr);
	return 1;
}

stock GangCreate_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != GANG_DLG_CHOOSE) return 0;
	if(!response)
	{
		msg_Client(playerid, COLOR_WHITE, "{9EC73D}[Gang]{FFFFFF} Fondation annulee.");
		return 1;
	}
	new name[32];
	new n = 0; while(n < 31 && gc_PendingName[playerid][n]) { name[n] = gc_PendingName[playerid][n]; n++; } name[n] = 0;
	GangCreate_Finalize(playerid, listitem, name);
	return 1;
}
