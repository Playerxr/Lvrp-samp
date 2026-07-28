/*
	afrp_gpspartage.inc - Partage de position en direct (telephone)

	Inspire des applications reelles : on partage sa position avec quelqu'un
	pour une duree limitee, et il te voit bouger en direct sur sa carte.

	  /position <id> [minutes]   partager avec un joueur
	  /position entreprise [min] partager avec toute son entreprise
	  /position                  menu : qui je suis, qui me suit
	  /position stop             couper tous mes partages

	=== ON NE PARTAGE QUE SA PROPRE POSITION ===
	Volontaire, et c'est la seule regle qui compte ici. Aucune commande ne
	permet de diffuser la position de quelqu'un d'autre : le partage part
	toujours de celui qui est localise, jamais de celui qui veut localiser.
	Le destinataire est prevenu, et l'emetteur peut couper a tout moment.

	=== PAS DE SAUVEGARDE, VOLONTAIREMENT ===
	Un partage est ephemere par nature. Le faire survivre a un redemarrage
	voudrait dire qu'un joueur deconnecte continue d'etre pistable sans l'avoir
	redemande : tout est efface au demarrage et a la deconnexion.

	=== ICONES ===
	Slots 62 a 66 : cinq personnes suivies au maximum. Le slot 50 est pris par
	le gamemode, 60 et 61 par les convois.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Gps_Init()
	- OnPlayerDisconnect -> Gps_OnDisconnect(playerid)
	- OnDialogResponse   -> Gps_OnDialog(playerid, dialogid, response, listitem)
	- /position          -> Gps_Command(playerid, cmdtext)
*/

#if defined AFRP_GPSPARTAGE_INCLUDED
	#endinput
#endif
#define AFRP_GPSPARTAGE_INCLUDED

#define GPS_MAX_SHARE      200      // partages simultanes sur le serveur
#define GPS_ICON_FIRST     62
#define GPS_ICON_COUNT     5        // personnes suivies au maximum
#define GPS_DUREE_DEF      15       // minutes par defaut
#define GPS_DUREE_MAX      60
#define GPS_TICK_MS        5000
#define GPS_ITEM_PHONE     1        // 1 = exiger un telephone, 0 = non

#define DIALOG_GPS_MENU    9626
#define DIALOG_GPS_QUI     9627

new gpsFrom[GPS_MAX_SHARE];         // qui partage (playerid)
new gpsTo[GPS_MAX_SHARE];           // destinataire, INVALID_PLAYER_ID si entreprise
new gpsEnt[GPS_MAX_SHARE];          // eid si partage entreprise, -1 sinon
new gpsExpire[GPS_MAX_SHARE];
new gpsUsed[GPS_MAX_SHARE];

new gpsIconCount[MAX_PLAYERS];      // icones actuellement posees chez ce joueur

// ============================================================================
// OUTILS
// ============================================================================
stock Gps_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Gps_AllocSlot()
{
	for(new k = 0; k < GPS_MAX_SHARE; k++)
		if(gpsUsed[k] == 0) return k;
	return -1;
}

stock Gps_Find(from, to, eid)
{
	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(gpsUsed[k] != 1) continue;
		if(gpsFrom[k] != from) continue;
		if(gpsTo[k] == to && gpsEnt[k] == eid) return k;
	}
	return -1;
}

// L'entreprise du joueur : la sienne s'il est patron, sinon celle qui l'emploie.
stock Gps_MonEntreprise(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid != -1) return eid;
	new outEid, outSlot;
	if(Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], outEid, outSlot)) return outEid;
	return -1;
}

// Vrai si ce partage doit s'afficher chez ce destinataire.
// destEid est fourni par l'appelant : Gps_MonEntreprise balaye toutes les
// entreprises et tous leurs employes, l'appeler pour chaque couple
// (partage, joueur) coutait jusqu'a un million d'iterations toutes les 5s.
stock Gps_ConcerneCe(k, dest, destEid)
{
	if(gpsUsed[k] != 1) return 0;
	if(gpsExpire[k] <= gettime()) return 0;
	if(gpsFrom[k] == dest) return 0;                  // on ne se suit pas soi-meme
	if(!IsPlayerConnected(gpsFrom[k])) return 0;
	if(gpsTo[k] != INVALID_PLAYER_ID) return (gpsTo[k] == dest);
	if(gpsEnt[k] < 0) return 0;
	return (destEid == gpsEnt[k]);
}

stock Gps_ClearIcons(playerid)
{
	for(new n = 0; n < gpsIconCount[playerid]; n++)
		RemovePlayerMapIcon(playerid, GPS_ICON_FIRST + n);
	gpsIconCount[playerid] = 0;
	return 1;
}

// ============================================================================
// TICK
// ============================================================================
forward Gps_Tick();
public Gps_Tick()
{
	// Purge des partages expires (et prevenance de l'emetteur).
	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(gpsUsed[k] != 1) continue;
		if(gpsExpire[k] > gettime() && IsPlayerConnected(gpsFrom[k])) continue;
		if(IsPlayerConnected(gpsFrom[k]) && gpsExpire[k] <= gettime())
			Gps_Msg(gpsFrom[k], "{AAAAAA} GPS  Un de tes partages de position a expire.");
		gpsUsed[k] = 0;
	}

	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i))
		{
			gpsIconCount[i] = 0;
			continue;
		}

		// Une seule resolution d'entreprise par joueur, reutilisee ensuite.
		new destEid = Gps_MonEntreprise(i);

		// Compter d'abord : un joueur sans partage et sans icone posee ne coute
		// aucun appel natif, sinon on balayerait tout le serveur toutes les 5s.
		new nb = 0;
		for(new k = 0; k < GPS_MAX_SHARE && nb < GPS_ICON_COUNT; k++)
			if(Gps_ConcerneCe(k, i, destEid)) nb++;
		if(nb == 0 && gpsIconCount[i] == 0) continue;

		Gps_ClearIcons(i);

		new pose = 0;
		for(new k = 0; k < GPS_MAX_SHARE && pose < GPS_ICON_COUNT; k++)
		{
			if(!Gps_ConcerneCe(k, i, destEid)) continue;
			new Float:x, Float:y, Float:z;
			GetPlayerPos(gpsFrom[k], x, y, z);
			SetPlayerMapIcon(i, GPS_ICON_FIRST + pose, x, y, z, 0, 0x00CCFFFF, MAPICON_GLOBAL);
			pose++;
		}
		gpsIconCount[i] = pose;
	}
	return 1;
}

// ============================================================================
// PARTAGE
// ============================================================================
stock Gps_PartagerJoueur(playerid, cible, minutes)
{
	if(!IsPlayerConnected(cible) || IsPlayerNPC(cible))
		return Gps_Msg(playerid, "{FF4444}Joueur invalide.");
	if(cible == playerid)
		return Gps_Msg(playerid, "{FF4444}Tu connais deja ta position.");
	#if GPS_ITEM_PHONE == 1
	if(hasPhone[playerid] == 0)
		return Gps_Msg(playerid, "{FF4444}Il te faut un telephone pour partager ta position.");
	if(hasPhone[cible] == 0)
		return Gps_Msg(playerid, "{FF4444}Cette personne n'a pas de telephone.");
	#endif
	if(minutes < 1) minutes = GPS_DUREE_DEF;
	if(minutes > GPS_DUREE_MAX) minutes = GPS_DUREE_MAX;

	new k = Gps_Find(playerid, cible, -1);
	if(k == -1)
	{
		k = Gps_AllocSlot();
		if(k == -1) return Gps_Msg(playerid, "{FF4444}Trop de partages en cours sur le serveur, reessaie plus tard.");
		gpsUsed[k] = 1;
		gpsFrom[k] = playerid;
		gpsTo[k]   = cible;
		gpsEnt[k]  = -1;
	}
	gpsExpire[k] = gettime() + minutes * 60;

	new s[190];
	format(s, sizeof(s), "{00FF00} GPS  Tu partages ta position avec %s pendant %d minute(s).", PlayerInfo[cible][pName], minutes);
	Gps_Msg(playerid, s);
	format(s, sizeof(s), "{00CCFF} GPS  %s partage sa position avec toi pendant %d minute(s). Regarde ta carte.", PlayerInfo[playerid][pName], minutes);
	Gps_Msg(cible, s);
	return 1;
}

stock Gps_PartagerEntreprise(playerid, minutes)
{
	new eid = Gps_MonEntreprise(playerid);
	if(eid == -1) return Gps_Msg(playerid, "{FF4444}Tu n'appartiens a aucune entreprise.");
	#if GPS_ITEM_PHONE == 1
	if(hasPhone[playerid] == 0)
		return Gps_Msg(playerid, "{FF4444}Il te faut un telephone pour partager ta position.");
	#endif
	if(minutes < 1) minutes = GPS_DUREE_DEF;
	if(minutes > GPS_DUREE_MAX) minutes = GPS_DUREE_MAX;

	new k = Gps_Find(playerid, INVALID_PLAYER_ID, eid);
	if(k == -1)
	{
		k = Gps_AllocSlot();
		if(k == -1) return Gps_Msg(playerid, "{FF4444}Trop de partages en cours sur le serveur, reessaie plus tard.");
		gpsUsed[k] = 1;
		gpsFrom[k] = playerid;
		gpsTo[k]   = INVALID_PLAYER_ID;
		gpsEnt[k]  = eid;
	}
	gpsExpire[k] = gettime() + minutes * 60;

	new s[200];
	format(s, sizeof(s), "{00FF00} GPS  Ta position est partagee avec %s pendant %d minute(s).", entreprises[eid][entName], minutes);
	Gps_Msg(playerid, s);

	format(s, sizeof(s), "{00CCFF} GPS  %s partage sa position avec l'entreprise pendant %d minute(s).", PlayerInfo[playerid][pName], minutes);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i) || i == playerid) continue;
		if(Gps_MonEntreprise(i) != eid) continue;
		Gps_Msg(i, s);
	}
	return 1;
}

stock Gps_Stop(playerid)
{
	new n = 0;
	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(gpsUsed[k] != 1 || gpsFrom[k] != playerid) continue;
		gpsUsed[k] = 0;
		n++;
	}
	if(n == 0) return Gps_Msg(playerid, "{FFFF00}Tu ne partages ta position avec personne.");
	new s[128];
	format(s, sizeof(s), "{FFFF00} GPS  %d partage(s) coupe(s). Plus personne ne te suit.", n);
	return Gps_Msg(playerid, s);
}

// ============================================================================
// MENU
// ============================================================================
stock Gps_ShowMenu(playerid)
{
	new nMoi = 0, nEux = 0, monEid = Gps_MonEntreprise(playerid);
	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(gpsUsed[k] != 1 || gpsExpire[k] <= gettime()) continue;
		if(gpsFrom[k] == playerid) nMoi++;
		else if(Gps_ConcerneCe(k, playerid, monEid)) nEux++;
	}

	new body[600];
	format(body, sizeof(body), "{AAAAAA}Tu partages avec %d personne(s) - %d personne(s) partagent avec toi\n{00FF00}Partager avec un joueur (/position <id>)\n{00FF00}Partager avec mon entreprise\n{FFFFFF}Qui partage avec moi\n{FF8888}Couper tous mes partages", nMoi, nEux);
	return ShowPlayerDialog(playerid, DIALOG_GPS_MENU, DIALOG_STYLE_LIST, "Partage de position", body, "Choisir", "Fermer");
}

stock Gps_ShowQui(playerid)
{
	new body[1000], str[176], n = 0, monEid = Gps_MonEntreprise(playerid);
	body[0] = 0;

	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(!Gps_ConcerneCe(k, playerid, monEid)) continue;
		new reste = (gpsExpire[k] - gettime()) / 60;
		if(gpsEnt[k] >= 0)
			format(str, sizeof(str), "{FFFFFF}%s{AAAAAA} (via l'entreprise) - encore %d min\n", PlayerInfo[gpsFrom[k]][pName], reste + 1);
		else
			format(str, sizeof(str), "{FFFFFF}%s{AAAAAA} - encore %d min\n", PlayerInfo[gpsFrom[k]][pName], reste + 1);
		strcat(body, str);
		n++;
	}

	if(n == 0) format(body, sizeof(body), "{AAAAAA}Personne ne partage sa position avec toi.");
	else strcat(body, "\n{AAAAAA}Ces personnes sont marquees en bleu sur ta carte (5 au maximum).");
	return ShowPlayerDialog(playerid, DIALOG_GPS_QUI, DIALOG_STYLE_MSGBOX, "Qui partage avec moi", body, "Fermer", "");
}

stock Gps_OnDialog(playerid, dialogid, response, listitem)
{
	switch(dialogid)
	{
		case DIALOG_GPS_MENU:
		{
			if(!response) return 1;
			// item 0 = ligne de resume, les actions commencent a 1.
			switch(listitem)
			{
				case 0: Gps_ShowMenu(playerid);
				case 1: Gps_Msg(playerid, "{FFFFFF}Tape {FFFF00}/position <id du joueur> [minutes]{FFFFFF} pour partager.");
				case 2: Gps_PartagerEntreprise(playerid, GPS_DUREE_DEF);
				case 3: Gps_ShowQui(playerid);
				case 4: Gps_Stop(playerid);
			}
			return 1;
		}
		case DIALOG_GPS_QUI: return 1;
	}
	return 0;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Gps_Tok(const src[], &gpsPos, dest[], destSize)
{
	new p = 0;
	while(src[gpsPos] == ' ') gpsPos++;
	while(src[gpsPos] && src[gpsPos] != ' ' && p < destSize - 1)
		dest[p++] = src[gpsPos++];
	dest[p] = 0;
	return p;
}

stock Gps_Command(playerid, const cmdline[])
{
	new gpsPos = 0, a1[24], a2[16];
	Gps_Tok(cmdline, gpsPos, a1, sizeof(a1));            // consomme "/position"
	if(Gps_Tok(cmdline, gpsPos, a1, sizeof(a1)) == 0) return Gps_ShowMenu(playerid);

	if(strcmp(a1, "stop", true) == 0)   return Gps_Stop(playerid);
	if(strcmp(a1, "liste", true) == 0)  return Gps_ShowQui(playerid);
	if(strcmp(a1, "menu", true) == 0)   return Gps_ShowMenu(playerid);

	Gps_Tok(cmdline, gpsPos, a2, sizeof(a2));
	new minutes = strval(a2);

	if(strcmp(a1, "entreprise", true) == 0 || strcmp(a1, "ent", true) == 0)
		return Gps_PartagerEntreprise(playerid, minutes);

	// strval("abc") vaut 0 : sans ce controle, une faute de frappe partageait
	// silencieusement la position avec le joueur d'ID 0.
	if(a1[0] < '0' || a1[0] > '9')
	{
		Gps_Msg(playerid, "{FF4444}Usage : {FFFF00}/position <id du joueur> [minutes]");
		Gps_Msg(playerid, "{AAAAAA}ou /position entreprise | /position liste | /position stop");
		return 1;
	}
	return Gps_PartagerJoueur(playerid, strval(a1), minutes);
}

// ============================================================================
// HOOKS
// ============================================================================
stock Gps_OnDisconnect(playerid)
{
	// Ses partages sortants ET les icones posees chez lui disparaissent : un
	// joueur deconnecte ne doit plus etre pistable, ni garder des marqueurs.
	for(new k = 0; k < GPS_MAX_SHARE; k++)
	{
		if(gpsUsed[k] != 1) continue;
		if(gpsFrom[k] == playerid || gpsTo[k] == playerid) gpsUsed[k] = 0;
	}
	gpsIconCount[playerid] = 0;
	return 1;
}

stock Gps_Init()
{
	for(new k = 0; k < GPS_MAX_SHARE; k++) gpsUsed[k] = 0;
	for(new i = 0; i < MAX_PLAYERS; i++) gpsIconCount[i] = 0;
	SetTimer("Gps_Tick", GPS_TICK_MS, true);
	return 1;
}
