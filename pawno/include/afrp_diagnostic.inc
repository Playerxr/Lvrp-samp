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
#define DIAG_BUILD        "2026-07-27 essence+event+armee"

// ============================================================================
// DIALOGUES : collisions d'ID + dialogues sans handler
// ============================================================================
//
// Deux controles complementaires, parce qu'aucun ne suffit seul :
//
//  1. REGISTRE (statique) - la liste ci-dessous declare quel module possede
//     quel ID. Le controle signale tout ID declare deux fois. C'est ce qui a
//     permis de trouver les 7 collisions ou cliquer "OK" sur un dialogue de
//     module declenchait le handler du telephone ou d'un peage. Limite
//     assumee : un ID ajoute sans etre inscrit ici echappe au controle.
//
//  2. DIALOGUES ORPHELINS (automatique) - OnDialogResponse signale tout
//     dialogue dont la reponse n'est traitee par aucun handler. Celui-la
//     n'a rien a maintenir et attrape aussi les ajouts futurs : un dialogue
//     qui s'ouvre et ne fait rien au clic est detecte des le premier usage.

#define DIAG_MAX_ORPHAN     12    // derniers dialogues orphelins memorises

enum e_DiagDlg
{
	ddId,
	ddOwner[28]
};
// Dialogues des modules (.inc). Ceux du gamemode historique ne sont pas
// listes un par un : ils sont trop nombreux, et ce sont les modules ajoutes
// apres coup qui creent les collisions.
static const DiagDialogs[][e_DiagDlg] = {
	{9600, "portarme"},          {9601, "cargomission"},
	{9602, "venuepriv"},         {9603, "entreprise"},
	{9604, "entreprise"},        {9605, "entreprise"},
	{9606, "venuepriv"},
	{9610, "event"},             {9611, "event"},
	{9612, "event"},             {9613, "event"},
	{9614, "event"},             {9615, "event"},
	{9616, "event"},             {9617, "event"},
	{9618, "event"},            {9619, "event"},
	{9620, "topjour"},        {9621, "armeebase"},
	{9622, "armeebase"},
	{9623, "assurance"},      {9624, "assurance"},
	{9625, "assurance"},     {9626, "gpspartage"},
	{9627, "gpspartage"},   {9628, "saison"},
	{9629, "saison"},       {9630, "braquage"},
	{9631, "armeniveau"},   {9632, "nouveautes"},
	{9633, "nouveautes"},   {9634, "mapedit"},
	{9635, "mapedit"},      {9636, "mapedit"},
	{9637, "mappack"},      {9638, "mappack"},
	{9639, "sacvisuel"},    {9640, "sacvisuel"},
	{9641, "telapps"},      {9642, "telapps"},
	{9643, "telapps"},      {9644, "telapps"},
	{9646, "telapps"},      {9647, "telapps"},
	{9648, "telapps"},      {9649, "telapps"},
	{9650, "portes"},
	{9651, "residence"},    {10651, "residence"},
	{9652, "residence"},    {9653, "residence"},
	{9654, "raccourcis"},   {9655, "banque"},
	{9656, "jobs_new"},
	{9800, "factionmenu"},       {9801, "factionmenu"},
	{9802, "factionmenu"},       {9803, "factionmenu"},
	{9940, "justice"},           {9941, "justice"},
	{9942, "justice"},           {9943, "justice"},
	{9944, "diagnostic"},        {9945, "objectifs"},
	{9955, "entmarket"},         {9956, "entmarket"},
	{9957, "entmarket"},         {9958, "entmarket"},
	{9964, "entmarket"},         {9965, "entcommerce"},
	{9966, "entcommerce"},       {9967, "entcommerce"},
	{9968, "entcommerce"},       {9969, "entmarket"},
	{9970, "meublesdehors"},     {9971, "meublesdehors"},
	{9972, "meublesdehors"},     {9973, "meublesdehors"},
	{9975, "entreprise"},        {9976, "entreprise"},
	{9977, "entreprise"},        {9978, "entreprise"},
	{9979, "entreprise"},        {9984, "entreprise"},
	{9985, "cargomission"},      {9986, "entreprise"},
	{9987, "entreprise"},        {9988, "entreprise"},
	{9989, "entreprise"},
	{9721, "genmiracle"},        {9750, "jobrank"},
	{9760, "bounty"},            {9770, "jobs"},
	{9780, "medical"},           {9946, "fidelite"},
	{9960, "venuepriv"},         {9961, "venuepriv"},
	{9962, "venuepriv"},         {9963, "venuepriv"}
	// 9999 et 12346 volontairement absents : 9999 est l'ID "message
	// d'information" generique, partage sans handler par plusieurs modules et
	// par le gamemode ; 12346 est le dialogue de connexion du gamemode, que
	// afrp_loginsound.inc ne fait que citer dans un commentaire. Les inscrire
	// produirait de fausses alertes.
};

// Derniers dialogues orphelins vus (id + nombre de fois).
new diagOrphanId[DIAG_MAX_ORPHAN];
new diagOrphanHits[DIAG_MAX_ORPHAN];
new diagOrphanCount = 0;

// Appelee tout en bas de OnDialogResponse : si on arrive la, aucun handler
// n'a traite ce dialogue. Le joueur a donc clique dans le vide.
stock Diag_UnhandledDialog(dialogid)
{
	if(dialogid < 0) return 0;
	for(new i = 0; i < diagOrphanCount; i++)
	{
		if(diagOrphanId[i] == dialogid) { diagOrphanHits[i]++; return 1; }
	}
	if(diagOrphanCount >= DIAG_MAX_ORPHAN) return 0;
	diagOrphanId[diagOrphanCount]   = dialogid;
	diagOrphanHits[diagOrphanCount] = 1;
	diagOrphanCount++;
	printf("[DIAGNOSTIC] ! Dialogue %d : reponse recue mais aucun handler ne la traite (clic sans effet).", dialogid);
	return 1;
}

// Cherche les ID declares plusieurs fois dans le registre.
// Retourne le nombre de doublons et remplit dest.
stock Diag_CheckDialogs(dest[], destSize)
{
	dest[0] = 0;
	new doublons = 0, tmp[160];
	new nb = sizeof(DiagDialogs);
	for(new i = 0; i < nb; i++)
	{
		for(new j = i + 1; j < nb; j++)
		{
			if(DiagDialogs[i][ddId] != DiagDialogs[j][ddId]) continue;
			format(tmp, sizeof(tmp), "{FF6666}- Dialogue %d declare par %s ET %s.{FFFFFF}\n",
				DiagDialogs[i][ddId], DiagDialogs[i][ddOwner], DiagDialogs[j][ddOwner]);
			strcat(dest, tmp, destSize);
			doublons++;
		}
	}
	return doublons;
}

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

	// Collisions d'ID de dialogue entre modules.
	new dlgTxt[400];
	new doublons = Diag_CheckDialogs(dlgTxt, sizeof(dlgTxt));
	if(doublons > 0)
	{
		strcat(dest, dlgTxt, destSize);
		anomalies += doublons;
	}

	// Dialogues dont la reponse n'est traitee par personne.
	if(diagOrphanCount > 0)
	{
		for(new i = 0; i < diagOrphanCount; i++)
		{
			format(tmp, sizeof(tmp), "{FFAA00}- Dialogue %d : clic sans effet, aucun handler (%d fois).{FFFFFF}\n",
				diagOrphanId[i], diagOrphanHits[i]);
			strcat(dest, tmp, destSize);
		}
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

	// Collisions d'ID de dialogue : deux modules qui partagent un ID, c'est un
	// clic sur l'un qui declenche l'action de l'autre. Silencieux en jeu.
	new dlgTxt[400];
	new doublons = Diag_CheckDialogs(dlgTxt, sizeof(dlgTxt));
	if(doublons > 0)
		printf("[DIAGNOSTIC] !! %d collision(s) d'ID de dialogue entre modules - voir /verif.", doublons);
	else
		print("[DIAGNOSTIC] Aucune collision d'ID de dialogue entre modules.");

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
