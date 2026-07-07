/*
	afrp_bounty.inc - Prime / chasseur de tetes

	Quand le niveau de recherche RP (pWanted) d'un joueur depasse un seuil,
	une prime est automatiquement placee sur sa tete. N'IMPORTE QUEL joueur
	(pas que la police) peut toucher la recompense en le tuant.

	- Check periodique (toutes les 60s) : pWanted >= BOUNTY_THRESHOLD sans
	  prime active -> en cree une, montant scale avec le wanted level.
	- /bounties : liste des primes actives.
	- Reclamee automatiquement via OnPlayerDeath (le killer touche le cash).
	- Log persistant (lvrp_log_bounty) pour transparence/anti-triche.
*/

#if defined AFRP_BOUNTY_INCLUDED
	#endinput
#endif
#define AFRP_BOUNTY_INCLUDED

#define BOUNTY_THRESHOLD     4      // pWanted minimum pour declencher une prime
#define BOUNTY_PER_STAR      4000   // $ par etoile de recherche
#define BOUNTY_CHECK_INTERVAL 60000 // 60s

new playerBounty[MAX_PLAYERS]; // 0 = pas de prime active, >0 = montant

stock Bounty_Init()
{
	mysql_tquery(MYSQL,
		"CREATE TABLE IF NOT EXISTS `lvrp_log_bounty` (\
			`id` INT AUTO_INCREMENT PRIMARY KEY,\
			`TargetName` VARCHAR(32) NOT NULL DEFAULT '',\
			`HunterName` VARCHAR(32) NOT NULL DEFAULT '',\
			`Amount` INT NOT NULL DEFAULT 0,\
			`Date` INT NOT NULL DEFAULT 0\
		) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "", "");
	SetTimer("Bounty_PeriodicCheck", BOUNTY_CHECK_INTERVAL, true);
	return 1;
}

forward Bounty_PeriodicCheck();
public Bounty_PeriodicCheck()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(playerBounty[i] > 0) continue; // deja une prime active
		if(PlayerInfo[i][pWanted] < BOUNTY_THRESHOLD) continue;

		new amount = PlayerInfo[i][pWanted] * BOUNTY_PER_STAR;
		playerBounty[i] = amount;

		new msg[200];
		format(msg, sizeof(msg),
			"{FF6666}[PRIME]{FFFFFF} Une prime de {FFD700}%d $ {FFFFFF}est placee sur la tete de {FFFF00}%s{FFFFFF} ! Premier arrive, premier servi.",
			amount, PlayerInfo[i][pRealName]);
		for(new p = 0; p < MAX_PLAYERS; p++)
		{
			if(IsPlayerConnected(p) && !IsPlayerNPC(p))
				SendClientMessage(p, -1, msg);
		}
		PlayerPlaySound(i, 1057, 0.0, 0.0, 0.0);
	}
	return 1;
}

// Appele depuis OnPlayerDeath : si la victime avait une prime, le killer la touche
stock Bounty_OnPlayerDeath(victim, killer)
{
	if(victim < 0 || victim >= MAX_PLAYERS) return 0;
	if(playerBounty[victim] <= 0) return 0;
	new amount = playerBounty[victim];
	playerBounty[victim] = 0;

	if(killer == INVALID_PLAYER_ID || killer < 0 || killer >= MAX_PLAYERS || killer == victim || !IsPlayerConnected(killer))
	{
		// Mort sans tueur valide (suicide, chute, etc.) -> prime perdue, juste annoncee
		new msg[180];
		format(msg, sizeof(msg),
			"{8C8C8C}[PRIME]{FFFFFF} %s est mort sans etre capture, la prime de {FFD700}%d ${FFFFFF} est annulee.",
			PlayerInfo[victim][pRealName], amount);
		for(new p = 0; p < MAX_PLAYERS; p++)
		{
			if(IsPlayerConnected(p) && !IsPlayerNPC(p)) SendClientMessage(p, -1, msg);
		}
		return 0;
	}

	GivePlayerMoney(killer, amount);
	PlayerInfo[killer][pCash] += amount;

	new msg[220];
	format(msg, sizeof(msg),
		"{00FF00}[PRIME]{FFFFFF} %s a touche la prime sur %s et empoche {FFD700}%d $ !",
		PlayerInfo[killer][pRealName], PlayerInfo[victim][pRealName], amount);
	for(new p = 0; p < MAX_PLAYERS; p++)
	{
		if(IsPlayerConnected(p) && !IsPlayerNPC(p)) SendClientMessage(p, -1, msg);
	}
	PlayerPlaySound(killer, 1150, 0.0, 0.0, 0.0);

	new sqlbuf[200];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
		"INSERT INTO lvrp_log_bounty (TargetName, HunterName, Amount, Date) VALUES ('%e', '%e', %d, UNIX_TIMESTAMP())",
		PlayerInfo[victim][pRealName], PlayerInfo[killer][pRealName], amount);
	mysql_tquery(MYSQL, sqlbuf);
	return 1;
}

stock Bounty_CmdList(playerid)
{
	new list[600];
	list[0] = 0;
	new count = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(playerBounty[i] <= 0) continue;
		new line[100];
		format(line, sizeof(line), "{FFFFFF}%s {8C8C8C}- {FFD700}%d $\n", PlayerInfo[i][pRealName], playerBounty[i]);
		strcat(list, line, sizeof(list));
		count++;
	}
	if(count == 0)
	{
		SendClientMessage(playerid, -1, "{8C8C8C}[PRIME]{FFFFFF} Aucune prime active pour le moment.");
		return 1;
	}
	ShowPlayerDialog(playerid, 9760, DIALOG_STYLE_MSGBOX, "{FFAA00}>> Primes actives <<{FFFFFF}", list, "OK", "");
	return 1;
}

stock Bounty_OnDisconnect(playerid)
{
	// Si le joueur deco avec une prime active, elle reste active (il pourra
	// etre repris a sa reconnexion si toujours assez wanted, sinon expirera
	// naturellement au prochain Bounty_PeriodicCheck via le reset wanted).
	#pragma unused playerid
	return 1;
}
