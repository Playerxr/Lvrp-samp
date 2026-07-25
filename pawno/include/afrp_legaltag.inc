/*
	afrp_legaltag.inc - Grade + faction legale flottant en dessous du joueur

	Un membre d'une faction legale (pMember 1..199 : police, FBI, gouvernement,
	CIA, San News, armee, hopital, justice - voir la garde deja utilisee dans le
	code pour differencier des gangs/mafias en pMember >= 200) porte desormais
	son GRADE et le NOM PRECIS de sa faction sous forme de label, mais SOUS le
	corps (hauteur ventre/jambes) plutot qu'au-dessus de la tete comme le tag de
	gang (afrp_gangtag.inc, offset +0.55). Ici offset +0.15 : nettement plus bas,
	distinct visuellement du tag de gang.

	Label attache au joueur (suit ses deplacements). Toujours blanc, comme le
	tag de gang, pour rester lisible sur n'importe quel decor.

	Mise a jour : OnPlayerSpawn (couvre login + respawn) + aux points cles
	d'entree/sortie de faction (invitation acceptee, /quitter faction) pour un
	affichage immediat sans attendre la mort.
*/

#if defined AFRP_LEGALTAG_INCLUDED
	#endinput
#endif
#define AFRP_LEGALTAG_INCLUDED

new Text3D:legalTagLabel[MAX_PLAYERS];
new bool:legalTagActive[MAX_PLAYERS];

stock LegalTag_Update(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;

	// Detruit l'ancien label s'il existe
	if(legalTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(legalTagLabel[playerid]);
		legalTagActive[playerid] = false;
	}

	new mem = PlayerInfo[playerid][pMember];
	if(mem < 1 || mem >= 200) return 0; // pas dans une faction legale

	new rankName[32];
	strmid(rankName, GetFactionRank(mem, PlayerInfo[playerid][pRank]), 0, 32, 32);
	new facName[32];
	strmid(facName, GetFactionName(mem), 0, 32, 32);
	if(!strlen(facName)) return 0;

	new tagColor = 0xFFFFFFFF;

	new tag[70];
	format(tag, sizeof(tag), "%s\n%s", rankName, facName);

	// Offset +0.15 : sous le corps (hauteur jambes/ventre), a l'oppose du tag
	// de gang (+0.55, au-dessus de la tete) - meme priorite (10) pour survivre
	// a la limite de labels visibles lors des rassemblements nombreux.
	legalTagLabel[playerid] = CreateDynamic3DTextLabel(tag, tagColor,
		0.0, 0.0, 0.15, 12.0, playerid, INVALID_VEHICLE_ID, 0, -1, -1, -1, LABEL_STREAM_DISTANCE, STREAMER_TAG_AREA:-1, 10);
	legalTagActive[playerid] = true;
	return 1;
}

stock LegalTag_OnConnect(playerid)
{
	legalTagActive[playerid] = false;
	return 1;
}

stock LegalTag_OnDisconnect(playerid)
{
	if(legalTagActive[playerid])
	{
		DestroyDynamic3DTextLabel(legalTagLabel[playerid]);
		legalTagActive[playerid] = false;
	}
	return 1;
}
