/*
	afrp_whitelist.inc - Candidature whitelist validee depuis Discord

	POURQUOI CE MODULE
	------------------
	Avant : un nouveau joueur restait bloque sur un dialog "en attente" jusqu'a
	ce qu'un admin CONNECTE tape /approuver. Concretement, si aucun staff
	n'etait en jeu au moment de l'inscription, le joueur attendait dans le vide
	et partait. Les investisseurs ont demande un vrai dossier de candidature
	traite depuis Discord (la ou le staff est reellement present).

	POURQUOI PASSER PAR MYSQL ET PAS PAR UN PLUGIN DISCORD
	------------------------------------------------------
	SA-MP ne sait faire que du HTTP/1.0 (natif HTTP()), Discord n'accepte que
	du HTTPS : impossible de parler a Discord directement. Le depot contient
	deja un relais SORTANT (afrp_discord.inc -> discord_relay.php), mais il est
	a sens unique : le serveur peut ecrire vers Discord, jamais l'inverse.
	Pour l'ENTRANT (Discord -> serveur) la solution evidente serait un plugin
	natif type samp-discord-connector. On la refuse volontairement : un plugin
	natif = un .so a compiler pour la bonne architecture, uploader en binaire,
	et le moindre ratage empeche le serveur de demarrer ("invalid ELF header").
	Sur un serveur de production tenu par une personne non technique, le risque
	est disproportionne.

	=> La BASE MYSQL sert de boite aux lettres entre les deux mondes :
	     gamemode  --INSERT-->  lvrp_whitelist  <--UPDATE--  bot Discord
	     gamemode  <--SELECT--  lvrp_whitelist

	Consequence VOULUE : le gamemode ne sait pas, et n'a pas besoin de savoir,
	OU tourne le bot (VPS, PC de la proprietaire, hebergement mutualise, autre
	langage...). Tant que quelque chose sait ecrire une decision dans la table,
	le serveur la respecte. L'hebergement du bot n'etant pas encore choisi,
	cette independance est le point central du design. Le contrat complet est
	documente dans discord_bot/README.md.

	CONTRAT DE LA TABLE lvrp_whitelist (cote gamemode)
	--------------------------------------------------
	  Statut = -1 : brouillon, le joueur est en train de remplir le formulaire.
	                Personne d'autre ne doit y toucher (le bot l'ignore).
	  Statut =  0 : dossier complet, EN ATTENTE de decision. Le bot le poste.
	  Statut =  1 : accepte.   Statut = 2 : refuse (Motif rempli).
	  Poste  =  1 : le bot a deja publie le dossier sur Discord (anti-doublon).
	  Traite =  1 : le gamemode a deja applique la decision (anti-doublon).

	Le formulaire est stocke champ par champ EN BASE au fur et a mesure
	(colonne Etape), et pas en memoire : 5 reponses x 1000 slots joueurs
	auraient coute des centaines de Ko de heap pour rien, et un joueur qui
	perd la connexion au milieu du formulaire aurait tout reperdu. La ligne
	brouillon fait aussi office de verrou "une seule candidature ouverte par
	compte" : tout est adresse par SQLid + Statut=-1.

	SECURITE
	--------
	Les 5 reponses sont de la saisie joueur BRUTE qui part en SQL : chaque
	valeur est bornee en longueur, nettoyee des caracteres de controle,
	validee (ages numeriques et plausibles, longueurs mini) puis injectee
	UNIQUEMENT via mysql_format + '%e' (l'echappement du plugin MySQL, deja
	l'idiome maison - cf. lvrp_afrp_phone). Aucune concatenation manuelle.

	Hooks dans LVRP.pwn :
	  - Whitelist_Init()          dans OnGameModeInit (a cote de Discord_Init)
	  - Whitelist_Reset(playerid) dans OnPlayerConnect et OnPlayerDisconnect
	  - Whitelist_Demarrer(playerid) partout ou l'ancien dialog 9903 etait
	    affiche (login, fin d'inscription, OnPlayerSpawn)
	  - Whitelist_OnDialog(...)   en tete de OnDialogResponse
	  - Whitelist_LaisserEntrer / Whitelist_MarquerDecision / Whitelist_AppliquerRefus
	    utilises par /approuver et /refuser (override manuel du staff)
*/

#if defined AFRP_WHITELIST_INCLUDED
	#endinput
#endif
#define AFRP_WHITELIST_INCLUDED

// --- Dialogs -----------------------------------------------------------
// 9810 a 9816 : plage libre verifiee sur tout le depot (gamemode, includes,
// filterscripts) - le premier ID utilise apres 9803 est 9901.
// 9903 est CONSERVE : c'est l'ecran "en attente" que les joueurs connaissent
// deja, on ne fait qu'en reecrire le contenu.
#define WL_DLG_INTRO        9810
#define WL_DLG_NOMRP        9811
#define WL_DLG_AGERP        9812
#define WL_DLG_BACKGROUND   9813
#define WL_DLG_AGEREEL      9814
#define WL_DLG_SOURCE       9815
#define WL_DLG_ENVOYE       9816
#define WL_DLG_ATTENTE      9903

// --- Bornes de validation ---------------------------------------------
// Les maxi correspondent EXACTEMENT a la taille des colonnes SQL : une
// reponse trop longue est coupee ici, jamais par MySQL (qui, selon le mode
// strict du serveur, refuserait la requete au lieu de tronquer).
#define WL_LEN_NOMRP        32
#define WL_LEN_BACKGROUND   255
#define WL_LEN_SOURCE       80
#define WL_LEN_MOTIF        128

#define WL_MIN_NOMRP        3
#define WL_MIN_BACKGROUND   40
#define WL_MIN_SOURCE       3
#define WL_AGERP_MIN        16
#define WL_AGERP_MAX        90
#define WL_AGEREEL_MIN      10
#define WL_AGEREEL_MAX      99

// Cadence de relecture des decisions.
// 12s : une decision Discord est un geste humain, le joueur qui attend
// percoit 12s comme "immediat" alors qu'une minute lui ferait croire que
// c'est casse. Cote base c'est un SELECT indexe sur une table qui contient
// au plus quelques lignes en attente, soit 5 requetes minuscules par minute -
// negligeable face aux timers a la seconde deja presents dans le gamemode.
// Descendre a 1-2s n'apporterait rien de perceptible et multiplierait par 10
// le bruit sur la base.
#define WL_POLL_MS          12000

// Etat du joueur dans le parcours (memoire volatile, 2 cellules par slot).
#define WL_ETAT_INCONNU     0   // rien de demarre
#define WL_ETAT_VERIF       1   // requete de verification du dossier en vol
#define WL_ETAT_FORMULAIRE  2   // formulaire en cours de saisie
#define WL_ETAT_ENVOYE      3   // dossier complet, attente de decision

new wl_Etat[MAX_PLAYERS];
new wl_Etape[MAX_PLAYERS];   // question courante : 1 a 5
new bool:wl_PollEnCours = false;

forward WL_OnCheckDossier(playerid);
forward WL_Poll();
forward WL_OnPollResult();

// ======================================================================
//  INITIALISATION
// ======================================================================

// La table est creee ici en plus d'etre versionnee dans lvrp.sql :
// la base de production tourne depuis longtemps et n'est jamais rejouee
// depuis le .sql, donc sans ce CREATE la fonctionnalite serait morte au
// premier demarrage. IF NOT EXISTS => aucun effet si elle existe deja.
stock Whitelist_Init()
{
	mysql_tquery(MYSQL,
		"CREATE TABLE IF NOT EXISTS `lvrp_whitelist` (\
 `id` int(11) NOT NULL AUTO_INCREMENT,\
 `SQLid` int(11) NOT NULL DEFAULT '0',\
 `Name` varchar(32) NOT NULL DEFAULT '',\
 `NomRP` varchar(32) NOT NULL DEFAULT '',\
 `AgeRP` smallint(6) NOT NULL DEFAULT '0',\
 `Background` varchar(255) NOT NULL DEFAULT '',\
 `AgeReel` smallint(6) NOT NULL DEFAULT '0',\
 `Source` varchar(80) NOT NULL DEFAULT '',\
 `Etape` smallint(6) NOT NULL DEFAULT '1',\
 `Statut` smallint(6) NOT NULL DEFAULT '-1',\
 `Motif` varchar(128) NOT NULL DEFAULT '',\
 `DecidePar` varchar(64) NOT NULL DEFAULT '',\
 `Poste` smallint(6) NOT NULL DEFAULT '0',\
 `Traite` smallint(6) NOT NULL DEFAULT '0',\
 `DateSoumission` int(11) NOT NULL DEFAULT '0',\
 `DateDecision` int(11) NOT NULL DEFAULT '0',\
 PRIMARY KEY (`id`),\
 KEY `SQLid` (`SQLid`),\
 KEY `Statut` (`Statut`)\
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1", "", "");

	SetTimer("WL_Poll", WL_POLL_MS, true);
	printf("[Whitelist] Candidatures Discord actives - relecture des decisions toutes les %d s.", WL_POLL_MS / 1000);
	return 1;
}

stock Whitelist_Reset(playerid)
{
	if(playerid < 0 || playerid >= MAX_PLAYERS) return 0;
	wl_Etat[playerid] = WL_ETAT_INCONNU;
	wl_Etape[playerid] = 0;
	return 1;
}

// ======================================================================
//  OUTILS DE VALIDATION / NETTOYAGE DE LA SAISIE JOUEUR
// ======================================================================

// Copie src -> dest en bornant la longueur et en neutralisant les
// caracteres de controle (< 32). Un retour a la ligne ou un caractere nul
// mal place casserait autant l'affichage du dialog que l'embed Discord.
// Les accents (latin1, >= 128) sont conserves : c'est un serveur francais.
stock wl_Nettoyer(dest[], const src[], maxLen)
{
	new di = 0;
	for(new i = 0; src[i] != 0 && di < maxLen; i++)
	{
		new c = src[i];
		if(c < 32) c = ' ';
		dest[di++] = c;
	}
	dest[di] = 0;
	// Retire les espaces de bord (un joueur qui n'envoie que des espaces
	// doit echouer le test de longueur mini, pas passer au travers).
	while(di > 0 && dest[di-1] == ' ') { dest[di-1] = 0; di--; }
	new start = 0;
	while(dest[start] == ' ') start++;
	if(start > 0)
	{
		new k = 0;
		while(dest[start + k]) { dest[k] = dest[start + k]; k++; }
		dest[k] = 0;
	}
	return strlen(dest);
}

stock bool:wl_EstNumerique(const s[])
{
	if(s[0] == 0) return false;
	for(new i = 0; s[i]; i++)
		if(s[i] < '0' || s[i] > '9') return false;
	return true;
}

// Nom RP : convention maison Prenom_Nom (c'est le format de tous les noms
// de comptes du serveur). On impose donc l'underscore et on refuse les
// chiffres, qui sont le marqueur numero un du nom non-RP ("Xx_Sniper_2007").
stock bool:wl_NomRPValide(const s[])
{
	new len = strlen(s);
	if(len < WL_MIN_NOMRP) return false;
	new bool:underscore = false, bool:lettre = false;
	for(new i = 0; i < len; i++)
	{
		new c = s[i];
		if(c == '_') { underscore = true; continue; }
		if(c == '-' || c == '\'') continue;
		if(c >= 'A' && c <= 'Z') { lettre = true; continue; }
		if(c >= 'a' && c <= 'z') { lettre = true; continue; }
		if(c < 0 || c >= 128) { lettre = true; continue; } // lettres accentuees latin1
		return false;
	}
	// "___" passerait sinon les deux tests precedents.
	return (underscore && lettre);
}

// ======================================================================
//  ECRANS
// ======================================================================

stock Whitelist_ShowAttente(playerid)
{
	ShowPlayerDialog(playerid, WL_DLG_ATTENTE, DIALOG_STYLE_MSGBOX,
		"{FFAA00}>>> La Vie RolePlay - Candidature en attente <<<",
		"{FFFFFF}Ta candidature a bien ete transmise au staff.\n\n\
{FFAA00}Elle est en cours d'examen sur notre Discord.{FFFFFF}\n\
Des qu'un membre du staff aura tranche, tu seras\n\
mis en jeu automatiquement, sans avoir a te reconnecter.\n\n\
{AAAAAA}Tu peux rester connecte. Clique sur \"Actualiser\" si tu veux\n\
verifier ou tu en es.",
		"Actualiser", "");
	return 1;
}

stock Whitelist_ShowVerif(playerid)
{
	ShowPlayerDialog(playerid, WL_DLG_ATTENTE, DIALOG_STYLE_MSGBOX,
		"{FFAA00}>>> La Vie RolePlay <<<",
		"{FFFFFF}Verification de ton dossier en cours...\n\n{AAAAAA}Un instant.",
		"Actualiser", "");
	return 1;
}

stock Whitelist_ShowIntro(playerid)
{
	ShowPlayerDialog(playerid, WL_DLG_INTRO, DIALOG_STYLE_MSGBOX,
		"{FFAA00}>>> La Vie RolePlay - Candidature <<<",
		"{FFFFFF}Bienvenue ! Le serveur est en {FFAA00}whitelist{FFFFFF} : pour\n\
jouer, tu dois d'abord remplir un court dossier de\n\
candidature ({FFAA00}5 questions{FFFFFF}).\n\n\
Ton dossier part directement sur le Discord du serveur,\n\
ou un membre du staff l'accepte ou le refuse.\n\n\
{AAAAAA}Prends le temps de repondre serieusement : c'est ce que\n\
le staff lit pour decider. Bonne chance !",
		"Commencer", "");
	return 1;
}

// Affiche la question correspondant a wl_Etape. `rappel` permet de
// reafficher la meme question avec un message d'erreur en tete quand la
// reponse precedente n'etait pas valide.
stock Whitelist_ShowQuestion(playerid, const rappel[] = "")
{
	new corps[900];
	switch(wl_Etape[playerid])
	{
		case 1:
		{
			format(corps, sizeof(corps),
				"%s{FFFFFF}Question {FFAA00}1{FFFFFF}/5 - {FFAA00}Nom RP de ton personnage{FFFFFF}\n\n\
Le prenom et le nom de ton personnage, au format\n\
{FFAA00}Prenom_Nom{FFFFFF} (exemple : Antoine_Bertrand).\n\n\
{AAAAAA}Lettres et underscore uniquement, pas de chiffres.", rappel);
			ShowPlayerDialog(playerid, WL_DLG_NOMRP, DIALOG_STYLE_INPUT,
				"{FFAA00}Candidature 1/5 - Nom RP", corps, "Suivant", "");
		}
		case 2:
		{
			format(corps, sizeof(corps),
				"%s{FFFFFF}Question {FFAA00}2{FFFFFF}/5 - {FFAA00}Age de ton personnage{FFFFFF}\n\n\
Quel age a ton personnage dans le RP ?\n\n\
{AAAAAA}Un nombre uniquement, entre %d et %d.", rappel, WL_AGERP_MIN, WL_AGERP_MAX);
			ShowPlayerDialog(playerid, WL_DLG_AGERP, DIALOG_STYLE_INPUT,
				"{FFAA00}Candidature 2/5 - Age RP", corps, "Suivant", "");
		}
		case 3:
		{
			format(corps, sizeof(corps),
				"%s{FFFFFF}Question {FFAA00}3{FFFFFF}/5 - {FFAA00}Background de ton personnage{FFFFFF}\n\n\
Raconte en quelques phrases qui est ton personnage :\n\
d'ou il vient, ce qu'il fait, pourquoi il arrive ici.\n\n\
{AAAAAA}Minimum %d caracteres. C'est la reponse que le staff\n\
regarde le plus.", rappel, WL_MIN_BACKGROUND);
			ShowPlayerDialog(playerid, WL_DLG_BACKGROUND, DIALOG_STYLE_INPUT,
				"{FFAA00}Candidature 3/5 - Background", corps, "Suivant", "");
		}
		case 4:
		{
			format(corps, sizeof(corps),
				"%s{FFFFFF}Question {FFAA00}4{FFFFFF}/5 - {FFAA00}Ton age reel{FFFFFF}\n\n\
Quel age as-tu, toi, dans la vraie vie ?\n\n\
{AAAAAA}Un nombre uniquement. Reponds honnetement : le staff\n\
verifie, et mentir est un motif de refus.", rappel);
			ShowPlayerDialog(playerid, WL_DLG_AGEREEL, DIALOG_STYLE_INPUT,
				"{FFAA00}Candidature 4/5 - Age reel", corps, "Suivant", "");
		}
		case 5:
		{
			format(corps, sizeof(corps),
				"%s{FFFFFF}Question {FFAA00}5{FFFFFF}/5 - {FFAA00}Comment as-tu connu le serveur ?{FFFFFF}\n\n\
Un ami, une video, un forum, une pub, le site...\n\n\
{AAAAAA}Une phrase suffit.", rappel);
			ShowPlayerDialog(playerid, WL_DLG_SOURCE, DIALOG_STYLE_INPUT,
				"{FFAA00}Candidature 5/5 - Provenance", corps, "Envoyer", "");
		}
		default: Whitelist_ShowAttente(playerid);
	}
	return 1;
}

// ======================================================================
//  ENTREE DANS LE PARCOURS
// ======================================================================

// Point d'entree unique : remplace tous les anciens affichages directs du
// dialog 9903. Decide tout seul s'il faut proposer le formulaire, reprendre
// une saisie interrompue, ou juste montrer l'ecran d'attente.
stock Whitelist_Demarrer(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;

	// Compte pas encore identifie en base : on ne peut rien ecrire, on se
	// contente de l'ecran d'attente (cas theorique, mais mieux vaut un
	// message qu'un ecran noir).
	if(PlayerInfo[playerid][pSQLID] < 1)
		return Whitelist_ShowAttente(playerid);

	switch(wl_Etat[playerid])
	{
		case WL_ETAT_VERIF:  return Whitelist_ShowVerif(playerid);
		// Formulaire en cours : on ne REAFFICHE surtout pas la question. Cette
		// fonction est aussi appelee depuis OnPlayerSpawn, et rouvrir le dialog
		// d'input effacerait ce que le joueur est en train d'ecrire. Le
		// formulaire se reaffiche tout seul si le joueur ferme la fenetre
		// (branche "!response" de Whitelist_OnDialog).
		case WL_ETAT_FORMULAIRE: return 1;
		case WL_ETAT_ENVOYE:     return Whitelist_ShowAttente(playerid);
	}

	wl_Etat[playerid] = WL_ETAT_VERIF;
	Whitelist_ShowVerif(playerid);

	new q[192];
	format(q, sizeof(q),
		"SELECT Etape, Statut, Motif FROM lvrp_whitelist WHERE SQLid=%d ORDER BY id DESC LIMIT 1",
		PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "WL_OnCheckDossier", "i", playerid);
	return 1;
}

public WL_OnCheckDossier(playerid)
{
	if(!IsPlayerConnected(playerid)) return 1;

	new rows = 0;
	cache_get_row_count(rows);

	if(rows == 0)
	{
		// Aucun dossier : on ouvre le brouillon TOUT DE SUITE. C'est cette
		// ligne (Statut=-1) qui garantit "une seule candidature ouverte par
		// compte" : toutes les ecritures suivantes ciblent SQLid + Statut=-1,
		// donc une reconnexion au milieu du formulaire reprend la meme ligne
		// au lieu d'en creer une deuxieme.
		new nom[MAX_PLAYER_NAME+1];
		GetPlayerName(playerid, nom, sizeof(nom));

		new q[256];
		mysql_format(MYSQL, q, sizeof(q),
			"INSERT INTO lvrp_whitelist (SQLid, Name, Etape, Statut, DateSoumission) VALUES (%d, '%e', 1, -1, 0)",
			PlayerInfo[playerid][pSQLID], nom);
		mysql_tquery(MYSQL, q, "", "");

		wl_Etat[playerid] = WL_ETAT_FORMULAIRE;
		wl_Etape[playerid] = 1;
		Whitelist_ShowIntro(playerid);
		return 1;
	}

	new wlEtape = 1, wlStatut = -1, wlMotif[WL_LEN_MOTIF+1];
	cache_get_value_name_int(0, "Etape", wlEtape);
	cache_get_value_name_int(0, "Statut", wlStatut);
	cache_get_value_name(0, "Motif", wlMotif, sizeof(wlMotif));

	if(wlStatut == 1)
	{
		// Deja accepte mais toujours marque non actif : cas de bord (decision
		// prise pendant qu'il se connectait). On le fait entrer.
		wl_Etat[playerid] = WL_ETAT_INCONNU;
		Whitelist_LaisserEntrer(playerid);
		return 1;
	}
	if(wlStatut == 2)
	{
		wl_Etat[playerid] = WL_ETAT_INCONNU;
		Whitelist_AppliquerRefus(PlayerInfo[playerid][pSQLID], playerid, wlMotif);
		return 1;
	}
	if(wlStatut == 0)
	{
		wl_Etat[playerid] = WL_ETAT_ENVOYE;
		Whitelist_ShowAttente(playerid);
		return 1;
	}

	// Brouillon : on reprend la ou il s'etait arrete.
	if(wlEtape < 1) wlEtape = 1;
	if(wlEtape > 5)
	{
		// Incoherence (5 reponses mais jamais soumis) : on considere le
		// dossier complet plutot que de bloquer le joueur.
		wl_Etat[playerid] = WL_ETAT_FORMULAIRE;
		wl_Etape[playerid] = 5;
		Whitelist_ShowQuestion(playerid);
		return 1;
	}
	wl_Etat[playerid] = WL_ETAT_FORMULAIRE;
	wl_Etape[playerid] = wlEtape;
	if(wlEtape == 1) Whitelist_ShowIntro(playerid);
	else Whitelist_ShowQuestion(playerid);
	return 1;
}

// ======================================================================
//  SAISIE DU FORMULAIRE
// ======================================================================

// Ecrit UNE reponse texte dans le brouillon. `colonne` est un litteral du
// code (jamais une donnee joueur), `valeur` est deja nettoyee/bornee et part
// via '%e'.
stock wl_SauverTexte(playerid, const colonne[], const valeur[], etapeSuivante)
{
	new q[512];
	mysql_format(MYSQL, q, sizeof(q),
		"UPDATE lvrp_whitelist SET %s='%e', Etape=%d WHERE SQLid=%d AND Statut=-1 LIMIT 1",
		colonne, valeur, etapeSuivante, PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "", "");
	wl_Etape[playerid] = etapeSuivante;
	return 1;
}

stock wl_SauverEntier(playerid, const colonne[], valeur, etapeSuivante)
{
	new q[256];
	format(q, sizeof(q),
		"UPDATE lvrp_whitelist SET %s=%d, Etape=%d WHERE SQLid=%d AND Statut=-1 LIMIT 1",
		colonne, valeur, etapeSuivante, PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "", "");
	wl_Etape[playerid] = etapeSuivante;
	return 1;
}

// Derniere etape : la reponse 5 et le passage en Statut=0 se font dans LA
// MEME requete. Tant que ce UPDATE n'est pas passe, le bot ne voit rien ;
// des qu'il est passe, le dossier est complet. Pas d'etat intermediaire ou
// le staff verrait un dossier tronque.
stock Whitelist_Envoyer(playerid, const source[])
{
	new q[512];
	mysql_format(MYSQL, q, sizeof(q),
		"UPDATE lvrp_whitelist SET Source='%e', Etape=6, Statut=0, Poste=0, Traite=0, DateSoumission=UNIX_TIMESTAMP() WHERE SQLid=%d AND Statut=-1 LIMIT 1",
		source, PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "", "");

	wl_Etat[playerid] = WL_ETAT_ENVOYE;
	wl_Etape[playerid] = 6;

	ShowPlayerDialog(playerid, WL_DLG_ENVOYE, DIALOG_STYLE_MSGBOX,
		"{9EC73D}>>> Candidature envoyee <<<",
		"{FFFFFF}C'est fait, ton dossier est parti !\n\n\
{FFAA00}Le staff va l'examiner sur le Discord du serveur.{FFFFFF}\n\
Des que la decision est prise, tu es mis en jeu\n\
automatiquement : {FFAA00}reste connecte{FFFFFF}, tu n'as rien\n\
d'autre a faire.\n\n\
{AAAAAA}Si tu te deconnectes, ce n'est pas grave : ta candidature\n\
reste valable, elle sera traitee quand meme.",
		"D'accord", "");

	// Heads-up sur le Discord public via le relais SORTANT deja en place
	// (afrp_discord.inc). Volontairement SANS aucune donnee du dossier :
	// ce salon est public, le contenu de la candidature n'a rien a y faire.
	// Le dossier complet est publie par le bot dans le salon staff.
	Discord_Send("Nouvelle candidature whitelist recue : un dossier est en attente de validation par le staff.", 0);

	// Le staff en jeu reste informe meme si le Discord tombe.
	new alerte[160], nom[MAX_PLAYER_NAME+1];
	GetPlayerName(playerid, nom, sizeof(nom));
	format(alerte, sizeof(alerte),
		"{FFAA00}[WHITELIST]{FFFFFF} %s vient d'envoyer sa candidature. Validation sur Discord, ou /attente en jeu.", nom);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(gPlayerLogged[i] != 1) continue;
		if(PlayerInfo[i][pAdmin] >= 1) SendClientMessage(i, 0xFFAA00FF, alerte);
	}
	return 1;
}

// Renvoie 1 si le dialog a ete consomme par ce module.
stock Whitelist_OnDialog(playerid, dialogid, response, listitem, inputtext[])
{
	#pragma unused listitem

	if(dialogid == WL_DLG_ATTENTE)
	{
		// Le joueur ne doit jamais pouvoir "fermer" cet ecran et se retrouver
		// libre de ses mouvements sans etre valide.
		if(IsPlayerConnected(playerid) && PlayerEnAttente[playerid])
			Whitelist_Demarrer(playerid);
		return 1;
	}

	if(dialogid == WL_DLG_ENVOYE)
	{
		Whitelist_ShowAttente(playerid);
		return 1;
	}

	if(dialogid == WL_DLG_INTRO)
	{
		wl_Etape[playerid] = 1;
		wl_Etat[playerid] = WL_ETAT_FORMULAIRE;
		Whitelist_ShowQuestion(playerid);
		return 1;
	}

	if(dialogid < WL_DLG_NOMRP || dialogid > WL_DLG_SOURCE) return 0;

	// A partir d'ici on est dans le formulaire. Un clic sur la croix ou un
	// champ vide ne fait pas sortir du parcours : on repose la question.
	if(!response)
	{
		Whitelist_ShowQuestion(playerid, "{FF6347}Cette question est obligatoire.{FFFFFF}\n\n");
		return 1;
	}

	switch(dialogid)
	{
		case WL_DLG_NOMRP:
		{
			new val[WL_LEN_NOMRP+1];
			wl_Nettoyer(val, inputtext, WL_LEN_NOMRP);
			if(!wl_NomRPValide(val))
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Nom invalide : format Prenom_Nom, lettres uniquement.{FFFFFF}\n\n");
				return 1;
			}
			wl_SauverTexte(playerid, "NomRP", val, 2);
			Whitelist_ShowQuestion(playerid);
		}
		case WL_DLG_AGERP:
		{
			new val[16];
			wl_Nettoyer(val, inputtext, 15);
			if(!wl_EstNumerique(val))
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Entre uniquement un nombre.{FFFFFF}\n\n");
				return 1;
			}
			new age = strval(val);
			if(age < WL_AGERP_MIN || age > WL_AGERP_MAX)
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Age RP hors des limites autorisees.{FFFFFF}\n\n");
				return 1;
			}
			wl_SauverEntier(playerid, "AgeRP", age, 3);
			Whitelist_ShowQuestion(playerid);
		}
		case WL_DLG_BACKGROUND:
		{
			new val[WL_LEN_BACKGROUND+1];
			new len = wl_Nettoyer(val, inputtext, WL_LEN_BACKGROUND);
			if(len < WL_MIN_BACKGROUND)
			{
				new err[128];
				format(err, sizeof(err), "{FF6347}Trop court (%d/%d caracteres minimum).{FFFFFF}\n\n", len, WL_MIN_BACKGROUND);
				Whitelist_ShowQuestion(playerid, err);
				return 1;
			}
			wl_SauverTexte(playerid, "Background", val, 4);
			Whitelist_ShowQuestion(playerid);
		}
		case WL_DLG_AGEREEL:
		{
			new val[16];
			wl_Nettoyer(val, inputtext, 15);
			if(!wl_EstNumerique(val))
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Entre uniquement un nombre.{FFFFFF}\n\n");
				return 1;
			}
			new age = strval(val);
			// On n'applique AUCUN refus automatique sur l'age reel : la regle
			// d'age est une decision de la direction, pas du script. La valeur
			// est transmise telle quelle au staff, qui tranche sur Discord
			// (le bot met en evidence les ages sous le seuil configure).
			if(age < WL_AGEREEL_MIN || age > WL_AGEREEL_MAX)
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Age invalide.{FFFFFF}\n\n");
				return 1;
			}
			wl_SauverEntier(playerid, "AgeReel", age, 5);
			Whitelist_ShowQuestion(playerid);
		}
		case WL_DLG_SOURCE:
		{
			new val[WL_LEN_SOURCE+1];
			new len = wl_Nettoyer(val, inputtext, WL_LEN_SOURCE);
			if(len < WL_MIN_SOURCE)
			{
				Whitelist_ShowQuestion(playerid, "{FF6347}Reponse trop courte.{FFFFFF}\n\n");
				return 1;
			}
			Whitelist_Envoyer(playerid, val);
		}
	}
	return 1;
}

// ======================================================================
//  APPLICATION DES DECISIONS (chemin PARTAGE Discord + staff en jeu)
// ======================================================================

// LE point unique qui fait reellement entrer un joueur. Le chemin Discord ET
// /approuver passent tous les deux par ici, precisement pour que les deux
// corrections ci-dessous ne puissent plus etre perdues par duplication de code.
//
// [FIX 1 - MOT DE PASSE] Ne JAMAIS relancer inscription_StartIntro ici.
// Cette fonction finit par appeler OnPlayerRegister -> MySQLCreateAccount ->
// sha1(pass) sur un pKey qui contient DEJA le hash SHA1 charge depuis la base
// au moment du login en attente. Resultat : double hash, mot de passe corrompu
// en base, le joueur ne peut plus jamais se connecter. Le joueur est deja
// authentifie a ce stade : on ne fait que le spawner.
//
// [FIX 2 - CLIENT FIGE] Le joueur a encore le dialog d'attente (9903) ouvert
// a l'ecran depuis sa connexion. Spawner cote serveur ne suffit pas : tant que
// ce dialog reste affiche, le client reste bloque (d'ou l'ancien "il doit se
// deconnecter et revenir"). On le ferme explicitement (dialogid -1 = fermer le
// dialog actif) AVANT de spawner.
stock Whitelist_LaisserEntrer(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;

	PlayerEnAttente[playerid] = false;
	PlayerInfo[playerid][pActive] = 1;
	inscription_Step[playerid] = 0;
	Whitelist_Reset(playerid);

	new q[128];
	format(q, sizeof(q), "UPDATE lvrp_users SET active=1 WHERE id=%d LIMIT 1", PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "", "");

	SendClientMessage(playerid, 0x00FF00FF, "{9EC73D}[WHITELIST]{FFFFFF} Ta candidature a ete ACCEPTEE ! Bienvenue sur La Vie RolePlay !");

	// [FIX 2] Fermeture explicite du dialog d'attente avant le spawn.
	ShowPlayerDialog(playerid, -1, DIALOG_STYLE_MSGBOX, "", "", "", "");

	server_TogglePlayerSpectating(playerid, 0);
	server_SetPlayerVirtualWorld(playerid, 0);
	SpawnPlayer(playerid);
	// Garde-fou : s'assurer que les controles sont bien actifs post-spawn.
	TogglePlayerControllable(playerid, 1);
	return 1;
}

// Refus applique. `playerid` peut valoir INVALID_PLAYER_ID : le staff peut
// refuser depuis Discord un joueur qui s'est deconnecte entre-temps, et la
// suppression du compte doit se faire quand meme - sinon il reviendrait dans
// un etat "en attente" eternel, sans aucun moyen de recandidater.
//
// Le compte est SUPPRIME (et pas juste laisse inactif) : c'est deja ce que
// faisait /refuser, et c'est ce qui permet au joueur refuse de recreer un
// compte et de retenter sa chance avec un meilleur dossier.
stock Whitelist_AppliquerRefus(wlSqlid, playerid, const wlMotif[])
{
	if(wlSqlid > 0)
	{
		new q[128];
		format(q, sizeof(q), "DELETE FROM lvrp_users WHERE id=%d LIMIT 1", wlSqlid);
		mysql_tquery(MYSQL, q, "", "");
	}

	if(playerid != INVALID_PLAYER_ID && IsPlayerConnected(playerid))
	{
		new msg[256];
		if(strlen(wlMotif) > 0)
			format(msg, sizeof(msg), "{FF6347}[WHITELIST]{FFFFFF} Ta candidature a ete refusee. Raison : %s", wlMotif);
		else
			format(msg, sizeof(msg), "{FF6347}[WHITELIST]{FFFFFF} Ta candidature a ete refusee par le staff.");
		SendClientMessage(playerid, 0xFF0000FF, msg);
		SendClientMessage(playerid, 0xFFFFFFFF, "{FFFFFF}Tu peux repostuler en te reconnectant et en soignant ton dossier.");

		ShowPlayerDialog(playerid, -1, DIALOG_STYLE_MSGBOX, "", "", "", "");
		Whitelist_Reset(playerid);
		SetTimerEx("TimerKick", 3000, 0, "i", playerid);
	}
	return 1;
}

// Ecrit une decision prise EN JEU dans la table, pour que le bot ne reposte
// pas un dossier deja tranche (Poste=1) et que le sondage ne rejoue pas la
// decision (Traite=1). Le garde-fou Statut<=0 evite d'ecraser une decision
// Discord arrivee entre-temps.
stock Whitelist_MarquerDecision(wlSqlid, wlStatut, const wlMotif[], const wlDecidePar[])
{
	if(wlSqlid < 1) return 0;
	new q[512];
	mysql_format(MYSQL, q, sizeof(q),
		"UPDATE lvrp_whitelist SET Statut=%d, Motif='%e', DecidePar='%e', DateDecision=UNIX_TIMESTAMP(), Poste=1, Traite=1 WHERE SQLid=%d AND Statut<=0 ORDER BY id DESC LIMIT 1",
		wlStatut, wlMotif, wlDecidePar, wlSqlid);
	mysql_tquery(MYSQL, q, "", "");
	return 1;
}

stock wl_TrouverJoueurParSQLID(wlSqlid)
{
	if(wlSqlid < 1) return INVALID_PLAYER_ID;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pSQLID] == wlSqlid) return i;
	}
	return INVALID_PLAYER_ID;
}

// ======================================================================
//  SONDAGE DES DECISIONS ECRITES PAR LE BOT
// ======================================================================

public WL_Poll()
{
	// Un seul sondage en vol a la fois : sans ce verrou, une base lente
	// pourrait empiler des SELECT et faire appliquer deux fois la meme
	// decision (le Traite=1 n'etant ecrit qu'au retour du premier).
	if(wl_PollEnCours) return 1;
	wl_PollEnCours = true;
	mysql_tquery(MYSQL,
		"SELECT id, SQLid, Statut, Motif, DecidePar FROM lvrp_whitelist WHERE Statut IN (1,2) AND Traite=0 ORDER BY id ASC LIMIT 20",
		"WL_OnPollResult", "");
	return 1;
}

public WL_OnPollResult()
{
	wl_PollEnCours = false;

	new rows = 0;
	cache_get_row_count(rows);
	if(rows <= 0) return 1;

	for(new r = 0; r < rows; r++)
	{
		new id = 0, wlSqlid = 0, wlStatut = 0;
		new wlMotif[WL_LEN_MOTIF+1], wlDecidePar[65];
		cache_get_value_name_int(r, "id", id);
		cache_get_value_name_int(r, "SQLid", wlSqlid);
		cache_get_value_name_int(r, "Statut", wlStatut);
		cache_get_value_name(r, "Motif", wlMotif, sizeof(wlMotif));
		cache_get_value_name(r, "DecidePar", wlDecidePar, sizeof(wlDecidePar));

		new playerid = wl_TrouverJoueurParSQLID(wlSqlid);

		if(wlStatut == 1)
		{
			if(playerid != INVALID_PLAYER_ID)
			{
				Whitelist_LaisserEntrer(playerid);
			}
			else if(wlSqlid > 0)
			{
				// Joueur hors ligne : on debloque juste le compte, il entrera
				// normalement a sa prochaine connexion.
				new q[128];
				format(q, sizeof(q), "UPDATE lvrp_users SET active=1 WHERE id=%d LIMIT 1", wlSqlid);
				mysql_tquery(MYSQL, q, "", "");
			}
		}
		else if(wlStatut == 2)
		{
			Whitelist_AppliquerRefus(wlSqlid, playerid, wlMotif);
		}

		// Marque la decision comme appliquee AVANT tout le reste des lignes :
		// meme si le serveur redemarre juste apres, elle ne sera pas rejouee.
		new mq[160];
		format(mq, sizeof(mq), "UPDATE lvrp_whitelist SET Traite=1 WHERE id=%d LIMIT 1", id);
		mysql_tquery(MYSQL, mq, "", "");

		// Trace pour le staff en jeu (qui n'est pas forcement sur Discord).
		if(wlDecidePar[0] == 0) format(wlDecidePar, sizeof(wlDecidePar), "le staff");
		new verdict[16];
		if(wlStatut == 1) format(verdict, sizeof(verdict), "ACCEPTEE");
		else format(verdict, sizeof(verdict), "REFUSEE");

		new log[192];
		format(log, sizeof(log), "{FFAA00}[WHITELIST]{FFFFFF} Decision Discord de %s : candidature #%d %s.",
			wlDecidePar, id, verdict);
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			if(gPlayerLogged[i] != 1) continue;
			if(PlayerInfo[i][pAdmin] >= 1) SendClientMessage(i, 0xFFAA00FF, log);
		}
	}
	return 1;
}
