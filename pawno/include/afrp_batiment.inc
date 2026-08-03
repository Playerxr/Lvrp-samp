/*
	afrp_batiment.inc - /a batiment <modelid> [rayon] : retirer un batiment GTA a la main

	POURQUOI CE MODULE EXISTE : afrp_mapmove.inc (/a deplacer) permet de reposer
	une map communautaire n'importe ou, mais si le nouvel endroit a deja un
	batiment GTA d'origine, ce batiment reste solide et visible par-dessus (ou
	autour) le nouveau decor - la map deplacee peut sembler "invisible" alors
	qu'elle est bien creee, juste noyee dans le decor GTA non retire. Comme
	pour /a deplacer, personne ici ne voit le jeu tourner : seule la
	proprietaire peut constater quel batiment gene et a quel endroit. Ce
	module lui donne le geste pour le retirer elle-meme, pour n'importe quelle
	map future, sans repasser par un aller-retour de code a chaque fois.

	  /a batiment <modelid> [rayon]   retire ce batiment autour de sa position
	                                  actuelle (rayon par defaut 30.0)
	  /a batiment liste               liste les retraits enregistres
	  /a batiment supprimer <n>       annule un retrait de la liste

	COMMENT TROUVER LE modelid : SA-MP ne donne aucun moyen serveur de savoir
	quel batiment se trouve sous les pieds de quelqu'un - il faut un outil
	cote client (editeur de map GTA SA, visionneuse d'objets) ou une base de
	donnees de batiments par coordonnees pour le lire.

	LIMITE DU JEU (pas du code) : RemoveBuildingForPlayer ne s'applique qu'au
	moment ou on l'appelle pour un joueur donne. Ajouter un retrait l'applique
	immediatement a tous les joueurs deja connectes ET a tous ceux qui se
	connecteront ensuite (persistance + hook OnPlayerConnect). Supprimer un
	retrait de la liste ne "remet" pas le batiment pour les joueurs deja en
	ligne : il faudra qu'ils se reconnectent pour le revoir.

	PERSISTANCE : meme principe que MapPack_Save/MM_Save - un fichier texte
	dans scriptfiles, une ligne par retrait, relu au demarrage.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit    -> Bat_Init()
	- OnPlayerConnect   -> Bat_OnConnect(playerid)
	- /a batiment       -> Bat_Cmd(playerid, arg1, arg2)
*/

#if defined AFRP_BATIMENT_INCLUDED
	#endinput
#endif
#define AFRP_BATIMENT_INCLUDED

#define BAT_ADMIN_LEVEL     3
#define BAT_FILE            "batiments.cfg"
#define BAT_MAX             60
#define BAT_RAYON_DEFAUT    30.0

new batModel[BAT_MAX];
new Float:batX[BAT_MAX];
new Float:batY[BAT_MAX];
new Float:batZ[BAT_MAX];
new Float:batR[BAT_MAX];
new batCount = 0;

stock Bat_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Bat_Save()
{
	if(fexist(BAT_FILE)) fremove(BAT_FILE);
	new File:f = fopen(BAT_FILE, io_write);
	if(!f) return 0;
	new line[128];
	for(new k = 0; k < batCount; k++)
	{
		format(line, sizeof(line), "%d|%f|%f|%f|%f\n", batModel[k], batX[k], batY[k], batZ[k], batR[k]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

// Meme decoupeur maison que MM_Champ (afrp_mapmove.inc) : pas de dependance a
// un include externe, le format est ecrit par Bat_Save juste au-dessus et ne
// bougera pas.
stock Bat_Champ(const line[], num, sortie[], taille)
{
	new bI = 0, bChamp = 0, bK = 0;
	sortie[0] = 0;
	while(line[bI] && line[bI] != '\r' && line[bI] != '\n')
	{
		if(line[bI] == '|') { bChamp++; bI++; continue; }
		if(bChamp == num && bK < taille - 1) sortie[bK++] = line[bI];
		bI++;
	}
	sortie[bK] = 0;
	return bK;
}

stock Bat_Load()
{
	batCount = 0;
	if(!fexist(BAT_FILE)) return 0;
	new File:f = fopen(BAT_FILE, io_read);
	if(!f) return 0;

	new line[128], buf[32];
	while(fread(f, line) && batCount < BAT_MAX)
	{
		Bat_Champ(line, 0, buf, sizeof(buf)); batModel[batCount] = strval(buf);
		Bat_Champ(line, 1, buf, sizeof(buf)); batX[batCount] = floatstr(buf);
		Bat_Champ(line, 2, buf, sizeof(buf)); batY[batCount] = floatstr(buf);
		Bat_Champ(line, 3, buf, sizeof(buf)); batZ[batCount] = floatstr(buf);
		Bat_Champ(line, 4, buf, sizeof(buf)); batR[batCount] = floatstr(buf);
		batCount++;
	}
	fclose(f);
	return batCount;
}

stock Bat_Init()
{
	new n = Bat_Load();
	printf("[BATIMENT] %d retrait(s) de batiment charge(s).", n);
	return n;
}

// A appeler depuis OnPlayerConnect : rejoue tous les retraits enregistres pour
// ce joueur, exactement comme le fait deja MapPack_OnConnect pour ses cartes.
stock Bat_OnConnect(playerid)
{
	for(new k = 0; k < batCount; k++)
		RemoveBuildingForPlayer(playerid, batModel[k], batX[k], batY[k], batZ[k], batR[k]);
	return 1;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Bat_Liste(playerid)
{
	if(batCount == 0)
	{
		Bat_Msg(playerid, "{FFFF00}Aucun retrait de batiment enregistre. /a batiment <modelid> [rayon] pour en ajouter un a ta position.");
		return 1;
	}
	new s[128];
	for(new k = 0; k < batCount; k++)
	{
		format(s, sizeof(s), "{FFFF00}%d{FFFFFF} - modele %d, rayon %.1f, autour de %.1f / %.1f / %.1f", k, batModel[k], batR[k], batX[k], batY[k], batZ[k]);
		Bat_Msg(playerid, s);
	}
	return 1;
}

stock Bat_Cmd(playerid, const arg1[], const arg2[])
{
	if(PlayerInfo[playerid][pAdmin] < BAT_ADMIN_LEVEL)
	{
		new err[160];
		format(err, sizeof(err), "{FF0069}\xbb Erreur \xab{FFFFFF} Reserve aux admins niveau %d (tu es niveau %d).", BAT_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return Bat_Msg(playerid, err);
	}

	if(!strlen(arg1) || strcmp(arg1, "liste", true) == 0)
		return Bat_Liste(playerid);

	if(strcmp(arg1, "supprimer", true) == 0 || strcmp(arg1, "sup", true) == 0)
	{
		if(!strlen(arg2))
			return Bat_Msg(playerid, "{FF6347}Usage {FFFFB2}/a batiment supprimer <n>{FFFFFF} (voir /a batiment liste).");
		new n = strval(arg2);
		if(n < 0 || n >= batCount)
			return Bat_Msg(playerid, "{FF4444}Numero invalide (voir /a batiment liste).");
		for(new k = n; k < batCount - 1; k++)
		{
			batModel[k] = batModel[k + 1];
			batX[k] = batX[k + 1]; batY[k] = batY[k + 1]; batZ[k] = batZ[k + 1]; batR[k] = batR[k + 1];
		}
		batCount--;
		Bat_Save();
		Bat_Msg(playerid, "{00FF00}Retrait supprime de la liste. Les joueurs deja connectes le reverront seulement en se reconnectant.");
		return 1;
	}

	new modelid = strval(arg1);
	if(modelid <= 0)
		return Bat_Msg(playerid, "{FF6347}Usage {FFFFB2}/a batiment <modelid> [rayon]{FFFFFF} - retire ce batiment autour de toi.");

	if(batCount >= BAT_MAX)
		return Bat_Msg(playerid, "{FF4444}Limite de retraits atteinte (60). Supprime-en un d'abord (/a batiment liste).");

	new Float:rayon = strlen(arg2) ? floatstr(arg2) : BAT_RAYON_DEFAUT;
	if(rayon <= 0.0) rayon = BAT_RAYON_DEFAUT;

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);

	batModel[batCount] = modelid;
	batX[batCount] = px; batY[batCount] = py; batZ[batCount] = pz; batR[batCount] = rayon;
	batCount++;
	Bat_Save();

	// Applique tout de suite a tout le monde deja connecte - pas besoin
	// d'attendre une reconnexion, contrairement a une carte du mappack.
	new applied = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		RemoveBuildingForPlayer(i, modelid, px, py, pz, rayon);
		applied++;
	}

	new s[160];
	format(s, sizeof(s), "{00FF00}Batiment %d retire dans un rayon de %.1f autour de toi (%d joueur(s) deja connecte(s), et tous les futurs).", modelid, rayon, applied);
	return Bat_Msg(playerid, s);
}
