/*
	main/commands.pwn - adapte pour LVRP inline
	Les CMD: Pawn.CMD ont ete convertis en stock fonctions appelees depuis le
	OnPlayerCommandText de LVRP.pwn (voir bloc dans LVRP.pwn).
*/

stock Mobile_CmdMobile(playerid)
{
	if(hasPhone[playerid] == 0)
	{
		SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[Telephone]{FFFFFF} Vous n'avez pas de telephone.");
		SendClientMessage(playerid, 0xFFFFFFFF, "{FFFFFF}Allez voir l'icone telephone sur la carte (3 marches dispo) et tapez {00FF00}/shop");
		return 1;
	}
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);

	if(usingPhone[playerid] == 0)
	{
		// [FIX] Le record `users` peut manquer (joueur venu de lvrp_users) : il
		// porte le fond d'ecran et le cadre, donc on le cree au premier usage.
		new sqlBuf[200];
		mysql_format(db_handle, sqlBuf, sizeof(sqlBuf),
			"INSERT IGNORE INTO `users` (`Username`, `HasPhone`, `Number`, `Credit`, `Frame`, `Background`) VALUES ('%e', 1, %d, %d, 0, 0)",
			GetName(playerid), playerNumber[playerid], playerCredit[playerid]);
		mysql_tquery(db_handle, sqlBuf);

		// [BUDGET TEXTDRAWS] Les 165 elements des apps naissent ici et meurent
		// a la fermeture ; CreateTextDraws se protege lui-meme du double appel.
		CreateTextDraws(playerid);

		SelectTextDraw(playerid, SELECTION_COLOR);
		UseMobile(playerid, HOME, SHOW);
		UseMobile(playerid, NOAPPS, SHOW);
		UpdateTimeDate(playerid, 2);

		usingPhone[playerid] = 1;
	}
	else
	{
		CancelSelectTextDraw(playerid);
		// Destroy et pas Hide : masquer ne rend pas le slot au client, et c'est
		// ce qui empechait le sac / le craft / le HUD de s'afficher.
		DestroyTextDraws(playerid);

		usingPhone[playerid] = 0;
	}
	return 1;
}

stock Mobile_CmdShop(playerid)
{
	if(!IsPlayerNearMarket(playerid)) return SendPlayerNotification(playerid, -1, NOT_IN_MARKET);
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);

	OnPlayerEnterShop(playerid, BUYING);
	return 1;
}

stock Mobile_CmdMobileSettings(playerid)
{
	if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE);
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);
	if(usingPhone[playerid] == 0) return SendClientMessage(playerid, -1, "Allumez d'abord le telephone.");
	if(playerOccupied[playerid] == 1) return SendClientMessage(playerid, -1, "Vous devez etre hors appel.");

	ShowPlayerDialog(playerid, SETTINGS_DIALOG, DIALOG_STYLE_LIST, "Reglages",
		"Fond - Ciel etoile\nFond - 2-Color\nFond - Ciel\nCadre - Defaut\nCadre - Bleu\nCadre - Rouge\nCadre - Vert\nCadre - Jaune\nCadre - Violet\nCadre - Rose",
		"Choisir", CANCEL);
	return 1;
}
