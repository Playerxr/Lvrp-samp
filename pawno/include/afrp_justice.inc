/*
	afrp_justice.inc - Proces : la Cour de Justice devient jouable

	CONSTAT : la faction Cour de Justice (FACTION_JUSTICE) existait avec 6
	grades (Greffier, Magistrat, Avocat, Juge, Procureur, Procureur de la
	Republique) et un metier Avocat, mais /justice ne proposait que prendre
	son service, inviter, changer un grade, demissionner et faire une annonce.
	Aucun gameplay : un joueur qui rejoignait cette faction n'avait
	litteralement rien a faire. Ce module lui donne son role.

	BOUCLE DE JEU (fait interagir 3 groupes qui s'ignoraient) :
	    la police arrete  ->  le detenu fait appel  ->  un avocat le defend
	    ->  un juge rend son verdict  ->  la peine est ajustee

	COMMANDES :
	  /appel            (detenu)  depose un recours, la Justice est prevenue
	  /defendre <id>    (avocat)  se porte defenseur, fixe ses honoraires
	  /proces           (Justice) liste des recours -> fiche -> verdict

	VERDICTS (juge = grade 4+ : Juge, Procureur, Procureur de la Republique)
	  Relaxe          liberation immediate + remboursement de l'amende
	  Reduire         peine divisee par deux
	  Confirmer       la peine est maintenue
	  Alourdir        peine augmentee de moitie (plafonnee)

	HONORAIRES : l'avocat n'est paye QUE s'il obtient un resultat (relaxe ou
	reduction). Pas de resultat, pas d'honoraires - ca evite les avocats
	fantomes qui s'inscrivent sur tous les dossiers pour encaisser.

	LIBERATION : on se contente de ramener pJailTime a 2 secondes et on laisse
	le timer de prison deja existant liberer le joueur. Reutiliser ce chemin
	teste evite de dupliquer (donc de desynchroniser) la sortie de prison.

	Les dossiers vivent en memoire : une peine dure 10 minutes au maximum,
	les persister en fichier n'aurait aucun interet.
*/

#if defined AFRP_JUSTICE_INCLUDED
	#endinput
#endif
#define AFRP_JUSTICE_INCLUDED

#define JU_DLG_LIST         9940        // liste des recours
#define JU_DLG_CASE         9941        // fiche + verdict
#define JU_DLG_REDUCE       9942        // (reserve) saisie libre
#define JU_DLG_FEE          9943        // honoraires de l'avocat

#define JU_MAX_CASE         30
#define JU_RANK_LAWYER      3           // grade Avocat dans la Cour de Justice
#define JU_RANK_JUDGE       4           // grade minimum pour rendre un verdict
#define JU_JOB_LAWYER       21          // metier Avocat
#define JU_FEE_MAX          500000      // honoraires maximum
#define JU_TIME_CAP         1800        // peine plafond apres alourdissement (30 min)

enum e_JuCase
{
	juUsed,
	juPrisonerSqlid,
	juOfficer[MAX_PLAYER_NAME],   // qui a procede a l'arrestation
	juFine,                       // amende payee lors de l'arrestation
	juTimeAtAppeal,               // peine restante au moment du recours (sec)
	juLawyerSqlid,                // 0 = pas d'avocat
	juFee,                        // honoraires convenus
	juCreatedTs
};
new JuCases[JU_MAX_CASE][e_JuCase];

new ju_CaseSel[MAX_PLAYERS];              // dossier consulte
new ju_CaseMap[MAX_PLAYERS][JU_MAX_CASE]; // listitem -> index dossier
new ju_PendingDefend[MAX_PLAYERS];        // dossier en attente de saisie d'honoraires

stock Justice_Init()
{
	for(new i = 0; i < JU_MAX_CASE; i++) JuCases[i][juUsed] = 0;
	for(new p = 0; p < MAX_PLAYERS; p++) { ju_CaseSel[p] = -1; ju_PendingDefend[p] = -1; }
	return 1;
}

stock Justice_FindCaseBySqlid(ju_findSqlid)
{
	if(ju_findSqlid < 1) return -1;
	for(new i = 0; i < JU_MAX_CASE; i++)
	{
		if(JuCases[i][juUsed] != 1) continue;
		if(JuCases[i][juPrisonerSqlid] == ju_findSqlid) return i;
	}
	return -1;
}

stock Justice_FindPlayerBySqlid(ju_sqlid)
{
	if(ju_sqlid < 1) return INVALID_PLAYER_ID;
	for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
	{
		if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
		if(PlayerInfo[p][pSQLID] == ju_sqlid) return p;
	}
	return INVALID_PLAYER_ID;
}

// Membre de la Cour de Justice, en service.
stock Justice_IsOnDuty(playerid)
{
	if(PlayerInfo[playerid][pMember] != FACTION_JUSTICE) return 0;
	if(police_Duty[playerid] != 1) return 0;
	return 1;
}

// Avocat : grade Avocat de la Cour, OU joueur exercant le metier d'avocat.
stock Justice_IsLawyer(playerid)
{
	if(PlayerInfo[playerid][pMember] == FACTION_JUSTICE
		&& PlayerInfo[playerid][pRank] >= JU_RANK_LAWYER) return 1;
	if(PlayerInfo[playerid][pJob] == JU_JOB_LAWYER) return 1;
	return 0;
}

// Libere le detenu en passant par le timer de prison existant.
stock Justice_Release(prisonerId)
{
	if(!IsPlayerConnected(prisonerId)) return 0;
	PlayerInfo[prisonerId][pJailTime] = 2;
	return 1;
}

// Ferme un dossier (fin de peine, deconnexion, verdict rendu).
stock Justice_CloseCase(ci)
{
	if(ci < 0 || ci >= JU_MAX_CASE) return 0;
	JuCases[ci][juUsed] = 0;
	return 1;
}

// Appelee quand un joueur sort de prison ou se deconnecte : le dossier n'a
// plus d'objet, on le retire pour ne pas polluer la liste des juges.
stock Justice_OnPlayerFree(playerid)
{
	new ci = Justice_FindCaseBySqlid(PlayerInfo[playerid][pSQLID]);
	if(ci != -1) Justice_CloseCase(ci);
	return 1;
}

// ============================================================================
// /appel : le detenu depose un recours
// ============================================================================
stock Justice_CmdAppel(playerid)
{
	if(PlayerInfo[playerid][pJailed] != 1 || PlayerInfo[playerid][pJailTime] <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Tu n'es pas en prison, tu n'as rien a contester.");

	if(Justice_FindCaseBySqlid(PlayerInfo[playerid][pSQLID]) != -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Ton recours est deja depose, attends qu'un juge l'examine.");

	new slot = -1;
	for(new i = 0; i < JU_MAX_CASE; i++) { if(JuCases[i][juUsed] == 0) { slot = i; break; } }
	if(slot == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Le greffe est sature, reessaie dans un instant.");

	JuCases[slot][juUsed]           = 1;
	JuCases[slot][juPrisonerSqlid]  = PlayerInfo[playerid][pSQLID];
	JuCases[slot][juTimeAtAppeal]   = PlayerInfo[playerid][pJailTime];
	JuCases[slot][juLawyerSqlid]    = 0;
	JuCases[slot][juFee]            = 0;
	JuCases[slot][juCreatedTs]      = gettime();
	JuCases[slot][juFine]           = 0;
	format(JuCases[slot][juOfficer], MAX_PLAYER_NAME, "Police");

	msg_Client(playerid, COLOR_WHITE, "{8B008B} Justice {FFFFFF} Recours depose. Un juge va examiner ton dossier. Un avocat peut te defendre ({FFFFB2}/defendre{FFFFFF}).");

	// Previent la Cour de Justice et les avocats en service.
	new nbJuges = 0, annonce[200];
	format(annonce, sizeof(annonce), "{8B008B} Justice {FFFFFF} Nouveau recours de {FFFFB2}%s{FFFFFF} ({FFFFB2}%d{FFFFFF} sec de peine). Tape {FFFFB2}/proces{FFFFFF}.",
		PlayerInfo[playerid][pRealName], PlayerInfo[playerid][pJailTime]);
	for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
	{
		if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
		if(PlayerInfo[pp][pMember] != FACTION_JUSTICE && !Justice_IsLawyer(pp)) continue;
		msg_Client(pp, COLOR_WHITE, annonce);
		nbJuges++;
	}
	if(nbJuges == 0)
		msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Aucun magistrat n'est connecte pour l'instant : ton recours reste enregistre.");
	return 1;
}

// ============================================================================
// /defendre : un avocat se porte defenseur
// ============================================================================
// params n'est volontairement PAS const : ReturnUser() attend une chaine
// modifiable (voir sa signature dans LVRP.pwn).
stock Justice_CmdDefendre(playerid, params[])
{
	if(!Justice_IsLawyer(playerid))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Seul un avocat peut se porter defenseur (grade Avocat de la Cour, ou metier Avocat).");
	if(!strlen(params))
		return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /defendre <id/joueur>");

	new target = ReturnUser(params);
	if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Joueur introuvable.");
	if(target == playerid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Tu ne peux pas te defendre toi-meme.");

	new ci = Justice_FindCaseBySqlid(PlayerInfo[target][pSQLID]);
	if(ci == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Ce joueur n'a aucun recours en cours.");
	if(JuCases[ci][juLawyerSqlid] != 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Ce dossier a deja un avocat.");

	ju_PendingDefend[playerid] = ci;
	new body[240];
	format(body, sizeof(body), "{FFFFFF}Client : {FFFFB2}%s{FFFFFF}\nPeine restante : {FFFFB2}%d{FFFFFF} secondes\n\n{CCCCCC}Tu ne seras paye que si tu obtiens une relaxe ou une reduction.{FFFFFF}\n\nTes honoraires (0 a %d$) :",
		PlayerInfo[target][pRealName], PlayerInfo[target][pJailTime], JU_FEE_MAX);
	return ShowPlayerDialog(playerid, JU_DLG_FEE, DIALOG_STYLE_INPUT,
		"{8B008B}Se porter defenseur", body, "Valider", "Annuler");
}

// ============================================================================
// /proces : la Cour examine les recours
// ============================================================================
stock Justice_ShowCases(playerid)
{
	if(PlayerInfo[playerid][pMember] != FACTION_JUSTICE)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Reserve a la Cour de Justice.");
	if(!Justice_IsOnDuty(playerid))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Prends ton service d'abord ({FFFFB2}/j duty{FFFFFF}).");

	new body[1400], tmp[190], c = 0;
	strcat(body, "Detenu\tPeine restante\tAvocat\n");
	for(new i = 0; i < JU_MAX_CASE; i++)
	{
		if(JuCases[i][juUsed] != 1) continue;
		new pr = Justice_FindPlayerBySqlid(JuCases[i][juPrisonerSqlid]);
		new prName[MAX_PLAYER_NAME]; strcat(prName, "hors ligne");
		new reste = 0;
		if(pr != INVALID_PLAYER_ID)
		{
			GetPlayerName(pr, prName, sizeof(prName));
			reste = PlayerInfo[pr][pJailTime];
		}
		new avName[MAX_PLAYER_NAME]; strcat(avName, "aucun");
		if(JuCases[i][juLawyerSqlid] != 0)
		{
			new av = Justice_FindPlayerBySqlid(JuCases[i][juLawyerSqlid]);
			if(av != INVALID_PLAYER_ID) { avName[0] = 0; GetPlayerName(av, avName, sizeof(avName)); }
			else { avName[0] = 0; strcat(avName, "hors ligne"); }
		}
		format(tmp, sizeof(tmp), "%s\t%d sec\t{CCCCCC}%s{FFFFFF}\n", prName, reste, avName);
		strcat(body, tmp);
		ju_CaseMap[playerid][c] = i;
		c++;
	}
	if(c == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Aucun recours en attente.");

	return ShowPlayerDialog(playerid, JU_DLG_LIST, DIALOG_STYLE_TABLIST_HEADERS,
		"{8B008B}Cour de Justice - Recours", body, "Examiner", "Fermer");
}

stock Justice_ShowCase(playerid)
{
	new ci = ju_CaseSel[playerid];
	if(ci < 0 || ci >= JU_MAX_CASE || JuCases[ci][juUsed] != 1)
		return Justice_ShowCases(playerid);

	new pr = Justice_FindPlayerBySqlid(JuCases[ci][juPrisonerSqlid]);
	if(pr == INVALID_PLAYER_ID)
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Le detenu s'est deconnecte, dossier clos.");
		Justice_CloseCase(ci);
		return Justice_ShowCases(playerid);
	}

	new avName[MAX_PLAYER_NAME]; strcat(avName, "aucun (le detenu se defend seul)");
	if(JuCases[ci][juLawyerSqlid] != 0)
	{
		new av = Justice_FindPlayerBySqlid(JuCases[ci][juLawyerSqlid]);
		avName[0] = 0;
		if(av != INVALID_PLAYER_ID) GetPlayerName(av, avName, sizeof(avName));
		else strcat(avName, "hors ligne");
	}

	new body[460];
	format(body, sizeof(body),
		"{FFFFFF}Detenu : {FFFFB2}%s{FFFFFF}\nPeine restante : {FFFFB2}%d{FFFFFF} secondes\nAvocat : {FFFFB2}%s{FFFFFF}\n\n{00FF7F}Relaxe {CCCCCC}(liberation immediate)\n{FFFFFF}Reduire la peine de moitie\n{FFFFFF}Confirmer la peine\n{FF6666}Alourdir la peine de moitie",
		PlayerInfo[pr][pRealName], PlayerInfo[pr][pJailTime], avName);

	return ShowPlayerDialog(playerid, JU_DLG_CASE, DIALOG_STYLE_LIST,
		"{8B008B}Dossier - Verdict", body, "Rendre", "Retour");
}

// Verdict : 0 relaxe, 1 reduction, 2 confirmation, 3 alourdissement
stock Justice_Verdict(playerid, verdict)
{
	if(PlayerInfo[playerid][pRank] < JU_RANK_JUDGE)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Seul un Juge (grade 4+) peut rendre un verdict.");

	new ci = ju_CaseSel[playerid];
	if(ci < 0 || ci >= JU_MAX_CASE || JuCases[ci][juUsed] != 1)
		return Justice_ShowCases(playerid);

	new pr = Justice_FindPlayerBySqlid(JuCases[ci][juPrisonerSqlid]);
	if(pr == INVALID_PLAYER_ID)
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Le detenu s'est deconnecte, dossier clos.");
		Justice_CloseCase(ci);
		return Justice_ShowCases(playerid);
	}
	// Un magistrat ne juge pas son propre dossier.
	if(pr == playerid)
	{
		Justice_CloseCase(ci);
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Tu ne peux pas juger ton propre dossier.");
	}

	new avant = PlayerInfo[pr][pJailTime];
	new apres = avant;
	new verdictTxt[48];
	new bool:favorable = false;

	switch(verdict)
	{
		case 0:   // relaxe
		{
			apres = 0;
			format(verdictTxt, sizeof(verdictTxt), "RELAXE");
			favorable = true;
		}
		case 1:   // reduction de moitie
		{
			apres = avant / 2;
			if(apres < 2) apres = 2;
			format(verdictTxt, sizeof(verdictTxt), "PEINE REDUITE");
			favorable = true;
		}
		case 2:   // confirmation
		{
			format(verdictTxt, sizeof(verdictTxt), "PEINE CONFIRMEE");
		}
		case 3:   // alourdissement
		{
			apres = avant + (avant / 2);
			if(apres > JU_TIME_CAP) apres = JU_TIME_CAP;
			format(verdictTxt, sizeof(verdictTxt), "PEINE ALOURDIE");
		}
		default: return 1;
	}

	if(verdict == 0) Justice_Release(pr);
	else             PlayerInfo[pr][pJailTime] = apres;

	// Honoraires : l'avocat n'est paye qu'en cas de resultat, et seulement si
	// le client a de quoi. Sinon il travaille gratuitement, comme dans la vraie
	// vie - ca evite les avocats qui s'inscrivent partout pour encaisser.
	new avPlayer = INVALID_PLAYER_ID;
	if(JuCases[ci][juLawyerSqlid] != 0)
		avPlayer = Justice_FindPlayerBySqlid(JuCases[ci][juLawyerSqlid]);

	if(favorable && avPlayer != INVALID_PLAYER_ID && JuCases[ci][juFee] > 0)
	{
		new fee = JuCases[ci][juFee];
		if(PlayerInfo[pr][pCash] >= fee)
		{
			SafeGivePlayerMoney(pr, -fee, "Honoraires avocat");
			SafeGivePlayerMoney(avPlayer, fee, "Honoraires avocat");
			new feeMsg[190];
			format(feeMsg, sizeof(feeMsg), "{8B008B} Justice {FFFFFF} Honoraires percus : {FFFFB2}%d${FFFFFF} de {FFFFB2}%s{FFFFFF}.", fee, PlayerInfo[pr][pRealName]);
			msg_Client(avPlayer, COLOR_WHITE, feeMsg);
			format(feeMsg, sizeof(feeMsg), "{8B008B} Justice {FFFFFF} Honoraires verses a ton avocat : {FFFFB2}-%d${FFFFFF}.", fee);
			msg_Client(pr, COLOR_WHITE, feeMsg);
		}
		else
		{
			msg_Client(avPlayer, COLOR_INFO, "{CF9756} Justice {FFFFFF} Ton client n'a pas de quoi payer tes honoraires.");
			msg_Client(pr, COLOR_INFO, "{CF9756} Justice {FFFFFF} Tu n'avais pas de quoi payer ton avocat.");
		}
	}

	// Notifications
	new msgJuge[220];
	format(msgJuge, sizeof(msgJuge), "{8B008B} Justice {FFFFFF} Verdict rendu : {FFFFB2}%s{FFFFFF} pour {FFFFB2}%s{FFFFFF} (%d sec -> %d sec).",
		verdictTxt, PlayerInfo[pr][pRealName], avant, apres);
	msg_Client(playerid, COLOR_WHITE, msgJuge);

	new msgPr[220];
	format(msgPr, sizeof(msgPr), "{8B008B} Justice {FFFFFF} Le juge {FFFFB2}%s{FFFFFF} a rendu son verdict : {FFFFB2}%s{FFFFFF}.",
		PlayerInfo[playerid][pRealName], verdictTxt);
	msg_Client(pr, COLOR_WHITE, msgPr);

	if(avPlayer != INVALID_PLAYER_ID && avPlayer != pr)
	{
		new msgAv[220];
		format(msgAv, sizeof(msgAv), "{8B008B} Justice {FFFFFF} Verdict sur ton dossier ({FFFFB2}%s{FFFFFF}) : {FFFFB2}%s{FFFFFF}.",
			PlayerInfo[pr][pRealName], verdictTxt);
		msg_Client(avPlayer, COLOR_WHITE, msgAv);
	}

	// Annonce publique : un proces, ca se sait. C'est ce qui rend la Justice
	// visible aux yeux du reste du serveur.
	new annonce[220];
	format(annonce, sizeof(annonce), "[Cour de Justice] %s : %s (juge %s)",
		PlayerInfo[pr][pRealName], verdictTxt, PlayerInfo[playerid][pRealName]);
	msg_OOC(0x8B008BFF, annonce);

	Justice_CloseCase(ci);
	return Justice_ShowCases(playerid);
}

// Dispatcher dialog
stock Justice_OnDialogResponse(playerid, dialogid, response, listitem, const inputtext[])
{
	if(dialogid == JU_DLG_LIST)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= JU_MAX_CASE) return 1;
		ju_CaseSel[playerid] = ju_CaseMap[playerid][listitem];
		return Justice_ShowCase(playerid);
	}
	if(dialogid == JU_DLG_CASE)
	{
		if(!response) return Justice_ShowCases(playerid);
		return Justice_Verdict(playerid, listitem);
	}
	if(dialogid == JU_DLG_FEE)
	{
		if(!response) { ju_PendingDefend[playerid] = -1; return 1; }
		new ci = ju_PendingDefend[playerid];
		ju_PendingDefend[playerid] = -1;
		if(ci < 0 || ci >= JU_MAX_CASE || JuCases[ci][juUsed] != 1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Ce dossier n'existe plus.");
		if(JuCases[ci][juLawyerSqlid] != 0)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Justice {FFFFFF} Un autre avocat a pris le dossier entre-temps.");

		new fee = strval(inputtext);
		if(fee < 0 || fee > JU_FEE_MAX)
		{
			new errStr[140];
			format(errStr, sizeof(errStr), "{CF9756} Justice {FFFFFF} Honoraires invalides (entre 0$ et %d$).", JU_FEE_MAX);
			return msg_Client(playerid, COLOR_INFO, errStr);
		}

		JuCases[ci][juLawyerSqlid] = PlayerInfo[playerid][pSQLID];
		JuCases[ci][juFee]         = fee;

		new pr = Justice_FindPlayerBySqlid(JuCases[ci][juPrisonerSqlid]);
		new okStr[200];
		format(okStr, sizeof(okStr), "{8B008B} Justice {FFFFFF} Tu defends ce dossier. Honoraires : {FFFFB2}%d${FFFFFF} (payes uniquement en cas de succes).", fee);
		msg_Client(playerid, COLOR_WHITE, okStr);
		if(pr != INVALID_PLAYER_ID)
		{
			new prMsg[220];
			format(prMsg, sizeof(prMsg), "{8B008B} Justice {FFFFFF} Maitre {FFFFB2}%s{FFFFFF} prend ta defense (honoraires : {FFFFB2}%d${FFFFFF}, dus seulement s'il obtient gain de cause).",
				PlayerInfo[playerid][pRealName], fee);
			msg_Client(pr, COLOR_WHITE, prMsg);
		}
		return 1;
	}
	return 0;
}
