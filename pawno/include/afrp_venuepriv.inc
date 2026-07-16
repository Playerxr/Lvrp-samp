/*
	afrp_venuepriv.inc - Lieux prives (restaurants, clubs, boutiques...)

	Un admin transforme un batiment public existant en "lieu prive"
	attribue a un joueur (le proprio) : ce dernier peut ferm/ouvr sa porte,
	mettre un code PIN, et fixer un prix d'entree. Les clients passent
	par un pickup a l'entree, un dialog leur propose de payer ou de saisir
	le code PIN. L'argent va directement dans la caisse du proprio.

	Regles :
	  - PIN connu = entree GRATUITE (VIP / membres / staff / amis).
	  - Sans PIN = paiement du prix d'entree (si venue ouvert).
	  - Ferme = personne n'entre sauf proprio + gens qui ont le PIN.
	  - Proprio entre toujours sans payer.

	Persistance : scriptfiles/venues_prives.cfg (une ligne par venue).

	Dialogs reserves : 9960 (menu entree), 9961 (input PIN client),
	                   9962 (input PIN owner set), 9963 (input prix owner set).
*/

#if defined AFRP_VENUEPRIV_INCLUDED
	#endinput
#endif
#define AFRP_VENUEPRIV_INCLUDED

#define VP_MAX_VENUE          50
#define VP_FILE               "venues_prives.cfg"
#define VP_ENTRY_RADIUS       3.0
#define VP_LABEL_DIST         15.0
#define VP_BARRIER_RADIUS     1.8      // rayon de la sphere qui repousse
#define VP_BARRIER_OFFSET     1.5      // distance en avant de l'entree
#define VP_BARRIER_PUSHBACK   3.5      // recul en metres si on passe sans payer
#define VP_AUTH_SEC           4        // grace period apres paiement legit
#define VP_DLG_ENTRY          9960
#define VP_DLG_PIN_CLIENT     9961
#define VP_DLG_PIN_SET        9962
#define VP_DLG_PRIX_SET       9963

enum e_Venue
{
	vpUsed,
	vpName[64],
	vpOwnerSqlid,
	Float:vpEntryX, Float:vpEntryY, Float:vpEntryZ,
	Float:vpExitX,  Float:vpExitY,  Float:vpExitZ,
	vpExitInt,
	vpExitVW,
	vpEntryFee,
	vpPin[8],           // "" (vide) = pas de PIN
	vpLocked,           // 0 ouvert, 1 ferme
	vpRevenue,          // total encaisse (info uniquement)
	Float:vpBarrierX, Float:vpBarrierY, Float:vpBarrierZ,   // centre de la sphere barriere
	STREAMER_TAG_AREA:vpBarrierArea,                        // handle streamer, -1 si pas cree
	vpEnterprise[64]    // "" (vide) = revenus au proprio, sinon vers caisse entreprise
};
new venues[VP_MAX_VENUE][e_Venue];
new venuePickup[VP_MAX_VENUE];
new venueExitPickup[VP_MAX_VENUE];
new Text3D:venueLabel[VP_MAX_VENUE];
new Text3D:venueExitLabel[VP_MAX_VENUE];
new venue_PendingVid[MAX_PLAYERS];    // venue selectionne pour dialog en cours
new venue_AuthUntil[MAX_PLAYERS];     // gettime() jusqu'auquel le joueur est authorise a franchir la barriere
new venue_Count = 0;

stock Venue_Init()
{
	for(new i = 0; i < VP_MAX_VENUE; i++)
	{
		venues[i][vpUsed]         = 0;
		venuePickup[i]            = -1;
		venueExitPickup[i]        = -1;
		venueLabel[i]             = Text3D:INVALID_3DTEXT_ID;
		venueExitLabel[i]         = Text3D:INVALID_3DTEXT_ID;
		venues[i][vpBarrierArea]  = STREAMER_TAG_AREA:-1;
	}
	for(new p = 0; p < MAX_PLAYERS; p++) venue_AuthUntil[p] = 0;
	Venue_Load();
	return 1;
}

// Extrait le prochain champ (delim = '|') depuis line[i] vers dest.
// Fait avancer i au caractere apres le delimiteur. Retourne la longueur ecrite.
stock Venue_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
	{
		dest[p++] = line[i++];
	}
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock Venue_Load()
{
	if(!fexist(VP_FILE)) return 0;
	new File:f = fopen(VP_FILE, io_read);
	if(!f) return 0;
	new line[512];
	new tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 5) continue;
		// Format : name|owner|ex|ey|ez|ix|iy|iz|int|vw|fee|pin|locked|revenue
		if(venue_Count >= VP_MAX_VENUE) break;
		new vid = venue_Count;
		new i = 0;

		Venue_ReadField(line, i, venues[vid][vpName], 64, 63);

		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpOwnerSqlid] = strval(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpEntryX]     = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpEntryY]     = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpEntryZ]     = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpExitX]      = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpExitY]      = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpExitZ]      = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpExitInt]    = strval(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpExitVW]     = strval(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpEntryFee]   = strval(tmpBuf);

		Venue_ReadField(line, i, venues[vid][vpPin], 8, 7);

		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpLocked]  = strval(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpRevenue] = strval(tmpBuf);

		// Champs barriere (peuvent etre absents des vieux fichiers)
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpBarrierX] = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpBarrierY] = floatstr(tmpBuf);
		Venue_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); venues[vid][vpBarrierZ] = floatstr(tmpBuf);

		// Nom entreprise (peut etre absent des vieux fichiers)
		Venue_ReadField(line, i, venues[vid][vpEnterprise], 64, 63);

		// Retro-compat : barriere pas dans le fichier => on la place a l'entree + 1.5m Y
		if(venues[vid][vpBarrierX] == 0.0 && venues[vid][vpBarrierY] == 0.0 && venues[vid][vpBarrierZ] == 0.0)
		{
			venues[vid][vpBarrierX] = venues[vid][vpEntryX];
			venues[vid][vpBarrierY] = venues[vid][vpEntryY] + VP_BARRIER_OFFSET;
			venues[vid][vpBarrierZ] = venues[vid][vpEntryZ];
		}

		venues[vid][vpBarrierArea] = STREAMER_TAG_AREA:-1;
		venues[vid][vpUsed] = 1;
		Venue_CreateVisuals(vid);
		venue_Count++;
	}
	fclose(f);
	printf("[Venue] %d lieux prives charges depuis %s", venue_Count, VP_FILE);
	return venue_Count;
}

stock Venue_Save()
{
	if(fexist(VP_FILE)) fremove(VP_FILE);
	new File:f = fopen(VP_FILE, io_write);
	if(!f) return 0;
	new line[640];
	for(new i = 0; i < VP_MAX_VENUE; i++)
	{
		if(venues[i][vpUsed] != 1) continue;
		format(line, sizeof(line), "%s|%d|%.4f|%.4f|%.4f|%.4f|%.4f|%.4f|%d|%d|%d|%s|%d|%d|%.4f|%.4f|%.4f|%s\n",
			venues[i][vpName],
			venues[i][vpOwnerSqlid],
			venues[i][vpEntryX], venues[i][vpEntryY], venues[i][vpEntryZ],
			venues[i][vpExitX],  venues[i][vpExitY],  venues[i][vpExitZ],
			venues[i][vpExitInt], venues[i][vpExitVW],
			venues[i][vpEntryFee],
			venues[i][vpPin],
			venues[i][vpLocked],
			venues[i][vpRevenue],
			venues[i][vpBarrierX], venues[i][vpBarrierY], venues[i][vpBarrierZ],
			venues[i][vpEnterprise]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Venue_UpdateLabel(vid)
{
	if(venues[vid][vpUsed] != 1) return 0;
	new labelStr[192];
	new stateColor[10], stateTxt[24];
	if(venues[vid][vpLocked] == 1) { format(stateColor, sizeof(stateColor), "FF3333"); format(stateTxt, sizeof(stateTxt), "FERME"); }
	else                            { format(stateColor, sizeof(stateColor), "42FF66"); format(stateTxt, sizeof(stateTxt), "OUVERT"); }

	new feeStr[32];
	if(venues[vid][vpEntryFee] > 0) format(feeStr, sizeof(feeStr), "Entree : %d$", venues[vid][vpEntryFee]);
	else                             format(feeStr, sizeof(feeStr), "Entree gratuite");

	format(labelStr, sizeof(labelStr), "{42AAFF}%s\n{%s}[%s]{FFFFFF}\n%s\n{CCCCCC}/venue pour entrer",
		venues[vid][vpName], stateColor, stateTxt, feeStr);

	if(_:venueLabel[vid] != _:INVALID_3DTEXT_ID)
	{
		Update3DTextLabelText(venueLabel[vid], 0xFFFFFFFF, labelStr);
	}
	return 1;
}

stock Venue_CreateVisuals(vid)
{
	if(venues[vid][vpUsed] != 1) return 0;

	if(venuePickup[vid] == -1)
		venuePickup[vid] = CreatePickup(1272, 1, venues[vid][vpEntryX], venues[vid][vpEntryY], venues[vid][vpEntryZ], 0);

	if(_:venueLabel[vid] == _:INVALID_3DTEXT_ID)
	{
		venueLabel[vid] = Create3DTextLabel("...", 0xFFFFFFFF,
			venues[vid][vpEntryX], venues[vid][vpEntryY], venues[vid][vpEntryZ] + 0.8,
			VP_LABEL_DIST, 0, 0);
	}
	Venue_UpdateLabel(vid);

	// Sphere barriere : bloque physiquement les gens qui essayent d'entrer
	// sans avoir paye. Position juste en avant de la porte (fixee au /venue creer).
	if(_:venues[vid][vpBarrierArea] == _:STREAMER_TAG_AREA:-1
		&& !(venues[vid][vpBarrierX] == 0.0 && venues[vid][vpBarrierY] == 0.0 && venues[vid][vpBarrierZ] == 0.0))
	{
		venues[vid][vpBarrierArea] = CreateDynamicSphere(
			venues[vid][vpBarrierX], venues[vid][vpBarrierY], venues[vid][vpBarrierZ],
			VP_BARRIER_RADIUS, 0, 0, -1, 0);
	}

	// Pickup + label a l'interieur (sortie) - seulement si un interieur est defini
	if(!(venues[vid][vpExitX] == 0.0 && venues[vid][vpExitY] == 0.0 && venues[vid][vpExitZ] == 0.0))
	{
		if(venueExitPickup[vid] == -1)
			venueExitPickup[vid] = CreatePickup(1272, 1, venues[vid][vpExitX], venues[vid][vpExitY], venues[vid][vpExitZ], venues[vid][vpExitVW]);
		if(_:venueExitLabel[vid] == _:INVALID_3DTEXT_ID)
		{
			venueExitLabel[vid] = Create3DTextLabel("{42AAFF}Sortie\n{CCCCCC}/venue sortir",
				0xFFFFFFFF,
				venues[vid][vpExitX], venues[vid][vpExitY], venues[vid][vpExitZ] + 0.8,
				10.0, venues[vid][vpExitVW], 0);
		}
	}
	return 1;
}

stock Venue_DestroyVisuals(vid)
{
	if(venuePickup[vid] != -1)      { DestroyPickup(venuePickup[vid]);       venuePickup[vid] = -1; }
	if(venueExitPickup[vid] != -1)  { DestroyPickup(venueExitPickup[vid]);   venueExitPickup[vid] = -1; }
	if(_:venueLabel[vid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(venueLabel[vid]);
		venueLabel[vid] = Text3D:INVALID_3DTEXT_ID;
	}
	if(_:venueExitLabel[vid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(venueExitLabel[vid]);
		venueExitLabel[vid] = Text3D:INVALID_3DTEXT_ID;
	}
	if(_:venues[vid][vpBarrierArea] != _:STREAMER_TAG_AREA:-1)
	{
		DestroyDynamicArea(venues[vid][vpBarrierArea]);
		venues[vid][vpBarrierArea] = STREAMER_TAG_AREA:-1;
	}
	return 1;
}

stock Venue_GetNearestEntry(playerid)
{
	new Float:bestDist = VP_ENTRY_RADIUS;
	new best = -1;
	for(new i = 0; i < VP_MAX_VENUE; i++)
	{
		if(venues[i][vpUsed] != 1) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, venues[i][vpEntryX], venues[i][vpEntryY], venues[i][vpEntryZ]);
		if(d < bestDist) { bestDist = d; best = i; }
	}
	return best;
}

stock Venue_GetInside(playerid)
{
	// Trouve le venue dans lequel le joueur se tient (< 5m de exit et meme VW)
	new pvw = GetPlayerVirtualWorld(playerid);
	new pint = GetPlayerInterior(playerid);
	for(new i = 0; i < VP_MAX_VENUE; i++)
	{
		if(venues[i][vpUsed] != 1) continue;
		if(venues[i][vpExitVW] != pvw) continue;
		if(venues[i][vpExitInt] != pint) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, venues[i][vpExitX], venues[i][vpExitY], venues[i][vpExitZ]);
		if(d < 20.0) return i;
	}
	return -1;
}

stock Venue_Enter(playerid, vid)
{
	if(vid < 0 || vid >= VP_MAX_VENUE) return 0;
	if(venues[vid][vpUsed] != 1) return 0;
	if(venues[vid][vpExitX] == 0.0 && venues[vid][vpExitY] == 0.0 && venues[vid][vpExitZ] == 0.0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} L'interieur n'est pas encore defini par l'admin.");
	// Grace period : le joueur peut franchir la barriere le temps du teleport.
	venue_AuthUntil[playerid] = gettime() + VP_AUTH_SEC;
	SetPlayerInterior(playerid, venues[vid][vpExitInt]);
	SetPlayerVirtualWorld(playerid, venues[vid][vpExitVW]);
	SafeSetPlayerPos(playerid, venues[vid][vpExitX], venues[vid][vpExitY], venues[vid][vpExitZ]);
	new msg[128];
	format(msg, sizeof(msg), "{42AAFF} Lieu {FFFFFF} Bienvenue chez {FFFFB2}%s{FFFFFF}.", venues[vid][vpName]);
	msg_Client(playerid, COLOR_WHITE, msg);
	return 1;
}

// Handler barriere : appele depuis OnPlayerEnterDynamicArea. Repousse les
// joueurs qui essaient d'entrer dans le batiment sans etre autorises.
// Retourne 1 si un venue a matche l'areaid, 0 sinon (pour ne pas court-circuiter
// les autres handlers d'aires dynamiques).
stock Venue_OnPlayerEnterBarrier(playerid, STREAMER_TAG_AREA:areaid)
{
	for(new vid = 0; vid < VP_MAX_VENUE; vid++)
	{
		if(venues[vid][vpUsed] != 1) continue;
		if(_:venues[vid][vpBarrierArea] != _:areaid) continue;

		// Grace period : le joueur vient de payer, il traverse la barriere le temps
		// de son teleport vers l'interieur - on le laisse passer.
		if(venue_AuthUntil[playerid] >= gettime()) return 1;

		// Le proprio entre toujours librement.
		if(venues[vid][vpOwnerSqlid] == PlayerInfo[playerid][pSQLID]) return 1;

		// Admin 5+ peut passer (pour verifier les lieux)
		if(PlayerInfo[playerid][pAdmin] >= 5) return 1;

		// Repousse le joueur vers l'entree (le pickup /venue)
		new Float:px, Float:py, Float:pz;
		GetPlayerPos(playerid, px, py, pz);
		new Float:dx = px - venues[vid][vpBarrierX];
		new Float:dy = py - venues[vid][vpBarrierY];
		new Float:dist = floatsqroot(dx*dx + dy*dy);
		if(dist < 0.01) { dx = 0.0; dy = -1.0; dist = 1.0; }  // pile au centre : recul arbitraire vers -Y
		new Float:pushX = venues[vid][vpBarrierX] + (dx / dist) * VP_BARRIER_PUSHBACK;
		new Float:pushY = venues[vid][vpBarrierY] + (dy / dist) * VP_BARRIER_PUSHBACK;
		SafeSetPlayerPos(playerid, pushX, pushY, venues[vid][vpEntryZ] + 0.2);

		new msg[192];
		if(venues[vid][vpLocked] == 1)
			format(msg, sizeof(msg), "{FF3333} Porte {FFFFFF} {FFFFB2}%s{FFFFFF} est ferme au public.", venues[vid][vpName]);
		else
			format(msg, sizeof(msg), "{FF3333} Porte {FFFFFF} Entree payante ({FFFFB2}%d${FFFFFF}). Tape {FFFFB2}/venue{FFFFFF} pour entrer.", venues[vid][vpEntryFee]);
		msg_Client(playerid, COLOR_INFO, msg);
		return 1;
	}
	return 0;
}

stock Venue_ChargeAndEnter(playerid, vid)
{
	new fee = venues[vid][vpEntryFee];
	if(fee > 0)
	{
		if(PlayerInfo[playerid][pCash] < fee)
		{
			new msg[128];
			format(msg, sizeof(msg), "{CF9756} Lieu {FFFFFF} Il te faut {FFFFB2}%d${FFFFFF} pour entrer.", fee);
			return msg_Client(playerid, COLOR_INFO, msg);
		}
		SafeGivePlayerMoney(playerid, -fee, "Entree lieu prive");
		venues[vid][vpRevenue] += fee;

		// Si le venue est lie a une entreprise : le paiement va dans la caisse.
		if(strlen(venues[vid][vpEnterprise]) > 0 && Ent_CreditByName(venues[vid][vpEnterprise], fee))
		{
			// Notifie le proprio si connecte (info seulement, argent va en caisse)
			for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
			{
				if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
				if(PlayerInfo[p][pSQLID] != venues[vid][vpOwnerSqlid]) continue;
				new notif[160];
				format(notif, sizeof(notif), "{42AAFF} Lieu {FFFFFF} Client chez {FFFFB2}%s{FFFFFF} : +%d$ dans la caisse {FFFFB2}%s{FFFFFF}.", venues[vid][vpName], fee, venues[vid][vpEnterprise]);
				msg_Client(p, COLOR_WHITE, notif);
				break;
			}
		}
		else
		{
			// Pas d'entreprise ou nom introuvable : argent dans la poche du proprio (comportement d'origine)
			for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
			{
				if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
				if(PlayerInfo[p][pSQLID] != venues[vid][vpOwnerSqlid]) continue;
				SafeGivePlayerMoney(p, fee, "Recette lieu prive");
				new notif[128];
				format(notif, sizeof(notif), "{42AAFF} Lieu {FFFFFF} Un client est entre chez toi ({FFFFB2}%s{FFFFFF}) : +%d$.", venues[vid][vpName], fee);
				msg_Client(p, COLOR_WHITE, notif);
				break;
			}
		}
		Venue_Save();
	}
	return Venue_Enter(playerid, vid);
}

// Ouvre le dialog approprie quand un joueur tape /venue pres d'une entree
stock Venue_TryEnter(playerid, vid)
{
	if(vid < 0 || vid >= VP_MAX_VENUE || venues[vid][vpUsed] != 1) return 0;

	// Proprio entre toujours
	if(venues[vid][vpOwnerSqlid] == PlayerInfo[playerid][pSQLID])
		return Venue_Enter(playerid, vid);

	venue_PendingVid[playerid] = vid;

	// Ferme : uniquement PIN (si defini) ou refus
	if(venues[vid][vpLocked] == 1)
	{
		if(strlen(venues[vid][vpPin]) == 0)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} C'est ferme.");
		return ShowPlayerDialog(playerid, VP_DLG_PIN_CLIENT, DIALOG_STYLE_INPUT,
			"{FFAA00}Code d'entree",
			"{FFFFFF}Ce lieu est ferme au public.\nEntrez le code PIN si vous en avez un :",
			"Entrer", "Annuler");
	}

	// Ouvert : proposer PIN (si defini) OU payer
	new body[256];
	if(strlen(venues[vid][vpPin]) > 0)
	{
		format(body, sizeof(body),
			"{FFFFFF}Bienvenue chez {FFFFB2}%s{FFFFFF}.\nPrix d'entree : {FFFFB2}%d${FFFFFF}\n\nSi tu as le code PIN, tu entres gratuitement.\nSinon, appuie sur Payer.",
			venues[vid][vpName], venues[vid][vpEntryFee]);
		return ShowPlayerDialog(playerid, VP_DLG_ENTRY, DIALOG_STYLE_MSGBOX,
			venues[vid][vpName], body, "Payer", "PIN");
	}
	// Pas de PIN : juste payer/annuler
	format(body, sizeof(body),
		"{FFFFFF}Bienvenue chez {FFFFB2}%s{FFFFFF}.\nPrix d'entree : {FFFFB2}%d${FFFFFF}\n\nSouhaitez-vous entrer ?",
		venues[vid][vpName], venues[vid][vpEntryFee]);
	return ShowPlayerDialog(playerid, VP_DLG_ENTRY, DIALOG_STYLE_MSGBOX,
		venues[vid][vpName], body, "Payer", "Non");
}

// Dispatcher dialog (a appeler depuis OnDialogResponse pour dialogid 9960-9963)
stock Venue_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	#pragma unused listitem
	new vid = venue_PendingVid[playerid];
	if(vid < 0 || vid >= VP_MAX_VENUE || venues[vid][vpUsed] != 1) return 1;

	if(dialogid == VP_DLG_ENTRY)
	{
		if(response == 1)
		{
			// Payer
			return Venue_ChargeAndEnter(playerid, vid);
		}
		else
		{
			// Bouton droit = PIN si dispo, sinon annuler
			if(strlen(venues[vid][vpPin]) > 0)
			{
				return ShowPlayerDialog(playerid, VP_DLG_PIN_CLIENT, DIALOG_STYLE_INPUT,
					"{FFAA00}Code d'entree",
					"{FFFFFF}Entre le code PIN :",
					"Entrer", "Annuler");
			}
			return 1;
		}
	}

	if(dialogid == VP_DLG_PIN_CLIENT)
	{
		if(!response) return 1;
		if(strcmp(inputtext, venues[vid][vpPin], true) == 0)
			return Venue_Enter(playerid, vid);
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Code PIN incorrect.");
	}

	if(dialogid == VP_DLG_PIN_SET)
	{
		if(!response) return 1;
		if(strlen(inputtext) == 0 || strcmp(inputtext, "off", true) == 0)
		{
			venues[vid][vpPin][0] = 0;
			Venue_Save();
			return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Code PIN supprime.");
		}
		if(strlen(inputtext) < 3 || strlen(inputtext) > 6)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Le PIN doit faire 3 a 6 caracteres.");
		new p = 0;
		while(p < 7 && inputtext[p]) { venues[vid][vpPin][p] = inputtext[p]; p++; }
		venues[vid][vpPin][p] = 0;
		Venue_Save();
		new msg[96];
		format(msg, sizeof(msg), "{42AAFF} Lieu {FFFFFF} Nouveau code PIN : {FFFFB2}%s{FFFFFF}.", venues[vid][vpPin]);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}

	if(dialogid == VP_DLG_PRIX_SET)
	{
		if(!response) return 1;
		new newFee = strval(inputtext);
		if(newFee < 0)   newFee = 0;
		if(newFee > 500000) newFee = 500000;
		venues[vid][vpEntryFee] = newFee;
		Venue_UpdateLabel(vid);
		Venue_Save();
		new msg[96];
		format(msg, sizeof(msg), "{42AAFF} Lieu {FFFFFF} Nouveau prix d'entree : {FFFFB2}%d${FFFFFF}.", newFee);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}
	return 1;
}

stock Venue_Cmd_Sortir(playerid)
{
	new vid = Venue_GetInside(playerid);
	if(vid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas dans un lieu prive.");
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
	SafeSetPlayerPos(playerid, venues[vid][vpEntryX], venues[vid][vpEntryY], venues[vid][vpEntryZ] + 0.5);
	return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Vous quittez l'etablissement.");
}

// Dispatcher principal /venue [sous-cmd]
stock Venue_Cmd(playerid, params[])
{
	// Sans argument : tenter d'entrer si a une entree, sinon aide
	if(!strlen(params))
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid != -1) return Venue_TryEnter(playerid, vid);
		msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Approche-toi d'une entree (icone info bleue) puis tape /venue.");
		msg_Client(playerid, COLOR_WHITE, "{FFFFFF}/venue sortir, /venue infos, /venue code, /venue prix, /venue ferme, /venue ouvre");
		if(PlayerInfo[playerid][pAdmin] >= 3)
			msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Admin: /venue creer <nom>, /venue interieur, /venue proprio <joueur>, /venue supprimer, /venue lister");
		return 1;
	}

	// Extrait la sous-commande et le reste
	new sub[16], rest[128];
	new i = 0, p = 0;
	while(p < 15 && params[i] && params[i] != ' ') { sub[p++] = params[i++]; }
	sub[p] = 0;
	while(params[i] == ' ') i++;
	p = 0;
	while(p < 127 && params[i]) { rest[p++] = params[i++]; }
	rest[p] = 0;

	if(strcmp(sub, "sortir", true) == 0) return Venue_Cmd_Sortir(playerid);

	if(strcmp(sub, "infos", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) vid = Venue_GetInside(playerid);
		if(vid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Aucun lieu a proximite.");
		new body[300], ownerName[MAX_PLAYER_NAME]; strcat(ownerName, "hors ligne");
		for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
		{
			if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
			if(PlayerInfo[pp][pSQLID] == venues[vid][vpOwnerSqlid])
			{
				GetPlayerName(pp, ownerName, sizeof(ownerName));
				break;
			}
		}
		new pinTxt[16]; if(strlen(venues[vid][vpPin]) > 0) format(pinTxt, sizeof(pinTxt), "OUI"); else format(pinTxt, sizeof(pinTxt), "NON");
		new stateTxt[16]; if(venues[vid][vpLocked] == 1) format(stateTxt, sizeof(stateTxt), "FERME"); else format(stateTxt, sizeof(stateTxt), "OUVERT");
		format(body, sizeof(body),
			"{FFFFFF}Nom : %s\nProprio : %s (SQLID %d)\nEtat : %s\nPrix d'entree : %d$\nPIN defini : %s\nRecettes cumulees : %d$",
			venues[vid][vpName], ownerName, venues[vid][vpOwnerSqlid],
			stateTxt, venues[vid][vpEntryFee], pinTxt, venues[vid][vpRevenue]);
		return ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_MSGBOX, "Lieu prive", body, "OK", "");
	}

	// Commandes proprio : doivent etre a l'entree ET etre proprio
	if(strcmp(sub, "code", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de ton entree.");
		if(venues[vid][vpOwnerSqlid] != PlayerInfo[playerid][pSQLID])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas le proprio.");
		venue_PendingVid[playerid] = vid;
		return ShowPlayerDialog(playerid, VP_DLG_PIN_SET, DIALOG_STYLE_INPUT,
			"{FFAA00}Code PIN",
			"{FFFFFF}Nouveau code PIN (3 a 6 caracteres).\nEcris 'off' pour supprimer.",
			"Valider", "Annuler");
	}

	if(strcmp(sub, "prix", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de ton entree.");
		if(venues[vid][vpOwnerSqlid] != PlayerInfo[playerid][pSQLID])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas le proprio.");
		venue_PendingVid[playerid] = vid;
		return ShowPlayerDialog(playerid, VP_DLG_PRIX_SET, DIALOG_STYLE_INPUT,
			"{FFAA00}Prix d'entree",
			"{FFFFFF}Nouveau prix d'entree (0 a 500000$) :",
			"Valider", "Annuler");
	}

	if(strcmp(sub, "entreprise", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de ton entree.");
		if(venues[vid][vpOwnerSqlid] != PlayerInfo[playerid][pSQLID])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas le proprio.");
		if(strlen(rest) == 0)
		{
			new curTxt[128];
			if(strlen(venues[vid][vpEnterprise]) == 0)
				format(curTxt, sizeof(curTxt), "{FFFFFF}Aucune entreprise liee. Recettes = poche proprio.\nUsage : /venue entreprise <nom> ou 'off'.");
			else
				format(curTxt, sizeof(curTxt), "{FFFFFF}Actuellement liee a : {FFFFB2}%s{FFFFFF}.\nUsage : /venue entreprise <nom> ou 'off'.", venues[vid][vpEnterprise]);
			return ShowPlayerDialog(playerid, 9994, DIALOG_STYLE_MSGBOX, "Lien entreprise", curTxt, "OK", "");
		}
		if(strcmp(rest, "off", true) == 0)
		{
			venues[vid][vpEnterprise][0] = 0;
			Venue_Save();
			return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Lien entreprise retire. Recettes vont en poche proprio.");
		}
		// Verifier que l'entreprise existe et que le joueur en fait partie
		if(Ent_FindByName(rest) == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Entreprise introuvable.");
		new eidCheck = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		new empEid, empSlot;
		new bool:hasAccess = (eidCheck != -1 && strcmp(entreprises[eidCheck][entName], rest, true) == 0);
		if(!hasAccess && Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], empEid, empSlot))
			hasAccess = (empEid != -1 && strcmp(entreprises[empEid][entName], rest, true) == 0);
		if(!hasAccess)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es ni patron ni employe de cette entreprise.");
		new n = 0; while(n < 63 && rest[n]) { venues[vid][vpEnterprise][n] = rest[n]; n++; } venues[vid][vpEnterprise][n] = 0;
		Venue_Save();
		new okStr[160];
		format(okStr, sizeof(okStr), "{42AAFF} Lieu {FFFFFF} Ce lieu envoie maintenant ses recettes dans la caisse {FFFFB2}%s{FFFFFF}.", venues[vid][vpEnterprise]);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}

	if(strcmp(sub, "ferme", true) == 0 || strcmp(sub, "fermer", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de ton entree.");
		if(venues[vid][vpOwnerSqlid] != PlayerInfo[playerid][pSQLID])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas le proprio.");
		venues[vid][vpLocked] = 1;
		Venue_UpdateLabel(vid);
		Venue_Save();
		return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Ton etablissement est ferme au public.");
	}
	if(strcmp(sub, "ouvre", true) == 0 || strcmp(sub, "ouvrir", true) == 0)
	{
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de ton entree.");
		if(venues[vid][vpOwnerSqlid] != PlayerInfo[playerid][pSQLID])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Tu n'es pas le proprio.");
		venues[vid][vpLocked] = 0;
		Venue_UpdateLabel(vid);
		Venue_Save();
		return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Ton etablissement est ouvert au public.");
	}

	// Commandes admin
	if(strcmp(sub, "creer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(strlen(rest) < 2)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /venue creer <nom>");
		if(venue_Count >= VP_MAX_VENUE)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Lieu {FFFFFF} Limite atteinte.");
		new vid = -1;
		for(new k = 0; k < VP_MAX_VENUE; k++) { if(venues[k][vpUsed] == 0) { vid = k; break; } }
		if(vid == -1) return msg_Client(playerid, COLOR_INFO, "{FF0000} Lieu {FFFFFF} Pas de slot libre.");
		venues[vid][vpUsed] = 1;
		new n = 0; while(n < 63 && rest[n]) { venues[vid][vpName][n] = rest[n]; n++; } venues[vid][vpName][n] = 0;
		venues[vid][vpOwnerSqlid] = 0;
		GetPlayerPos(playerid, venues[vid][vpEntryX], venues[vid][vpEntryY], venues[vid][vpEntryZ]);
		// Barriere placee 1.5m dans la direction ou regarde l'admin (vers la porte).
		new Float:facing;
		GetPlayerFacingAngle(playerid, facing);
		venues[vid][vpBarrierX] = venues[vid][vpEntryX] - floatsin(facing, degrees) * VP_BARRIER_OFFSET;
		venues[vid][vpBarrierY] = venues[vid][vpEntryY] + floatcos(facing, degrees) * VP_BARRIER_OFFSET;
		venues[vid][vpBarrierZ] = venues[vid][vpEntryZ];
		venues[vid][vpBarrierArea] = STREAMER_TAG_AREA:-1;
		venues[vid][vpExitX] = 0.0; venues[vid][vpExitY] = 0.0; venues[vid][vpExitZ] = 0.0;
		venues[vid][vpExitInt] = 0; venues[vid][vpExitVW] = 0;
		venues[vid][vpEntryFee] = 500;
		venues[vid][vpPin][0] = 0;
		venues[vid][vpLocked] = 0;
		venues[vid][vpRevenue] = 0;
		venue_Count++;
		Venue_CreateVisuals(vid);
		Venue_Save();
		new okStr[220];
		format(okStr, sizeof(okStr), "{42AAFF} Lieu {FFFFFF} Cree (ID %d) : {FFFFB2}%s{FFFFFF}. Barriere placee dans la direction ou tu regardes. Etape suivante : /venue interieur DEDANS puis /venue proprio <joueur>.", vid, venues[vid][vpName]);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}

	if(strcmp(sub, "barriere", true) == 0 || strcmp(sub, "porte", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		new vid = -1;
		new Float:bestDist = 30.0;
		for(new k = 0; k < VP_MAX_VENUE; k++)
		{
			if(venues[k][vpUsed] != 1) continue;
			new Float:d = GetPlayerDistanceFromPoint(playerid, venues[k][vpEntryX], venues[k][vpEntryY], venues[k][vpEntryZ]);
			if(d < bestDist) { bestDist = d; vid = k; }
		}
		if(vid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Aucun lieu a moins de 30m.");
		// Nouvelle barriere = position actuelle du joueur (mets-toi pile a l'endroit du blocage)
		GetPlayerPos(playerid, venues[vid][vpBarrierX], venues[vid][vpBarrierY], venues[vid][vpBarrierZ]);
		Venue_DestroyVisuals(vid);
		Venue_CreateVisuals(vid);
		Venue_Save();
		new okStr[160];
		format(okStr, sizeof(okStr), "{42AAFF} Lieu {FFFFFF} Barriere de {FFFFB2}%s{FFFFFF} placee ici (rayon %.1fm).", venues[vid][vpName], VP_BARRIER_RADIUS);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}

	if(strcmp(sub, "interieur", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		// On cible le venue le plus proche de l'entree connue (dans le monde exterieur)
		new bestVid = -1;
		new Float:bestDist = 1000.0;
		for(new k = 0; k < VP_MAX_VENUE; k++)
		{
			if(venues[k][vpUsed] != 1) continue;
			new Float:d = GetPlayerDistanceFromPoint(playerid, venues[k][vpEntryX], venues[k][vpEntryY], venues[k][vpEntryZ]);
			if(d < bestDist) { bestDist = d; bestVid = k; }
		}
		if(bestVid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Aucun lieu enregistre.");
		GetPlayerPos(playerid, venues[bestVid][vpExitX], venues[bestVid][vpExitY], venues[bestVid][vpExitZ]);
		venues[bestVid][vpExitInt] = GetPlayerInterior(playerid);
		venues[bestVid][vpExitVW]  = GetPlayerVirtualWorld(playerid);
		Venue_DestroyVisuals(bestVid);
		Venue_CreateVisuals(bestVid);
		Venue_Save();
		new okStr[160];
		format(okStr, sizeof(okStr), "{42AAFF} Lieu {FFFFFF} Interieur fixe pour {FFFFB2}%s{FFFFFF} (int %d / VW %d).", venues[bestVid][vpName], venues[bestVid][vpExitInt], venues[bestVid][vpExitVW]);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}

	if(strcmp(sub, "proprio", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(strlen(rest) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /venue proprio <joueur>");
		new target = ReturnUser(rest);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Joueur introuvable.");
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Approche-toi de l'entree du lieu concerne.");
		venues[vid][vpOwnerSqlid] = PlayerInfo[target][pSQLID];
		Venue_Save();
		new okStr[160], tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		format(okStr, sizeof(okStr), "{42AAFF} Lieu {FFFFFF} {FFFFB2}%s{FFFFFF} est maintenant proprio de {FFFFB2}%s{FFFFFF}.", tname, venues[vid][vpName]);
		msg_Client(playerid, COLOR_WHITE, okStr);
		new notif[160];
		format(notif, sizeof(notif), "{42AAFF} Lieu {FFFFFF} Tu es maintenant proprio de {FFFFB2}%s{FFFFFF} ! Utilise /venue code /venue prix /venue ferme /venue ouvre.", venues[vid][vpName]);
		msg_Client(target, COLOR_WHITE, notif);
		return 1;
	}

	if(strcmp(sub, "supprimer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		new vid = Venue_GetNearestEntry(playerid);
		if(vid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Lieu {FFFFFF} Aucun lieu a proximite.");
		Venue_DestroyVisuals(vid);
		venues[vid][vpUsed] = 0;
		venue_Count--; if(venue_Count < 0) venue_Count = 0;
		Venue_Save();
		return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Lieu {FFFFFF} Lieu supprime.");
	}

	if(strcmp(sub, "lister", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 1)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve staff.");
		new listBody[2048] = "{42AAFF}Lieux prives :{FFFFFF}\n\n";
		new tmp[160];
		new c = 0;
		for(new k = 0; k < VP_MAX_VENUE; k++)
		{
			if(venues[k][vpUsed] != 1) continue;
			format(tmp, sizeof(tmp), "ID %d : %s (owner SQL %d) fee %d$ %s\n",
				k, venues[k][vpName], venues[k][vpOwnerSqlid], venues[k][vpEntryFee],
				(venues[k][vpLocked] == 1) ? "[FERME]" : "[OUVERT]");
			strcat(listBody, tmp);
			c++;
		}
		if(c == 0) strcat(listBody, "Aucun lieu cree.");
		return ShowPlayerDialog(playerid, 9998, DIALOG_STYLE_MSGBOX, "Lieux prives", listBody, "OK", "");
	}

	msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /venue [sortir|infos|code|prix|ferme|ouvre]");
	if(PlayerInfo[playerid][pAdmin] >= 3)
		msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Admin: /venue [creer <nom>|interieur|proprio <joueur>|supprimer|lister]");
	return 1;
}
