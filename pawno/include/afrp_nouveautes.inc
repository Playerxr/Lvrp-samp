/*
	afrp_nouveautes.inc - Menu des nouveautes, affiche automatiquement

	Huit systemes ont ete ajoutes au serveur sans qu'aucun n'apparaisse dans le
	tutoriel, la FAQ ou le message d'accueil. Un joueur n'avait aucun moyen de
	savoir que /convoi, /braquage, /saison, /assurance ou /position existaient :
	du travail invisible ne rapporte rien.

	Ce module affiche un menu au spawn, quelques fois, puis se tait.

	  /nouveautes  (ou /news) -> rouvrir le menu quand on veut

	=== POURQUOI UNE VERSION ET PAS UN SIMPLE "DEJA VU" ===
	NEWS_VERSION est un numero. Chaque joueur retient la derniere version qu'il
	a vue. Quand on ajoutera d'autres fonctionnalites, il suffira d'incrementer
	ce numero et de mettre a jour la liste : tout le monde reverra le menu, y
	compris les anciens joueurs. Un simple drapeau "deja vu" aurait oblige a
	effacer le fichier a la main a chaque mise a jour.

	=== POURQUOI TROIS FOIS ET PAS UNE ===
	Un joueur qui se connecte, voit un dialogue et le ferme par reflexe n'a
	rien lu. Trois affichages laissent une vraie chance de tomber dessus au
	bon moment, et le bouton "ne plus afficher" rend la main a ceux que ca
	agace des le premier jour.

	=== POURQUOI UN DELAI APRES LE SPAWN ===
	Le tutoriel des nouveaux et l'ecran de connexion ouvrent leurs propres
	dialogues. Afficher le notre immediatement les ecraserait. Huit secondes
	suffisent a laisser la place.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit   -> News_Init()
	- OnPlayerSpawn    -> News_OnSpawn(playerid)
	- OnDialogResponse -> News_OnDialog(playerid, dialogid, response, listitem)
	- /nouveautes      -> News_Cmd(playerid)
*/

#if defined AFRP_NOUVEAUTES_INCLUDED
	#endinput
#endif
#define AFRP_NOUVEAUTES_INCLUDED

// Incrementer a chaque nouvelle vague de fonctionnalites : tout le monde
// reverra le menu, anciens joueurs compris.
#define NEWS_VERSION       1
#define NEWS_MAX_AFFICHAGE 3
#define NEWS_DELAI_MS      8000
#define NEWS_MAX_JOUEUR    2000
#define NEWS_FILE          "nouveautes.cfg"
#define NEWS_NB            7

#define DIALOG_NEWS_MENU   9632
#define DIALOG_NEWS_DETAIL 9633

new newsSqlid[NEWS_MAX_JOUEUR];
new newsVersionVue[NEWS_MAX_JOUEUR];
new newsFois[NEWS_MAX_JOUEUR];
new newsPris[NEWS_MAX_JOUEUR];

new newsTitre[NEWS_NB][32] = {
	"Passe saisonnier",
	"Meilleurs joueurs du jour",
	"Assurance vehicule",
	"Convois escortes",
	"Braquages en equipe",
	"Partage de position",
	"Armes par niveau"
};

new newsCmd[NEWS_NB][20] = {
	"/saison", "/top", "/assurance", "/convoi", "/braquage", "/position", "/armes"
};

new newsDetail[NEWS_NB][400] = {
	"{FFFFFF}Une saison dure 30 jours. Tu gagnes des points en jouant normalement : minutes jouees, objectifs quotidiens, paie de job, convois, podium du jour.\n\nChaque palier franchi te paye, jusqu'a 600.000$ au dernier. A la fin de la saison tout repart a zero.\n\n{FFFF00}/saison{FFFFFF} pour voir ta progression et les paliers.",
	"{FFFFFF}Chaque jour, les joueurs les plus actifs sont classes. A minuit, les trois premiers touchent 200.000$, 100.000$ et 50.000$.\n\nRester connecte sans rien faire ne rapporte rien : il faut bouger ou parler pour que la minute compte.\n\n{FFFF00}/top{FFFFFF} pour le classement, {FFFF00}/toptd{FFFFFF} pour masquer le panneau.",
	"{FFFFFF}Assure ta voiture pour 8% de sa valeur, pendant 7 jours.\n\nEn fourriere, tu paies 5.000$ au lieu de 25.000$. Et si ta voiture est detruite, elle ressort reparee au lieu d'attendre un mecanicien.\n\n{FFFF00}/assurance{FFFFFF} pour souscrire.",
	"{FFFFFF}Une entreprise expedie un chargement de valeur d'un point A a un point B. Le depart est annonce a tout le serveur.\n\nLa police peut escorter contre une part. Les criminels peuvent braquer le camion. Le convoi est visible sur la carte de tous.\n\n{FFFF00}/convoi{FFFFFF} pour suivre celui en cours.",
	"{FFFFFF}Monte une equipe de 2 a 4 joueurs sur un lieu de braquage. Chacun paie son materiel, puis vous enchainez trois phases : forcer la porte, pirater l'alarme, percer le coffre.\n\nEnsuite il faut fuir. Chaque braqueur qui tombe perd sa part.\n\n{FFFF00}/braquage{FFFFFF} pour commencer.",
	"{FFFFFF}Partage ta position en direct avec un joueur ou avec ton entreprise, pour une duree limitee. Il te voit bouger sur sa carte.\n\nTu ne peux partager QUE ta propre position, et tu coupes quand tu veux.\n\n{FFFF00}/position <id> [minutes]{FFFFFF}, {FFFF00}/position stop{FFFFFF}.",
	"{FFFFFF}Le permis d'arme a 20.000.000$ n'existe plus. Les armes se debloquent avec ton NIVEAU.\n\nNiveau 4 : le 9mm. Puis fusil a pompe, uzi, Desert Eagle, AK-47... jusqu'au fusil a lunette au niveau 16.\n\n{FFFF00}/armes{FFFFFF} pour voir ce que tu as debloque."
};

// ============================================================================
// REGISTRE
// ============================================================================
stock News_Find(sqlid)
{
	if(sqlid <= 0) return -1;
	for(new k = 0; k < NEWS_MAX_JOUEUR; k++)
		if(newsPris[k] == 1 && newsSqlid[k] == sqlid) return k;
	return -1;
}

stock News_Entry(playerid)
{
	new my = PlayerInfo[playerid][pSQLID];
	if(my <= 0) return -1;
	new k = News_Find(my);
	if(k != -1) return k;
	for(new n = 0; n < NEWS_MAX_JOUEUR; n++)
		if(newsPris[n] == 0)
		{
			newsPris[n]       = 1;
			newsSqlid[n]      = my;
			newsVersionVue[n] = 0;
			newsFois[n]       = 0;
			return n;
		}
	return -1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock News_Field(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock News_Save()
{
	if(fexist(NEWS_FILE)) fremove(NEWS_FILE);
	new File:f = fopen(NEWS_FILE, io_write);
	if(!f) return 0;
	new line[64];
	for(new k = 0; k < NEWS_MAX_JOUEUR; k++)
	{
		if(newsPris[k] != 1) continue;
		format(line, sizeof(line), "%d|%d|%d\n", newsSqlid[k], newsVersionVue[k], newsFois[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock News_Load()
{
	for(new k = 0; k < NEWS_MAX_JOUEUR; k++) newsPris[k] = 0;
	if(!fexist(NEWS_FILE)) return 0;
	new File:f = fopen(NEWS_FILE, io_read);
	if(!f) return 0;
	new line[64], buf[24], n = 0;
	while(fread(f, line) && n < NEWS_MAX_JOUEUR)
	{
		if(strlen(line) < 3) continue;
		new k = 0;
		News_Field(line, k, buf, sizeof(buf)); newsSqlid[n]      = strval(buf);
		News_Field(line, k, buf, sizeof(buf)); newsVersionVue[n] = strval(buf);
		News_Field(line, k, buf, sizeof(buf)); newsFois[n]       = strval(buf);
		if(newsSqlid[n] <= 0) continue;
		newsPris[n] = 1;
		n++;
	}
	fclose(f);
	printf("[Nouveautes] %d joueur(s) enregistre(s), version courante %d", n, NEWS_VERSION);
	return n;
}

// ============================================================================
// AFFICHAGE
// ============================================================================
stock News_ShowMenu(playerid)
{
	new body[1000], str[96];
	body[0] = 0;
	for(new i = 0; i < NEWS_NB; i++)
	{
		format(str, sizeof(str), "{FFFFFF}%s  {FFFF00}%s{FFFFFF}\n", newsTitre[i], newsCmd[i]);
		strcat(body, str);
	}
	strcat(body, "{888888}Ne plus afficher ce message");

	return ShowPlayerDialog(playerid, DIALOG_NEWS_MENU, DIALOG_STYLE_LIST,
		"Nouveautes du serveur - clique pour le detail", body, "Voir", "Fermer");
}

stock News_ShowDetail(playerid, i)
{
	if(i < 0 || i >= NEWS_NB) return 0;
	new titre[64];
	format(titre, sizeof(titre), "%s  (%s)", newsTitre[i], newsCmd[i]);
	return ShowPlayerDialog(playerid, DIALOG_NEWS_DETAIL, DIALOG_STYLE_MSGBOX,
		titre, newsDetail[i], "Retour", "Fermer");
}

// Marque les nouveautes comme lues : le menu ne reviendra plus tout seul
// jusqu'a la prochaine version.
stock News_MarquerLu(playerid)
{
	new k = News_Entry(playerid);
	if(k == -1) return 0;
	newsVersionVue[k] = NEWS_VERSION;
	News_Save();
	return 1;
}

// ============================================================================
// AFFICHAGE AUTOMATIQUE
// ============================================================================
forward News_Differe(playerid);
public News_Differe(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 1;
	if(gPlayerLogged[playerid] != 1) return 1;

	new k = News_Entry(playerid);
	if(k == -1) return 1;
	if(newsVersionVue[k] >= NEWS_VERSION) return 1;

	newsFois[k]++;
	// Au dernier affichage prevu, on considere que le joueur a eu sa chance :
	// inutile de le relancer a chaque connexion pour l'eternite.
	if(newsFois[k] >= NEWS_MAX_AFFICHAGE) newsVersionVue[k] = NEWS_VERSION;
	News_Save();

	News_ShowMenu(playerid);
	return 1;
}

stock News_OnSpawn(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(gPlayerLogged[playerid] != 1) return 0;

	new k = News_Entry(playerid);
	if(k == -1) return 0;
	if(newsVersionVue[k] >= NEWS_VERSION) return 0;

	// Differe : le tutoriel des nouveaux et l'ecran de connexion ouvrent leurs
	// propres dialogues, le notre les ecraserait s'il partait tout de suite.
	SetTimerEx("News_Differe", NEWS_DELAI_MS, false, "i", playerid);
	return 1;
}

// ============================================================================
// DIALOGUES
// ============================================================================
stock News_OnDialog(playerid, dialogid, response, listitem)
{
	switch(dialogid)
	{
		case DIALOG_NEWS_MENU:
		{
			if(!response) return 1;
			if(listitem >= 0 && listitem < NEWS_NB) News_ShowDetail(playerid, listitem);
			else if(listitem == NEWS_NB)
			{
				News_MarquerLu(playerid);
				msg_Client(playerid, COLOR_WHITE, "{AAAAAA} Nouveautes  Ce message ne reviendra plus. Tape {FFFFB2}/nouveautes{AAAAAA} quand tu veux.");
			}
			return 1;
		}
		case DIALOG_NEWS_DETAIL:
		{
			if(response) News_ShowMenu(playerid);   // bouton Retour
			return 1;
		}
	}
	return 0;
}

stock News_Cmd(playerid)
{
	return News_ShowMenu(playerid);
}

stock News_Init()
{
	News_Load();
	return 1;
}
