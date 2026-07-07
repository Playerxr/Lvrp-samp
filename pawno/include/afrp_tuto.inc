/*
	afrp_tuto.inc - Tutoriel interactif pour nouveaux joueurs

	Declenche automatiquement au premier spawn (PlayerInfo[playerid][pLevel] <= 1).
	Egalement accessible via /tuto a tout moment.

	Dialogs : 9400 a 9407 (8 etapes)
	Handler ajoute dans OnDialogResponse de LVRP.pwn.
*/

#define DIALOG_TUTO_WELCOME    9400
#define DIALOG_TUTO_RP         9401
#define DIALOG_TUTO_CONTROLS   9402
#define DIALOG_TUTO_COMMANDS   9403
#define DIALOG_TUTO_JOBS       9404
#define DIALOG_TUTO_PHONE      9405
#define DIALOG_TUTO_FACTIONS   9406
#define DIALOG_TUTO_DONE       9407

forward Tuto_Start(playerid);
public Tuto_Start(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	Tuto_ShowStep(playerid, 0);
	return 1;
}

stock Tuto_ShowStep(playerid, tStep)
{
	switch(tStep)
	{
		case 0: // Welcome
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_WELCOME, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Bienvenue sur AFRP-RP <<",
				"{FFFFFF}Salut et bienvenue sur notre serveur RolePlay francais !\n\n\
{FFFF00}Ce serveur est base sur le RolePlay (RP).\n\
{FFFFFF}Tu vas incarner un personnage et vivre une vie dans Los Santos.\n\n\
Ce mini-tuto en {00FF00}7 etapes{FFFFFF} va te montrer l'essentiel.\n\
Tu peux le relancer a tout moment avec {FFFF00}/tuto{FFFFFF}.\n\n\
{8C8C8C}Etape 1/7",
				"Suivant", "Passer");
		}
		case 1: // RP basics
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_RP, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Qu'est-ce que le RP ? <<",
				"{FFFFFF}Le {FFFF00}RolePlay{FFFFFF} c'est jouer un role :\n\n\
{00FF00}-{FFFFFF} Ton personnage a une vie, un metier, des amis.\n\
{00FF00}-{FFFFFF} Tu parles {FFFF00}en jeu{FFFFFF} comme si c'etait reel.\n\
{00FF00}-{FFFFFF} Pas de {FF0000}DM{FFFFFF} (deathmatch sans raison RP).\n\
{00FF00}-{FFFFFF} Pas de {FF0000}fly{FFFFFF}, {FF0000}aimbot{FFFFFF} ou autre triche.\n\n\
Quand tu meurs : {FFFF00}ton perso est blesse{FFFFFF}, il ne se souvient pas du tueur (regle PK).\n\n\
{8C8C8C}Etape 2/7",
				"Suivant", "Retour");
		}
		case 2: // Controls
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_CONTROLS, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Controles de base <<",
				"{FFFFFF}{FFFF00}Deplacements :{FFFFFF}\n\
{00FF00}-{FFFFFF} ZQSD / Fleches : marcher\n\
{00FF00}-{FFFFFF} Shift : courir\n\
{00FF00}-{FFFFFF} Espace : sauter / freiner\n\
{00FF00}-{FFFFFF} F (en voiture) : entrer/sortir\n\n\
{FFFF00}Interactions :{FFFFFF}\n\
{00FF00}-{FFFFFF} N : parler/interagir avec un PNJ ou objet\n\
{00FF00}-{FFFFFF} Y : verrouiller/deverrouiller une voiture\n\
{00FF00}-{FFFFFF} T : chat IC (parler en jeu)\n\
{00FF00}-{FFFFFF} K : ouvrir le menu d'aide rapide\n\n\
{8C8C8C}Etape 3/7",
				"Suivant", "Retour");
		}
		case 3: // RP Commands
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_COMMANDS, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Commandes RP essentielles <<",
				"{FFFFFF}{FFFF00}Pour le RP :{FFFFFF}\n\
{00FF00}/me {FFFFFF}<action>  -  decrire ce que ton perso fait\n\
   {8C8C8C}Ex : /me sort son telephone\n\
{00FF00}/do {FFFFFF}<description>  -  decrire l'environnement\n\
   {8C8C8C}Ex : /do La pluie tombe fort\n\
{00FF00}/b {FFFFFF}<msg>  -  parler {FFFF00}HORS RP{FFFFFF} (entre crochets)\n\
{00FF00}/pm {FFFFFF}<id> <msg>  -  message prive HRP\n\n\
{FFFF00}Infos :{FFFFFF}\n\
{00FF00}/stats{FFFFFF}  -  voir ton perso  |  {00FF00}/aide{FFFFFF}  -  menu d'aide\n\
{00FF00}/regles{FFFFFF}  -  reglement du serveur\n\n\
{8C8C8C}Etape 4/7",
				"Suivant", "Retour");
		}
		case 4: // Jobs
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_JOBS, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Gagner de l'argent : les jobs <<",
				"{FFFFFF}Tu commences avec {FFFF00}25 000${FFFFFF} de bonus.\n\
Pour gagner plus, fais un {FFFF00}metier{FFFFFF} :\n\n\
{00FF00}Tape /jobs{FFFFFF}  -  liste de tous les metiers (13 dispo)\n\
{00FF00}-{FFFFFF} Pizza, Eboueur, Pecheur, Mecanicien, Taxi...\n\
{00FF00}-{FFFFFF} Clique sur un metier : un {FF0000}point rouge{FFFFFF} apparait sur ta carte.\n\
{00FF00}-{FFFFFF} Va voir l'employeur puis touche {FFFF00}N{FFFFFF} a cote de lui.\n\
{00FF00}-{FFFFFF} Fais les missions, le {FFFF00}PayDay{FFFFFF} tombe toutes les 30 min de travail.\n\n\
{FFAA00}Astuce :{FFFFFF} commence par {FFFF00}Pizza{FFFFFF} ou {FFFF00}Eboueur{FFFFFF}, ils sont les plus simples.\n\n\
{8C8C8C}Etape 5/7",
				"Suivant", "Retour");
		}
		case 5: // Phone
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_PHONE, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Le telephone <<",
				"{FFFFFF}Le telephone te permet de :\n\
{00FF00}-{FFFFFF} Envoyer des SMS et appels\n\
{00FF00}-{FFFFFF} Acceder a ta banque\n\
{00FF00}-{FFFFFF} Voir les contacts d'urgence (Police, SAMU...)\n\
{00FF00}-{FFFFFF} Twitter local\n\n\
{FFFF00}Comment l'obtenir :{FFFFFF}\n\
{00FF00}1.{FFFFFF} Repere les {FFFF00}icones telephone{FFFFFF} sur ta carte (3 marches).\n\
{00FF00}2.{FFFFFF} Va a un marche.\n\
{00FF00}3.{FFFFFF} Tape {FFFF00}/shop{FFFFFF} pour acheter un telephone.\n\
{00FF00}4.{FFFFFF} Tape {FFFF00}/mobile{FFFFFF} pour l'ouvrir !\n\n\
{8C8C8C}Etape 6/7",
				"Suivant", "Retour");
		}
		case 6: // Factions
		{
			ShowPlayerDialog(playerid, DIALOG_TUTO_FACTIONS, DIALOG_STYLE_MSGBOX,
				"{FF6600}>> Les factions <<",
				"{FFFFFF}Tu peux rejoindre une {FFFF00}faction{FFFFFF} pour vivre une vie organisee :\n\n\
{2B6AFF}Police, SAMU, Pompiers, Justice, Armee{FFFFFF}  -  legals\n\
{FF3333}Mafias, Gangs, Cartels{FFFFFF}  -  illegaux\n\
{8B4513}Entreprises{FFFFFF}  -  Mecanicien, Taxi, Concession...\n\n\
{FFFF00}Pour rejoindre :{FFFFFF}\n\
{00FF00}-{FFFFFF} Trouve un membre de la faction en jeu.\n\
{00FF00}-{FFFFFF} Fais ton RP avec lui pour postuler.\n\
{00FF00}-{FFFFFF} Le chef te recrute s'il accepte.\n\n\
Tape {FFFF00}/factions{FFFFFF} pour voir la liste.\n\n\
{8C8C8C}Etape 7/7",
				"Suivant", "Retour");
		}
		case 7: // Done
		{
			SetPVarInt(playerid, "TutoDone", 1);
			ShowPlayerDialog(playerid, DIALOG_TUTO_DONE, DIALOG_STYLE_MSGBOX,
				"{00FF00}>> Tutoriel termine ! <<",
				"{FFFFFF}Bravo, tu connais maintenant les bases.\n\n\
{FFFF00}Recapitulatif des commandes utiles :{FFFFFF}\n\
{00FF00}/jobs{FFFFFF} - liste metiers   |   {00FF00}/shop{FFFFFF} - acheter phone\n\
{00FF00}/mobile{FFFFFF} - ouvrir phone   |   {00FF00}/aide{FFFFFF} - menu aide\n\
{00FF00}/me /do /b{FFFFFF} - RP   |   {00FF00}/regles{FFFFFF} - reglement\n\
{00FF00}/tuto{FFFFFF} - revoir ce tutoriel\n\n\
{FFAA00}>>> Bonus :{FFFFFF} tu recois {FFFF00}5 000${FFFFFF} pour avoir termine le tuto !\n\n\
Amuse-toi bien sur {FF6600}AFRP-RP{FFFFFF} !",
				"OK", "");
			GivePlayerMoney(playerid, 5000);
			PlayerInfo[playerid][pCash] += 5000;
		}
	}
	return 1;
}

// Handler dialog : a appeler depuis OnDialogResponse de LVRP.pwn
stock Tuto_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	#pragma unused listitem, inputtext
	new tStep = -1;
	switch(dialogid)
	{
		case DIALOG_TUTO_WELCOME:  { tStep = 0; }
		case DIALOG_TUTO_RP:       { tStep = 1; }
		case DIALOG_TUTO_CONTROLS: { tStep = 2; }
		case DIALOG_TUTO_COMMANDS: { tStep = 3; }
		case DIALOG_TUTO_JOBS:     { tStep = 4; }
		case DIALOG_TUTO_PHONE:    { tStep = 5; }
		case DIALOG_TUTO_FACTIONS: { tStep = 6; }
		case DIALOG_TUTO_DONE:     { return 1; }
		default:                   { return 0; }
	}
	if(response)
	{
		Tuto_ShowStep(playerid, tStep + 1);
	}
	else if(tStep > 0)
	{
		Tuto_ShowStep(playerid, tStep - 1);
	}
	return 1;
}

stock Tuto_Cmd(playerid)
{
	Tuto_ShowStep(playerid, 0);
	return 1;
}
