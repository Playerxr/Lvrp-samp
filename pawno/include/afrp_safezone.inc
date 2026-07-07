/*
	afrp_safezone.inc - Desarmement automatique en zone verte/rose

	Les degats etaient deja bloques en zone safe (turf tType 1=verte police,
	2=rose hopital), mais un joueur pouvait toujours GARDER son arme en main,
	viser, menacer, RP-threat... ce qui crée des situations de kill/abus a la
	limite de la zone (sortir d'un pas, tirer, rentrer).

	Ce module force le retour aux poings (SetPlayerArmedWeapon 0) tant que le
	joueur est physiquement dans une zone safe, sans toucher a son inventaire
	d'armes (il regarde juste son arme en quittant la zone).

	Hook : appele depuis OnPlayerUpdate (deja existant, tourne a chaque tick
	reseau du joueur - tres bon endroit pour ce genre de check leger).
*/

#if defined AFRP_SAFEZONE_INCLUDED
	#endinput
#endif
#define AFRP_SAFEZONE_INCLUDED

new safezoneWasArmed[MAX_PLAYERS]; // anti-spam du message d'avertissement

stock bool:IsPlayerInGreenZone(playerid)
{
	if(!IsPlayerConnected(playerid)) return false;
	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	for(new ti = 0; ti < MAX_TURFS; ti++)
	{
		if(Turfs[ti][tUsed] != 1) continue;
		if(Turfs[ti][tType] != 1 && Turfs[ti][tType] != 2) continue;
		if(px >= Turfs[ti][tMinX] && px <= Turfs[ti][tMaxX] &&
		   py >= Turfs[ti][tMinY] && py <= Turfs[ti][tMaxY])
		{
			return true;
		}
	}
	return false;
}

// A appeler depuis OnPlayerUpdate
stock SafeZone_OnUpdate(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(AdminDuty[playerid] == 1) return 0; // admins en service exemptes
	if(GetPlayerWeapon(playerid) == 0) { safezoneWasArmed[playerid] = 0; return 0; } // deja a mains nues

	if(IsPlayerInGreenZone(playerid))
	{
		SetPlayerArmedWeapon(playerid, 0);
		if(!safezoneWasArmed[playerid])
		{
			safezoneWasArmed[playerid] = 1;
			SendClientMessage(playerid, 0xCF9756FF,
				"{CF9756}\xbb Zone Protegee \xab{FFFFFF} Armes interdites ici. Vous rangez votre arme automatiquement.");
		}
		return 1;
	}
	safezoneWasArmed[playerid] = 0;
	return 0;
}

stock SafeZone_OnDisconnect(playerid)
{
	safezoneWasArmed[playerid] = 0;
	return 1;
}
