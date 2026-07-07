/*
	afrp_emergency.inc - /911, /sos, /feu : alerte d'urgence directe + auto-broadcast

	Independant du systeme telephone existant (numero 911/15 via /call), qui
	exige un telephone + credit + operateur. Ici : commande directe, gratuite,
	toujours disponible (utile si le joueur n'a pas de tel ou plus de credit).

	/911 -> alerte tous les policiers/FBI en service (police_Duty==1, pMember 1-5)
	        avec coords + checkpoint auto pose chez chaque destinataire.
	/sos -> alerte tous les medecins hopital en service (hopital_Duty==1)
	        avec coords + checkpoint auto.
	/feu -> alerte tous les pompiers en service (fire_Duty==1)
	        avec coords + checkpoint auto. Aucun equivalent direct n'existait
	        pour les pompiers avant (seul /call 912 payant existait).

	Cooldown 60s/joueur/type pour eviter le spam.
*/

#if defined AFRP_EMERGENCY_INCLUDED
	#endinput
#endif
#define AFRP_EMERGENCY_INCLUDED

#define EMERGENCY_COOLDOWN  60 // s

new emergency911Last[MAX_PLAYERS];
new emergencySOSLast[MAX_PLAYERS];
new emergencyFireLast[MAX_PLAYERS];

stock Emergency_Cmd911(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new now = gettime();
	if(now - emergency911Last[playerid] < EMERGENCY_COOLDOWN)
	{
		new left = EMERGENCY_COOLDOWN - (now - emergency911Last[playerid]);
		new msg[100];
		format(msg, sizeof(msg), "{FF4444}[911]{FFFFFF} Attends encore {FFFF00}%ds{FFFFFF} avant de rappeler.", left);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new responders = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		new f = PlayerInfo[i][pMember];
		new bool:isPolice = (f >= 1 && f <= 5);
		if(!isPolice || police_Duty[i] != 1) continue;

		new amsg[200];
		format(amsg, sizeof(amsg),
			"{FF0000}[911]{FFFFFF} %s a besoin d'aide urgente ! Position : {FFFF00}(%.0f, %.0f, %.0f){FFFFFF}. Checkpoint pose sur ta carte.",
			PlayerInfo[playerid][pName], px, py, pz);
		SendClientMessage(i, -1, amsg);
		PlayerPlaySound(i, 1058, 0.0, 0.0, 0.0);
		SetPlayerCheckpoint(i, px, py, pz, 5.0);
		responders++;
	}

	emergency911Last[playerid] = now;

	if(responders > 0)
	{
		new cmsg[140];
		format(cmsg, sizeof(cmsg),
			"{00FF00}[911]{FFFFFF} Alerte envoyee a {FFFF00}%d{FFFFFF} policier(s) en service.",
			responders);
		SendClientMessage(playerid, -1, cmsg);
	}
	else
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[911]{FFFFFF} Aucun policier en service actuellement. Reste prudent.");
	}
	return 1;
}

stock Emergency_CmdSOS(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new now = gettime();
	if(now - emergencySOSLast[playerid] < EMERGENCY_COOLDOWN)
	{
		new left = EMERGENCY_COOLDOWN - (now - emergencySOSLast[playerid]);
		new msg[100];
		format(msg, sizeof(msg), "{FF4444}[SOS]{FFFFFF} Attends encore {FFFF00}%ds{FFFFFF} avant de rappeler.", left);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new responders = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] != FACTION_HOPITAL || hopital_Duty[i] != 1) continue;

		new amsg[200];
		format(amsg, sizeof(amsg),
			"{FF8282}[SOS]{FFFFFF} %s a besoin de soins urgents ! Position : {FFFF00}(%.0f, %.0f, %.0f){FFFFFF}. Checkpoint pose sur ta carte.",
			PlayerInfo[playerid][pName], px, py, pz);
		SendClientMessage(i, -1, amsg);
		PlayerPlaySound(i, 1058, 0.0, 0.0, 0.0);
		SetPlayerCheckpoint(i, px, py, pz, 5.0);
		responders++;
	}

	emergencySOSLast[playerid] = now;

	if(responders > 0)
	{
		new cmsg[140];
		format(cmsg, sizeof(cmsg),
			"{00FF00}[SOS]{FFFFFF} Alerte envoyee a {FFFF00}%d{FFFFFF} medecin(s) en service.",
			responders);
		SendClientMessage(playerid, -1, cmsg);
	}
	else
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[SOS]{FFFFFF} Aucun medecin en service actuellement. Reste prudent.");
	}
	return 1;
}

stock Emergency_CmdFire(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new now = gettime();
	if(now - emergencyFireLast[playerid] < EMERGENCY_COOLDOWN)
	{
		new left = EMERGENCY_COOLDOWN - (now - emergencyFireLast[playerid]);
		new msg[100];
		format(msg, sizeof(msg), "{FF4444}[Pompiers]{FFFFFF} Attends encore {FFFF00}%ds{FFFFFF} avant de rappeler.", left);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new responders = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(fire_Duty[i] != 1) continue;

		new amsg[200];
		format(amsg, sizeof(amsg),
			"{FF8000}[Pompiers]{FFFFFF} %s signale un incendie/urgence ! Position : {FFFF00}(%.0f, %.0f, %.0f){FFFFFF}. Checkpoint pose sur ta carte.",
			PlayerInfo[playerid][pName], px, py, pz);
		SendClientMessage(i, -1, amsg);
		PlayerPlaySound(i, 1058, 0.0, 0.0, 0.0);
		SetPlayerCheckpoint(i, px, py, pz, 5.0);
		responders++;
	}

	emergencyFireLast[playerid] = now;

	if(responders > 0)
	{
		new cmsg[140];
		format(cmsg, sizeof(cmsg),
			"{00FF00}[Pompiers]{FFFFFF} Alerte envoyee a {FFFF00}%d{FFFFFF} pompier(s) en service.",
			responders);
		SendClientMessage(playerid, -1, cmsg);
	}
	else
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Pompiers]{FFFFFF} Aucun pompier en service actuellement.");
	}
	return 1;
}

stock Emergency_OnDisconnect(playerid)
{
	emergency911Last[playerid] = 0;
	emergencySOSLast[playerid] = 0;
	emergencyFireLast[playerid] = 0;
	return 1;
}
