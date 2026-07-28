/*
	afrp_saison.inc - Passe saisonnier (progression sur 30 jours)

	Inspire de Fortnite et Rocket League. Une saison dure 30 jours. Le joueur
	gagne des points en jouant normalement et franchit des paliers ; chaque
	palier paye. A la fin, tout repart a zero et une nouvelle saison commence.

	  /saison   (ou /passe) -> progression, palier, recompenses, jours restants

	=== D'OU VIENNENT LES POINTS ===
	  +1   par minute reellement jouee (meme controle anti-AFK que le top du jour)
	  +50  par objectif quotidien accompli
	  +25  par paie horaire de job
	  +80  convoi livre (le convoyeur) ou braque (chaque braqueur)
	  +200 / +120 / +60  podium du top du jour

	Rien n'a ete invente pour l'occasion : le passe recompense ce que le joueur
	fait deja. C'est ce qui le rend lisible, et ce qui evite de creer une
	activite artificielle a farmer.

	=== PALIERS ===
	20 paliers. Seuil du palier N = 100*N + 20*N*N points cumules, soit 120
	points pour le premier et 10.000 pour le dernier. Un joueur regulier
	(environ 2h par jour) tourne autour de 300 points quotidiens : le palier 20
	demande de la constance sur le mois, pas du farm.

	Recompense du palier N = 15.000$ * N, doublee aux paliers 5, 10, 15 et 20.
	Un joueur qui va au bout touche environ 3,9 millions sur le mois.

	=== PAS DE PANNEAU PERMANENT, VOLONTAIREMENT ===
	L'ecran porte deja le bloc vie/faim, le top du jour, les objectifs et le
	ping. Un panneau de plus le rendrait illisible. Le passage de palier passe
	par le bandeau d'annonce, qui s'affiche puis s'efface, et /saison donne le
	detail a la demande.

	=== RECOMPENSES VERSEES IMMEDIATEMENT ===
	Les points ne s'obtiennent qu'en etant connecte : un joueur est donc
	toujours en ligne au moment ou il franchit un palier. Pas besoin d'un
	systeme de gains en attente comme pour le top du jour, dont le podium
	tombe a minuit alors que le gagnant dort peut-etre.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Saison_Init()
	- OnPlayerConnect    -> Saison_OnConnect(playerid)
	- OnPlayerDisconnect -> Saison_OnDisconnect(playerid)
	- OnDialogResponse   -> Saison_OnDialog(playerid, dialogid, response, listitem)
	- /saison            -> Saison_Cmd(playerid)
*/

#if defined AFRP_SAISON_INCLUDED
	#endinput
#endif
#define AFRP_SAISON_INCLUDED

#define SAI_MAX_JOUEUR    500        // joueurs classes dans la saison
#define SAI_FILE          "saison.cfg"
#define SAI_DUREE_JOURS   30
#define SAI_PALIER_MAX    20
#define SAI_GAIN_PALIER   15000      // multiplie par le numero du palier
#define SAI_TOP_AFFICHE   15

// --- valeur des actions ---
#define SAI_XP_MINUTE     1
#define SAI_XP_OBJECTIF   50
#define SAI_XP_PAIE       25
#define SAI_XP_CONVOI     80
#define SAI_XP_TOP1       200
#define SAI_XP_TOP2       120
#define SAI_XP_TOP3       60

#define DIALOG_SAI_MENU   9628
#define DIALOG_SAI_PAL    9629

new saiNumero = 1;
new saiStartTs;

new saiSqlid[SAI_MAX_JOUEUR];
new saiNom[SAI_MAX_JOUEUR][MAX_PLAYER_NAME];
new saiXp[SAI_MAX_JOUEUR];
new saiPalier[SAI_MAX_JOUEUR];        // dernier palier deja paye
new saiPris[SAI_MAX_JOUEUR];

new saiIdx[MAX_PLAYERS];              // index du joueur, -1 si aucun

// ============================================================================
// PALIERS
// ============================================================================
// Seuil cumule du palier n (1..SAI_PALIER_MAX).
stock Saison_Seuil(n)
{
	if(n < 1) return 0;
	if(n > SAI_PALIER_MAX) n = SAI_PALIER_MAX;
	return 100 * n + 20 * n * n;
}

stock Saison_EstMajeur(n)
{
	return (n == 5 || n == 10 || n == 15 || n == 20);
}

stock Saison_Gain(n)
{
	new g = SAI_GAIN_PALIER * n;
	if(Saison_EstMajeur(n)) g *= 2;
	return g;
}

// Palier atteint avec ce total de points.
stock Saison_PalierPour(xp)
{
	new p = 0;
	for(new n = 1; n <= SAI_PALIER_MAX; n++)
		if(xp >= Saison_Seuil(n)) p = n;
	return p;
}

stock Saison_JoursRestants()
{
	new fin = saiStartTs + SAI_DUREE_JOURS * 86400;
	if(gettime() >= fin) return 0;
	return (fin - gettime()) / 86400 + 1;
}

// ============================================================================
// REGISTRE
// ============================================================================
stock Saison_Find(sqlid)
{
	if(sqlid <= 0) return -1;
	for(new k = 0; k < SAI_MAX_JOUEUR; k++)
		if(saiPris[k] == 1 && saiSqlid[k] == sqlid) return k;
	return -1;
}

stock Saison_Entry(playerid)
{
	if(saiIdx[playerid] != -1) return saiIdx[playerid];

	new my = PlayerInfo[playerid][pSQLID];
	if(my <= 0) return -1;                       // pas encore identifie

	new k = Saison_Find(my);
	if(k == -1)
	{
		for(new n = 0; n < SAI_MAX_JOUEUR; n++)
			if(saiPris[n] == 0) { k = n; break; }
		if(k == -1) return -1;
		saiPris[k]   = 1;
		saiSqlid[k]  = my;
		saiXp[k]     = 0;
		saiPalier[k] = 0;
	}
	format(saiNom[k], MAX_PLAYER_NAME, "%s", PlayerInfo[playerid][pName]);
	saiIdx[playerid] = k;
	return k;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Saison_Field(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Saison_Save()
{
	if(fexist(SAI_FILE)) fremove(SAI_FILE);
	new File:f = fopen(SAI_FILE, io_write);
	if(!f) return 0;
	new line[128];
	format(line, sizeof(line), "S|%d|%d\n", saiNumero, saiStartTs);
	fwrite(f, line);
	for(new k = 0; k < SAI_MAX_JOUEUR; k++)
	{
		if(saiPris[k] != 1 || saiXp[k] <= 0) continue;
		format(line, sizeof(line), "P|%d|%d|%d|%s\n", saiSqlid[k], saiXp[k], saiPalier[k], saiNom[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Saison_NouvelleSaison()
{
	saiNumero++;
	saiStartTs = gettime();
	for(new k = 0; k < SAI_MAX_JOUEUR; k++) { saiPris[k] = 0; saiXp[k] = 0; saiPalier[k] = 0; }
	for(new i = 0; i < MAX_PLAYERS; i++) saiIdx[i] = -1;
	Saison_Save();

	new s[144];
	format(s, sizeof(s), "[SAISON] La saison %d commence ! Tout repart a zero, %d jours pour aller au bout. /saison", saiNumero, SAI_DUREE_JOURS);
	SendClientMessageToAll(0xFFD700FF, s);
	format(s, sizeof(s), "Saison %d  -  %d jours  -  /saison", saiNumero, SAI_DUREE_JOURS);
	Annonce_All("~y~NOUVELLE SAISON", s, 0xFFD700FF, 12000);
	printf("[Saison] Nouvelle saison : %d", saiNumero);
	return 1;
}

stock Saison_Load()
{
	for(new k = 0; k < SAI_MAX_JOUEUR; k++) saiPris[k] = 0;
	for(new i = 0; i < MAX_PLAYERS; i++) saiIdx[i] = -1;
	saiNumero  = 1;
	saiStartTs = gettime();

	if(!fexist(SAI_FILE)) { Saison_Save(); return 0; }
	new File:f = fopen(SAI_FILE, io_read);
	if(!f) return 0;

	new line[160], buf[48], n = 0;
	while(fread(f, line))
	{
		if(strlen(line) < 3) continue;
		new k = 0;
		Saison_Field(line, k, buf, sizeof(buf));
		if(strcmp(buf, "S", true) == 0)
		{
			Saison_Field(line, k, buf, sizeof(buf)); saiNumero  = strval(buf);
			Saison_Field(line, k, buf, sizeof(buf)); saiStartTs = strval(buf);
			if(saiNumero < 1) saiNumero = 1;
			if(saiStartTs <= 0) saiStartTs = gettime();
		}
		else if(strcmp(buf, "P", true) == 0)
		{
			if(n >= SAI_MAX_JOUEUR) continue;
			Saison_Field(line, k, buf, sizeof(buf)); saiSqlid[n]  = strval(buf);
			Saison_Field(line, k, buf, sizeof(buf)); saiXp[n]     = strval(buf);
			Saison_Field(line, k, buf, sizeof(buf)); saiPalier[n] = strval(buf);
			Saison_Field(line, k, saiNom[n], MAX_PLAYER_NAME);
			if(saiSqlid[n] <= 0) continue;
			saiPris[n] = 1;
			n++;
		}
	}
	fclose(f);
	printf("[Saison] Saison %d, %d joueur(s) classe(s)", saiNumero, n);

	// Saison terminee pendant que le serveur etait eteint : on enchaine.
	if(gettime() >= saiStartTs + SAI_DUREE_JOURS * 86400) Saison_NouvelleSaison();
	return n;
}

// ============================================================================
// GAIN DE POINTS
// ============================================================================
stock Saison_AddXp(playerid, montant)
{
	if(montant <= 0) return 0;
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;

	new k = Saison_Entry(playerid);
	if(k == -1) return 0;

	saiXp[k] += montant;

	// Un joueur peut franchir plusieurs paliers d'un coup (gros gain de points) :
	// on paye chacun d'eux, sans en sauter aucun.
	new atteint = Saison_PalierPour(saiXp[k]);
	while(saiPalier[k] < atteint)
	{
		saiPalier[k]++;
		new n = saiPalier[k];
		new gain = Saison_Gain(n);
		SafeGivePlayerMoney(playerid, gain, "Palier de saison");

		new s[200];
		format(s, sizeof(s), "{FFD700} Saison  Palier {FFFFB2}%d{FFFFFF} atteint : {FFFFB2}+%d${FFFFFF}%s", n, gain, (Saison_EstMajeur(n)) ? (" (palier majeur, gain double)") : (""));
		msg_Client(playerid, COLOR_WHITE, s);
		PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);

		new titre[48], texte[144];
		format(titre, sizeof(titre), "~y~PALIER %d", n);
		format(texte, sizeof(texte), "%s atteint le palier %d de la saison %d", PlayerInfo[playerid][pName], n, saiNumero);
		Annonce_Show(playerid, titre, texte, 0xFFD700FF, 6000);

		// Le dernier palier merite que tout le serveur le sache.
		if(n >= SAI_PALIER_MAX)
		{
			new all[190];
			format(all, sizeof(all), "[SAISON] %s termine la saison %d au palier maximum !", PlayerInfo[playerid][pName], saiNumero);
			SendClientMessageToAll(0xFFD700FF, all);
		}
	}
	return 1;
}

// Appele par le tick du top du jour : meme controle anti-AFK, pas de doublon.
stock Saison_MinuteActive(playerid)
{
	return Saison_AddXp(playerid, SAI_XP_MINUTE);
}

// ============================================================================
// TICK (1 minute) : uniquement le changement de saison
// ============================================================================
forward Saison_Tick();
public Saison_Tick()
{
	if(gettime() >= saiStartTs + SAI_DUREE_JOURS * 86400) Saison_NouvelleSaison();
	else Saison_Save();
	return 1;
}

// ============================================================================
// MENUS
// ============================================================================
stock Saison_ShowMenu(playerid)
{
	new k = Saison_Entry(playerid);
	new xp = (k != -1) ? saiXp[k] : 0;
	new pal = (k != -1) ? saiPalier[k] : 0;

	new body[900], str[220];
	format(body, sizeof(body), "{FFD700}Saison %d{FFFFFF} - encore {FFFF00}%d jour(s){FFFFFF}\n\n", saiNumero, Saison_JoursRestants());

	format(str, sizeof(str), "{FFFFFF}Palier : {FFFF00}%d{FFFFFF} / %d\n{FFFFFF}Points : {FFFF00}%d{FFFFFF}\n", pal, SAI_PALIER_MAX, xp);
	strcat(body, str);

	if(pal < SAI_PALIER_MAX)
	{
		new suivant = Saison_Seuil(pal + 1);
		format(str, sizeof(str), "{AAAAAA}Palier suivant a %d points (encore %d)\n", suivant, suivant - xp);
		strcat(body, str);
	}
	else strcat(body, "{00FF00}Palier maximum atteint. Bravo.\n");

	strcat(body, "\n{AAAAAA}Comment gagner des points :\n");
	format(str, sizeof(str), "{AAAAAA}  +%d par minute reellement jouee\n{AAAAAA}  +%d par objectif quotidien\n{AAAAAA}  +%d par paie horaire de job\n{AAAAAA}  +%d convoi livre ou braque\n{AAAAAA}  +%d / +%d / +%d podium du top du jour\n",
		SAI_XP_MINUTE, SAI_XP_OBJECTIF, SAI_XP_PAIE, SAI_XP_CONVOI, SAI_XP_TOP1, SAI_XP_TOP2, SAI_XP_TOP3);
	strcat(body, str);
	strcat(body, "\n{AAAAAA}Rester AFK ne rapporte rien.");

	return ShowPlayerDialog(playerid, DIALOG_SAI_MENU, DIALOG_STYLE_MSGBOX, "Passe saisonnier", body, "Les paliers", "Fermer");
}

stock Saison_ShowPaliers(playerid)
{
	new k = Saison_Entry(playerid);
	new pal = (k != -1) ? saiPalier[k] : 0;

	new body[1600], str[176];
	body[0] = 0;
	for(new n = 1; n <= SAI_PALIER_MAX; n++)
	{
		new marque[16];
		if(n <= pal) format(marque, sizeof(marque), "{00FF00}[ok]");
		else format(marque, sizeof(marque), "{AAAAAA}[  ]");

		if(Saison_EstMajeur(n))
			format(str, sizeof(str), "%s {FFD700}Palier %d{FFFFFF} - %d pts - {FFFF00}%d${FFFFFF} (majeur)\n", marque, n, Saison_Seuil(n), Saison_Gain(n));
		else
			format(str, sizeof(str), "%s {FFFFFF}Palier %d - %d pts - {FFFF00}%d${FFFFFF}\n", marque, n, Saison_Seuil(n), Saison_Gain(n));
		strcat(body, str);
	}
	return ShowPlayerDialog(playerid, DIALOG_SAI_PAL, DIALOG_STYLE_MSGBOX, "Paliers de la saison", body, "Retour", "Fermer");
}

stock Saison_OnDialog(playerid, dialogid, response, listitem)
{
	#pragma unused listitem
	switch(dialogid)
	{
		case DIALOG_SAI_MENU:
		{
			if(response) Saison_ShowPaliers(playerid);
			return 1;
		}
		case DIALOG_SAI_PAL:
		{
			if(response) Saison_ShowMenu(playerid);
			return 1;
		}
	}
	return 0;
}

stock Saison_Cmd(playerid)
{
	return Saison_ShowMenu(playerid);
}

// ============================================================================
// HOOKS
// ============================================================================
// Indispensable : sans cette remise a -1, un nouveau joueur reprenant un slot
// libere heriterait de l'index du precedent occupant, et ses points de saison
// seraient credites sur le compte de quelqu'un d'autre.
stock Saison_OnConnect(playerid)
{
	saiIdx[playerid] = -1;
	return 1;
}

stock Saison_OnDisconnect(playerid)
{
	saiIdx[playerid] = -1;
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
stock Saison_Init()
{
	Saison_Load();
	SetTimer("Saison_Tick", 60000, true);
	return 1;
}
