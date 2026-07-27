/*
	afrp_armeebase.inc - Choix de l'interieur du QG des armees

	Sur le meme principe que /entreprise local : le chef de chaque armee
	(Terre, Air, Marine) choisit l'interieur de sa caserne dans une liste, en
	payant de sa poche. Chaque armee peut donc avoir une base differente.

	  /armee base   -> menu des casernes disponibles

	=== POINTS DELICATS TRAITES ===

	1. ar_Exit n'etait pas sauvegarde.
	   armee_Save n'ecrivait que Interior + Spawn. Or ar_Exit est la position
	   A L'INTERIEUR d'ou l'on ressort (/armee qg). Changer d'interieur sans
	   sauvegarder ar_Exit aurait renvoye les soldats, apres un redemarrage,
	   aux coordonnees de sortie de l'ANCIEN interieur : coinces dehors du
	   decor. La requete de armee_Save a donc ete completee (les colonnes
	   Exit_* existaient deja, armee_Load les lisait).

	2. Deux armees ne peuvent pas prendre le meme interieur.
	   Les trois armees spawnent en monde virtuel 0 : deux casernes identiques
	   auraient mis la Terre et la Marine dans la meme piece, a se voir et se
	   marcher dessus. Le menu refuse un interieur deja pris.

	3. Les soldats deja a l'interieur sont deplaces.
	   Au changement de caserne, tout membre de cette armee se trouvant dans
	   l'ancien interieur est teleporte dans le nouveau, sinon il resterait
	   bloque dans un decor que plus rien ne relie a sa base.

	Les interieurs proposes sont ceux deja utilises ailleurs dans le gamemode :
	ils sont donc garantis valides et testes en jeu.

	Hooks a brancher dans LVRP.pwn :
	- OnDialogResponse -> ArmBase_OnDialog(playerid, dialogid, response, listitem)
	- /armee base      -> ArmBase_Cmd(playerid)
*/

#if defined AFRP_ARMEEBASE_INCLUDED
	#endinput
#endif
#define AFRP_ARMEEBASE_INCLUDED

#define ARMBASE_RANK        6       // rang requis (chef d'etat-major)
#define ARMBASE_ADMIN       4       // ... ou ce niveau admin
#define DIALOG_ARMBASE      9621
#define DIALOG_ARMBASE_OK   9622

enum e_ArmInt
{
	aiName[40],
	aiPrice,
	aiInt,
	Float:aiX, Float:aiY, Float:aiZ, Float:aiA
};

static const ArmInteriors[][e_ArmInt] = {
	{"Poste de garde",          500000,  1,  1199.4508,  -74.6475, 1006.2589, 180.0},
	{"Baraquement",             800000, 12,   446.3247,  509.9662, 1001.4195,   0.0},
	{"Bureau du commandement", 1200000,  3,  1494.2778, 1303.7288, 1093.2891,   0.0},
	{"Caserne",                1600000,  6,   -68.5145, 1353.8485, 1080.2109,   0.0},
	{"Salle de briefing",      2000000,  5,   316.7042,  294.6810,  998.4849,   0.0},
	{"Etat-major",             2600000,  3, -1032.7937, 1967.4984, 3001.0859, 270.0},
	{"Hangar",                 3300000,  1,  1964.0809, -349.6421, 1092.9454, 180.0},
	{"Quartier general",       4200000,  1,  -774.6643,  550.3431, 2001.0859, 270.0}
};

new armBaseMap[MAX_PLAYERS][sizeof(ArmInteriors)]; // listitem -> index catalogue

stock ArmBase_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock ArmBase_ArmyName(armId, dest[], destSize)
{
	switch(armId)
	{
		case 0: format(dest, destSize, "Armee de Terre");
		case 1: format(dest, destSize, "Armee de l'Air");
		case 2: format(dest, destSize, "Marine");
		default: format(dest, destSize, "Armee");
	}
	return 1;
}

// Vrai si une AUTRE armee occupe deja cet interieur (meme int + memes coords).
stock ArmBase_DejaPris(armId, iid)
{
	for(new a = 0; a < 3; a++)
	{
		if(a == armId) continue;
		if(ArmeeInfo[a][ar_Interior] != ArmInteriors[iid][aiInt]) continue;
		if(floatabs(ArmeeInfo[a][ar_Spawn][0] - ArmInteriors[iid][aiX]) > 1.0) continue;
		if(floatabs(ArmeeInfo[a][ar_Spawn][1] - ArmInteriors[iid][aiY]) > 1.0) continue;
		return a;
	}
	return -1;
}

stock ArmBase_Cmd(playerid)
{
	if(PlayerInfo[playerid][pMember] < 11 || PlayerInfo[playerid][pMember] > 13)
		return ArmBase_Msg(playerid, "{FF4444}Tu ne fais pas partie de l'Armee.");

	if(PlayerInfo[playerid][pRank] < ARMBASE_RANK && PlayerInfo[playerid][pAdmin] < ARMBASE_ADMIN)
	{
		new str[160];
		format(str, sizeof(str), "{FF4444}Reserve au rang %d de l'Armee (tu es rang %d) ou a un admin niveau %d.",
			ARMBASE_RANK, PlayerInfo[playerid][pRank], ARMBASE_ADMIN);
		return ArmBase_Msg(playerid, str);
	}

	new armId = PlayerInfo[playerid][pMember] - 11;
	new body[1200], str[176], armName[32], n = 0;
	ArmBase_ArmyName(armId, armName, sizeof(armName));

	format(body, sizeof(body), "{FFFFFF}Caserne de l'%s{FFFFFF}\n{AAAAAA}Tu payes de ta poche. Le changement est immediat.\n\n", armName);

	for(new i = 0; i < sizeof(ArmInteriors); i++)
	{
		new autre = ArmBase_DejaPris(armId, i);
		if(autre != -1)
		{
			new nomAutre[32];
			ArmBase_ArmyName(autre, nomAutre, sizeof(nomAutre));
			format(str, sizeof(str), "{666666}%s - deja occupe par : %s\n", ArmInteriors[i][aiName], nomAutre);
		}
		else if(ArmeeInfo[armId][ar_Interior] == ArmInteriors[i][aiInt]
			&& floatabs(ArmeeInfo[armId][ar_Spawn][0] - ArmInteriors[i][aiX]) < 1.0)
			format(str, sizeof(str), "{00FF00}%s - caserne actuelle\n", ArmInteriors[i][aiName]);
		else
			format(str, sizeof(str), "{FFFFFF}%s - {FFFF00}%d${FFFFFF}\n", ArmInteriors[i][aiName], ArmInteriors[i][aiPrice]);

		strcat(body, str);
		armBaseMap[playerid][n] = i;
		n++;
	}

	return ShowPlayerDialog(playerid, DIALOG_ARMBASE, DIALOG_STYLE_LIST, "Choisir la caserne", body, "Choisir", "Fermer");
}

stock ArmBase_Apply(playerid, iid)
{
	new armId = PlayerInfo[playerid][pMember] - 11;
	if(armId < 0 || armId > 2) return 0;
	if(iid < 0 || iid >= sizeof(ArmInteriors)) return 0;

	new autre = ArmBase_DejaPris(armId, iid);
	if(autre != -1)
	{
		new nomAutre[32], str1[160];
		ArmBase_ArmyName(autre, nomAutre, sizeof(nomAutre));
		format(str1, sizeof(str1), "{FF4444}Cette caserne est deja occupee par : %s. Choisis-en une autre.", nomAutre);
		return ArmBase_Msg(playerid, str1);
	}

	// Deja la caserne actuelle : on ne fait pas payer deux fois.
	if(ArmeeInfo[armId][ar_Interior] == ArmInteriors[iid][aiInt]
		&& floatabs(ArmeeInfo[armId][ar_Spawn][0] - ArmInteriors[iid][aiX]) < 1.0)
		return ArmBase_Msg(playerid, "{FFFF00}C'est deja ta caserne actuelle.");

	new locPrice = ArmInteriors[iid][aiPrice];
	if(PlayerInfo[playerid][pCash] < locPrice)
	{
		new str2[160];
		format(str2, sizeof(str2), "{FF4444}Il te faut %d$ pour cet amenagement (tu as %d$).", locPrice, PlayerInfo[playerid][pCash]);
		return ArmBase_Msg(playerid, str2);
	}

	// ancienZ DOIT etre capture ici : plus bas on ecrase ar_Spawn[2] avec la
	// nouvelle valeur, et le test de portee qui suit comparait alors l'ancien
	// X/Y avec le NOUVEAU Z - il echouait toujours et plus aucun soldat
	// n'etait deplace.
	new ancienInt = ArmeeInfo[armId][ar_Interior];
	new Float:ancienX = ArmeeInfo[armId][ar_Spawn][0];
	new Float:ancienY = ArmeeInfo[armId][ar_Spawn][1];
	new Float:ancienZ = ArmeeInfo[armId][ar_Spawn][2];

	SafeGivePlayerMoney(playerid, -locPrice, "Amenagement caserne armee");

	ArmeeInfo[armId][ar_Interior]  = ArmInteriors[iid][aiInt];
	ArmeeInfo[armId][ar_Spawn][0]  = ArmInteriors[iid][aiX];
	ArmeeInfo[armId][ar_Spawn][1]  = ArmInteriors[iid][aiY];
	ArmeeInfo[armId][ar_Spawn][2]  = ArmInteriors[iid][aiZ];
	ArmeeInfo[armId][ar_Spawn][3]  = ArmInteriors[iid][aiA];
	// ar_Exit = le point d'ou l'on ressort, donc a l'interieur lui aussi.
	ArmeeInfo[armId][ar_Exit][0]   = ArmInteriors[iid][aiX];
	ArmeeInfo[armId][ar_Exit][1]   = ArmInteriors[iid][aiY];
	ArmeeInfo[armId][ar_Exit][2]   = ArmInteriors[iid][aiZ];
	ArmeeInfo[armId][ar_Exit][3]   = ArmInteriors[iid][aiA];
	armee_Save(armId);

	// Les soldats restes dans l'ancienne caserne seraient bloques dans un
	// decor que plus rien ne relie a leur base : on les amene dans la nouvelle.
	new deplaces = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] != PlayerInfo[playerid][pMember]) continue;
		if(GetPlayerInterior(i) != ancienInt) continue;
		if(!IsPlayerInRangeOfPoint(i, 120.0, ancienX, ancienY, ancienZ)) continue;

		SafeTeleportPlayer(i, ArmInteriors[iid][aiX], ArmInteriors[iid][aiY], ArmInteriors[iid][aiZ],
			ArmInteriors[iid][aiInt], 0, ArmInteriors[iid][aiA]);
		SendClientMessage(i, 0xFFCC00FF, "[ARMEE] La caserne a change : tu as ete amene dans la nouvelle.");
		deplaces++;
	}

	new armName[32], str3[200];
	ArmBase_ArmyName(armId, armName, sizeof(armName));
	format(str3, sizeof(str3), "{00FF00}Caserne de l'%s changee : %s (-%d$).", armName, ArmInteriors[iid][aiName], locPrice);
	ArmBase_Msg(playerid, str3);
	if(deplaces > 0)
	{
		format(str3, sizeof(str3), "{AAAAAA}%d soldat(s) qui etaient dans l'ancienne caserne ont ete deplaces.", deplaces);
		ArmBase_Msg(playerid, str3);
	}

	format(str3, sizeof(str3), "{5B7A3A} Armee {FFFFFF} %s dispose d'une nouvelle caserne : {FFFFB2}%s{FFFFFF}.", armName, ArmInteriors[iid][aiName]);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pMember] != PlayerInfo[playerid][pMember]) continue;
		msg_Client(i, COLOR_WHITE, str3);
	}
	return 1;
}

stock ArmBase_OnDialog(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_ARMBASE) return 0;
	if(!response) return 1;
	if(listitem < 0 || listitem >= sizeof(ArmInteriors)) return 1;
	ArmBase_Apply(playerid, armBaseMap[playerid][listitem]);
	return 1;
}
