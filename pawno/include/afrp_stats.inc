/*
	afrp_stats.inc - /stats enrichi avec compteurs session

	Compteurs maintenus en memoire (par session) :
	- Playtime : minutes depuis le login
	- Kills session
	- Deaths session
	- /me count + /do count -> RP score
	- Achievements debloques (basiques)

	Affichage : dialog MSGBOX joliment formate.

	Hooks :
	- OnPlayerConnect    -> reset compteurs
	- OnPlayerSpawn      -> set sessionStart timestamp
	- OnPlayerDeath      -> +1 deaths (et +1 kills pour killer)
	- OnPlayerText       -> detecter /me /do et +1 rpScore
	- Commande /mystats  -> show dialog
*/

#if defined AFRP_STATS_INCLUDED
	#endinput
#endif
#define AFRP_STATS_INCLUDED

#define DIALOG_MYSTATS  9710

new statsSessionStart[MAX_PLAYERS];  // gettime() au login
new statsKillsSession[MAX_PLAYERS];
new statsDeathsSession[MAX_PLAYERS];
new statsRpScore[MAX_PLAYERS];       // /me + /do utilises

stock Stats_OnConnect(playerid)
{
	statsSessionStart[playerid] = gettime();
	statsKillsSession[playerid] = 0;
	statsDeathsSession[playerid] = 0;
	statsRpScore[playerid] = 0;
	return 1;
}

stock Stats_OnDeath(victim, killer)
{
	if(victim >= 0 && victim < MAX_PLAYERS) statsDeathsSession[victim]++;
	if(killer != INVALID_PLAYER_ID && killer >= 0 && killer < MAX_PLAYERS && killer != victim)
		statsKillsSession[killer]++;
	return 1;
}

// Appel depuis OnPlayerText quand un /me ou /do est detecte
stock Stats_OnRPCommand(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	statsRpScore[playerid]++;
	return 1;
}

// Achievements basiques selon les compteurs
stock Stats_GetAchievements(playerid, buf[], size)
{
	buf[0] = 0;
	new count = 0;
	if(statsRpScore[playerid] >= 5)   { strcat(buf, "{00FF00}- Bavard RP (5+ /me-/do)\n", size); count++; }
	if(statsRpScore[playerid] >= 20)  { strcat(buf, "{00FF00}- RPer experimente (20+)\n", size); count++; }
	if(statsRpScore[playerid] >= 50)  { strcat(buf, "{00FF00}- Veteran RP (50+)\n", size); count++; }
	if(statsKillsSession[playerid] >= 3)  { strcat(buf, "{FF6666}- Killer (3+ kills session)\n", size); count++; }
	if(statsKillsSession[playerid] >= 10) { strcat(buf, "{FF6666}- Boucher (10+ kills session)\n", size); count++; }
	new playtime = (gettime() - statsSessionStart[playerid]) / 60;
	if(playtime >= 60)   { strcat(buf, "{FFD700}- Dedie (1h+ session)\n", size); count++; }
	if(playtime >= 180)  { strcat(buf, "{FFD700}- Marathonien (3h+ session)\n", size); count++; }
	if(count == 0) strcat(buf, "{8C8C8C}Aucun achievement debloque cette session.", size);
	return count;
}

stock Stats_CmdMyStats(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new playtime = (gettime() - statsSessionStart[playerid]) / 60; // minutes
	new totalDeaths = PlayerInfo[playerid][pDeaths];
	new ratio[12];
	if(statsDeathsSession[playerid] > 0)
		format(ratio, sizeof(ratio), "%.2f", float(statsKillsSession[playerid]) / float(statsDeathsSession[playerid]));
	else
		ratio = (statsKillsSession[playerid] > 0) ? "Perfect" : "N/A";

	new achievements[400];
	Stats_GetAchievements(playerid, achievements, sizeof(achievements));

	new info[1200];
	format(info, sizeof(info),
		"{FFFFFF}== Stats de %s ==\n\n{8C8C8C}Niveau:{FFFFFF} %d\n{8C8C8C}Argent:{00FF00} %d ${FFFFFF}\n{8C8C8C}Banque:{00FF00} %d ${FFFFFF}\n\n{FFAA00}== Session courante ==\n{8C8C8C}Playtime:{FFFFFF} %d min\n{8C8C8C}Kills:{FF6666} %d{FFFFFF} / {8C8C8C}Deaths:{FF6666} %d{FFFFFF} (K/D: %s)\n{8C8C8C}RP Score:{00CCFF} %d{FFFFFF} (/me + /do)\n\n{FFAA00}== Total compte ==\n{8C8C8C}Deaths total:{FFFFFF} %d\n\n{FFAA00}== Achievements ==\n%s",
		PlayerInfo[playerid][pName],
		PlayerInfo[playerid][pLevel],
		GetPlayerMoney(playerid),
		PlayerInfo[playerid][pAccount],
		playtime,
		statsKillsSession[playerid],
		statsDeathsSession[playerid],
		ratio,
		statsRpScore[playerid],
		totalDeaths,
		achievements);

	ShowPlayerDialog(playerid, DIALOG_MYSTATS, DIALOG_STYLE_MSGBOX,
		"{FFAA00}>> Mes stats <<", info, "OK", "");
	return 1;
}
