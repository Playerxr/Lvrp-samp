/*
	afrp_entmarket.inc - Annuaire des entreprises + bourse de contrats

	Deux manques que le systeme d'entreprise avait :

	1. ANNUAIRE (/entreprises)
	   Une entreprise n'existait que pour son patron et ses employes. Aucun
	   moyen, pour un joueur lambda ou un patron de biz, de savoir qu'elle
	   existe ni ou la trouver. Tout le commerce (afrp_entcommerce.inc) etait
	   donc condamne a ne servir qu'entre gens qui se connaissaient deja.
	   L'annuaire liste les entreprises (nom, secteur, patron, employes) et
	   pose un point rouge sur la map vers le local choisi.

	2. BOURSE DE CONTRATS (/contrat, /contrats)
	   Un patron de biz publie un besoin ("500 produits a 1.500$ l'unite"),
	   les entreprises le voient dans /contrats et vont livrer. Inverse la
	   demarche : au lieu que le vendeur cherche un client au hasard, c'est le
	   client qui exprime son besoin. Le paiement sort de la caisse du biz a
	   la livraison, exactement comme une vente directe.

	Depend de afrp_entreprise.inc, afrp_cargomission.inc et afrp_entcommerce.inc
	(inclus AVANT) : entreprises[], EC_PlayerEnt, EC_NearestBiz, cargo_MissionActive.

	Persistance des contrats : scriptfiles/contrats.cfg
*/

#if defined AFRP_ENTMARKET_INCLUDED
	#endinput
#endif
#define AFRP_ENTMARKET_INCLUDED

#define EM_DLG_DIR          9969        // annuaire : liste
#define EM_DLG_DIR_INFO     9964        // annuaire : fiche entreprise
#define EM_DLG_CONTRACTS    9955        // bourse : liste des contrats
#define EM_DLG_CONTRACT_ACT 9956        // bourse : actions sur un contrat
#define EM_DLG_NEW_QTY      9957        // creation : quantite
#define EM_DLG_NEW_PRICE    9958        // creation : prix unitaire

#define EM_MAX_CONTRACT     40
#define EM_FILE             "contrats.cfg"
#define EM_PRICE_MIN        200         // prix unitaire plancher
#define EM_PRICE_MAX        10000       // prix unitaire plafond

enum e_Contract
{
	coUsed,
	coBizId,                 // biz demandeur
	coQty,                   // quantite encore demandee
	coPrice,                 // prix par unite
	coOwnerSqlid,            // SQLID du patron du biz au moment de la creation
	coCreatedTs
};
new Contracts[EM_MAX_CONTRACT][e_Contract];

// Selection en cours, par joueur
new em_DirSel[MAX_PLAYERS];        // eid affiche dans la fiche annuaire
new em_ContractSel[MAX_PLAYERS];   // index contrat selectionne
new em_NewQty[MAX_PLAYERS];        // quantite saisie avant le prix
new em_DirMap[MAX_PLAYERS][ENT_MAX];      // listitem -> eid
new em_ContractMap[MAX_PLAYERS][EM_MAX_CONTRACT]; // listitem -> index contrat

// ============================================================================
// PERSISTANCE
// ============================================================================
stock EM_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock EM_Save()
{
	if(fexist(EM_FILE)) fremove(EM_FILE);
	new File:f = fopen(EM_FILE, io_write);
	if(!f) return 0;
	new line[128];
	for(new i = 0; i < EM_MAX_CONTRACT; i++)
	{
		if(Contracts[i][coUsed] != 1) continue;
		format(line, sizeof(line), "%d|%d|%d|%d|%d\n",
			Contracts[i][coBizId], Contracts[i][coQty], Contracts[i][coPrice],
			Contracts[i][coOwnerSqlid], Contracts[i][coCreatedTs]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock EM_Load()
{
	if(!fexist(EM_FILE)) return 0;
	new File:f = fopen(EM_FILE, io_read);
	if(!f) return 0;
	new line[128], tmpBuf[32], c = 0;
	while(fread(f, line) && c < EM_MAX_CONTRACT)
	{
		if(strlen(line) < 3) continue;
		new i = 0;
		EM_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); Contracts[c][coBizId]      = strval(tmpBuf);
		EM_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); Contracts[c][coQty]        = strval(tmpBuf);
		EM_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); Contracts[c][coPrice]      = strval(tmpBuf);
		EM_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); Contracts[c][coOwnerSqlid] = strval(tmpBuf);
		EM_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); Contracts[c][coCreatedTs]  = strval(tmpBuf);
		// Un contrat dont le biz a disparu ou dont la quantite est nulle est ignore.
		if(Contracts[c][coBizId] < 0 || Contracts[c][coBizId] >= MAX_BIZZ) continue;
		if(Contracts[c][coQty] < 1) continue;
		Contracts[c][coUsed] = 1;
		c++;
	}
	fclose(f);
	printf("[EntMarket] %d contrat(s) charge(s)", c);
	return c;
}

stock EM_Init()
{
	for(new i = 0; i < EM_MAX_CONTRACT; i++) Contracts[i][coUsed] = 0;
	EM_Load();
	return 1;
}

// ============================================================================
// ANNUAIRE
// ============================================================================
stock EM_ShowDirectory(playerid)
{
	new body[1400], tmp[160], c = 0;
	strcat(body, "Entreprise\tSecteur\tEmployes\n");
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		new nbEmp = 0;
		for(new e = 0; e < ENT_MAX_EMP; e++)
			if(entreprises[eid][entEmpUsed][e] == 1) nbEmp++;
		new typeStr[32]; Ent_TypeName(eid, typeStr, sizeof(typeStr));
		format(tmp, sizeof(tmp), "%s\t{CCCCCC}%s{FFFFFF}\t%d\n", entreprises[eid][entName], typeStr, nbEmp);
		strcat(body, tmp);
		em_DirMap[playerid][c] = eid;
		c++;
	}
	if(c == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Annuaire {FFFFFF} Aucune entreprise n'existe pour le moment.");

	return ShowPlayerDialog(playerid, EM_DLG_DIR, DIALOG_STYLE_TABLIST_HEADERS,
		"{42AAFF}Annuaire des entreprises", body, "Voir", "Fermer");
}

stock EM_ShowDirInfo(playerid)
{
	new eid = em_DirSel[playerid];
	if(eid < 0 || eid >= ENT_MAX || entreprises[eid][entUsed] != 1)
		return EM_ShowDirectory(playerid);

	new nbEmp = 0;
	for(new e = 0; e < ENT_MAX_EMP; e++)
		if(entreprises[eid][entEmpUsed][e] == 1) nbEmp++;

	new typeStr[32]; Ent_TypeName(eid, typeStr, sizeof(typeStr));
	new ownerName[MAX_PLAYER_NAME]; strcat(ownerName, "hors ligne");
	new po = Ent_FindPlayerBySqlid(entreprises[eid][entOwnerSqlid]);
	if(po != INVALID_PLAYER_ID) GetPlayerName(po, ownerName, sizeof(ownerName));

	new localStr[40];
	if(entreprises[eid][entIntId] >= 1) strcat(localStr, "local ouvert au public");
	else                                strcat(localStr, "pas encore de local");

	new body[420];
	format(body, sizeof(body),
		"{FFFFFF}Nom : {FFFFB2}%s{FFFFFF}\nSecteur : {FFFFB2}%s{FFFFFF}\nPatron : %s\nEmployes : %d / %d\nLocal : %s\n\n{CCCCCC}Choisis \"Y aller\" pour marquer le local d'un point rouge sur ta map.",
		entreprises[eid][entName], typeStr, ownerName, nbEmp, ENT_MAX_EMP, localStr);

	return ShowPlayerDialog(playerid, EM_DLG_DIR_INFO, DIALOG_STYLE_MSGBOX,
		"{42AAFF}Fiche entreprise", body, "Y aller", "Retour");
}

// Pose un checkpoint sur le local de l'entreprise selectionnee.
stock EM_GotoEnt(playerid)
{
	new eid = em_DirSel[playerid];
	if(eid < 0 || eid >= ENT_MAX || entreprises[eid][entUsed] != 1) return 1;
	if(entreprises[eid][entIntId] < 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Annuaire {FFFFFF} Cette entreprise n'a pas encore de local.");

	// [SECURITE] Ne pas ecraser le checkpoint d'une mission cargo en cours :
	// le joueur perdrait sa livraison sans comprendre pourquoi.
	if(cargo_MissionActive[playerid] == 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Annuaire {FFFFFF} Termine ou annule ta mission cargo d'abord (/cargo annuler), sinon tu perdrais ton point de livraison.");

	SetPlayerCheckpoint(playerid, entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ], 4.0);
	new okStr[190];
	format(okStr, sizeof(okStr), "{42AAFF} Annuaire {FFFFFF} Direction {FFFFB2}%s{FFFFFF} : suis le point rouge sur ta map.", entreprises[eid][entName]);
	return msg_Client(playerid, COLOR_WHITE, okStr);
}

// ============================================================================
// BOURSE DE CONTRATS
// ============================================================================

// Le joueur est-il proprietaire du biz bid ?
stock EM_IsBizOwner(playerid, bid)
{
	if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1) return 0;
	if(strcmp(PlayerInfo[playerid][pRealName], bizz[bid][owner], true) == 0) return 1;
	return 0;
}

stock EM_FindContractByBiz(bid)
{
	for(new i = 0; i < EM_MAX_CONTRACT; i++)
	{
		if(Contracts[i][coUsed] != 1) continue;
		if(Contracts[i][coBizId] == bid) return i;
	}
	return -1;
}

stock EM_ShowContracts(playerid)
{
	new body[1400], tmp[190], c = 0;
	strcat(body, "Biz\tQuantite\tPrix/unite\tTotal\n");
	for(new i = 0; i < EM_MAX_CONTRACT; i++)
	{
		if(Contracts[i][coUsed] != 1) continue;
		new bid = Contracts[i][coBizId];
		if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1) continue;
		format(tmp, sizeof(tmp), "%s\t%d\t{FFFFB2}%d${FFFFFF}\t%d$\n",
			bizz[bid][description], Contracts[i][coQty], Contracts[i][coPrice],
			Contracts[i][coQty] * Contracts[i][coPrice]);
		strcat(body, tmp);
		em_ContractMap[playerid][c] = i;
		c++;
	}
	if(c == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Aucun contrat en cours. Un patron de biz peut en publier un avec /contrat.");

	return ShowPlayerDialog(playerid, EM_DLG_CONTRACTS, DIALOG_STYLE_TABLIST_HEADERS,
		"{FF8C00}Bourse de contrats", body, "Voir", "Fermer");
}

stock EM_ShowContractActions(playerid)
{
	new ci = em_ContractSel[playerid];
	if(ci < 0 || ci >= EM_MAX_CONTRACT || Contracts[ci][coUsed] != 1)
		return EM_ShowContracts(playerid);
	new bid = Contracts[ci][coBizId];
	if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1)
		return EM_ShowContracts(playerid);

	new body[400];
	format(body, sizeof(body),
		"{FFFFFF}Biz : {FFFFB2}%s{FFFFFF}\nDemande : {FFFFB2}%d{FFFFFF} unites a {FFFFB2}%d${FFFFFF} l'unite\nCaisse du biz : {FFFFB2}%d${FFFFFF}\n\nY aller {CCCCCC}(point rouge sur la map)\n{FFFFFF}Livrer maintenant {CCCCCC}(il faut etre sur place)\n{FF6666}Annuler ce contrat {CCCCCC}(proprietaire du biz)",
		bizz[bid][description], Contracts[ci][coQty], Contracts[ci][coPrice], bizz[bid][fund]);

	return ShowPlayerDialog(playerid, EM_DLG_CONTRACT_ACT, DIALOG_STYLE_LIST,
		"{FF8C00}Contrat", body, "Valider", "Retour");
}

// Publication : le patron du biz sur place lance /contrat
stock EM_CmdNewContract(playerid)
{
	new bid = EC_NearestBiz(playerid);
	if(bid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Approche-toi de ton biz (ou entre dedans) pour publier un contrat.");
	if(!EM_IsBizOwner(playerid, bid))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Tu n'es pas le proprietaire de ce biz.");
	if(EM_FindContractByBiz(bid) != -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Ce biz a deja un contrat en cours. Annule-le d'abord via /contrats.");

	new besoin = bizz[bid][prodMax] - bizz[bid][products];
	if(besoin <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Ton biz est deja plein de produits.");

	new body[240];
	format(body, sizeof(body), "{FFFFFF}Biz : {FFFFB2}%s{FFFFFF}\nIl te manque {FFFFB2}%d{FFFFFF} produits\nCaisse : {FFFFB2}%d${FFFFFF}\n\nQuantite souhaitee (max {FFFFB2}%d{FFFFFF}) :",
		bizz[bid][description], besoin, bizz[bid][fund], besoin);
	em_ContractSel[playerid] = bid;   // reutilise pour porter le bizid en attente
	return ShowPlayerDialog(playerid, EM_DLG_NEW_QTY, DIALOG_STYLE_INPUT,
		"{FF8C00}Publier un contrat", body, "Suivant", "Annuler");
}

// Livraison d'un contrat par une entreprise presente sur place.
stock EM_DeliverContract(playerid)
{
	new ci = em_ContractSel[playerid];
	if(ci < 0 || ci >= EM_MAX_CONTRACT || Contracts[ci][coUsed] != 1)
		return EM_ShowContracts(playerid);

	new eid = EC_PlayerEnt(playerid);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Tu n'as ni entreprise ni emploi.");

	new bid = Contracts[ci][coBizId];
	if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Ce biz n'existe plus.");
	if(EC_NearestBiz(playerid) != bid)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Tu dois etre sur place, a l'entree du biz.");
	if(entreprises[eid][entStock] <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Ton entreprise n'a aucun stock. Achete-en via /entreprise commerce.");

	// On livre ce qu'on peut : limite par le stock, la demande restante, la
	// place dans le biz et surtout ce que sa caisse peut reellement payer.
	new qty = entreprises[eid][entStock];
	if(qty > Contracts[ci][coQty]) qty = Contracts[ci][coQty];

	new place = bizz[bid][prodMax] - bizz[bid][products];
	if(qty > place) qty = place;

	new payable = bizz[bid][fund] / Contracts[ci][coPrice];
	if(qty > payable) qty = payable;

	if(qty < 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Livraison impossible : le biz est plein ou sa caisse ne peut plus payer.");

	new gain = qty * Contracts[ci][coPrice];
	bizz[bid][fund]     -= gain;
	bizz[bid][products] += qty;
	entreprises[eid][entStock] -= qty;
	entreprises[eid][entCash]  += gain;
	Contracts[ci][coQty] -= qty;

	new bool:fini = (Contracts[ci][coQty] <= 0);
	if(fini) Contracts[ci][coUsed] = 0;

	bizz_Save(bid);
	Ent_Save();
	EM_Save();

	new okStr[220];
	format(okStr, sizeof(okStr), "{FF8C00} Contrat {FFFFFF} {FFFFB2}%d{FFFFFF} unites livrees a {FFFFB2}%s{FFFFFF}. Caisse entreprise : {FFFFB2}+%d${FFFFFF}.%s",
		qty, bizz[bid][description], gain, fini ? " Contrat termine !" : "");
	msg_Client(playerid, COLOR_WHITE, okStr);

	// Previent le patron du biz s'il est en ligne.
	for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
	{
		if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
		if(strcmp(PlayerInfo[pp][pRealName], bizz[bid][owner], true) != 0) continue;
		new ownMsg[220];
		format(ownMsg, sizeof(ownMsg), "{FF8C00} Contrat {FFFFFF} {FFFFB2}%s{FFFFFF} a livre {FFFFB2}%d{FFFFFF} produits ({FFFFB2}-%d${FFFFFF}). Reste a livrer : {FFFFB2}%d{FFFFFF}.",
			entreprises[eid][entName], qty, gain, fini ? 0 : Contracts[ci][coQty]);
		msg_Client(pp, COLOR_WHITE, ownMsg);
		break;
	}
	return 1;
}

// Dispatcher dialog
stock EM_OnDialogResponse(playerid, dialogid, response, listitem, const inputtext[])
{
	if(dialogid == EM_DLG_DIR)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= ENT_MAX) return 1;
		em_DirSel[playerid] = em_DirMap[playerid][listitem];
		return EM_ShowDirInfo(playerid);
	}
	if(dialogid == EM_DLG_DIR_INFO)
	{
		if(!response) return EM_ShowDirectory(playerid);
		return EM_GotoEnt(playerid);
	}
	if(dialogid == EM_DLG_CONTRACTS)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= EM_MAX_CONTRACT) return 1;
		em_ContractSel[playerid] = em_ContractMap[playerid][listitem];
		return EM_ShowContractActions(playerid);
	}
	if(dialogid == EM_DLG_CONTRACT_ACT)
	{
		if(!response) return EM_ShowContracts(playerid);
		new ci = em_ContractSel[playerid];
		if(ci < 0 || ci >= EM_MAX_CONTRACT || Contracts[ci][coUsed] != 1)
			return EM_ShowContracts(playerid);
		new bid = Contracts[ci][coBizId];

		if(listitem == 0)   // y aller
		{
			if(bid < 0 || bid >= MAX_BIZZ) return 1;
			if(cargo_MissionActive[playerid] == 1)
				return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Termine ou annule ta mission cargo d'abord (/cargo annuler).");
			SetPlayerCheckpoint(playerid, bizz[bid][pos][0], bizz[bid][pos][1], bizz[bid][pos][2], 4.0);
			return msg_Client(playerid, COLOR_WHITE, "{FF8C00} Contrat {FFFFFF} Suis le point rouge sur ta map.");
		}
		if(listitem == 1)   // livrer
			return EM_DeliverContract(playerid);
		if(listitem == 2)   // annuler (proprietaire du biz uniquement)
		{
			if(!EM_IsBizOwner(playerid, bid))
				return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Seul le proprietaire du biz peut annuler son contrat.");
			Contracts[ci][coUsed] = 0;
			EM_Save();
			msg_Client(playerid, COLOR_WHITE, "{FF8C00} Contrat {FFFFFF} Contrat annule.");
			return EM_ShowContracts(playerid);
		}
		return 1;
	}
	if(dialogid == EM_DLG_NEW_QTY)
	{
		if(!response) return 1;
		new bid = em_ContractSel[playerid];   // porte le bizid pendant la creation
		if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1) return 1;
		new besoin = bizz[bid][prodMax] - bizz[bid][products];
		new q = strval(inputtext);
		if(q < 1 || q > besoin)
		{
			new errStr[160];
			format(errStr, sizeof(errStr), "{CF9756} Contrats {FFFFFF} Quantite invalide (entre 1 et %d).", besoin);
			msg_Client(playerid, COLOR_INFO, errStr);
			return EM_CmdNewContract(playerid);
		}
		em_NewQty[playerid] = q;

		new body[240];
		format(body, sizeof(body), "{FFFFFF}Quantite : {FFFFB2}%d{FFFFFF} unites\nCaisse du biz : {FFFFB2}%d${FFFFFF}\n\nPrix par unite (entre {FFFFB2}%d${FFFFFF} et {FFFFB2}%d${FFFFFF}) :",
			q, bizz[bid][fund], EM_PRICE_MIN, EM_PRICE_MAX);
		return ShowPlayerDialog(playerid, EM_DLG_NEW_PRICE, DIALOG_STYLE_INPUT,
			"{FF8C00}Publier un contrat", body, "Publier", "Annuler");
	}
	if(dialogid == EM_DLG_NEW_PRICE)
	{
		if(!response) return 1;
		new bid = em_ContractSel[playerid];
		if(bid < 0 || bid >= MAX_BIZZ || bizz[bid][used] != 1) return 1;
		if(!EM_IsBizOwner(playerid, bid))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} Tu n'es pas le proprietaire de ce biz.");

		new prix = strval(inputtext);
		if(prix < EM_PRICE_MIN || prix > EM_PRICE_MAX)
		{
			new errStr[160];
			format(errStr, sizeof(errStr), "{CF9756} Contrats {FFFFFF} Prix invalide (entre %d$ et %d$).", EM_PRICE_MIN, EM_PRICE_MAX);
			return msg_Client(playerid, COLOR_INFO, errStr);
		}

		new slot = -1;
		for(new i = 0; i < EM_MAX_CONTRACT; i++) { if(Contracts[i][coUsed] == 0) { slot = i; break; } }
		if(slot == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Contrats {FFFFFF} La bourse est pleine, reessaie plus tard.");

		Contracts[slot][coUsed]       = 1;
		Contracts[slot][coBizId]      = bid;
		Contracts[slot][coQty]        = em_NewQty[playerid];
		Contracts[slot][coPrice]      = prix;
		Contracts[slot][coOwnerSqlid] = PlayerInfo[playerid][pSQLID];
		Contracts[slot][coCreatedTs]  = gettime();
		EM_Save();

		new total = em_NewQty[playerid] * prix;
		new okStr[220];
		format(okStr, sizeof(okStr), "{FF8C00} Contrat {FFFFFF} Publie : {FFFFB2}%d{FFFFFF} unites a {FFFFB2}%d${FFFFFF} (total {FFFFB2}%d${FFFFFF}). Les entreprises le verront dans /contrats.",
			em_NewQty[playerid], prix, total);
		msg_Client(playerid, COLOR_WHITE, okStr);

		// Avertit les patrons d'entreprise connectes : un contrat qui dort ne
		// sert a rien, autant que l'offre trouve preneur tout de suite.
		for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
		{
			if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
			if(Ent_FindByOwner(PlayerInfo[pp][pSQLID]) == -1) continue;
			if(pp == playerid) continue;
			new annonce[200];
			format(annonce, sizeof(annonce), "{FF8C00} Contrat {FFFFFF} Nouveau contrat : {FFFFB2}%d{FFFFFF} unites a {FFFFB2}%d${FFFFFF} l'unite. Tape {FFFFB2}/contrats{FFFFFF}.",
				Contracts[slot][coQty], prix);
			msg_Client(pp, COLOR_WHITE, annonce);
		}
		return 1;
	}
	return 0;
}
