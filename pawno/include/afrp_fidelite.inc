/*
	afrp_fidelite.inc - Recompense de fidelite basee sur le temps de jeu

	Recompense AUTOMATIQUE les joueurs pour leur temps de presence en jeu.
	Le gamemode compte deja les heures de jeu reelles (hors AFK) dans
	pConnectTime (incremente de 1 a chaque heure jouee, dans PayDay).

	Regle : tous les 50 HEURES de jeu (un "palier"), le joueur recoit
	2.000.000$ automatiquement, avec un message et un son. C'est recurrent :
	a 50h, 100h, 150h... il touche 2M a chaque fois.

	Joueurs deja fideles (au deploiement) : a leur premiere connexion apres
	l'installation, s'ils ont deja au moins 1 palier (>= 50h), ils recoivent
	UNE recompense de bienvenue-fidelite (2M) pour les remercier, puis le
	compteur repart de leur palier actuel (pas de versement retroactif massif).

	Anti-abus : le nombre de paliers deja payes est memorise par COMPTE (SQLID)
	dans scriptfiles/fidelite.cfg (une ligne "sqlid|paliers_payes"). Aucune
	nouvelle colonne DB requise.

	Hooks :
	  - Fidelite_Init() dans OnGameModeInit.
	  - Fidelite_OnLogin(playerid) apres un login reussi.
	  - Fidelite_CheckMilestone(playerid) dans PayDay, juste apres
	    l'increment de pConnectTime.
*/

#if defined AFRP_FIDELITE_INCLUDED
	#endinput
#endif
#define AFRP_FIDELITE_INCLUDED

#define FID_FILE                 "fidelite.cfg"
#define FID_MAX                  10000
#define FID_HOURS_PER_MILESTONE  50
#define FID_REWARD               2000000

new fid_Sqlid[FID_MAX];
new fid_PaidMilestones[FID_MAX];
new fid_Count = 0;

stock Fidelite_Init()
{
	fid_Count = 0;
	if(!fexist(FID_FILE)) return 0;
	new File:f = fopen(FID_FILE, io_read);
	if(!f) return 0;
	new line[32];
	while(fread(f, line))
	{
		if(strlen(line) < 1) continue;
		if(fid_Count >= FID_MAX) break;
		new sep = -1;
		for(new i = 0; line[i]; i++) { if(line[i] == '|') { sep = i; break; } }
		if(sep <= 0) continue;
		new sqBuf[16], mBuf[16];
		new p = 0;
		for(new i = 0; i < sep && p < 15; i++) sqBuf[p++] = line[i];
		sqBuf[p] = 0;
		p = 0;
		for(new i = sep + 1; line[i] && line[i] != '\r' && line[i] != '\n' && p < 15; i++) mBuf[p++] = line[i];
		mBuf[p] = 0;
		new sq = strval(sqBuf);
		if(sq < 1) continue;
		fid_Sqlid[fid_Count] = sq;
		fid_PaidMilestones[fid_Count] = strval(mBuf);
		fid_Count++;
	}
	fclose(f);
	printf("[Fidelite] %d compte(s) suivis pour la recompense de temps de jeu", fid_Count);
	return fid_Count;
}

stock Fidelite_Find(acctSqlid)
{
	if(acctSqlid < 1) return -1;
	for(new i = 0; i < fid_Count; i++)
		if(fid_Sqlid[i] == acctSqlid) return i;
	return -1;
}

stock Fidelite_Save()
{
	if(fexist(FID_FILE)) fremove(FID_FILE);
	new File:f = fopen(FID_FILE, io_write);
	if(!f) return 0;
	new line[32];
	for(new i = 0; i < fid_Count; i++)
	{
		format(line, sizeof(line), "%d|%d\n", fid_Sqlid[i], fid_PaidMilestones[i]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Fidelite_SetPaid(acctSqlid, milestones)
{
	new idx = Fidelite_Find(acctSqlid);
	if(idx == -1)
	{
		if(fid_Count >= FID_MAX) return 0;
		idx = fid_Count;
		fid_Sqlid[idx] = acctSqlid;
		fid_Count++;
	}
	fid_PaidMilestones[idx] = milestones;
	Fidelite_Save();
	return 1;
}

// A appeler apres un login reussi. Gere le cas des joueurs deja fideles.
stock Fidelite_OnLogin(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(PlayerInfo[playerid][pSQLID] < 1) return 0;

	new currentMilestone = PlayerInfo[playerid][pConnectTime] / FID_HOURS_PER_MILESTONE;
	new idx = Fidelite_Find(PlayerInfo[playerid][pSQLID]);

	if(idx != -1) return 0; // compte deja suivi, rien de special au login

	// Compte jamais vu par ce systeme (nouveau, ou existant au deploiement)
	if(currentMilestone >= 1)
	{
		// Joueur deja fidele : une recompense de remerciement, puis le compteur
		// repart de son palier actuel (pas de versement retroactif massif).
		SafeGivePlayerMoney(playerid, FID_REWARD, "Fidelite - remerciement anciens joueurs");
		new rwStr[24];
		FormatMoney(FID_REWARD, rwStr, sizeof(rwStr));
		new msg[220];
		format(msg, sizeof(msg),
			"{FFD700}[Fidelite]{FFFFFF} Merci pour ta fidelite ! Tu totalises {FFFF00}%d h{FFFFFF} de jeu. Cadeau : {00FF00}+%s ${FFFFFF}. Tu gagnes %s $ tous les %d h de jeu desormais.",
			PlayerInfo[playerid][pConnectTime], rwStr, rwStr, FID_HOURS_PER_MILESTONE);
		SendClientMessage(playerid, -1, msg);
		PlayerPlaySound(playerid, 1150, 0.0, 0.0, 0.0);
	}
	Fidelite_SetPaid(PlayerInfo[playerid][pSQLID], currentMilestone);
	return 1;
}

// /tempsjeu : affiche au joueur ses heures de jeu et son prochain palier.
stock Fidelite_ShowStatus(playerid)
{
	new heures = PlayerInfo[playerid][pConnectTime];
	new currentMilestone = heures / FID_HOURS_PER_MILESTONE;
	new nextAt = (currentMilestone + 1) * FID_HOURS_PER_MILESTONE;
	new reste = nextAt - heures;
	new rwStr[24];
	FormatMoney(FID_REWARD, rwStr, sizeof(rwStr));
	new body[400];
	format(body, sizeof(body),
		"{FFFFFF}Temps de jeu total : {FFFF00}%d heures{FFFFFF}\nPaliers de fidelite atteints : {FFFF00}%d{FFFFFF}\n\n{FFD700}Prochaine recompense :{FFFFFF}\nDans {FFFF00}%d heure(s){FFFFFF} de jeu (a %d h) tu gagneras {00FF00}%s ${FFFFFF}.\n\n{CCCCCC}Tu gagnes %s $ automatiquement tous les %d h de presence en jeu.",
		heures, currentMilestone, reste, nextAt, rwStr, rwStr, FID_HOURS_PER_MILESTONE);
	ShowPlayerDialog(playerid, 9946, DIALOG_STYLE_MSGBOX, "{FFD700}Ton temps de jeu", body, "OK", "");
	return 1;
}

// A appeler juste apres l'increment de pConnectTime (dans PayDay).
// Verse 2M par nouveau palier de 50h franchi.
stock Fidelite_CheckMilestone(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(PlayerInfo[playerid][pSQLID] < 1) return 0;

	new currentMilestone = PlayerInfo[playerid][pConnectTime] / FID_HOURS_PER_MILESTONE;
	new idx = Fidelite_Find(PlayerInfo[playerid][pSQLID]);
	new paid = (idx != -1) ? fid_PaidMilestones[idx] : 0;

	if(currentMilestone <= paid) return 0; // pas de nouveau palier

	new gained = currentMilestone - paid;
	new total = gained * FID_REWARD;
	SafeGivePlayerMoney(playerid, total, "Fidelite - palier temps de jeu");
	Fidelite_SetPaid(PlayerInfo[playerid][pSQLID], currentMilestone);

	new rwStr[24];
	FormatMoney(total, rwStr, sizeof(rwStr));
	new msg[220];
	format(msg, sizeof(msg),
		"{FFD700}[Fidelite]{FFFFFF} Palier de temps de jeu atteint ({FFFF00}%d h{FFFFFF}) ! Recompense : {00FF00}+%s ${FFFFFF}. Continue comme ca !",
		currentMilestone * FID_HOURS_PER_MILESTONE, rwStr);
	SendClientMessage(playerid, -1, msg);
	PlayerPlaySound(playerid, 1150, 0.0, 0.0, 0.0);
	return 1;
}
