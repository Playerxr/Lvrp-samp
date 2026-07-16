/*
	afrp_coffrefix.inc - /v coffre fonctionne sur les gros vehicules

	Probleme : la detection du vehicule pour /v coffre utilisait un rayon
	fixe de 3.5m mesure depuis le CENTRE du vehicule. Sur un camion, un
	poids-lourd ou un conteneur (remorque), le joueur qui se place a
	l'arriere (la ou est le coffre) est a bien plus de 3.5m du centre :
	le systeme repondait "Aucun vehicule pret de vous" et rien ne
	fonctionnait (ni ouvrir, ni gestion, ni fermer).

	Solution : rayon de detection adapte a la longueur du vehicule.
	Les modeles longs (camions, remorques, bus...) beneficient d'un
	rayon eleve pour pouvoir ouvrir/gerer le coffre depuis l'arriere.
*/

#if defined AFRP_COFFREFIX_INCLUDED
	#endinput
#endif
#define AFRP_COFFREFIX_INCLUDED

// Vrai pour les vehicules "longs" : camions, poids-lourds, remorques,
// conteneurs, bus, vans de grande taille. Sur ces vehicules on autorise
// une detection du coffre a plus grande distance.
stock bool:IsLongVehicle(vmodel)
{
	switch(vmodel)
	{
		// Camions / poids-lourds
		case 403, 414, 443, 455, 456, 499, 514, 515, 524, 578, 609, 611,
		// Remorques / conteneurs
		     435, 450, 584, 591, 606, 607, 610,
		// Bus / coach
		     431, 437, 438,
		// Gros utilitaires / vans longs
		     408, 416, 433, 544, 407, 601, 428:
		{
			return true;
		}
	}
	return false;
}

// Rayon de detection du coffre pour un vehicule SA-MP donne.
stock Float:CoffreDetectRadius(vehicleid)
{
	new vmodel = GetVehicleModel(vehicleid);
	if(IsLongVehicle(vmodel)) return 9.0;   // gros vehicules : large rayon
	return 3.5;                            // vehicules normaux : rayon d'origine
}
