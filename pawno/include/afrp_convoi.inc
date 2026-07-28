/*
	afrp_convoi.inc - Convois de marchandises escortes

	Une entreprise expedie un chargement de grande valeur d'un point A a un
	point B. Le depart est annonce a TOUT le serveur : la police peut se faire
	payer pour escorter, les criminels peuvent tenter le braquage.

	C'est le seul systeme du serveur qui met entreprise, police et gangs dans
	la meme scene sans qu'un admin ait a animer quoi que ce soit.

	=== DEROULE ===
	  /convoi lancer     (patron ou employe, a l'entree de son entreprise)
	     -> 300 unites de stock partent, une camionnette apparait
	     -> destination : le depot cargo le PLUS LOIN, pour laisser le temps
	     -> annonce serveur + icone sur la carte pour TOUT LE MONDE
	  /convoi            infos, temps restant, valeur, qui escorte
	  /convoi escorter   police / faction legale en service (6 max)
	  /convoi braquer    criminel a moins de 8m de la camionnette
	  /convoi quitter    se retirer de l'escorte

	=== L'ARGENT NE SE CREE PAS ===
	Le convoi a UNE valeur, fixee au depart par le stock consomme. Elle est
	ensuite soit livree, soit volee, jamais les deux :
	  - arrivee  : l'entreprise touche 85%, les escortes se partagent 15%
	  - braquage : les braqueurs se partagent 60%, l'entreprise ne touche rien
	  - echec    : personne ne touche rien, la marchandise est perdue
	300 unites achetees 1000$ l'unite valent 660.000$ en convoi, contre
	480.000$ en vente sure a un biz : la prime de risque est reelle mais le
	gain reste plafonne par le stock reellement consomme. Aucune boucle a
	argent n'est possible.

	=== CHOIX TECHNIQUES ===
	- Pas de checkpoint SA-MP pour l'arrivee : le gamemode en utilise deja
	  partout pour les metiers, et les race checkpoints servent aux courses
	  d'event. L'arrivee est detectee par distance dans le tick de 5s, ce qui
	  n'entre en conflit avec rien.
	- La camionnette est enregistree dans vehicle[] (type CAR_ENTREPRISE,
	  reservoir plein). Un vehicule cree sans cet enregistrement a cGas = 0 et
	  car_Engine refuse de le demarrer : le conducteur reste assis dans un
	  camion mort.
	- Icones de carte 60 (camion) et 61 (destination) : seul le slot 50 est
	  utilise ailleurs dans le gamemode.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Cv_Init()
	- OnPlayerDisconnect -> Cv_OnDisconnect(playerid)
	- OnPlayerDeath      -> Cv_OnDeath(playerid)
	- /convoi            -> Cv_Command(playerid, cmdtext)
*/

#if defined AFRP_CONVOI_INCLUDED
	#endinput
#endif
#define AFRP_CONVOI_INCLUDED

#define CV_OFF              0
#define CV_ROUTE            1

#define CV_UNITS            300     // stock consomme par convoi
#define CV_PRICE_UNIT       2200    // valeur par unite (vente sure : 1600)
#define CV_DUREE_MIN        20      // minutes avant echec
#define CV_COOLDOWN_MIN     10      // delai entre deux convois sur le serveur
#define CV_ESCORT_MAX       6
#define CV_PART_ESCORTE     15      // % de la valeur pour les escortes
#define CV_PART_BRAQUAGE    60      // % de la valeur pour les braqueurs
#define CV_BRAQUAGE_SEC     40      // secondes a tenir pres du camion
#define CV_RADIUS           8.0     // distance pour braquer / arriver
#define CV_TRUCK_MODEL      414
#define CV_TICK_MS          5000
#define CV_ICON_TRUCK       60
#define CV_ICON_DEST        61

new cvState = CV_OFF;
new cvEid = -1;                     // entreprise expeditrice
new cvEntName[64];
new cvDriver = INVALID_PLAYER_ID;
new cvVeh = 0;
new cvUnits;
new cvValue;
new cvExpireTs;
new cvNextTs;                       // fin du delai entre deux convois
new Float:cvDestX, Float:cvDestY, Float:cvDestZ;
new cvDestName[48];

new cvEscort[MAX_PLAYERS];          // 1 = escorte inscrite
new cvBrigand[MAX_PLAYERS];         // 1 = a participe au braquage
new cvHold = INVALID_PLAYER_ID;     // braqueur en train de tenir la position
new cvHoldSec;

// ============================================================================
// OUTILS
// ============================================================================
stock Cv_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Cv_Chat(const text[])
{
	SendClientMessageToAll(0xFF8C00FF, text);
	return 1;
}

stock Cv_EstLegal(playerid)
{
	// Police, FBI, gouvernement, CIA, armee, hopital, justice : pMember 1..199
	return (PlayerInfo[playerid][pMember] >= 1 && PlayerInfo[playerid][pMember] < 200);
}

stock Cv_CountEscorte()
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && cvEscort[i] == 1) n++;
	return n;
}

stock Cv_CountBrigands()
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && cvBrigand[i] == 1) n++;
	return n;
}

// L'entreprise du joueur : la sienne s'il est patron, sinon celle qui l'emploie.
stock Cv_MonEntreprise(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid != -1) return eid;
	new outEid, outSlot;
	if(Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], outEid, outSlot)) return outEid;
	return -1;
}

stock Cv_VehValide(vid)
{
	return (vid > 0 && vid != INVALID_VEHICLE_ID && vid < MAX_CAR);
}

// ============================================================================
// NETTOYAGE
// ============================================================================
stock Cv_ClearIcons()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		RemovePlayerMapIcon(i, CV_ICON_TRUCK);
		RemovePlayerMapIcon(i, CV_ICON_DEST);
	}
	return 1;
}

stock Cv_Reset()
{
	if(Cv_VehValide(cvVeh))
	{
		vehicle[cvVeh][used]  = 0;
		vehicle[cvVeh][cType] = 0;
		vehicle[cvVeh][SQLID] = 0;
		DestroyVehicle(cvVeh);
	}
	cvVeh = 0;
	Cv_ClearIcons();
	for(new i = 0; i < MAX_PLAYERS; i++) { cvEscort[i] = 0; cvBrigand[i] = 0; }
	cvState  = CV_OFF;
	cvEid    = -1;
	cvDriver = INVALID_PLAYER_ID;
	cvHold   = INVALID_PLAYER_ID;
	cvHoldSec = 0;
	cvUnits  = 0;
	cvValue  = 0;
	cvEntName[0] = 0;
	return 1;
}

// ============================================================================
// LANCEMENT
// ============================================================================
stock Cv_Lancer(playerid)
{
	if(cvState != CV_OFF)
		return Cv_Msg(playerid, "{FF4444}Un convoi est deja en route. Un seul a la fois sur le serveur.");
	if(gettime() < cvNextTs)
	{
		new att[144];
		format(att, sizeof(att), "{FF4444}Le prochain convoi ne peut partir que dans %d minute(s).", (cvNextTs - gettime()) / 60 + 1);
		return Cv_Msg(playerid, att);
	}

	new eid = Cv_MonEntreprise(playerid);
	if(eid == -1) return Cv_Msg(playerid, "{FF4444}Tu n'appartiens a aucune entreprise.");
	if(Ent_GetNearestEntry(playerid) != eid)
		return Cv_Msg(playerid, "{FF4444}Va a l'entree de ton entreprise pour charger le convoi.");
	if(entreprises[eid][entStock] < CV_UNITS)
	{
		new manque[176];
		format(manque, sizeof(manque), "{FF4444}Il faut %d unites en stock pour un convoi (tu en as %d).", CV_UNITS, entreprises[eid][entStock]);
		return Cv_Msg(playerid, manque);
	}
	if(IsPlayerInAnyVehicle(playerid))
		return Cv_Msg(playerid, "{FF4444}Descends de ton vehicule avant de charger le convoi.");

	// Destination : le depot cargo le plus eloigne. Un trajet long est ce qui
	// laisse aux gangs et a la police le temps de reagir - une destination
	// proche transformerait le convoi en formalite sans interet.
	new best = -1;
	new Float:bestDist = 0.0, Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	for(new d = 0; d < CG_MAX_DEPOT; d++)
	{
		if(cargoDepot[d][cdUsed] != 1) continue;
		new Float:dist = GetPlayerDistanceFromPoint(playerid, cargoDepot[d][cdX], cargoDepot[d][cdY], cargoDepot[d][cdZ]);
		if(dist > bestDist) { bestDist = dist; best = d; }
	}
	if(best == -1) return Cv_Msg(playerid, "{FF4444}Aucun depot cargo n'est configure : previens un admin.");

	// Debit du stock AVANT de creer quoi que ce soit : si la suite echoue, on
	// prefere une marchandise perdue a un convoi gratuit.
	entreprises[eid][entStock] -= CV_UNITS;
	Ent_Save();

	cvVeh = CreateVehicle(CV_TRUCK_MODEL, px + 3.0, py, pz + 0.5, 0.0, -1, -1, 600);
	if(!Cv_VehValide(cvVeh))
	{
		entreprises[eid][entStock] += CV_UNITS;   // on rend le stock
		Ent_Save();
		cvVeh = 0;
		return Cv_Msg(playerid, "{FF4444}Impossible de sortir la camionnette, reessaie.");
	}

	// Sans cet enregistrement, car_Engine lit cGas = 0 et refuse de demarrer.
	vehicle[cvVeh][used]    = 1;
	vehicle[cvVeh][SQLID]   = 0;
	vehicle[cvVeh][cModel]  = CV_TRUCK_MODEL;
	vehicle[cvVeh][cType]   = CAR_ENTREPRISE;
	vehicle[cvVeh][cOwned]  = 2;
	vehicle[cvVeh][cGas]    = 100;
	vehicle[cvVeh][cHealth] = 1000.0;
	vehicle[cvVeh][cLock]   = 0;
	vehicle[cvVeh][cEngine] = false;

	cvState  = CV_ROUTE;
	cvEid    = eid;
	cvDriver = playerid;
	cvUnits  = CV_UNITS;
	cvValue  = CV_UNITS * CV_PRICE_UNIT;
	cvExpireTs = gettime() + CV_DUREE_MIN * 60;
	cvDestX  = cargoDepot[best][cdX];
	cvDestY  = cargoDepot[best][cdY];
	cvDestZ  = cargoDepot[best][cdZ];
	format(cvDestName, sizeof(cvDestName), "%s", cargoDepot[best][cdName]);
	format(cvEntName, sizeof(cvEntName), "%s", entreprises[eid][entName]);

	PutPlayerInVehicle(playerid, cvVeh, 0);

	// Le camion est visible de tous : c'est ce qui rend le convoi jouable.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SetPlayerMapIcon(i, CV_ICON_TRUCK, px, py, pz, 51, 0xFF8C00FF, MAPICON_GLOBAL);
	}
	SetPlayerMapIcon(playerid, CV_ICON_DEST, cvDestX, cvDestY, cvDestZ, 53, 0x00FF00FF, MAPICON_GLOBAL);

	new chatStr[200], ecran[144];
	format(chatStr, sizeof(chatStr), "[CONVOI] %s expedie un chargement de %d$ vers %s. /convoi pour le suivre.", cvEntName, cvValue, cvDestName);
	Cv_Chat(chatStr);
	Cv_Chat("[CONVOI] Police : /convoi escorter pour une part. Criminels : /convoi braquer sur place.");
	format(ecran, sizeof(ecran), "%s  ->  %s   (%d$)", cvEntName, cvDestName, cvValue);
	Annonce_All("~o~CONVOI EN ROUTE", ecran, 0xFF8C00FF, 10000);

	new me[190];
	format(me, sizeof(me), "{00FF00}Convoi charge : %d unites, valeur %d$. Direction {FFFF00}%s{FFFFFF}. Tu as %d minutes.", cvUnits, cvValue, cvDestName, CV_DUREE_MIN);
	Cv_Msg(playerid, me);
	return 1;
}

// ============================================================================
// ESCORTE
// ============================================================================
stock Cv_Escorter(playerid)
{
	if(cvState != CV_ROUTE) return Cv_Msg(playerid, "{FF4444}Aucun convoi en route.");
	if(playerid == cvDriver)  return Cv_Msg(playerid, "{FF4444}Tu conduis le convoi.");
	if(cvEscort[playerid] == 1) return Cv_Msg(playerid, "{FFFF00}Tu escortes deja ce convoi. /convoi quitter pour te retirer.");
	if(!Cv_EstLegal(playerid))
		return Cv_Msg(playerid, "{FF4444}L'escorte est reservee aux factions legales en service.");
	if(Cv_CountEscorte() >= CV_ESCORT_MAX)
		return Cv_Msg(playerid, "{FF4444}L'escorte est complete.");
	if(cvBrigand[playerid] == 1)
		return Cv_Msg(playerid, "{FF4444}Tu as participe au braquage, tu ne peux pas escorter ce convoi.");

	cvEscort[playerid] = 1;
	SetPlayerMapIcon(playerid, CV_ICON_DEST, cvDestX, cvDestY, cvDestZ, 53, 0x00FF00FF, MAPICON_GLOBAL);

	new s[176];
	format(s, sizeof(s), "{00FF00}Tu escortes le convoi. A l'arrivee, les escortes se partagent %d%% de %d$.", CV_PART_ESCORTE, cvValue);
	Cv_Msg(playerid, s);
	format(s, sizeof(s), "[CONVOI] %s rejoint l'escorte (%d/%d).", PlayerInfo[playerid][pName], Cv_CountEscorte(), CV_ESCORT_MAX);
	Cv_Chat(s);
	return 1;
}

stock Cv_Quitter(playerid)
{
	if(cvEscort[playerid] != 1) return Cv_Msg(playerid, "{FF4444}Tu n'escortes aucun convoi.");
	cvEscort[playerid] = 0;
	RemovePlayerMapIcon(playerid, CV_ICON_DEST);
	return Cv_Msg(playerid, "{FFFF00}Tu quittes l'escorte.");
}

// ============================================================================
// BRAQUAGE
// ============================================================================
stock Cv_Braquer(playerid)
{
	if(cvState != CV_ROUTE) return Cv_Msg(playerid, "{FF4444}Aucun convoi en route.");
	if(playerid == cvDriver) return Cv_Msg(playerid, "{FF4444}Tu conduis le convoi.");
	if(cvEscort[playerid] == 1) return Cv_Msg(playerid, "{FF4444}Tu escortes ce convoi.");
	if(Cv_EstLegal(playerid)) return Cv_Msg(playerid, "{FF4444}Une faction legale ne braque pas un convoi.");
	if(!Cv_VehValide(cvVeh)) return Cv_Msg(playerid, "{FF4444}Camion introuvable.");
	new Float:vx, Float:vy, Float:vz;
	GetVehiclePos(cvVeh, vx, vy, vz);
	if(!IsPlayerInRangeOfPoint(playerid, CV_RADIUS, vx, vy, vz))
		return Cv_Msg(playerid, "{FF4444}Tu dois etre a moins de 8 metres de la camionnette.");

	// Un braquage est deja en cours : le renfort est enregistre comme complice
	// et touchera sa part. Avant, le message le lui promettait mais cvBrigand
	// n'etait jamais mis a 1 : il ne recevait rien au partage.
	if(cvHold != INVALID_PLAYER_ID && cvHold != playerid)
	{
		if(cvBrigand[playerid] == 1)
			return Cv_Msg(playerid, "{FFFF00}Tu es deja sur le coup. Reste pres du camion.");
		cvBrigand[playerid] = 1;
		return Cv_Msg(playerid, "{00FF00}Tu prets main-forte au braquage : tu toucheras ta part du butin.");
	}

	cvHold = playerid;
	cvHoldSec = 0;
	cvBrigand[playerid] = 1;

	new s[190];
	format(s, sizeof(s), "{FF4444}Braquage en cours : reste pres du camion pendant %d secondes.", CV_BRAQUAGE_SEC);
	Cv_Msg(playerid, s);
	format(s, sizeof(s), "[CONVOI] Le convoi de %s est attaque !", cvEntName);
	Cv_Chat(s);
	Annonce_All("~r~CONVOI ATTAQUE", "Le chargement est en train d'etre braque", 0xFF4444FF, 8000);
	if(cvDriver != INVALID_PLAYER_ID && IsPlayerConnected(cvDriver))
		Cv_Msg(cvDriver, "{FF4444}On braque ton convoi ! Eloigne le camion ou fais-les fuir.");
	return 1;
}

// ============================================================================
// ISSUES
// ============================================================================
stock Cv_Arrivee()
{
	new eid = cvEid, valeur = cvValue;
	new partEscorte = (valeur * CV_PART_ESCORTE) / 100;
	new nEsc = Cv_CountEscorte();
	new partEnt = valeur - ((nEsc > 0) ? partEscorte : 0);

	// L'entreprise a pu etre supprimee pendant le trajet : on ne crediterait
	// alors qu'une ligne libre, reattribuee plus tard a quelqu'un d'autre.
	if(eid >= 0 && eid < ENT_MAX && entreprises[eid][entUsed] == 1)
	{
		entreprises[eid][entCash] += partEnt;
		Ent_Save();
	}

	new s[200];
	format(s, sizeof(s), "[CONVOI] Le convoi de %s est arrive a %s. %d$ pour l'entreprise.", cvEntName, cvDestName, partEnt);
	Cv_Chat(s);
	Annonce_All("~g~CONVOI LIVRE", cvEntName, 0x00FF00FF, 8000);

	if(nEsc > 0)
	{
		new parPersonne = partEscorte / nEsc;
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || cvEscort[i] != 1) continue;
			SafeGivePlayerMoney(i, parPersonne, "Escorte de convoi");
			format(s, sizeof(s), "{00FF00}Convoi arrive a bon port : {FFFF00}%d${FFFFFF} pour ton escorte.", parPersonne);
			Cv_Msg(i, s);
		}
		format(s, sizeof(s), "[CONVOI] %d escorte(s) se partagent %d$.", nEsc, partEscorte);
		Cv_Chat(s);
	}

	if(cvDriver != INVALID_PLAYER_ID && IsPlayerConnected(cvDriver))
		Cv_Msg(cvDriver, "{00FF00}Livraison reussie. La caisse de l'entreprise a ete creditee.");

	cvNextTs = gettime() + CV_COOLDOWN_MIN * 60;
	Cv_Reset();
	return 1;
}

stock Cv_Braquage_Reussi()
{
	new valeur = cvValue;
	new butin = (valeur * CV_PART_BRAQUAGE) / 100;
	new n = Cv_CountBrigands();
	if(n < 1) n = 1;
	new parPersonne = butin / n;

	new s[200];
	format(s, sizeof(s), "[CONVOI] Le convoi de %s a ete braque ! %d$ envoles.", cvEntName, butin);
	Cv_Chat(s);
	Annonce_All("~r~CONVOI BRAQUE", cvEntName, 0xFF4444FF, 8000);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || cvBrigand[i] != 1) continue;
		SafeGivePlayerMoney(i, parPersonne, "Braquage de convoi");
		format(s, sizeof(s), "{00FF00}Braquage reussi : {FFFF00}%d${FFFFFF} pour ta part.", parPersonne);
		Cv_Msg(i, s);
	}

	if(cvDriver != INVALID_PLAYER_ID && IsPlayerConnected(cvDriver))
		Cv_Msg(cvDriver, "{FF4444}Ton convoi a ete braque. L'entreprise ne touche rien.");

	cvNextTs = gettime() + CV_COOLDOWN_MIN * 60;
	Cv_Reset();
	return 1;
}

// Pas de 'const' : raison est transmis a Annonce_All, qui le passe a
// PlayerTextDrawSetString - un natif qui refuse les tableaux const.
stock Cv_Echec(raison[])
{
	new s[200];
	format(s, sizeof(s), "[CONVOI] Le convoi de %s est perdu : %s", cvEntName, raison);
	Cv_Chat(s);
	Annonce_All("~r~CONVOI PERDU", raison, 0xFF4444FF, 7000);
	if(cvDriver != INVALID_PLAYER_ID && IsPlayerConnected(cvDriver))
		Cv_Msg(cvDriver, "{FF4444}Convoi perdu : la marchandise ne sera pas payee.");
	cvNextTs = gettime() + CV_COOLDOWN_MIN * 60;
	Cv_Reset();
	return 1;
}

// ============================================================================
// TICK (5s)
// ============================================================================
forward Cv_Tick();
public Cv_Tick()
{
	if(cvState != CV_ROUTE) return 1;

	if(cvDriver == INVALID_PLAYER_ID || !IsPlayerConnected(cvDriver))
		return Cv_Echec("le convoyeur a quitte la partie"), 1;
	if(!Cv_VehValide(cvVeh))
		return Cv_Echec("la camionnette a disparu"), 1;
	if(gettime() >= cvExpireTs)
		return Cv_Echec("le delai de livraison est depasse"), 1;

	new Float:vx, Float:vy, Float:vz;
	GetVehiclePos(cvVeh, vx, vy, vz);

	// Icone du camion rafraichie pour tout le monde : le convoi reste tracable.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SetPlayerMapIcon(i, CV_ICON_TRUCK, vx, vy, vz, 51, 0xFF8C00FF, MAPICON_GLOBAL);
	}

	// --- braquage en cours ---
	if(cvHold != INVALID_PLAYER_ID)
	{
		if(!IsPlayerConnected(cvHold) || !IsPlayerInRangeOfPoint(cvHold, CV_RADIUS, vx, vy, vz))
		{
			if(IsPlayerConnected(cvHold))
				Cv_Msg(cvHold, "{FF4444}Tu t'es eloigne du camion : le braquage est interrompu.");
			cvHold = INVALID_PLAYER_ID;
			cvHoldSec = 0;
		}
		else
		{
			cvHoldSec += CV_TICK_MS / 1000;
			if(cvHoldSec >= CV_BRAQUAGE_SEC) return Cv_Braquage_Reussi(), 1;

			new reste = CV_BRAQUAGE_SEC - cvHoldSec, gt[48];
			format(gt, sizeof(gt), "~r~Braquage~n~~w~%d s", reste);
			GameTextForPlayer(cvHold, gt, CV_TICK_MS, 3);
		}
	}

	// --- arrivee ---
	// Detection par distance et non par checkpoint : le gamemode utilise deja
	// les checkpoints pour les metiers et les race checkpoints pour les courses.
	if(IsPlayerInVehicle(cvDriver, cvVeh) && IsPlayerInRangeOfPoint(cvDriver, CV_RADIUS, cvDestX, cvDestY, cvDestZ))
		return Cv_Arrivee(), 1;

	return 1;
}

// ============================================================================
// INFOS
// ============================================================================
stock Cv_Info(playerid)
{
	if(cvState != CV_ROUTE)
	{
		Cv_Msg(playerid, "{AAAAAA}Aucun convoi en route.");
		if(gettime() < cvNextTs)
		{
			new att[144];
			format(att, sizeof(att), "{AAAAAA}Prochain convoi possible dans %d minute(s).", (cvNextTs - gettime()) / 60 + 1);
			Cv_Msg(playerid, att);
		}
		Cv_Msg(playerid, "{FFFFFF}/convoi lancer {AAAAAA}: a l'entree de ton entreprise, avec 300 unites en stock.");
		return 1;
	}

	new s[200], reste = (cvExpireTs - gettime()) / 60;
	format(s, sizeof(s), "{FF8C00}[CONVOI]{FFFFFF} %s -> %s", cvEntName, cvDestName);
	Cv_Msg(playerid, s);
	format(s, sizeof(s), "{FFFFFF}Valeur : {FFFF00}%d${FFFFFF} | Escorte : {FFFF00}%d/%d{FFFFFF} | Temps restant : {FFFF00}%d min", cvValue, Cv_CountEscorte(), CV_ESCORT_MAX, reste);
	Cv_Msg(playerid, s);

	if(Cv_VehValide(cvVeh))
	{
		new Float:vx, Float:vy, Float:vz;
		GetVehiclePos(cvVeh, vx, vy, vz);
		SetPlayerMapIcon(playerid, CV_ICON_TRUCK, vx, vy, vz, 51, 0xFF8C00FF, MAPICON_GLOBAL);
		Cv_Msg(playerid, "{AAAAAA}La camionnette est marquee en orange sur ta carte.");
	}

	if(cvHold != INVALID_PLAYER_ID)
		Cv_Msg(playerid, "{FF4444}Un braquage est en cours sur ce convoi !");

	if(playerid != cvDriver && cvEscort[playerid] == 0)
	{
		if(Cv_EstLegal(playerid)) Cv_Msg(playerid, "{FFFFFF}/convoi escorter {AAAAAA}: rejoins l'escorte pour une part de la prime.");
		else Cv_Msg(playerid, "{FFFFFF}/convoi braquer {AAAAAA}: a moins de 8m de la camionnette.");
	}
	return 1;
}

// ============================================================================
// HOOKS
// ============================================================================
stock Cv_OnDisconnect(playerid)
{
	cvEscort[playerid] = 0;
	cvBrigand[playerid] = 0;
	if(cvHold == playerid) { cvHold = INVALID_PLAYER_ID; cvHoldSec = 0; }
	// Le convoyeur parti, le convoi est perdu : le tick s'en charge au prochain
	// passage, on ne fait rien ici pour ne pas manipuler l'etat en pleine
	// deconnexion.
	return 1;
}

stock Cv_OnDeath(playerid)
{
	// Mourir n'annule pas le convoi : la camionnette reste sur place et reste
	// braquable. C'est justement le moment ou la scene devient interessante.
	if(cvHold == playerid) { cvHold = INVALID_PLAYER_ID; cvHoldSec = 0; }
	return 1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Cv_Tok(const src[], &cvPos, dest[], destSize)
{
	new p = 0;
	while(src[cvPos] == ' ') cvPos++;
	while(src[cvPos] && src[cvPos] != ' ' && p < destSize - 1)
		dest[p++] = src[cvPos++];
	dest[p] = 0;
	return p;
}

stock Cv_Command(playerid, const cmdline[])
{
	new cvPos = 0, sub[24];
	Cv_Tok(cmdline, cvPos, sub, sizeof(sub));            // consomme "/convoi"
	if(Cv_Tok(cmdline, cvPos, sub, sizeof(sub)) == 0) return Cv_Info(playerid);

	if(strcmp(sub, "lancer",   true) == 0) return Cv_Lancer(playerid);
	if(strcmp(sub, "escorter", true) == 0) return Cv_Escorter(playerid);
	if(strcmp(sub, "quitter",  true) == 0) return Cv_Quitter(playerid);
	if(strcmp(sub, "braquer",  true) == 0) return Cv_Braquer(playerid);
	return Cv_Info(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Cv_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++) { cvEscort[i] = 0; cvBrigand[i] = 0; }
	cvState = CV_OFF;
	cvNextTs = 0;
	SetTimer("Cv_Tick", CV_TICK_MS, true);
	return 1;
}
