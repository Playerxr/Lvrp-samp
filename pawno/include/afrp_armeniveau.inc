/*
	afrp_armeniveau.inc - Acces aux armes par NIVEAU de joueur

	Remplace le port d'arme a 20.000.000$. Le permis etait un mur d'argent :
	un joueur riche s'equipait le premier jour, un joueur assidu mais pauvre
	restait desarme des mois. Le niveau recompense le temps de jeu, pas le
	portefeuille, et il etale l'armement au lieu de l'ouvrir d'un coup.

	  /armes   -> tableau des niveaux et de ce qu'ils debloquent

	=== PROGRESSION ===
	  Niveau 1 a 3  : aucune arme a feu
	  Niveau 4      : 9mm (le premier pistolet)
	  Niveau 6      : 9mm silencieux, taser
	  Niveau 8      : fusil a pompe, micro-uzi
	  Niveau 10     : Desert Eagle, MP5
	  Niveau 12     : fusil a canon scie, fusil de combat, Tec-9
	  Niveau 14     : AK-47, M4
	  Niveau 16     : carabine, fusil a lunette
	  Niveau 18     : grenades, cocktails molotov

	Lance-roquettes, minigun, lance-flammes et explosifs telecommandes ne sont
	accessibles a AUCUN niveau : ils restent reserves aux factions et aux
	admins, qui passent outre ce systeme.

	=== OU LE CONTROLE EST POSE ===
	Dans SafeGivePlayerWeapon, l'entonnoir unique par lequel TOUTE arme est
	donnee dans le gamemode : boutique, ramassage au sol, restauration apres
	crash, coffre de vehicule. Poser le controle ailleurs aurait laisse autant
	de portes derobees qu'il existe de sources d'armes.

	=== QUI PASSE OUTRE ===
	  - les admins en service (AdminDuty)
	  - les membres des factions legales (pMember 1 a 199) : leur armement de
	    service ne depend pas de leur niveau de personnage
	  - les armes blanches, objets et utilitaires, toujours libres

	Hooks a brancher dans LVRP.pwn :
	- SafeGivePlayerWeapon -> ArmNiv_Autorise(plyid, weaponid)
	- /armes               -> ArmNiv_Cmd(playerid)
*/

#if defined AFRP_ARMENIVEAU_INCLUDED
	#endinput
#endif
#define AFRP_ARMENIVEAU_INCLUDED

#define ARMNIV_JAMAIS      999      // aucun niveau ne debloque cette arme
#define DIALOG_ARMNIV      9631

// Niveau requis pour chaque arme SA-MP (index = weaponid, 0 a 46).
// 0 = libre (armes blanches, objets, utilitaires).
new armNivRequis[47] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   //  0-9  poings et armes blanches
	0,  0,  0,  0,  0,  0,                   // 10-15 objets (fleurs, canne...)
	18, 18, 18,                              // 16-18 grenades, gaz, molotov
	ARMNIV_JAMAIS, ARMNIV_JAMAIS, ARMNIV_JAMAIS, // 19-21 inutilises
	4,                                       // 22 9mm       <- premiere arme
	6,                                       // 23 9mm silencieux
	10,                                      // 24 Desert Eagle
	8,                                       // 25 fusil a pompe
	12,                                      // 26 canon scie
	12,                                      // 27 fusil de combat
	8,                                       // 28 micro-uzi
	10,                                      // 29 MP5
	14,                                      // 30 AK-47
	14,                                      // 31 M4
	12,                                      // 32 Tec-9
	16,                                      // 33 carabine
	16,                                      // 34 fusil a lunette
	ARMNIV_JAMAIS,                           // 35 lance-roquettes
	ARMNIV_JAMAIS,                           // 36 roquette a tete chercheuse
	ARMNIV_JAMAIS,                           // 37 lance-flammes
	ARMNIV_JAMAIS,                           // 38 minigun
	ARMNIV_JAMAIS,                           // 39 explosif telecommande
	ARMNIV_JAMAIS,                           // 40 detonateur
	0,  0,  0,  0,  0,  0                    // 41-46 spray, extincteur, appareil photo, lunettes, parachute
};

new armNivNom[47][24] = {
	"Poings", "Poing americain", "Club de golf", "Matraque", "Couteau",
	"Batte", "Pelle", "Queue de billard", "Katana", "Tronconneuse",
	"Objet", "Objet", "Objet", "Objet", "Objet", "Objet",
	"Grenade", "Grenade a gaz", "Cocktail molotov",
	"-", "-", "-",
	"9mm", "9mm silencieux", "Desert Eagle", "Fusil a pompe",
	"Canon scie", "Fusil de combat", "Micro-uzi", "MP5",
	"AK-47", "M4", "Tec-9", "Carabine", "Fusil a lunette",
	"Lance-roquettes", "Roquette chercheuse", "Lance-flammes", "Minigun",
	"Explosif telecommande", "Detonateur",
	"Bombe de peinture", "Extincteur", "Appareil photo",
	"Lunettes vision nuit", "Lunettes thermiques", "Parachute"
};

// Un message par joueur et par arme suffirait a inonder le chat quand un
// systeme tente de rendre plusieurs armes d'affilee (restauration apres
// crash). On ne previent qu'une fois toutes les 10 secondes.
new armNivDernierMsg[MAX_PLAYERS];

stock ArmNiv_NiveauRequis(weaponid)
{
	if(weaponid < 0 || weaponid > 46) return ARMNIV_JAMAIS;
	return armNivRequis[weaponid];
}

// Coeur du systeme : appele par SafeGivePlayerWeapon avant toute remise d'arme.
stock ArmNiv_Autorise(plyid, weaponid)
{
	if(!IsPlayerConnected(plyid) || IsPlayerNPC(plyid)) return 1;
	if(weaponid < 0 || weaponid > 46) return 1;   // deja filtre par l'appelant

	new requis = armNivRequis[weaponid];
	if(requis == 0) return 1;                     // arme blanche ou utilitaire

	// Admin en service : outil de travail, pas de blocage.
	if(AdminDuty[plyid] == 1) return 1;

	// Faction legale : l'armement de service ne depend pas du niveau du
	// personnage. Un jeune recrue de police doit pouvoir porter son arme.
	if(PlayerInfo[plyid][pMember] >= 1 && PlayerInfo[plyid][pMember] < 200) return 1;

	if(PlayerInfo[plyid][pLevel] >= requis && requis != ARMNIV_JAMAIS) return 1;

	// Refus : on previent, mais pas en boucle.
	if(gettime() - armNivDernierMsg[plyid] >= 10)
	{
		armNivDernierMsg[plyid] = gettime();
		new s[190];
		if(requis == ARMNIV_JAMAIS)
			format(s, sizeof(s), "{FF4444} Armes {FFFFFF} %s est reservee aux factions. Tu ne peux pas la porter.", armNivNom[weaponid]);
		else
			format(s, sizeof(s), "{FF4444} Armes {FFFFFF} %s demande le niveau {FFFFB2}%d{FFFFFF} (tu es niveau %d). Tape /armes.", armNivNom[weaponid], requis, PlayerInfo[plyid][pLevel]);
		msg_Client(plyid, COLOR_WHITE, s);
	}
	return 0;
}

// ============================================================================
// /armes
// ============================================================================
stock ArmNiv_Cmd(playerid)
{
	new body[1400], str[176];
	format(body, sizeof(body), "{FFFFFF}Ton niveau : {FFFF00}%d{FFFFFF}\n{AAAAAA}Les armes se debloquent avec le niveau, pas avec l'argent.\n\n", PlayerInfo[playerid][pLevel]);

	// Une ligne par palier, dans l'ordre, plutot qu'une ligne par arme :
	// 25 lignes seraient illisibles dans un dialogue.
	new paliers[8] = { 4, 6, 8, 10, 12, 14, 16, 18 };
	for(new p = 0; p < 8; p++)
	{
		new niv = paliers[p], liste[144];
		liste[0] = 0;
		for(new w = 16; w <= 40; w++)
		{
			if(armNivRequis[w] != niv) continue;
			if(strlen(liste) > 0) strcat(liste, ", ");
			strcat(liste, armNivNom[w]);
		}
		if(!strlen(liste)) continue;

		if(PlayerInfo[playerid][pLevel] >= niv)
			format(str, sizeof(str), "{00FF00}Niveau %d{FFFFFF} - %s\n", niv, liste);
		else
			format(str, sizeof(str), "{888888}Niveau %d - %s\n", niv, liste);
		strcat(body, str);
	}

	strcat(body, "\n{AAAAAA}Niveaux 1 a 3 : aucune arme a feu.\n");
	strcat(body, "{AAAAAA}Lance-roquettes, minigun, lance-flammes et explosifs sont reserves aux factions.\n");
	strcat(body, "{AAAAAA}Les armes blanches restent libres a tous les niveaux.");

	return ShowPlayerDialog(playerid, DIALOG_ARMNIV, DIALOG_STYLE_MSGBOX, "Armes et niveaux", body, "Fermer", "");
}

stock ArmNiv_OnConnect(playerid)
{
	armNivDernierMsg[playerid] = 0;
	return 1;
}
