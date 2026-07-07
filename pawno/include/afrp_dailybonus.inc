/*
	afrp_dailybonus.inc - Bonus de connexion quotidien (5000$ + item aleatoire)

	A chaque PREMIERE connexion de la journee (UTC, jour calendaire), le
	joueur recoit automatiquement :
	- 5000$ cash
	- 1-3x un item aleatoire (Eau, Soda, Jus de fruit, Briquet)

	Stockage : colonne lvrp_users.LastDailyBonus (jour epoch = gettime()/86400).
	Anti double-claim : compare le jour stocke au jour actuel.

	Hook : appeler DailyBonus_CheckOnLogin(playerid) juste apres un login
	reussi (gPlayerLogged[playerid] = 1), PAS pendant l'attente d'approbation.
*/

#if defined AFRP_DAILYBONUS_INCLUDED
	#endinput
#endif
#define AFRP_DAILYBONUS_INCLUDED

#define DAILYBONUS_CASH 5000

static const dailyBonusItems[4] = {1668, 2601, 19563, 2351}; // Eau, Soda, Jus de fruit, Briquet

stock DailyBonus_CheckOnLogin(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(PlayerInfo[playerid][pSQLID] <= 0) return 0;
	new sql[128];
	format(sql, sizeof(sql), "SELECT LastDailyBonus FROM lvrp_users WHERE id=%d LIMIT 1", PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, sql, "DailyBonus_OnLoadCallback", "i", playerid);
	return 1;
}

forward DailyBonus_OnLoadCallback(playerid);
public DailyBonus_OnLoadCallback(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	new lastDay = 0;
	if(rows > 0) cache_get_value_name_int(0, "LastDailyBonus", lastDay);

	new today = gettime() / 86400;
	if(lastDay == today) return 0; // deja recupere aujourd'hui

	// Donne le cash
	GivePlayerMoney(playerid, DAILYBONUS_CASH);
	PlayerInfo[playerid][pCash] += DAILYBONUS_CASH;

	// Donne un item aleatoire (1-3 unites)
	new pickedItem = dailyBonusItems[random(sizeof(dailyBonusItems))];
	new qty = 1 + random(3);
	inventory_UpdateItem(playerid, pickedItem, qty);

	new itemName[32];
	itemName = GetItemName(pickedItem);

	new msg[220];
	format(msg, sizeof(msg),
		"{FFD700}[Bonus quotidien]{FFFFFF} Premiere connexion du jour ! Tu recois {00FF00}%d ${FFFFFF} + {FFFF00}%dx %s{FFFFFF}.",
		DAILYBONUS_CASH, qty, itemName);
	SendClientMessage(playerid, -1, msg);
	PlayerPlaySound(playerid, 1150, 0.0, 0.0, 0.0);

	// Persiste le jour de claim
	new updSql[128];
	format(updSql, sizeof(updSql), "UPDATE lvrp_users SET LastDailyBonus=%d WHERE id=%d", today, PlayerInfo[playerid][pSQLID]);
	mysql_pquery(MYSQL, updSql);
	return 1;
}
