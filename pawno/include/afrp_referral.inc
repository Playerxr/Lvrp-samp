/*
	afrp_referral.inc - Systeme de parrainage

	Quand un nouveau joueur s'inscrit, on lui demande qui l'a parraine
	(format strict Prenom_Nom, ex: Junior_Seka). Si le nom correspond a un
	compte existant, le PARRAIN recoit une recompense en argent (en jeu
	s'il est connecte, sinon credite directement en base pour sa prochaine
	connexion).

	Etape d'inscription ajoutee : inscription_Step[playerid] == 11
	(injectee entre la validation de l'email et la creation du compte).

	Table : lvrp_referrals (ReferrerSQLID, ReferredSQLID, ReferredName, Date)
	Commande : /parrainage -> stats persos (nb de parrainages, total gagne)
*/

#if defined AFRP_REFERRAL_INCLUDED
	#endinput
#endif
#define AFRP_REFERRAL_INCLUDED

#define REFERRAL_REWARD     5000   // Montant gagne par le parrain a chaque parrainage valide
// [IMPORTANT] Tout le flux d'inscription (LVRP.pwn) multiplexe via UN SEUL
// dialogid (81) + la variable inscription_Step pour distinguer les etapes.
// Il FAUT reutiliser ce meme dialogid ici, sinon OnDialogResponse ne route
// jamais vers le bloc "if(inscription_Step[playerid] == 11)".
#define DIALOG_REFERRAL     81

new pendingReferrerSQLID[MAX_PLAYERS]; // 0 = aucun parrain en attente de validation

stock Referral_Init()
{
	mysql_tquery(MYSQL,
		"CREATE TABLE IF NOT EXISTS `lvrp_referrals` (\
			`id` INT AUTO_INCREMENT PRIMARY KEY,\
			`ReferrerSQLID` INT NOT NULL,\
			`ReferredSQLID` INT NOT NULL,\
			`ReferredName` VARCHAR(32) NOT NULL DEFAULT '',\
			`Date` INT NOT NULL DEFAULT 0\
		) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4", "", "");
	return 1;
}

// Verifie le format strict "Prenom_Nom" : un seul '_', 2 parties non vides,
// lettres uniquement de part et d'autre (meme convention que les pseudos du serveur).
stock bool:Referral_IsValidNameFormat(const name[])
{
	new len = strlen(name);
	if(len < 3 || len > 32) return false;

	new underscoreIdx = -1, underscoreCount = 0;
	for(new i = 0; i < len; i++)
	{
		if(name[i] == '_')
		{
			underscoreCount++;
			underscoreIdx = i;
		}
		else if(!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z')))
		{
			return false; // caractere non-lettre et non-underscore
		}
	}
	if(underscoreCount != 1) return false;       // exactement un underscore
	if(underscoreIdx == 0 || underscoreIdx == len - 1) return false; // pas en debut/fin
	return true;
}

stock Referral_AskName(playerid)
{
	new infostring[200];
	format(infostring, sizeof(infostring),
		"{FFFFFF}Qui vous a parraine ? {8C8C8C}(Prenom_Nom, ex: Junior_Seka)\n{AAAAAA}Laissez vide si personne ne vous a parraine.");
	ShowPlayerDialog(playerid, DIALOG_REFERRAL, DIALOG_STYLE_INPUT,
		"{33CCFF}\xbb Inscription \xab{FFFFFF} Parrainage", infostring, "Valider", "Passer");
	return 1;
}

// Callback : verifie si le nom du parrain existe en base
forward Referral_OnCheckName(playerid);
public Referral_OnCheckName(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new count = 0;
	cache_get_row_count(count);
	if(count > 0)
	{
		new refSqlId = 0;
		cache_get_value_name_int(0, "id", refSqlId);
		pendingReferrerSQLID[playerid] = refSqlId;
		Referral_Finalize(playerid);
	}
	else
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Parrainage]{FFFFFF} Ce joueur n'existe pas. Verifiez l'orthographe (Prenom_Nom).");
		Referral_AskName(playerid);
	}
	return 1;
}

// Termine l'inscription (cree le compte) puis applique la recompense de
// parrainage si un parrain valide a ete trouve.
stock Referral_Finalize(playerid)
{
	server_HideIntroTexts(playerid);
	inscription_Step[playerid] = 10;
	server_TogglePlayerSpectating(playerid, 0);
	TimerConnectOff(playerid);
	server_SetPlayerVirtualWorld(playerid, 0);
	server_SetPlayerInterior(playerid, 0);
	OnPlayerRegister(playerid, PlayerInfo[playerid][pKey]);
	msg_Client(playerid, COLOR_INFO, "{FFAA00}[AFRP]{FFFFFF} Compte cree avec succes ! Bienvenue a Los Santos.");
	msg_Client(playerid, COLOR_INFO, "{FFAA00}[AFRP]{FFFFFF} Changez votre sexe/ville de spawn a la mairie.");

	if(pendingReferrerSQLID[playerid] > 0)
	{
		new refSqlId = pendingReferrerSQLID[playerid];
		pendingReferrerSQLID[playerid] = 0;

		// Log du parrainage
		new sqlbuf[200];
		mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
			"INSERT INTO `lvrp_referrals` (ReferrerSQLID, ReferredSQLID, ReferredName, Date) VALUES (%d, %d, '%e', UNIX_TIMESTAMP())",
			refSqlId, PlayerInfo[playerid][pSQLID], PlayerInfo[playerid][pRealName]);
		mysql_tquery(MYSQL, sqlbuf);

		// Le parrain est-il connecte maintenant ?
		new bool:found = false;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			if(PlayerInfo[i][pSQLID] == refSqlId)
			{
				GivePlayerMoney(i, REFERRAL_REWARD);
				PlayerInfo[i][pCash] += REFERRAL_REWARD;
				new rmsg[180];
				format(rmsg, sizeof(rmsg),
					"{00FF00}[Parrainage]{FFFFFF} %s s'est inscrit grace a vous ! +{FFFF00}%d $",
					PlayerInfo[playerid][pRealName], REFERRAL_REWARD);
				SendClientMessage(i, -1, rmsg);
				found = true;
				break;
			}
		}
		// Sinon, credite directement en DB pour la prochaine connexion
		if(!found)
		{
			mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
				"UPDATE lvrp_users SET Cash = Cash + %d WHERE id = %d",
				REFERRAL_REWARD, refSqlId);
			mysql_tquery(MYSQL, sqlbuf);
		}
	}
	return 1;
}

// Dispatch depuis OnDialogResponse pour le step 11 (saisie du nom du parrain)
stock Referral_OnDialogResponse(playerid, response, inputtext[])
{
	if(!response || !strlen(inputtext))
	{
		// Pas de parrain -> finalise directement
		pendingReferrerSQLID[playerid] = 0;
		Referral_Finalize(playerid);
		return 1;
	}
	if(!Referral_IsValidNameFormat(inputtext))
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Parrainage]{FFFFFF} Format invalide. Utilisez Prenom_Nom (ex: Junior_Seka).");
		Referral_AskName(playerid);
		return 1;
	}
	if(strcmp(inputtext, PlayerInfo[playerid][pRealName], true) == 0)
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Parrainage]{FFFFFF} Vous ne pouvez pas vous parrainer vous-meme.");
		Referral_AskName(playerid);
		return 1;
	}
	new sqlbuf[120];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf), "SELECT id FROM lvrp_users WHERE Name='%e' LIMIT 1", inputtext);
	mysql_tquery(MYSQL, sqlbuf, "Referral_OnCheckName", "d", playerid);
	return 1;
}

// /parrainage : stats persos
stock Referral_CmdStats(playerid)
{
	new sqlbuf[150];
	mysql_format(MYSQL, sqlbuf, sizeof(sqlbuf),
		"SELECT COUNT(*) AS nb FROM lvrp_referrals WHERE ReferrerSQLID=%d",
		PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, sqlbuf, "Referral_OnStatsResult", "d", playerid);
	return 1;
}

forward Referral_OnStatsResult(playerid);
public Referral_OnStatsResult(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new nb = 0;
	new count = 0;
	cache_get_row_count(count);
	if(count > 0) cache_get_value_name_int(0, "nb", nb);
	new msg[200];
	format(msg, sizeof(msg),
		"{FFAA00}[Parrainage]{FFFFFF} Vous avez parraine {00FF00}%d{FFFFFF} joueur(s), soit {00FF00}%d $ {FFFFFF}gagnes au total.\n{8C8C8C}Donnez votre pseudo (Prenom_Nom) aux nouveaux pour gagner %d$ par inscription !",
		nb, nb * REFERRAL_REWARD, REFERRAL_REWARD);
	SendClientMessage(playerid, -1, msg);
	return 1;
}
