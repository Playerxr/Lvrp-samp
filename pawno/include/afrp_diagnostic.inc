/*
	afrp_diagnostic.inc - Auto-controle du serveur au demarrage + /verif

	POURQUOI CE MODULE :
	Les bugs qui ont coute le plus cher sur ce serveur avaient tous le meme
	profil : du code present, qui ne marchait pas, ET QUI NE DISAIT RIEN.
	  - des noms d'animations inexistants dans /emotes : rien ne se passait
	  - des camionnettes cargo sans essence : indemarrables, aucun message
	  - des meubles poses dans un monde virtuel invisible
	  - un .amx pas recompile pendant deux semaines
	Aucun de ces cas ne produisait la moindre erreur : il fallait qu'un joueur
	le remarque, le signale, et qu'on fouille le code.

	Ce module retourne le probleme : au demarrage, le serveur verifie ses
	propres donnees et CRIE dans la console quand une fonctionnalite est
	condamnee a ne pas marcher. Exemple concret deja vecu : avec moins de
	2 depots cargo, aucune mission /cargo ne peut demarrer - la console
	l'annonce desormais au lieu de laisser chercher.

	  Console : rapport automatique a chaque demarrage
	  /verif   : le meme rapport en jeu (admin), a tout moment

	A inclure EN DERNIER : il lit les compteurs de tous les autres modules.
*/

#if defined AFRP_DIAGNOSTIC_INCLUDED
	#endinput
#endif
#define AFRP_DIAGNOSTIC_INCLUDED

#define DIAG_DLG          9944

// Repere de version. A remonter a chaque build : si le serveur affiche une
// valeur plus ancienne que le dernier commit, c'est que l'.amx charge n'est
// pas le bon (le compilateur Pawn de SA-MP ne fournit pas __DATE__, on ne
// peut donc pas le deduire automatiquement).
#define DIAG_BUILD        "2026-07-26 justice"

// Compte les elements actifs d'un module dont le compteur global n'existe pas.
stock Diag_CountContracts()
{
	new c = 0;
	for(new i = 0; i < EM_MAX_CONTRACT; i++) if(Contracts[i][coUsed] == 1) c++;
	return c;
}

stock Diag_CountEntLocals()
{
	new c = 0;
	for(new i = 0; i < ENT_MAX; i++)
		if(entreprises[i][entUsed] == 1 && entreprises[i][entIntId] >= 1) c++;
	return c;
}

// Construit le rapport. anomalies = nombre de problemes bloquants trouves.
// dest recoit le texte (formate pour un dialog, lisible aussi en console).
stock Diag_Build(dest[], destSize, &anomalies)
{
	new tmp[220];
	anomalies = 0;
	dest[0] = 0;

	format(tmp, sizeof(tmp), "{FFFFFF}Version chargee : {FFFFB2}%s{FFFFFF}\n\n", DIAG_BUILD);
	strcat(dest, tmp, destSize);

	// ---- Anomalies bloquantes ----
	strcat(dest, "{FF6666}Problemes detectes :{FFFFFF}\n", destSize);

	if(cargo_DepotCount < 2)
	{
		format(tmp, sizeof(tmp), "{FF6666}- Depots cargo : %d. Il en faut au moins 2, sinon AUCUNE mission /cargo ne peut demarrer (/cargo creer <nom>).{FFFFFF}\n", cargo_DepotCount);
		strcat(dest, tmp, destSize);
		anomalies++;
	}
	if(parking_Count == 0)
	{
		strcat(dest, "{FF6666}- Aucun parking charge : /parking est inutilisable (/parking creer <nom>).{FFFFFF}\n", destSize);
		anomalies++;
	}
	if(totalHouses == 0)
	{
		strcat(dest, "{FF6666}- Aucune maison chargee : probleme de base de donnees ?{FFFFFF}\n", destSize);
		anomalies++;
	}
	if(totalBizz == 0)
	{
		strcat(dest, "{FF6666}- Aucun biz charge : probleme de base de donnees ?{FFFFFF}\n", destSize);
		anomalies++;
	}
	if(totalVehicles <= 1)
	{
		strcat(dest, "{FF6666}- Aucun vehicule charge : probleme de base de donnees ?{FFFFFF}\n", destSize);
		anomalies++;
	}
	if(totalInteriors == 0)
	{
		strcat(dest, "{FF6666}- Aucun interieur charge : les entrees de batiments ne marcheront pas.{FFFFFF}\n", destSize);
		anomalies++;
	}
	// Une entreprise sans local ne peut ni etre visitee, ni recevoir de
	// livraison entre entreprises : ca merite d'etre signale sans etre bloquant.
	if(ent_Count > 0 && Diag_CountEntLocals() == 0)
	{
		strcat(dest, "{FFAA00}- Des entreprises existent mais aucune n'a de local : elles sont introuvables sur la map (/entreprise local).{FFFFFF}\n", destSize);
	}
	if(anomalies == 0)
		strcat(dest, "{00FF7F}- Aucun probleme bloquant detecte.{FFFFFF}\n", destSize);

	// ---- Compteurs ----
	strcat(dest, "\n{42AAFF}Donnees chargees :{FFFFFF}\n", destSize);
	format(tmp, sizeof(tmp), "Maisons %d  -  Biz %d  -  Vehicules %d  -  Garages %d\n", totalHouses, totalBizz, totalVehicles, totalGarages);
	strcat(dest, tmp, destSize);
	format(tmp, sizeof(tmp), "Interieurs %d  -  Portes %d  -  Factions %d  -  Mobilier %d\n", totalInteriors, totalGates, totalFactions, totalFurnitures);
	strcat(dest, tmp, destSize);
	format(tmp, sizeof(tmp), "Parkings %d  -  Depots cargo %d  -  Camionnettes %d\n", parking_Count, cargo_DepotCount, cargo_TruckCount);
	strcat(dest, tmp, destSize);
	format(tmp, sizeof(tmp), "Entreprises %d (dont %d avec local)  -  Contrats %d  -  Meubles exterieurs %d\n",
		ent_Count, Diag_CountEntLocals(), Diag_CountContracts(), md_Count);
	strcat(dest, tmp, destSize);
	return 1;
}

// Rapport console au demarrage.
stock Diag_RunStartup()
{
	new rapport[1400], anomalies;
	Diag_Build(rapport, sizeof(rapport), anomalies);

	print("========================================================");
	printf("[DIAGNOSTIC] Version : %s", DIAG_BUILD);
	printf("[DIAGNOSTIC] Maisons %d | Biz %d | Vehicules %d | Interieurs %d",
		totalHouses, totalBizz, totalVehicles, totalInteriors);
	printf("[DIAGNOSTIC] Parkings %d | Depots cargo %d | Camionnettes %d",
		parking_Count, cargo_DepotCount, cargo_TruckCount);
	printf("[DIAGNOSTIC] Entreprises %d (dont %d avec local) | Contrats %d | Meubles ext. %d",
		ent_Count, Diag_CountEntLocals(), Diag_CountContracts(), md_Count);

	// Les anomalies sont repetees ligne par ligne, en clair : c'est ce qu'on
	// veut voir sauter aux yeux dans la console au demarrage.
	if(cargo_DepotCount < 2)
		printf("[DIAGNOSTIC] !! %d depot(s) cargo : il en faut 2 minimum, AUCUNE mission /cargo ne peut demarrer.", cargo_DepotCount);
	if(parking_Count == 0)
		print("[DIAGNOSTIC] !! Aucun parking charge : /parking est inutilisable.");
	if(totalHouses == 0)
		print("[DIAGNOSTIC] !! Aucune maison chargee : probleme de base de donnees ?");
	if(totalBizz == 0)
		print("[DIAGNOSTIC] !! Aucun biz charge : probleme de base de donnees ?");
	if(totalVehicles <= 1)
		print("[DIAGNOSTIC] !! Aucun vehicule charge : probleme de base de donnees ?");
	if(totalInteriors == 0)
		print("[DIAGNOSTIC] !! Aucun interieur charge : les entrees de batiments ne marcheront pas.");
	if(ent_Count > 0 && Diag_CountEntLocals() == 0)
		print("[DIAGNOSTIC] ! Des entreprises existent mais aucune n'a de local : introuvables sur la map.");

	if(anomalies == 0) print("[DIAGNOSTIC] Aucun probleme bloquant detecte.");
	else               printf("[DIAGNOSTIC] %d probleme(s) bloquant(s) - voir ci-dessus.", anomalies);
	print("========================================================");
	return anomalies;
}

// Le rapport ne peut PAS tourner directement dans OnGameModeInit : les maisons,
// biz et vehicules sont charges par des requetes MySQL asynchrones qui ne sont
// pas terminees a ce moment-la. On l'aurait vu annoncer "aucune maison chargee"
// a chaque demarrage. D'ou ce differe.
forward Diag_StartupTimer();
public Diag_StartupTimer()
{
	return Diag_RunStartup();
}

stock Diag_ScheduleStartup()
{
	SetTimer("Diag_StartupTimer", 20000, 0);
	return 1;
}

// /verif en jeu (admin)
stock Diag_ShowDialog(playerid)
{
	if(PlayerInfo[playerid][pAdmin] < 1)
		return msg_Client(playerid, COLOR_NOACCES, "{FF0069} Erreur {FFFFFF} Reserve au staff.");

	new rapport[1400], anomalies;
	Diag_Build(rapport, sizeof(rapport), anomalies);

	new titre[64];
	if(anomalies == 0) format(titre, sizeof(titre), "{00FF7F}Diagnostic serveur - tout va bien");
	else               format(titre, sizeof(titre), "{FF6666}Diagnostic serveur - %d probleme(s)", anomalies);

	return ShowPlayerDialog(playerid, DIAG_DLG, DIALOG_STYLE_MSGBOX, titre, rapport, "OK", "");
}
