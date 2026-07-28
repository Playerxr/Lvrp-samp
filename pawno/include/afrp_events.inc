/*
	afrp_events.inc - Evenements aleatoires automatiques

	Timer auto : un event toutes les 30-60 min (aleatoire dans plage).
	4 types d'events :
	1. Transport de fonds : pickup $$ vers route specifique
	2. Drop d'armes : cache d'armes dans zone illegale
	3. Livraison drogue : pickup drogue, doit etre rapporte
	4. Convoi VIP : escorter un PNJ (pickup atteindre destination)

	Pour chaque event :
	- Broadcast a tous les joueurs avec coords
	- Pickup cliquable spawnable
	- Premier qui touche = recompense
	- Despawn auto apres 10 min si personne

	Commande admin : /event <type> pour declencher manuellement
*/

#if defined AFRP_EVENTS_INCLUDED
	#endinput
#endif
#define AFRP_EVENTS_INCLUDED

#define EVENT_MIN_INTERVAL  1800    // 30 min minimum entre events
#define EVENT_MAX_INTERVAL  3600    // 60 min maximum
#define EVENT_DESPAWN_AFTER 600     // 10 min sans pickup -> despawn

#define EVENT_NONE          0
#define EVENT_MONEY_TRUCK   1
#define EVENT_WEAPON_DROP   2
#define EVENT_DRUG_PICKUP   3
#define EVENT_VIP_CONVOY    4

new currentEvent = EVENT_NONE;
new eventPickup = -1;            // ID Streamer DynamicPickup
new Text3D:eventLabel3D = Text3D:INVALID_3DTEXT_ID; // label flottant au-dessus
new Float:eventX, Float:eventY, Float:eventZ;
new eventStartTime = 0;
new eventReward = 0;
new eventLabel[64];
new eventSequence = 0;           // [FIX] anti-stack des timers despawn

// Spots possibles pour chaque event (X, Y, Z)
static const Float:moneyTruckSpots[][3] = {
	{1456.8, -1023.6, 23.8},  // Banque LS centre
	{2440.0, -1979.0, 13.5},  // Sud LS
	{1117.8, -1797.0, 16.5},  // Vinewood
	{-1990.0, 1107.0, 53.1}   // SF banque
};

static const Float:weaponDropSpots[][3] = {
	{2229.0, -1721.0, 13.5},  // Ganton
	{-2243.0, -2560.0, 31.7}, // Angel Pine
	{2188.0, 1991.0, 11.5},   // LV
	{-58.0, -1126.0, 1.1}     // Beach
};

static const Float:drugSpots[][3] = {
	{2493.0, -1672.0, 13.3},  // Grove
	{-1929.0, 137.0, 27.6},   // SF
	{2178.0, -1779.0, 13.5},  // South LS
	{1340.0, -1318.0, 14.0}   // Vinewood
};

static const Float:vipDestinations[][3] = {
	{-1655.0, 1213.0, 7.2},   // Aeroport SF
	{1683.0, 1447.0, 10.7},   // Aeroport LV
	{1942.0, -2186.0, 13.5},  // Aeroport LS
	{-1336.0, 493.0, 11.1}    // FBI HQ SF
};

stock Events_GetTypeName(type)
{
	new buf[24];
	switch(type)
	{
		case EVENT_MONEY_TRUCK: buf = "Transport fonds";
		case EVENT_WEAPON_DROP: buf = "Drop d'armes";
		case EVENT_DRUG_PICKUP: buf = "Livraison drogue";
		case EVENT_VIP_CONVOY:  buf = "Convoi VIP";
		default: buf = "Inconnu";
	}
	return buf;
}

stock Events_BroadcastStart()
{
	new msg[200], zname[32] = "Inconnu";
	switch(currentEvent)
	{
		case EVENT_MONEY_TRUCK:
		{
			format(msg, sizeof(msg), "{FFD700}>>> EVENT <<<{FFFFFF} Transport de fonds renverse ! Reward {00FF00}%d ${FFFFFF}. Tape {FFFF00}/event{FFFFFF} pour coords.", eventReward);
		}
		case EVENT_WEAPON_DROP:
		{
			format(msg, sizeof(msg), "{FF6666}>>> EVENT <<<{FFFFFF} Cache d'armes repere ! Premier arrive premier servi. Tape {FFFF00}/event{FFFFFF} pour coords.");
		}
		case EVENT_DRUG_PICKUP:
		{
			format(msg, sizeof(msg), "{8B4513}>>> EVENT <<<{FFFFFF} Livraison de drogue abandonnee ! Reward {00FF00}%d ${FFFFFF}. Tape {FFFF00}/event{FFFFFF} pour coords.", eventReward);
		}
		case EVENT_VIP_CONVOY:
		{
			format(msg, sizeof(msg), "{4169E1}>>> EVENT <<<{FFFFFF} Mission escorte VIP ! Reward {00FF00}%d ${FFFFFF}. Tape {FFFF00}/event{FFFFFF} pour coords.", eventReward);
		}
	}
	// Broadcast a tous
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SendClientMessage(i, -1, msg);
		PlayerPlaySound(i, 1057, 0.0, 0.0, 0.0);
	}
	// [BANDEAU] Un message de chat se noie en trois lignes. Le bandeau du haut
	// reste affiche tant que l'event est disponible, et Events_Stop l'efface -
	// que l'event ait expire ou que quelqu'un l'ait ramasse.
	new bandeau[144];
	switch(currentEvent)
	{
		case EVENT_MONEY_TRUCK: format(bandeau, sizeof(bandeau), "EVENT : transport de fonds renverse  -  %d$  -  /event", eventReward);
		case EVENT_WEAPON_DROP: format(bandeau, sizeof(bandeau), "EVENT : cache d'armes reperee  -  /event");
		case EVENT_DRUG_PICKUP: format(bandeau, sizeof(bandeau), "EVENT : livraison de drogue abandonnee  -  %d$  -  /event", eventReward);
		case EVENT_VIP_CONVOY:  format(bandeau, sizeof(bandeau), "EVENT : mission escorte VIP  -  %d$  -  /event", eventReward);
		default:                format(bandeau, sizeof(bandeau), "EVENT en cours  -  /event");
	}
	Annonce_PermSet(bandeau, 0xFFD700FF);

	#pragma unused zname
	return 1;
}

stock Events_Stop()
{
	if(eventPickup != -1)
	{
		DestroyDynamicPickup(eventPickup);
		eventPickup = -1;
	}
	if(eventLabel3D != Text3D:INVALID_3DTEXT_ID)
	{
		DestroyDynamic3DTextLabel(eventLabel3D);
		eventLabel3D = Text3D:INVALID_3DTEXT_ID;
	}
	// [BANDEAU] Seul point de sortie du module : appele aussi bien quand l'event
	// expire (Events_AutoDespawn) que quand un joueur le ramasse
	// (Events_OnPickup). Effacer ici couvre donc les deux cas d'un seul coup.
	Annonce_PermClear();

	currentEvent = EVENT_NONE;
	eventReward = 0;
	eventLabel[0] = 0;
	return 1;
}

stock Events_Trigger(type)
{
	if(type < 1 || type > 4) return 0;
	if(currentEvent != EVENT_NONE)
	{
		Events_Stop();
	}

	currentEvent = type;
	eventStartTime = gettime();

	new spotIdx;
	switch(type)
	{
		case EVENT_MONEY_TRUCK:
		{
			spotIdx = random(sizeof(moneyTruckSpots));
			eventX = moneyTruckSpots[spotIdx][0];
			eventY = moneyTruckSpots[spotIdx][1];
			eventZ = moneyTruckSpots[spotIdx][2];
			eventReward = 25000 + random(25000); // 25-50k
			eventPickup = CreateDynamicPickup(1212, 2, eventX, eventY, eventZ, -1); // $$ icon
			eventLabel = "Transport de fonds";
		}
		case EVENT_WEAPON_DROP:
		{
			spotIdx = random(sizeof(weaponDropSpots));
			eventX = weaponDropSpots[spotIdx][0];
			eventY = weaponDropSpots[spotIdx][1];
			eventZ = weaponDropSpots[spotIdx][2];
			eventReward = 0; // pas de cash, c'est une arme
			eventPickup = CreateDynamicPickup(363, 2, eventX, eventY, eventZ, -1); // RPG icon
			eventLabel = "Cache d'armes";
		}
		case EVENT_DRUG_PICKUP:
		{
			spotIdx = random(sizeof(drugSpots));
			eventX = drugSpots[spotIdx][0];
			eventY = drugSpots[spotIdx][1];
			eventZ = drugSpots[spotIdx][2];
			eventReward = 15000 + random(15000); // 15-30k
			eventPickup = CreateDynamicPickup(1279, 2, eventX, eventY, eventZ, -1); // valise
			eventLabel = "Livraison drogue";
		}
		case EVENT_VIP_CONVOY:
		{
			spotIdx = random(sizeof(vipDestinations));
			eventX = vipDestinations[spotIdx][0];
			eventY = vipDestinations[spotIdx][1];
			eventZ = vipDestinations[spotIdx][2];
			eventReward = 35000 + random(15000); // 35-50k
			eventPickup = CreateDynamicPickup(1239, 2, eventX, eventY, eventZ, -1); // question mark
			eventLabel = "Convoi VIP";
		}
	}
	// Label 3D flottant au-dessus du pickup (visible 100m)
	new lblText[96];
	format(lblText, sizeof(lblText), "{FFD700}>>> EVENT <<<\n{FFFFFF}%s\n{00FF00}Recompense : %d $", eventLabel, eventReward);
	eventLabel3D = CreateDynamic3DTextLabel(lblText, 0xFFD700FF, eventX, eventY, eventZ + 1.2, 100.0);

	Events_BroadcastStart();
	// Auto despawn apres EVENT_DESPAWN_AFTER (avec sequence pour eviter le stack)
	eventSequence++;
	SetTimerEx("Events_AutoDespawn", EVENT_DESPAWN_AFTER * 1000, false, "i", eventSequence);
	return 1;
}

forward Events_AutoDespawn(seq);
public Events_AutoDespawn(seq)
{
	if(currentEvent == EVENT_NONE) return 0;
	// Si la sequence ne correspond pas, un nouvel event a ete declenche
	// entre temps : ce timer obsolete doit etre ignore.
	if(seq != eventSequence) return 0;
	new now = gettime();
	if(now - eventStartTime >= EVENT_DESPAWN_AFTER)
	{
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			SendClientMessage(i, 0x888888FF, "{888888}[EVENT]{FFFFFF} L'event a expire, personne ne l'a recupere.");
		}
		Events_Stop();
	}
	return 1;
}

// Appel depuis OnPlayerPickUpPickup
stock Events_OnPickup(playerid, pickupid)
{
	if(currentEvent == EVENT_NONE || pickupid != eventPickup) return 0;

	new msg[200];
	switch(currentEvent)
	{
		case EVENT_MONEY_TRUCK:
		{
			GivePlayerMoney(playerid, eventReward);
			PlayerInfo[playerid][pCash] += eventReward;
			format(msg, sizeof(msg),
				"{FFD700}>>> EVENT <<<{FFFFFF} %s a recupere le transport de fonds. +{00FF00}%d ${FFFFFF}",
				PlayerInfo[playerid][pName], eventReward);
		}
		case EVENT_WEAPON_DROP:
		{
			GivePlayerWeapon(playerid, 31, 200); // M4 + 200 munitions
			format(msg, sizeof(msg),
				"{FF6666}>>> EVENT <<<{FFFFFF} %s a recupere la cache d'armes. {FFFF00}+ M4 200 cart.",
				PlayerInfo[playerid][pName]);
		}
		case EVENT_DRUG_PICKUP:
		{
			GivePlayerMoney(playerid, eventReward);
			PlayerInfo[playerid][pCash] += eventReward;
			format(msg, sizeof(msg),
				"{8B4513}>>> EVENT <<<{FFFFFF} %s a recupere la livraison de drogue. +{00FF00}%d ${FFFFFF}",
				PlayerInfo[playerid][pName], eventReward);
		}
		case EVENT_VIP_CONVOY:
		{
			GivePlayerMoney(playerid, eventReward);
			PlayerInfo[playerid][pCash] += eventReward;
			format(msg, sizeof(msg),
				"{4169E1}>>> EVENT <<<{FFFFFF} %s a complete le convoi VIP. +{00FF00}%d ${FFFFFF}",
				PlayerInfo[playerid][pName], eventReward);
		}
	}
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SendClientMessage(i, -1, msg);
	}
	Events_Stop();
	return 1;
}

// Timer principal : auto-fire un event apres un delai aleatoire
forward Events_AutoTrigger();
public Events_AutoTrigger()
{
	if(currentEvent == EVENT_NONE)
	{
		new type = 1 + random(4); // 1..4
		Events_Trigger(type);
	}
	// Re-schedule pour la prochaine fois (delai aleatoire)
	new nextDelay = EVENT_MIN_INTERVAL + random(EVENT_MAX_INTERVAL - EVENT_MIN_INTERVAL);
	SetTimer("Events_AutoTrigger", nextDelay * 1000, false);
	return 1;
}

stock Events_Init()
{
	// Premier event dans 5 minutes (laisse le serveur se stabiliser)
	SetTimer("Events_AutoTrigger", 300 * 1000, false);
	return 1;
}

// Commande joueur : voir l'event en cours
stock Events_CmdInfo(playerid)
{
	if(currentEvent == EVENT_NONE)
	{
		SendClientMessage(playerid, 0x888888FF, "{888888}[EVENT]{FFFFFF} Aucun event en cours. Patience !");
		return 1;
	}
	new now = gettime();
	new elapsed = now - eventStartTime;
	new left = EVENT_DESPAWN_AFTER - elapsed;
	if(left < 0) left = 0;

	new msg[200];
	format(msg, sizeof(msg),
		"{FFAA00}[EVENT]{FFFFFF} %s | Reward : {00FF00}%d ${FFFFFF} | Coords : %.0f,%.0f,%.0f | Expire dans %ds",
		eventLabel, eventReward, eventX, eventY, eventZ, left);
	SendClientMessage(playerid, -1, msg);

	// Set checkpoint pour guidance
	SetPlayerCheckpoint(playerid, eventX, eventY, eventZ, 5.0);
	SendClientMessage(playerid, 0x00FF00FF, "{00FF00}Un checkpoint a ete ajoute sur ta map.");
	return 1;
}

// Commande admin : declencher manuellement
stock Events_CmdAdmin(playerid, type)
{
	if(PlayerInfo[playerid][pAdmin] < 2)
		return SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 2+.");
	if(type < 1 || type > 4)
	{
		SendClientMessage(playerid, -1, "{FFFFFF}Usage: /eventtrigger <1=fonds 2=armes 3=drogue 4=VIP>");
		return 1;
	}
	Events_Trigger(type);
	SendClientMessage(playerid, 0x00FF00FF, "{00FF00}[Admin]{FFFFFF} Event declenche.");
	return 1;
}
