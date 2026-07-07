/*
	afrp_genmiracle.inc - Generation Miracle pour Police / FBI / Armee

	Commande : /generationmiracle (alias /gm /generation)

	Equipement lourd unique pour intervention musclee.
	Reserve aux factions legales (pMember 1-5 police, 11-13 armee).
	Doit etre en service (police_Duty == 1).
	Reserve au rang 5+ de la faction (pRank >= 5) pour eviter les abus des bas rangs.
	Cooldown 30 min par joueur.
	Broadcast a tous les membres faction online (transparence).

	Loadout :
	- Parachute (46) + Body armor + Heal 100%
	- Desert Eagle (24) + 50 munitions
	- MP5 (29) + 200 munitions
	- M4 (31) + 300 munitions
	- Sniper (34) + 50 munitions
	- RPG (35) + 5 roquettes
	- HS Rocket (36) + 2 missiles
	- Grenade (16) + 5 grenades
	- Cocktail molotov (18) + 5
*/

#if defined AFRP_GENMIRACLE_INCLUDED
	#endinput
#endif
#define AFRP_GENMIRACLE_INCLUDED

#define GENMIRACLE_COOLDOWN  1800 // 30 min
#define DIALOG_GENMIRACLE    9720

new genMiracleLastUse[MAX_PLAYERS];

stock bool:GenMiracle_CanUse(playerid)
{
	if(!IsPlayerConnected(playerid)) return false;
	// Faction legale only
	new f = PlayerInfo[playerid][pMember];
	new bool:isLegal = (f >= 1 && f <= 5) || (f >= 11 && f <= 13);
	if(!isLegal) return false;
	// Doit etre en service
	if(police_Duty[playerid] != 1) return false;
	return true;
}

stock bool:GenMiracle_CheckEligible(playerid)
{
	if(!IsPlayerConnected(playerid)) return false;
	new f = PlayerInfo[playerid][pMember];
	new bool:isPolice = (f >= 1 && f <= 5);
	new bool:isArmy   = (f >= 11 && f <= 13);
	new bool:isLegal  = isPolice || isArmy;
	new bool:isAdminOverride = (PlayerInfo[playerid][pAdmin] >= 2);

	if(!isLegal && !isAdminOverride)
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Generation Miracle]{FFFFFF} Reserve Police / FBI / Armee uniquement.");
		return false;
	}
	if(isPolice && police_Duty[playerid] != 1 && !isAdminOverride)
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Generation Miracle]{FFFFFF} Vous devez etre en service ({FFFF00}/duty{FFFFFF}).");
		return false;
	}
	// [FIX ABUS] Reserve au rang 5+ dans la faction (trop de bas rangs en abusaient).
	if(PlayerInfo[playerid][pRank] < 5 && !isAdminOverride)
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Generation Miracle]{FFFFFF} Reserve au rang 5+ de votre faction.");
		return false;
	}
	new now = gettime();
	if(now - genMiracleLastUse[playerid] < GENMIRACLE_COOLDOWN && !isAdminOverride)
	{
		new left = GENMIRACLE_COOLDOWN - (now - genMiracleLastUse[playerid]);
		new msg[140];
		format(msg, sizeof(msg),
			"{FF4444}[Generation Miracle]{FFFFFF} Cooldown actif. Reessayez dans {FFFF00}%dm %ds{FFFFFF}.",
			left/60, left%60);
		SendClientMessage(playerid, -1, msg);
		return false;
	}
	return true;
}

stock GenMiracle_Init()
{
	mysql_tquery(MYSQL,
		"CREATE TABLE IF NOT EXISTS `lvrp_log_genmiracle` (\
			`id` INT AUTO_INCREMENT PRIMARY KEY,\
			`SQLid` INT NOT NULL,\
			`Name` VARCHAR(32) NOT NULL DEFAULT '',\
			`Weapon` VARCHAR(24) NOT NULL DEFAULT '',\
			`Date` INT NOT NULL DEFAULT 0\
		) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "", "");
	return 1;
}

// Applique le loadout avec l'arme heavy choisie (35=RPG ou 36=HS Rocket/Bazooka)
stock GenMiracle_Apply(playerid, heavyWeapon)
{
	if(heavyWeapon != 35 && heavyWeapon != 36) heavyWeapon = 35;
	genMiracleLastUse[playerid] = gettime();

	// [ANTI-ABUS] Log persistant : qui, quand, quelle arme -> preuve en cas de recidive
	new logbuf[200];
	mysql_format(MYSQL, logbuf, sizeof(logbuf),
		"INSERT INTO `lvrp_log_genmiracle` (SQLid, Name, Weapon, Date) VALUES (%d, '%e', '%s', UNIX_TIMESTAMP())",
		PlayerInfo[playerid][pSQLID], PlayerInfo[playerid][pName], (heavyWeapon == 35) ? "RPG" : "Missile");
	mysql_tquery(MYSQL, logbuf);

	SetPlayerHealth(playerid, 100.0);
	SetPlayerArmour(playerid, 100.0);

	SafeGivePlayerWeapon(playerid, 46, 1);      // Parachute      (slot 0)
	SafeGivePlayerWeapon(playerid, 24, 50);     // Desert Eagle   (slot 2)
	SafeGivePlayerWeapon(playerid, 29, 200);    // MP5            (slot 4)
	SafeGivePlayerWeapon(playerid, 31, 300);    // M4             (slot 5)
	SafeGivePlayerWeapon(playerid, 34, 50);     // Sniper         (slot 6)
	SafeGivePlayerWeapon(playerid, heavyWeapon, 5); // RPG ou HS Rocket (slot 7)
	SafeGivePlayerWeapon(playerid, 16, 5);      // Grenades       (slot 8)
	SafeGivePlayerWeapon(playerid, 41, 1000);   // Spray bonus    (slot 9)

	PlayerPlaySound(playerid, 1133, 0.0, 0.0, 0.0);
	GameTextForPlayer(playerid, "~r~~h~GENERATION~n~~w~MIRACLE~n~~y~ACTIVE", 4000, 0);

	new heavyName[16];
	heavyName = (heavyWeapon == 35) ? "RPG (Bazooka)" : "Missile teleguide";
	SendClientMessage(playerid, 0xFFD700FF, "{FFD700}[Generation Miracle]{FFFFFF} Equipement lourd active. Bonne intervention.");
	new lmsg[160];
	format(lmsg, sizeof(lmsg),
		"{AAAAAA}>{FFFFFF} Parachute, Deagle, MP5, M4, Sniper, {FFAA00}%s{FFFFFF}, Grenades, Spray.",
		heavyName);
	SendClientMessage(playerid, 0xAAAAAAFF, lmsg);
	SendClientMessage(playerid, 0xAAAAAAFF, "{AAAAAA}>{FFFFFF} Cooldown 30 min avant prochaine utilisation.");

	// Broadcast aux membres de la faction (transparence)
	new f = PlayerInfo[playerid][pMember];
	new amsg[200];
	new factionName[24] = "Faction";
	if(f >= 1 && f <= 5)         factionName = "Police";
	else if(f >= 11 && f <= 13)  factionName = "Armee";
	format(amsg, sizeof(amsg),
		"{FFD700}[Gen Miracle]{FFFFFF} %s ({FFFF00}%s{FFFFFF}) -> equipement lourd + {FFAA00}%s{FFFFFF}.",
		PlayerInfo[playerid][pName], factionName, heavyName);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || i == playerid) continue;
		new fi = PlayerInfo[i][pMember];
		new bool:sameTeam = (fi >= 1 && fi <= 5) && (f >= 1 && f <= 5);
		new bool:sameArmy = (fi >= 11 && fi <= 13) && (f >= 11 && f <= 13);
		if(sameTeam || sameArmy || PlayerInfo[i][pAdmin] >= 1)
			SendClientMessage(i, -1, amsg);
	}
	return 1;
}

forward GenMiracle_OnLogsResult(playerid);
public GenMiracle_OnLogsResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new rows = 0;
	cache_get_row_count(rows);
	if(rows == 0)
	{
		SendClientMessage(playerid, -1, "{8C8C8C}[GM Logs]{FFFFFF} Aucune utilisation enregistree.");
		return 1;
	}
	new list[1200];
	list[0] = 0;
	new name[32], weapon[24], ts;
	for(new i = 0; i < rows; i++)
	{
		cache_get_value_name(i, "Name", name, sizeof(name));
		cache_get_value_name(i, "Weapon", weapon, sizeof(weapon));
		cache_get_value_name_int(i, "Date", ts);
		new line[100];
		format(line, sizeof(line), "{FFFFFF}%s {8C8C8C}- %s - %s\n", name, weapon, date(ts, 1));
		strcat(list, line, sizeof(list));
	}
	ShowPlayerDialog(playerid, 9721, DIALOG_STYLE_MSGBOX, "{FFAA00}>> Historique /gm (15 derniers) <<", list, "OK", "");
	return 1;
}

stock GenMiracle_Cmd(playerid)
{
	if(!GenMiracle_CheckEligible(playerid)) return 1;
	// Ouvre le dialog de choix d'arme heavy
	ShowPlayerDialog(playerid, DIALOG_GENMIRACLE, DIALOG_STYLE_LIST,
		"{FFAA00}>> Generation Miracle <<{FFFFFF}",
		"{FFFFFF}1. {FFAA00}RPG (Bazooka classique){FFFFFF}  {8C8C8C}- gros degats au sol\n{FFFFFF}2. {FFAA00}Missile teleguide{FFFFFF}  {8C8C8C}- locks-on vehicules/avions",
		"Choisir", "Annuler");
	return 1;
}

stock GenMiracle_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_GENMIRACLE) return 0;
	if(!response) return 1;
	// Re-check eligibilite au cas ou l'etat a change entre le clic /gm et le choix
	if(!GenMiracle_CheckEligible(playerid)) return 1;
	new heavyWeapon = (listitem == 0) ? 35 : 36;
	GenMiracle_Apply(playerid, heavyWeapon);
	return 1;
}

stock GenMiracle_OnDisconnect(playerid)
{
	genMiracleLastUse[playerid] = 0;
	return 1;
}
