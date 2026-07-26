/*
	afrp_factionmenu.inc - Menu de faction (simplifie les slashs)

	Au lieu de retenir et taper "/p menotter <id>", le joueur tape /menu :
	un dialogue liste les actions de SA faction. Il clique "Menotter", choisit
	la cible parmi les joueurs proches, et l'action s'execute.

	ASTUCE TECHNIQUE : le menu ne reimplemente RIEN. Il reconstruit la vraie
	commande (ex: "/p menotter 5") et appelle OnPlayerCommandText -> toute la
	logique/verifs existantes (rang, service, distance...) s'appliquent telles
	quelles. Zero duplication, zero risque de divergence.

	Types d'action :
	  FM_PLAIN     : execute la commande telle quelle
	  FM_TARGET    : choisir un joueur proche -> "cmd <id>"
	  FM_NUM       : saisir un nombre        -> "cmd <n>"
	  FM_TEXT      : saisir un texte         -> "cmd <texte>"
	  FM_TARGETNUM : cible + nombre          -> "cmd <id> <n>"

	Un rappel chat toutes les 3 min informe les membres de faction de /menu.
*/

#if defined AFRP_FACTIONMENU_INCLUDED
	#endinput
#endif
#define AFRP_FACTIONMENU_INCLUDED

#define FM_PLAIN      0
#define FM_TARGET     1
#define FM_NUM        2
#define FM_TEXT       3
#define FM_TARGETNUM  4

// Groupes de faction
#define FMG_NONE     0
#define FMG_POLICE   1
#define FMG_MEDIC    2
#define FMG_ARMY     3
#define FMG_POMPIER  4
#define FMG_GANG     5
#define FMG_MAFIA    6
#define FMG_CIA      7
#define FMG_JUSTICE  8

#define FM_DLG_MENU    9800
#define FM_DLG_TARGET  9801
#define FM_DLG_NUM     9802
#define FM_DLG_TEXT    9803

#define FM_TARGET_RANGE   20.0  // portee de selection des joueurs proches
#define FM_REMINDER_SEC   180   // rappel /menu toutes les 3 min

enum e_fmEntry {
	fmGrp,
	fmLabel[36],
	fmCmd[26],
	fmType
};

static const fmEntries[][e_fmEntry] = {
	// ---- POLICE (pMember 1-5, /p fonctionne aussi pour le FBI) ----
	{FMG_POLICE,  "Menotter un suspect",        "/p menotter",   FM_TARGET},
	{FMG_POLICE,  "Demenotter",                 "/p demenotter", FM_TARGET},
	{FMG_POLICE,  "Fouiller",                   "/p fouiller",   FM_TARGET},
	{FMG_POLICE,  "Mettre une amende",          "/p amende",     FM_TARGET},
	{FMG_POLICE,  "Taser",                      "/p tazer",      FM_TARGET},
	{FMG_POLICE,  "Montrer le badge",           "/p badge",      FM_TARGET},
	{FMG_POLICE,  "Garde a vue (minutes)",      "/p gav",        FM_NUM},
	{FMG_POLICE,  "Gyrophare / Sirene",         "/p gyrophare",  FM_PLAIN},
	{FMG_POLICE,  "Annonce radio",              "/p ann",        FM_TEXT},
	{FMG_POLICE,  "Voir toutes mes commandes",  "/p",            FM_PLAIN},

	// ---- MEDECIN / HOPITAL (pMember 15) ----
	{FMG_MEDIC,   "Prendre/Quitter le service", "/duty hopital", FM_PLAIN},
	{FMG_MEDIC,   "Reanimer (personne proche)", "/med reanim",   FM_PLAIN},
	{FMG_MEDIC,   "Soigner un joueur",          "/med soigner",  FM_TARGETNUM},
	{FMG_MEDIC,   "Tracer le dernier appel",    "/med tda",      FM_PLAIN},
	{FMG_MEDIC,   "Voir toutes mes commandes",  "/med",          FM_PLAIN},

	// ---- ARMEE (pMember 11-13) ----
	{FMG_ARMY,    "Voir toutes mes commandes",  "/armee",        FM_PLAIN},

	// ---- POMPIERS (pMember 999) ----
	{FMG_POMPIER, "Voir toutes mes commandes",  "/po",           FM_PLAIN},

	// ---- GANG (illegal fType 1) ----
	{FMG_GANG,    "Voir toutes mes commandes",  "/gang",         FM_PLAIN},

	// ---- MAFIA (illegal fType 2) ----
	{FMG_MAFIA,   "Voir toutes mes commandes",  "/mafia",        FM_PLAIN},

	// ---- CIA (pMember 9) ----
	{FMG_CIA,     "Voir toutes mes commandes",  "/cia",          FM_PLAIN},

	// ---- JUSTICE / COUR (pMember 14) ----
	{FMG_JUSTICE, "Prendre/Quitter le service", "/j duty",       FM_PLAIN},
	{FMG_JUSTICE, "Examiner les recours",       "/proces",       FM_PLAIN},
	{FMG_JUSTICE, "Defendre un detenu",         "/defendre",     FM_TARGET},
	{FMG_JUSTICE, "Voir toutes mes commandes",  "/justice",      FM_PLAIN}
};

// Etat par joueur
new fmEntryIndex[MAX_PLAYERS][32]; // listitem -> index global dans fmEntries
new fmEntryCount[MAX_PLAYERS];
new fmNearbyIds[MAX_PLAYERS][64];  // listitem -> playerid cible
new fmNearbyCount[MAX_PLAYERS];

forward FactionMenu_ReminderTick();

stock FactionMenu_Init()
{
	SetTimer("FactionMenu_ReminderTick", FM_REMINDER_SEC * 1000, true);
	return 1;
}

stock FactionMenu_OnConnect(playerid)
{
	fmEntryCount[playerid] = 0;
	fmNearbyCount[playerid] = 0;
	return 1;
}

stock FactionMenu_GetGroup(playerid)
{
	new m = PlayerInfo[playerid][pMember];
	if(m >= 1 && m <= 5) return FMG_POLICE;
	if(m == 9) return FMG_CIA;
	if(m == FACTION_HOPITAL) return FMG_MEDIC;
	if(m == FACTION_JUSTICE) return FMG_JUSTICE;
	if(m >= 11 && m <= 13) return FMG_ARMY;
	if(m == 999) return FMG_POMPIER;
	if(m >= 200)
	{
		new facID = m - 200;
		if(facID >= 0 && facID < MAX_FACTION && FactionInfo[facID][fType] == 2) return FMG_MAFIA;
		return FMG_GANG;
	}
	return FMG_NONE;
}

stock FactionMenu_GroupName(grp, dest[], size)
{
	switch(grp)
	{
		case FMG_POLICE:  format(dest, size, "Police / LSPD");
		case FMG_MEDIC:   format(dest, size, "Hopital / EMS");
		case FMG_ARMY:    format(dest, size, "Armee");
		case FMG_POMPIER: format(dest, size, "Pompiers");
		case FMG_GANG:    format(dest, size, "Gang");
		case FMG_MAFIA:   format(dest, size, "Mafia");
		case FMG_CIA:     format(dest, size, "CIA");
		case FMG_JUSTICE: format(dest, size, "Justice / Cour");
		default:          format(dest, size, "Faction");
	}
	return 1;
}

stock FactionMenu_Open(playerid)
{
	new grp = FactionMenu_GetGroup(playerid);
	if(grp == FMG_NONE)
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Menu]{FFFFFF} Vous n'etes dans aucune faction. Ce menu est reserve aux factions.");
		return 1;
	}

	new list[1200];
	list[0] = 0;
	fmEntryCount[playerid] = 0;
	for(new i = 0; i < sizeof(fmEntries); i++)
	{
		if(fmEntries[i][fmGrp] != grp) continue;
		if(fmEntryCount[playerid] >= 32) break;
		strcat(list, fmEntries[i][fmLabel], sizeof(list));
		strcat(list, "\n", sizeof(list));
		fmEntryIndex[playerid][fmEntryCount[playerid]] = i;
		fmEntryCount[playerid]++;
	}

	new gname[24], title[64];
	FactionMenu_GroupName(grp, gname, sizeof(gname));
	format(title, sizeof(title), "{FFAA00}>> Menu %s <<", gname);
	ShowPlayerDialog(playerid, FM_DLG_MENU, DIALOG_STYLE_LIST, title, list, "Choisir", "Fermer");
	return 1;
}

// Construit et joue la vraie commande
stock FactionMenu_Exec(playerid, cmd[])
{
	OnPlayerCommandText(playerid, cmd);
	return 1;
}

// Affiche la liste des joueurs proches pour selectionner une cible
stock FactionMenu_ShowTargets(playerid)
{
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	new list[900];
	list[0] = 0;
	fmNearbyCount[playerid] = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(!IsPlayerInRangeOfPoint(i, FM_TARGET_RANGE, px, py, pz)) continue;
		if(fmNearbyCount[playerid] >= 64) break;
		new Float:d = GetPlayerDistanceFromPoint(i, px, py, pz);
		new line[80];
		format(line, sizeof(line), "{FFFFFF}%s {8C8C8C}(id %d) - {FFFF00}%.0fm\n", PlayerInfo[i][pName], i, d);
		strcat(list, line, sizeof(list));
		fmNearbyIds[playerid][fmNearbyCount[playerid]] = i;
		fmNearbyCount[playerid]++;
	}

	if(fmNearbyCount[playerid] == 0)
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Menu]{FFFFFF} Aucun joueur a proximite. Rapprochez-vous de la cible.");
		return 1;
	}
	ShowPlayerDialog(playerid, FM_DLG_TARGET, DIALOG_STYLE_LIST,
		"{FFAA00}>> Choisir la cible <<{FFFFFF}", list, "Valider", "Retour");
	return 1;
}

stock FactionMenu_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	if(dialogid == FM_DLG_MENU)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= fmEntryCount[playerid]) return 1;
		new gi = fmEntryIndex[playerid][listitem];
		new type = fmEntries[gi][fmType];
		new cmd[26];
		cmd[0] = 0;
		strcat(cmd, fmEntries[gi][fmCmd], sizeof(cmd));

		SetPVarString(playerid, "fm_cmd", cmd);
		SetPVarInt(playerid, "fm_type", type);

		switch(type)
		{
			case FM_PLAIN:                 FactionMenu_Exec(playerid, cmd);
			case FM_TARGET, FM_TARGETNUM:  FactionMenu_ShowTargets(playerid);
			case FM_NUM:  ShowPlayerDialog(playerid, FM_DLG_NUM, DIALOG_STYLE_INPUT, "{FFAA00}>> Valeur <<", "{FFFFFF}Entrez un nombre :", "Valider", "Retour");
			case FM_TEXT: ShowPlayerDialog(playerid, FM_DLG_TEXT, DIALOG_STYLE_INPUT, "{FFAA00}>> Texte <<", "{FFFFFF}Entrez le texte :", "Valider", "Retour");
		}
		return 1;
	}

	if(dialogid == FM_DLG_TARGET)
	{
		if(!response) { FactionMenu_Open(playerid); return 1; }
		if(listitem < 0 || listitem >= fmNearbyCount[playerid]) return 1;
		new targetid = fmNearbyIds[playerid][listitem];
		if(!IsPlayerConnected(targetid))
			{SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Menu]{FFFFFF} Cible non disponible."); return 1;}

		new cmd[26];
		GetPVarString(playerid, "fm_cmd", cmd, sizeof(cmd));
		new type = GetPVarInt(playerid, "fm_type");

		if(type == FM_TARGETNUM)
		{
			SetPVarInt(playerid, "fm_target", targetid);
			ShowPlayerDialog(playerid, FM_DLG_NUM, DIALOG_STYLE_INPUT, "{FFAA00}>> Valeur <<", "{FFFFFF}Entrez un nombre (ex: prix) :", "Valider", "Retour");
			return 1;
		}
		// FM_TARGET simple
		new full[48];
		format(full, sizeof(full), "%s %d", cmd, targetid);
		FactionMenu_Exec(playerid, full);
		return 1;
	}

	if(dialogid == FM_DLG_NUM)
	{
		if(!response) return 1;
		new cmd[26];
		GetPVarString(playerid, "fm_cmd", cmd, sizeof(cmd));
		new type = GetPVarInt(playerid, "fm_type");
		new n = strval(inputtext);

		if(type == FM_TARGETNUM)
		{
			new targetid = GetPVarInt(playerid, "fm_target");
			new full[56];
			format(full, sizeof(full), "%s %d %d", cmd, targetid, n);
			FactionMenu_Exec(playerid, full);
		}
		else // FM_NUM
		{
			new full[40];
			format(full, sizeof(full), "%s %d", cmd, n);
			FactionMenu_Exec(playerid, full);
		}
		return 1;
	}

	if(dialogid == FM_DLG_TEXT)
	{
		if(!response) return 1;
		if(!strlen(inputtext)) return 1;
		new cmd[26];
		GetPVarString(playerid, "fm_cmd", cmd, sizeof(cmd));
		new full[170];
		format(full, sizeof(full), "%s %s", cmd, inputtext);
		FactionMenu_Exec(playerid, full);
		return 1;
	}
	return 0;
}

public FactionMenu_ReminderTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(FactionMenu_GetGroup(i) == FMG_NONE) continue;
		SendClientMessage(i, 0x82C8FFFF, "{82C8FF}[Menu]{FFFFFF} Astuce : tape {FFFF00}/menu{FFFFFF} pour acceder a toutes les commandes de ta faction en un clic.");
	}
	return 1;
}
