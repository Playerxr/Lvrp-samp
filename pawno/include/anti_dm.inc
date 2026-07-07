/*
	anti_dm.inc - Detection auto des DM (deathmatch sans RP) + alerte admin

	Principe : a chaque tir, on verifie si le contexte autorise le combat.
	Si non -> alerte admin avec details (qui, qui, ou, arme, strike count).
	N'EMPECHE PAS les degats (pour pas casser le RP legitime), mais loggue
	pour modération a posteriori.

	Exemptions (DM autorise) :
	- Admin en service
	- Attaquant policier en service (police_Duty=1)
	- Attaquant en faction (police ou justice ou armee)
	- Meme faction (entrainement)
	- Attaquant DANS zone de gang (tType >= 3)
	- Self-attack ou no attacker
*/

#if defined ANTI_DM_INCLUDED
	#endinput
#endif
#define ANTI_DM_INCLUDED

#define ANTIDM_RESET_AFTER  1800    // 30 min sans incident -> reset strikes
#define ANTIDM_WARN_AT      3       // strikes avant warning au joueur
#define ANTIDM_ALERT_AT     5       // strikes avant alerte sonore admin

new dmStrikes[MAX_PLAYERS];
new dmLastTime[MAX_PLAYERS];

stock bool:AntiDM_IsAllowed(victim, attacker)
{
	if(attacker == INVALID_PLAYER_ID || attacker == victim) return true;
	if(attacker < 0 || attacker >= MAX_PLAYERS) return true;
	if(!IsPlayerConnected(attacker)) return true;
	if(IsPlayerNPC(attacker) || IsPlayerNPC(victim)) return true;

	// Admins en service : exempts
	if(AdminDuty[attacker] == 1 || AdminDuty[victim] == 1) return true;

	// Police ou armee en service : DM legitime
	if(police_Duty[attacker] == 1) return true;

	// Meme faction (entrainement / RP interne)
	new fAtt = PlayerInfo[attacker][pMember];
	new fVic = PlayerInfo[victim][pMember];
	if(fAtt > 0 && fAtt == fVic) return true;

	// Attaquant dans une zone de gang (type 3+ = gang/illegal turf)
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(attacker, px, py, pz);
	for(new ti = 0; ti < MAX_TURFS; ti++)
	{
		if(Turfs[ti][tUsed] != 1) continue;
		if(Turfs[ti][tType] < 3) continue; // 1,2 = safe (deja gere ailleurs)
		if(px >= Turfs[ti][tMinX] && px <= Turfs[ti][tMaxX] &&
		   py >= Turfs[ti][tMinY] && py <= Turfs[ti][tMaxY])
			return true;
	}

	return false;
}

stock AntiDM_Report(victim, attacker, weaponid)
{
	new now = gettime();
	if(now - dmLastTime[attacker] > ANTIDM_RESET_AFTER) dmStrikes[attacker] = 0;
	dmLastTime[attacker] = now;
	dmStrikes[attacker]++;

	new wname[32];
	GetWeaponName(weaponid, wname, sizeof(wname));
	if(strlen(wname) == 0) wname = "Inconnu";

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(attacker, px, py, pz);

	// Construit le message admin
	new msg[220];
	format(msg, sizeof(msg),
		"{FF6666}[Anti-DM]{FFFFFF} %s(%d) -> %s(%d) | %s | (%.0f,%.0f,%.0f) | Strike #%d",
		PlayerInfo[attacker][pName], attacker,
		PlayerInfo[victim][pName], victim,
		wname, px, py, pz, dmStrikes[attacker]);

	// Alerte tous les admins en ligne
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		if(PlayerInfo[i][pAdmin] >= 1)
		{
			SendClientMessage(i, 0xFF6666FF, msg);
			if(dmStrikes[attacker] >= ANTIDM_ALERT_AT)
				PlayerPlaySound(i, 1085, 0.0, 0.0, 0.0); // alerte sonore admins
		}
	}

	// Avertir l'attaquant a partir de ANTIDM_WARN_AT strikes
	if(dmStrikes[attacker] == ANTIDM_WARN_AT)
	{
		SendClientMessage(attacker, 0xFFAA00FF,
			"{FFAA00}[Anti-DM]{FFFFFF} Plusieurs combats hors zone detectes. Pense au RP, les admins sont prevenus.");
	}
	else if(dmStrikes[attacker] >= ANTIDM_ALERT_AT)
	{
		new wmsg[160];
		format(wmsg, sizeof(wmsg),
			"{FF0000}[Anti-DM]{FFFFFF} %d incidents en moins de %dmin. Un admin va probablement intervenir.",
			dmStrikes[attacker], ANTIDM_RESET_AFTER/60);
		SendClientMessage(attacker, -1, wmsg);
	}
}

// Reset des strikes sur deconnexion (pour que reconnection compte separement)
stock AntiDM_OnDisconnect(playerid)
{
	dmStrikes[playerid] = 0;
	dmLastTime[playerid] = 0;
	return 1;
}

// Commande admin pour voir les strikes d'un joueur
stock AntiDM_CmdInfo(adminid, targetid)
{
	if(PlayerInfo[adminid][pAdmin] < 1)
	{
		SendClientMessage(adminid, -1, "{FF4444}Pas la permission.");
		return 1;
	}
	if(!IsPlayerConnected(targetid))
	{
		SendClientMessage(adminid, -1, "{FF4444}Joueur invalide.");
		return 1;
	}
	new msg[140], now = gettime();
	new ago = (dmLastTime[targetid] > 0) ? (now - dmLastTime[targetid]) : 0;
	format(msg, sizeof(msg), "{FFAA00}[Anti-DM]{FFFFFF} %s : %d strikes (dernier il y a %ds)",
		PlayerInfo[targetid][pName], dmStrikes[targetid], ago);
	SendClientMessage(adminid, -1, msg);
	return 1;
}
