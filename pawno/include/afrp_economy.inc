/*
	afrp_economy.inc - Petits boulots, marche joueur-a-joueur, monitoring admin

	1) PETITS BOULOTS (/travailler)
	   Revenu de secours SANS AUCUNE condition (pas de vehicule, pas de
	   permis, pas de lieu precis) pour les nouveaux joueurs qui n'ont pas
	   encore lance un vrai job. Cooldown 10 min, montant modeste (300-700$)
	   pour ne pas concurrencer les vrais jobs.

	2) MARCHE JOUEUR-A-JOUEUR (annonces classees)
	   /annonce <prix> <description> - poste une annonce (1 active max)
	   /annonces                     - liste les annonces actives
	   /retirerannonce               - retire la sienne
	   Le contact acheteur/vendeur se fait par /pm, l'echange (argent+objet)
	   se fait manuellement en RP - pas d'escrow automatique (volontaire,
	   pour eviter tout bug de duplication d'argent/objet).

	3) MONITORING ADMIN (/eco)
	   Vue d'ensemble : argent total en circulation (cash+banque), moyenne
	   par joueur, top 5 des plus riches.
*/

#if defined AFRP_ECONOMY_INCLUDED
	#endinput
#endif
#define AFRP_ECONOMY_INCLUDED

#define ODDJOB_COOLDOWN     600   // 10 min
#define ODDJOB_MIN_REWARD   300
#define ODDJOB_MAX_REWARD   700
#define MARKET_MAX_DESC     80
#define DIALOG_MARKET_LIST  9740
#define DIALOG_MARKET_ITEM  9741
#define DIALOG_ECO_STATS    9742

new oddJobLastUse[MAX_PLAYERS];
new marketSelectedId[MAX_PLAYERS]; // id de l'annonce en cours de consultation

static const oddJobFlavors[][] = {
	"Vous avez aide a decharger un camion de livraison.",
	"Vous avez nettoye la terrasse d'un restaurant.",
	"Vous avez promene le chien d'un voisin.",
	"Vous avez lave des vitres de magasin.",
	"Vous avez aide a demenager des cartons.",
	"Vous avez distribue des flyers en ville.",
	"Vous avez fait les courses pour une personne agee.",
	"Vous avez tondu une pelouse.",
	"Vous avez aide a ranger un entrepot.",
	"Vous avez lave des voitures sur un parking."
};

stock Economy_Init()
{
	mysql_tquery(MYSQL,
		"CREATE TABLE IF NOT EXISTS `lvrp_marketplace` (\
			`id` INT AUTO_INCREMENT PRIMARY KEY,\
			`SQLid` INT NOT NULL,\
			`SellerName` VARCHAR(32) NOT NULL DEFAULT '',\
			`Price` INT NOT NULL DEFAULT 0,\
			`Description` VARCHAR(80) NOT NULL DEFAULT '',\
			`Date` INT NOT NULL DEFAULT 0,\
			`Active` TINYINT NOT NULL DEFAULT 1\
		) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "", "");
	return 1;
}

// ==================== 1) PETITS BOULOTS ====================

stock OddJob_Cmd(playerid)
{
	new now = gettime();
	if(now - oddJobLastUse[playerid] < ODDJOB_COOLDOWN)
	{
		new left = ODDJOB_COOLDOWN - (now - oddJobLastUse[playerid]);
		new msg[120];
		format(msg, sizeof(msg), "{FF4444}[Petit boulot]{FFFFFF} Reposez-vous encore {FFFF00}%dm %ds{FFFFFF}.", left/60, left%60);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}
	oddJobLastUse[playerid] = now;

	new reward = ODDJOB_MIN_REWARD + random(ODDJOB_MAX_REWARD - ODDJOB_MIN_REWARD + 1);
	GivePlayerMoney(playerid, reward);
	PlayerInfo[playerid][pCash] += reward;

	new flavorIdx = random(sizeof(oddJobFlavors));
	new msg[180];
	format(msg, sizeof(msg), "{00FF00}[Petit boulot]{FFFFFF} %s {FFFFFF}+{00FF00}%d $", oddJobFlavors[flavorIdx], reward);
	SendClientMessage(playerid, -1, msg);
	ApplyAnimation(playerid, "BOMBER", "BOM_Plant_Crouch_Out", 2.0, 0, 0, 0, 0, 0);
	return 1;
}

stock OddJob_OnDisconnect(playerid)
{
	oddJobLastUse[playerid] = 0;
	return 1;
}

// ==================== 2) MARCHE JOUEUR-A-JOUEUR ====================

stock Market_CmdPost(playerid, price, const desc[])
{
	if(PlayerInfo[playerid][pSQLID] <= 0) return 0;
	if(price <= 0 || price > 999999999)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Annonce]{FFFFFF} Prix invalide.");
		return 1;
	}
	if(strlen(desc) < 3)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Annonce]{FFFFFF} Usage: /annonce <prix> <description>");
		return 1;
	}
	new sqlbuf[150];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
		"SELECT id FROM lvrp_marketplace WHERE SQLid=%d AND Active=1 LIMIT 1", PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, sqlbuf, "Market_OnCheckExisting", "dds", playerid, price, desc);
	return 1;
}

forward Market_OnCheckExisting(playerid, price, const desc[]);
public Market_OnCheckExisting(playerid, price, const desc[])
{
	if(!IsPlayerConnected(playerid)) return 0;
	new count = 0;
	cache_get_row_count(count);
	if(count > 0)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Annonce]{FFFFFF} Vous avez deja une annonce active. Tapez {FFFF00}/retirerannonce{FFFFFF} d'abord.");
		return 1;
	}
	new descClean[MARKET_MAX_DESC];
	strmid(descClean, desc, 0, MARKET_MAX_DESC - 1, MARKET_MAX_DESC);

	new sqlbuf[250];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
		"INSERT INTO lvrp_marketplace (SQLid, SellerName, Price, Description, Date, Active) VALUES (%d, '%e', %d, '%e', UNIX_TIMESTAMP(), 1)",
		PlayerInfo[playerid][pSQLID], PlayerInfo[playerid][pRealName], price, descClean);
	mysql_tquery(MYSQL, sqlbuf);

	new msg[150];
	format(msg, sizeof(msg), "{00FF00}[Annonce]{FFFFFF} Postee : {FFFF00}%d $ {FFFFFF}- %s", price, descClean);
	SendClientMessage(playerid, -1, msg);
	return 1;
}

stock Market_CmdRemove(playerid)
{
	if(PlayerInfo[playerid][pSQLID] <= 0) return 0;
	new sqlbuf[150];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
		"UPDATE lvrp_marketplace SET Active=0 WHERE SQLid=%d AND Active=1", PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, sqlbuf, "Market_OnRemoveResult", "d", playerid);
	return 1;
}

forward Market_OnRemoveResult(playerid);
public Market_OnRemoveResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	SendClientMessage(playerid, -1, "{00FF00}[Annonce]{FFFFFF} Annonce retiree (si vous en aviez une active).");
	return 1;
}

stock Market_CmdList(playerid)
{
	mysql_tquery(MYSQL, "SELECT id, SellerName, Price, Description FROM lvrp_marketplace WHERE Active=1 ORDER BY id DESC LIMIT 25", "Market_OnListResult", "d", playerid);
	return 1;
}

// Stocke en memoire les ids correspondant a chaque ligne de la derniere liste affichee
new marketListIds[MAX_PLAYERS][25];
new marketListCount[MAX_PLAYERS];

forward Market_OnListResult(playerid);
public Market_OnListResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		SendClientMessage(playerid, -1, "{8C8C8C}[Annonces]{FFFFFF} Aucune annonce active pour le moment.");
		return 1;
	}
	new list[1500];
	list[0] = 0;
	marketListCount[playerid] = rows;
	new seller[32], desc[MARKET_MAX_DESC], itemPrice, id;
	for(new i = 0; i < rows; i++)
	{
		cache_get_value_name_int(i, "id", id);
		cache_get_value_name(i, "SellerName", seller, sizeof(seller));
		cache_get_value_name_int(i, "Price", itemPrice);
		cache_get_value_name(i, "Description", desc, sizeof(desc));
		marketListIds[playerid][i] = id;

		new line[160];
		format(line, sizeof(line), "{FFFF00}%d $ {FFFFFF}- %s {8C8C8C}(%s)\n", itemPrice, desc, seller);
		strcat(list, line, sizeof(list));
	}
	ShowPlayerDialog(playerid, DIALOG_MARKET_LIST, DIALOG_STYLE_LIST, "{FFAA00}>> Annonces actives <<{FFFFFF}", list, "Voir", "Fermer");
	return 1;
}

stock Market_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == DIALOG_MARKET_LIST)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= marketListCount[playerid]) return 1;
		new id = marketListIds[playerid][listitem];
		new sqlbuf[100];
		mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf), "SELECT SellerName, Price, Description FROM lvrp_marketplace WHERE id=%d AND Active=1 LIMIT 1", id);
		mysql_tquery(MYSQL, sqlbuf, "Market_OnItemResult", "d", playerid);
		return 1;
	}
	if(dialogid == DIALOG_MARKET_ITEM)
	{
		return 1; // juste un MSGBOX informatif, rien a faire au clic OK
	}
	return 0;
}

forward Market_OnItemResult(playerid);
public Market_OnItemResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Annonces]{FFFFFF} Cette annonce vient d'etre retiree.");
		return 1;
	}
	new seller[32], desc[MARKET_MAX_DESC], itemPrice;
	cache_get_value_name(0, "SellerName", seller, sizeof(seller));
	cache_get_value_name_int(0, "Price", itemPrice);
	cache_get_value_name(0, "Description", desc, sizeof(desc));

	new info[250];
	format(info, sizeof(info),
		"{FFFFFF}%s\n\n{8C8C8C}Prix:{FFFFFF} %d $\n{8C8C8C}Vendeur:{FFFFFF} %s\n\n{00CCFF}Contactez le vendeur via {FFFF00}/pm <id> <message>{00CCFF} pour negocier l'echange en RP.",
		desc, itemPrice, seller);
	ShowPlayerDialog(playerid, DIALOG_MARKET_ITEM, DIALOG_STYLE_MSGBOX, "{FFAA00}>> Detail de l'annonce <<", info, "OK", "");
	return 1;
}

// ==================== 3) MONITORING ADMIN ====================

stock Economy_CmdStats(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 2)
	{
		SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 2+.");
		return 1;
	}
	mysql_tquery(MYSQL,
		"SELECT COUNT(*) AS nb, SUM(Cash) AS totalCash, SUM(Bank) AS totalBank, AVG(Cash+Bank) AS avgWealth FROM lvrp_users",
		"Economy_OnStatsResult", "d", playerid);
	return 1;
}

forward Economy_OnStatsResult(playerid);
public Economy_OnStatsResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	if(rows == 0) return 0;

	new nb = 0, totalCash = 0, totalBank = 0;
	new Float:avgWealth = 0.0;
	cache_get_value_name_int(0, "nb", nb);
	cache_get_value_name_int(0, "totalCash", totalCash);
	cache_get_value_name_int(0, "totalBank", totalBank);
	cache_get_value_name_float(0, "avgWealth", avgWealth);

	new msg[300];
	format(msg, sizeof(msg),
		"{FFAA00}[Economie]{FFFFFF} %d comptes | Cash total: {00FF00}%d ${FFFFFF} | Banque totale: {00FF00}%d ${FFFFFF} | Total circulation: {00FF00}%d $",
		nb, totalCash, totalBank, totalCash + totalBank);
	SendClientMessage(playerid, -1, msg);
	format(msg, sizeof(msg), "{FFAA00}[Economie]{FFFFFF} Richesse moyenne par compte: {00FF00}%.0f $", avgWealth);
	SendClientMessage(playerid, -1, msg);

	mysql_tquery(MYSQL, "SELECT Name, (Cash+Bank) AS Total FROM lvrp_users ORDER BY Total DESC LIMIT 5", "Economy_OnTopResult", "d", playerid);
	return 1;
}

forward Economy_OnTopResult(playerid);
public Economy_OnTopResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	SendClientMessage(playerid, -1, "{FFAA00}[Economie]{FFFFFF} Top 5 des plus riches :");
	new name[32], total;
	for(new i = 0; i < rows; i++)
	{
		cache_get_value_name(i, "Name", name, sizeof(name));
		cache_get_value_name_int(i, "Total", total);
		new msg[120];
		format(msg, sizeof(msg), "{8C8C8C}%d.{FFFFFF} %s - {00FF00}%d $", i+1, name, total);
		SendClientMessage(playerid, -1, msg);
	}
	return 1;
}
