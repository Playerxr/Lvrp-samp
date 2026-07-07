/*
	afrp_carattack.inc - Alarme + alerte si tentative vol voiture

	Quand un joueur essaie d'entrer dans un vehicule verrouille qu'il ne
	possede pas (et qui n'appartient pas a sa faction), on declenche :
	- Alerte au proprio si online (avec coords)
	- Son d'alarme pour les joueurs autour
	- Cooldown 30s pour eviter le spam

	Hook : OnPlayerEnterVehicle (fires avant d'etre vraiment dedans)
*/

#if defined AFRP_CARATTACK_INCLUDED
	#endinput
#endif
#define AFRP_CARATTACK_INCLUDED

#define CARATTACK_COOLDOWN  30 // s

new carAttackLastTime[MAX_VEHICLES];

stock CarAttack_OnEnterVehicle(playerid, vehicleid, ispassenger)
{
	if(ispassenger) return 0;                     // passager, pas de check
	if(vehicleid <= 0 || vehicleid > MAX_VEHICLES) return 0;
	if(!IsPlayerConnected(playerid)) return 0;
	if(IsPlayerNPC(playerid)) return 0;
	if(AdminDuty[playerid] == 1) return 0;        // admins exempts

	// Vehicule pas verrouille -> pas de tentative
	if(vehicle[vehicleid][cLock] != 1) return 0;

	// Owner check : si le joueur est le proprio par nom OU par slot SQLID OU faction
	new bool:isOwner = false;
	if(strcmp(PlayerInfo[playerid][pRealName], vehicle[vehicleid][cOwner], true) == 0) isOwner = true;
	for(new s = 0; s < 6 && !isOwner; s++)
	{
		if(PlayerInfo[playerid][pCar][s] == vehicle[vehicleid][SQLID]) isOwner = true;
	}
	if(isOwner) return 0;

	// Cooldown anti-spam
	new now = gettime();
	if(now - carAttackLastTime[vehicleid] < CARATTACK_COOLDOWN) return 0;
	carAttackLastTime[vehicleid] = now;

	// Coords
	new Float:vx, Float:vy, Float:vz;
	GetVehiclePos(vehicleid, vx, vy, vz);

	// Trouver le proprio online (par nom dans cOwner)
	new ownerid = -1;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(strcmp(PlayerInfo[i][pRealName], vehicle[vehicleid][cOwner], true) == 0)
		{
			ownerid = i;
			break;
		}
	}
	if(ownerid != -1)
	{
		new amsg[180];
		format(amsg, sizeof(amsg),
			"{FF0000}[!] ALARME !{FFFFFF} Quelqu'un essaie de voler ton vehicule a {FFFF00}(%.0f, %.0f, %.0f){FFFFFF}.",
			vx, vy, vz);
		SendClientMessage(ownerid, -1, amsg);
		PlayerPlaySound(ownerid, 1145, 0.0, 0.0, 0.0); // sound alerte
	}

	// Son d'alarme pour les joueurs a moins de 30m
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		new Float:px, Float:py, Float:pz;
		GetPlayerPos(i, px, py, pz);
		new Float:d = floatsqroot((vx-px)*(vx-px) + (vy-py)*(vy-py) + (vz-pz)*(vz-pz));
		if(d <= 30.0) PlayerPlaySound(i, 1145, vx, vy, vz);
	}
	return 1;
}
