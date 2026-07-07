/*
	afrp_modtools.inc - Outils de moderation auto

	1) Anti-spam chat :
	   - Detecte 3+ messages identiques en 10s OU 5+ messages en 5s
	   - Mute auto 60s + alerte admins
	   - Reset au disconnect

	2) /calm <id> [secondes] :
	   - Admin freeze un joueur N secondes (default 30s)
	   - Effet visuel : countdown TextDraw + son
	   - Pour de-escalation de conflits RP

	3) /unfreeze <id> :
	   - Admin libere prematurement un calm

	A inclure APRES PlayerInfo, AdminDuty, AntiDM, etc.
*/

#if defined AFRP_MODTOOLS_INCLUDED
	#endinput
#endif
#define AFRP_MODTOOLS_INCLUDED

// ==================== ANTI-SPAM ====================

#define ANTISPAM_REPEAT_MAX     3       // 3 messages identiques = spam
#define ANTISPAM_REPEAT_WINDOW  10      // dans une fenetre de 10s
#define ANTISPAM_FLOOD_MAX      5       // 5 messages au total
#define ANTISPAM_FLOOD_WINDOW   5       // dans 5s = flood
#define ANTISPAM_MUTE_DURATION  60      // mute 60s
#define ANTISPAM_HISTORY_SIZE   5       // garde 5 derniers messages

new spamHistory[MAX_PLAYERS][ANTISPAM_HISTORY_SIZE][128];
new spamTimes[MAX_PLAYERS][ANTISPAM_HISTORY_SIZE];
new spamIndex[MAX_PLAYERS];
new spamMutedUntil[MAX_PLAYERS]; // 0 = pas mute, sinon timestamp fin

stock bool:AntiSpam_Check(playerid, const text[])
{
	if(!IsPlayerConnected(playerid)) return false;
	// Admin en duty : exempt
	if(AdminDuty[playerid] == 1) return false;

	new now = gettime();

	// Deja mute ?
	if(spamMutedUntil[playerid] > now)
	{
		new msg[120], left = spamMutedUntil[playerid] - now;
		format(msg, sizeof(msg), "{FF4444}[Anti-Spam]{FFFFFF} Vous etes mute encore {FFFF00}%ds{FFFFFF}.", left);
		SendClientMessage(playerid, -1, msg);
		return true; // bloque le chat
	}

	// Insert message dans l'historique circulaire
	new idx = spamIndex[playerid] % ANTISPAM_HISTORY_SIZE;
	strmid(spamHistory[playerid][idx], text, 0, strlen(text), 128);
	spamTimes[playerid][idx] = now;
	spamIndex[playerid]++;

	// 1) Compte les messages identiques dans la fenetre
	new repeats = 0;
	for(new i = 0; i < ANTISPAM_HISTORY_SIZE; i++)
	{
		if(spamTimes[playerid][i] == 0) continue;
		if(now - spamTimes[playerid][i] > ANTISPAM_REPEAT_WINDOW) continue;
		if(strcmp(spamHistory[playerid][i], text, false) == 0) repeats++;
	}

	// 2) Compte les messages totaux dans la fenetre flood
	new total = 0;
	for(new i = 0; i < ANTISPAM_HISTORY_SIZE; i++)
	{
		if(spamTimes[playerid][i] == 0) continue;
		if(now - spamTimes[playerid][i] <= ANTISPAM_FLOOD_WINDOW) total++;
	}

	if(repeats >= ANTISPAM_REPEAT_MAX || total >= ANTISPAM_FLOOD_MAX)
	{
		spamMutedUntil[playerid] = now + ANTISPAM_MUTE_DURATION;
		new msg[140];
		format(msg, sizeof(msg), "{FF4444}[Anti-Spam]{FFFFFF} Vous etes mute {FFFF00}%ds{FFFFFF} pour spam (%s).",
			ANTISPAM_MUTE_DURATION,
			(repeats >= ANTISPAM_REPEAT_MAX) ? "messages identiques" : "flood");
		SendClientMessage(playerid, -1, msg);

		// Alerte admins
		new amsg[180];
		format(amsg, sizeof(amsg), "{FFAA00}[Anti-Spam]{FFFFFF} %s(%d) mute %ds (%d repeats, %d msgs/%ds).",
			PlayerInfo[playerid][pName], playerid, ANTISPAM_MUTE_DURATION,
			repeats, total, ANTISPAM_FLOOD_WINDOW);
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i)) continue;
			if(PlayerInfo[i][pAdmin] >= 1)
				SendClientMessage(i, 0xFFAA00FF, amsg);
		}
		return true;
	}
	return false;
}

stock AntiSpam_OnDisconnect(playerid)
{
	spamIndex[playerid] = 0;
	spamMutedUntil[playerid] = 0;
	for(new i = 0; i < ANTISPAM_HISTORY_SIZE; i++)
	{
		spamHistory[playerid][i][0] = 0;
		spamTimes[playerid][i] = 0;
	}
	return 1;
}

// Admin : unmute force
stock AntiSpam_Unmute(playerid)
{
	spamMutedUntil[playerid] = 0;
	return 1;
}

// ==================== /CALM (admin freeze RP) ====================

new calmUntil[MAX_PLAYERS];        // 0 = libre, sinon timestamp fin
new PlayerText:calmTD[MAX_PLAYERS];

forward Calm_Tick(playerid);
public Calm_Tick(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(calmUntil[playerid] == 0) return 0;
	new now = gettime();
	if(now >= calmUntil[playerid])
	{
		Calm_End(playerid);
		return 0;
	}
	// Update countdown TD
	new left = calmUntil[playerid] - now;
	new buf[64];
	format(buf, sizeof(buf), "~r~Calme~n~~w~%ds", left);
	if(calmTD[playerid] != PlayerText:INVALID_TEXT_DRAW)
		PlayerTextDrawSetString(playerid, calmTD[playerid], buf);
	SetTimerEx("Calm_Tick", 1000, false, "i", playerid);
	return 1;
}

stock Calm_Start(adminid, targetid, duration)
{
	if(PlayerInfo[adminid][pAdmin] < 2)
		return SendClientMessage(adminid, -1, "{FF4444}Reserve admin niveau 2+.");
	if(!IsPlayerConnected(targetid))
		return SendClientMessage(adminid, -1, "{FF4444}Joueur invalide.");
	if(duration < 5) duration = 30;
	if(duration > 300) duration = 300;
	if(calmUntil[targetid] != 0)
		return SendClientMessage(adminid, -1, "{FF4444}Deja calm. /uncalm pour liberer.");

	calmUntil[targetid] = gettime() + duration;
	TogglePlayerControllable(targetid, false);
	PlayerPlaySound(targetid, 1057, 0.0, 0.0, 0.0);

	// Cree countdown TD
	new buf[64];
	format(buf, sizeof(buf), "~r~Calme~n~~w~%ds", duration);
	calmTD[targetid] = CreatePlayerTextDraw(targetid, 320.0, 200.0, buf);
	PlayerTextDrawFont(targetid, calmTD[targetid], 2);
	PlayerTextDrawLetterSize(targetid, calmTD[targetid], 0.6, 2.4);
	PlayerTextDrawColor(targetid, calmTD[targetid], -1);
	PlayerTextDrawSetShadow(targetid, calmTD[targetid], 2);
	PlayerTextDrawSetOutline(targetid, calmTD[targetid], 1);
	PlayerTextDrawAlignment(targetid, calmTD[targetid], 2);
	PlayerTextDrawShow(targetid, calmTD[targetid]);

	// Messages
	new tmsg[160];
	format(tmsg, sizeof(tmsg), "{FFAA00}[Admin]{FFFFFF} %s vous a calm pendant {FFFF00}%ds{FFFFFF}. Respirez, regardez votre RP.",
		PlayerInfo[adminid][pName], duration);
	SendClientMessage(targetid, -1, tmsg);

	new amsg[160];
	format(amsg, sizeof(amsg), "{FFAA00}[Admin]{FFFFFF} Vous avez calm %s pendant %ds.",
		PlayerInfo[targetid][pName], duration);
	SendClientMessage(adminid, -1, amsg);

	SetTimerEx("Calm_Tick", 1000, false, "i", targetid);
	return 1;
}

stock Calm_End(playerid)
{
	if(calmUntil[playerid] == 0) return 0;
	calmUntil[playerid] = 0;
	if(IsPlayerConnected(playerid))
	{
		TogglePlayerControllable(playerid, true);
		if(calmTD[playerid] != PlayerText:INVALID_TEXT_DRAW)
		{
			PlayerTextDrawHide(playerid, calmTD[playerid]);
			PlayerTextDrawDestroy(playerid, calmTD[playerid]);
			calmTD[playerid] = PlayerText:INVALID_TEXT_DRAW;
		}
		SendClientMessage(playerid, -1, "{00FF00}[Admin]{FFFFFF} Calm termine. Bon RP.");
	}
	return 1;
}

stock Calm_OnConnect(playerid)
{
	calmUntil[playerid] = 0;
	calmTD[playerid] = PlayerText:INVALID_TEXT_DRAW;
	return 1;
}

stock Calm_OnDisconnect(playerid)
{
	Calm_End(playerid);
	return 1;
}
