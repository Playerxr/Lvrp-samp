/*
	afrp_housefix.inc - Anti-doublon d'entrees de maisons

	Empeche la creation d'une maison dont l'entree serait trop proche
	d'une maison existante. Deux entrees trop proches faisaient que
	PlayerToPoint() (utilise pour detecter qu'un joueur entre chez lui)
	matchait la mauvaise maison en premier dans la boucle totalHouses,
	et le joueur ressortait ensuite chez une autre maison que la sienne.
*/

#if defined AFRP_HOUSEFIX_INCLUDED
	#endinput
#endif
#define AFRP_HOUSEFIX_INCLUDED

#define HOUSEFIX_MIN_DISTANCE 3.0

// Renvoie l'id de la maison en conflit (a moins de HOUSEFIX_MIN_DISTANCE
// unites sur les 3 axes de x,y,z), ou -1 si aucun conflit.
stock HouseFix_FindConflict(Float:x, Float:y, Float:z)
{
	for(new mc = 0; mc < totalHouses; mc++)
	{
		if(house[mc][used] != 1) continue;
		if(floatabs(house[mc][pos][0]-x) < HOUSEFIX_MIN_DISTANCE
		&& floatabs(house[mc][pos][1]-y) < HOUSEFIX_MIN_DISTANCE
		&& floatabs(house[mc][pos][2]-z) < HOUSEFIX_MIN_DISTANCE)
		{
			return mc;
		}
	}
	return -1;
}
