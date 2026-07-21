/*
	afrp_bienvenue.inc - Bonus de premiere apparition (retention nouveaux joueurs)

	But : eviter qu'un nouveau joueur decouvre l'economie "chere" (prix a
	plusieurs millions) des ses premieres minutes et quitte le serveur.

	A la TOUTE PREMIERE connexion d'un compte (une seule fois, jamais repris) :
	  - 750.000$ de cash offerts pour demarrer (manger, se loger, 1ere arme)
	    sans pouvoir tout debloquer d'un coup (gang 50M, entreprise 5M...).
	  - Un message d'accueil qui CADRE l'economie positivement (le reste se
	    gagne en jouant) au lieu de laisser le joueur tomber seul sur des prix
	    a 9 chiffres.
	  - Un boost de 72h : PAIE HORAIRE DES JOBS DOUBLEE, pour que les premieres
	    heures donnent une impression de progression rapide (periode critique
	    ou le joueur decide de rester ou pas).

	Anti-abus : un seul bonus par COMPTE (SQLID), jamais remis a zero. Stocke
	dans scriptfiles/bienvenue.cfg (une ligne "sqlid|premiere_connexion_ts").
	Pas de nouvelle colonne DB requise.

	Hooks :
	  - Bienvenue_Init() dans OnGameModeInit.
	  - Bienvenue_CheckOnLogin(playerid) apres un login reussi (a cote de
	    DailyBonus_CheckOnLogin).
	  - Bienvenue_PayMultiplier(playerid) applique dans PaieAuto_Tick.
*/

#if defined AFRP_BIENVENUE_INCLUDED
	#endinput
#endif
#define AFRP_BIENVENUE_INCLUDED

#define BV_FILE          "bienvenue.cfg"
#define BV_MAX           10000
#define BV_CASH          750000
#define BV_BOOST_SEC     (72 * 3600)   // 72 heures de boost
#define BV_PAY_MULT      2             // paie horaire doublee pendant le boost
#define BV_DLG           9945

new bv_Sqlid[BV_MAX];
new bv_FirstTs[BV_MAX];
new bv_Count = 0;

stock Bienvenue_Init()
{
	bv_Count = 0;
	if(!fexist(BV_FILE)) return 0;
	new File:f = fopen(BV_FILE, io_read);
	if(!f) return 0;
	new line[32];
	while(fread(f, line))
	{
		if(strlen(line) < 1) continue;
		if(bv_Count >= BV_MAX) break;
		// Format : sqlid|firstTs
		new sep = -1;
		for(new i = 0; line[i]; i++) { if(line[i] == '|') { sep = i; break; } }
		if(sep <= 0) continue;
		new sqlidBuf[16], tsBuf[16];
		new p = 0;
		for(new i = 0; i < sep && p < 15; i++) sqlidBuf[p++] = line[i];
		sqlidBuf[p] = 0;
		p = 0;
		for(new i = sep + 1; line[i] && line[i] != '\r' && line[i] != '\n' && p < 15; i++) tsBuf[p++] = line[i];
		tsBuf[p] = 0;
		new sq = strval(sqlidBuf);
		if(sq < 1) continue;
		bv_Sqlid[bv_Count] = sq;
		bv_FirstTs[bv_Count] = strval(tsBuf);
		bv_Count++;
	}
	fclose(f);
	printf("[Bienvenue] %d compte(s) ont deja recu le bonus de premiere apparition", bv_Count);
	return bv_Count;
}

stock Bienvenue_Find(acctSqlid)
{
	if(acctSqlid < 1) return -1;
	for(new i = 0; i < bv_Count; i++)
		if(bv_Sqlid[i] == acctSqlid) return i;
	return -1;
}

stock Bienvenue_Append(acctSqlid, firstTs)
{
	if(bv_Count >= BV_MAX) return 0;
	bv_Sqlid[bv_Count] = acctSqlid;
	bv_FirstTs[bv_Count] = firstTs;
	bv_Count++;
	// Ajout en fin de fichier (append manuel : relecture + reecriture simple)
	new File:f = fopen(BV_FILE, io_append);
	if(!f) f = fopen(BV_FILE, io_write);
	if(f)
	{
		new line[32];
		format(line, sizeof(line), "%d|%d\n", acctSqlid, firstTs);
		fwrite(f, line);
		fclose(f);
	}
	return 1;
}

// Boost actif = compte connu ET premiere connexion il y a moins de 72h.
stock bool:Bienvenue_BoostActive(playerid)
{
	new idx = Bienvenue_Find(PlayerInfo[playerid][pSQLID]);
	if(idx == -1) return false;
	if((gettime() - bv_FirstTs[idx]) >= BV_BOOST_SEC) return false;
	return true;
}

// Multiplicateur applique a la paie horaire des jobs.
stock Bienvenue_PayMultiplier(playerid)
{
	if(Bienvenue_BoostActive(playerid)) return BV_PAY_MULT;
	return 1;
}

// A appeler apres un login reussi. Ne fait rien si le compte a deja recu le bonus.
stock Bienvenue_CheckOnLogin(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(PlayerInfo[playerid][pSQLID] < 1) return 0;
	if(Bienvenue_Find(PlayerInfo[playerid][pSQLID]) != -1) return 0; // deja recu

	// Premiere apparition : bonus + enregistrement
	SafeGivePlayerMoney(playerid, BV_CASH, "Bonus de premiere apparition");
	Bienvenue_Append(PlayerInfo[playerid][pSQLID], gettime());

	new cashStr[24];
	FormatMoney(BV_CASH, cashStr, sizeof(cashStr));

	new body[512];
	format(body, sizeof(body),
		"{FFFFFF}Bienvenue sur {FFD700}La Vie RolePlay{FFFFFF} !\n\n\
{00FF00}+ %s ${FFFFFF} viennent d'etre deposes sur toi pour bien demarrer.\n\n\
{FFD700}Pendant tes 72 premieres heures :{FFFFFF}\n\
- Ta paie horaire de job est {00FF00}DOUBLEE{FFFFFF}.\n\n\
{CCCCCC}Ne t'inquiete pas des gros prix (gangs, entreprises, port d'arme) :\n\
tout ca se gagne en jouant. Commence par prendre un {FFFF00}job{CCCCCC} et\n\
explore la ville. L'argent vient vite quand on joue.\n\n\
{FFFFFF}Tape {FFFF00}/aide{FFFFFF} a tout moment si tu es perdu. Bon jeu !",
		cashStr);

	ShowPlayerDialog(playerid, BV_DLG, DIALOG_STYLE_MSGBOX, "{FFD700}>> Cadeau de bienvenue <<", body, "C'est parti !", "");

	new chatMsg[200];
	format(chatMsg, sizeof(chatMsg),
		"{FFD700}[Bienvenue]{FFFFFF} Cadeau de premiere apparition : {00FF00}+%s ${FFFFFF} + paie de job {00FF00}doublee pendant 72h{FFFFFF} !",
		cashStr);
	SendClientMessage(playerid, -1, chatMsg);
	PlayerPlaySound(playerid, 1150, 0.0, 0.0, 0.0);
	return 1;
}
