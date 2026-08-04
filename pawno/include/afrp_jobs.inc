/*
	afrp_jobs.inc - Mise en evidence des jobs + incitations

	Objectif : rendre les jobs visibles et attractifs pour que les joueurs
	les fassent enfin.

	1) ICONES CARTE : une icone a chaque employeur principal (par ville LS).
	2) /jobs : menu listant chaque metier + son salaire, avec pose d'un GPS
	   (checkpoint) vers l'employeur choisi.
	3) RAPPEL AUTO : message periodique aux joueurs SANS job/faction pour les
	   inciter a bosser.
	4) BONUS DEBUTANT : les joueurs de bas niveau (<=3) gagnent le DOUBLE sur
	   chaque action de job -> accroche les nouveaux.
	5) PRIME DE SERIE (streak) : enchainer les actions de job sans s'arreter
	   augmente le gain (+10% par action consecutive, plafonne a +100%).

	Les points 4 et 5 s'appliquent dans job_TakePay (choke point unique des
	gains de job) via Jobs_ComputeReward().
*/

#if defined AFRP_JOBS_INCLUDED
	#endinput
#endif
#define AFRP_JOBS_INCLUDED

#define JOB_ICON_ID        22      // icone employeur (fork/knife, tres visible)
#define JOB_REMINDER_SEC   300     // rappel chat toutes les 5 min
#define JOB_DIALOG_SEC     600     // dialogue promo toutes les 10 min
#define JOB_DIALOG_PROMO   9773    // dialog promo jobs
#define JOB_STREAK_WINDOW  180     // s : delai max entre 2 actions pour garder la serie
#define JOB_STREAK_STEP    10      // % de bonus par action consecutive
#define JOB_STREAK_CAPPCT  100     // plafond du bonus de serie (%)
#define JOB_DEBUTANT_LEVEL 3       // niveau <= -> bonus debutant (x2)

// Employeur principal par metier NPC : {jobtype, x, y, z}
// (coords prises dans init_Actors, ville LS en priorite)
static const Float:jobEmployer[][4] = {
	{1.0,  2117.3208, -1790.5564, 13.5547}, // Pizza
	{2.0,  -382.6989, -1426.2883, 26.2492}, // Fermier
	{3.0,  -1357.5438, 2450.8828, 88.9278}, // Mineur (El Quebrados)
	{4.0,  2276.6580, -2226.9092, 13.5469}, // Eboueur
	{5.0,  2481.3362,  1958.0083, 10.6214}, // Ouvrier (LV)
	{6.0,  1969.4204, -2189.0742, 13.5469}, // Pilote de Ligne
	{7.0,  1743.1252, -1582.0260, 13.5494}, // Facteur
	{8.0,  906.1063,  -1904.2996,  1.1277}, // Pecheur
	{9.0,  321.8867,  -1519.5338, 36.0391}, // Voiturier
	{10.0, 2422.9304, -2075.8716, 13.5538}, // Camioneur
	{12.0, 1906.7000, -2207.9000, 13.5469}, // Bagagiste
	{14.0, 1174.0000,  -485.0000, 26.5400}, // Bucheron
	{17.0, 1008.4938, -1358.3921, 13.3909}, // Mecanicien
	{19.0, 1516.9620, -1022.1600, 23.8301}, // Transporteur de fonds
	{20.0, 1059.8000, -1350.6000, 13.5469}, // Chauffeur de Taxi
	// [JOBS NEW] Ajoute EN FIN de tableau : les index precedents, qui servent
	// aussi de numero de ligne dans le dialogue /jobs, ne bougent pas.
	{13.0, 2428.9304, -2075.8716, 13.5538}  // Benevole d'entreprise (docks LS)
};

new jobStreak[MAX_PLAYERS];
new jobStreakLast[MAX_PLAYERS];

forward Jobs_ReminderTick();
forward Jobs_DialogTick();

stock Jobs_Init()
{
	for(new i = 0; i < sizeof(jobEmployer); i++)
	{
		CreateDynamicMapIcon(jobEmployer[i][1], jobEmployer[i][2], jobEmployer[i][3],
			JOB_ICON_ID, 0, -1, -1, -1, 1500.0);
	}
	SetTimer("Jobs_ReminderTick", JOB_REMINDER_SEC * 1000, true);
	SetTimer("Jobs_DialogTick", JOB_DIALOG_SEC * 1000, true);
	return 1;
}

stock Jobs_OnConnect(playerid)
{
	jobStreak[playerid] = 0;
	jobStreakLast[playerid] = 0;
	return 1;
}

stock Jobs_ResetStreak(playerid)
{
	jobStreak[playerid] = 0;
	jobStreakLast[playerid] = 0;
	return 1;
}

// Appele depuis job_TakePay AVANT d'ajouter le cash : met a jour la serie et
// renvoie le cash ajuste (bonus debutant + prime de serie).
stock Jobs_ComputeReward(playerid, baseCash)
{
	new now = gettime();

	// Serie : si dernier gain trop ancien -> reset, sinon on incremente
	if(jobStreakLast[playerid] == 0 || (now - jobStreakLast[playerid]) > JOB_STREAK_WINDOW)
		jobStreak[playerid] = 1;
	else
		jobStreak[playerid]++;
	jobStreakLast[playerid] = now;

	new mult = 100; // %

	// Prime de serie : +STEP% par action consecutive au-dela de la 1ere
	new streakBonus = (jobStreak[playerid] - 1) * JOB_STREAK_STEP;
	if(streakBonus > JOB_STREAK_CAPPCT) streakBonus = JOB_STREAK_CAPPCT;
	mult += streakBonus;

	// Bonus debutant : double pour les bas niveaux
	new bool:debutant = (PlayerInfo[playerid][pLevel] <= JOB_DEBUTANT_LEVEL);
	if(debutant) mult += 100;

	new finalCash = (baseCash * mult) / 100;

	// Feedback discret quand la serie ou le bonus s'applique
	if(streakBonus > 0 || debutant)
	{
		new msg[128];
		if(debutant)
			format(msg, sizeof(msg), "{00FF88}[Job]{FFFFFF} Serie x%d {8C8C8C}(+%d%%){FFFFFF} + {FFD700}bonus debutant x2{FFFFFF} !", jobStreak[playerid], streakBonus);
		else
			format(msg, sizeof(msg), "{00FF88}[Job]{FFFFFF} Serie x%d {8C8C8C}(+%d%% de gains){FFFFFF} !", jobStreak[playerid], streakBonus);
		SendClientMessage(playerid, -1, msg);
	}
	return finalCash;
}

// /jobs : liste des metiers + salaire, avec GPS
stock Jobs_CmdList(playerid)
{
	new list[1500];
	list[0] = 0;
	for(new i = 0; i < sizeof(jobEmployer); i++)
	{
		new job = floatround(jobEmployer[i][0]);
		new salaire = governement[3][salaryJob][job-1];
		new line[128];
		format(line, sizeof(line), "{FFFFFF}%s\t{00FF00}$%d\t{FFD700}GPS\n", GetJobName(job), salaire);
		strcat(list, line, sizeof(list));
	}
	ShowPlayerDialog(playerid, 9770, DIALOG_STYLE_TABLIST_HEADERS,
		"{FFAA00}>> Metiers disponibles <<{FFFFFF}\tSalaire\t ", list, "GPS", "Fermer");
	return 1;
}

stock Jobs_OnDialogResponse(playerid, dialogid, response, listitem)
{
	// Dialog promo (toutes les 10 min) : bouton "Voir les jobs" -> ouvre la liste GPS
	if(dialogid == JOB_DIALOG_PROMO)
	{
		if(response) Jobs_CmdList(playerid);
		return 1;
	}
	if(dialogid != 9770) return 0;
	if(!response) return 1;
	if(listitem < 0 || listitem >= sizeof(jobEmployer)) return 1;

	new job = floatround(jobEmployer[listitem][0]);
	SetPlayerCheckpoint(playerid, jobEmployer[listitem][1], jobEmployer[listitem][2], jobEmployer[listitem][3], 4.0);

	new msg[144];
	format(msg, sizeof(msg),
		"{00FF88}[Job]{FFFFFF} GPS active vers l'employeur {FFFF00}%s{FFFFFF}. Suivez le marqueur, puis touche {FFFF00}'N'{FFFFFF} sur place.",
		GetJobName(job));
	SendClientMessage(playerid, -1, msg);
	return 1;
}

public Jobs_ReminderTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pJob] != 0) continue;   // a deja un job
		if(PlayerInfo[i][pMember] != 0) continue; // membre d'une faction
		if(PlayerInfo[i][pLeader] != 0) continue;
		SendClientMessage(i, 0x00FF88FF, "{00FF88}[Job]{FFFFFF} Sans emploi ? Tape {FFFF00}/jobs{FFFFFF} pour voir les metiers, leur salaire et un GPS. Les nouveaux gagnent le {FFD700}DOUBLE{FFFFFF} !");
	}
	return 1;
}

// Toutes les 10 min : dialogue promo aux joueurs sans emploi (ni faction).
// On evite d'interrompre ceux qui sont deja occupes (dialog deja ouvert, coma).
public Jobs_DialogTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pJob] != 0) continue;   // a deja un job
		if(PlayerInfo[i][pMember] != 0) continue; // faction
		if(PlayerInfo[i][pLeader] != 0) continue;
		ShowPlayerDialog(i, JOB_DIALOG_PROMO, DIALOG_STYLE_MSGBOX,
			"{FFAA00}>> Besoin d'argent ? <<{FFFFFF}",
			"{FFFFFF}Tu n'as pas encore de metier !\n\nPrends-en un pour gagner ta vie legalement :\n\n{00FF00}- Salaire regulier a chaque paye\n- Bonus x2 pour les nouveaux joueurs\n- Prime de serie en enchainant les taches\n- GPS automatique vers l'employeur\n\n{FFFFFF}Clique sur {FFFF00}Voir les jobs{FFFFFF} pour commencer !",
			"Voir les jobs", "Plus tard");
	}
	return 1;
}
