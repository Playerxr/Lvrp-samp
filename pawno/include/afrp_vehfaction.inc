/*
	afrp_vehfaction.inc - Commande admin /vehfaction

	Resout le bug "le membre n'a pas les cles" quand on assigne un vehicule
	a une faction. Le check des cles regarde cType (pas cOwner), donc il faut
	set les DEUX correctement.

	Usage : /vehfaction <vehid> <faction>
	  ou : /vehfaction current <faction>  -> applique au vehicule du joueur

	Factions :
	  armee_terre / armee  -> CAR_ARMEE_TERRE (53)
	  armee_air            -> CAR_ARMEE_AIR (54)
	  marine               -> CAR_MARINE (55)
	  police               -> CAR_POLICE (43)
	  fbi                  -> CAR_FBI (5)
	  pompier              -> CAR_POMPIER (44)
	  samu                 -> CAR_MEDIC (41)
	  hopital              -> CAR_HOPITAL (51)
	  justice              -> CAR_JUSTICE (52)
	  none / clear         -> CAR_CREATED (-1) (retire l'assignation)
*/

#if defined AFRP_VEHFACTION_INCLUDED
	#endinput
#endif
#define AFRP_VEHFACTION_INCLUDED

stock VehFaction_Cmd(playerid, target_carid, const factionParam[])
{
	if(PlayerInfo[playerid][pAdmin] < 3)
		return SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 3+.");
	if(target_carid <= 0)
	{
		SendClientMessage(playerid, -1, "{FFFFFF}Usage: {FFFF00}/vehfaction <vehid> <faction>{FFFFFF} OU {FFFF00}/vehfaction current <faction>");
		SendClientMessage(playerid, -1, "{8C8C8C}Factions: armee_terre, armee_air, marine, police, fbi, pompier, samu, hopital, justice, none");
		return 1;
	}
	if(target_carid > totalVehicles)
		return SendClientMessage(playerid, -1, "{FF4444}ID vehicule invalide.");

	new newType = CAR_CREATED, ownerStr[32];
	if(strcmp(factionParam, "armee_terre", true) == 0 || strcmp(factionParam, "armee", true) == 0)
		{ newType = CAR_ARMEE_TERRE; ownerStr = "Armee de Terre"; }
	else if(strcmp(factionParam, "armee_air", true) == 0)
		{ newType = CAR_ARMEE_AIR;   ownerStr = "Armee de l'Air"; }
	else if(strcmp(factionParam, "marine", true) == 0 || strcmp(factionParam, "armee_marine", true) == 0)
		{ newType = CAR_MARINE;      ownerStr = "Marine"; }
	else if(strcmp(factionParam, "police", true) == 0)
		{ newType = CAR_POLICE;      ownerStr = "Police"; }
	else if(strcmp(factionParam, "fbi", true) == 0)
		{ newType = CAR_FBI;         ownerStr = "FBI"; }
	else if(strcmp(factionParam, "pompier", true) == 0)
		{ newType = CAR_POMPIER;     ownerStr = "Pompiers"; }
	else if(strcmp(factionParam, "samu", true) == 0 || strcmp(factionParam, "medic", true) == 0)
		{ newType = CAR_MEDIC;       ownerStr = "SAMU"; }
	else if(strcmp(factionParam, "hopital", true) == 0 || strcmp(factionParam, "ems", true) == 0)
		{ newType = CAR_HOPITAL;     ownerStr = "Hopital"; }
	else if(strcmp(factionParam, "justice", true) == 0)
		{ newType = CAR_JUSTICE;     ownerStr = "Justice"; }
	else if(strcmp(factionParam, "none", true) == 0 || strcmp(factionParam, "clear", true) == 0)
		{ newType = CAR_CREATED;     ownerStr = "Aucun"; }
	else
	{
		SendClientMessage(playerid, -1, "{FF4444}Faction inconnue.");
		SendClientMessage(playerid, -1, "{8C8C8C}Valides: armee_terre, armee_air, marine, police, fbi, pompier, samu, hopital, justice, none");
		return 1;
	}

	vehicle[target_carid][cType] = newType;
	strmid(vehicle[target_carid][cOwner], ownerStr, 0, strlen(ownerStr), MAX_PLAYER_NAME);
	vehicle[target_carid][cOwned] = (newType == CAR_CREATED) ? 0 : 2; // 2 = owned by faction
	vehicle_Save(target_carid, 0, 0);

	new msg[140];
	format(msg, sizeof(msg),
		"{00FF00}[VehFaction]{FFFFFF} Vehicule {FFFF00}#%d{FFFFFF} -> {FFFF00}%s{FFFFFF} (cType=%d). Tout membre actif peut le demarrer.",
		target_carid, ownerStr, newType);
	SendClientMessage(playerid, -1, msg);
	return 1;
}
