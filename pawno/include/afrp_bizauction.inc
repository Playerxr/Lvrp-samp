/*
	afrp_bizauction.inc - Systeme d'enchere sur les biz (commerces)

	Meme principe que afrp_houseauction.inc, applique aux commerces
	(bizz[]) : /enchere <montant> pres d'un biz non possede demarre ou
	fait progresser une enchere (texte 3D avec prix actuel + temps
	restant). L'argent est bloque (escrow) des la mise, rembourse si
	surenchri, et le meilleur encherisseur remporte le biz a la fin du
	delai (ou est rembourse s'il s'est deconnecte entre-temps).

	L'achat instantane (/biz acheter) reste bloque tant qu'une enchere
	est en cours sur le biz.
*/

#if defined AFRP_BIZAUCTION_INCLUDED
	#endinput
#endif
#define AFRP_BIZAUCTION_INCLUDED

#define BIZAUCTION_DURATION       300   // secondes (5 min) avant resolution
#define BIZAUCTION_MIN_INCREMENT  500   // increment minimum entre deux encheres
#define BIZAUCTION_REFRESH        5     // rafraichit le texte 3D toutes les X secondes

new bizAuction_Active[MAX_BIZZ];
new bizAuction_TimeLeft[MAX_BIZZ];
new bizAuction_CurrentBid[MAX_BIZZ];
new bizAuction_BidderSQLID[MAX_BIZZ];
new bizAuction_BidderName[MAX_BIZZ][MAX_PLAYER_NAME];
new Text3D:bizAuction_Label[MAX_BIZZ];

forward BizAuction_Tick();

stock BizAuction_Init()
{
	for(new i = 0; i < MAX_BIZZ; i++)
	{
		bizAuction_Label[i] = Text3D:INVALID_3DTEXT_ID;
		bizAuction_BidderSQLID[i] = -1;
	}
	SetTimer("BizAuction_Tick", 1000, true);
	return 1;
}

// Meme detection que /biz acheter : biz non possede a moins de 3 unites
stock BizAuction_FindNearby(playerid)
{
	for(new i = 0; i < totalBizz; i++)
	{
		if(bizz[i][owned] == 1) continue;
		if(PlayerToPoint(3, playerid, bizz[i][pos][0], bizz[i][pos][1], bizz[i][pos][2]))
			return i;
	}
	return -1;
}

stock BizAuction_UpdateLabel(bid)
{
	new mins = bizAuction_TimeLeft[bid] / 60;
	new secs = bizAuction_TimeLeft[bid] % 60;
	new bidStr[16];
	FormatMoney(bizAuction_CurrentBid[bid], bidStr, 16);
	new labelStr[160];
	format(labelStr, sizeof(labelStr),
		"{FFD700}ENCHERE EN COURS{FFFFFF}\nPrix actuel : $%s\nPar : %s\nTemps restant : %d:%02d\n{CCCCCC}/enchere <montant> pour surenchir",
		bidStr, bizAuction_BidderName[bid], mins, secs);

	if(_:bizAuction_Label[bid] != _:INVALID_3DTEXT_ID)
	{
		Update3DTextLabelText(bizAuction_Label[bid], 0xFFD700FF, labelStr);
	}
	else
	{
		bizAuction_Label[bid] = Create3DTextLabel(labelStr, 0xFFD700FF,
			bizz[bid][pos][0], bizz[bid][pos][1], bizz[bid][pos][2] + 1.2, 20.0, 0, 0);
	}
	return 1;
}

stock BizAuction_RefundCurrent(bid)
{
	new prevSQLID = bizAuction_BidderSQLID[bid];
	new prevAmount = bizAuction_CurrentBid[bid];
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
		SafeGivePlayerMoney(found, prevAmount, "Rembourse enchere biz (surenchere)");
		msg_Client(found, COLOR_BIZ, "{2B6AFF} Biz {FFFFFF} Vous avez ete surenchri, votre argent vous a ete rembourse.");
	}
	else
	{
		new sql[128];
		format(sql, sizeof(sql), "UPDATE lvrp_users SET Cash=Cash+%d WHERE id=%d", prevAmount, prevSQLID);
		mysql_tquery(MYSQL, sql, "", "");
	}
	return 1;
}

stock BizAuction_Reset(bid)
{
	bizAuction_Active[bid] = 0;
	bizAuction_TimeLeft[bid] = 0;
	bizAuction_CurrentBid[bid] = 0;
	bizAuction_BidderSQLID[bid] = -1;
	bizAuction_BidderName[bid][0] = EOS;
	if(_:bizAuction_Label[bid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(bizAuction_Label[bid]);
		bizAuction_Label[bid] = Text3D:INVALID_3DTEXT_ID;
	}
	return 1;
}

stock BizAuction_Resolve(bid)
{
	if(bizAuction_BidderSQLID[bid] == -1)
	{
		printf("[ENCHERE BIZ] Biz id %d : enchere terminee sans aucune offre.", bid);
		BizAuction_Reset(bid);
		return 1;
	}

	new winnerSQLID = bizAuction_BidderSQLID[bid];
	new winAmount = bizAuction_CurrentBid[bid];
	new winnerName[MAX_PLAYER_NAME];
	strmid(winnerName, bizAuction_BidderName[bid], 0, MAX_PLAYER_NAME, MAX_PLAYER_NAME);

	new winner = -1;
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pSQLID] == winnerSQLID) { winner = i; break; }
	}

	if(winner != -1)
	{
		if(PlayerInfo[winner][pBizkey1] == -1) { PlayerInfo[winner][pBizkey1] = bid; }
		else if(PlayerInfo[winner][pBizkey2] == -1) { PlayerInfo[winner][pBizkey2] = bid; }
		else if(PlayerInfo[winner][pBizkey3] == -1) { PlayerInfo[winner][pBizkey3] = bid; }
		else
		{
			SafeGivePlayerMoney(winner, winAmount, "Rembourse enchere (3 biz max)");
			msg_Client(winner, COLOR_BIZ, "{2B6AFF} Biz {FFFFFF} Vous avez deja 3 biz, votre enchere a ete remboursee.");
			BizAuction_Reset(bid);
			return 1;
		}
		bizz[bid][owned] = 1;
		bizz[bid][lock] = 0;
		strmid(bizz[bid][owner], winnerName, 0, strlen(winnerName), MAX_PLAYER_NAME);
		bizz_Save(bid);
		bizz_UpdateInfos(bid);
		msg_Client(winner, COLOR_BIZ, "{2B6AFF} Biz {FFFFFF} Felicitations, vous avez remporte l'enchere ! (aide: /biz type et /biz gestion)");
	}
	else
	{
		new sql[128];
		format(sql, sizeof(sql), "UPDATE lvrp_users SET Cash=Cash+%d WHERE id=%d", winAmount, winnerSQLID);
		mysql_tquery(MYSQL, sql, "", "");
		printf("[ENCHERE BIZ] Biz id %d : gagnant deconnecte (SQLID %d), enchere annulee et remboursee.", bid, winnerSQLID);
	}

	BizAuction_Reset(bid);
	return 1;
}

public BizAuction_Tick()
{
	for(new bid = 0; bid < totalBizz; bid++)
	{
		if(!bizAuction_Active[bid]) continue;
		bizAuction_TimeLeft[bid]--;
		if(bizAuction_TimeLeft[bid] <= 0)
		{
			BizAuction_Resolve(bid);
			continue;
		}
		if(bizAuction_TimeLeft[bid] % BIZAUCTION_REFRESH == 0)
			BizAuction_UpdateLabel(bid);
	}
	return 1;
}

// Renvoie 0 si aucun biz a proximite (laisse la main a HouseAuction_Cmd via
// la commande /enchere partagee), sinon traite l'enchere et renvoie 1.
stock BizAuction_Cmd(playerid, montant)
{
	new bid = BizAuction_FindNearby(playerid);
	if(bid == -1)
		return 0;

	if(PlayerInfo[playerid][pIdentie] == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous devez avoir une carte d'identite pour encherir.");
	if(PlayerInfo[playerid][pBizkey1] != -1 && PlayerInfo[playerid][pBizkey2] != -1 && PlayerInfo[playerid][pBizkey3] != -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous avez deja 3 biz.");

	if(montant <= 0)
		return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /enchere <montant>");

	new minBid = (bizAuction_Active[bid] && bizAuction_CurrentBid[bid] > 0)
		? (bizAuction_CurrentBid[bid] + BIZAUCTION_MIN_INCREMENT)
		: bizz[bid][price];
	if(montant < minBid)
	{
		new msg[128], minBidStr[16];
		FormatMoney(minBid, minBidStr, 16);
		format(msg, sizeof(msg), "{CF9756} Info {FFFFFF} Enchere minimum : $%s.", minBidStr);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	if(PlayerInfo[playerid][pCash] < montant)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Vous n'avez pas assez d'argent.");

	if(bizAuction_Active[bid])
		BizAuction_RefundCurrent(bid);

	SafeGivePlayerMoney(playerid, -montant, "Enchere biz");
	bizAuction_Active[bid] = 1;
	bizAuction_TimeLeft[bid] = BIZAUCTION_DURATION;
	bizAuction_CurrentBid[bid] = montant;
	bizAuction_BidderSQLID[bid] = PlayerInfo[playerid][pSQLID];
	strmid(bizAuction_BidderName[bid], PlayerInfo[playerid][pRealName], 0, strlen(PlayerInfo[playerid][pRealName]), MAX_PLAYER_NAME);

	BizAuction_UpdateLabel(bid);

	new announceMsg[144], montantStr[16];
	FormatMoney(montant, montantStr, 16);
	format(announceMsg, sizeof(announceMsg), "{FFD700}[Enchere]{FFFFFF} %s encherit $%s sur un biz !", PlayerInfo[playerid][pRealName], montantStr);
	for(new i = 0; i < MAX_PLAYERS_CURRENT + 1; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerToPoint(30.0, i, bizz[bid][pos][0], bizz[bid][pos][1], bizz[bid][pos][2]))
			msg_Client(i, COLOR_BIZ, announceMsg);
	}
	return 1;
}
