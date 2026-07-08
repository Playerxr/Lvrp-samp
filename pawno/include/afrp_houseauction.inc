/*
	afrp_houseauction.inc - Systeme d'enchere sur les maisons

	Commande : /enchere <montant>
	Le joueur doit etre a cote d'une maison non possedee (meme detection
	que /maison acheter). La premiere enchere demarre un compte a rebours
	affiche au-dessus de la maison via un texte 3D (prix actuel + temps
	restant). Les autres joueurs peuvent surenchir avant la fin du delai
	(montant strictement superieur a HOUSEAUCTION_MIN_INCREMENT pres).

	L'argent est bloque (escrow) des la mise :
	- si un autre joueur surenchit, l'ancien encherisseur est rembourse
	  immediatement (en jeu s'il est connecte, sinon via une mise a jour
	  SQL directe sur son compte).
	- a la fin du delai, le meilleur encherisseur remporte la maison. S'il
	  s'est deconnecte entre temps, il est rembourse et la maison redevient
	  disponible (pas de transfert a l'aveugle a un joueur absent).

	L'achat instantane (/maison acheter) reste bloque tant qu'une enchere
	est en cours sur la maison (voir garde-fou dans /ma(ison) acheter).
*/

#if defined AFRP_HOUSEAUCTION_INCLUDED
	#endinput
#endif
#define AFRP_HOUSEAUCTION_INCLUDED

#define HOUSEAUCTION_DURATION       300   // secondes (5 min) avant resolution
#define HOUSEAUCTION_MIN_INCREMENT  500   // increment minimum entre deux encheres
#define HOUSEAUCTION_REFRESH        5     // rafraichit le texte 3D toutes les X secondes

new houseAuction_Active[MAX_HOUSE];
new houseAuction_TimeLeft[MAX_HOUSE];
new houseAuction_CurrentBid[MAX_HOUSE];
new houseAuction_BidderSQLID[MAX_HOUSE];
new houseAuction_BidderName[MAX_HOUSE][MAX_PLAYER_NAME];
new Text3D:houseAuction_Label[MAX_HOUSE];

forward HouseAuction_Tick();

stock HouseAuction_Init()
{
	for(new i = 0; i < MAX_HOUSE; i++)
	{
		houseAuction_Label[i] = Text3D:INVALID_3DTEXT_ID;
	}
	SetTimer("HouseAuction_Tick", 1000, true);
	return 1;
}

// Meme detection que /maison acheter : maison non possedee a moins de 3 unites
stock HouseAuction_FindNearby(playerid)
{
	for(new i = 0; i < totalHouses; i++)
	{
		if(house[i][owned] == 1) continue;
		if(PlayerToPoint(3, playerid, house[i][pos][0], house[i][pos][1], house[i][pos][2]))
			return i;
	}
	return -1;
}

stock HouseAuction_UpdateLabel(hid)
{
	new mins = houseAuction_TimeLeft[hid] / 60;
	new secs = houseAuction_TimeLeft[hid] % 60;
	new labelStr[160];
	format(labelStr, sizeof(labelStr),
		"{FFD700}ENCHERE EN COURS{FFFFFF}\nPrix actuel : $%d\nPar : %s\nTemps restant : %d:%02d\n{CCCCCC}/enchere <montant> pour surenchir",
		houseAuction_CurrentBid[hid], houseAuction_BidderName[hid], mins, secs);

	if(_:houseAuction_Label[hid] != _:INVALID_3DTEXT_ID)
	{
		Update3DTextLabelText(houseAuction_Label[hid], 0xFFD700FF, labelStr);
	}
	else
	{
		houseAuction_Label[hid] = Create3DTextLabel(labelStr, 0xFFD700FF,
			house[hid][pos][0], house[hid][pos][1], house[hid][pos][2] + 1.2, 20.0, 0, 0);
	}
	return 1;
}

// Rembourse l'encherisseur actuel de la maison hid (en ligne ou hors ligne)
stock HouseAuction_RefundCurrent(hid)
{
	new prevSQLID = houseAuction_BidderSQLID[hid];
	new prevAmount = houseAuction_CurrentBid[hid];
	if(prevAmount <= 0) return 1;

	new found = -1;
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pSQLID] == prevSQLID) { found = i; break; }
	}

	if(found != -1)
	{
		SafeGivePlayerMoney(found, prevAmount, "Rembourse enchere maison (surenchere)");
		msg_Client(found, COLOR_HOUSE, "{00C600} Maison {FFFFFF} Vous avez ete surenchri, votre argent vous a ete rembourse.");
	}
	else
	{
		new sql[128];
		format(sql, sizeof(sql), "UPDATE lvrp_users SET Cash=Cash+%d WHERE id=%d", prevAmount, prevSQLID);
		mysql_tquery(MYSQL, sql, "", "");
	}
	return 1;
}

stock HouseAuction_Reset(hid)
{
	houseAuction_Active[hid] = 0;
	houseAuction_TimeLeft[hid] = 0;
	houseAuction_CurrentBid[hid] = 0;
	houseAuction_BidderSQLID[hid] = 0;
	houseAuction_BidderName[hid][0] = EOS;
	if(_:houseAuction_Label[hid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(houseAuction_Label[hid]);
		houseAuction_Label[hid] = Text3D:INVALID_3DTEXT_ID;
	}
	return 1;
}

stock HouseAuction_Resolve(hid)
{
	new winnerSQLID = houseAuction_BidderSQLID[hid];
	new winAmount = houseAuction_CurrentBid[hid];
	new winnerName[MAX_PLAYER_NAME];
	strmid(winnerName, houseAuction_BidderName[hid], 0, MAX_PLAYER_NAME, MAX_PLAYER_NAME);

	new winner = -1;
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pSQLID] == winnerSQLID) { winner = i; break; }
	}

	if(winner != -1)
	{
		if(PlayerInfo[winner][pHousekey1] == -1) { PlayerInfo[winner][pHousekey1] = hid; }
		else if(PlayerInfo[winner][pHousekey2] == -1) { PlayerInfo[winner][pHousekey2] = hid; }
		else if(PlayerInfo[winner][pHousekey3] == -1) { PlayerInfo[winner][pHousekey3] = hid; }
		else
		{
			// Cas rare : le joueur a obtenu 3 maisons entre-temps -> rembourse, pas de transfert
			SafeGivePlayerMoney(winner, winAmount, "Rembourse enchere (3 maisons max)");
			msg_Client(winner, COLOR_HOUSE, "{00C600} Maison {FFFFFF} Vous avez deja 3 maisons, votre enchere a ete remboursee.");
			HouseAuction_Reset(hid);
			return 1;
		}
		house[hid][owned] = 1;
		strmid(house[hid][owner], winnerName, 0, strlen(winnerName), MAX_PLAYER_NAME);
		house_Save(hid);
		house_UpdateInfos(hid);
		msg_Client(winner, COLOR_HOUSE, "{00C600} Maison {FFFFFF} Felicitations, vous avez remporte l'enchere ! (aide: /ma(ison))");
	}
	else
	{
		// Le gagnant s'est deconnecte : rembourse via SQL, la maison redevient disponible
		new sql[128];
		format(sql, sizeof(sql), "UPDATE lvrp_users SET Cash=Cash+%d WHERE id=%d", winAmount, winnerSQLID);
		mysql_tquery(MYSQL, sql, "", "");
		printf("[ENCHERE] Maison id %d : gagnant deconnecte (SQLID %d), enchere annulee et remboursee.", hid, winnerSQLID);
	}

	HouseAuction_Reset(hid);
	return 1;
}

public HouseAuction_Tick()
{
	for(new hid = 0; hid < totalHouses; hid++)
	{
		if(!houseAuction_Active[hid]) continue;
		houseAuction_TimeLeft[hid]--;
		if(houseAuction_TimeLeft[hid] <= 0)
		{
			HouseAuction_Resolve(hid);
			continue;
		}
		if(houseAuction_TimeLeft[hid] % HOUSEAUCTION_REFRESH == 0)
			HouseAuction_UpdateLabel(hid);
	}
	return 1;
}

stock HouseAuction_Cmd(playerid, montant)
{
	if(PlayerInfo[playerid][pIdentie] == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous devez avoir une carte d'identite pour encherir.");
	if(PlayerInfo[playerid][pHousekey1] != -1 && PlayerInfo[playerid][pHousekey2] != -1 && PlayerInfo[playerid][pHousekey3] != -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous avez deja 3 maisons.");

	new hid = HouseAuction_FindNearby(playerid);
	if(hid == -1)
		return msg_Client(playerid, COLOR_HOUSE, "{00C600} Maison {FFFFFF} Vous n'etes pas a cote d'une maison a vendre.");

	if(montant <= 0)
		return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /enchere <montant>");

	new minBid = houseAuction_Active[hid] ? (houseAuction_CurrentBid[hid] + HOUSEAUCTION_MIN_INCREMENT) : house[hid][price];
	if(montant < minBid)
	{
		new msg[128];
		format(msg, sizeof(msg), "{CF9756} Info {FFFFFF} Enchere minimum : $%d.", minBid);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	if(PlayerInfo[playerid][pCash] < montant)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous n'avez pas assez d'argent.");

	if(houseAuction_Active[hid])
		HouseAuction_RefundCurrent(hid);

	SafeGivePlayerMoney(playerid, -montant, "Enchere maison");
	houseAuction_Active[hid] = 1;
	houseAuction_TimeLeft[hid] = HOUSEAUCTION_DURATION;
	houseAuction_CurrentBid[hid] = montant;
	houseAuction_BidderSQLID[hid] = PlayerInfo[playerid][pSQLID];
	strmid(houseAuction_BidderName[hid], PlayerInfo[playerid][pRealName], 0, strlen(PlayerInfo[playerid][pRealName]), MAX_PLAYER_NAME);

	HouseAuction_UpdateLabel(hid);

	new announceMsg[144];
	format(announceMsg, sizeof(announceMsg), "{FFD700}[Enchere]{FFFFFF} %s encherit $%d sur une maison !", PlayerInfo[playerid][pRealName], montant);
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerToPoint(30.0, i, house[hid][pos][0], house[hid][pos][1], house[hid][pos][2]))
			msg_Client(i, COLOR_HOUSE, announceMsg);
	}
	return 1;
}
