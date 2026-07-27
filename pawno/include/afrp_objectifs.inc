/*
	afrp_objectifs.inc - Objectifs quotidiens

	Le bonus de connexion (afrp_dailybonus.inc) recompense le fait de SE
	CONNECTER. Ces objectifs recompensent le fait de JOUER : il faut les
	accomplir. C'est la difference entre un joueur qui passe prendre son
	argent et repart, et un joueur qui reste.

	Chaque jour, 3 objectifs tires au sort parmi 6. Chacun paie
	independamment des autres, plus une prime si les 3 sont accomplis.
	Un joueur qui ne peut pas faire un objectif (pas policier, pas en
	entreprise...) touche quand meme les deux autres : c'est volontaire,
	sinon un civil serait bloque des qu'il tire "arrestations".

	Les objectifs pointent vers des systemes existants (cargo, commerce,
	arrestations, metiers) : ils servent aussi de vitrine a des
	fonctionnalites que beaucoup de joueurs ignorent.

	Stockage : scriptfiles/objectifs.cfg, une ligne par joueur et par jour.
	Les lignes des jours precedents sont ecartees au chargement, le fichier
	ne grossit donc pas indefiniment. Pas de colonne SQL a ajouter.
*/

#if defined AFRP_OBJECTIFS_INCLUDED
	#endinput
#endif
#define AFRP_OBJECTIFS_INCLUDED

#define OBJ_DLG            9945
#define OBJ_FILE           "objectifs.cfg"
#define OBJ_PER_DAY        3            // objectifs tires chaque jour
#define OBJ_TYPE_COUNT     6            // taille du catalogue
#define OBJ_MAX_LINES      600          // lignes lues au chargement
#define OBJ_BONUS_ALL      150000       // prime si les 3 sont accomplis

// Types d'objectifs (l'ordre fixe l'index dans ObjCatalog)
#define OBJ_TEMPS          0            // minutes de jeu
#define OBJ_CARGO          1            // livraisons cargo
#define OBJ_VENTE          2            // unites de marchandises vendues
#define OBJ_ARREST         3            // arrestations (police/FBI)
#define OBJ_ARGENT         4            // argent gagne par le travail
#define OBJ_KM             5            // metres parcourus en vehicule

enum e_ObjDef
{
	odName[48],
	odCible,                 // quantite a atteindre
	odRecompense
};
static const ObjCatalog[OBJ_TYPE_COUNT][e_ObjDef] = {
	{"Jouer 45 minutes",                    45,    60000},
	{"Reussir 2 livraisons cargo",           2,   120000},
	{"Vendre 150 marchandises",            150,   120000},
	{"Proceder a 2 arrestations",            2,   120000},
	{"Gagner 50.000$ en travaillant",    50000,    90000},
	{"Parcourir 8 km en vehicule",        8000,    70000}
};

// --- Panneau textdraw ---
// Place sous le HUD vie/armure (x 548, y 60-100 dans afrp_hud.inc) pour
// regrouper les informations du joueur du meme cote de l'ecran.
#define OBJ_TD_X          496.0
#define OBJ_TD_Y          125.0
#define OBJ_TD_LINE_H     10.0

new PlayerText:objTdBox[MAX_PLAYERS];
new PlayerText:objTdTitle[MAX_PLAYERS];
new PlayerText:objTdLine[MAX_PLAYERS][OBJ_PER_DAY];
new obj_TdVisible[MAX_PLAYERS];
new obj_TdCreated[MAX_PLAYERS];

// Etat en memoire du joueur connecte
new obj_Type[MAX_PLAYERS][OBJ_PER_DAY];
new obj_Prog[MAX_PLAYERS][OBJ_PER_DAY];
new obj_Done[MAX_PLAYERS][OBJ_PER_DAY];   // 1 = recompense versee
new obj_BonusPris[MAX_PLAYERS];
new obj_Day[MAX_PLAYERS];                 // jour epoch des objectifs charges
new obj_Loaded[MAX_PLAYERS];
new Float:obj_LastX[MAX_PLAYERS], Float:obj_LastY[MAX_PLAYERS];

stock Obj_Today() { return gettime() / 86400; }

stock Obj_ReadField(const line[], &i, dest[], destSize, maxCopy)
{
	new p = 0;
	while(p < maxCopy && p < destSize - 1 && line[i] && line[i] != '|' && line[i] != '\r' && line[i] != '\n')
		dest[p++] = line[i++];
	dest[p] = 0;
	if(line[i] == '|') i++;
	return p;
}

// Tire OBJ_PER_DAY objectifs DIFFERENTS parmi le catalogue.
stock Obj_Generate(playerid)
{
	new pris[OBJ_TYPE_COUNT];
	for(new k = 0; k < OBJ_TYPE_COUNT; k++) pris[k] = 0;

	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		new t, garde = 0;
		do
		{
			t = random(OBJ_TYPE_COUNT);
			garde++;
		}
		while(pris[t] == 1 && garde < 50);
		// Filet : si le tirage n'a pas trouve de libre, on prend le premier dispo.
		if(pris[t] == 1)
		{
			for(new k = 0; k < OBJ_TYPE_COUNT; k++) { if(pris[k] == 0) { t = k; break; } }
		}
		pris[t] = 1;
		obj_Type[playerid][s] = t;
		obj_Prog[playerid][s] = 0;
		obj_Done[playerid][s] = 0;
	}
	obj_BonusPris[playerid] = 0;
	obj_Day[playerid] = Obj_Today();
	return 1;
}

// Sauvegarde : on reecrit le fichier en gardant les lignes du jour des AUTRES
// joueurs et en remplacant la sienne. Les jours passes sont abandonnes, ce qui
// empeche le fichier de grossir sans fin.
stock Obj_Save(playerid)
{
	if(obj_Loaded[playerid] != 1) return 0;
	new mySqlid = PlayerInfo[playerid][pSQLID];
	if(mySqlid < 1) return 0;

	new today = Obj_Today();
	new lignes[OBJ_MAX_LINES][96];
	new nb = 0;

	if(fexist(OBJ_FILE))
	{
		new File:fr = fopen(OBJ_FILE, io_read);
		if(fr)
		{
			new line[96], tmpBuf[24];
			while(fread(fr, line) && nb < OBJ_MAX_LINES)
			{
				if(strlen(line) < 5) continue;
				new i = 0;
				Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23);
				new lSqlid = strval(tmpBuf);
				Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23);
				new lDay = strval(tmpBuf);
				if(lDay != today) continue;       // jour perime : on abandonne
				if(lSqlid == mySqlid) continue;   // sa propre ligne : reecrite plus bas
				format(lignes[nb], 96, "%s", line);
				nb++;
			}
			fclose(fr);
		}
	}

	new File:fw = fopen(OBJ_FILE, io_write);
	if(!fw) return 0;
	for(new k = 0; k < nb; k++)
	{
		// Les lignes relues gardent deja leur retour a la ligne eventuel.
		if(strfind(lignes[k], "\n") == -1) strcat(lignes[k], "\n");
		fwrite(fw, lignes[k]);
	}
	new outLine[96];
	format(outLine, sizeof(outLine), "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d\n",
		mySqlid, obj_Day[playerid],
		obj_Type[playerid][0], obj_Prog[playerid][0], obj_Done[playerid][0],
		obj_Type[playerid][1], obj_Prog[playerid][1], obj_Done[playerid][1],
		obj_Type[playerid][2], obj_Prog[playerid][2], obj_Done[playerid][2],
		obj_BonusPris[playerid]);
	fwrite(fw, outLine);
	fclose(fw);
	return 1;
}

stock Obj_Load(playerid)
{
	new mySqlid = PlayerInfo[playerid][pSQLID];
	obj_Loaded[playerid] = 1;
	obj_LastX[playerid] = 0.0;
	obj_LastY[playerid] = 0.0;
	if(mySqlid < 1) { Obj_Generate(playerid); return 0; }

	new today = Obj_Today();
	if(fexist(OBJ_FILE))
	{
		new File:f = fopen(OBJ_FILE, io_read);
		if(f)
		{
			new line[96], tmpBuf[24];
			while(fread(f, line))
			{
				if(strlen(line) < 5) continue;
				new i = 0;
				Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23);
				if(strval(tmpBuf) != mySqlid) continue;
				Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23);
				if(strval(tmpBuf) != today) continue;   // objectifs d'hier : on retire

				for(new s = 0; s < OBJ_PER_DAY; s++)
				{
					Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23); obj_Type[playerid][s] = strval(tmpBuf);
					Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23); obj_Prog[playerid][s] = strval(tmpBuf);
					Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23); obj_Done[playerid][s] = strval(tmpBuf);
					if(obj_Type[playerid][s] < 0 || obj_Type[playerid][s] >= OBJ_TYPE_COUNT)
						obj_Type[playerid][s] = 0;
				}
				Obj_ReadField(line, i, tmpBuf, sizeof(tmpBuf), 23); obj_BonusPris[playerid] = strval(tmpBuf);
				obj_Day[playerid] = today;
				fclose(f);
				return 1;
			}
			fclose(f);
		}
	}
	Obj_Generate(playerid);
	return 0;
}

stock Obj_OnPlayerConnect(playerid)
{
	obj_TdCreated[playerid] = 0;
	obj_TdVisible[playerid] = 0;
	obj_Loaded[playerid] = 0;
	obj_BonusPris[playerid] = 0;
	obj_Day[playerid] = 0;
	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		obj_Type[playerid][s] = 0;
		obj_Prog[playerid][s] = 0;
		obj_Done[playerid][s] = 0;
	}
	return 1;
}

// A appeler apres un login reussi (le SQLID est alors connu).
stock Obj_OnPlayerLogin(playerid)
{
	Obj_Load(playerid);
	new nbRestants = 0;
	for(new s = 0; s < OBJ_PER_DAY; s++)
		if(obj_Done[playerid][s] == 0) nbRestants++;
	if(nbRestants > 0)
	{
		new msg[190];
		format(msg, sizeof(msg), "{FFD700} Objectifs {FFFFFF} Tu as {FFFFB2}%d{FFFFFF} objectif(s) a accomplir aujourd'hui. Tape {FFFFB2}/objectifs{FFFFFF}.", nbRestants);
		msg_Client(playerid, COLOR_WHITE, msg);
	}
	// Panneau affiche d'office : son interet est justement d'etre vu sans
	// que le joueur ait a demander. /objectifs hud le cache s'il gene.
	Obj_TdShow(playerid);
	return 1;
}

stock Obj_OnPlayerDisconnect(playerid)
{
	if(obj_Loaded[playerid] == 1) Obj_Save(playerid);
	Obj_TdDestroy(playerid);
	obj_Loaded[playerid] = 0;
	return 1;
}

// Coeur du systeme : fait avancer un objectif du joueur.
stock Obj_Progress(playerid, objType, montant)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(obj_Loaded[playerid] != 1) return 0;
	if(montant <= 0) return 0;

	// Changement de jour en pleine session : on retire de nouveaux objectifs.
	if(obj_Day[playerid] != Obj_Today())
	{
		Obj_Generate(playerid);
		Obj_TdRefresh(playerid);
		msg_Client(playerid, COLOR_WHITE, "{FFD700} Objectifs {FFFFFF} Nouvelle journee : tes objectifs ont ete renouveles ({FFFFB2}/objectifs{FFFFFF}).");
	}

	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		if(obj_Type[playerid][s] != objType) continue;
		if(obj_Done[playerid][s] == 1) continue;

		new cible = ObjCatalog[objType][odCible];
		obj_Prog[playerid][s] += montant;
		if(obj_Prog[playerid][s] < cible)
		{
			// Retour discret : on n'inonde pas le chat a chaque unite vendue.
			new avant = obj_Prog[playerid][s] - montant;
			new pctAvant = (avant * 100) / cible;
			new pctApres = (obj_Prog[playerid][s] * 100) / cible;
			if(pctApres >= 50 && pctAvant < 50)
			{
				new mid[190];
				format(mid, sizeof(mid), "{FFD700} Objectif {FFFFFF} %s : {FFFFB2}%d/%d{FFFFFF} (moitie faite).",
					ObjCatalog[objType][odName], obj_Prog[playerid][s], cible);
				msg_Client(playerid, COLOR_WHITE, mid);
			}
			Obj_TdRefresh(playerid);   // le panneau suit la progression en direct
			return 1;
		}

		// Objectif accompli
		obj_Prog[playerid][s] = cible;
		obj_Done[playerid][s] = 1;
		new gain = ObjCatalog[objType][odRecompense];
		SafeGivePlayerMoney(playerid, gain, "Objectif quotidien");

		new okStr[200];
		format(okStr, sizeof(okStr), "{FFD700} Objectif accompli {FFFFFF} %s : {FFFFB2}+%d${FFFFFF}",
			ObjCatalog[objType][odName], gain);
		msg_Client(playerid, COLOR_WHITE, okStr);
		PlayerPlaySound(playerid, 1057, 0.0, 0.0, 0.0);
		// [TOP DU JOUR] un objectif accompli vaut plus que 25 minutes de presence :
		// c'est ce qui empeche le classement de recompenser la seule anciennete.
		TopJour_AddPoints(playerid, TJ_PTS_OBJECTIF);

		// Prime si les trois sont faits
		new tousFaits = 1;
		for(new k = 0; k < OBJ_PER_DAY; k++)
			if(obj_Done[playerid][k] == 0) { tousFaits = 0; break; }
		if(tousFaits == 1 && obj_BonusPris[playerid] == 0)
		{
			obj_BonusPris[playerid] = 1;
			SafeGivePlayerMoney(playerid, OBJ_BONUS_ALL, "Prime objectifs du jour");
			new bonusStr[200];
			format(bonusStr, sizeof(bonusStr), "{FFD700} Objectifs {FFFFFF} Les 3 objectifs du jour sont accomplis : prime de {FFFFB2}+%d${FFFFFF} !", OBJ_BONUS_ALL);
			msg_Client(playerid, COLOR_WHITE, bonusStr);
		}
		Obj_Save(playerid);
		Obj_TdRefresh(playerid);
		return 1;
	}
	return 0;
}

// Timer 60s : temps de jeu + distance parcourue en vehicule.
forward Obj_Tick();
public Obj_Tick()
{
	for(new p = 0; p < MAX_PLAYERS_CURRENT + 1; p++)
	{
		if(!IsPlayerConnected(p) || IsPlayerNPC(p)) continue;
		if(obj_Loaded[p] != 1) continue;

		// Le temps de jeu ne compte pas si le joueur est AFK ou en prison :
		// un objectif doit recompenser le jeu, pas le fait de laisser tourner.
		if(PlayerInfo[p][pJailed] == 0)
			Obj_Progress(p, OBJ_TEMPS, 1);

		// Distance parcourue au volant, mesuree entre deux passages du timer.
		new Float:x, Float:y, Float:z;
		GetPlayerPos(p, x, y, z);
		if(GetPlayerState(p) == PLAYER_STATE_DRIVER
			&& obj_LastX[p] != 0.0 && obj_LastY[p] != 0.0)
		{
			new Float:dx = x - obj_LastX[p];
			new Float:dy = y - obj_LastY[p];
			new Float:d = floatsqroot(dx*dx + dy*dy);
			// Au-dela de 3000 m en une minute c'est un teleport, pas un trajet.
			if(d > 1.0 && d < 3000.0) Obj_Progress(p, OBJ_KM, floatround(d));
		}
		obj_LastX[p] = x;
		obj_LastY[p] = y;
	}
	return 1;
}

stock Obj_Init()
{
	for(new p = 0; p < MAX_PLAYERS; p++)
	{
		obj_Loaded[p] = 0;
		obj_LastX[p] = 0.0;
		obj_LastY[p] = 0.0;
	}
	SetTimer("Obj_Tick", 60000, 1);
	return 1;
}

// ============================================================================
// PANNEAU TEXTDRAW
// ============================================================================
stock Obj_TdCreate(playerid)
{
	if(obj_TdCreated[playerid] == 1) return 1;

	// Fond : un textdraw "box" transparent qui sert de cadre aux lignes.
	objTdBox[playerid] = CreatePlayerTextDraw(playerid, OBJ_TD_X - 4.0, OBJ_TD_Y - 3.0, "_");
	PlayerTextDrawUseBox(playerid, objTdBox[playerid], 1);
	PlayerTextDrawBoxColor(playerid, objTdBox[playerid], 0x00000066);
	PlayerTextDrawTextSize(playerid, objTdBox[playerid], 636.0, 0.0);
	PlayerTextDrawLetterSize(playerid, objTdBox[playerid], 0.0, 5.4);
	PlayerTextDrawColor(playerid, objTdBox[playerid], 0x00000000);
	PlayerTextDrawSetOutline(playerid, objTdBox[playerid], 0);
	PlayerTextDrawSetShadow(playerid, objTdBox[playerid], 0);
	PlayerTextDrawFont(playerid, objTdBox[playerid], 1);

	objTdTitle[playerid] = CreatePlayerTextDraw(playerid, OBJ_TD_X, OBJ_TD_Y, "OBJECTIFS DU JOUR");
	PlayerTextDrawFont(playerid, objTdTitle[playerid], 2);
	PlayerTextDrawLetterSize(playerid, objTdTitle[playerid], 0.16, 0.9);
	PlayerTextDrawColor(playerid, objTdTitle[playerid], 0xFFD700FF);
	PlayerTextDrawSetOutline(playerid, objTdTitle[playerid], 1);
	PlayerTextDrawSetShadow(playerid, objTdTitle[playerid], 0);

	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		objTdLine[playerid][s] = CreatePlayerTextDraw(playerid,
			OBJ_TD_X, OBJ_TD_Y + 11.0 + (float(s) * OBJ_TD_LINE_H), "-");
		PlayerTextDrawFont(playerid, objTdLine[playerid][s], 1);
		PlayerTextDrawLetterSize(playerid, objTdLine[playerid][s], 0.19, 0.9);
		PlayerTextDrawColor(playerid, objTdLine[playerid][s], 0xFFFFFFFF);
		PlayerTextDrawSetOutline(playerid, objTdLine[playerid][s], 1);
		PlayerTextDrawSetShadow(playerid, objTdLine[playerid][s], 0);
	}
	obj_TdCreated[playerid] = 1;
	return 1;
}

stock Obj_TdDestroy(playerid)
{
	if(obj_TdCreated[playerid] != 1) return 0;
	PlayerTextDrawDestroy(playerid, objTdBox[playerid]);
	PlayerTextDrawDestroy(playerid, objTdTitle[playerid]);
	for(new s = 0; s < OBJ_PER_DAY; s++)
		PlayerTextDrawDestroy(playerid, objTdLine[playerid][s]);
	obj_TdCreated[playerid] = 0;
	obj_TdVisible[playerid] = 0;
	return 1;
}

// Recalcule le texte des lignes et, si le panneau est visible, l'affiche.
stock Obj_TdRefresh(playerid)
{
	if(obj_TdCreated[playerid] != 1) return 0;
	if(obj_Loaded[playerid] != 1) return 0;

	new ligne[80];
	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		new t = obj_Type[playerid][s];
		if(t < 0 || t >= OBJ_TYPE_COUNT) t = 0;
		new cible = ObjCatalog[t][odCible];
		new prog = obj_Prog[playerid][s];
		if(prog > cible) prog = cible;

		if(obj_Done[playerid][s] == 1)
		{
			format(ligne, sizeof(ligne), "~g~%s", ObjCatalog[t][odName]);
			PlayerTextDrawColor(playerid, objTdLine[playerid][s], 0x66FF99FF);
		}
		else
		{
			format(ligne, sizeof(ligne), "%s  ~y~%d~w~/%d", ObjCatalog[t][odName], prog, cible);
			PlayerTextDrawColor(playerid, objTdLine[playerid][s], 0xFFFFFFFF);
		}
		PlayerTextDrawSetString(playerid, objTdLine[playerid][s], ligne);
	}
	if(obj_TdVisible[playerid] == 1)
	{
		PlayerTextDrawShow(playerid, objTdBox[playerid]);
		PlayerTextDrawShow(playerid, objTdTitle[playerid]);
		for(new s = 0; s < OBJ_PER_DAY; s++)
			PlayerTextDrawShow(playerid, objTdLine[playerid][s]);
	}
	return 1;
}

stock Obj_TdShow(playerid)
{
	Obj_TdCreate(playerid);
	obj_TdVisible[playerid] = 1;
	return Obj_TdRefresh(playerid);
}

stock Obj_TdHide(playerid)
{
	if(obj_TdCreated[playerid] != 1) return 0;
	obj_TdVisible[playerid] = 0;
	PlayerTextDrawHide(playerid, objTdBox[playerid]);
	PlayerTextDrawHide(playerid, objTdTitle[playerid]);
	for(new s = 0; s < OBJ_PER_DAY; s++)
		PlayerTextDrawHide(playerid, objTdLine[playerid][s]);
	return 1;
}

stock Obj_TdToggle(playerid)
{
	if(obj_TdVisible[playerid] == 1)
	{
		Obj_TdHide(playerid);
		return msg_Client(playerid, COLOR_WHITE, "{FFD700} Objectifs {FFFFFF} Panneau cache. {FFFFB2}/objectifs hud{FFFFFF} pour le reafficher.");
	}
	Obj_TdShow(playerid);
	return msg_Client(playerid, COLOR_WHITE, "{FFD700} Objectifs {FFFFFF} Panneau affiche.");
}

stock Obj_ShowDialog(playerid)
{
	if(obj_Loaded[playerid] != 1)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Objectifs {FFFFFF} Tes objectifs ne sont pas encore charges, reessaie dans un instant.");

	if(obj_Day[playerid] != Obj_Today()) Obj_Generate(playerid);

	new body[700], tmp[220], nbFaits = 0;
	strcat(body, "{FFFFFF}Renouveles chaque jour. Chaque objectif paie separement.\n\n");
	for(new s = 0; s < OBJ_PER_DAY; s++)
	{
		new t = obj_Type[playerid][s];
		if(t < 0 || t >= OBJ_TYPE_COUNT) continue;
		new cible = ObjCatalog[t][odCible];
		new prog = obj_Prog[playerid][s];
		if(prog > cible) prog = cible;

		if(obj_Done[playerid][s] == 1)
		{
			format(tmp, sizeof(tmp), "{00FF7F}[FAIT] %s{FFFFFF}  (+%d$)\n", ObjCatalog[t][odName], ObjCatalog[t][odRecompense]);
			nbFaits++;
		}
		else
			format(tmp, sizeof(tmp), "{FFFFFF}%s\n     {CCCCCC}Progression : {FFFFB2}%d{CCCCCC} / %d  -  recompense {FFFFB2}%d${FFFFFF}\n",
				ObjCatalog[t][odName], prog, cible, ObjCatalog[t][odRecompense]);
		strcat(body, tmp);
	}

	if(nbFaits == OBJ_PER_DAY)
		format(tmp, sizeof(tmp), "\n{00FF7F}Les 3 objectifs sont accomplis, prime de %d$ versee.{FFFFFF}", OBJ_BONUS_ALL);
	else
		format(tmp, sizeof(tmp), "\n{CCCCCC}Prime de {FFFFB2}%d${CCCCCC} si tu accomplis les 3 ({FFFFB2}%d/%d{CCCCCC} faits).{FFFFFF}", OBJ_BONUS_ALL, nbFaits, OBJ_PER_DAY);
	strcat(body, tmp);

	return ShowPlayerDialog(playerid, OBJ_DLG, DIALOG_STYLE_MSGBOX,
		"{FFD700}Objectifs du jour", body, "OK", "");
}
