/*
	afrp_explosives.inc - Damage auto aux vehicules par explosions

	SAMP par defaut : RPG/HS Rocket/grenade/molotov ne damagent PAS les
	vehicules (les explosions sont cosmetiques cote client). Ce module
	hook OnPlayerWeaponShot et applique du damage manuel aux vehicules
	dans le rayon d'impact.

	Armes gerees :
	- RPG (35)         : 8m, 800 damage
	- HS Rocket (36)   : 10m, 1000 damage (insta-kill)
	- Grenade (16)     : 6m, 500 damage
	- Molotov (18)     : 5m, 350 damage
	- Sat Charge (39)  : 12m, 1500 damage
	- Spray (41) bonus : 2m, 50 damage (juste pour le fun)

	Hook : OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, fX, fY, fZ)
*/

#if defined AFRP_EXPLOSIVES_INCLUDED
	#endinput
#endif
#define AFRP_EXPLOSIVES_INCLUDED

// [BLINDAGE LEGAL] Vehicules des factions legales : reduction de degats.
// 0.15 = ils ne subissent que 15% des degats normaux (~5-6 RPG pour detruire).
// Ajuste cette valeur si besoin (0.0=immunite totale, 1.0=aucune reduction).
#define LEGAL_VEHICLE_DAMAGE_FACTOR  0.15

stock bool:Explosives_IsLegalVehicle(vehicleid)
{
	new t = vehicle[vehicleid][cType];
	if(t == CAR_POLICE || t == CAR_FBI || t == CAR_POMPIER || t == CAR_MEDIC ||
	   t == CAR_HOPITAL || t == CAR_JUSTICE || t == CAR_ARMEE_TERRE ||
	   t == CAR_ARMEE_AIR || t == CAR_MARINE)
		return true;
	return false;
}

stock Explosives_GetBlastInfo(weaponid, &Float:radius, &Float:damage)
{
	switch(weaponid)
	{
		case 35: { radius = 8.0;  damage = 800.0;  return 1; }  // RPG
		case 36: { radius = 10.0; damage = 1000.0; return 1; }  // HS Rocket
		case 16: { radius = 6.0;  damage = 500.0;  return 1; }  // Grenade
		case 18: { radius = 5.0;  damage = 350.0;  return 1; }  // Molotov
		case 39: { radius = 12.0; damage = 1500.0; return 1; }  // Sat Charge (C4)
	}
	return 0;
}

// Applique le degat a UN vehicule (avec blindage legal applique), gere la destruction.
stock Explosives_ApplyDamage(vehicleid, Float:damage)
{
	if(GetVehicleModel(vehicleid) == 0) return 0;

	if(Explosives_IsLegalVehicle(vehicleid))
		damage *= LEGAL_VEHICLE_DAMAGE_FACTOR;

	new Float:hp;
	GetVehicleHealth(vehicleid, hp);
	new Float:newHp = hp - damage;
	if(newHp <= 250.0)
	{
		new Float:vx, Float:vy, Float:vz;
		GetVehiclePos(vehicleid, vx, vy, vz);
		SetVehicleHealth(vehicleid, 1.0); // declenche OnVehicleDeath naturellement
		CreateExplosion(vx, vy, vz + 1.0, 7, 5.0); // bonus explosion visuelle
	}
	else
	{
		SetVehicleHealth(vehicleid, newHp);
	}
	return 1;
}

stock Explosives_OnWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
	new Float:radius, Float:damage;
	if(!Explosives_GetBlastInfo(weaponid, radius, damage)) return 0;

	new affected = 0;
	new directHit = -1;

	// [FIX AVIONS] fX/fY/fZ predit le point d'impact AU MOMENT DU TIR en suivant
	// la ligne de visee. Pour une cible qui bouge vite (avion, heli, voiture en
	// mouvement), la roquette met du temps a arriver et la cible n'est plus au
	// point predit -> le check par rayon autour de fX/Y/Z rate la cible.
	// SA-MP resout deja le raycast exact via hittype/hitid : si c'est un vehicule,
	// on l'endommage directement en plein, peu importe sa position/vitesse.
	if(hittype == BULLET_HIT_TYPE_VEHICLE && hitid >= 1 && hitid < MAX_VEHICLES)
	{
		Explosives_ApplyDamage(hitid, damage);
		directHit = hitid;
		affected++;
		new dmsg[120];
		format(dmsg, sizeof(dmsg), "{FF6666}[Explosion]{FFFFFF} Impact direct sur le vehicule #%d.", hitid);
		SendClientMessage(playerid, -1, dmsg);
	}

	// Degats de zone sur les autres vehicules proches du point d'impact au sol
	// (fX/Y/Z). Le vehicule deja touche en direct (directHit) n'est pas recompte.
	new Float:vx, Float:vy, Float:vz;
	for(new v = 1; v < MAX_VEHICLES; v++)
	{
		if(v == directHit) continue;
		if(GetVehicleModel(v) == 0) continue; // vehicule invalide
		GetVehiclePos(v, vx, vy, vz);
		new Float:d = floatsqroot((vx-fX)*(vx-fX) + (vy-fY)*(vy-fY) + (vz-fZ)*(vz-fZ));
		if(d > radius) continue;

		// Degats decroissants avec la distance (lineaire)
		new Float:actualDamage = damage * (1.0 - (d / radius));
		if(actualDamage < 50.0) continue;

		Explosives_ApplyDamage(v, actualDamage);
		affected++;
	}

	if(affected > 0)
	{
		new msg[120];
		format(msg, sizeof(msg),
			"{FF6666}[Explosion]{FFFFFF} %d vehicule(s) endommage(s) dans le rayon.",
			affected);
		SendClientMessage(playerid, -1, msg);
	}
	return affected;
}
