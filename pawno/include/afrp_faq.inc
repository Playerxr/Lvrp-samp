/*
	afrp_faq.inc - Aide automatique pour reduire les appels aux admins

	1) FAQ CHAT : quand un joueur pose une question courante dans le chat
	   ("comment gagner de l'argent ?", "comment prendre un job ?"...), le
	   serveur lui repond automatiquement avec la bonne commande, en prive.
	   Cooldown 15s/joueur pour eviter le spam. Le message du joueur passe
	   normalement (on ajoute juste une reponse utile).

	2) GUIDE RAPIDE (/aide enrichi + /guide + /faq) : un menu clair qui repond
	   directement aux besoins frequents (argent, job, maison, voiture, tel,
	   faction, VIP, regles) pour que le joueur se debrouille seul. Le menu
	   d'aide detaille d'origine reste accessible via la derniere entree.

	Tout est en texte non accentue (rendu CP1252 du client SA-MP).
*/

#if defined AFRP_FAQ_INCLUDED
	#endinput
#endif
#define AFRP_FAQ_INCLUDED

#define FAQ_DIALOG_GUIDE   9771
#define FAQ_DIALOG_ANSWER  9772
#define FAQ_CHAT_COOLDOWN  15   // s entre 2 reponses auto par joueur

new faqLastAuto[MAX_PLAYERS];

stock FAQ_OnConnect(playerid)
{
	faqLastAuto[playerid] = 0;
	return 1;
}

// Reponses detaillees par sujet (index = listitem du guide)
stock FAQ_GetAnswer(topic, dest[], size)
{
	switch(topic)
	{
		case 0: format(dest, size, "{FFD700}Gagner de l'argent{FFFFFF}\n\nLe plus simple : prends un metier avec {00FF00}/jobs{FFFFFF} (liste + salaire + GPS).\nLes {FFD700}nouveaux joueurs gagnent le DOUBLE{FFFFFF} sur chaque tache, et enchainer les taches sans t'arreter augmente tes gains (prime de serie).\n\nAutres pistes : {00FF00}/petitboulot{FFFFFF}, revendre des objets, ou ouvrir un business.");
		case 1: format(dest, size, "{FFD700}Prendre un metier{FFFFFF}\n\n1. Tape {00FF00}/jobs{FFFFFF} et choisis un metier -> un GPS se pose vers l'employeur.\n2. Va au marqueur et appuie sur la touche {FFFF00}'N'{FFFFFF} devant l'employeur.\n3. Suis les checkpoints pour travailler et gagner ta paye.\n\nTu peux changer de metier a tout moment via un autre employeur.");
		case 2: format(dest, size, "{FFD700}Acheter une maison{FFFFFF}\n\nCherche un panneau {FFFF00}A VENDRE{FFFFFF} (icone maison sur la carte).\nEntre dedans (touche {FFFF00}'F'{FFFFFF} ou 'N') et tape {00FF00}/achetermaison{FFFFFF}.\n\nGere ta maison avec {00FF00}/maison{FFFFFF} (coffre, serrure, mobilier...).");
		case 3: format(dest, size, "{FFD700}Voiture{FFFFFF}\n\nAchat : va dans une {FFFF00}concession{FFFFFF} (icone voiture sur la carte).\nVerrouiller/deverrouiller : touche {FFFF00}'Y'{FFFFFF} ou {00FF00}/verrouiller{FFFFFF}.\nEssence : {00FF00}/v remplir <litres>{FFFFFF} dans une station.\nRetrouver ta voiture : {00FF00}/v{FFFFFF} (menu vehicule).");
		case 4: format(dest, size, "{FFD700}Telephone{FFFFFF}\n\nAchete un telephone au magasin {FFFF00}24/7{FFFFFF} (icone sur la carte).\nOuvre-le avec {00FF00}/mobile{FFFFFF} : SMS, appels, banque, annuaire.\nNumeros utiles : {00FF00}/annuaire{FFFFFF}. Urgences directes : {00FF00}/911{FFFFFF} (police), {00FF00}/sos{FFFFFF} (medecin), {00FF00}/feu{FFFFFF} (pompiers).");
		case 5: format(dest, size, "{FFD700}Faction / Gang{FFFFFF}\n\nTape {00FF00}/factions{FFFFFF} pour voir la liste des factions legales et gangs.\nPour rejoindre, il faut etre {FFFF00}invite{FFFFFF} par un membre (rang eleve) de la faction/gang.\nUne fois membre d'un gang, ton nom de gang s'affiche au-dessus de ta tete.");
		case 6: format(dest, size, "{FFD700}Devenir VIP{FFFFFF}\n\nLe VIP soutient le serveur et debloque : {FFFF00}tag colore{FFFFFF}, {FFFF00}salaire garanti{FFFFFF} a chaque paye, taxes reduites, progression job acceleree, teleport {00FF00}/vip maison{FFFFFF} et {00FF00}/vip garage{FFFFFF}, emotes exclusives, chat VIP {00FF00}/vc{FFFFFF}.\n\nRends-toi sur le site du serveur pour en profiter.");
		case 7: format(dest, size, "{FFD700}RP de base / Regles{FFFFFF}\n\n{00FF00}/me <action>{FFFFFF} : decrire une action de ton perso.\n{00FF00}/do <description>{FFFFFF} : decrire l'environnement.\nParle IC avec la touche {FFFF00}T{FFFFFF}, OOC avec {00FF00}/b{FFFFFF}.\nUn probleme avec un joueur ? {00FF00}/report{FFFFFF}.\nReglement complet : {00FF00}/regles{FFFFFF}.");
		default: format(dest, size, "Sujet inconnu.");
	}
	return 1;
}

stock FAQ_ShowGuide(playerid)
{
	ShowPlayerDialog(playerid, FAQ_DIALOG_GUIDE, DIALOG_STYLE_LIST,
		"{FFAA00}>> Guide rapide AFRP <<{FFFFFF}",
		"{FFFFFF}Comment gagner de l'argent ?\n\
{FFFFFF}Comment prendre un metier ?\n\
{FFFFFF}Comment acheter une maison ?\n\
{FFFFFF}Comment gerer une voiture ?\n\
{FFFFFF}Comment utiliser le telephone ?\n\
{FFFFFF}Comment rejoindre une faction/gang ?\n\
{FFD700}Comment devenir VIP ?\n\
{FFFFFF}RP de base / regles\n\
{8C8C8C}>> Aide detaillee (commandes)",
		"Voir", "Fermer");
	return 1;
}

stock FAQ_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == FAQ_DIALOG_GUIDE)
	{
		if(!response) return 1;
		if(listitem == 8)
		{
			// Ouvre l'ancien menu d'aide detaille (dialog 1)
			ShowPlayerDialog(playerid, 1, DIALOG_STYLE_LIST, " Aide ",
				"[General]\n[Tchat]\n[Job]\n[Faction]\n[Maison(s)]\n[Biz]\n[Voiture(s)]\n[Garage(s)]", "Ouvrir", "Fermer");
			return 1;
		}
		new ans[600];
		FAQ_GetAnswer(listitem, ans, sizeof(ans));
		ShowPlayerDialog(playerid, FAQ_DIALOG_ANSWER, DIALOG_STYLE_MSGBOX,
			"{FFAA00}>> Guide rapide <<{FFFFFF}", ans, "Retour", "Fermer");
		return 1;
	}
	if(dialogid == FAQ_DIALOG_ANSWER)
	{
		// "Retour" -> re-affiche le guide
		if(response) FAQ_ShowGuide(playerid);
		return 1;
	}
	return 0;
}

// Auto-reponse dans le chat. Appelee depuis OnPlayerText (ne bloque pas le chat).
stock FAQ_CheckMessage(playerid, const text[])
{
	// On ne repond qu'aux vraies questions pour eviter le bruit
	if(strfind(text, "?", true) == -1 && strfind(text, "comment", true) == -1) return 0;

	new now = gettime();
	if(now - faqLastAuto[playerid] < FAQ_CHAT_COOLDOWN) return 0;

	new hint[160];
	hint[0] = 0;

	if(strfind(text, "argent", true) != -1 || strfind(text, "gagner", true) != -1 || strfind(text, "riche", true) != -1 || strfind(text, "pauvre", true) != -1 || strfind(text, "money", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Pour gagner de l'argent : tape {FFFF00}/jobs{FFFFFF} (metiers + salaire + GPS). Nouveaux = gains {FFD700}x2{FFFFFF} !");
	else if(strfind(text, "job", true) != -1 || strfind(text, "metier", true) != -1 || strfind(text, "travail", true) != -1 || strfind(text, "bosser", true) != -1 || strfind(text, "boulot", true) != -1 || strfind(text, "emploi", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Pour bosser : {FFFF00}/jobs{FFFFFF} -> choisis un metier -> suis le GPS -> touche {FFFF00}'N'{FFFFFF} chez l'employeur.");
	else if(strfind(text, "maison", true) != -1 || strfind(text, "logement", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Maison : entre dans une maison {FFFF00}A VENDRE{FFFFFF} et tape {FFFF00}/achetermaison{FFFFFF}. Gestion : {FFFF00}/maison{FFFFFF}.");
	else if(strfind(text, "voiture", true) != -1 || strfind(text, "vehicule", true) != -1 || strfind(text, "caisse", true) != -1 || strfind(text, "bagnole", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Voiture : achat en {FFFF00}concession{FFFFFF}, verrouiller avec {FFFF00}'Y'{FFFFFF}, menu {FFFF00}/v{FFFFFF}, plein {FFFF00}/v remplir{FFFFFF}.");
	else if(strfind(text, "telephone", true) != -1 || strfind(text, "portable", true) != -1 || strfind(text, "numero", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Telephone : achete-le au {FFFF00}24/7{FFFFFF}, ouvre avec {FFFF00}/mobile{FFFFFF}. Urgences : {FFFF00}/911 /sos /feu{FFFFFF}.");
	else if(strfind(text, "faction", true) != -1 || strfind(text, "gang", true) != -1 || strfind(text, "police", true) != -1 || strfind(text, "rejoindre", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Factions/gangs : {FFFF00}/factions{FFFFFF}. Il faut etre invite par un membre grade pour rejoindre.");
	else if(strfind(text, "vip", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Le VIP donne salaire garanti, taxes reduites, teleports {FFFF00}/vip maison/garage{FFFFFF} et plus. Infos sur le site du serveur.");
	else if(strfind(text, "arme", true) != -1 || strfind(text, "flingue", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Les armes a feu s'achetent a l'{FFFF00}Ammunation{FFFFFF} (permis requis) et sont tres cheres. Melee au {FFFF00}24/7{FFFFFF}.");
	else if(strfind(text, "admin", true) != -1 || strfind(text, "aide", true) != -1 || strfind(text, "bloque", true) != -1 || strfind(text, "coince", true) != -1)
		format(hint, sizeof(hint), "{00FF88}[Aide auto]{FFFFFF} Ouvre le guide complet avec {FFFF00}/aide{FFFFFF}. Souci avec un joueur : {FFFF00}/report{FFFFFF}.");

	if(strlen(hint) == 0) return 0; // aucune correspondance

	SendClientMessage(playerid, -1, hint);
	SendClientMessage(playerid, 0x8C8C8CFF, "{8C8C8C}(Astuce automatique - tape {FFFF00}/aide{8C8C8C} pour le guide complet)");
	faqLastAuto[playerid] = now;
	return 1;
}
