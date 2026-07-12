/*
	afrp_gangtag.inc - Nom du gang/mafia flottant au-dessus du joueur

	Un membre d'une faction illegale (pMember >= 200 => facID = pMember-200,
	fType 1=gang, 2=mafia) porte desormais le NOM de son groupe au-dessus de la
	tete, visible par tous (ex: "MS13"). Rend les affiliations lisibles en un
	coup d'oeil, sans avoir a taper une commande.

	Label attache au joueur (suit ses deplacements). Couleur = couleur du gang
	si choisie, sinon rouge fonce pour les mafias / gris-rouge pour les gangs.

	Mise a jour : OnPlayerSpawn (couvre login + respawn) + aux points cles de
	recrutement/depart de gang pour un affichage immediat sans attendre la mort.
*/

#if defined AFRP_GANGTAG_INCLUDED
	#endinput
#endif
#define AFRP_GANGTAG_INCLUDED

new Text3D:gangTagLabel[MAX_PLAYERS];
new bool:gangTagActive[MAX_PLAYERS];

stock GangTag_Update(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;

	// Detruit l'ancien label s'il existe
	if(gangTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(gangTagLabel[playerid]);
		gangTagActive[playerid] = false;
	}

	new mem = PlayerInfo[playerid][pMember];
	if(mem < 200) return 0; // pas dans une faction illegale
	new facID = mem - 200;
	if(facID < 0 || facID >= MAX_FACTION) return 0;
	if(FactionInfo[facID][fCreate] != 1) return 0;
	if(!strlen(FactionInfo[facID][fName])) return 0;

	// [FIX] Toujours blanc, quelle que soit la couleur du gang
	new tagColor = 0xFFFFFFFF;

	new tag[40];
	format(tag, sizeof(tag), "%s", FactionInfo[facID][fName]);

	// [FIX DISPARITION] priorite elevee (10) pour que ce label survive en
	// dernier si la limite de labels visibles est atteinte (rassemblements
	// de gang/mafia nombreux) - voir Streamer_SetVisibleItems dans OnGameModeInit
	gangTagLabel[playerid] = CreateDynamic3DTextLabel(tag, tagColor,
		0.0, 0.0, 0.55, 12.0, playerid, INVALID_VEHICLE_ID, 0, -1, -1, -1, LABEL_STREAM_DISTANCE, STREAMER_TAG_AREA:-1, 10);
	gangTagActive[playerid] = true;
	return 1;
}

stock GangTag_OnConnect(playerid)
{
	gangTagActive[playerid] = false;
	return 1;
}

stock GangTag_OnDisconnect(playerid)
{
	if(gangTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(gangTagLabel[playerid]);
		gangTagActive[playerid] = false;
	}
	return 1;
}
