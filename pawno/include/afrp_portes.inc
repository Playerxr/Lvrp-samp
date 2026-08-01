/*
	afrp_portes.inc - Relier un mapping au jeu : des portes posees en jeu

	Un mapping tout seul n'est qu'un decor. Le commissariat, la prison, l'ile,
	le garage : on peut tourner autour, on ne peut pas y entrer. Le gamemode
	savait deja poser une entree pour une ENTREPRISE (/entreprise local, qui
	pose la porte sous tes pieds et te vend un local au catalogue), mais rien
	pour tout le reste : un batiment de faction, une cellule, une planque.

	Ce module pose des portes libres. Une porte, c'est deux points : l'entree,
	dehors devant le batiment, et la sortie, dedans. Les deux sont enregistres
	la ou tu te trouves quand tu tapes la commande : aucune coordonnee n'est
	jamais ecrite dans le code.

	Les noms evitent deux systemes deja pris : /entrer gere deja commerces,
	garages, tuning et pay'n'spray ; /porte est deja un clavier a code.

	  /traverser                franchir la porte devant toi (entree ou sortie)
	  /portail                  aide et liste

	Administrateur niveau 3 :
	  /portail creer <nom>      pose l'ENTREE ici, la porte attend sa sortie
	  /portail sortie <id>      pose la SORTIE ici (va d'abord a l'interieur)
	  /portail faction <id> <n> reserve la porte a une faction (0 = tout le monde)
	  /portail proprietaire <id> <joueur>  reserve la porte a UN joueur (0 = vider)
	  /portail liste            toutes les portes et leur etat
	  /portail aller <id>       te teleporte a l'entree
	  /portail suppr <id>

	=== PORTE RESERVEE A UN JOUEUR ===
	Une porte peut aussi etre reservee a une seule personne plutot qu'a une
	faction entiere : utile pour une residence privee. Le nom du joueur est
	enregistre (pas son id, qui change a chaque connexion), comme le fait deja
	le systeme de maisons du gamemode pour son proprietaire. Un proprietaire
	et une faction ne se cumulent pas sur la meme porte : des qu'un joueur est
	assigne, la faction est ignoree.

	=== COMMENT ON S'EN SERT AVEC UN MAPPING ===
	  1. /mappack, on allume la carte et on va voir
	  2. devant la porte du batiment : /portail creer Commissariat
	  3. on se rend dans l'interieur voulu (un interieur GTA, ou une piece
	     construite avec /mapedit dans un monde virtuel a part)
	  4. /portail sortie 1
	  5. c'est fini : un marqueur apparait aux deux bouts, /traverser traverse

	=== L'INTERIEUR ET LE MONDE VIRTUEL SONT RETENUS ===
	Deux batiments differents peuvent utiliser le meme interieur GTA sans se
	melanger : chaque porte retient aussi le monde virtuel, et deux joueurs dans
	deux mondes differents ne se voient pas.

	=== PORTE INCOMPLETE ===
	Une porte dont la sortie n'a pas encore ete posee n'est pas franchissable et
	le dit. Elle reste visible dans /portail liste pour qu'on n'oublie pas de la
	terminer, plutot que de disparaitre silencieusement.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit    -> Portes_Init()
	- OnPlayerConnect   -> Portes_OnConnect(playerid)
	- OnDialogResponse  -> if(Portes_OnDialog(playerid, dialogid, response, listitem)) return 1;
	- /traverser        -> Portes_Entrer(playerid)
	- /portail          -> Portes_Cmd(playerid, cmdtext)
*/

#if defined AFRP_PORTES_INCLUDED
	#endinput
#endif
#define AFRP_PORTES_INCLUDED

#define PORTE_MAX          60
#define PORTE_FILE         "portes.cfg"
#define PORTE_ADMIN_LEVEL  3
#define PORTE_DIST         2.0      // distance pour franchir
#define DIALOG_PORTE_LISTE 9650

new porteNom[PORTE_MAX][32];
new Float:porteEnX[PORTE_MAX], Float:porteEnY[PORTE_MAX], Float:porteEnZ[PORTE_MAX];
new porteEnInt[PORTE_MAX], porteEnVw[PORTE_MAX];
new Float:porteSoX[PORTE_MAX], Float:porteSoY[PORTE_MAX], Float:porteSoZ[PORTE_MAX];
new porteSoInt[PORTE_MAX], porteSoVw[PORTE_MAX];
new porteFaction[PORTE_MAX];        // 0 = ouverte a tous, sinon pMember requis
new porteProprio[PORTE_MAX][MAX_PLAYER_NAME];  // "" = pas de proprietaire unique
new portePris[PORTE_MAX];           // 1 = porte existante
new porteComplete[PORTE_MAX];       // 1 = la sortie est posee

new Text3D:porteLblEn[PORTE_MAX];
new Text3D:porteLblSo[PORTE_MAX];

// ============================================================================
// OUTILS
// ============================================================================
stock Portes_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Portes_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= PORTE_ADMIN_LEVEL);
}

stock Portes_Nombre()
{
	new n = 0;
	for(new k = 0; k < PORTE_MAX; k++) if(portePris[k] == 1) n++;
	return n;
}

stock Portes_LibreId()
{
	for(new k = 0; k < PORTE_MAX; k++) if(portePris[k] == 0) return k;
	return -1;
}

// ============================================================================
// AFFICHAGE
// ============================================================================
stock Portes_DetruireVisuels(k)
{
	if(k < 0 || k >= PORTE_MAX) return 0;
	if(IsValidDynamic3DTextLabel(porteLblEn[k])) DestroyDynamic3DTextLabel(porteLblEn[k]);
	if(IsValidDynamic3DTextLabel(porteLblSo[k])) DestroyDynamic3DTextLabel(porteLblSo[k]);
	porteLblEn[k] = Text3D:INVALID_3DTEXT_ID;
	porteLblSo[k] = Text3D:INVALID_3DTEXT_ID;
	return 1;
}

stock Portes_CreerVisuels(k)
{
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1) return 0;
	Portes_DetruireVisuels(k);

	new txt[128];
	if(porteComplete[k] != 1)
		format(txt, sizeof(txt), "{FFFF00}%s\n{FF8888}porte inachevee : /portail sortie %d", porteNom[k], k + 1);
	else if(strlen(porteProprio[k]) > 0)
		format(txt, sizeof(txt), "{FFFF00}%s\n{FF8888}Propriete privee de %s\n{FFFFFF}/traverser", porteNom[k], porteProprio[k]);
	else
		format(txt, sizeof(txt), "{FFFF00}%s\n{FFFFFF}/traverser", porteNom[k]);

	// Le label est visible dans le monde virtuel et l'interieur enregistres :
	// une porte posee dans un appartement ne doit pas flotter dans la rue.
	porteLblEn[k] = CreateDynamic3DTextLabel(txt, 0xFFFF00FF,
		porteEnX[k], porteEnY[k], porteEnZ[k] + 0.4, 8.0,
		INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, porteEnVw[k], porteEnInt[k], -1, 30.0);

	if(porteComplete[k] == 1)
	{
		format(txt, sizeof(txt), "{FFFF00}Sortie - %s\n{FFFFFF}/traverser", porteNom[k]);
		porteLblSo[k] = CreateDynamic3DTextLabel(txt, 0xFFFF00FF,
			porteSoX[k], porteSoY[k], porteSoZ[k] + 0.4, 8.0,
			INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0, porteSoVw[k], porteSoInt[k], -1, 30.0);
	}
	return 1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Portes_Save()
{
	if(fexist(PORTE_FILE)) fremove(PORTE_FILE);
	new File:f = fopen(PORTE_FILE, io_write);
	if(!f) return 0;
	new line[256];
	for(new k = 0; k < PORTE_MAX; k++)
	{
		if(portePris[k] != 1) continue;
		format(line, sizeof(line), "%s|%.4f|%.4f|%.4f|%d|%d|%.4f|%.4f|%.4f|%d|%d|%d|%d|%s\n",
			porteNom[k], porteEnX[k], porteEnY[k], porteEnZ[k], porteEnInt[k], porteEnVw[k],
			porteSoX[k], porteSoY[k], porteSoZ[k], porteSoInt[k], porteSoVw[k],
			porteFaction[k], porteComplete[k], porteProprio[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Portes_Champ(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Portes_Load()
{
	for(new k = 0; k < PORTE_MAX; k++)
	{
		portePris[k] = 0;
		porteLblEn[k] = Text3D:INVALID_3DTEXT_ID;
		porteLblSo[k] = Text3D:INVALID_3DTEXT_ID;
	}
	if(!fexist(PORTE_FILE)) return 0;
	new File:f = fopen(PORTE_FILE, io_read);
	if(!f) return 0;

	new line[256], buf[40], n = 0;
	while(fread(f, line) && n < PORTE_MAX)
	{
		if(strlen(line) < 10) continue;
		new i = 0;
		Portes_Champ(line, i, porteNom[n], 32);
		Portes_Champ(line, i, buf, sizeof(buf)); porteEnX[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteEnY[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteEnZ[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteEnInt[n]    = strval(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteEnVw[n]     = strval(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteSoX[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteSoY[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteSoZ[n]      = floatstr(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteSoInt[n]    = strval(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteSoVw[n]     = strval(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteFaction[n]  = strval(buf);
		Portes_Champ(line, i, buf, sizeof(buf)); porteComplete[n] = strval(buf);
		// Champ ajoute apres coup : un fichier plus ancien s'arrete simplement
		// avant, et Portes_Champ renvoie une chaine vide (aucun proprietaire).
		Portes_Champ(line, i, porteProprio[n], MAX_PLAYER_NAME);
		if(!strlen(porteNom[n])) continue;
		portePris[n] = 1;
		Portes_CreerVisuels(n);
		n++;
	}
	fclose(f);
	printf("[Portes] %d porte(s) chargee(s).", n);
	return n;
}

// ============================================================================
// FRANCHIR
// ============================================================================
// Porte dont l'ENTREE est a portee, -1 sinon.
stock Portes_EntreeProche(playerid)
{
	new vw = GetPlayerVirtualWorld(playerid), inter = GetPlayerInterior(playerid);
	for(new k = 0; k < PORTE_MAX; k++)
	{
		if(portePris[k] != 1) continue;
		if(porteEnVw[k] != vw || porteEnInt[k] != inter) continue;
		if(!IsPlayerInRangeOfPoint(playerid, PORTE_DIST, porteEnX[k], porteEnY[k], porteEnZ[k])) continue;
		return k;
	}
	return -1;
}

// Porte dont la SORTIE est a portee, -1 sinon.
stock Portes_SortieProche(playerid)
{
	new vw = GetPlayerVirtualWorld(playerid), inter = GetPlayerInterior(playerid);
	for(new k = 0; k < PORTE_MAX; k++)
	{
		if(portePris[k] != 1 || porteComplete[k] != 1) continue;
		if(porteSoVw[k] != vw || porteSoInt[k] != inter) continue;
		if(!IsPlayerInRangeOfPoint(playerid, PORTE_DIST, porteSoX[k], porteSoY[k], porteSoZ[k])) continue;
		return k;
	}
	return -1;
}

stock Portes_Autorise(playerid, k)
{
	if(AdminDuty[playerid] == 1) return 1;

	// Un proprietaire unique prime sur la faction : une residence privee ne
	// doit pas s'ouvrir a toute une faction sous pretexte que le proprietaire
	// en fait partie.
	if(strlen(porteProprio[k]) > 0)
		return (strcmp(PlayerInfo[playerid][pName], porteProprio[k], true) == 0);

	if(porteFaction[k] == 0) return 1;
	if(PlayerInfo[playerid][pMember] == porteFaction[k]) return 1;
	return 0;
}

stock Portes_Entrer(playerid)
{
	if(IsPlayerInAnyVehicle(playerid))
		return Portes_Msg(playerid, "{FF8888}Descends du vehicule d'abord.");

	// La sortie est testee en premier : quand une porte est posee sur elle-meme
	// (une piece dont l'entree et la sortie sont proches), on veut ressortir.
	new k = Portes_SortieProche(playerid);
	if(k != -1)
	{
		SafeTeleportPlayer(playerid, porteEnX[k], porteEnY[k], porteEnZ[k] + 0.4, porteEnInt[k], porteEnVw[k]);
		new s[144];
		format(s, sizeof(s), "{FFFF00}Tu sors de %s.", porteNom[k]);
		return Portes_Msg(playerid, s);
	}

	k = Portes_EntreeProche(playerid);
	if(k == -1) return Portes_Msg(playerid, "{FF8888}Aucune porte a moins de 2 metres.");

	if(porteComplete[k] != 1)
	{
		new s[190];
		format(s, sizeof(s), "{FF8888}%s n'a pas encore d'interieur. Un admin doit poser la sortie : /portail sortie %d", porteNom[k], k + 1);
		return Portes_Msg(playerid, s);
	}
	if(!Portes_Autorise(playerid, k))
	{
		if(strlen(porteProprio[k]) > 0)
		{
			new s[144];
			format(s, sizeof(s), "{FF4444}Propriete privee de %s. Tu n'es pas autorise a entrer.", porteProprio[k]);
			return Portes_Msg(playerid, s);
		}
		return Portes_Msg(playerid, "{FF4444}Cette porte est reservee a une faction.");
	}

	SafeTeleportPlayer(playerid, porteSoX[k], porteSoY[k], porteSoZ[k] + 0.4, porteSoInt[k], porteSoVw[k]);
	new s[144];
	format(s, sizeof(s), "{00FF00}Tu entres dans %s. ({FFFFFF}/traverser{00FF00} pour ressortir)", porteNom[k]);
	return Portes_Msg(playerid, s);
}

// ============================================================================
// ADMINISTRATION
// ============================================================================
stock Portes_Creer(playerid, const nom[])
{
	new k = Portes_LibreId();
	if(k == -1) return Portes_Msg(playerid, "{FF4444}Limite de portes atteinte.");

	format(porteNom[k], 32, "%s", nom);
	GetPlayerPos(playerid, porteEnX[k], porteEnY[k], porteEnZ[k]);
	porteEnInt[k]    = GetPlayerInterior(playerid);
	porteEnVw[k]     = GetPlayerVirtualWorld(playerid);
	porteSoX[k]      = 0.0; porteSoY[k] = 0.0; porteSoZ[k] = 0.0;
	porteSoInt[k]    = 0;   porteSoVw[k] = 0;
	porteFaction[k]  = 0;
	porteProprio[k][0] = 0;
	porteComplete[k] = 0;
	portePris[k]     = 1;
	Portes_CreerVisuels(k);
	Portes_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}Porte %d creee : %s. L'entree est ici.", k + 1, porteNom[k]);
	Portes_Msg(playerid, s);
	format(s, sizeof(s), "{AAAAAA}Va maintenant dans l'interieur voulu et tape {FFFF00}/portail sortie %d{AAAAAA}.", k + 1);
	return Portes_Msg(playerid, s);
}

stock Portes_Sortie(playerid, num)
{
	new k = num - 1;
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1)
		return Portes_Msg(playerid, "{FF4444}Numero de porte invalide. /portail liste");

	GetPlayerPos(playerid, porteSoX[k], porteSoY[k], porteSoZ[k]);
	porteSoInt[k]    = GetPlayerInterior(playerid);
	porteSoVw[k]     = GetPlayerVirtualWorld(playerid);
	porteComplete[k] = 1;
	Portes_CreerVisuels(k);
	Portes_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}Sortie de %s posee ici (interieur %d, monde %d). La porte est complete.", porteNom[k], porteSoInt[k], porteSoVw[k]);
	Portes_Msg(playerid, s);
	return Portes_Msg(playerid, "{AAAAAA}Teste-la : /traverser ici pour ressortir, puis /traverser devant l'entree.");
}

stock Portes_Faction(playerid, num, faction)
{
	new k = num - 1;
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1)
		return Portes_Msg(playerid, "{FF4444}Numero de porte invalide. /portail liste");

	porteFaction[k] = faction;
	Portes_CreerVisuels(k);
	Portes_Save();

	new s[190];
	if(faction == 0) format(s, sizeof(s), "{00FF00}%s est maintenant ouverte a tout le monde.", porteNom[k]);
	else             format(s, sizeof(s), "{00FF00}%s est reservee a la faction %d. Les admins en service passent quand meme.", porteNom[k], faction);
	return Portes_Msg(playerid, s);
}

stock Portes_Proprietaire(playerid, num, cible)
{
	new k = num - 1;
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1)
		return Portes_Msg(playerid, "{FF4444}Numero de porte invalide. /portail liste");

	if(cible == 0)
	{
		porteProprio[k][0] = 0;
		Portes_CreerVisuels(k);
		Portes_Save();
		new s0[144];
		format(s0, sizeof(s0), "{FFFF00}%s n'a plus de proprietaire unique.", porteNom[k]);
		return Portes_Msg(playerid, s0);
	}

	if(!IsPlayerConnected(cible) || IsPlayerNPC(cible))
		return Portes_Msg(playerid, "{FF4444}Ce joueur n'est pas connecte.");

	format(porteProprio[k], MAX_PLAYER_NAME, "%s", PlayerInfo[cible][pName]);
	Portes_CreerVisuels(k);
	Portes_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}%s appartient maintenant a %s uniquement.", porteNom[k], porteProprio[k]);
	return Portes_Msg(playerid, s);
}

stock Portes_Supprimer(playerid, num)
{
	new k = num - 1;
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1)
		return Portes_Msg(playerid, "{FF4444}Numero de porte invalide. /portail liste");

	new s[144];
	format(s, sizeof(s), "{FFFF00}Porte %d (%s) supprimee.", num, porteNom[k]);
	Portes_DetruireVisuels(k);
	portePris[k] = 0;
	porteComplete[k] = 0;
	Portes_Save();
	return Portes_Msg(playerid, s);
}

stock Portes_Aller(playerid, num)
{
	new k = num - 1;
	if(k < 0 || k >= PORTE_MAX || portePris[k] != 1)
		return Portes_Msg(playerid, "{FF4444}Numero de porte invalide. /portail liste");

	SafeTeleportPlayer(playerid, porteEnX[k], porteEnY[k], porteEnZ[k] + 0.4, porteEnInt[k], porteEnVw[k]);
	new s[144];
	format(s, sizeof(s), "{00FF00}Devant l'entree de %s.", porteNom[k]);
	return Portes_Msg(playerid, s);
}

stock Portes_Liste(playerid)
{
	new body[1400], line[144], n = 0;
	body[0] = 0;
	for(new k = 0; k < PORTE_MAX; k++)
	{
		if(portePris[k] != 1) continue;
		if(n >= 40) break;
		new acces[24];
		if(strlen(porteProprio[k]) > 0) format(acces, sizeof(acces), "%s", porteProprio[k]);
		else if(porteFaction[k] == 0)   format(acces, sizeof(acces), "tous");
		else                             format(acces, sizeof(acces), "faction");
		format(line, sizeof(line), "%d\t%s\t%s\t%s\n", k + 1, porteNom[k],
			(porteComplete[k] == 1) ? ("{00FF00}complete") : ("{FF8888}sans sortie"),
			acces);
		strcat(body, line);
		n++;
	}
	if(n == 0) return Portes_Msg(playerid, "{AAAAAA}Aucune porte posee. /portail creer <nom> devant un batiment.");
	return ShowPlayerDialog(playerid, DIALOG_PORTE_LISTE, DIALOG_STYLE_TABLIST, "Portes - choisis pour t'y rendre", body, "Aller", "Fermer");
}

stock Portes_OnDialog(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_PORTE_LISTE) return 0;
	if(!response) return 1;

	// La liste saute les emplacements vides : on la reparcourt dans le meme
	// ordre plutot que de supposer que la ligne N est la porte N.
	new n = 0;
	for(new k = 0; k < PORTE_MAX; k++)
	{
		if(portePris[k] != 1) continue;
		if(n == listitem) return Portes_Aller(playerid, k + 1);
		n++;
	}
	return 1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Portes_Aide(playerid)
{
	Portes_Msg(playerid, "{FFFF00}--- Portes ---");
	Portes_Msg(playerid, "{FFFFFF}/traverser {AAAAAA}: franchir la porte devant toi");
	if(!Portes_IsAdmin(playerid)) return Portes_Liste(playerid);

	Portes_Msg(playerid, "{FFFFFF}/portail creer <nom> {AAAAAA}: pose l'entree ici");
	Portes_Msg(playerid, "{FFFFFF}/portail sortie <id> {AAAAAA}: pose la sortie ici (va d'abord a l'interieur)");
	Portes_Msg(playerid, "{FFFFFF}/portail faction <id> <numero> {AAAAAA}: reserve la porte (0 = tous)");
	Portes_Msg(playerid, "{FFFFFF}/portail proprietaire <id> <joueur> {AAAAAA}: reserve a UN joueur (0 = vider)");
	Portes_Msg(playerid, "{FFFFFF}/portail liste | aller <id> | suppr <id>");
	Portes_Msg(playerid, "{AAAAAA}Pour relier un mapping : allume la carte, pose l'entree devant la porte, va dans l'interieur, pose la sortie.");
	return Portes_Liste(playerid);
}

stock Portes_Tok(const src[], &pos2, dest[], destSize)
{
	new p = 0;
	while(src[pos2] == ' ') pos2++;
	while(src[pos2] && src[pos2] != ' ' && p < destSize - 1) dest[p++] = src[pos2++];
	dest[p] = 0;
	return p;
}

stock Portes_Cmd(playerid, const cmdline[])
{
	new pos2 = 0, sub[24], arg[40];
	Portes_Tok(cmdline, pos2, sub, sizeof(sub));            // consomme "/porte"
	if(Portes_Tok(cmdline, pos2, sub, sizeof(sub)) == 0) return Portes_Aide(playerid);

	if(strcmp(sub, "liste", true) == 0) return Portes_Liste(playerid);
	if(strcmp(sub, "aide",  true) == 0) return Portes_Aide(playerid);

	if(!Portes_IsAdmin(playerid))
	{
		new s[160];
		format(s, sizeof(s), "{FF4444}Reserve aux admins niveau %d (tu es niveau %d).", PORTE_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return Portes_Msg(playerid, s);
	}

	if(strcmp(sub, "creer", true) == 0)
	{
		// Le nom peut contenir des espaces : on prend tout ce qui reste.
		while(cmdline[pos2] == ' ') pos2++;
		if(!strlen(cmdline[pos2]))
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail creer <nom>");
		new nom[32];
		format(nom, sizeof(nom), "%s", cmdline[pos2]);
		return Portes_Creer(playerid, nom);
	}
	if(strcmp(sub, "sortie", true) == 0)
	{
		if(Portes_Tok(cmdline, pos2, arg, sizeof(arg)) == 0)
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail sortie <id>");
		return Portes_Sortie(playerid, strval(arg));
	}
	if(strcmp(sub, "faction", true) == 0)
	{
		new arg2[24];
		if(Portes_Tok(cmdline, pos2, arg, sizeof(arg)) == 0 || Portes_Tok(cmdline, pos2, arg2, sizeof(arg2)) == 0)
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail faction <id> <numero de faction, 0 = tous>");
		return Portes_Faction(playerid, strval(arg), strval(arg2));
	}
	if(strcmp(sub, "proprietaire", true) == 0)
	{
		new arg2[24];
		if(Portes_Tok(cmdline, pos2, arg, sizeof(arg)) == 0 || Portes_Tok(cmdline, pos2, arg2, sizeof(arg2)) == 0)
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail proprietaire <id> <id du joueur, 0 = vider>");
		return Portes_Proprietaire(playerid, strval(arg), strval(arg2));
	}
	if(strcmp(sub, "aller", true) == 0)
	{
		if(Portes_Tok(cmdline, pos2, arg, sizeof(arg)) == 0)
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail aller <id>");
		return Portes_Aller(playerid, strval(arg));
	}
	if(strcmp(sub, "suppr", true) == 0)
	{
		if(Portes_Tok(cmdline, pos2, arg, sizeof(arg)) == 0)
			return Portes_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/portail suppr <id>");
		return Portes_Supprimer(playerid, strval(arg));
	}
	return Portes_Aide(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Portes_OnConnect(playerid)
{
	#pragma unused playerid
	return 1;
}

stock Portes_Init()
{
	Portes_Load();
	return 1;
}
