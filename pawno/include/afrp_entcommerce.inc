/*
	afrp_entcommerce.inc - Commerce de marchandises entre entreprises et biz

	Complete les missions cargo (afrp_cargomission.inc) : au lieu de seulement
	transporter, une entreprise peut acheter du stock puis le REVENDRE, soit a
	un biz (qui a besoin de produits pour tourner), soit a une autre entreprise.

	Boucle economique :
	  1. L'entreprise achete des marchandises en gros    (caisse -> stock)
	  2. Elle les revend a un biz proche                 (stock -> caisse, +marge)
	     ou a une autre entreprise                       (stock -> caisse)

	Cote biz, c'est un vrai service : le proprietaire pouvait deja acheter des
	produits depuis sa propre caisse (dialog 98), mais au prix prodCost fixe par
	le biz. Passer par une entreprise devient une alternative RP, et surtout ca
	donne une raison d'exister aux entreprises non-transport.

	Depend de afrp_entreprise.inc (inclus AVANT) : entreprises[], entCash,
	entName, Ent_FindByOwner, Ent_FindEmployeeSlot, Ent_Save, Ent_FindByName.
	Depend aussi de bizz[] (declare dans LVRP.pwn avant les includes).

	Le stock est sauvegarde avec l'entreprise (champ ajoute en fin de ligne dans
	entreprises.cfg, voir Ent_Load/Ent_Save).
*/

#if defined AFRP_ENTCOMMERCE_INCLUDED
	#endinput
#endif
#define AFRP_ENTCOMMERCE_INCLUDED

#define EC_DLG_MENU         9965        // menu commerce
#define EC_DLG_BUY          9966        // saisie quantite a acheter
#define EC_DLG_SELLBIZ      9967        // saisie quantite a vendre au biz
#define EC_DLG_SELLENT      9968        // saisie quantite a vendre a une entreprise

#define EC_STOCK_MAX        5000        // capacite de stock d'une entreprise
#define EC_PRICE_BUY        1000        // prix d'achat en gros (par unite)
#define EC_PRICE_SELL_BIZ   1600        // revente a un biz (par unite)
#define EC_PRICE_SELL_ENT   1400        // revente a une autre entreprise
#define EC_BIZ_RADIUS       6.0         // distance max de l'entree d'un biz
#define EC_ENT_RADIUS       6.0         // distance max de l'entree d'une entreprise

// Cible memorisee entre l'ouverture du dialog et la saisie de la quantite.
new ec_TargetBiz[MAX_PLAYERS];
new ec_TargetEnt[MAX_PLAYERS];

// Entreprise du joueur (patron OU employe), -1 sinon.
stock EC_PlayerEnt(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid != -1) return eid;
	new empEid, empSlot;
	if(Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], empEid, empSlot)) return empEid;
	return -1;
}

// Biz le plus proche (entree) dans EC_BIZ_RADIUS, ou celui dans lequel le
// joueur se trouve. -1 sinon.
stock EC_NearestBiz(playerid)
{
	if(gPlayerInBizz[playerid] >= 0 && gPlayerInBizz[playerid] < MAX_BIZZ
		&& bizz[gPlayerInBizz[playerid]][used] == 1)
		return gPlayerInBizz[playerid];

	new Float:best = EC_BIZ_RADIUS, bid = -1;
	for(new i = 0; i < MAX_BIZZ; i++)
	{
		if(bizz[i][used] != 1) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, bizz[i][pos][0], bizz[i][pos][1], bizz[i][pos][2]);
		if(d < best) { best = d; bid = i; }
	}
	return bid;
}

// Entreprise (autre que la sienne) dont l'entree est la plus proche. -1 sinon.
stock EC_NearestOtherEnt(playerid, myEid)
{
	new Float:best = EC_ENT_RADIUS, found = -1;
	for(new e = 0; e < ENT_MAX; e++)
	{
		if(entreprises[e][entUsed] != 1) continue;
		if(e == myEid) continue;
		if(entreprises[e][entIntId] < 1) continue;   // pas de local = pas d'entree
		new Float:d = GetPlayerDistanceFromPoint(playerid,
			entreprises[e][entEntryX], entreprises[e][entEntryY], entreprises[e][entEntryZ]);
		if(d < best) { best = d; found = e; }
	}
	return found;
}

public EC_ShowMenu(playerid)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Tu n'as ni entreprise ni emploi.");

	new body[600], tmp[160];
	format(tmp, sizeof(tmp), "{FFFFFF}%s\nStock : {FFFFB2}%d{FFFFFF} / %d unites  -  Caisse : {FFFFB2}%d${FFFFFF}\n\n",
		entreprises[eid][entName], entreprises[eid][entStock], EC_STOCK_MAX, entreprises[eid][entCash]);
	strcat(body, tmp);

	format(tmp, sizeof(tmp), "Acheter des marchandises {CCCCCC}(%d$ / unite)\n", EC_PRICE_BUY);
	strcat(body, tmp);
	format(tmp, sizeof(tmp), "{FFFFFF}Vendre a un biz proche {CCCCCC}(%d$ / unite)\n", EC_PRICE_SELL_BIZ);
	strcat(body, tmp);
	format(tmp, sizeof(tmp), "{FFFFFF}Vendre a une entreprise proche {CCCCCC}(%d$ / unite)", EC_PRICE_SELL_ENT);
	strcat(body, tmp);

	return ShowPlayerDialog(playerid, EC_DLG_MENU, DIALOG_STYLE_LIST,
		"{FF8C00}Entreprise - Commerce", body, "Choisir", "Fermer");
}

// ---------------- Achat en gros ----------------
stock EC_AskBuy(playerid)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	// Seul le patron engage l'argent de la caisse.
	if(Ent_FindByOwner(PlayerInfo[playerid][pSQLID]) != eid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Seul le patron peut acheter du stock.");

	new libre = EC_STOCK_MAX - entreprises[eid][entStock];
	if(libre <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ton stock est plein.");

	new maxAbordable = entreprises[eid][entCash] / EC_PRICE_BUY;
	if(maxAbordable <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Caisse insuffisante pour acheter ne serait-ce qu'une unite.");
	if(maxAbordable > libre) maxAbordable = libre;

	new body[220];
	format(body, sizeof(body), "{FFFFFF}Prix : {FFFFB2}%d${FFFFFF} par unite\nPlace libre : %d unites\nCaisse : {FFFFB2}%d${FFFFFF}\n\nQuantite a acheter (max {FFFFB2}%d{FFFFFF}) :",
		EC_PRICE_BUY, libre, entreprises[eid][entCash], maxAbordable);
	return ShowPlayerDialog(playerid, EC_DLG_BUY, DIALOG_STYLE_INPUT,
		"{FF8C00}Commerce - Acheter", body, "Acheter", "Retour");
}

stock EC_DoBuy(playerid, qty)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	if(Ent_FindByOwner(PlayerInfo[playerid][pSQLID]) != eid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Seul le patron peut acheter du stock.");
	if(qty < 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Quantite invalide.");

	new libre = EC_STOCK_MAX - entreprises[eid][entStock];
	if(qty > libre)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ton stock ne peut pas absorber autant.");

	new cost = qty * EC_PRICE_BUY;
	if(entreprises[eid][entCash] < cost)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Caisse insuffisante.");

	entreprises[eid][entCash]  -= cost;
	entreprises[eid][entStock] += qty;
	Ent_Save();

	new okStr[190];
	format(okStr, sizeof(okStr), "{FF8C00} Commerce {FFFFFF} {FFFFB2}%d{FFFFFF} unites achetees pour {FFFFB2}%d${FFFFFF}. Stock : {FFFFB2}%d{FFFFFF} / %d.",
		qty, cost, entreprises[eid][entStock], EC_STOCK_MAX);
	return msg_Client(playerid, COLOR_WHITE, okStr);
}

// ---------------- Vente a un biz ----------------
stock EC_AskSellBiz(playerid)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	if(entreprises[eid][entStock] <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ton entreprise n'a aucun stock a vendre.");

	new bid = EC_NearestBiz(playerid);
	if(bid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Approche-toi de l'entree d'un biz (ou entre dedans).");

	new besoin = bizz[bid][prodMax] - bizz[bid][products];
	if(besoin <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ce biz est deja plein de produits.");

	new maxPayable = bizz[bid][fund] / EC_PRICE_SELL_BIZ;
	if(maxPayable <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} La caisse de ce biz est trop vide pour t'acheter quoi que ce soit.");

	new maxQty = entreprises[eid][entStock];
	if(maxQty > besoin)     maxQty = besoin;
	if(maxQty > maxPayable) maxQty = maxPayable;

	ec_TargetBiz[playerid] = bid;
	new body[280];
	format(body, sizeof(body), "{FFFFFF}Biz : {FFFFB2}%s{FFFFFF}\nIl lui manque : %d produits\nSa caisse : {FFFFB2}%d${FFFFFF}\nTon stock : {FFFFB2}%d{FFFFFF}\nPrix : {FFFFB2}%d${FFFFFF} / unite\n\nQuantite a vendre (max {FFFFB2}%d{FFFFFF}) :",
		bizz[bid][description], besoin, bizz[bid][fund], entreprises[eid][entStock], EC_PRICE_SELL_BIZ, maxQty);
	return ShowPlayerDialog(playerid, EC_DLG_SELLBIZ, DIALOG_STYLE_INPUT,
		"{FF8C00}Commerce - Vendre a un biz", body, "Vendre", "Retour");
}

stock EC_DoSellBiz(playerid, qty)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	new bid = ec_TargetBiz[playerid];
	if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Biz introuvable.");
	// Le joueur a pu s'eloigner entre l'ouverture du dialog et la validation.
	if(EC_NearestBiz(playerid) != bid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Tu t'es eloigne du biz.");
	if(qty < 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Quantite invalide.");
	if(qty > entreprises[eid][entStock])
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Tu n'as pas autant de stock.");

	new besoin = bizz[bid][prodMax] - bizz[bid][products];
	if(qty > besoin)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ce biz n'a pas la place pour autant de produits.");

	new gain = qty * EC_PRICE_SELL_BIZ;
	if(bizz[bid][fund] < gain)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} La caisse du biz ne peut pas payer cette quantite.");

	bizz[bid][fund]     -= gain;
	bizz[bid][products] += qty;
	entreprises[eid][entStock] -= qty;
	entreprises[eid][entCash]  += gain;
	bizz_Save(bid);
	Ent_Save();
	Obj_Progress(playerid, OBJ_VENTE, qty);   // [OBJECTIFS] unites vendues

	new okStr[200];
	format(okStr, sizeof(okStr), "{FF8C00} Commerce {FFFFFF} {FFFFB2}%d{FFFFFF} unites livrees a {FFFFB2}%s{FFFFFF}. Caisse entreprise : {FFFFB2}+%d${FFFFFF}.",
		qty, bizz[bid][description], gain);
	msg_Client(playerid, COLOR_WHITE, okStr);

	// Previent le proprietaire du biz s'il est en ligne.
	for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
	{
		if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
		if(strcmp(PlayerInfo[pp][pRealName], bizz[bid][owner], true) != 0) continue;
		new ownMsg[200];
		format(ownMsg, sizeof(ownMsg), "{FF8C00} Commerce {FFFFFF} {FFFFB2}%s{FFFFFF} a livre {FFFFB2}%d{FFFFFF} produits a ton biz ({FFFFB2}-%d${FFFFFF} de sa caisse).",
			entreprises[eid][entName], qty, gain);
		msg_Client(pp, COLOR_WHITE, ownMsg);
		break;
	}
	return 1;
}

// ---------------- Vente a une autre entreprise ----------------
stock EC_AskSellEnt(playerid)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	if(entreprises[eid][entStock] <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Ton entreprise n'a aucun stock a vendre.");

	new other = EC_NearestOtherEnt(playerid, eid);
	if(other == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Approche-toi de l'entree du local d'une autre entreprise.");

	new libre = EC_STOCK_MAX - entreprises[other][entStock];
	if(libre <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Le stock de cette entreprise est plein.");

	new maxPayable = entreprises[other][entCash] / EC_PRICE_SELL_ENT;
	if(maxPayable <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} La caisse de cette entreprise est trop vide.");

	new maxQty = entreprises[eid][entStock];
	if(maxQty > libre)      maxQty = libre;
	if(maxQty > maxPayable) maxQty = maxPayable;

	ec_TargetEnt[playerid] = other;
	new body[280];
	format(body, sizeof(body), "{FFFFFF}Entreprise : {FFFFB2}%s{FFFFFF}\nPlace dans son stock : %d\nSa caisse : {FFFFB2}%d${FFFFFF}\nTon stock : {FFFFB2}%d{FFFFFF}\nPrix : {FFFFB2}%d${FFFFFF} / unite\n\nQuantite a vendre (max {FFFFB2}%d{FFFFFF}) :",
		entreprises[other][entName], libre, entreprises[other][entCash], entreprises[eid][entStock], EC_PRICE_SELL_ENT, maxQty);
	return ShowPlayerDialog(playerid, EC_DLG_SELLENT, DIALOG_STYLE_INPUT,
		"{FF8C00}Commerce - Vendre a une entreprise", body, "Vendre", "Retour");
}

stock EC_DoSellEnt(playerid, qty)
{
	new eid = EC_PlayerEnt(playerid);
	if(eid == -1) return 1;
	new other = ec_TargetEnt[playerid];
	if(other < 0 || other >= ENT_MAX || entreprises[other][entUsed] != 1 || other == eid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Entreprise introuvable.");
	if(EC_NearestOtherEnt(playerid, eid) != other)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Tu t'es eloigne du local.");
	if(qty < 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Quantite invalide.");
	if(qty > entreprises[eid][entStock])
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Tu n'as pas autant de stock.");

	new libre = EC_STOCK_MAX - entreprises[other][entStock];
	if(qty > libre)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Son stock ne peut pas absorber autant.");

	new gain = qty * EC_PRICE_SELL_ENT;
	if(entreprises[other][entCash] < gain)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Commerce {FFFFFF} Sa caisse ne peut pas payer cette quantite.");

	entreprises[other][entCash]  -= gain;
	entreprises[other][entStock] += qty;
	entreprises[eid][entStock]   -= qty;
	entreprises[eid][entCash]    += gain;
	Ent_Save();
	Obj_Progress(playerid, OBJ_VENTE, qty);   // [OBJECTIFS] unites vendues

	new okStr[200];
	format(okStr, sizeof(okStr), "{FF8C00} Commerce {FFFFFF} {FFFFB2}%d{FFFFFF} unites vendues a {FFFFB2}%s{FFFFFF}. Caisse : {FFFFB2}+%d${FFFFFF}.",
		qty, entreprises[other][entName], gain);
	msg_Client(playerid, COLOR_WHITE, okStr);

	// Previent le patron de l'entreprise acheteuse s'il est en ligne.
	new po = Ent_FindPlayerBySqlid(entreprises[other][entOwnerSqlid]);
	if(po != INVALID_PLAYER_ID)
	{
		new ownMsg[200];
		format(ownMsg, sizeof(ownMsg), "{FF8C00} Commerce {FFFFFF} {FFFFB2}%s{FFFFFF} t'a livre {FFFFB2}%d{FFFFFF} unites ({FFFFB2}-%d${FFFFFF} de ta caisse).",
			entreprises[eid][entName], qty, gain);
		msg_Client(po, COLOR_WHITE, ownMsg);
	}
	return 1;
}

// Dispatcher dialog (a appeler depuis OnDialogResponse)
stock EC_OnDialogResponse(playerid, dialogid, response, listitem, const inputtext[])
{
	if(dialogid == EC_DLG_MENU)
	{
		if(!response) return 1;
		if(listitem == 0) return EC_AskBuy(playerid);
		if(listitem == 1) return EC_AskSellBiz(playerid);
		if(listitem == 2) return EC_AskSellEnt(playerid);
		return 1;
	}
	if(dialogid == EC_DLG_BUY)
	{
		if(!response) return EC_ShowMenu(playerid);
		EC_DoBuy(playerid, strval(inputtext));
		return EC_ShowMenu(playerid);
	}
	if(dialogid == EC_DLG_SELLBIZ)
	{
		if(!response) return EC_ShowMenu(playerid);
		EC_DoSellBiz(playerid, strval(inputtext));
		return 1;
	}
	if(dialogid == EC_DLG_SELLENT)
	{
		if(!response) return EC_ShowMenu(playerid);
		EC_DoSellEnt(playerid, strval(inputtext));
		return 1;
	}
	return 0;
}
