/*
	afrp_residence.inc - Une residence privee pour un joueur special

	Le complexe de Las Venturas (carte importee, /mappack) est un grand decor
	ouvert : n'importe qui peut y marcher. En faire "une veritable maison"
	demande trois choses qu'un decor seul n'a pas : un proprietaire, un point
	de retour, et un endroit pour ranger de l'argent en securite. Ce module
	ajoute les trois, sans toucher au systeme de maisons existant du gamemode
	(achat, meubles, loyer) : celui-ci est concu pour des interieurs GTA
	achetables par n'importe qui, pas pour une residence unique confiee a une
	personne choisie.

	  /maisonvip              statut, et pour le proprietaire : ses actions

	Administrateur niveau 3 :
	  /maisonvip assigner <id>     fait de ce joueur le proprietaire
	  /maisonvip revoquer          retire le proprietaire actuel
	  /maisonvip definir           pose ICI le point de retour du proprietaire
	  /maisonvip definircoffre     pose ICI le coffre-fort du proprietaire

	=== POURQUOI L'ACCES SE FAIT PAR /portail, PAS PAR CE MODULE ===
	Verrouiller l'entree du complexe, c'est le travail deja fait par
	afrp_portes.inc : /portail creer a la porte, puis /portail proprietaire
	pour la reserver a ce joueur precis. Ce module ne gere que ce qui est
	propre a une residence (retour, coffre) ; il ne duplique pas la logique
	de porte.

	=== LE COFFRE NE STOCKE QUE DE L'ARGENT ===
	Un coffre a objets aurait demande de reconstruire tout ce que /sac fait
	deja (grille, poids, categories). Le proprietaire range deja ses objets
	dans son sac ; ce coffre lui donne seulement un endroit pour mettre de
	l'argent a l'abri d'un braquage ou d'une arrestation, comme un vrai
	coffre-fort de maison.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit    -> Residence_Init()
	- OnDialogResponse  -> if(Residence_OnDialog(playerid, dialogid, response, listitem, inputtext)) return 1;
	- /maisonvip        -> Residence_Cmd(playerid, cmdtext)
*/

#if defined AFRP_RESIDENCE_INCLUDED
	#endinput
#endif
#define AFRP_RESIDENCE_INCLUDED

#define RES_ADMIN_LEVEL   3
#define RES_FILE          "residence.cfg"
#define RES_COFFRE_DIST   3.0

#define DIALOG_RES_MENU    9651
#define DIALOG_RES_DEPOSER 9652
#define DIALOG_RES_RETIRER 9653

new resProprio[MAX_PLAYER_NAME];        // "" = pas de proprietaire

new bool:resAccueilPose = false;
new Float:resAccueilX, Float:resAccueilY, Float:resAccueilZ;
new resAccueilInt, resAccueilVw;

new bool:resCoffrePose = false;
new Float:resCoffreX, Float:resCoffreY, Float:resCoffreZ;
new resCoffreInt, resCoffreVw;
new resCoffreCash;

// ============================================================================
// OUTILS
// ============================================================================
stock Residence_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

stock Residence_IsAdmin(playerid)
{
	return (PlayerInfo[playerid][pAdmin] >= RES_ADMIN_LEVEL);
}

stock Residence_EstProprio(playerid)
{
	if(!strlen(resProprio)) return 0;
	return (strcmp(PlayerInfo[playerid][pName], resProprio, true) == 0);
}

// ============================================================================
// PERSISTANCE
// ============================================================================
stock Residence_Save()
{
	if(fexist(RES_FILE)) fremove(RES_FILE);
	new File:f = fopen(RES_FILE, io_write);
	if(!f) return 0;
	new line[220];
	format(line, sizeof(line), "%s|%d|%.4f|%.4f|%.4f|%d|%d|%d|%.4f|%.4f|%.4f|%d|%d|%d\n",
		resProprio,
		resAccueilPose, resAccueilX, resAccueilY, resAccueilZ, resAccueilInt, resAccueilVw,
		resCoffrePose, resCoffreX, resCoffreY, resCoffreZ, resCoffreInt, resCoffreVw,
		resCoffreCash);
	fwrite(f, line);
	fclose(f);
	return 1;
}

stock Residence_Champ(const line[], &k, dest[], destSize)
{
	new p = 0;
	while(p < destSize - 1 && line[k] && line[k] != '|' && line[k] != '\r' && line[k] != '\n')
		dest[p++] = line[k++];
	dest[p] = 0;
	if(line[k] == '|') k++;
	return p;
}

stock Residence_Load()
{
	resProprio[0] = 0;
	resAccueilPose = false;
	resCoffrePose  = false;
	resCoffreCash  = 0;
	if(!fexist(RES_FILE)) return 0;
	new File:f = fopen(RES_FILE, io_read);
	if(!f) return 0;

	new line[220], buf[40];
	if(fread(f, line))
	{
		new i = 0;
		Residence_Champ(line, i, resProprio, MAX_PLAYER_NAME);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilPose = bool:strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilX    = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilY    = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilZ    = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilInt  = strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resAccueilVw   = strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffrePose  = bool:strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreX     = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreY     = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreZ     = floatstr(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreInt   = strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreVw    = strval(buf);
		Residence_Champ(line, i, buf, sizeof(buf)); resCoffreCash  = strval(buf);
	}
	fclose(f);
	if(strlen(resProprio)) printf("[Residence] Proprietaire charge : %s", resProprio);
	return 1;
}

// ============================================================================
// ADMINISTRATION
// ============================================================================
stock Residence_Assigner(playerid, cible)
{
	if(!IsPlayerConnected(cible) || IsPlayerNPC(cible))
		return Residence_Msg(playerid, "{FF4444}Ce joueur n'est pas connecte.");

	format(resProprio, MAX_PLAYER_NAME, "%s", PlayerInfo[cible][pName]);
	Residence_Save();

	new s[160];
	format(s, sizeof(s), "{00FF00}%s est maintenant proprietaire de la residence.", resProprio);
	Residence_Msg(playerid, s);
	format(s, sizeof(s), "{00FF00}Tu es maintenant proprietaire d'une residence. Tape /maisonvip.");
	Residence_Msg(cible, s);
	return 1;
}

stock Residence_Revoquer(playerid)
{
	if(!strlen(resProprio)) return Residence_Msg(playerid, "{FF8888}Il n'y a deja aucun proprietaire.");
	new s[160];
	format(s, sizeof(s), "{FFFF00}%s n'est plus proprietaire de la residence.", resProprio);
	resProprio[0] = 0;
	Residence_Save();
	return Residence_Msg(playerid, s);
}

stock Residence_Definir(playerid)
{
	GetPlayerPos(playerid, resAccueilX, resAccueilY, resAccueilZ);
	resAccueilInt = GetPlayerInterior(playerid);
	resAccueilVw  = GetPlayerVirtualWorld(playerid);
	resAccueilPose = true;
	Residence_Save();
	return Residence_Msg(playerid, "{00FF00}Point de retour de la residence pose ici.");
}

stock Residence_DefinirCoffre(playerid)
{
	GetPlayerPos(playerid, resCoffreX, resCoffreY, resCoffreZ);
	resCoffreInt = GetPlayerInterior(playerid);
	resCoffreVw  = GetPlayerVirtualWorld(playerid);
	resCoffrePose = true;
	Residence_Save();
	return Residence_Msg(playerid, "{00FF00}Coffre-fort de la residence pose ici.");
}

// ============================================================================
// MENU DU PROPRIETAIRE
// ============================================================================
stock Residence_PresCoffre(playerid)
{
	if(!resCoffrePose) return 0;
	if(GetPlayerVirtualWorld(playerid) != resCoffreVw || GetPlayerInterior(playerid) != resCoffreInt) return 0;
	return IsPlayerInRangeOfPoint(playerid, RES_COFFRE_DIST, resCoffreX, resCoffreY, resCoffreZ);
}

stock Residence_Menu(playerid)
{
	if(!Residence_EstProprio(playerid))
	{
		if(!strlen(resProprio))
			return Residence_Msg(playerid, "{AAAAAA}Cette residence n'a pas encore de proprietaire.");
		new s[144];
		format(s, sizeof(s), "{AAAAAA}Cette residence appartient a %s.", resProprio);
		return Residence_Msg(playerid, s);
	}

	new body[400];
	format(body, sizeof(body), "{FFFFFF}Retourner chez moi\n{FFFFFF}Coffre-fort {AAAAAA}(solde : $%d)\n{AAAAAA}Le coffre ne s'ouvre que place devant lui.", resCoffreCash);
	return ShowPlayerDialog(playerid, DIALOG_RES_MENU, DIALOG_STYLE_LIST, "Ma residence", body, "Choisir", "Fermer");
}

stock Residence_RetournerChezMoi(playerid)
{
	if(!resAccueilPose) return Residence_Msg(playerid, "{FF8888}Le point de retour n'a pas encore ete pose par un admin.");
	SafeTeleportPlayer(playerid, resAccueilX, resAccueilY, resAccueilZ, resAccueilInt, resAccueilVw);
	return Residence_Msg(playerid, "{00FF00}Te voila chez toi.");
}

stock Residence_OuvrirCoffre(playerid)
{
	if(!resCoffrePose) return Residence_Msg(playerid, "{FF8888}Le coffre n'a pas encore ete pose par un admin.");
	if(!Residence_PresCoffre(playerid)) return Residence_Msg(playerid, "{FF8888}Tu dois etre devant le coffre pour l'ouvrir.");

	new body[300];
	format(body, sizeof(body), "{FFFFFF}Deposer de l'argent\n{FFFFFF}Retirer de l'argent\n{AAAAAA}Solde du coffre : $%d\n{AAAAAA}Ton liquide : $%d", resCoffreCash, PlayerInfo[playerid][pCash]);
	return ShowPlayerDialog(playerid, DIALOG_RES_MENU + 1000, DIALOG_STYLE_LIST, "Coffre-fort", body, "Choisir", "Retour");
}

// ============================================================================
// DIALOGUES
// ============================================================================
stock Residence_OnDialog(playerid, dialogid, response, listitem, const inputtext[])
{
	if(dialogid == DIALOG_RES_MENU)
	{
		if(!response) return 1;
		if(!Residence_EstProprio(playerid)) return 1;
		if(listitem == 0) Residence_RetournerChezMoi(playerid);
		else if(listitem == 1) Residence_OuvrirCoffre(playerid);
		return 1;
	}

	// Sous-menu du coffre : identifiant separe pour ne pas confondre les deux
	// listes, toutes deux DIALOG_STYLE_LIST.
	if(dialogid == DIALOG_RES_MENU + 1000)
	{
		if(!response) return Residence_Menu(playerid);
		if(!Residence_EstProprio(playerid) || !Residence_PresCoffre(playerid)) return 1;

		if(listitem == 0)
			return ShowPlayerDialog(playerid, DIALOG_RES_DEPOSER, DIALOG_STYLE_INPUT, "Deposer au coffre",
				"{FFFFFF}Montant a deposer :", "Deposer", "Retour");
		if(listitem == 1)
			return ShowPlayerDialog(playerid, DIALOG_RES_RETIRER, DIALOG_STYLE_INPUT, "Retirer du coffre",
				"{FFFFFF}Montant a retirer :", "Retirer", "Retour");
		return 1;
	}

	if(dialogid == DIALOG_RES_DEPOSER)
	{
		if(!response) return Residence_OuvrirCoffre(playerid);
		if(!Residence_EstProprio(playerid) || !Residence_PresCoffre(playerid)) return 1;

		new montant = strval(inputtext);
		if(montant <= 0 || montant > PlayerInfo[playerid][pCash])
		{
			Residence_Msg(playerid, "{FF4444}Montant invalide.");
			return Residence_OuvrirCoffre(playerid);
		}
		SafeGivePlayerMoney(playerid, -montant, "Coffre residence");
		resCoffreCash += montant;
		Residence_Save();
		new s[144];
		format(s, sizeof(s), "{00FF00}%d$ deposes. Solde du coffre : $%d.", montant, resCoffreCash);
		Residence_Msg(playerid, s);
		return Residence_OuvrirCoffre(playerid);
	}

	if(dialogid == DIALOG_RES_RETIRER)
	{
		if(!response) return Residence_OuvrirCoffre(playerid);
		if(!Residence_EstProprio(playerid) || !Residence_PresCoffre(playerid)) return 1;

		new montant = strval(inputtext);
		if(montant <= 0 || montant > resCoffreCash)
		{
			Residence_Msg(playerid, "{FF4444}Montant invalide.");
			return Residence_OuvrirCoffre(playerid);
		}
		resCoffreCash -= montant;
		SafeGivePlayerMoney(playerid, montant, "Coffre residence");
		Residence_Save();
		new s[144];
		format(s, sizeof(s), "{00FF00}%d$ retires. Solde du coffre : $%d.", montant, resCoffreCash);
		Residence_Msg(playerid, s);
		return Residence_OuvrirCoffre(playerid);
	}

	return 0;
}

// ============================================================================
// COMMANDE
// ============================================================================
stock Residence_Tok(const src[], &pos2, dest[], destSize)
{
	new p = 0;
	while(src[pos2] == ' ') pos2++;
	while(src[pos2] && src[pos2] != ' ' && p < destSize - 1) dest[p++] = src[pos2++];
	dest[p] = 0;
	return p;
}

stock Residence_Cmd(playerid, const cmdline[])
{
	new pos2 = 0, sub[24];
	Residence_Tok(cmdline, pos2, sub, sizeof(sub));            // consomme "/maisonvip"
	if(Residence_Tok(cmdline, pos2, sub, sizeof(sub)) == 0) return Residence_Menu(playerid);

	if(!Residence_IsAdmin(playerid))
		return Residence_Menu(playerid);

	if(strcmp(sub, "assigner", true) == 0)
	{
		new arg[24];
		if(Residence_Tok(cmdline, pos2, arg, sizeof(arg)) == 0)
			return Residence_Msg(playerid, "{FFFFFF}Usage : {FFFF00}/maisonvip assigner <id du joueur>");
		return Residence_Assigner(playerid, strval(arg));
	}
	if(strcmp(sub, "revoquer", true) == 0) return Residence_Revoquer(playerid);
	if(strcmp(sub, "definir", true) == 0) return Residence_Definir(playerid);
	if(strcmp(sub, "definircoffre", true) == 0) return Residence_DefinirCoffre(playerid);

	return Residence_Menu(playerid);
}

// ============================================================================
// INIT
// ============================================================================
stock Residence_Init()
{
	Residence_Load();
	return 1;
}
