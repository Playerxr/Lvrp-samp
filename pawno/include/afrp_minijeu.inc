/*
	afrp_minijeu.inc - Minijeu de competence (barre a arreter dans la zone)

	Remplace les actions "attends et regarde une barre monter" par quelque
	chose que le joueur reussit ou rate. Inspire de FiveM et Rust.

	Une jauge se remplit et se vide en boucle. Une zone verte est tiree au
	hasard sur la barre. Le joueur appuie sur TIR (clic gauche) ou SPRINT
	quand le bord de la jauge est dans la zone. Plusieurs manches, et la zone
	retrecit a chaque manche reussie.

	=== API GENERIQUE ===
	  Mini_Start(playerid, "MaFonctionOk", "MaFonctionRatee", param, manches, vitesse)

	Les deux fonctions doivent etre des publics prenant (playerid, param) :

	  forward MaFonctionOk(playerid, param);
	  public  MaFonctionOk(playerid, param) { ... }

	C'est volontairement generique : les braquages cooperatifs a venir s'en
	serviront pour percer un coffre, et n'importe quelle action longue du
	gamemode peut y passer sans toucher a ce fichier.

	=== POURQUOI UNE JAUGE QUI SE REMPLIT ET NON UN CURSEUR QUI SE DEPLACE ===
	Un textdraw SA-MP ne peut pas etre deplace apres sa creation. Un curseur
	mobile obligerait a detruire et recreer un textdraw 20 fois par seconde.
	Une jauge dont on fait varier la LARGEUR se redimensionne avec
	PlayerTextDrawTextSize, comme les barres de vie du HUD : meme rendu a
	l'ecran, sans destruction/recreation permanente.

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect         -> Mini_OnConnect(playerid)
	- OnPlayerDisconnect      -> Mini_OnDisconnect(playerid)
	- OnPlayerDeath           -> Mini_Abandon(playerid)
	- OnPlayerKeyStateChange  -> Mini_OnKey(playerid, newkeys)
*/

#if defined AFRP_MINIJEU_INCLUDED
	#endinput
#endif
#define AFRP_MINIJEU_INCLUDED

#define MINI_BAR_X1      200.0
#define MINI_BAR_X2      440.0
#define MINI_BAR_Y       286.0
#define MINI_BAR_H       3.2       // hauteur en unites de LetterSize
#define MINI_TITRE_Y     262.0
#define MINI_INFO_Y      300.0
#define MINI_LARGEUR     240       // MINI_BAR_X2 - MINI_BAR_X1, en pixels
#define MINI_TICK_MS     50
#define MINI_MANCHE_MS   7000      // temps maxi pour une manche
#define MINI_ZONE_DEP    56        // largeur de la zone a la 1re manche (px)
#define MINI_ZONE_PAS    10        // retrecissement par manche reussie
#define MINI_ZONE_MIN    18

new PlayerText:miniBg[MAX_PLAYERS];
new PlayerText:miniZone[MAX_PLAYERS];
new PlayerText:miniFill[MAX_PLAYERS];
new PlayerText:miniTitre[MAX_PLAYERS];
new PlayerText:miniInfo[MAX_PLAYERS];

new miniActif[MAX_PLAYERS];
new miniFillPos[MAX_PLAYERS];      // 0..MINI_LARGEUR
new miniDir[MAX_PLAYERS];          // +1 ou -1
new miniVitesse[MAX_PLAYERS];      // pixels par tick
new miniZoneStart[MAX_PLAYERS];
new miniZoneW[MAX_PLAYERS];
new miniManche[MAX_PLAYERS];       // manche en cours (1..N)
new miniManches[MAX_PLAYERS];      // total
new miniDeadline[MAX_PLAYERS];     // GetTickCount limite de la manche
new miniCbOk[MAX_PLAYERS][32];
new miniCbKo[MAX_PLAYERS][32];
new miniParam[MAX_PLAYERS];

new miniTimer = -1;
new miniCount = 0;                 // parties en cours

// ============================================================================
// TEXTDRAWS
// ============================================================================
stock Mini_Create(playerid)
{
	if(miniBg[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	miniBg[playerid] = CreatePlayerTextDraw(playerid, MINI_BAR_X1, MINI_BAR_Y, "_");
	PlayerTextDrawFont(playerid, miniBg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, miniBg[playerid], 0.0, MINI_BAR_H);
	PlayerTextDrawColor(playerid, miniBg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, miniBg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, miniBg[playerid], MINI_BAR_X2, 0.0);
	PlayerTextDrawUseBox(playerid, miniBg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, miniBg[playerid], 0x000000C0);

	miniTitre[playerid] = CreatePlayerTextDraw(playerid, 320.0, MINI_TITRE_Y, " ");
	PlayerTextDrawFont(playerid, miniTitre[playerid], 1);
	PlayerTextDrawAlignment(playerid, miniTitre[playerid], 2);
	PlayerTextDrawLetterSize(playerid, miniTitre[playerid], 0.30, 1.4);
	PlayerTextDrawColor(playerid, miniTitre[playerid], 0xFFD700FF);
	PlayerTextDrawSetOutline(playerid, miniTitre[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, miniTitre[playerid], 0x000000FF);
	PlayerTextDrawSetProportional(playerid, miniTitre[playerid], 1);

	miniInfo[playerid] = CreatePlayerTextDraw(playerid, 320.0, MINI_INFO_Y, " ");
	PlayerTextDrawFont(playerid, miniInfo[playerid], 1);
	PlayerTextDrawAlignment(playerid, miniInfo[playerid], 2);
	PlayerTextDrawLetterSize(playerid, miniInfo[playerid], 0.21, 1.0);
	PlayerTextDrawColor(playerid, miniInfo[playerid], 0xFFFFFFFF);
	PlayerTextDrawSetOutline(playerid, miniInfo[playerid], 1);
	PlayerTextDrawBackgroundColor(playerid, miniInfo[playerid], 0x000000FF);
	PlayerTextDrawSetProportional(playerid, miniInfo[playerid], 1);
	return 1;
}

// La zone verte change de place a chaque manche. Un textdraw ne pouvant pas
// etre deplace, on ne recree que celui-la, et une seule fois par manche.
stock Mini_ZoneDraw(playerid)
{
	if(miniZone[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, miniZone[playerid]);
		miniZone[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	new Float:x1 = MINI_BAR_X1 + float(miniZoneStart[playerid]);
	new Float:x2 = x1 + float(miniZoneW[playerid]);

	miniZone[playerid] = CreatePlayerTextDraw(playerid, x1, MINI_BAR_Y, "_");
	PlayerTextDrawFont(playerid, miniZone[playerid], 1);
	PlayerTextDrawLetterSize(playerid, miniZone[playerid], 0.0, MINI_BAR_H);
	PlayerTextDrawColor(playerid, miniZone[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, miniZone[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, miniZone[playerid], x2, 0.0);
	PlayerTextDrawUseBox(playerid, miniZone[playerid], 1);
	PlayerTextDrawBoxColor(playerid, miniZone[playerid], 0x00C85080);
	PlayerTextDrawShow(playerid, miniZone[playerid]);

	// La jauge est recreee juste apres pour rester DEVANT la zone : SA-MP
	// dessine les textdraws d'un joueur dans leur ordre de creation.
	if(miniFill[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, miniFill[playerid]);
		miniFill[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	miniFill[playerid] = CreatePlayerTextDraw(playerid, MINI_BAR_X1, MINI_BAR_Y, "_");
	PlayerTextDrawFont(playerid, miniFill[playerid], 1);
	PlayerTextDrawLetterSize(playerid, miniFill[playerid], 0.0, MINI_BAR_H);
	PlayerTextDrawColor(playerid, miniFill[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, miniFill[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, miniFill[playerid], MINI_BAR_X1 + 2.0, 0.0);
	PlayerTextDrawUseBox(playerid, miniFill[playerid], 1);
	PlayerTextDrawBoxColor(playerid, miniFill[playerid], 0xFFFFFFFF);
	PlayerTextDrawShow(playerid, miniFill[playerid]);
	return 1;
}

stock Mini_Hide(playerid)
{
	if(miniBg[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawHide(playerid, miniBg[playerid]);
		PlayerTextDrawHide(playerid, miniTitre[playerid]);
		PlayerTextDrawHide(playerid, miniInfo[playerid]);
	}
	if(miniZone[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, miniZone[playerid]);
		miniZone[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	if(miniFill[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, miniFill[playerid]);
		miniFill[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	return 1;
}

// ============================================================================
// DEROULEMENT
// ============================================================================
stock Mini_NouvelleManche(playerid)
{
	new w = MINI_ZONE_DEP - (miniManche[playerid] - 1) * MINI_ZONE_PAS;
	if(w < MINI_ZONE_MIN) w = MINI_ZONE_MIN;
	miniZoneW[playerid]     = w;
	miniZoneStart[playerid] = random(MINI_LARGEUR - w);
	miniFillPos[playerid]   = 0;
	miniDir[playerid]       = 1;
	miniDeadline[playerid]  = GetTickCount() + MINI_MANCHE_MS;

	Mini_ZoneDraw(playerid);

	new s[64];
	format(s, sizeof(s), "Manche %d/%d  -  TIR ou SPRINT dans la zone verte", miniManche[playerid], miniManches[playerid]);
	PlayerTextDrawSetString(playerid, miniInfo[playerid], s);
	PlayerTextDrawShow(playerid, miniBg[playerid]);
	PlayerTextDrawShow(playerid, miniTitre[playerid]);
	PlayerTextDrawShow(playerid, miniInfo[playerid]);
	return 1;
}

stock Mini_Termine(playerid, reussi)
{
	if(miniActif[playerid] != 1) return 0;

	new cb[32];
	if(reussi) format(cb, sizeof(cb), "%s", miniCbOk[playerid]);
	else       format(cb, sizeof(cb), "%s", miniCbKo[playerid]);
	new param = miniParam[playerid];

	miniActif[playerid] = 0;
	miniCount--;
	if(miniCount <= 0 && miniTimer != -1) { KillTimer(miniTimer); miniTimer = -1; miniCount = 0; }
	Mini_Hide(playerid);

	// Le rappel est fait EN DERNIER : il peut relancer un minijeu, et l'etat
	// doit deja etre propre a ce moment-la.
	if(strlen(cb) > 0) CallLocalFunction(cb, "ii", playerid, param);
	return 1;
}

forward Mini_Tick();
public Mini_Tick()
{
	// On sort des que les miniCount parties en cours ont ete traitees : sans ce
	// compteur, le tick balayait les 1000 slots joueur 20 fois par seconde
	// meme quand une seule personne jouait.
	new traites = 0;
	for(new i = 0; i < MAX_PLAYERS && traites < miniCount; i++)
	{
		if(miniActif[i] != 1) continue;
		traites++;
		if(!IsPlayerConnected(i)) { miniActif[i] = 0; miniCount--; continue; }

		if(GetTickCount() >= miniDeadline[i])
		{
			SendClientMessage(i, 0xFF4444FF, "{FF4444} Echec  Tu as laisse passer le temps.");
			Mini_Termine(i, 0);
			continue;
		}

		miniFillPos[i] += miniDir[i] * miniVitesse[i];
		if(miniFillPos[i] >= MINI_LARGEUR) { miniFillPos[i] = MINI_LARGEUR; miniDir[i] = -1; }
		else if(miniFillPos[i] <= 0)       { miniFillPos[i] = 0;            miniDir[i] = 1;  }

		// Redimensionnement plutot que deplacement : voir l'en-tete du fichier.
		new Float:x2 = MINI_BAR_X1 + float(miniFillPos[i]);
		if(x2 < MINI_BAR_X1 + 2.0) x2 = MINI_BAR_X1 + 2.0;
		PlayerTextDrawTextSize(i, miniFill[i], x2, 0.0);
		PlayerTextDrawShow(i, miniFill[i]);
	}
	if(miniCount <= 0 && miniTimer != -1) { KillTimer(miniTimer); miniTimer = -1; miniCount = 0; }
	return 1;
}

// ============================================================================
// TOUCHE
// ============================================================================
// Renvoie 1 si la touche a ete consommee par un minijeu en cours.
stock Mini_OnKey(playerid, newkeys)
{
	if(miniActif[playerid] != 1) return 0;
	if(!(newkeys & KEY_FIRE) && !(newkeys & KEY_SPRINT)) return 0;

	new fin = miniZoneStart[playerid] + miniZoneW[playerid];
	new bool:dansZone = (miniFillPos[playerid] >= miniZoneStart[playerid] && miniFillPos[playerid] <= fin);

	if(!dansZone)
	{
		SendClientMessage(playerid, 0xFF4444FF, "{FF4444} Rate  Tu as manque la zone.");
		PlayerPlaySound(playerid, 1085, 0.0, 0.0, 0.0);
		Mini_Termine(playerid, 0);
		return 1;
	}

	PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);
	if(miniManche[playerid] >= miniManches[playerid])
	{
		SendClientMessage(playerid, 0x00FF00FF, "{00FF00} Reussi  Toutes les manches sont passees.");
		Mini_Termine(playerid, 1);
		return 1;
	}

	miniManche[playerid]++;
	new s[80];
	format(s, sizeof(s), "{00FF00} Bien  Manche %d/%d.", miniManche[playerid], miniManches[playerid]);
	SendClientMessage(playerid, 0x00FF00FF, s);
	Mini_NouvelleManche(playerid);
	return 1;
}

// ============================================================================
// DEMARRAGE
// ============================================================================
stock Mini_EnCours(playerid)
{
	return (miniActif[playerid] == 1);
}

stock Mini_Start(playerid, onOk[], onKo[], param = 0, manches = 3, vitesse = 4, titre[] = "~y~CONCENTRATION")
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(miniActif[playerid] == 1) return 0;
	if(manches < 1) manches = 1;
	if(manches > 6) manches = 6;
	if(vitesse < 2) vitesse = 2;
	if(vitesse > 12) vitesse = 12;

	Mini_Create(playerid);
	format(miniCbOk[playerid], 32, "%s", onOk);
	format(miniCbKo[playerid], 32, "%s", onKo);
	miniParam[playerid]   = param;
	miniManches[playerid] = manches;
	miniManche[playerid]  = 1;
	miniVitesse[playerid] = vitesse;
	miniActif[playerid]   = 1;
	miniCount++;

	PlayerTextDrawSetString(playerid, miniTitre[playerid], titre);
	Mini_NouvelleManche(playerid);

	if(miniTimer == -1) miniTimer = SetTimer("Mini_Tick", MINI_TICK_MS, true);
	return 1;
}

// Interrompt sans appeler le rappel d'echec (mort, deconnexion, annulation).
stock Mini_Abandon(playerid)
{
	if(miniActif[playerid] != 1) return 0;
	miniActif[playerid] = 0;
	miniCount--;
	if(miniCount <= 0 && miniTimer != -1) { KillTimer(miniTimer); miniTimer = -1; miniCount = 0; }
	Mini_Hide(playerid);
	return 1;
}

// ============================================================================
// HOOKS
// ============================================================================
stock Mini_OnConnect(playerid)
{
	miniBg[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	miniZone[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	miniFill[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	miniTitre[playerid] = PlayerText:INVALID_TEXT_DRAW;
	miniInfo[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	miniActif[playerid] = 0;
	return 1;
}

stock Mini_OnDisconnect(playerid)
{
	Mini_Abandon(playerid);
	if(miniBg[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, miniBg[playerid]);
		PlayerTextDrawDestroy(playerid, miniTitre[playerid]);
		PlayerTextDrawDestroy(playerid, miniInfo[playerid]);
	}
	Mini_OnConnect(playerid);
	return 1;
}

// ============================================================================
// COMMANDE DE TEST
// ============================================================================
forward Mini_TestOk(playerid, param);
public Mini_TestOk(playerid, param)
{
	#pragma unused param
	SendClientMessage(playerid, 0x00FF00FF, "{00FF00} Minijeu  Test reussi. C'est ce rappel qui serait branche sur une vraie action.");
	return 1;
}

forward Mini_TestKo(playerid, param);
public Mini_TestKo(playerid, param)
{
	#pragma unused param
	SendClientMessage(playerid, 0xFF4444FF, "{FF4444} Minijeu  Test rate.");
	return 1;
}

stock Mini_CmdTest(playerid)
{
	if(Mini_EnCours(playerid))
		return SendClientMessage(playerid, 0xFF4444FF, "{FF4444}Tu es deja dans un minijeu."), 1;
	Mini_Start(playerid, "Mini_TestOk", "Mini_TestKo", 0, 3, 4, "~y~TEST DU MINIJEU");
	return 1;
}
