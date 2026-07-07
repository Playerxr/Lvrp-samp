/*
	afrp_cuffs.inc - Auto-follow menottes + auto-jail combat-log

	1) AUTO-FOLLOW : un joueur menotte (player_Cuffed[playerid]==1) suit
	   automatiquement le policier le plus proche qui l'a (ou pas) menotte.
	   Timer 2s : si distance > 4m, teleporte le menotte derriere le cop.
	   Plus besoin de taper /p suivre.

	2) AUTO-JAIL COMBAT-LOG : si un joueur deco en etant menotte, on flag
	   son compte dans lvrp_users (JailTime=1800, Jailed=1) -> a la
	   reconnexion, il spawn directement en prison 30 min.

	Pre-requis : LVRP doit avoir 'new player_Cuffed[MAX_PLAYERS]' et les
	colonnes JailTime / Jailed dans lvrp_users (cas LVRP).
*/

#if defined AFRP_CUFFS_INCLUDED
	#endinput
#endif
#define AFRP_CUFFS_INCLUDED

#define CUFF_FOLLOW_DISTANCE   4.0    // m : declenche teleport si plus loin
#define CUFF_FOLLOW_OFFSET     1.5    // m : derriere le cop
#define CUFF_FOLLOW_INTERVAL   2000   // ms

#define CUFF_AUTOJAIL_TIME     1800   // 30 min en secondes (pJailTime stocke en sec)

// Garde l'id du cop qui suit (ou -1) - rafraichi automatiquement par proximite
new cuffedFollowingCop[MAX_PLAYERS];

// Trouve le cop le plus proche du joueur menotte (en duty, faction legale)
stock Cuff_FindNearestCop(playerid)
{
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	new bestId = -1;
	new Float:bestDist = 50.0; // limite recherche 50m
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		new f = PlayerInfo[i][pMember];
		// Police 1-5, FBI 4, armee 11-13 (toutes factions arrestation)
		new bool:isLegal = (f >= 1 && f <= 5) || (f >= 11 && f <= 13);
		if(!isLegal) continue;
		if(police_Duty[i] != 1) continue;
		if(GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(playerid)) continue;
		if(GetPlayerInterior(i) != GetPlayerInterior(playerid)) continue;
		new Float:cx, Float:cy, Float:cz;
		GetPlayerPos(i, cx, cy, cz);
		new Float:d = floatsqroot((cx-px)*(cx-px) + (cy-py)*(cy-py) + (cz-pz)*(cz-pz));
		if(d < bestDist)
		{
			bestDist = d;
			bestId = i;
		}
	}
	return bestId;
}

forward Cuff_FollowTick();
public Cuff_FollowTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(player_Cuffed[i] != 1) { cuffedFollowingCop[i] = -1; continue; }
		// Pas en vehicule (sinon le menotte est deja dans la voiture du cop)
		if(IsPlayerInAnyVehicle(i)) continue;

		new cop = cuffedFollowingCop[i];
		// Verifie que le cop "suivi" est toujours valide ; sinon repick le plus proche
		if(cop == -1 || !IsPlayerConnected(cop) || player_Cuffed[cop] == 1
			|| (PlayerInfo[cop][pMember] < 1 || PlayerInfo[cop][pMember] > 13)
			|| (PlayerInfo[cop][pMember] >= 6 && PlayerInfo[cop][pMember] <= 10))
		{
			cop = Cuff_FindNearestCop(i);
			cuffedFollowingCop[i] = cop;
			if(cop == -1) continue; // pas de cop a proximite
		}

		// Calcule distance
		new Float:px, Float:py, Float:pz, Float:cx, Float:cy, Float:cz, Float:ca;
		GetPlayerPos(i, px, py, pz);
		GetPlayerPos(cop, cx, cy, cz);
		GetPlayerFacingAngle(cop, ca);
		new Float:d = floatsqroot((cx-px)*(cx-px) + (cy-py)*(cy-py) + (cz-pz)*(cz-pz));

		// Si trop loin, teleport derriere le cop
		if(d > CUFF_FOLLOW_DISTANCE)
		{
			// Position derriere le cop : cop.pos - offset * direction_facing
			new Float:rx = cx - CUFF_FOLLOW_OFFSET * floatsin(-ca, degrees);
			new Float:ry = cy - CUFF_FOLLOW_OFFSET * floatcos(-ca, degrees);
			SetPlayerPos(i, rx, ry, cz);
			SetPlayerFacingAngle(i, ca);
			// Re-applique l'anim cuffed (perdue apres teleport)
			SetPlayerSpecialAction(i, SPECIAL_ACTION_CUFFED);
		}
	}
	return 1;
}

stock Cuff_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++) cuffedFollowingCop[i] = -1;
	SetTimer("Cuff_FollowTick", CUFF_FOLLOW_INTERVAL, true);
	return 1;
}

stock Cuff_OnConnect(playerid)
{
	cuffedFollowingCop[playerid] = -1;
	return 1;
}

// Appele au disconnect : si le joueur etait menotte, flag son compte
// pour qu'il spawn en prison a la reconnexion (anti combat-log).
stock Cuff_OnDisconnect(playerid)
{
	cuffedFollowingCop[playerid] = -1;
	if(player_Cuffed[playerid] != 1) return 0;
	if(PlayerInfo[playerid][pSQLID] <= 0) return 0;

	// Set en MEMOIRE -> ainsi quand LVRP save juste apres, il persiste ces valeurs.
	PlayerInfo[playerid][pJailTime] = CUFF_AUTOJAIL_TIME;
	PlayerInfo[playerid][pJailed]   = 1;

	// Aussi UPDATE direct au cas ou (force la persistance meme si save LVRP rate)
	new buf[200];
	format(buf, sizeof(buf),
		"UPDATE lvrp_users SET JailTime=%d, Jailed=1 WHERE id=%d",
		CUFF_AUTOJAIL_TIME, PlayerInfo[playerid][pSQLID]);
	mysql_pquery(MYSQL, buf);

	// Notifier les forces de l'ordre online
	new amsg[180];
	format(amsg, sizeof(amsg),
		"{FF6666}[Anti-CombatLog]{FFFFFF} %s s'est deco menotte -> auto-jail 30min a la reconnexion.",
		PlayerInfo[playerid][pName]);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		new f = PlayerInfo[i][pMember];
		new bool:isLegal = (f >= 1 && f <= 5) || (f >= 11 && f <= 13);
		if(isLegal || PlayerInfo[i][pAdmin] >= 1)
			SendClientMessage(i, -1, amsg);
	}
	return 1;
}
