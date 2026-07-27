/*
	afrp_portarme.inc - Port d'arme (permis de detention d'armes a feu)

	Un admin place une guerite physique (meme modele que les cabanes de
	peage : 1273) a un ou plusieurs endroits. Les joueurs s'en approchent
	et tapent /portarme pour acheter le permis pour 20.000.000$.

	Sans ce permis, il est desormais impossible de RAMASSER une arme a
	feu tombee au sol (armes de melee toujours libres pour tout le monde).
	Une tentative de ramassage sans permis fait disparaitre l'arme au sol
	au lieu de la donner - objectif : rendre inutile le fait de looter des
	armes via un hack/exploit puisque rien n'est jamais donne sans permis.
	Les admins en service (AdminDuty) contournent toujours cette regle.

	Persistance :
	  scriptfiles/portarme_booths.cfg   (position des guerites, admin)
	  scriptfiles/portarme_holders.cfg  (SQLID des detenteurs du permis)
*/

#if defined AFRP_PORTARME_INCLUDED
	#endinput
#endif
#define AFRP_PORTARME_INCLUDED

#define PA_MAX_BOOTH        20
#define PA_MAX_HOLDERS      5000
#define PA_PRICE            20000000
#define PA_RADIUS           4.0
#define PA_BOOTH_MODEL      1273     // meme guerite que les cabanes de peage
#define PA_BOOTH_FILE       "portarme_booths.cfg"
#define PA_HOLDER_FILE      "portarme_holders.cfg"

enum e_PABooth
{
	paUsedB,
	Float:paBX, Float:paBY, Float:paBZ, Float:paBAngle
};
new paBooth[PA_MAX_BOOTH][e_PABooth];
new paBooth_Object[PA_MAX_BOOTH];
new Text3D:paBooth_Label[PA_MAX_BOOTH];
new paBooth_Count = 0;

new portArme_HolderSqlid[PA_MAX_HOLDERS];
new portArme_HolderCount = 0;

// ===================== MELEE / ANTI-HACK =====================

// Armes de melee GTA SA (slot melee, id 1 a 15) : libres pour tous, sans permis.
stock bool:IsWeaponMelee(weaponid)
{
	return bool:(weaponid >= 1 && weaponid <= 15);
}

stock bool:PortArme_Has(sqlid)
{
	if(sqlid < 1) return false;
	for(new i = 0; i < portArme_HolderCount; i++)
	{
		if(portArme_HolderSqlid[i] == sqlid) return true;
	}
	return false;
}

// Regle centrale : un joueur peut-il ramasser cette arme au sol ?
// Admin en service : toujours. Arme de melee : toujours. Sinon : permis requis.
stock bool:PortArme_CanPickup(playerid, weaponModel)
{
	if(AdminDuty[playerid] == 1) return true;
	if(IsWeaponMelee(weaponModel)) return true;
	return PortArme_Has(PlayerInfo[playerid][pSQLID]);
}

// ===================== PERSISTANCE =====================

stock PortArme_Init()
{
	for(new i = 0; i < PA_MAX_BOOTH; i++)
	{
		paBooth[i][paUsedB] = 0;
		paBooth_Object[i] = -1;
		paBooth_Label[i] = Text3D:INVALID_3DTEXT_ID;
	}
	PortArme_LoadBooths();
	PortArme_LoadHolders();
	return 1;
}

stock PortArme_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock PortArme_CreateVisuals(id)
{
	if(paBooth[id][paUsedB] != 1) return 0;
	if(paBooth_Object[id] == -1)
	{
		paBooth_Object[id] = CreateDynamicObject(PA_BOOTH_MODEL,
			paBooth[id][paBX], paBooth[id][paBY], paBooth[id][paBZ], 0.0, 0.0, paBooth[id][paBAngle]);
	}
	if(_:paBooth_Label[id] == _:INVALID_3DTEXT_ID)
	{
		new lblTxt[160];
		new priceStr[24];
		FormatMoney(PA_PRICE, priceStr, sizeof(priceStr));
		format(lblTxt, sizeof(lblTxt), "{F0A020}{== PORT D'ARME ==}\n{FFFFFF}Permis obligatoire pour les armes a feu\n{9EC73D}Prix : $%s\n{CCCCCC}/portarme pour acheter", priceStr);
		paBooth_Label[id] = Create3DTextLabel(lblTxt, 0xFFFFFFFF, paBooth[id][paBX], paBooth[id][paBY], paBooth[id][paBZ] + 2.3, 20.0, 0, 0);
	}
	return 1;
}

stock PortArme_DestroyVisuals(id)
{
	if(paBooth_Object[id] != -1) { DestroyDynamicObject(paBooth_Object[id]); paBooth_Object[id] = -1; }
	if(_:paBooth_Label[id] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(paBooth_Label[id]);
		paBooth_Label[id] = Text3D:INVALID_3DTEXT_ID;
	}
	return 1;
}

stock PortArme_LoadBooths()
{
	if(!fexist(PA_BOOTH_FILE)) return 0;
	new File:f = fopen(PA_BOOTH_FILE, io_read);
	if(!f) return 0;
	new line[160], tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 3) continue;
		if(paBooth_Count >= PA_MAX_BOOTH) break;
		new id = paBooth_Count;
		new i = 0;
		PortArme_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); paBooth[id][paBX] = floatstr(tmpBuf);
		PortArme_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); paBooth[id][paBY] = floatstr(tmpBuf);
		PortArme_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); paBooth[id][paBZ] = floatstr(tmpBuf);
		PortArme_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); paBooth[id][paBAngle] = floatstr(tmpBuf);
		paBooth[id][paUsedB] = 1;
		PortArme_CreateVisuals(id);
		paBooth_Count++;
	}
	fclose(f);
	printf("[PortArme] %d guerite(s) chargee(s)", paBooth_Count);
	return paBooth_Count;
}

stock PortArme_SaveBooths()
{
	if(fexist(PA_BOOTH_FILE)) fremove(PA_BOOTH_FILE);
	new File:f = fopen(PA_BOOTH_FILE, io_write);
	if(!f) return 0;
	new line[160];
	for(new i = 0; i < PA_MAX_BOOTH; i++)
	{
		if(paBooth[i][paUsedB] != 1) continue;
		format(line, sizeof(line), "%.4f|%.4f|%.4f|%.4f\n", paBooth[i][paBX], paBooth[i][paBY], paBooth[i][paBZ], paBooth[i][paBAngle]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock PortArme_LoadHolders()
{
	if(!fexist(PA_HOLDER_FILE)) return 0;
	new File:f = fopen(PA_HOLDER_FILE, io_read);
	if(!f) return 0;
	new line[16];
	while(fread(f, line))
	{
		if(strlen(line) < 1) continue;
		if(portArme_HolderCount >= PA_MAX_HOLDERS) break;
		new sqlid = strval(line);
		if(sqlid < 1) continue;
		portArme_HolderSqlid[portArme_HolderCount++] = sqlid;
	}
	fclose(f);
	printf("[PortArme] %d detenteur(s) de permis charge(s)", portArme_HolderCount);
	return portArme_HolderCount;
}

stock PortArme_SaveHolders()
{
	if(fexist(PA_HOLDER_FILE)) fremove(PA_HOLDER_FILE);
	new File:f = fopen(PA_HOLDER_FILE, io_write);
	if(!f) return 0;
	new line[16];
	for(new i = 0; i < portArme_HolderCount; i++)
	{
		format(line, sizeof(line), "%d\n", portArme_HolderSqlid[i]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock PortArme_Grant(sqlid)
{
	if(sqlid < 1 || PortArme_Has(sqlid)) return 0;
	if(portArme_HolderCount >= PA_MAX_HOLDERS) return 0;
	portArme_HolderSqlid[portArme_HolderCount++] = sqlid;
	PortArme_SaveHolders();
	return 1;
}

stock PortArme_Revoke(sqlid)
{
	for(new i = 0; i < portArme_HolderCount; i++)
	{
		if(portArme_HolderSqlid[i] != sqlid) continue;
		for(new j = i; j < portArme_HolderCount - 1; j++)
			portArme_HolderSqlid[j] = portArme_HolderSqlid[j + 1];
		portArme_HolderCount--;
		PortArme_SaveHolders();
		return 1;
	}
	return 0;
}

stock PortArme_NearestBooth(playerid)
{
	new Float:best = PA_RADIUS;
	new bid = -1;
	for(new i = 0; i < PA_MAX_BOOTH; i++)
	{
		if(paBooth[i][paUsedB] != 1) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid, paBooth[i][paBX], paBooth[i][paBY], paBooth[i][paBZ]);
		if(d < best) { best = d; bid = i; }
	}
	return bid;
}

// ===================== COMMANDE =====================

stock PortArme_Cmd(playerid, params[])
{
	new sub[16], rest[64];
	new i = 0, p = 0;
	while(p < 15 && params[i] && params[i] != ' ') { sub[p++] = params[i++]; }
	sub[p] = 0;
	while(params[i] == ' ') i++;
	p = 0;
	while(p < 63 && params[i]) { rest[p++] = params[i++]; }
	rest[p] = 0;

	// -------- admin --------
	if(strcmp(sub, "creer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(paBooth_Count >= PA_MAX_BOOTH)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Port d'arme {FFFFFF} Limite de guerites atteinte.");
		new id = -1;
		for(new k = 0; k < PA_MAX_BOOTH; k++) { if(paBooth[k][paUsedB] == 0) { id = k; break; } }
		if(id == -1) return msg_Client(playerid, COLOR_INFO, "{FF0000} Port d'arme {FFFFFF} Pas de slot libre.");
		GetPlayerPos(playerid, paBooth[id][paBX], paBooth[id][paBY], paBooth[id][paBZ]);
		GetPlayerFacingAngle(playerid, paBooth[id][paBAngle]);
		paBooth[id][paUsedB] = 1;
		paBooth_Count++;
		PortArme_CreateVisuals(id);
		PortArme_SaveBooths();
		new okStr[160];
		format(okStr, sizeof(okStr), "{F0A020} Port d'arme {FFFFFF} Guerite creee ici (ID %d).", id);
		return msg_Client(playerid, COLOR_WHITE, okStr);
	}
	if(strcmp(sub, "supprimer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		new id = PortArme_NearestBooth(playerid);
		if(id == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Aucune guerite a proximite.");
		PortArme_DestroyVisuals(id);
		paBooth[id][paUsedB] = 0;
		paBooth_Count--; if(paBooth_Count < 0) paBooth_Count = 0;
		PortArme_SaveBooths();
		return msg_Client(playerid, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Guerite supprimee.");
	}
	if(strcmp(sub, "lister", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 1)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve staff.");
		new body[1024], tmp[96], c = 0;
		strcat(body, "{FFFFFF}Guerites port d'arme :\n\n");
		for(new k = 0; k < PA_MAX_BOOTH; k++)
		{
			if(paBooth[k][paUsedB] != 1) continue;
			format(tmp, sizeof(tmp), "ID %d : (%.0f, %.0f)\n", k, paBooth[k][paBX], paBooth[k][paBY]);
			strcat(body, tmp); c++;
		}
		if(c == 0) strcat(body, "Aucune guerite.");
		new footTmp[64];
		format(footTmp, sizeof(footTmp), "\n%d joueur(s) possedent le permis.", portArme_HolderCount);
		strcat(body, footTmp);
		return ShowPlayerDialog(playerid, 9600, DIALOG_STYLE_MSGBOX, "Port d'arme", body, "OK", "");
	}
	if(strcmp(sub, "donner", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(strlen(rest) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /portarme donner <joueur>");
		new target = ReturnUser(rest);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Joueur introuvable.");
		if(!PortArme_Grant(PlayerInfo[target][pSQLID]))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Ce joueur a deja le permis.");
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new okStr[128];
		format(okStr, sizeof(okStr), "{F0A020} Port d'arme {FFFFFF} Permis accorde a %s.", tname);
		msg_Client(playerid, COLOR_WHITE, okStr);
		msg_Client(target, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Un admin t'a accorde le port d'arme.");
		return 1;
	}
	if(strcmp(sub, "retirer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 3)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 3+.");
		if(strlen(rest) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /portarme retirer <joueur>");
		new target = ReturnUser(rest);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Joueur introuvable.");
		if(!PortArme_Revoke(PlayerInfo[target][pSQLID]))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Ce joueur n'a pas le permis.");
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new okStr[128];
		format(okStr, sizeof(okStr), "{F0A020} Port d'arme {FFFFFF} Permis retire a %s.", tname);
		msg_Client(playerid, COLOR_WHITE, okStr);
		msg_Client(target, COLOR_WHITE, "{FF3333} Port d'arme {FFFFFF} Ton permis de port d'arme a ete retire.");
		return 1;
	}

	// -------- joueur : infos --------
	if(strcmp(sub, "infos", true) == 0)
	{
		if(PortArme_Has(PlayerInfo[playerid][pSQLID]))
			return msg_Client(playerid, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Tu possedes deja le permis. Tu peux ramasser toutes les armes.");
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Tu n'as pas le permis. Trouve une guerite et tape /portarme.");
	}

	// -------- joueur : achat (pres d'une guerite) --------
	new id = PortArme_NearestBooth(playerid);
	if(id == -1)
	{
		if(PortArme_Has(PlayerInfo[playerid][pSQLID]))
			return msg_Client(playerid, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Tu possedes deja le permis.");
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Port d'arme {FFFFFF} Approche-toi d'une guerite (voir la carte).");
	}
	if(PortArme_Has(PlayerInfo[playerid][pSQLID]))
		return msg_Client(playerid, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Tu possedes deja le permis.");
	if(PlayerInfo[playerid][pCash] < PA_PRICE)
	{
		new priceStr[24], msg[160];
		FormatMoney(PA_PRICE, priceStr, sizeof(priceStr));
		format(msg, sizeof(msg), "{CF9756} Port d'arme {FFFFFF} Il te faut $%s pour acheter le permis.", priceStr);
		return msg_Client(playerid, COLOR_INFO, msg);
	}
	SafeGivePlayerMoney(playerid, -PA_PRICE, "Achat port d'arme");
	PortArme_Grant(PlayerInfo[playerid][pSQLID]);
	return msg_Client(playerid, COLOR_WHITE, "{F0A020} Port d'arme {FFFFFF} Felicitations, tu peux desormais ramasser les armes a feu au sol.");
}
