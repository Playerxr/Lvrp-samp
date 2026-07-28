/*
	afrp_braquage.inc - Braquages cooperatifs

	Inspire de Payday 2 et des heists de GTA Online. Une equipe de 2 a 4
	joueurs prepare un braquage, enchaine trois phases de competence sur place,
	puis doit fuir avec le butin pendant que la police rapplique.

	  /braquage                 infos, etat, aide
	  /braquage creer           constituer une equipe sur un lieu de braquage
	  /braquage rejoindre       rejoindre l'equipe (sur place)
	  /braquage quitter         quitter l'equipe
	  /braquage lancer          le chef lance : chacun paie son materiel
	  /braquage lieu <nom>      (admin) definir un lieu ici
	  /braquage lieux           liste des lieux
	  /braquage lieusuppr <n>   (admin) supprimer un lieu

	=== POURQUOI LES LIEUX SONT DEFINIS PAR LES ADMINS ===
	Aucune coordonnee n'est ecrite en dur dans ce fichier. Inventer des
	positions sans pouvoir les verifier en jeu, c'est risquer des braquages
	dans un mur ou sous la map. Les lieux se posent avec /braquage lieu, comme
	les depots cargo, et sont sauvegardes.

	=== TROIS PHASES, TROIS JOUEURS DIFFERENTS ===
	Forcer la porte, pirater l'alarme, percer le coffre. Chaque phase est un
	minijeu de competence (afrp_minijeu). Tant qu'il reste des membres qui
	n'ont pas encore joue, la phase suivante est confiee a l'un d'eux : c'est
	ce qui rend l'equipe reellement necessaire au lieu d'un chef qui fait tout
	pendant que les autres regardent.

	=== L'ARGENT ===
	Chaque membre paie SON materiel au lancement (80.000$) et touche SA part a
	l'arrivee (350.000$). Le risque est partage, pas supporte par le seul chef.
	Un membre qui meurt pendant le braquage est elimine : sa part n'est versee
	a personne, elle disparait. La police n'a donc pas besoin d'arreter qui que
	ce soit pour faire mal : chaque mort coute 350.000$ a l'equipe.

	Un braquage rate coute 80.000$ par personne et ne rapporte rien. Avec un
	delai de 60 minutes entre deux braquages sur le serveur, le gain horaire
	reste comparable a celui d'un convoi livre.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Br_Init()
	- OnPlayerDisconnect -> Br_OnDisconnect(playerid)
	- OnPlayerDeath      -> Br_OnDeath(playerid)
	- /braquage          -> Br_Command(playerid, cmdtext)
*/

#if defined AFRP_BRAQUAGE_INCLUDED
	#endinput
#endif
#define AFRP_BRAQUAGE_INCLUDED

#define BR_OFF              0
#define BR_EQUIPE           1       // recrutement
#define BR_ENCOURS          2       // phases sur place
#define BR_FUITE            3       // butin en main, il faut s'eloigner

#define BR_MAX_LIEU         10
#define BR_MAX_MEMBRE       4
#define BR_MIN_MEMBRE       2
#define BR_FILE             "braquages.cfg"
#define BR_ADMIN_LEVEL      3

#define BR_EQUIP_COST       80000   // materiel, paye par chaque membre
#define BR_PART             350000  // part de butin par membre arrive
#define BR_PHASES           3
#define BR_PHASE_SEC        150     // temps pour reussir une phase
#define BR_FUITE_SEC        300     // temps pour s'eloigner
#define BR_FUITE_DIST       500.0   // distance a parcourir pour etre en fuite
#define BR_RAYON            12.0    // distance max du lieu pendant les phases
#define BR_COOLDOWN_MIN     60
#define BR_TICK_MS          5000
#define BR_ICON             67

#define DIALOG_BR_MENU      9630

new brLieuNom[BR_MAX_LIEU][40];
new Float:brLieuX[BR_MAX_LIEU], Float:brLieuY[BR_MAX_LIEU], Float:brLieuZ[BR_MAX_LIEU];
new brLieuPris[BR_MAX_LIEU];

new brState = BR_OFF;
new brLieu = -1;
new brChef = INVALID_PLAYER_ID;
new brPhase;                        // 1..BR_PHASES
new brPhaseFin;                     // gettime limite de la phase
new brFuiteFin;
new brNextTs;                       // fin du delai entre deux braquages
new brActeur = INVALID_PLAYER_ID;   // qui joue la phase en cours

new brIn[MAX_PLAYERS];              // 1 = membre de l'equipe
new brVivant[MAX_PLAYERS];          // 0 = elimine (mort pendant le coup)
new brAJoue[MAX_PLAYERS];           // 1 = a deja fait une phase

// ============================================================================
// OUTILS
// ============================================================================
stock Br_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Br_Equipe(const text[])
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1) continue;
		SendClientMessage(i, 0xFF8C00FF, text);
	}
	return 1;
}

stock Br_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= BR_ADMIN_LEVEL);
}

stock Br_CountMembres()
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && brIn[i] == 1) n++;
	return n;
}

stock Br_CountVivants()
{
	new n = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && brIn[i] == 1 && brVivant[i] == 1) n++;
	return n;
}

stock Br_PhaseNom(p, dest[], destSize)
{
	switch(p)
	{
		case 1: format(dest, destSize, "Forcer la porte");
		case 2: format(dest, destSize, "Pirater l'alarme");
		case 3: format(dest, destSize, "Percer le coffre");
		default: format(dest, destSize, "Phase %d", p);
	}
	return 1;
}

// Lieu de braquage a portee du joueur, -1 sinon.
stock Br_LieuProche(playerid)
{
	for(new l = 0; l < BR_MAX_LIEU; l++)
	{
		if(brLieuPris[l] != 1) continue;
		if(IsPlayerInRangeOfPoint(playerid, BR_RAYON, brLieuX[l], brLieuY[l], brLieuZ[l])) return l;
	}
	return -1;
}

// Prochain membre vivant qui n'a pas encore joue de phase ; si tous ont joue,
// n'importe quel vivant. C'est ce qui force la rotation dans l'equipe.
stock Br_ProchainActeur()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && brIn[i] == 1 && brVivant[i] == 1 && brAJoue[i] == 0) return i;
	for(new i = 0; i < MAX_PLAYERS; i++)
		if(IsPlayerConnected(i) && brIn[i] == 1 && brVivant[i] == 1) return i;
	return INVALID_PLAYER_ID;
}

// ============================================================================
// PERSISTANCE DES LIEUX
// ============================================================================
stock Br_Field(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Br_SaveLieux()
{
	if(fexist(BR_FILE)) fremove(BR_FILE);
	new File:f = fopen(BR_FILE, io_write);
	if(!f) return 0;
	new line[128];
	for(new l = 0; l < BR_MAX_LIEU; l++)
	{
		if(brLieuPris[l] != 1) continue;
		format(line, sizeof(line), "%.4f|%.4f|%.4f|%s\n", brLieuX[l], brLieuY[l], brLieuZ[l], brLieuNom[l]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Br_LoadLieux()
{
	for(new l = 0; l < BR_MAX_LIEU; l++) brLieuPris[l] = 0;
	if(!fexist(BR_FILE)) return 0;
	new File:f = fopen(BR_FILE, io_read);
	if(!f) return 0;

	new line[160], buf[48], n = 0;
	while(fread(f, line) && n < BR_MAX_LIEU)
	{
		if(strlen(line) < 5) continue;
		new k = 0;
		Br_Field(line, k, buf, sizeof(buf)); brLieuX[n] = floatstr(buf);
		Br_Field(line, k, buf, sizeof(buf)); brLieuY[n] = floatstr(buf);
		Br_Field(line, k, buf, sizeof(buf)); brLieuZ[n] = floatstr(buf);
		Br_Field(line, k, brLieuNom[n], 40);
		if(!strlen(brLieuNom[n])) continue;
		brLieuPris[n] = 1;
		n++;
	}
	fclose(f);
	printf("[Braquage] %d lieu(x) charge(s)", n);
	return n;
}

// ============================================================================
// CYCLE DE VIE
// ============================================================================
stock Br_ClearIcons()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		RemovePlayerMapIcon(i, BR_ICON);
	}
	return 1;
}

stock Br_Reset()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(brIn[i] == 1 && IsPlayerConnected(i)) Mini_Abandon(i);
		brIn[i] = 0; brVivant[i] = 0; brAJoue[i] = 0;
	}
	Br_ClearIcons();
	Annonce_PermClear(ANNP_SRC_BRAQUAGE);
	brState  = BR_OFF;
	brLieu   = -1;
	brChef   = INVALID_PLAYER_ID;
	brActeur = INVALID_PLAYER_ID;
	brPhase  = 0;
	return 1;
}

// Pas de 'const' : raison part dans Annonce_All, qui le transmet a
// PlayerTextDrawSetString - un natif qui refuse les tableaux const.
stock Br_Echec(raison[])
{
	new s[200];
	format(s, sizeof(s), "[BRAQUAGE] Le coup a echoue : %s", raison);
	SendClientMessageToAll(0xFF8C00FF, s);
	Annonce_All("~r~BRAQUAGE ECHOUE", raison, 0xFF4444FF, 7000);
	Br_Equipe("{FF4444}Le materiel est perdu. Rien n'est verse.");
	brNextTs = gettime() + BR_COOLDOWN_MIN * 60;
	Br_Reset();
	return 1;
}

// ============================================================================
// EQUIPE
// ============================================================================
stock Br_Creer(playerid)
{
	if(brState != BR_OFF) return Br_Msg(playerid, "{FF4444}Un braquage est deja en preparation ou en cours.");
	if(gettime() < brNextTs)
	{
		new att[144];
		format(att, sizeof(att), "{FF4444}Prochain braquage possible dans %d minute(s).", (brNextTs - gettime()) / 60 + 1);
		return Br_Msg(playerid, att);
	}
	new l = Br_LieuProche(playerid);
	if(l == -1) return Br_Msg(playerid, "{FF4444}Tu n'es pas sur un lieu de braquage. /braquage lieux pour la liste.");

	Br_Reset();
	brState = BR_EQUIPE;
	brLieu  = l;
	brChef  = playerid;
	brIn[playerid] = 1;
	brVivant[playerid] = 1;

	new s[200];
	format(s, sizeof(s), "{00FF00}Equipe constituee sur {FFFF00}%s{FFFFFF}. Il faut au moins %d membres.", brLieuNom[l], BR_MIN_MEMBRE);
	Br_Msg(playerid, s);
	format(s, sizeof(s), "{AAAAAA}Les autres tapent /braquage rejoindre sur place. Puis /braquage lancer (%d$ de materiel chacun).", BR_EQUIP_COST);
	Br_Msg(playerid, s);
	return 1;
}

stock Br_Rejoindre(playerid)
{
	if(brState != BR_EQUIPE) return Br_Msg(playerid, "{FF4444}Aucune equipe ne recrute en ce moment.");
	if(brIn[playerid] == 1) return Br_Msg(playerid, "{FFFF00}Tu es deja dans l'equipe.");
	if(Br_CountMembres() >= BR_MAX_MEMBRE) return Br_Msg(playerid, "{FF4444}L'equipe est complete.");
	if(Br_LieuProche(playerid) != brLieu) return Br_Msg(playerid, "{FF4444}Tu dois etre sur le lieu du braquage.");

	brIn[playerid] = 1;
	brVivant[playerid] = 1;

	new s[176];
	format(s, sizeof(s), "[BRAQUAGE] %s rejoint l'equipe (%d/%d).", PlayerInfo[playerid][pName], Br_CountMembres(), BR_MAX_MEMBRE);
	Br_Equipe(s);
	return 1;
}

stock Br_Quitter(playerid)
{
	if(brIn[playerid] != 1) return Br_Msg(playerid, "{FF4444}Tu n'es dans aucune equipe.");
	if(brState != BR_EQUIPE) return Br_Msg(playerid, "{FF4444}Trop tard pour se retirer, le coup est lance.");

	brIn[playerid] = 0;
	brVivant[playerid] = 0;
	Br_Msg(playerid, "{FFFF00}Tu quittes l'equipe.");

	if(playerid == brChef || Br_CountMembres() == 0)
	{
		SendClientMessageToAll(0xFF8C00FF, "[BRAQUAGE] L'equipe s'est dispersee avant le coup.");
		Br_Reset();
	}
	return 1;
}

// ============================================================================
// LANCEMENT ET PHASES
// ============================================================================
stock Br_Lancer(playerid)
{
	if(brState != BR_EQUIPE) return Br_Msg(playerid, "{FF4444}Aucune equipe prete.");
	if(playerid != brChef)   return Br_Msg(playerid, "{FF4444}Seul le chef d'equipe lance le coup.");
	if(Br_CountMembres() < BR_MIN_MEMBRE)
	{
		new s1[160];
		format(s1, sizeof(s1), "{FF4444}Il faut au moins %d membres (vous etes %d).", BR_MIN_MEMBRE, Br_CountMembres());
		return Br_Msg(playerid, s1);
	}

	// Tout le monde doit etre sur place ET pouvoir payer, AVANT de debiter qui
	// que ce soit : sinon un membre absent ou fauche laisserait les autres
	// payes pour rien.
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1) continue;
		if(Br_LieuProche(i) != brLieu)
		{
			new s2[176];
			format(s2, sizeof(s2), "{FF4444}%s n'est pas sur le lieu du braquage.", PlayerInfo[i][pName]);
			return Br_Msg(playerid, s2);
		}
		if(PlayerInfo[i][pCash] < BR_EQUIP_COST)
		{
			new s3[176];
			format(s3, sizeof(s3), "{FF4444}%s n'a pas les %d$ de materiel.", PlayerInfo[i][pName], BR_EQUIP_COST);
			return Br_Msg(playerid, s3);
		}
	}

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1) continue;
		SafeGivePlayerMoney(i, -BR_EQUIP_COST, "Materiel de braquage");
		brAJoue[i] = 0;
	}

	brState = BR_ENCOURS;
	brPhase = 0;

	// Le lieu est signale a tout le serveur : sans cela, la police n'aurait
	// aucun moyen de savoir ou aller, et le braquage serait sans risque.
	new s[200];
	format(s, sizeof(s), "[BRAQUAGE] Alarme ! Un braquage est en cours a %s. Police, intervenez.", brLieuNom[brLieu]);
	SendClientMessageToAll(0xFF4444FF, s);
	format(s, sizeof(s), "BRAQUAGE EN COURS  -  %s", brLieuNom[brLieu]);
	Annonce_PermSet(ANNP_SRC_BRAQUAGE, s, 0xFF4444FF);
	Annonce_All("~r~BRAQUAGE", brLieuNom[brLieu], 0xFF4444FF, 9000);

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		SetPlayerMapIcon(i, BR_ICON, brLieuX[brLieu], brLieuY[brLieu], brLieuZ[brLieu], 30, 0xFF0000FF, MAPICON_GLOBAL);
	}

	Br_PhaseSuivante();
	return 1;
}

stock Br_PhaseSuivante()
{
	if(brState != BR_ENCOURS) return 0;

	brPhase++;
	if(brPhase > BR_PHASES)
	{
		// Coffre ouvert : place a la fuite.
		brState    = BR_FUITE;
		brFuiteFin = gettime() + BR_FUITE_SEC;
		brActeur   = INVALID_PLAYER_ID;

		new s[200];
		format(s, sizeof(s), "{00FF00}Le coffre est ouvert. Eloignez-vous de %d metres en moins de %d minutes !", floatround(BR_FUITE_DIST), BR_FUITE_SEC / 60);
		Br_Equipe(s);
		SendClientMessageToAll(0xFF8C00FF, "[BRAQUAGE] Le coffre est ouvert, l'equipe prend la fuite !");
		Annonce_All("~r~LES BRAQUEURS FUIENT", "Ils quittent les lieux avec le butin", 0xFF4444FF, 8000);
		return 1;
	}

	new acteur = Br_ProchainActeur();
	if(acteur == INVALID_PLAYER_ID) return Br_Echec("il ne reste plus personne dans l'equipe"), 0;

	brActeur   = acteur;
	brAJoue[acteur] = 1;
	brPhaseFin = gettime() + BR_PHASE_SEC;

	new nom[40], s[200];
	Br_PhaseNom(brPhase, nom, sizeof(nom));
	format(s, sizeof(s), "[BRAQUAGE] Phase %d/%d : %s  -  %s s'en charge.", brPhase, BR_PHASES, nom, PlayerInfo[acteur][pName]);
	Br_Equipe(s);

	new titre[48];
	format(titre, sizeof(titre), "~r~%s", nom);
	Mini_Start(acteur, "Br_PhaseOk", "Br_PhaseKo", 0, 3 + brPhase, 4 + brPhase, titre);
	return 1;
}

forward Br_PhaseOk(playerid, param);
public Br_PhaseOk(playerid, param)
{
	#pragma unused param
	if(brState != BR_ENCOURS || brActeur != playerid) return 1;
	new nom[40], s[176];
	Br_PhaseNom(brPhase, nom, sizeof(nom));
	format(s, sizeof(s), "[BRAQUAGE] %s a reussi : %s.", PlayerInfo[playerid][pName], nom);
	Br_Equipe(s);
	Br_PhaseSuivante();
	return 1;
}

forward Br_PhaseKo(playerid, param);
public Br_PhaseKo(playerid, param)
{
	#pragma unused param
	if(brState != BR_ENCOURS || brActeur != playerid) return 1;

	// Un ratage ne condamne pas le coup : un autre membre peut reprendre la
	// meme phase tant que le temps imparti n'est pas ecoule. C'est ce qui rend
	// l'equipe utile plutot que punitive.
	new relais = INVALID_PLAYER_ID;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1 || brVivant[i] != 1) continue;
		if(i == playerid) continue;
		if(Br_LieuProche(i) != brLieu) continue;
		relais = i;
		break;
	}
	if(relais == INVALID_PLAYER_ID || gettime() >= brPhaseFin)
		return Br_Echec("l'equipe n'a pas su ouvrir le passage"), 1;

	new nom[40], s[190];
	Br_PhaseNom(brPhase, nom, sizeof(nom));
	format(s, sizeof(s), "[BRAQUAGE] %s a echoue. %s prend le relais.", PlayerInfo[playerid][pName], PlayerInfo[relais][pName]);
	Br_Equipe(s);

	brActeur = relais;
	brAJoue[relais] = 1;
	new titre[48];
	format(titre, sizeof(titre), "~r~%s", nom);
	Mini_Start(relais, "Br_PhaseOk", "Br_PhaseKo", 0, 3 + brPhase, 4 + brPhase, titre);
	return 1;
}

// ============================================================================
// REUSSITE
// ============================================================================
stock Br_Reussite()
{
	new n = 0, s[200];
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1 || brVivant[i] != 1) continue;
		SafeGivePlayerMoney(i, BR_PART, "Part de braquage");
		Saison_AddXp(i, SAI_XP_CONVOI);
		format(s, sizeof(s), "{00FF00}Braquage reussi : {FFFF00}%d${FFFFFF} pour ta part.", BR_PART);
		Br_Msg(i, s);
		n++;
	}
	format(s, sizeof(s), "[BRAQUAGE] L'equipe s'en est sortie : %d braqueur(s) avec le butin.", n);
	SendClientMessageToAll(0xFF8C00FF, s);
	Annonce_All("~g~BRAQUAGE REUSSI", "L'equipe a disparu dans la nature", 0x00FF00FF, 8000);

	brNextTs = gettime() + BR_COOLDOWN_MIN * 60;
	Br_Reset();
	return 1;
}

// ============================================================================
// TICK
// ============================================================================
forward Br_Tick();
public Br_Tick()
{
	if(brState == BR_OFF || brState == BR_EQUIPE) return 1;

	if(Br_CountVivants() == 0) return Br_Echec("plus aucun braqueur debout"), 1;

	if(brState == BR_ENCOURS)
	{
		if(gettime() >= brPhaseFin) return Br_Echec("l'equipe a ete trop lente"), 1;

		// Un membre qui s'eloigne pendant les phases abandonne le coup : sans
		// cette regle, un joueur pourrait rester a l'abri pendant que les
		// autres prennent les risques et toucher quand meme sa part.
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || brIn[i] != 1 || brVivant[i] != 1) continue;
			if(Br_LieuProche(i) == brLieu) continue;
			brVivant[i] = 0;
			Mini_Abandon(i);
			Br_Msg(i, "{FF4444}Tu as quitte les lieux : tu es hors du coup, sans part.");
			new s[176];
			format(s, sizeof(s), "[BRAQUAGE] %s a lache l'equipe.", PlayerInfo[i][pName]);
			Br_Equipe(s);
		}
		return 1;
	}

	// --- fuite ---
	if(gettime() >= brFuiteFin) return Br_Echec("la police a boucle le quartier"), 1;

	new loin = 0, total = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || brIn[i] != 1 || brVivant[i] != 1) continue;
		total++;
		if(!IsPlayerInRangeOfPoint(i, BR_FUITE_DIST, brLieuX[brLieu], brLieuY[brLieu], brLieuZ[brLieu])) loin++;
	}
	// Tous les survivants sont assez loin : le coup est boucle.
	if(total > 0 && loin == total) return Br_Reussite(), 1;
	return 1;
}

// ============================================================================
// HOOKS
// ============================================================================
stock Br_OnDeath(playerid)
{
	if(brIn[playerid] != 1 || brVivant[playerid] != 1) return 0;
	if(brState == BR_OFF || brState == BR_EQUIPE) return 0;

	brVivant[playerid] = 0;
	Mini_Abandon(playerid);
	Br_Msg(playerid, "{FF4444}Tu es hors du coup. Ta part est perdue.");

	new s[176];
	format(s, sizeof(s), "[BRAQUAGE] %s est tombe. Sa part est perdue.", PlayerInfo[playerid][pName]);
	Br_Equipe(s);

	// Si c'est lui qui tenait la phase, un autre doit reprendre tout de suite.
	if(brState == BR_ENCOURS && brActeur == playerid) Br_PhaseKo(playerid, 0);
	return 1;
}

stock Br_OnDisconnect(playerid)
{
	if(brIn[playerid] != 1) return 0;
	if(brState == BR_EQUIPE) { brIn[playerid] = 0; brVivant[playerid] = 0; return 1; }
	Br_OnDeath(playerid);
	brIn[playerid] = 0;
	return 1;
}

// ============================================================================
// LIEUX (admin)
// ============================================================================
stock Br_AjouterLieu(playerid, nom[])
{
	if(!Br_IsAdmin(playerid)) return Br_Msg(playerid, "{FF4444}Reserve aux administrateurs.");
	if(!strlen(nom)) return Br_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/braquage lieu <nom>");

	new l = -1;
	for(new k = 0; k < BR_MAX_LIEU; k++)
		if(brLieuPris[k] == 0) { l = k; break; }
	if(l == -1) return Br_Msg(playerid, "{FF4444}Nombre maximum de lieux atteint.");

	GetPlayerPos(playerid, brLieuX[l], brLieuY[l], brLieuZ[l]);
	format(brLieuNom[l], 40, "%s", nom);
	brLieuPris[l] = 1;
	Br_SaveLieux();

	new s[176];
	format(s, sizeof(s), "{00FF00}Lieu de braquage \"%s\" cree ici (numero %d).", brLieuNom[l], l + 1);
	return Br_Msg(playerid, s);
}

stock Br_SupprimerLieu(playerid, num)
{
	if(!Br_IsAdmin(playerid)) return Br_Msg(playerid, "{FF4444}Reserve aux administrateurs.");
	new l = num - 1;
	if(l < 0 || l >= BR_MAX_LIEU || brLieuPris[l] != 1) return Br_Msg(playerid, "{FF4444}Numero invalide. /braquage lieux");
	if(brState != BR_OFF && brLieu == l) return Br_Msg(playerid, "{FF4444}Un braquage est en cours sur ce lieu.");
	brLieuPris[l] = 0;
	Br_SaveLieux();
	return Br_Msg(playerid, "{FFFF00}Lieu supprime.");
}

stock Br_ListeLieux(playerid)
{
	new body[900], str[128], n = 0;
	body[0] = 0;
	for(new l = 0; l < BR_MAX_LIEU; l++)
	{
		if(brLieuPris[l] != 1) continue;
		format(str, sizeof(str), "{FFFFFF}%d. %s\n", l + 1, brLieuNom[l]);
		strcat(body, str);
		n++;
	}
	if(n == 0)
	{
		Br_Msg(playerid, "{FF4444}Aucun lieu de braquage n'est defini.");
		if(Br_IsAdmin(playerid)) Br_Msg(playerid, "{AAAAAA}Place-toi ou tu veux et tape /braquage lieu <nom>.");
		return 0;
	}
	return ShowPlayerDialog(playerid, DIALOG_BR_MENU, DIALOG_STYLE_MSGBOX, "Lieux de braquage", body, "Fermer", "");
}

// ============================================================================
// INFOS
// ============================================================================
stock Br_Info(playerid)
{
	new s[200];
	if(brState == BR_OFF)
	{
		Br_Msg(playerid, "{AAAAAA}Aucun braquage en cours.");
		if(gettime() < brNextTs)
		{
			format(s, sizeof(s), "{AAAAAA}Prochain braquage possible dans %d minute(s).", (brNextTs - gettime()) / 60 + 1);
			Br_Msg(playerid, s);
		}
		format(s, sizeof(s), "{FFFFFF}/braquage creer {AAAAAA}: sur un lieu de braquage, avec %d a %d equipiers.", BR_MIN_MEMBRE, BR_MAX_MEMBRE);
		Br_Msg(playerid, s);
		format(s, sizeof(s), "{AAAAAA}Materiel %d$ par personne, part %d$ par personne. /braquage lieux", BR_EQUIP_COST, BR_PART);
		Br_Msg(playerid, s);
		return 1;
	}

	format(s, sizeof(s), "{FF8C00}[BRAQUAGE]{FFFFFF} %s - equipe : %d membre(s), %d debout", brLieuNom[brLieu], Br_CountMembres(), Br_CountVivants());
	Br_Msg(playerid, s);

	if(brState == BR_EQUIPE)
		Br_Msg(playerid, "{FFFFFF}Recrutement en cours. {AAAAAA}/braquage rejoindre sur place.");
	else if(brState == BR_ENCOURS)
	{
		new nom[40];
		Br_PhaseNom(brPhase, nom, sizeof(nom));
		format(s, sizeof(s), "{FFFFFF}Phase %d/%d : %s", brPhase, BR_PHASES, nom);
		Br_Msg(playerid, s);
	}
	else
	{
		format(s, sizeof(s), "{FFFFFF}Fuite en cours : %d seconde(s) restantes.", brFuiteFin - gettime());
		Br_Msg(playerid, s);
	}
	return 1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Br_Tok(const src[], &brPos, dest[], destSize)
{
	new p = 0;
	while(src[brPos] == ' ') brPos++;
	while(src[brPos] && src[brPos] != ' ' && p < destSize - 1)
		dest[p++] = src[brPos++];
	dest[p] = 0;
	return p;
}

stock Br_Rest(const src[], brPos, dest[], destSize)
{
	new p = 0;
	while(src[brPos] == ' ') brPos++;
	while(src[brPos] && p < destSize - 1) dest[p++] = src[brPos++];
	dest[p] = 0;
	return p;
}

stock Br_Command(playerid, const cmdline[])
{
	new brPos = 0, sub[24], arg[48];
	Br_Tok(cmdline, brPos, sub, sizeof(sub));          // consomme "/braquage"
	if(Br_Tok(cmdline, brPos, sub, sizeof(sub)) == 0) return Br_Info(playerid);

	if(strcmp(sub, "creer",     true) == 0) return Br_Creer(playerid);
	if(strcmp(sub, "rejoindre", true) == 0) return Br_Rejoindre(playerid);
	if(strcmp(sub, "quitter",   true) == 0) return Br_Quitter(playerid);
	if(strcmp(sub, "lancer",    true) == 0) return Br_Lancer(playerid);
	if(strcmp(sub, "lieux",     true) == 0) return Br_ListeLieux(playerid);

	if(strcmp(sub, "lieu", true) == 0)
	{
		Br_Rest(cmdline, brPos, arg, sizeof(arg));
		return Br_AjouterLieu(playerid, arg);
	}
	if(strcmp(sub, "lieusuppr", true) == 0)
	{
		Br_Tok(cmdline, brPos, arg, sizeof(arg));
		return Br_SupprimerLieu(playerid, strval(arg));
	}
	return Br_Info(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Br_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++) { brIn[i] = 0; brVivant[i] = 0; brAJoue[i] = 0; }
	brState  = BR_OFF;
	brNextTs = 0;
	Br_LoadLieux();
	SetTimer("Br_Tick", BR_TICK_MS, true);
	return 1;
}
