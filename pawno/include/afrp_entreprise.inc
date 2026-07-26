/*
	afrp_entreprise.inc - Entreprises legales (concurrent RP des gangs)

	Un joueur cree une entreprise pour 5.000.000$, y embauche des salaries
	avec un salaire horaire, gere une caisse commune. Le salaire est verse
	automatiquement toutes les 60 minutes de connexion de l'employe -
	meme rythme que les paies de jobs existantes.

	Integration venue :
	  - Le proprio d'un venue peut lier ce venue a son entreprise via
	    /venue entreprise <nom>. Des lors, tout paiement client va dans la
	    CAISSE de l'entreprise au lieu de la poche du proprio.

	Payday :
	  - Chaque minute de connexion incremente un compteur par employe.
	  - A 60, on prend salaire dans la caisse et on paie l'employe.
	  - Si caisse insuffisante : employe et proprio notifies, compteur remis
	    a 60 (pas grille) => la paie est due des que la caisse se remplit.

	Persistance : scriptfiles/entreprises.cfg (une ligne par entreprise).
	Format : name|owner_sqlid|cash|created_ts|emp1_sqlid,salary1;emp2_sqlid,salary2;...
*/

#if defined AFRP_ENTREPRISE_INCLUDED
	#endinput
#endif
#define AFRP_ENTREPRISE_INCLUDED

#define ENT_MAX             30
#define ENT_MAX_EMP         12          // employes max par entreprise
#define ENT_CREATE_COST     50000000    // 50M pour creer
#define ENT_DISSOLVE_FEE    100000      // 100k retenu a la dissolution
#define ENT_FILE            "entreprises.cfg"
#define ENT_TICK_MS         60000       // 60s
#define ENT_DLG_LOCAL       9984        // dialog choix du local
#define ENT_DLG_TYPE        9986        // dialog choix du secteur d'activite
#define ENT_VW_BASE         7100        // monde virtuel dedie : ENT_VW_BASE + eid
#define ENT_INT_COUNT       13          // nb de locaux au catalogue
#define ENT_TYPE_COUNT      8           // nb de secteurs d'activite

// Secteurs d'activite. Purement identitaires pour l'instant (affiches dans
// /entreprise info, sur l'enseigne du local et dans la liste admin) : aucun
// bonus chiffre, pour ne desequilibrer aucun type par rapport aux autres.
enum e_EntType
{
	etName[28],
	etDesc[40]
};
static const EntTypes[ENT_TYPE_COUNT][e_EntType] = {
	{"Transport & Logistique", "Livraisons, fret, missions cargo"},
	{"Batiment & Travaux",     "Construction, renovation"},
	{"Securite privee",        "Gardiennage, escorte"},
	{"Restauration",           "Restaurant, bar, traiteur"},
	{"Commerce & Import",      "Boutique, revente, import"},
	{"Garage & Mecanique",     "Reparation, tuning, depannage"},
	{"Immobilier",             "Achat, vente, gestion de biens"},
	{"Evenementiel & Medias",  "Soirees, concerts, communication"}
};

// Catalogue des locaux achetables. Prix volontairement bien plus bas que la
// creation d'entreprise (50M) : c'est un amenagement, pas un investissement.
// Interieurs repris de ceux deja utilises par le gamemode (GetInteriorInfo),
// donc garantis valides et testes en jeu.
enum e_EntInt
{
	eiName[32],
	eiPrice,
	eiInt,
	Float:eiX, Float:eiY, Float:eiZ, Float:eiA
};
// Les "bureaux" viennent des interieurs deja utilises par le gamemode
// (GetInteriorInfo) ; les logements viennent du catalogue d'interieurs de
// maison (dialog 24 admin). Tous sont donc deja testes en jeu.
static const EntInteriors[ENT_INT_COUNT][e_EntInt] = {
	{"Petit bureau",         250000,  1,  1199.4508,  -74.6475, 1006.2589, 180.0},
	{"Appartement 2 pieces", 350000, 12,   446.3247,  509.9662, 1001.4195,   0.0},
	{"Bureau standard",      500000,  3,  1494.2778, 1303.7288, 1093.2891,   0.0},
	{"Appartement 3 pieces", 650000,  6,   -68.5145, 1353.8485, 1080.2109,   0.0},
	{"Bureau de direction",  900000,  3, -1032.7937, 1967.4984, 3001.0859, 270.0},
	{"Maison 4 pieces",     1100000,  8,    2365.30, -1134.920,  1050.910,   0.0},
	{"Salle de reunion",    1400000,  5,   316.7042,  294.6810,  998.4849,   0.0},
	{"Maison 5 pieces",     1700000, 10,    24.3769, 1341.1829, 1084.3750,   0.0},
	{"Grand hall",          2000000,  1,  1964.0809, -349.6421, 1092.9454, 180.0},
	{"Maison 6 pieces",     2400000,  9,  2319.1272,-1023.9562, 1050.2109,   0.0},
	{"Siege social",        3000000,  1,  -774.6643,  550.3431, 2001.0859, 270.0},
	{"Villa 8 pieces",      4000000,  6,   234.0459, 1064.8798, 1084.3099,   0.0},
	{"Villa 9 pieces",      5000000,  5,    140.180, 1366.5800, 1083.9700,   0.0}
};

enum e_Ent
{
	entUsed,
	entName[64],
	entOwnerSqlid,
	entCash,
	entCreatedTs,
	entType,                         // 0 = non defini, sinon 1..ENT_TYPE_COUNT
	entIntId,                        // 0 = aucun local, sinon 1..ENT_INT_COUNT
	Float:entEntryX, Float:entEntryY, Float:entEntryZ,  // porte d'entree dans le monde
	entEmpSqlid[ENT_MAX_EMP],
	entEmpSalary[ENT_MAX_EMP],
	entEmpUsed[ENT_MAX_EMP],
	entEmpPendingMin[ENT_MAX_EMP]    // compteur minutes connectees en attente de paie
};
new entreprises[ENT_MAX][e_Ent];
new ent_Count = 0;
new entPickup[ENT_MAX];              // pickup entree (monde exterieur)
new Text3D:entLabel[ENT_MAX];        // label 3D entree
new entExitPickup[ENT_MAX];          // pickup sortie (dans le local)
new Text3D:entExitLabel[ENT_MAX];    // label 3D sortie

stock Ent_Init()
{
	for(new i = 0; i < ENT_MAX; i++)
	{
		entreprises[i][entUsed] = 0;
		entreprises[i][entIntId] = 0;
		entPickup[i]     = -1;
		entExitPickup[i] = -1;
		entLabel[i]      = Text3D:INVALID_3DTEXT_ID;
		entExitLabel[i]  = Text3D:INVALID_3DTEXT_ID;
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			entreprises[i][entEmpUsed][e] = 0;
			entreprises[i][entEmpSqlid][e] = 0;
			entreprises[i][entEmpSalary][e] = 0;
			entreprises[i][entEmpPendingMin][e] = 0;
		}
	}
	Ent_Load();
	SetTimer("Ent_Tick", ENT_TICK_MS, 1);
	return 1;
}

// Nom du secteur d'activite d'une entreprise ("Non defini" si pas choisi).
// Defini ici (avant Ent_CreateVisuals) car l'enseigne du local l'affiche.
stock Ent_TypeName(eid, dest[], destSize)
{
	dest[0] = 0;
	new t = entreprises[eid][entType];
	if(t < 1 || t > ENT_TYPE_COUNT) { strcat(dest, "Non defini", destSize); return 0; }
	strcat(dest, EntTypes[t-1][etName], destSize);
	return 1;
}

// Detruit puis recree le pickup + label d'entree/sortie d'une entreprise.
stock Ent_DestroyVisuals(eid)
{
	if(entPickup[eid] != -1)     { DestroyPickup(entPickup[eid]);     entPickup[eid] = -1; }
	if(entExitPickup[eid] != -1) { DestroyPickup(entExitPickup[eid]); entExitPickup[eid] = -1; }
	if(_:entLabel[eid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(entLabel[eid]);
		entLabel[eid] = Text3D:INVALID_3DTEXT_ID;
	}
	if(_:entExitLabel[eid] != _:INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(entExitLabel[eid]);
		entExitLabel[eid] = Text3D:INVALID_3DTEXT_ID;
	}
	return 1;
}

stock Ent_CreateVisuals(eid)
{
	Ent_DestroyVisuals(eid);
	if(entreprises[eid][entUsed] != 1) return 0;
	new iid = entreprises[eid][entIntId];
	if(iid < 1 || iid > ENT_INT_COUNT) return 0;   // pas de local achete

	new labelStr[220], typeStr[32];
	Ent_TypeName(eid, typeStr, sizeof(typeStr));
	format(labelStr, sizeof(labelStr), "{42AAFF}%s\n{FFFF66}%s\n{FFFFFF}%s\n{CCCCCC}/entreprise entrer",
		entreprises[eid][entName], typeStr, EntInteriors[iid-1][eiName]);

	entPickup[eid] = CreatePickup(1272, 1,
		entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ], 0);
	entLabel[eid] = Create3DTextLabel(labelStr, 0xFFFFFFFF,
		entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ] + 0.8,
		20.0, 0, 0);

	// Sortie, dans le monde virtuel dedie a cette entreprise
	new vw = ENT_VW_BASE + eid;
	entExitPickup[eid] = CreatePickup(1272, 1,
		EntInteriors[iid-1][eiX], EntInteriors[iid-1][eiY], EntInteriors[iid-1][eiZ], vw);
	entExitLabel[eid] = Create3DTextLabel("{42AAFF}Sortie\n{CCCCCC}/entreprise sortir", 0xFFFFFFFF,
		EntInteriors[iid-1][eiX], EntInteriors[iid-1][eiY], EntInteriors[iid-1][eiZ] + 0.8,
		15.0, vw, 0);
	return 1;
}

stock Ent_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

stock Ent_Load()
{
	if(!fexist(ENT_FILE)) return 0;
	new File:f = fopen(ENT_FILE, io_read);
	if(!f) return 0;
	new line[1024];
	new tmpBuf[32];
	while(fread(f, line))
	{
		if(strlen(line) < 5) continue;
		if(ent_Count >= ENT_MAX) break;
		new eid = ent_Count;
		new i = 0;

		Ent_ReadField(line, i, entreprises[eid][entName], 64, 63);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entOwnerSqlid] = strval(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entCash]       = strval(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entCreatedTs]  = strval(tmpBuf);

		// Employes format : sqlid,salary;sqlid,salary;...
		new empBuf[512];
		Ent_ReadField(line, i, empBuf, sizeof(empBuf), 511);
		new eslot = 0;
		new j = 0;
		new numBuf[32], pnum;
		while(empBuf[j] && eslot < ENT_MAX_EMP)
		{
			// sqlid
			pnum = 0;
			while(empBuf[j] && empBuf[j] != ',' && empBuf[j] != ';' && pnum < 31) { numBuf[pnum++] = empBuf[j++]; }
			numBuf[pnum] = 0;
			if(pnum == 0) { if(empBuf[j]) j++; continue; }
			entreprises[eid][entEmpSqlid][eslot] = strval(numBuf);
			if(empBuf[j] == ',') j++;
			// salary
			pnum = 0;
			while(empBuf[j] && empBuf[j] != ';' && pnum < 31) { numBuf[pnum++] = empBuf[j++]; }
			numBuf[pnum] = 0;
			entreprises[eid][entEmpSalary][eslot] = strval(numBuf);
			entreprises[eid][entEmpUsed][eslot] = 1;
			entreprises[eid][entEmpPendingMin][eslot] = 0;
			if(empBuf[j] == ';') j++;
			eslot++;
		}

		// Local achete (champs ajoutes apres coup : les anciennes lignes sans ces
		// champs relisent une chaine vide -> 0, donc "aucun local", pas de casse).
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entIntId]  = strval(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entEntryX] = floatstr(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entEntryY] = floatstr(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entEntryZ] = floatstr(tmpBuf);
		Ent_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 31); entreprises[eid][entType]   = strval(tmpBuf);
		if(entreprises[eid][entIntId] < 0 || entreprises[eid][entIntId] > ENT_INT_COUNT)
			entreprises[eid][entIntId] = 0;
		if(entreprises[eid][entType] < 0 || entreprises[eid][entType] > ENT_TYPE_COUNT)
			entreprises[eid][entType] = 0;

		entreprises[eid][entUsed] = 1;
		Ent_CreateVisuals(eid);
		ent_Count++;
	}
	fclose(f);
	printf("[Entreprise] %d entreprise(s) chargee(s) depuis %s", ent_Count, ENT_FILE);
	return ent_Count;
}

stock Ent_Save()
{
	if(fexist(ENT_FILE)) fremove(ENT_FILE);
	new File:f = fopen(ENT_FILE, io_write);
	if(!f) return 0;
	new line[1024], empPart[512], tmp[64];
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		empPart[0] = 0;
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			if(entreprises[eid][entEmpUsed][e] != 1) continue;
			format(tmp, sizeof(tmp), "%d,%d;", entreprises[eid][entEmpSqlid][e], entreprises[eid][entEmpSalary][e]);
			strcat(empPart, tmp);
		}
		format(line, sizeof(line), "%s|%d|%d|%d|%s|%d|%.4f|%.4f|%.4f|%d\n",
			entreprises[eid][entName],
			entreprises[eid][entOwnerSqlid],
			entreprises[eid][entCash],
			entreprises[eid][entCreatedTs],
			empPart,
			entreprises[eid][entIntId],
			entreprises[eid][entEntryX],
			entreprises[eid][entEntryY],
			entreprises[eid][entEntryZ],
			entreprises[eid][entType]);
		fwrite(f, line);
	}
	fclose(f);
	return 1;
}

stock Ent_FindByOwner(ownerSqlid)
{
	if(ownerSqlid < 1) return -1;
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		if(entreprises[eid][entOwnerSqlid] == ownerSqlid) return eid;
	}
	return -1;
}

stock Ent_FindByName(const name[])
{
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		if(strcmp(entreprises[eid][entName], name, true) == 0) return eid;
	}
	return -1;
}

// Cherche un employe (eid, slot) pour un SQLID donne. -1/-1 si introuvable.
stock Ent_FindEmployeeSlot(empSqlid, &outEid, &outSlot)
{
	outEid = -1; outSlot = -1;
	if(empSqlid < 1) return 0;
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			if(entreprises[eid][entEmpUsed][e] != 1) continue;
			if(entreprises[eid][entEmpSqlid][e] == empSqlid)
			{
				outEid = eid; outSlot = e; return 1;
			}
		}
	}
	return 0;
}

stock Ent_AllocEmpSlot(eid)
{
	if(entreprises[eid][entUsed] != 1) return -1;
	for(new e = 0; e < ENT_MAX_EMP; e++)
		if(entreprises[eid][entEmpUsed][e] == 0) return e;
	return -1;
}

// Cherche le joueur online pour un SQLID donne.
stock Ent_FindPlayerBySqlid(ent_sqlid)
{
	if(ent_sqlid < 1) return INVALID_PLAYER_ID;
	for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
	{
		if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
		if(PlayerInfo[p][pSQLID] == ent_sqlid) return p;
	}
	return INVALID_PLAYER_ID;
}

// API utilisee par afrp_venuepriv : crediter la caisse d'une entreprise
// par nom. Retourne 1 si credite, 0 sinon.
stock Ent_CreditByName(const name[], amount)
{
	if(amount <= 0) return 0;
	new eid = Ent_FindByName(name);
	if(eid == -1) return 0;
	entreprises[eid][entCash] += amount;
	Ent_Save();
	return 1;
}

// Timer 60s : incremente compteur employes connectes + verse salaires
forward Ent_Tick();
public Ent_Tick()
{
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		new bool:dirty = false;
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			if(entreprises[eid][entEmpUsed][e] != 1) continue;
			new p = Ent_FindPlayerBySqlid(entreprises[eid][entEmpSqlid][e]);
			if(p == INVALID_PLAYER_ID) continue;
			entreprises[eid][entEmpPendingMin][e]++;
			if(entreprises[eid][entEmpPendingMin][e] < 60) continue;

			// 60 minutes de connexion accumulees : on paie
			new salary = entreprises[eid][entEmpSalary][e];
			if(salary <= 0)
			{
				entreprises[eid][entEmpPendingMin][e] = 0;
				continue;
			}
			if(entreprises[eid][entCash] < salary)
			{
				// Compteur reste a 60 : la paie sera versee des que la caisse
				// se remplira (au prochain tick).
				msg_Client(p, COLOR_INFO, "{FFAA00} Entreprise {FFFFFF} Ton salaire n'a pas pu etre verse : caisse insuffisante.");
				new po = Ent_FindPlayerBySqlid(entreprises[eid][entOwnerSqlid]);
				if(po != INVALID_PLAYER_ID)
				{
					new msgOwn[160];
					format(msgOwn, sizeof(msgOwn), "{FFAA00} Entreprise {FFFFFF} Caisse insuffisante pour payer un employe (salaire %d$).", salary);
					msg_Client(po, COLOR_INFO, msgOwn);
				}
				continue;
			}
			entreprises[eid][entCash] -= salary;
			SafeGivePlayerMoney(p, salary, "Salaire entreprise");
			entreprises[eid][entEmpPendingMin][e] = 0;
			dirty = true;
			new msgEmp[160];
			format(msgEmp, sizeof(msgEmp), "{42AAFF} Entreprise {FFFFFF} Salaire horaire recu : {FFFFB2}+%d${FFFFFF} (de {FFFFB2}%s{FFFFFF}).", salary, entreprises[eid][entName]);
			msg_Client(p, COLOR_WHITE, msgEmp);
		}
		if(dirty) Ent_Save();
	}
	return 1;
}

// Reset le compteur d'un employe quand il se connecte (evite paie instantanee
// si le tick tombe pile apres connexion).
stock Ent_OnPlayerConnect(playerid)
{
	new eid, slot;
	if(!Ent_FindEmployeeSlot(PlayerInfo[playerid][pSQLID], eid, slot)) return 0;
	entreprises[eid][entEmpPendingMin][slot] = 0;
	return 1;
}

// ============================================================================
// LOCAUX (interieurs achetables)
// ============================================================================

// Retourne l'eid de l'entreprise dans laquelle se tient le joueur, sinon -1.
// On se base sur le monde virtuel dedie (ENT_VW_BASE + eid), donc deux
// entreprises ayant choisi le meme interieur ne se voient pas entre elles.
stock Ent_GetInside(playerid)
{
	new vw = GetPlayerVirtualWorld(playerid);
	new eid = vw - ENT_VW_BASE;
	if(eid < 0 || eid >= ENT_MAX) return -1;
	if(entreprises[eid][entUsed] != 1) return -1;
	if(entreprises[eid][entIntId] < 1) return -1;
	return eid;
}

// Entreprise dont l'entree est la plus proche du joueur (< 3m), sinon -1.
stock Ent_GetNearestEntry(playerid)
{
	new Float:bestDist = 3.0, best = -1;
	for(new eid = 0; eid < ENT_MAX; eid++)
	{
		if(entreprises[eid][entUsed] != 1) continue;
		if(entreprises[eid][entIntId] < 1) continue;
		new Float:d = GetPlayerDistanceFromPoint(playerid,
			entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ]);
		if(d < bestDist) { bestDist = d; best = eid; }
	}
	return best;
}

stock Ent_ShowTypeDialog(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");

	new body[900], tmp[96], cur[32];
	Ent_TypeName(eid, cur, sizeof(cur));
	format(tmp, sizeof(tmp), "{FFFFFF}Secteur actuel : {FFFFB2}%s{FFFFFF}\n\n", cur);
	strcat(body, tmp);
	for(new k = 0; k < ENT_TYPE_COUNT; k++)
	{
		format(tmp, sizeof(tmp), "%s{FFFFFF}\t{CCCCCC}%s\n", EntTypes[k][etName], EntTypes[k][etDesc]);
		strcat(body, tmp);
	}
	return ShowPlayerDialog(playerid, ENT_DLG_TYPE, DIALOG_STYLE_TABLIST,
		"{42AAFF}Entreprise - Secteur d'activite", body, "Choisir", "Annuler");
}

stock Ent_SetType(playerid, choice)
{
	if(choice < 0 || choice >= ENT_TYPE_COUNT) return 0;
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");

	entreprises[eid][entType] = choice + 1;
	if(entreprises[eid][entIntId] >= 1) Ent_CreateVisuals(eid);  // enseigne a jour
	Ent_Save();

	new okStr[190];
	format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} Secteur d'activite : {FFFFB2}%s{FFFFFF} (%s).",
		EntTypes[choice][etName], EntTypes[choice][etDesc]);
	return msg_Client(playerid, COLOR_WHITE, okStr);
}

stock Ent_ShowLocalDialog(playerid)
{
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");

	new body[1200], tmp[96];
	if(entreprises[eid][entIntId] >= 1)
	{
		format(tmp, sizeof(tmp), "{FFFFFF}Local actuel : {FFFFB2}%s{FFFFFF}\nChanger de local repose l'entree ici.\n\n",
			EntInteriors[entreprises[eid][entIntId]-1][eiName]);
		strcat(body, tmp);
	}
	else
		strcat(body, "{FFFFFF}L'entree sera posee la ou tu te trouves.\n\n");

	for(new k = 0; k < ENT_INT_COUNT; k++)
	{
		format(tmp, sizeof(tmp), "%s{FFFFFF} - {FFFFB2}%d${FFFFFF}\n",
			EntInteriors[k][eiName], EntInteriors[k][eiPrice]);
		strcat(body, tmp);
	}
	return ShowPlayerDialog(playerid, ENT_DLG_LOCAL, DIALOG_STYLE_LIST,
		"{42AAFF}Entreprise - Choisir un local", body, "Acheter", "Annuler");
}

stock Ent_BuyInterior(playerid, choice)
{
	if(choice < 0 || choice >= ENT_INT_COUNT) return 0;
	new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
	if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");

	// L'entree doit etre posee dehors : un local dont la porte est dans un autre
	// interieur serait inaccessible aux clients et aux employes.
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Va dehors (rue) pour poser l'entree de ton local.");

	new locPrice = EntInteriors[choice][eiPrice];
	if(PlayerInfo[playerid][pCash] < locPrice)
	{
		new msg[160];
		format(msg, sizeof(msg), "{CF9756} Entreprise {FFFFFF} Ce local coute {FFFFB2}%d${FFFFFF}, tu n'as pas assez en poche.", locPrice);
		return msg_Client(playerid, COLOR_INFO, msg);
	}

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	SafeGivePlayerMoney(playerid, -locPrice, "Local entreprise");

	entreprises[eid][entIntId]  = choice + 1;
	entreprises[eid][entEntryX] = x;
	entreprises[eid][entEntryY] = y;
	entreprises[eid][entEntryZ] = z;
	Ent_CreateVisuals(eid);
	Ent_Save();

	new okStr[220];
	format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} Local {FFFFB2}%s{FFFFFF} achete (-%d$). L'entree est posee ici : tape {FFFFB2}/entreprise entrer{FFFFFF} devant.",
		EntInteriors[choice][eiName], locPrice);
	return msg_Client(playerid, COLOR_WHITE, okStr);
}

stock Ent_Cmd_Entrer(playerid)
{
	new eid = Ent_GetNearestEntry(playerid);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Aucune entree d'entreprise a proximite.");
	new iid = entreprises[eid][entIntId];
	if(iid < 1 || iid > ENT_INT_COUNT)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Cette entreprise n'a pas de local.");

	SetPlayerInterior(playerid, EntInteriors[iid-1][eiInt]);
	SetPlayerVirtualWorld(playerid, ENT_VW_BASE + eid);
	SafeSetPlayerPos(playerid, EntInteriors[iid-1][eiX], EntInteriors[iid-1][eiY], EntInteriors[iid-1][eiZ]);
	SetPlayerFacingAngle(playerid, EntInteriors[iid-1][eiA]);
	SetCameraBehindPlayer(playerid);

	new msg[160];
	format(msg, sizeof(msg), "{42AAFF} Entreprise {FFFFFF} Bienvenue chez {FFFFB2}%s{FFFFFF}. ({FFFFB2}/entreprise sortir{FFFFFF})", entreprises[eid][entName]);
	return msg_Client(playerid, COLOR_WHITE, msg);
}

stock Ent_Cmd_Sortir(playerid)
{
	new eid = Ent_GetInside(playerid);
	if(eid == -1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas dans un local d'entreprise.");
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
	SafeSetPlayerPos(playerid, entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ] + 0.5);
	SetCameraBehindPlayer(playerid);
	return msg_Client(playerid, COLOR_WHITE, "{42AAFF} Entreprise {FFFFFF} Tu quittes le local.");
}

// Dispatcher dialog (a appeler depuis OnDialogResponse)
stock Ent_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == ENT_DLG_LOCAL)
	{
		if(!response) return 1;
		Ent_BuyInterior(playerid, listitem);
		return 1;
	}
	if(dialogid == ENT_DLG_TYPE)
	{
		if(!response) return 1;
		Ent_SetType(playerid, listitem);
		return 1;
	}
	return 0;
}

// Dispatcher principal /entreprise
stock Ent_Cmd(playerid, params[])
{
	if(!strlen(params))
	{
		msg_Client(playerid, COLOR_WHITE, "{42AAFF} Entreprise {FFFFFF} /entreprise [info|creer|verser|retirer|embauche|virer|salaire|employes|camionnette|supprimer]");
		msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Identite : /entreprise type (secteur d'activite)");
		msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Local : /entreprise local (acheter un interieur), entrer, sortir");
		msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Missions : /cargo (dans une camionnette d'entreprise, pres d'un depot)");
		if(PlayerInfo[playerid][pAdmin] >= 3)
			msg_Client(playerid, COLOR_WHITE, "{FFFFFF}Admin : /entreprise lister/transferer, /cargo creer/supprimer/lister");
		return 1;
	}

	new sub[20], rest[128];
	new i = 0, p = 0;
	while(p < 19 && params[i] && params[i] != ' ') { sub[p++] = params[i++]; }
	sub[p] = 0;
	while(params[i] == ' ') i++;
	p = 0;
	while(p < 127 && params[i]) { rest[p++] = params[i++]; }
	rest[p] = 0;

	// ===== creer =====
	if(strcmp(sub, "creer", true) == 0)
	{
		if(strlen(rest) < 3)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise creer <nom>");
		if(Ent_FindByOwner(PlayerInfo[playerid][pSQLID]) != -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu as deja une entreprise. Dissous-la d'abord.");
		if(Ent_FindByName(rest) != -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Ce nom est deja pris.");
		if(PlayerInfo[playerid][pCash] < ENT_CREATE_COST)
		{
			new msg[128];
			format(msg, sizeof(msg), "{CF9756} Entreprise {FFFFFF} Creation : {FFFFB2}%d${FFFFFF}. Tu n'as pas assez.", ENT_CREATE_COST);
			return msg_Client(playerid, COLOR_INFO, msg);
		}
		if(ent_Count >= ENT_MAX)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Limite globale d'entreprises atteinte.");
		new eid = -1;
		for(new k = 0; k < ENT_MAX; k++) { if(entreprises[k][entUsed] == 0) { eid = k; break; } }
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Pas de slot libre.");

		SafeGivePlayerMoney(playerid, -ENT_CREATE_COST, "Creation entreprise");
		entreprises[eid][entUsed] = 1;
		new n = 0; while(n < 63 && rest[n]) { entreprises[eid][entName][n] = rest[n]; n++; } entreprises[eid][entName][n] = 0;
		entreprises[eid][entOwnerSqlid] = PlayerInfo[playerid][pSQLID];
		entreprises[eid][entCash] = 0;
		entreprises[eid][entCreatedTs] = gettime();
		entreprises[eid][entType]  = 0;   // secteur a choisir juste apres
		entreprises[eid][entIntId] = 0;   // pas de local tant qu'il n'a pas achete
		entreprises[eid][entEntryX] = 0.0;
		entreprises[eid][entEntryY] = 0.0;
		entreprises[eid][entEntryZ] = 0.0;
		for(new e = 0; e < ENT_MAX_EMP; e++) { entreprises[eid][entEmpUsed][e] = 0; }
		ent_Count++;
		Ent_Save();
		new okStr[220];
		format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} {FFFFB2}%s{FFFFFF} creee (-%d$). Choisis ton secteur d'activite, puis /entreprise local pour un local et /entreprise embauche pour recruter.", entreprises[eid][entName], ENT_CREATE_COST);
		msg_Client(playerid, COLOR_WHITE, okStr);
		// Enchaine directement sur le choix du secteur d'activite.
		return Ent_ShowTypeDialog(playerid);
	}

	// ===== type / local / entrer / sortir =====
	if(strcmp(sub, "type", true) == 0 || strcmp(sub, "secteur", true) == 0
		|| strcmp(sub, "activite", true) == 0)
		return Ent_ShowTypeDialog(playerid);
	if(strcmp(sub, "local", true) == 0 || strcmp(sub, "interieur", true) == 0)
		return Ent_ShowLocalDialog(playerid);
	if(strcmp(sub, "entrer", true) == 0)
		return Ent_Cmd_Entrer(playerid);
	if(strcmp(sub, "sortir", true) == 0)
		return Ent_Cmd_Sortir(playerid);

	// ===== camionnette (achat vehicule cargo) - "camion" garde en alias =====
	if(strcmp(sub, "camionnette", true) == 0 || strcmp(sub, "camion", true) == 0)
	{
		return Cargo_BuyTruck(playerid);
	}

	// ===== info =====
	if(strcmp(sub, "info", true) == 0)
	{
		new mySqlid = PlayerInfo[playerid][pSQLID];
		new eid = Ent_FindByOwner(mySqlid);
		new bool:asOwner = (eid != -1);
		if(!asOwner)
		{
			new dummySlot;
			Ent_FindEmployeeSlot(mySqlid, eid, dummySlot);
		}
		if(eid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'as pas d'entreprise ni d'emploi.");

		new nbEmp = 0;
		for(new e = 0; e < ENT_MAX_EMP; e++)
			if(entreprises[eid][entEmpUsed][e] == 1) nbEmp++;

		new ownerName[MAX_PLAYER_NAME]; strcat(ownerName, "hors ligne");
		new po = Ent_FindPlayerBySqlid(entreprises[eid][entOwnerSqlid]);
		if(po != INVALID_PLAYER_ID) GetPlayerName(po, ownerName, sizeof(ownerName));

		new typeStr[32]; Ent_TypeName(eid, typeStr, sizeof(typeStr));
		new localStr[32]; strcat(localStr, "aucun");
		if(entreprises[eid][entIntId] >= 1)
		{
			localStr[0] = 0;
			strcat(localStr, EntInteriors[entreprises[eid][entIntId]-1][eiName], sizeof(localStr));
		}

		new body[500];
		format(body, sizeof(body),
			"{FFFFFF}Nom : {FFFFB2}%s{FFFFFF}\nSecteur : {FFFFB2}%s{FFFFFF}\nProprio : %s\nCaisse : {FFFFB2}%d${FFFFFF}\nEmployes : %d / %d\nLocal : %s\nRole : %s",
			entreprises[eid][entName], typeStr, ownerName, entreprises[eid][entCash], nbEmp, ENT_MAX_EMP,
			localStr, asOwner ? "PATRON" : "EMPLOYE");
		return ShowPlayerDialog(playerid, 9997, DIALOG_STYLE_MSGBOX, "Entreprise", body, "OK", "");
	}

	// ===== verser =====
	if(strcmp(sub, "verser", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		new montant = strval(rest);
		if(montant <= 0) return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise verser <montant>");
		if(PlayerInfo[playerid][pCash] < montant)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'as pas assez en poche.");
		SafeGivePlayerMoney(playerid, -montant, "Depot entreprise");
		entreprises[eid][entCash] += montant;
		Ent_Save();
		new msg[128];
		format(msg, sizeof(msg), "{42AAFF} Entreprise {FFFFFF} +%d$ dans la caisse (total : %d$).", montant, entreprises[eid][entCash]);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}

	// ===== retirer =====
	if(strcmp(sub, "retirer", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		new montant = strval(rest);
		if(montant <= 0) return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise retirer <montant>");
		if(entreprises[eid][entCash] < montant)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Caisse insuffisante.");
		entreprises[eid][entCash] -= montant;
		SafeGivePlayerMoney(playerid, montant, "Retrait entreprise");
		Ent_Save();
		new msg[128];
		format(msg, sizeof(msg), "{42AAFF} Entreprise {FFFFFF} -%d$ retires (caisse : %d$).", montant, entreprises[eid][entCash]);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}

	// ===== embauche =====
	if(strcmp(sub, "embauche", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		// Format : "<part_nom> <salaire>"
		new targetPart[MAX_PLAYER_NAME], salaireStr[32];
		new ri = 0, rp = 0;
		while(rp < 23 && rest[ri] && rest[ri] != ' ') { targetPart[rp++] = rest[ri++]; }
		targetPart[rp] = 0;
		while(rest[ri] == ' ') ri++;
		rp = 0;
		while(rp < 31 && rest[ri]) { salaireStr[rp++] = rest[ri++]; }
		salaireStr[rp] = 0;
		if(strlen(targetPart) < 1 || strlen(salaireStr) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise embauche <joueur> <salaire_horaire>");
		new target = ReturnUser(targetPart);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Joueur introuvable.");
		new tSqlid = PlayerInfo[target][pSQLID];
		if(tSqlid == entreprises[eid][entOwnerSqlid])
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu ne peux pas t'embaucher toi-meme.");
		new otherEid, otherSlot;
		if(Ent_FindEmployeeSlot(tSqlid, otherEid, otherSlot))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Ce joueur est deja employe ailleurs.");
		if(Ent_FindByOwner(tSqlid) != -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Ce joueur possede deja une entreprise.");
		new salaire = strval(salaireStr);
		if(salaire < 100 || salaire > 500000)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Salaire horaire entre 100$ et 500.000$.");
		new slot = Ent_AllocEmpSlot(eid);
		if(slot == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Plus de place dans l'entreprise.");
		entreprises[eid][entEmpSqlid][slot]      = tSqlid;
		entreprises[eid][entEmpSalary][slot]     = salaire;
		entreprises[eid][entEmpUsed][slot]       = 1;
		entreprises[eid][entEmpPendingMin][slot] = 0;
		Ent_Save();
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new msg1[160], msg2[160];
		format(msg1, sizeof(msg1), "{42AAFF} Entreprise {FFFFFF} {FFFFB2}%s{FFFFFF} embauche a {FFFFB2}%d$/h{FFFFFF}.", tname, salaire);
		msg_Client(playerid, COLOR_WHITE, msg1);
		format(msg2, sizeof(msg2), "{42AAFF} Entreprise {FFFFFF} Tu es embauche par {FFFFB2}%s{FFFFFF} ({FFFFB2}%d$/h{FFFFFF} apres 60 min de connexion).", entreprises[eid][entName], salaire);
		msg_Client(target, COLOR_WHITE, msg2);
		return 1;
	}

	// ===== virer =====
	if(strcmp(sub, "virer", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		if(strlen(rest) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise virer <joueur>");
		new target = ReturnUser(rest);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Joueur introuvable.");
		new otherEid, slot;
		if(!Ent_FindEmployeeSlot(PlayerInfo[target][pSQLID], otherEid, slot) || otherEid != eid)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Ce joueur n'est pas de tes employes.");
		entreprises[eid][entEmpUsed][slot] = 0;
		Ent_Save();
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new msg1[160];
		format(msg1, sizeof(msg1), "{42AAFF} Entreprise {FFFFFF} {FFFFB2}%s{FFFFFF} vire de l'entreprise.", tname);
		msg_Client(playerid, COLOR_WHITE, msg1);
		msg_Client(target, COLOR_WHITE, "{FF3333} Entreprise {FFFFFF} Tu as ete licencie de ton entreprise.");
		return 1;
	}

	// ===== salaire =====
	if(strcmp(sub, "salaire", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		new targetPart[MAX_PLAYER_NAME], salaireStr[32];
		new ri = 0, rp = 0;
		while(rp < 23 && rest[ri] && rest[ri] != ' ') { targetPart[rp++] = rest[ri++]; }
		targetPart[rp] = 0;
		while(rest[ri] == ' ') ri++;
		rp = 0;
		while(rp < 31 && rest[ri]) { salaireStr[rp++] = rest[ri++]; }
		salaireStr[rp] = 0;
		if(strlen(targetPart) < 1 || strlen(salaireStr) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise salaire <joueur> <nouveau_salaire>");
		new target = ReturnUser(targetPart);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Joueur introuvable.");
		new otherEid, slot;
		if(!Ent_FindEmployeeSlot(PlayerInfo[target][pSQLID], otherEid, slot) || otherEid != eid)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Ce joueur n'est pas de tes employes.");
		new salaire = strval(salaireStr);
		if(salaire < 100 || salaire > 500000)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Salaire entre 100 et 500000.");
		entreprises[eid][entEmpSalary][slot] = salaire;
		Ent_Save();
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new msg1[160];
		format(msg1, sizeof(msg1), "{42AAFF} Entreprise {FFFFFF} Salaire de {FFFFB2}%s{FFFFFF} : %d$/h.", tname, salaire);
		msg_Client(playerid, COLOR_WHITE, msg1);
		new msg2[160];
		format(msg2, sizeof(msg2), "{42AAFF} Entreprise {FFFFFF} Ton salaire est desormais {FFFFB2}%d$/h{FFFFFF}.", salaire);
		msg_Client(target, COLOR_WHITE, msg2);
		return 1;
	}

	// ===== employes =====
	if(strcmp(sub, "employes", true) == 0)
	{
		new mySqlid = PlayerInfo[playerid][pSQLID];
		new eid = Ent_FindByOwner(mySqlid);
		if(eid == -1)
		{
			new dummySlot;
			Ent_FindEmployeeSlot(mySqlid, eid, dummySlot);
		}
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Aucune entreprise associee.");
		new body[1024];
		strcat(body, "{FFFFFF}Employes :\n\n");
		new tmp[128], c = 0;
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			if(entreprises[eid][entEmpUsed][e] != 1) continue;
			new empName[MAX_PLAYER_NAME]; strcat(empName, "hors ligne");
			new pp = Ent_FindPlayerBySqlid(entreprises[eid][entEmpSqlid][e]);
			if(pp != INVALID_PLAYER_ID) GetPlayerName(pp, empName, sizeof(empName));
			format(tmp, sizeof(tmp), "SQL %d - %s : %d$/h\n", entreprises[eid][entEmpSqlid][e], empName, entreprises[eid][entEmpSalary][e]);
			strcat(body, tmp);
			c++;
		}
		if(c == 0) strcat(body, "Aucun employe.");
		return ShowPlayerDialog(playerid, 9996, DIALOG_STYLE_MSGBOX, "Employes entreprise", body, "OK", "");
	}

	// ===== supprimer =====
	if(strcmp(sub, "supprimer", true) == 0 || strcmp(sub, "dissoudre", true) == 0)
	{
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1) return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu n'es pas patron.");
		new refund = entreprises[eid][entCash] - ENT_DISSOLVE_FEE;
		if(refund < 0) refund = 0;
		SafeGivePlayerMoney(playerid, refund, "Dissolution entreprise");
		// Notif employes
		for(new e = 0; e < ENT_MAX_EMP; e++)
		{
			if(entreprises[eid][entEmpUsed][e] != 1) continue;
			new pp = Ent_FindPlayerBySqlid(entreprises[eid][entEmpSqlid][e]);
			if(pp != INVALID_PLAYER_ID)
				msg_Client(pp, COLOR_WHITE, "{FF3333} Entreprise {FFFFFF} Ton entreprise a ete dissoute par son patron.");
			entreprises[eid][entEmpUsed][e] = 0;
		}
		// Sortir tout le monde du local avant de le supprimer, sinon les joueurs
		// restent bloques dans un monde virtuel qui n'existe plus.
		if(entreprises[eid][entIntId] >= 1)
		{
			for(new pp = 0; pp < MAX_PLAYERS_CURRENT + 1; pp++)
			{
				if(!IsPlayerConnected(pp) || IsPlayerNPC(pp)) continue;
				if(GetPlayerVirtualWorld(pp) != ENT_VW_BASE + eid) continue;
				SetPlayerInterior(pp, 0);
				SetPlayerVirtualWorld(pp, 0);
				SafeSetPlayerPos(pp, entreprises[eid][entEntryX], entreprises[eid][entEntryY], entreprises[eid][entEntryZ] + 0.5);
				msg_Client(pp, COLOR_WHITE, "{FF3333} Entreprise {FFFFFF} Le local a ete ferme, tu es sorti.");
			}
		}
		Ent_DestroyVisuals(eid);
		entreprises[eid][entIntId] = 0;
		entreprises[eid][entUsed] = 0;
		ent_Count--; if(ent_Count < 0) ent_Count = 0;
		Ent_Save();
		new msg[160];
		format(msg, sizeof(msg), "{42AAFF} Entreprise {FFFFFF} Dissoute. Caisse recuperee : {FFFFB2}%d${FFFFFF} (retenue %d$).", refund, ENT_DISSOLVE_FEE);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}

	// ===== admin lister =====
	if(strcmp(sub, "lister", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 1)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve staff.");
		new body[2048];
		strcat(body, "{FFFFFF}Entreprises :\n\n");
		new tmp[160], c = 0;
		for(new eid = 0; eid < ENT_MAX; eid++)
		{
			if(entreprises[eid][entUsed] != 1) continue;
			new nbEmp = 0;
			for(new e = 0; e < ENT_MAX_EMP; e++) if(entreprises[eid][entEmpUsed][e] == 1) nbEmp++;
			new lstType[32]; Ent_TypeName(eid, lstType, sizeof(lstType));
			format(tmp, sizeof(tmp), "ID %d - %s [%s] (owner SQL %d) caisse %d$ - %d employes\n",
				eid, entreprises[eid][entName], lstType, entreprises[eid][entOwnerSqlid],
				entreprises[eid][entCash], nbEmp);
			strcat(body, tmp);
			c++;
		}
		if(c == 0) strcat(body, "Aucune entreprise.");
		return ShowPlayerDialog(playerid, 9995, DIALOG_STYLE_MSGBOX, "Entreprises", body, "OK", "");
	}

	// ===== admin transferer =====
	if(strcmp(sub, "transferer", true) == 0)
	{
		if(PlayerInfo[playerid][pAdmin] < 5)
			return msg_Client(playerid, COLOR_INFO, "{FF0000} Admin {FFFFFF} Reserve admin 5+.");
		if(strlen(rest) < 1)
			return msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise transferer <nouveau_proprio>");
		new target = ReturnUser(rest);
		if(target == INVALID_PLAYER_ID || !IsPlayerConnected(target))
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Joueur introuvable.");
		if(Ent_FindByOwner(PlayerInfo[target][pSQLID]) != -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Cible possede deja une entreprise.");
		// Trouve la premiere entreprise dont je suis proche (ou juste la mienne)
		new eid = Ent_FindByOwner(PlayerInfo[playerid][pSQLID]);
		if(eid == -1)
			return msg_Client(playerid, COLOR_INFO, "{CF9756} Entreprise {FFFFFF} Tu dois etre patron pour transferer (utilise /a pour reprendre en admin).");
		entreprises[eid][entOwnerSqlid] = PlayerInfo[target][pSQLID];
		Ent_Save();
		new tname[MAX_PLAYER_NAME]; GetPlayerName(target, tname, sizeof(tname));
		new okStr[160];
		format(okStr, sizeof(okStr), "{42AAFF} Entreprise {FFFFFF} Transferee a {FFFFB2}%s{FFFFFF}.", tname);
		msg_Client(playerid, COLOR_WHITE, okStr);
		msg_Client(target, COLOR_WHITE, "{42AAFF} Entreprise {FFFFFF} Tu es le nouveau patron de l'entreprise.");
		return 1;
	}

	msg_Client(playerid, COLOR_WHITE, "{A98500} Usage {FFFFB2} /entreprise [info|creer|verser|retirer|embauche|virer|salaire|employes|supprimer]");
	return 1;
}
