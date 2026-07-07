/*
	afrp_vipperks.inc - Nouveaux avantages V.I.P (packs "Confort & prestige")

	1) TAG FLOTTANT : petit texte colore au-dessus de la tete selon le palier
	   (Fer/Argent/Or/Diamant), visible par tout le monde. Purement cosmetique,
	   ne touche PAS SetPlayerColor (deja utilise pour identifier les factions
	   police/FBI/hopital/armee - un VIP policier doit rester bleu).

	2) TELEPORT RAPIDE : /vip maison et /vip garage, cooldown qui baisse selon
	   le palier (30 min Fer -> 2 min Diamant). Pur confort, pas d'avantage
	   combat/economique.

	Le palier Fer (tier 1) ne beneficiait avant que du chat VIP + changement de
	skin libre : ce module lui donne enfin des avantages tangibles.
*/

#if defined AFRP_VIPPERKS_INCLUDED
	#endinput
#endif
#define AFRP_VIPPERKS_INCLUDED

new Text3D:vipTagLabel[MAX_PLAYERS];
new bool:vipTagActive[MAX_PLAYERS];
new vipTpMaisonLast[MAX_PLAYERS];
new vipTpGarageLast[MAX_PLAYERS];

stock VipPerks_GetColor(rank)
{
	switch(rank)
	{
		case 1: return 0x8C7853FF; // Fer
		case 2: return 0xC0C0C0FF; // Argent
		case 3: return 0xFFD700FF; // Or
		case 4: return 0x00FFFFFF; // Diamant
	}
	return 0xFFFFFFFF;
}

stock VipPerks_GetTpCooldown(rank)
{
	switch(rank)
	{
		case 1: return 1800; // 30 min
		case 2: return 1200; // 20 min
		case 3: return 600;  // 10 min
		case 4: return 120;  // 2 min
	}
	return 1800;
}

stock VipTag_Update(playerid)
{
	if(vipTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(vipTagLabel[playerid]);
		vipTagActive[playerid] = false;
	}
	new rank = PlayerInfo[playerid][pDonateRank];
	if(rank < 1 || rank > 4) return 0;

	new const titles[5][20] = {"", "* VIP FER *", "* VIP ARGENT *", "* VIP OR *", "* VIP DIAMANT *"};
	vipTagLabel[playerid] = CreateDynamic3DTextLabel(titles[rank], VipPerks_GetColor(rank),
		0.0, 0.0, 1.05, 8.0, playerid, INVALID_VEHICLE_ID, 0, -1, -1, -1, LABEL_STREAM_DISTANCE);
	vipTagActive[playerid] = true;
	return 1;
}

stock VipPerks_OnConnect(playerid)
{
	vipTagActive[playerid] = false;
	vipTpMaisonLast[playerid] = 0;
	vipTpGarageLast[playerid] = 0;
	return 1;
}

stock VipPerks_OnDisconnect(playerid)
{
	if(vipTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(vipTagLabel[playerid]);
		vipTagActive[playerid] = false;
	}
	return 1;
}

stock VipPerks_CmdTpMaison(playerid)
{
	new rank = PlayerInfo[playerid][pDonateRank];
	if(rank < 1)
		{SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[VIP]{FFFFFF} Commande reservee aux membres V.I.P."); return 1;}

	new now = gettime();
	new cd = VipPerks_GetTpCooldown(rank);
	if(now - vipTpMaisonLast[playerid] < cd)
	{
		new left = cd - (now - vipTpMaisonLast[playerid]);
		new msg[100];
		format(msg, sizeof(msg), "{FFAA00}[VIP]{FFFFFF} Attendez encore {FFFF00}%d min{FFFFFF} avant de reutiliser.", (left/60)+1);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}

	new hid = -1;
	if(PlayerInfo[playerid][pHousekey1] > -1) hid = PlayerInfo[playerid][pHousekey1];
	else if(PlayerInfo[playerid][pHousekey2] > -1) hid = PlayerInfo[playerid][pHousekey2];
	else if(PlayerInfo[playerid][pHousekey3] > -1) hid = PlayerInfo[playerid][pHousekey3];

	if(hid == -1)
		{SendClientMessage(playerid, -1, "{FFAA00}[VIP]{FFFFFF} Vous ne possedez aucune maison."); return 1;}

	SafeSetPlayerPos(playerid, house[hid][pos][0], house[hid][pos][1], house[hid][pos][2]);
	vipTpMaisonLast[playerid] = now;
	SendClientMessage(playerid, -1, "{00FF00}[VIP]{FFFFFF} Teleportation vers votre maison.");
	return 1;
}

stock VipPerks_CmdTpGarage(playerid)
{
	new rank = PlayerInfo[playerid][pDonateRank];
	if(rank < 1)
		{SendClientMessage(playerid, 0xFFAA00FF, "{FFAA00}[VIP]{FFFFFF} Commande reservee aux membres V.I.P."); return 1;}

	new now = gettime();
	new cd = VipPerks_GetTpCooldown(rank);
	if(now - vipTpGarageLast[playerid] < cd)
	{
		new left = cd - (now - vipTpGarageLast[playerid]);
		new msg[100];
		format(msg, sizeof(msg), "{FFAA00}[VIP]{FFFFFF} Attendez encore {FFFF00}%d min{FFFFFF} avant de reutiliser.", (left/60)+1);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}

	new gid = -1;
	if(PlayerInfo[playerid][pGarageKey1] > -1) gid = PlayerInfo[playerid][pGarageKey1];
	else if(PlayerInfo[playerid][pGarageKey2] > -1) gid = PlayerInfo[playerid][pGarageKey2];
	else if(PlayerInfo[playerid][pGarageKey3] > -1) gid = PlayerInfo[playerid][pGarageKey3];

	if(gid == -1)
		{SendClientMessage(playerid, -1, "{FFAA00}[VIP]{FFFFFF} Vous ne possedez aucun garage."); return 1;}

	SafeSetPlayerPos(playerid, garage[gid][pos][0], garage[gid][pos][1], garage[gid][pos][2]);
	vipTpGarageLast[playerid] = now;
	SendClientMessage(playerid, -1, "{00FF00}[VIP]{FFFFFF} Teleportation vers votre garage.");
	return 1;
}
