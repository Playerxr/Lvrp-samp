/*
	afrp_jobrank.inc - Titres de rang + paliers visibles pour la progression job

	Le systeme de niveau/XP par job EXISTE DEJA (pJobLvl 1-10, pJobExp,
	job_GetXp(), bonus de paie qui scale). Ce module ajoute la couche
	"feeling gros serveur" qui manquait :
	- Titre de rang RP (Debutant -> Maitre) au lieu d'un simple chiffre
	- Celebration speciale (son + gametext + bonus cash) aux paliers 3/5/7/10
	- /jobrank : dialog recap propre (rang, XP, bonus, prochain palier)

	Hook : JobRank_OnLevelUp(playerid, newLevel) a appeler depuis le point
	exact ou pJobLvl est incremente (job_UpdateTexts dans LVRP.pwn).
*/

#if defined AFRP_JOBRANK_INCLUDED
	#endinput
#endif
#define AFRP_JOBRANK_INCLUDED

// Barre XP en bas a gauche, juste au-dessus du radar (qui occupe ~0-100/370-480)
// pour ne pas le chevaucher.
#define JOBBAR_X        108.0
#define JOBBAR_Y        425.0
#define JOBBAR_W_MAX    130.0
#define JOBBAR_H         6.0

new PlayerText:jobBarBg[MAX_PLAYERS];
new PlayerText:jobBarFg[MAX_PLAYERS];
new PlayerText:jobBarLbl[MAX_PLAYERS];
new jobBarVisible[MAX_PLAYERS];

static const jobRankTitles[10][16] = {
	"Debutant", "Debutant", "Apprenti", "Apprenti", "Confirme",
	"Confirme", "Experimente", "Expert", "Expert", "Maitre"
};

stock JobRank_GetTitle(level)
{
	if(level < 1) level = 1;
	if(level > 10) level = 10;
	return jobRankTitles[level - 1];
}

// ==================== BARRE XP (bas-gauche) ====================

stock JobBar_Create(playerid)
{
	if(jobBarBg[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	// Label au-dessus de la barre
	jobBarLbl[playerid] = CreatePlayerTextDraw(playerid, JOBBAR_X, JOBBAR_Y - 10.0, "Job");
	PlayerTextDrawFont(playerid, jobBarLbl[playerid], 2);
	PlayerTextDrawLetterSize(playerid, jobBarLbl[playerid], 0.18, 0.8);
	PlayerTextDrawColor(playerid, jobBarLbl[playerid], 0xFFFFFFFF);
	PlayerTextDrawSetShadow(playerid, jobBarLbl[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, jobBarLbl[playerid], 0x00000000);

	// Fond de la barre
	jobBarBg[playerid] = CreatePlayerTextDraw(playerid, JOBBAR_X, JOBBAR_Y, "_");
	PlayerTextDrawFont(playerid, jobBarBg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, jobBarBg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, jobBarBg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, jobBarBg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, jobBarBg[playerid], JOBBAR_X + JOBBAR_W_MAX, JOBBAR_Y + JOBBAR_H);
	PlayerTextDrawUseBox(playerid, jobBarBg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, jobBarBg[playerid], 0x000000AA);

	// Barre qui se remplit (largeur variable)
	jobBarFg[playerid] = CreatePlayerTextDraw(playerid, JOBBAR_X, JOBBAR_Y, "_");
	PlayerTextDrawFont(playerid, jobBarFg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, jobBarFg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, jobBarFg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, jobBarFg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, jobBarFg[playerid], JOBBAR_X + 1.0, JOBBAR_Y + JOBBAR_H);
	PlayerTextDrawUseBox(playerid, jobBarFg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, jobBarFg[playerid], 0xFFD700FF);
	return 1;
}

stock JobBar_Update(playerid)
{
	if(PlayerInfo[playerid][pJob] == 0)
	{
		JobBar_Hide(playerid);
		return 0;
	}
	JobBar_Create(playerid);

	new level = PlayerInfo[playerid][pJobLvl];
	new title[16];
	title = JobRank_GetTitle(level);

	new Float:pct;
	if(level >= 10)
	{
		pct = 1.0;
	}
	else
	{
		new xpNeeded = job_GetXp(playerid, PlayerInfo[playerid][pJob]);
		if(xpNeeded <= 0) xpNeeded = 1;
		pct = float(PlayerInfo[playerid][pJobExp]) / float(xpNeeded);
		if(pct > 1.0) pct = 1.0;
		if(pct < 0.0) pct = 0.0;
	}

	new Float:fillW = JOBBAR_W_MAX * pct;
	if(fillW < 1.0) fillW = 1.0;
	PlayerTextDrawTextSize(playerid, jobBarFg[playerid], JOBBAR_X + fillW, JOBBAR_Y + JOBBAR_H);

	new lbl[48];
	if(level >= 10)
		format(lbl, sizeof(lbl), "%s - %s (MAX)", GetJobName(PlayerInfo[playerid][pJob]), title);
	else
		format(lbl, sizeof(lbl), "%s - %s Lvl %d", GetJobName(PlayerInfo[playerid][pJob]), title, level);
	PlayerTextDrawSetString(playerid, jobBarLbl[playerid], lbl);

	if(jobBarVisible[playerid] != 1)
	{
		PlayerTextDrawShow(playerid, jobBarLbl[playerid]);
		PlayerTextDrawShow(playerid, jobBarBg[playerid]);
		PlayerTextDrawShow(playerid, jobBarFg[playerid]);
		jobBarVisible[playerid] = 1;
	}
	return 1;
}

stock JobBar_Hide(playerid)
{
	if(jobBarBg[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;
	PlayerTextDrawHide(playerid, jobBarLbl[playerid]);
	PlayerTextDrawHide(playerid, jobBarBg[playerid]);
	PlayerTextDrawHide(playerid, jobBarFg[playerid]);
	jobBarVisible[playerid] = 0;
	return 1;
}

stock JobBar_OnConnect(playerid)
{
	jobBarBg[playerid] = PlayerText:INVALID_TEXT_DRAW;
	jobBarFg[playerid] = PlayerText:INVALID_TEXT_DRAW;
	jobBarLbl[playerid] = PlayerText:INVALID_TEXT_DRAW;
	jobBarVisible[playerid] = 0;
	return 1;
}

stock JobBar_OnDisconnect(playerid)
{
	if(jobBarBg[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, jobBarLbl[playerid]);
		PlayerTextDrawDestroy(playerid, jobBarBg[playerid]);
		PlayerTextDrawDestroy(playerid, jobBarFg[playerid]);
	}
	JobBar_OnConnect(playerid);
	return 1;
}

// Appele juste apres l'incrementation de pJobLvl (voir job_UpdateTexts)
stock JobRank_OnLevelUp(playerid, newLevel)
{
	new title[16];
	title = JobRank_GetTitle(newLevel);

	// Paliers celebres avec bonus cash + gametext + son
	if(newLevel == 3 || newLevel == 5 || newLevel == 7 || newLevel == 10)
	{
		new bonus = newLevel * 1000;
		GivePlayerMoney(playerid, bonus);
		PlayerInfo[playerid][pCash] += bonus;

		new gtext[100];
		format(gtext, sizeof(gtext), "~y~~h~PALIER ATTEINT~n~~w~%s~n~~g~+%d $", title, bonus);
		GameTextForPlayer(playerid, gtext, 5000, 4);
		PlayerPlaySound(playerid, 1149, 0.0, 0.0, 0.0);

		new msg[180];
		format(msg, sizeof(msg),
			"{FFD700}[Carriere]{FFFFFF} Vous etes maintenant {FFFF00}%s{FFFFFF} (niveau %d) ! Bonus de palier: {00FF00}%d $",
			title, newLevel, bonus);
		SendClientMessage(playerid, -1, msg);
	}
	else
	{
		new msg[150];
		format(msg, sizeof(msg), "{78769D}[Carriere]{FFFFFF} Niveau %d atteint - Rang: {FFFF00}%s", newLevel, title);
		SendClientMessage(playerid, -1, msg);
	}
	return 1;
}

stock JobRank_CmdInfo(playerid)
{
	if(PlayerInfo[playerid][pJob] == 0)
	{
		SendClientMessage(playerid, -1, "{FF4444}[Carriere]{FFFFFF} Vous n'avez pas de job. Tapez {FFFF00}/jobs{FFFFFF} pour en choisir un.");
		return 1;
	}
	new level = PlayerInfo[playerid][pJobLvl];
	new title[16];
	title = JobRank_GetTitle(level);
	new xpNeeded = job_GetXp(playerid, PlayerInfo[playerid][pJob]);
	new xpCur = PlayerInfo[playerid][pJobExp];

	new nextMilestone = 0;
	if(level < 3) nextMilestone = 3;
	else if(level < 5) nextMilestone = 5;
	else if(level < 7) nextMilestone = 7;
	else if(level < 10) nextMilestone = 10;

	new info[400];
	if(level >= 10)
	{
		format(info, sizeof(info),
			"{FFFFFF}Job:{FFFF00} %s\n{FFFFFF}Rang:{FFFF00} %s {FFFFFF}(niveau MAX)\n{FFFFFF}Bonus paye:{00FF00} %d $\n\n{FFD700}Vous avez atteint le rang maximum !",
			GetJobName(PlayerInfo[playerid][pJob]), title, PlayerInfo[playerid][pJobBonus]);
	}
	else
	{
		format(info, sizeof(info),
			"{FFFFFF}Job:{FFFF00} %s\n{FFFFFF}Rang:{FFFF00} %s {FFFFFF}(niveau %d/10)\n{FFFFFF}XP:{FFFFFF} %d / %d\n{FFFFFF}Bonus paye:{00FF00} %d $\n\n{8C8C8C}Prochain palier special: niveau %d",
			GetJobName(PlayerInfo[playerid][pJob]), title, level, xpCur, xpNeeded, PlayerInfo[playerid][pJobBonus], nextMilestone);
	}
	ShowPlayerDialog(playerid, 9750, DIALOG_STYLE_MSGBOX, "{FFAA00}>> Ma carriere <<", info, "OK", "");
	return 1;
}
