/*
	afrp_mappack.inc - Pack de mappings importes

	Sept mappings faits par des mappeurs de la communaute ont ete fournis en
	export Texture Studio. Les coller tels quels dans le gamemode aurait pose
	deux problemes : impossible de faire marche arriere si un decor tombe au
	mauvais endroit, et aucun moyen de regarder le resultat avant de le rendre
	visible a tous les joueurs.

	Chaque carte est donc une unite qu'on allume et qu'on eteint en jeu, dont
	le choix survit au redemarrage, et sur laquelle on peut se teleporter pour
	juger avant de decider.

	  /mappack              liste les cartes, leur etat, et permet d'agir
	  /coffrebanque         ouvre / ferme la porte du coffre (Banque du coin)

	=== POURQUOI TOUT EST ETEINT AU DEPART ===
	Deux de ces cartes se posent la ou le gamemode a deja des objets (le
	commissariat et la banque du coin). Allumer les sept d'office aurait pu
	enterrer un decor existant sans que personne ne s'en apercoive avant les
	plaintes des joueurs. On regarde, puis on allume.

	=== LES BATIMENTS D'ORIGINE ===
	Une carte qui remplace un batiment doit d'abord le retirer. SA-MP ne sait
	retirer un batiment qu'au moment ou le joueur se connecte : pour les
	joueurs deja en ligne quand on allume une carte, le batiment d'origine
	reste visible jusqu'a leur reconnexion. C'est une limite du jeu, pas du
	code, et le message le dit.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit    -> MapPack_Init()
	- OnPlayerConnect   -> MapPack_OnConnect(playerid)
	- OnDialogResponse  -> MapPack_OnDialog(playerid, ...)
	- /mappack          -> MapPack_Cmd(playerid)
	- /coffrebanque     -> MapPack_Coffre(playerid)
*/

#if defined AFRP_MAPPACK_INCLUDED
	#endinput
#endif
#define AFRP_MAPPACK_INCLUDED

#define MP_ADMIN_LEVEL   3
#define MP_FILE          "mappack.cfg"
#define DIALOG_MP_LISTE  9637
#define DIALOG_MP_CARTE  9638

new mpActif[MP_MAX_CARTES];              // 1 = objets crees
new mpObj[MP_MAX_CARTES][MP_MAX_OBJ];    // ids streamer, pour pouvoir detruire
new mpNbCree[MP_MAX_CARTES];
new mpCarteSel[MAX_PLAYERS];             // carte ouverte dans le dialogue
new bool:mpCoffreOuvert = false;

// ============================================================================
// OUTILS
// ============================================================================
stock MapPack_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock MapPack_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= MP_ADMIN_LEVEL);
}

// Appele par le code genere a chaque objet cree : c'est ce qui rend une carte
// desactivable. Renvoie l'id pour rester utilisable en une seule expression.
stock MapPack_Add(carte, objectid)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return objectid;
	if(mpNbCree[carte] < MP_MAX_OBJ) mpObj[carte][mpNbCree[carte]++] = objectid;
	return objectid;
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock MapPack_Save()
{
	if(fexist(MP_FILE)) fremove(MP_FILE);
	new File:f = fopen(MP_FILE, io_write);
	if(!f) return 0;
	new line[16];
	for(new c = 0; c < MP_MAX_CARTES; c++)
	{
		format(line, sizeof(line), "%d|%d\n", c, mpActif[c]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock MapPack_Load()
{
	for(new c = 0; c < MP_MAX_CARTES; c++) { mpActif[c] = 0; mpNbCree[c] = 0; }
	if(!fexist(MP_FILE)) return 0;
	new File:f = fopen(MP_FILE, io_read);
	if(!f) return 0;

	new line[32], n = 0;
	while(fread(f, line))
	{
		new p = 0, buf[8], carte, etat;
		while(p < 7 && line[p] && line[p] != '|') { buf[p] = line[p]; p++; }
		buf[p] = 0;
		carte = strval(buf);
		if(line[p] == '|') p++;
		etat = strval(line[p]);
		if(carte >= 0 && carte < MP_MAX_CARTES && etat == 1) { mpActif[carte] = 1; n++; }
	}
	fclose(f);
	return n;
}

// ============================================================================
// ALLUMER / ETEINDRE
// ============================================================================
stock MapPack_Creer(carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return 0;
	if(mpNbCree[carte] > 0) return 0;         // deja en place
	MapPack_CreerCarte(carte);
	printf("[MapPack] %s : %d objet(s) crees.", mpNom[carte], mpNbCree[carte]);
	return mpNbCree[carte];
}

stock MapPack_Detruire(carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return 0;
	new n = 0;
	for(new k = 0; k < mpNbCree[carte]; k++)
	{
		if(mpObj[carte][k] > 0 && IsValidDynamicObject(mpObj[carte][k]))
		{
			DestroyDynamicObject(mpObj[carte][k]);
			n++;
		}
		mpObj[carte][k] = 0;
	}
	mpNbCree[carte] = 0;
	if(carte == MP_BANQUE2) { mpVaultDoor = -1; mpCoffreOuvert = false; }
	return n;
}

// Retire les batiments d'origine des cartes allumees. SA-MP exige que ce soit
// fait a la connexion : plus tard, le batiment reste visible pour ce joueur.
stock MapPack_Retraits(playerid, carteSeule = -1)
{
	new n = 0;
	for(new r = 0; r < MP_MAX_RETRAIT; r++)
	{
		new c = mpRmCarte[r];
		if(carteSeule != -1 && c != carteSeule) continue;
		if(mpActif[c] != 1) continue;
		RemoveBuildingForPlayer(playerid, mpRmModel[r], mpRmX[r], mpRmY[r], mpRmZ[r], mpRmR[r]);
		n++;
	}
	return n;
}

stock MapPack_Activer(playerid, carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return 0;
	if(mpActif[carte] == 1) return MapPack_Msg(playerid, "{FFFF00}Cette carte est deja allumee.");

	// [COMMERCES] Trois cartes du pack sont exactement les memes maps que celles
	// desormais posees en dur par afrp_commerces_map.inc / afrp_postepolice.inc,
	// aux memes coordonnees. Les allumer poserait un deuxieme jeu d'objets par
	// dessus le premier : rien de visible au premier coup d'oeil, mais toutes
	// les surfaces se mettent a clignoter et le decor devient impossible a
	// deplacer proprement. On refuse plutot que de laisser le piege ouvert.
	if(carte == MP_WISE || carte == MP_MARKET || carte == MP_PRF)
	{
		MapPack_Msg(playerid, "{FF4444}Cette carte est deja installee en permanence par le serveur.");
		MapPack_Msg(playerid, "{AAAAAA}L'allumer ici la poserait une deuxieme fois au meme endroit. Utilise /a deplacer pour la bouger.");
		return 1;
	}

	mpActif[carte] = 1;
	new n = MapPack_Creer(carte);
	MapPack_Save();

	new s[190];
	format(s, sizeof(s), "{00FF00}%s allumee : %d objet(s) poses.", mpNom[carte], n);
	MapPack_Msg(playerid, s);

	// Combien de batiments d'origine cette carte remplace-t-elle ?
	new nbRm = 0;
	for(new r = 0; r < MP_MAX_RETRAIT; r++) if(mpRmCarte[r] == carte) nbRm++;
	if(nbRm > 0)
	{
		for(new i = 0; i < MAX_PLAYERS; i++)
		{
			if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
			MapPack_Retraits(i, carte);
		}
		format(s, sizeof(s), "{FFFF00}%d batiment(s) d'origine a retirer. Les joueurs deja connectes devront se reconnecter pour ne plus les voir.", nbRm);
		MapPack_Msg(playerid, s);
	}
	format(s, sizeof(s), "{AAAAAA}Va voir sur place avant de laisser la carte allumee : /mappack, puis Aller voir.");
	MapPack_Msg(playerid, s);
	return 1;
}

stock MapPack_Desactiver(playerid, carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return 0;
	if(mpActif[carte] == 0) return MapPack_Msg(playerid, "{FFFF00}Cette carte est deja eteinte.");

	new n = MapPack_Detruire(carte);
	mpActif[carte] = 0;
	MapPack_Save();

	new s[190];
	format(s, sizeof(s), "{FFFF00}%s eteinte : %d objet(s) retires.", mpNom[carte], n);
	MapPack_Msg(playerid, s);
	MapPack_Msg(playerid, "{AAAAAA}Les batiments d'origine qu'elle remplacait reviendront a la prochaine reconnexion des joueurs.");
	return 1;
}

// ============================================================================
// PORTE DU COFFRE (Banque du coin)
// ============================================================================
stock MapPack_Coffre(playerid)
{
	if(!MapPack_IsAdmin(playerid)) return MapPack_Msg(playerid, "{FF4444}Reserve aux administrateurs niveau 3.");
	if(mpActif[MP_BANQUE2] != 1) return MapPack_Msg(playerid, "{FF4444}La carte Banque du coin n'est pas allumee. /mappack");
	if(mpVaultDoor == -1 || !IsValidDynamicObject(mpVaultDoor))
		return MapPack_Msg(playerid, "{FF4444}Porte de coffre introuvable.");

	// La porte pivote : l'export d'origine deplace l'objet d'un dix-millieme
	// d'unite pour n'animer que la rotation.
	if(mpCoffreOuvert)
	{
		mpCoffreOuvert = false;
		MoveDynamicObject(mpVaultDoor, 1444.822631, -1124.319946, 24.4879, 0.0001, 0.000000, 0.000000, 579.799987);
		MapPack_Msg(playerid, "{FFFF00}Coffre : fermeture.");
	}
	else
	{
		mpCoffreOuvert = true;
		MoveDynamicObject(mpVaultDoor, 1444.822631, -1124.319946, 24.4881, 0.0001, 0.000000, 0.000000, 127.799964);
		MapPack_Msg(playerid, "{00FF00}Coffre : ouverture.");
	}
	return 1;
}

// ============================================================================
// DIALOGUES
// ============================================================================
stock MapPack_Liste(playerid)
{
	if(!MapPack_IsAdmin(playerid))
	{
		new s[160];
		format(s, sizeof(s), "{FF4444}Reserve aux admins niveau %d (tu es niveau %d).", MP_ADMIN_LEVEL, PlayerInfo[playerid][pAdmin]);
		return MapPack_Msg(playerid, s);
	}

	new body[900], str[128];
	body[0] = 0;
	for(new c = 0; c < MP_MAX_CARTES; c++)
	{
		format(str, sizeof(str), "%s%s\t%d objets\t%s\n",
			(mpActif[c] == 1) ? ("{00FF00}") : ("{888888}"), mpNom[c], mpNbObj[c],
			(mpActif[c] == 1) ? ("allumee") : ("eteinte"));
		strcat(body, str);
	}
	mpCarteSel[playerid] = -1;
	return ShowPlayerDialog(playerid, DIALOG_MP_LISTE, DIALOG_STYLE_TABLIST, "Pack de mappings", body, "Ouvrir", "Fermer");
}

stock MapPack_Carte(playerid, carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return MapPack_Liste(playerid);

	// Les trois actions sont placees EN PREMIER et dans un ordre fixe. Dans un
	// dialogue LIST chaque ligne est cliquable, y compris les lignes vides :
	// mettre l'information d'abord obligerait a recalculer un decalage a
	// chaque changement de texte, et une ligne oubliee ferait cliquer le
	// joueur sur la mauvaise action.
	new body[600], str[176];
	body[0] = 0;
	strcat(body, "Aller voir sur place\n");
	strcat(body, (mpActif[carte] == 1) ? ("{FF8888}Eteindre cette carte\n") : ("{00FF00}Allumer cette carte\n"));
	strcat(body, "{FFFF00}<< Retour a la liste\n");

	format(str, sizeof(str), "{AAAAAA}Mapping de %s - %d objets\n", mpAuteur[carte], mpNbObj[carte]);
	strcat(body, str);
	format(str, sizeof(str), "{AAAAAA}Etat : %s\n", (mpActif[carte] == 1) ? ("allumee") : ("eteinte"));
	strcat(body, str);

	new nbRm = 0;
	for(new r = 0; r < MP_MAX_RETRAIT; r++) if(mpRmCarte[r] == carte) nbRm++;
	if(nbRm > 0)
	{
		format(str, sizeof(str), "{AAAAAA}Remplace %d batiment(s) d'origine.", nbRm);
		strcat(body, str);
	}

	mpCarteSel[playerid] = carte;
	return ShowPlayerDialog(playerid, DIALOG_MP_CARTE, DIALOG_STYLE_LIST, mpNom[carte], body, "Choisir", "Retour");
}

stock MapPack_Aller(playerid, carte)
{
	if(carte < 0 || carte >= MP_MAX_CARTES) return 0;
	SafeTeleportPlayer(playerid, mpVoirX[carte], mpVoirY[carte], mpVoirZ[carte], 0, 0);
	new s[190];
	format(s, sizeof(s), "{00FF00}Sur place : %s.%s", mpNom[carte],
		(mpActif[carte] == 1) ? ("") : (" {FFFF00}Attention, la carte est eteinte : il n'y a rien a voir."));
	return MapPack_Msg(playerid, s);
}

stock MapPack_OnDialog(playerid, dialogid, response, listitem)
{
	if(dialogid != DIALOG_MP_LISTE && dialogid != DIALOG_MP_CARTE) return 0;
	if(!MapPack_IsAdmin(playerid)) return 1;

	if(dialogid == DIALOG_MP_LISTE)
	{
		if(!response) return 1;
		return MapPack_Carte(playerid, listitem);
	}

	// DIALOG_MP_CARTE : les actions occupent les lignes 0, 1 et 2. Tout ce qui
	// suit n'est que de l'information : un clic dessus rouvre la fiche.
	if(!response) return MapPack_Liste(playerid);

	new carte = mpCarteSel[playerid];
	if(carte < 0 || carte >= MP_MAX_CARTES) return MapPack_Liste(playerid);

	if(listitem == 0) return MapPack_Aller(playerid, carte);
	if(listitem == 1)
	{
		if(mpActif[carte] == 1) MapPack_Desactiver(playerid, carte);
		else                    MapPack_Activer(playerid, carte);
		return MapPack_Carte(playerid, carte);
	}
	if(listitem == 2) return MapPack_Liste(playerid);
	return MapPack_Carte(playerid, carte);
}

stock MapPack_Cmd(playerid)
{
	return MapPack_Liste(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock MapPack_OnConnect(playerid)
{
	mpCarteSel[playerid] = -1;
	MapPack_Retraits(playerid);
	return 1;
}

stock MapPack_Init()
{
	for(new i = 0; i < MAX_PLAYERS; i++) mpCarteSel[i] = -1;
	new n = MapPack_Load();
	// Meme raison que dans MapPack_Activer : ces trois cartes sont maintenant
	// posees en permanence ailleurs. Si un ancien mappack.cfg les avait laissees
	// allumees, on les eteint au chargement plutot que de doubler le decor.
	if(mpActif[MP_WISE] == 1)   { mpActif[MP_WISE] = 0;   n--; }
	if(mpActif[MP_MARKET] == 1) { mpActif[MP_MARKET] = 0; n--; }
	if(mpActif[MP_PRF] == 1)    { mpActif[MP_PRF] = 0;    n--; }
	if(n < 0) n = 0;
	for(new c = 0; c < MP_MAX_CARTES; c++) if(mpActif[c] == 1) MapPack_Creer(c);
	printf("[MapPack] %d carte(s) allumee(s) sur %d disponibles.", n, MP_MAX_CARTES);
	return 1;
}
