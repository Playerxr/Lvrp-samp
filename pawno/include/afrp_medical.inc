/*
	afrp_medical.inc - Renforcement du systeme medical / EMS

	1) ALERTE AUTO : quand un joueur tombe a terre (coma), TOUS les medecins de
	   l'hopital en service (hopital_Duty==1) recoivent automatiquement un signal
	   d'urgence : nom de la victime, position, checkpoint pose + son d'alerte.
	   La victime est informee que les secours sont prevenus.

	2) PRIORITE EMS : /urgences (medecins en service) liste tous les blesses en
	   coma avec leur position, et pose un GPS vers le plus proche.

	3) APPEL HOPITAL : /hopital (alias /ambulance /samu) permet a n'importe qui
	   d'appeler l'hopital, comme /911 pour la police. Alerte les medecins avec
	   position + checkpoint. Cooldown anti-spam.

	FACTION_HOPITAL = 15 (defini dans LVRP.pwn).
*/

#if defined AFRP_MEDICAL_INCLUDED
	#endinput
#endif
#define AFRP_MEDICAL_INCLUDED

#define MEDICAL_CALL_COOLDOWN  45 // s entre 2 appels /hopital par joueur

new medicalCallLast[MAX_PLAYERS];

stock Medical_OnConnect(playerid)
{
	medicalCallLast[playerid] = 0;
	return 1;
}

stock Medical_CountMedicsOnDuty()
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] == FACTION_HOPITAL && hopital_Duty[i] == 1) n++;
	}
	return n;
}

// Appele depuis OnPlayerDeath quand la victime entre en coma.
stock Medical_OnComa(playerid)
{
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new medics = 0;
	new amsg[200];
	format(amsg, sizeof(amsg),
		"{FF1493}[URGENCE MEDICALE]{FFFFFF} %s est a terre et a besoin de soins ! Position : {FFFF00}(%.0f, %.0f){FFFFFF}. Checkpoint pose.",
		PlayerInfo[playerid][pRealName], px, py);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] != FACTION_HOPITAL || hopital_Duty[i] != 1) continue;
		SendClientMessage(i, -1, amsg);
		PlayerPlaySound(i, 1058, 0.0, 0.0, 0.0);
		SetPlayerCheckpoint(i, px, py, pz, 5.0);
		medics++;
	}

	if(medics > 0)
		SendClientMessage(playerid, 0xFF8282FF, "{FF8282}[Sante]{FFFFFF} Les secours ont ete prevenus automatiquement. Un medecin arrive, patientez.");
	else
		SendClientMessage(playerid, 0xFF8282FF, "{FF8282}[Sante]{FFFFFF} Aucun medecin en service. Attendez la fin du coma ou appelez a l'aide ({FFFF00}/ambulance{FFFFFF}).");
	return 1;
}

// /urgences : reserve aux medecins en service. Liste les blesses en coma + GPS.
stock Medical_CmdUrgences(playerid)
{
	if(PlayerInfo[playerid][pMember] != FACTION_HOPITAL)
		{SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Urgences]{FFFFFF} Reserve au personnel de l'hopital."); return 1;}
	if(hopital_Duty[playerid] != 1)
		{SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Urgences]{FFFFFF} Vous devez etre en service (duty)."); return 1;}

	new Float:mx, Float:my, Float:mz;
	GetPlayerPos(playerid, mx, my, mz);

	new list[900];
	list[0] = 0;
	new count = 0;
	new Float:bestDist = 999999.0, Float:bx, Float:by, Float:bz;
	new bool:haveTarget = false;

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(medic_PlayerNeedMedic[i] != 2) continue; // pas en coma
		new Float:vx, Float:vy, Float:vz;
		GetPlayerPos(i, vx, vy, vz);
		new Float:d = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);
		new line[128];
		format(line, sizeof(line), "{FFFFFF}%s {8C8C8C}- {FFFF00}%.0fm {8C8C8C}(coma %d:%02d)\n",
			PlayerInfo[i][pRealName], d, medic_PlayerMedicTime[i]/60, medic_PlayerMedicTime[i]%60);
		strcat(list, line, sizeof(list));
		count++;
		if(d < bestDist) { bestDist = d; bx = vx; by = vy; bz = vz; haveTarget = true; }
	}

	if(count == 0)
	{
		SendClientMessage(playerid, 0x82FF82FF, "{82FF82}[Urgences]{FFFFFF} Aucun blesse en coma actuellement. RAS.");
		return 1;
	}

	// GPS vers le plus proche
	if(haveTarget) SetPlayerCheckpoint(playerid, bx, by, bz, 5.0);

	new head[64];
	format(head, sizeof(head), "{FF1493}>> %d urgence(s) en cours <<", count);
	ShowPlayerDialog(playerid, 9780, DIALOG_STYLE_MSGBOX, head, list, "OK", "");
	SendClientMessage(playerid, 0xFF1493FF, "{FF1493}[Urgences]{FFFFFF} GPS pose vers le blesse le plus proche.");
	return 1;
}

// /hopital : appel d'urgence medical par n'importe quel joueur (comme /911).
stock Medical_CmdCallHospital(playerid)
{
	new now = gettime();
	if(now - medicalCallLast[playerid] < MEDICAL_CALL_COOLDOWN)
	{
		new left = MEDICAL_CALL_COOLDOWN - (now - medicalCallLast[playerid]);
		new m[110];
		format(m, sizeof(m), "{FF4444}[Hopital]{FFFFFF} Attends encore {FFFF00}%ds{FFFFFF} avant de rappeler.", left);
		SendClientMessage(playerid, -1, m);
		return 1;
	}

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new amsg[200];
	format(amsg, sizeof(amsg),
		"{FF1493}[Appel Hopital]{FFFFFF} %s demande une ambulance ! Position : {FFFF00}(%.0f, %.0f){FFFFFF}. Checkpoint pose.",
		PlayerInfo[playerid][pRealName], px, py);

	new medics = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] != FACTION_HOPITAL || hopital_Duty[i] != 1) continue;
		SendClientMessage(i, -1, amsg);
		PlayerPlaySound(i, 1058, 0.0, 0.0, 0.0);
		SetPlayerCheckpoint(i, px, py, pz, 5.0);
		medics++;
	}

	medicalCallLast[playerid] = now;

	if(medics > 0)
	{
		new c[120];
		format(c, sizeof(c), "{00FF00}[Hopital]{FFFFFF} Appel transmis a {FFFF00}%d{FFFFFF} medecin(s) en service.", medics);
		SendClientMessage(playerid, -1, c);
	}
	else
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Hopital]{FFFFFF} Aucun medecin en service. Rendez-vous directement a l'hopital le plus proche.");
	}
	return 1;
}
