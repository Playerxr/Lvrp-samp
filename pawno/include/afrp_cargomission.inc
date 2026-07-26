/*
	afrp_cargomission.inc - Missions cargo d'entreprise

	Le patron achete des camionnettes d'entreprise (/entreprise camionnette),
	un admin place des depots (/cargo creer). Le patron ou ses employes prennent
	une camionnette, chargent du cargo a un depot (/cargo), roulent vers
	le depot de destination (choisi aleatoirement) et livrent (/cargo). Le
	paiement, calcule selon la distance parcourue, est verse dans la CAISSE
	de l'entreprise.

	Depend de afrp_entreprise.inc (doit etre inclus AVANT) :
	  entreprises[], Ent_FindByName, Ent_FindByOwner, Ent_FindEmployeeSlot,
	  Ent_CreditByName, entName, entCash.

	Persistance :
	  scriptfiles/cargo_depots.cfg      (depots admin)
	  scriptfiles/entreprise_camions.cfg (camions d'entreprise)
*/

#if defined AFRP_CARGOMISSION_INCLUDED
	#endinput
#endif
#define AFRP_CARGOMISSION_INCLUDED

#define CG_MAX_DEPOT        40
#define CG_MAX_TRUCK        60
#define CG_MAX_PER_ENT      3            // camions max par entreprise
#define CG_TRUCK_COST       2000000      // prix par defaut (voir CargoModels)
#define CG_TRUCK_MODEL      414          // Mule : modele par defaut si non precise
#define CG_MODEL_COUNT      6            // nb de camionnettes au catalogue
#define CG_DLG_MODEL        9985         // dialog choix du modele
#define CG_PAY_PER_UNIT     30           // $ par unite de distance (1 unite ~ 1m)
#define CG_DEPOT_RADIUS     8.0
#define CG_DEPOT_FILE       "cargo_depots.cfg"
#define CG_TRUCK_FILE       "entreprise_camions.cfg"

enum e_CargoDepot
{
	cdUsed,
	cdName[48],
	Float:cdX, Float:cdY, Float:cdZ
};
new cargoDepot[CG_MAX_DEPOT][e_CargoDepot];
new cargoDepot_Pickup[CG_MAX_DEPOT];
new Text3D:cargoDepot_Label[CG_MAX_DEPOT];
new cargo_DepotCount = 0;

enum e_CargoTruck
{
	ctUsed,
	ctEntName[64],
	ctVehicleid,
	ctModel,                         // modele choisi a l'achat (0 = defaut)
	Float:ctSpawnX, Float:ctSpawnY, Float:ctSpawnZ, Float:ctAngle
};
new cargoTruck[CG_MAX_TRUCK][e_CargoTruck];
new cargo_TruckCount = 0;

// Catalogue des camionnettes achetables. Toutes sont des utilitaires legers,
// maniables en ville, contrairement au Roadtrain (semi-remorque) d'origine.
enum e_CargoModel
{
	cmName[24],
	cmModel,
	cmPrice
};
static const CargoModels[CG_MODEL_COUNT][e_CargoModel] = {
	{"Mule",     414, 2000000},   // camionnette de livraison classique
	{"Boxville", 498, 2200000},   // fourgon de livraison
	{"Benson",   499, 2400000},   // le plus gros volume
	{"Yankee",   456, 2300000},   // camion-caisse moyen
	{"Pony",     413, 1600000},   // petit van, le moins cher
	{"Burrito",  482, 1800000}    // van rapide
};

// Retourne l'index catalogue d'un modele, -1 si inconnu.
stock Cargo_ModelIndex(cgModelId)
{
	for(new i = 0; i < CG_MODEL_COUNT; i++)
		if(CargoModels[i][cmModel] == cgModelId) return i;
	return -1;
}

// Etat mission par joueur
new cargo_MissionActive[MAX_PLAYERS]; // 0/1
new cargo_DestDepot[MAX_PLAYERS];     // depot de destination
new Float:cargo_PickupX[MAX_PLAYERS], Float:cargo_PickupY[MAX_PLAYERS], Float:cargo_PickupZ[MAX_PLAYERS];
new cargo_TruckVeh[MAX_PLAYERS];      // vehicleid utilise pour la mission

stock Cargo_Init()
{
	for(new i = 0; i < CG_MAX_DEPOT; i++)
	{
		cargoDepot[i][cdUsed] = 0;
		cargoDepot_Pickup[i] = -1;
		cargoDepot_Label[i] = Text3D:INVALID_3DTEXT_ID;
	}
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		cargoTruck[i][ctUsed] = 0;
		cargoTruck[i][ctVehicleid] = INVALID_VEHICLE_ID;
	}
	for(new p = 0; p < MAX_PLAYERS; p++)
	{
		cargo_MissionActive[p] = 0;
		cargo_DestDepot[p] = -1;
		cargo_TruckVeh[p] = INVALID_VEHICLE_ID;
	}
	Cargo_LoadDepots();
	Cargo_LoadTrucks();
	return 1;
}

stock Cargo_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

// ===================== DEPOTS =====================

stock Cargo_CreateDepotVisuals(id)
{
	if(cargoDepot[id][cdUsed] != 1) return 0;
	if(cargoDepot_Pickup[id] == -1)
		cargoDepot_Pickup[id] = CreatePickup(1239, 1, cargoDepot[id][cdX], cargoDepot[id][cdY], cargoDepot[id][cdZ], 0);
	if(_:cargoDepot_Label[id] == _:INVALID_3DTEXT_ID)
	{
		new labelStr[128];
		format(labelStr, sizeof(labelStr), "{FF8C00}DEPOT CARGO\n{FFFFFF}%s\n{CCCCCC}/cargo (dans une camionnette d'entreprise)", cargoDepot[id][cdName]);
		cargoDepot_Label[id] = Create3DTextLabel(labelStr, 0xFFFFFFFF, cargoDepot[id][cdX], cargoDepot[id][cdY], cargoDepot[id][cdZ] + 0.8, 25.0, 0, 0);
	}
	return 1;
}

stock Cargo_DestroyDepotVisuals(id)
{
	if(cargoDepot_Pickup[id] != -1) { DestroyPickup(cargoDepot_Pickup[id]); cargoDepot_Pickup[id] = -1; }
	if(_:cargoDepot_Label[id] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(cargoDepot_Label[id]);
		cargoDepot_Label[id] = Text3D:INVALID_3DTEXT_ID;
	}
	return 1;
}

stock Cargo_LoadDepots()
{
	if(!fexist(CG_DEPOT_FILE)) return 0;
	new File:f = fopen(CG_DEPOT_FILE, io_read);
	if(!f) return 0;
	new line[192], tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 3) continue;
		if(cargo_DepotCount >= CG_MAX_DEPOT) break;
		new id = cargo_DepotCount;
		new i = 0;
		Cargo_ReadField(line, i, cargoDepot[id][cdName], 48, 47);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoDepot[id][cdX] = floatstr(tmpBuf);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoDepot[id][cdY] = floatstr(tmpBuf);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoDepot[id][cdZ] = floatstr(tmpBuf);
		if(strlen(cargoDepot[id][cdName]) < 1) continue;
		cargoDepot[id][cdUsed] = 1;
		Cargo_CreateDepotVisuals(id);
		cargo_DepotCount++;
	}
	fclose(f);
	printf("[Cargo] %d depot(s) charge(s)", cargo_DepotCount);
	return cargo_DepotCount;
}

stock Cargo_SaveDepots()
{
	if(fexist(CG_DEPOT_FILE)) fremove(CG_DEPOT_FILE);
	new File:f = fopen(CG_DEPOT_FILE, io_write);
	if(!f) return 0;
	new line[192];
	for(new i = 0; i < CG_MAX_DEPOT; i++)
	{
		if(cargoDepot[i][cdUsed] != 1) continue;
		format(line, sizeof(line), "%s|%.4f|%.4f|%.4f\n", cargoDepot[i][cdName], cargoDepot[i][cdX], cargoDepot[i][cdY], cargoDepot[i][cdZ]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Cargo_NearestDepot(playerid)
{
	new Float:best = CG_DEPOT_RADIUS;
	new bid = -1;
	for(new i = 0; i < CG_MAX_DEPOT; i++)
	{
		if(cargoDepot[i][cdUsed] != 1) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, cargoDepot[i][cdX], cargoDepot[i][cdY], cargoDepot[i][cdZ]);
		if(d < best) { best = d; bid = i; }
	}
	return bid;
}

// ===================== CAMIONS =====================

stock Cargo_SpawnTruck(id)
{
	if(cargoTruck[id][ctUsed] != 1) return 0;
	// Modele choisi a l'achat ; les camionnettes d'avant le catalogue (ou une
	// valeur corrompue) retombent sur le modele par defaut.
	new mdl = cargoTruck[id][ctModel];
	if(Cargo_ModelIndex(mdl) == -1) { mdl = CG_TRUCK_MODEL; cargoTruck[id][ctModel] = mdl; }

	new vid = CreateVehicle(mdl,
		cargoTruck[id][ctSpawnX], cargoTruck[id][ctSpawnY], cargoTruck[id][ctSpawnZ],
		cargoTruck[id][ctAngle], -1, -1, -1);
	cargoTruck[id][ctVehicleid] = vid;
	if(vid == INVALID_VEHICLE_ID || vid < 1 || vid >= MAX_CAR) return 0;

	// [FIX MOTEUR] Le vehicule etait cree en CreateVehicle "brut", sans etre
	// enregistre dans le tableau vehicle[] du gamemode. Resultat : cGas valait 0
	// ("Ce vehicule n'a plus d'essence") et cType valait 0 (aucune autorisation
	// moteur), donc PERSONNE ne pouvait demarrer une camionnette cargo - seul un
	// admin en mode admin passait outre. On enregistre donc le minimum vital.
	// SQLID reste a 0 : les deux boucles de sauvegarde (AutoSave_Vehicles et le
	// reboot) ignorent SQLID <= 0, ce vehicule n'ira donc jamais polluer la BDD.
	vehicle[vid][used]        = 1;
	vehicle[vid][SQLID]       = 0;
	vehicle[vid][cModel]      = mdl;
	vehicle[vid][cType]       = CAR_ENTREPRISE;
	vehicle[vid][cOwned]      = 2;
	vehicle[vid][cGas]        = 100;
	vehicle[vid][cHealth]     = 1000.0;
	vehicle[vid][cLock]       = 0;
	vehicle[vid][cEngine]     = false;
	vehicle[vid][cLocationx]  = cargoTruck[id][ctSpawnX];
	vehicle[vid][cLocationy]  = cargoTruck[id][ctSpawnY];
	vehicle[vid][cLocationz]  = cargoTruck[id][ctSpawnZ];
	vehicle[vid][cAngle]      = cargoTruck[id][ctAngle];
	format(vehicle[vid][cDescription], 32, "Camionnette entreprise");
	return 1;
}

stock Cargo_LoadTrucks()
{
	if(!fexist(CG_TRUCK_FILE)) return 0;
	new File:f = fopen(CG_TRUCK_FILE, io_read);
	if(!f) return 0;
	new line[192], tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 3) continue;
		if(cargo_TruckCount >= CG_MAX_TRUCK) break;
		new id = cargo_TruckCount;
		new i = 0;
		Cargo_ReadField(line, i, cargoTruck[id][ctEntName], 64, 63);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoTruck[id][ctSpawnX] = floatstr(tmpBuf);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoTruck[id][ctSpawnY] = floatstr(tmpBuf);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoTruck[id][ctSpawnZ] = floatstr(tmpBuf);
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoTruck[id][ctAngle]  = floatstr(tmpBuf);
		// Modele ajoute apres coup : les anciennes lignes n'ont pas ce champ, la
		// lecture renvoie une chaine vide -> 0 -> modele par defaut (Cargo_SpawnTruck).
		Cargo_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); cargoTruck[id][ctModel]  = strval(tmpBuf);
		if(strlen(cargoTruck[id][ctEntName]) < 1) continue;
		cargoTruck[id][ctUsed] = 1;
		Cargo_SpawnTruck(id);
		cargo_TruckCount++;
	}
	fclose(f);
	printf("[Cargo] %d camion(s) d'entreprise charge(s)", cargo_TruckCount);
	return cargo_TruckCount;
}

stock Cargo_SaveTrucks()
{
	if(fexist(CG_TRUCK_FILE)) fremove(CG_TRUCK_FILE);
	new File:f = fopen(CG_TRUCK_FILE, io_write);
	if(!f) return 0;
	new line[192];
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		if(cargoTruck[i][ctUsed] != 1) continue;
		format(line, sizeof(line), "%s|%.4f|%.4f|%.4f|%.4f|%d\n",
			cargoTruck[i][ctEntName], cargoTruck[i][ctSpawnX], cargoTruck[i][ctSpawnY], cargoTruck[i][ctSpawnZ], cargoTruck[i][ctAngle],
			cargoTruck[i][ctModel]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Cargo_CountTrucksForEnt(const entName2[])
{
	new c = 0;
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		if(cargoTruck[i][ctUsed] != 1) continue;
		if(strcmp(cargoTruck[i][ctEntName], entName2, true) == 0) c++;
	}
	return c;
}

// Retourne le nom d'entreprise du camion (vehicleid), "" si pas un camion d'entreprise.
stock Cargo_TruckEntName(vehicleid, dest[], destSize)
{
	dest[0] = 0;
	if(vehicleid == INVALID_VEHICLE_ID) return 0;
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		if(cargoTruck[i][ctUsed] != 1) continue;
		if(cargoTruck[i][ctVehicleid] == vehicleid)
		{
			strcat(dest, cargoTruck[i][ctEntName], destSize);
			return 1;
		}
	}
	return 0;
}

// Achat d'une camionnette : appele depuis /entreprise camionnette.
// Affiche le catalogue ; l'achat reel se fait dans Cargo_BuyModel.
stock Cargo_BuyTruck(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron d'une entreprise.");
	if(Cargo_CountTrucksForEnt(entreprises[eid][entName]) >= CG_MAX_PER_ENT)
	{
		new msg[128];
		format(msg, sizeof(msg), "{CF9756} Entreprise {FFFFFF} Limite de %d camionnettes atteinte.", CG_MAX_PER_ENT);
		return msg_Client(playerid, COLOR_INFO, msg);
	}

	new body[400], tmp[96];
	format(tmp, sizeof(tmp), "{FFFFFF}Caisse : {FFFFB2}%d${FFFFFF}\nLa camionnette spawne devant toi.\n\n", entreprises[eid][entCash]);
	strcat(body, tmp);
	for(new k = 0; k < CG_MODEL_COUNT; k++)
	{
		format(tmp, sizeof(tmp), "%s{FFFFFF} - {FFFFB2}%d${FFFFFF}\n", CargoModels[k][cmName], CargoModels[k][cmPrice]);
		strcat(body, tmp);
	}
	return ShowPlayerDialog(playerid, CG_DLG_MODEL, DIALOG_STYLE_LIST,
		"{FF8C00}Entreprise - Choisir une camionnette", body, "Acheter", "Annuler");
}

// Achat effectif d'un modele du catalogue. Debite la caisse de l'entreprise.
stock Cargo_BuyModel(playerid, choice)
{
	if(choice < 0 || choice >= CG_MODEL_COUNT) return 0;
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron d'une entreprise.");
	if(Cargo_CountTrucksForEnt(entreprises[eid][entName]) >= CG_MAX_PER_ENT)
	{
		new msg[128];
		format(msg, sizeof(msg), "{CF9756} Entreprise {FFFFFF} Limite de %d camionnettes atteinte.", CG_MAX_PER_ENT);
		return msg_Client(playerid, COLOR_INFO, msg);
	}

	new cgPrice = CargoModels[choice][cmPrice];
	if(entreprises[eid][entCash] < cgPrice)
	{
		new msg[160];
		format(msg, sizeof(msg), "{CF9756} Entreprise {FFFFFF} Caisse insuffisante : ce modele coute {FFFFB2}%d${FFFFFF} (caisse : %d$).", cgPrice, entreprises[eid][entCash]);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	new slot = -1;
	for(new i = 0; i < CG_MAX_TRUCK; i++) { if(cargoTruck[i][ctUsed] == 0) { slot = i; break; } }
	if(slot == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Limite serveur de camionnettes atteinte.");

	entreprises[eid][entCash] -= cgPrice;
	Ent_Save();

	new Float:x, Float:y, Float:z, Float:a;
	GetPlayerPos(playerid, x, y, z);
	GetPlayerFacingAngle(playerid, a);
	// Spawn un peu devant le joueur
	x -= floatsin(a, degrees) * 6.0;
	y += floatcos(a, degrees) * 6.0;

	cargoTruck[slot][ctUsed] = 1;
	new n = 0; while(n < 63 && entreprises[eid][entName][n]) { cargoTruck[slot][ctEntName][n] = entreprises[eid][entName][n]; n++; } cargoTruck[slot][ctEntName][n] = 0;
	cargoTruck[slot][ctModel]  = CargoModels[choice][cmModel];
	cargoTruck[slot][ctSpawnX] = x; cargoTruck[slot][ctSpawnY] = y; cargoTruck[slot][ctSpawnZ] = z;
	cargoTruck[slot][ctAngle] = a;
	cargo_TruckCount++;
	Cargo_SpawnTruck(slot);
	Cargo_SaveTrucks();

	new okStr[210];
	format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} Camionnette {FFFFB2}%s{FFFFFF} achetee (-%d$ de la caisse). Elle spawne ici. Utilise-la pour les missions /cargo.",
		CargoModels[choice][cmName], cgPrice);
	return msg_Client(playerid, COLOR_WHITE, okStr);
}

// Dispatcher dialog (a appeler depuis OnDialogResponse)
stock Cargo_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != CG_DLG_MODEL) return 0;
	if(!response) return 1;
	Cargo_BuyModel(playerid, listitem);
	return 1;
}

// Liste des camionnettes de l'entreprise (suppression).
public Ent_ShowGestVeh(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");

	new body[600], tmp[96], c = 0;
	strcat(body, "{FFFFFF}Choisis une camionnette pour la supprimer :\n\n");
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		if(cargoTruck[i][ctUsed] != 1) continue;
		if(strcmp(cargoTruck[i][ctEntName], entreprises[eid][entName], true) != 0) continue;
		new mi = Cargo_ModelIndex(cargoTruck[i][ctModel]);
		new mName[24];
		if(mi != -1) strcat(mName, CargoModels[mi][cmName]);
		else         strcat(mName, "Camionnette");
		format(tmp, sizeof(tmp), "%s\n", mName);
		strcat(body, tmp);
		c++;
	}
	if(c == 0)
	{
		msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Aucune camionnette. /entreprise camionnette pour en acheter une.");
		return Ent_ShowGestion(playerid);
	}
	return ShowPlayerDialog(playerid, ENT_DLG_GEST_VEH, DIALOG_STYLE_LIST,
		"{42AAFF}Entreprise - Camionnettes", body, "Supprimer", "Retour");
}

// Supprime la Nieme camionnette de l'entreprise (index d'affichage).
public Ent_DeleteVeh(playerid, listitem)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return 0;
	new c = 0;
	for(new i = 0; i < CG_MAX_TRUCK; i++)
	{
		if(cargoTruck[i][ctUsed] != 1) continue;
		if(strcmp(cargoTruck[i][ctEntName], entreprises[eid][entName], true) != 0) continue;
		if(c == listitem)
		{
			new mi = Cargo_ModelIndex(cargoTruck[i][ctModel]);
			new vname[24];
			if(mi != -1) strcat(vname, CargoModels[mi][cmName]);
			else         strcat(vname, "Camionnette");

			// Ejecte les occupants avant de detruire le vehicule.
			new vid = cargoTruck[i][ctVehicleid];
			if(vid != INVALID_VEHICLE_ID && vid >= 1 && vid < MAX_CAR)
			{
				for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
				{
					if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
					if(GetPlayerVehicleID(pp) == vid) RemovePlayerFromVehicle(pp);
				}
				DestroyVehicle(vid);
				vehicle[vid][used] = 0;      // libere le slot du gamemode
				vehicle[vid][cType] = 0;
			}
			cargoTruck[i][ctUsed] = 0;
			cargoTruck[i][ctVehicleid] = INVALID_VEHICLE_ID;
			cargo_TruckCount--; if(cargo_TruckCount < 0) cargo_TruckCount = 0;
			Cargo_SaveTrucks();

			new okStr[160];
			format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} Camionnette {FFFFB2}%s{FFFFFF} supprimee. (aucun remboursement)", vname);
			msg_Client(playerid, COLOR_WHITE, okStr);
			return 1;
		}
		c++;
	}
	return 0;
}

// ===================== MISSIONS =====================

// Verifie que le joueur a le droit d'utiliser ce camion (patron ou employe de l'ent).
stock Cargo_PlayerCanUse(playerid, const entName2[])
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid != -1 && strcmp(entreprises[eid][entName], entName2, true) == 0) return 1;
	new empEid, empSlot;
	if(Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], empEid, empSlot))
	{
		if(empEid != -1 && strcmp(entreprises[empEid][entName], entName2, true) == 0) return 1;
	}
	return 0;
}

stock Float:Cargo_Distance2D(Float:x1, Float:y1, Float:x2, Float:y2)
{
	return floatsqroot((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

stock Cargo_StartMission(playerid, pickupDepot)
{
	// Choisit un depot de destination different, le plus loin possible parmi
	// un tirage simple (evite meme depot).
	new candidates[CG_MAX_DEPOT], nbCand = 0;
	for(new i = 0; i < CG_MAX_DEPOT; i++)
	{
		if(cargoDepot[i][cdUsed] != 1) continue;
		if(i == pickupDepot) continue;
		candidates[nbCand++] = i;
	}
	if(nbCand == 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Il faut au moins 2 depots pour une mission. Demande a un admin.");

	new dest = candidates[random(nbCand)];
	cargo_MissionActive[playerid] = 1;
	cargo_DestDepot[playerid] = dest;
	cargo_PickupX[playerid] = cargoDepot[pickupDepot][cdX];
	cargo_PickupY[playerid] = cargoDepot[pickupDepot][cdY];
	cargo_PickupZ[playerid] = cargoDepot[pickupDepot][cdZ];
	cargo_TruckVeh[playerid] = GetPlayerVehicleID(playerid);

	SetPlayerCheckpoint(playerid, cargoDepot[dest][cdX], cargoDepot[dest][cdY], cargoDepot[dest][cdZ], 6.0);

	new Float:dist = Cargo_Distance2D(cargoDepot[pickupDepot][cdX], cargoDepot[pickupDepot][cdY], cargoDepot[dest][cdX], cargoDepot[dest][cdY]);
	new estPay = floatround(dist) * CG_PAY_PER_UNIT;
	new msg[200];
	format(msg, sizeof(msg), "{FF8C00} Cargo {FFFFFF} Chargement OK ! Livre a {FFFFB2}%s{FFFFFF} (point rouge). Paiement estime : {FFFFB2}%d${FFFFFF} en caisse.", cargoDepot[dest][cdName], estPay);
	msg_Client(playerid, COLOR_WHITE, msg);
	return 1;
}

stock Cargo_DeliverMission(playerid, atDepot, const entName2[])
{
	if(atDepot != cargo_DestDepot[playerid])
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Ce n'est pas le bon depot de livraison.");

	new Float:dist = Cargo_Distance2D(cargo_PickupX[playerid], cargo_PickupY[playerid], cargoDepot[atDepot][cdX], cargoDepot[atDepot][cdY]);
	new pay = floatround(dist) * CG_PAY_PER_UNIT;
	if(pay < 1000) pay = 1000;

	// Verse dans la caisse de l'entreprise du camion
	Ent_CreditByName(entName2, pay);

	cargo_MissionActive[playerid] = 0;
	cargo_DestDepot[playerid] = -1;
	cargo_TruckVeh[playerid] = INVALID_VEHICLE_ID;
	DisablePlayerCheckpoint(playerid);
	Obj_Progress(playerid, OBJ_CARGO, 1);   // [OBJECTIFS] livraison reussie

	new msg[200];
	format(msg, sizeof(msg), "{FF8C00} Cargo {FFFFFF} Livraison reussie ! {FFFFB2}+%d${FFFFFF} verses dans la caisse de {FFFFB2}%s{FFFFFF}.", pay, entName2);
	msg_Client(playerid, COLOR_WHITE, msg);
	return 1;
}

// Dispatcher /cargo
stock Cargo_Cmd(playerid, params[])
{
	new sub[16], rest[128];
	new i = 0, p = 0;
	while(p < 15 && params[i] && params[i] != ' ') { sub[p++] = params[i++]; }
	sub[p] = 0;
	while(params[i] == ' ') i++;
	p = 0;
	while(p < 127 && params[i]) { rest[p++] = params[i++]; }
	rest[p] = 0;

	// -------- admin : creer / supprimer / lister --------
	if(strcmp(sub, "creer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(strlen(rest) < 2)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /cargo creer <nom du depot>");
		if(cargo_DepotCount >= CG_MAX_DEPOT)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Cargo {FFFFFF} Limite de depots atteinte.");
		new id = -1;
		for(new k = 0; k < CG_MAX_DEPOT; k++) { if(cargoDepot[k][cdUsed] == 0) { id = k; break; } }
		if(id == -1) return msg_Client(playerid, COLOR_INFO, "{FF0000} Cargo {FFFFFF} Pas de slot libre.");
		cargoDepot[id][cdUsed] = 1;
		new n = 0; while(n < 47 && rest[n]) { cargoDepot[id][cdName][n] = rest[n]; n++; } cargoDepot[id][cdName][n] = 0;
		GetPlayerPos(playerid, cargoDepot[id][cdX], cargoDepot[id][cdY], cargoDepot[id][cdZ]);
		cargo_DepotCount++;
		Cargo_CreateDepotVisuals(id);
		Cargo_SaveDepots();
		new okStr[160];
		format(okStr, sizeof(okStr), "{FF8C00} Cargo {FFFFFF} Depot '%s' cree (ID %d).", cargoDepot[id][cdName], id);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}
	if(strcmp(sub, "supprimer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		new id = Cargo_NearestDepot(playerid);
		if(id == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Aucun depot a proximite.");
		Cargo_DestroyDepotVisuals(id);
		cargoDepot[id][cdUsed] = 0;
		cargo_DepotCount--; if(cargo_DepotCount < 0) cargo_DepotCount = 0;
		Cargo_SaveDepots();
		return msg_Client(playerid, COLOR_WHITE, "{FF8C00} Cargo {FFFFFF} Depot supprime.");
	}
	if(strcmp(sub, "lister", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 1)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve staff.");
		new body[1536], tmp[128], c = 0;
		strcat(body, "{FFFFFF}Depots cargo :\n\n");
		for(new k = 0; k < CG_MAX_DEPOT; k++)
		{
			if(cargoDepot[k][cdUsed] != 1) continue;
			format(tmp, sizeof(tmp), "ID %d : %s (%.0f, %.0f)\n", k, cargoDepot[k][cdName], cargoDepot[k][cdX], cargoDepot[k][cdY]);
			strcat(body, tmp); c++;
		}
		if(c == 0) strcat(body, "Aucun depot.");
		return ShowPlayerDialog(playerid, 9993, DIALOG_STYLE_MSGBOX, "Depots cargo", body, "OK", "");
	}
	if(strcmp(sub, "annuler", true) == 0)
	{
		if(cargo_MissionActive[playerid] != 1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Aucune mission en cours.");
		cargo_MissionActive[playerid] = 0;
		cargo_DestDepot[playerid] = -1;
		cargo_TruckVeh[playerid] = INVALID_VEHICLE_ID;
		DisablePlayerCheckpoint(playerid);
		return msg_Client(playerid, COLOR_WHITE, "{FF8C00} Cargo {FFFFFF} Mission annulee.");
	}

	// -------- joueur : /cargo (sans arg) charge ou livre selon le contexte --------
	if(!IsPlayerInAnyVehicle(playerid))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Monte dans une camionnette de ton entreprise, pres d'un depot.");

	new veh = GetPlayerVehicleID(playerid);
	new entName2[64];
	if(!Cargo_TruckEntName(veh, entName2, sizeof(entName2)))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Ce vehicule n'est pas une camionnette d'entreprise.");
	if(!Cargo_PlayerCanUse(playerid, entName2))
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Tu n'es ni patron ni employe de cette entreprise.");

	new depot = Cargo_NearestDepot(playerid);
	if(depot == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Cargo {FFFFFF} Approche-toi d'un depot cargo.");

	if(cargo_MissionActive[playerid] == 1)
	{
		// Livraison
		return Cargo_DeliverMission(playerid, depot, entName2);
	}
	// Chargement
	return Cargo_StartMission(playerid, depot);
}
