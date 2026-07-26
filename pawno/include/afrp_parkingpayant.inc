/*
	afrp_parkingpayant.inc - Parking payant : 500$ / 30 minutes

	Se greffe sur le systeme /parking existant sans y toucher, via
	quelques appels de hooks dans LVRP.pwn :
	  - ParkingPay_CanAfford(playerid, carSqlid) : verif de solde avant spawn
	  - ParkingPay_ApplyPayment(playerid, carSqlid, pid, vidx) : debit + ticket
	  - ParkingPay_OnManualDespawn(carSqlid) : nettoie le ticket sans amende
	  - ParkingPay_Tick() : timer 30s (label + expiration + fourriere)
	  - ParkingPay_Cmd_Extend(playerid) : /parking payer pour prolonger

	Regles :
	  - Spawn au parking = 500$ pour 30 min. Ticket cree.
	  - Label 3D au-dessus du vehicule : "PARKING PAYE - XX min restantes"
	    (blanc > 5min, orange < 5min, rouge = expire imminent).
	  - A 5 min de la fin : message d'avertissement envoye au proprio.
	  - A 0 min : vehicule despawn automatique (fourriere).
	  - /parking payer : +30 min pour 500$ (a portee du vehicule, pour
    n'importe quel vehicule - voir ParkingPay_Cmd_Extend).
	  - Vehicule fourriere : ressortir 25.000$ + 500$ (parking neuf) = 25.500$
	    au prochain spawn via /parking. La bagnole revient au parking le
	    plus proche du joueur.

	Persistence : les tickets ne sont PAS sauvegardes en base (volontaire :
	un restart serveur remet tout a plat, les vehicules resortent gratos).
*/

#if defined AFRP_PARKINGPAYANT_INCLUDED
	#endinput
#endif
#define AFRP_PARKINGPAYANT_INCLUDED

#define PP_FEE_INITIAL      500
#define PP_FEE_EXTEND       500
#define PP_FEE_IMPOUND      25000        // supplement fourriere
#define PP_DURATION_SEC     (30 * 60)
#define PP_WARN_SEC         (5 * 60)     // avertir 5 min avant expiration
#define PP_MAX_TICKETS      300
#define PP_TICK_MS          30000        // 30s : rafraichit label + expiration

enum e_PPTicket
{
	ppUsed,              // 0 libre, 1 actif, 2 fourriere (attente recuperation)
	ppCarSqlid,          // SQLID vehicule concerne
	ppOwnerSqlid,        // SQLID proprio (pour identifier a la reconnexion)
	ppExpireTs,          // gettime() a expiration
	ppParkingId,         // id du parking (pour ré-teleport a la recup fourriere)
	ppWarned,            // 1 si SMS 5min deja envoye
	Text3D:ppLabel       // label 3D attache au vehicule
};
new PPTicket[PP_MAX_TICKETS][e_PPTicket];

stock ParkingPay_Init()
{
	for(new i = 0; i < PP_MAX_TICKETS; i++)
	{
		PPTicket[i][ppUsed] = 0;
		PPTicket[i][ppLabel] = Text3D:INVALID_3DTEXT_ID;
	}
	SetTimer("ParkingPay_Tick", PP_TICK_MS, 1);
	return 1;
}

// Retourne l'index du ticket (actif OU fourriere) pour ce vehicule, sinon -1.
stock ParkingPay_FindByCar(carSqlid)
{
	if(carSqlid <= 0) return -1;
	for(new i = 0; i < PP_MAX_TICKETS; i++)
	{
		if(PPTicket[i][ppUsed] == 0) continue;
		if(PPTicket[i][ppCarSqlid] == carSqlid) return i;
	}
	return -1;
}

stock ParkingPay_AllocSlot()
{
	for(new i = 0; i < PP_MAX_TICKETS; i++)
	{
		if(PPTicket[i][ppUsed] == 0) return i;
	}
	return -1;
}

// Verifie que le joueur peut payer le spawn. Renvoie 0 si non (avec msg).
// Prend en compte le supplement fourriere si la voiture y est.
stock ParkingPay_CanAfford(playerid, carSqlid)
{
	new existing = ParkingPay_FindByCar(carSqlid);
	// Ticket deja actif (non expire) : pas de nouvelle facturation, voir ParkingPay_ApplyPayment.
	if(existing != -1 && PPTicket[existing][ppUsed] == 1)
		return 1;
	new required = PP_FEE_INITIAL;
	if(existing != -1 && PPTicket[existing][ppUsed] == 2)
		required += PP_FEE_IMPOUND;
	if(PlayerInfo[playerid][pCash] < required)
	{
		new msg[160];
		if(required == PP_FEE_INITIAL)
			format(msg, sizeof(msg), "{CF9756} Parking {FFFFFF} Il te faut {FFFFB2}%d${FFFFFF} pour te garer ici (30 min).", required);
		else
			format(msg, sizeof(msg), "{CF9756} Parking {FFFFFF} Ta voiture est en fourriere : {FFFFB2}%d${FFFFFF} pour la recuperer (fourriere %d$ + parking %d$).", required, PP_FEE_IMPOUND, PP_FEE_INITIAL);
		msg_Client(playerid, COLOR_INFO, msg);
		return 0;
	}
	return 1;
}

// Debite le joueur et cree/rafraichit le ticket + label 3D. A appeler UNE
// SEULE FOIS apres un spawn parking reussi.
stock ParkingPay_ApplyPayment(playerid, carSqlid, pid, vidx)
{
	if(carSqlid <= 0 || vidx == -1) return 0;
	new tid = ParkingPay_FindByCar(carSqlid);

	// [FIX RESET TICKET] un vehicule deja gare avec un ticket ACTIF (pas expire,
	// pas en fourriere) ne doit pas etre re-facture ni voir son ticket ecrase :
	// avant ce fix, re-spawner/deplacer un vehicule deja paye (ex: /parking sur
	// un autre parking, ou rechargement apres reconnexion) remettait le
	// chrono a 30 min et prenait a nouveau les frais, meme si le joueur avait
	// deja prolonge (ex: 175 min restantes perdues). On se contente de
	// reattacher le label 3D a la nouvelle position du vehicule.
	if(tid != -1 && PPTicket[tid][ppUsed] == 1)
	{
		if(_:PPTicket[tid][ppLabel] != _:INVALID_3DTEXT_ID)
		{
			Delete3DTextLabel(PPTicket[tid][ppLabel]);
			PPTicket[tid][ppLabel] = Text3D:INVALID_3DTEXT_ID;
		}
		PPTicket[tid][ppParkingId] = pid;
		new keepStr[96];
		format(keepStr, sizeof(keepStr), "{42AAFF}PARKING PAYE");
		PPTicket[tid][ppLabel] = Create3DTextLabel(keepStr, 0x42AAFFFF, 0.0, 0.0, 0.0, 20.0, 0, 0);
		Attach3DTextLabelToVehicle(PPTicket[tid][ppLabel], vidx, 0.0, 0.0, 0.7);
		ParkingPay_UpdateLabel(tid);

		new keepMinutes = (PPTicket[tid][ppExpireTs] - gettime()) / 60;
		new keepMsg[160];
		format(keepMsg, sizeof(keepMsg), "{42AAFF} Parking {FFFFFF} Vehicule replace, ticket deja actif conserve ({FFFFB2}%d min restantes{FFFFFF}).", keepMinutes);
		msg_Client(playerid, COLOR_WHITE, keepMsg);
		return 1;
	}

	new required = PP_FEE_INITIAL;
	new bool:wasImpounded = (tid != -1 && PPTicket[tid][ppUsed] == 2);
	if(wasImpounded) required += PP_FEE_IMPOUND;
	SafeGivePlayerMoney(playerid, -required, "Parking payant");

	if(tid == -1) tid = ParkingPay_AllocSlot();
	if(tid == -1) return 0;

	// Detruit un eventuel ancien label
	if(_:PPTicket[tid][ppLabel] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(PPTicket[tid][ppLabel]);
		PPTicket[tid][ppLabel] = Text3D:INVALID_3DTEXT_ID;
	}

	PPTicket[tid][ppUsed]       = 1;
	PPTicket[tid][ppCarSqlid]   = carSqlid;
	PPTicket[tid][ppOwnerSqlid] = PlayerInfo[playerid][pSQLID];
	PPTicket[tid][ppExpireTs]   = gettime() + PP_DURATION_SEC;
	PPTicket[tid][ppParkingId]  = pid;
	PPTicket[tid][ppWarned]     = 0;

	new labelStr[96];
	format(labelStr, sizeof(labelStr), "{42AAFF}PARKING PAYE\n{FFFFFF}30 min restantes");
	PPTicket[tid][ppLabel] = Create3DTextLabel(labelStr, 0x42AAFFFF, 0.0, 0.0, 0.0, 20.0, 0, 0);
	Attach3DTextLabelToVehicle(PPTicket[tid][ppLabel], vidx, 0.0, 0.0, 0.7);

	new msg[160];
	if(wasImpounded)
		format(msg, sizeof(msg), "{42AAFF} Parking {FFFFFF} Voiture recuperee de la fourriere + parking (30 min). {FFFFB2}-%d${FFFFFF}.", required);
	else
		format(msg, sizeof(msg), "{42AAFF} Parking {FFFFFF} Parking paye 30 min ({FFFFB2}-%d${FFFFFF}). Prolonge avec {FFFFB2}/parking payer{FFFFFF}.", required);
	msg_Client(playerid, COLOR_WHITE, msg);
	return 1;
}

// Appelee quand un joueur despawn manuellement son vehicule (pas d'amende).
stock ParkingPay_OnManualDespawn(carSqlid)
{
	new tid = ParkingPay_FindByCar(carSqlid);
	if(tid == -1) return 0;
	if(_:PPTicket[tid][ppLabel] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(PPTicket[tid][ppLabel]);
		PPTicket[tid][ppLabel] = Text3D:INVALID_3DTEXT_ID;
	}
	PPTicket[tid][ppUsed] = 0;
	return 1;
}

// Cherche playerid connecte pour un SQLID donne (retourne INVALID_PLAYER_ID sinon).
stock ParkingPay_FindPlayerBySqlid(pp_sqlid)
{
	if(pp_sqlid < 1) return INVALID_PLAYER_ID;
	for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
	{
		if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
		if(PlayerInfo[p][pSQLID] == pp_sqlid) return p;
	}
	return INVALID_PLAYER_ID;
}

stock ParkingPay_UpdateLabel(tid)
{
	if(PPTicket[tid][ppUsed] != 1) return 0;
	if(_:PPTicket[tid][ppLabel] == _:INVALID_3DTEXT_ID) return 0;
	new secLeft = PPTicket[tid][ppExpireTs] - gettime();
	if(secLeft < 0) secLeft = 0;
	new minLeft = secLeft / 60;
	new color, header[32];
	if(secLeft > PP_WARN_SEC)
	{
		color = 0x42AAFFFF;
		strcat(header, "{42AAFF}PARKING PAYE\n");
	}
	else if(secLeft > 60)
	{
		color = 0xFFAA00FF;
		strcat(header, "{FFAA00}PARKING BIENTOT EXPIRE\n");
	}
	else
	{
		color = 0xFF3333FF;
		strcat(header, "{FF3333}EXPIRATION IMMINENTE\n");
	}
	new labelStr[128];
	format(labelStr, sizeof(labelStr), "%s{FFFFFF}%d min restantes\n{CCCCCC}/parking payer pour prolonger", header, minLeft);
	Update3DTextLabelText(PPTicket[tid][ppLabel], color, labelStr);
	return 1;
}

// Vehicule expire : envoye en zone reserve, ticket bascule en fourriere.
stock ParkingPay_Impound(tid)
{
	if(PPTicket[tid][ppUsed] != 1) return 0;
	new vidx = vehicle_GetId(PPTicket[tid][ppCarSqlid]);
	if(vidx != -1)
	{
		SafeSetVehiclePos(vidx, Car_Reserve_x, Car_Reserve_y, Car_Reserve_z);
		SetVehicleVirtualWorld(vidx, 9999);
		vehicle[vidx][cVW] = 9999;
		vehicle[vidx][cLocationx] = Car_Reserve_x;
		vehicle[vidx][cLocationy] = Car_Reserve_y;
		vehicle[vidx][cLocationz] = Car_Reserve_z;
		vehicle_Save(vidx, 0, 0);
	}
	if(_:PPTicket[tid][ppLabel] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(PPTicket[tid][ppLabel]);
		PPTicket[tid][ppLabel] = Text3D:INVALID_3DTEXT_ID;
	}
	PPTicket[tid][ppUsed] = 2;

	new p = ParkingPay_FindPlayerBySqlid(PPTicket[tid][ppOwnerSqlid]);
	if(p != INVALID_PLAYER_ID)
	{
		new msg[160];
		format(msg, sizeof(msg), "{FF3333} Fourriere {FFFFFF} Ton vehicule a ete envoye a la fourriere (parking expire). Recupere-le au prochain /parking pour {FFFFB2}%d${FFFFFF}.", PP_FEE_INITIAL + PP_FEE_IMPOUND);
		msg_Client(p, COLOR_WHITE, msg);
	}
	return 1;
}

forward ParkingPay_Tick();
public ParkingPay_Tick()
{
	new now = gettime();
	for(new i = 0; i < PP_MAX_TICKETS; i++)
	{
		if(PPTicket[i][ppUsed] != 1) continue;

		// Si le vehicule n'est plus charge en memoire, on garde le ticket
		// tel quel : le timer continue de tourner, il finira en fourriere
		// (Impound sait gerer vidx==-1).

		new secLeft = PPTicket[i][ppExpireTs] - now;
		if(secLeft <= 0)
		{
			ParkingPay_Impound(i);
			continue;
		}
		// Avertissement 5 min avant
		if(secLeft <= PP_WARN_SEC && PPTicket[i][ppWarned] == 0)
		{
			PPTicket[i][ppWarned] = 1;
			new p = ParkingPay_FindPlayerBySqlid(PPTicket[i][ppOwnerSqlid]);
			if(p != INVALID_PLAYER_ID)
			{
				msg_Client(p, COLOR_INFO, "{FFAA00} Parking {FFFFFF} Ton vehicule expire dans moins de 5 min. Tape {FFFFB2}/parking payer{FFFFFF} pour prolonger 30 min.");
			}
		}
		ParkingPay_UpdateLabel(i);
	}
	return 1;
}

// Verifie si playerid est bien le "proprietaire" du vehicule vidx pour le
// parking payant : proprio personnel (ownerSQLID = pSQLID), ou membre de la
// faction qui l'a achete via /factionbuy (CAR_FACTION : ownerSQLID stocke le
// facID = pMember-200, PAS le SQLID joueur - d'ou le faux "n'est pas a toi"
// que se prenaient les joueurs en gang/mafia sur leurs propres vehicules).
stock ParkingPay_IsOwner(playerid, vidx)
{
	new mySqlid = PlayerInfo[playerid][pSQLID];
	if(mySqlid < 1) return 0;

	// [FIX "PAS A TOI"] Source de verite = les slots pCar du joueur, exactement
	// ce que /parking utilise pour lister SES vehicules. On se basait avant
	// uniquement sur ownerSQLID, un champ different qui peut ne pas avoir ete
	// renseigne (vieux vehicules, transferts par d'autres chemins de code) :
	// le joueur pouvait donc sortir sa voiture du parking mais pas la payer.
	new carSqlid = vehicle[vidx][SQLID];
	if(carSqlid > 0)
	{
		for(new s = 0; s < 6; s++)
			if(PlayerInfo[playerid][pCar][s] == carSqlid) return 1;
	}

	if(vehicle[vidx][ownerSQLID] == mySqlid) return 1;
	if(vehicle[vidx][cType] == CAR_FACTION && PlayerInfo[playerid][pMember] >= 200
		&& vehicle[vidx][ownerSQLID] == PlayerInfo[playerid][pMember] - 200)
		return 1;
	return 0;
}

// /parking payer : prolonge de 30 min le ticket du vehicule dans lequel
// le joueur est assis, ou du vehicule le plus proche (< 10m) si a pied.
//
// [OUVERT A TOUS] Aucune verification de propriete : n'importe qui peut payer
// la place de n'importe quel vehicule. C'est un choix volontaire - la detection
// du proprietaire s'appuyait sur des donnees pas toujours coherentes entre
// elles (pCar / ownerSQLID / vehicules donnes par un admin), et des joueurs
// se voyaient refuser le paiement de LEUR propre voiture. Payer pour un autre
// ne peut de toute facon que lui rendre service, donc le risque est nul.
stock ParkingPay_Cmd_Extend(playerid)
{
	new vidx = -1;
	if(IsPlayerInAnyVehicle(playerid))
	{
		vidx = GetPlayerVehicleID(playerid);
	}
	else
	{
		// Vehicule le plus proche a < 10m, a qui qu'il appartienne.
		new Float:bestDist = 10.0;
		new Float:vx, Float:vy, Float:vz;
		for(new v = 1; v < MAX_CAR; v++)
		{
			if(vehicle[v][used] != 1) continue;
			if(vehicle[v][SQLID] <= 0) continue;   // vehicule non enregistre
			GetVehiclePos(v, vx, vy, vz);
			new Float:d = GetPlayerDistanceFromPoint(playerid, vx, vy, vz);
			if(d < bestDist) { bestDist = d; vidx = v; }
		}
	}
	if(vidx == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Parking {FFFFFF} Monte dans un vehicule ou approche-toi (< 10m) pour payer sa place.");

	new carSqlid = vehicle[vidx][SQLID];
	if(carSqlid <= 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Parking {FFFFFF} Ce vehicule n'est pas enregistre, impossible de le payer.");

	new tid = ParkingPay_FindByCar(carSqlid);
	if(tid == -1 || PPTicket[tid][ppUsed] != 1)
	{
		// [FIX] Pas de ticket actif : au lieu de renvoyer le joueur au parking,
		// on en ouvre simplement un ici. Les tickets ne sont volontairement pas
		// sauvegardes, donc apres CHAQUE redemarrage du serveur /parking payer
		// repondait "aucun parking paye actif" pour une voiture pourtant garee.
		if(!ParkingPay_CanAfford(playerid, carSqlid)) return 1;
		return ParkingPay_ApplyPayment(playerid, carSqlid, -1, vidx);
	}

	if(PlayerInfo[playerid][pCash] < PP_FEE_EXTEND)
	{
		new msg[128];
		format(msg, sizeof(msg), "{CF9756} Parking {FFFFFF} Il te faut {FFFFB2}%d${FFFFFF} pour prolonger de 30 min.", PP_FEE_EXTEND);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	SafeGivePlayerMoney(playerid, -PP_FEE_EXTEND, "Parking prolongation");
	PPTicket[tid][ppExpireTs] += PP_DURATION_SEC;
	PPTicket[tid][ppWarned] = 0;
	ParkingPay_UpdateLabel(tid);

	new totalMin = (PPTicket[tid][ppExpireTs] - gettime()) / 60;
	new msg2[176];
	format(msg2, sizeof(msg2), "{42AAFF} Parking {FFFFFF} +30 min. Nouveau temps restant : {FFFFB2}%d min{FFFFFF} ({FFFFB2}-%d${FFFFFF}).", totalMin, PP_FEE_EXTEND);
	msg_Client(playerid, COLOR_WHITE, msg2);

	// Payer pour quelqu'un d'autre est autorise : on previent le proprietaire
	// du ticket s'il est en ligne et que ce n'est pas lui qui vient de payer.
	if(!ParkingPay_IsOwner(playerid, vidx))
	{
		new ppOwner = ParkingPay_FindPlayerBySqlid(PPTicket[tid][ppOwnerSqlid]);
		if(ppOwner != INVALID_PLAYER_ID && ppOwner != playerid)
		{
			new payerName[MAX_PLAYER_NAME]; GetPlayerName(playerid, payerName, sizeof(payerName));
			new msg3[176];
			format(msg3, sizeof(msg3), "{42AAFF} Parking {FFFFFF} {FFFFB2}%s{FFFFFF} a paye la place de ton vehicule (+30 min).", payerName);
			msg_Client(ppOwner, COLOR_WHITE, msg3);
		}
	}
	return 1;
}
