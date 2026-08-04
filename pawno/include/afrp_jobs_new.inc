/*
	afrp_jobs_new.inc - Metiers qui n'existaient que de nom

	POURQUOI CE FICHIER
	GetJobName() annonce 21 metiers, mais la moitie n'avait aucune mission
	codee : le joueur pouvait etre embauche puis ne rien avoir a faire. Deux
	d'entre eux etaient meme deja a moitie branches par une passe precedente
	(Bucheron a son employeur PNJ, son texte d'aide et son /job localiser) :
	il ne manquait que la boucle de jeu. C'est ce que ce module apporte.

	On l'isole ici plutot que de gonfler LVRP.pwn (72 000 lignes) : cote
	LVRP.pwn il ne reste que 3 branchements d'une ligne (debut de job, entree
	de checkpoint, touche 'Y'), tous commentes [JOBS NEW].

	CE QUI EST IMPLANTE
	  - Bucheron (14)  : abattre un arbre, porter le rondin jusqu'au depot.
	  - Jardinier (15) : entretenir les parcelles des villas de Richman.

	CHOIX TECHNIQUE IMPORTANT - ROBUSTESSE DES COORDONNEES
	Ce code a ete ecrit sans acces au jeu : impossible de verifier qu'un point
	choisi est bien sur un sol praticable et pas dans un rocher. On a donc
	volontairement evite tout ce qui casse quand une coordonnee est fausse :
	  - aucun SetPlayerPos (jamais de teleportation : un mauvais Z ne peut pas
	    coincer le joueur dans le decor) ;
	  - aucun objet pose dans le monde (un objet mal place flotterait ou
	    s'enfoncerait) - on n'utilise que SetPlayerAttachedObject, qui suit le
	    joueur et est donc toujours correct ;
	  - des rayons de detection genereux (voir JOBNEW_ACTION_RANGE) pour qu'une
	    erreur de quelques metres en hauteur n'empeche pas de travailler.
	Consequence : au pire une zone de travail est mal placee visuellement, mais
	le metier reste jouable. Toutes les coordonnees sont regroupees dans les
	deux tableaux ci-dessous pour etre corrigees en jeu sans toucher au reste.
*/

#if defined AFRP_JOBS_NEW_INCLUDED
	#endinput
#endif
#define AFRP_JOBS_NEW_INCLUDED

// --- Types de checkpoint (gPlayerCheckpoint) -------------------------------
// Les valeurs 0 a 30 sont deja prises par le reste du gamemode : on continue
// a 31 pour ne rien recouvrir.
#define JOBNEW_CP_BUCHERON_ARBRE   31   // aller couper un arbre
#define JOBNEW_CP_BUCHERON_DEPOT   32   // rapporter le rondin a l'employeur
#define JOBNEW_CP_JARDINIER        33   // entretenir une parcelle

// --- Reglages de la boucle de jeu ------------------------------------------
// Nombre de taches a enchainer avant que le bonus de paye soit verse. Plus le
// chiffre est bas, plus le metier paye souvent (voir docs/equilibrage_jobs.md).
#define JOBNEW_BUCHERON_PAR_PAYE   3    // 3 rondins livres = 1 bonus
#define JOBNEW_JARDINIER_PAR_PAYE  4    // 4 parcelles finies = 1 bonus

// Duree de l'animation de travail, en millisecondes. Le joueur est immobilise
// pendant ce temps : c'est ce qui empeche de spammer la touche 'Y'.
#define JOBNEW_BUCHERON_DUREE      5000
#define JOBNEW_JARDINIER_DUREE     6000

// Distance a laquelle la touche 'Y' est acceptee autour du point de travail.
// Volontairement large : elle absorbe une erreur de hauteur si la coordonnee
// n'est pas exactement au niveau du sol (voir l'entete du fichier).
#define JOBNEW_ACTION_RANGE        10.0

// Rayon du marqueur rouge sur la minimap.
#define JOBNEW_CP_SIZE             5.0

// Modele d'objet porte par le bucheron. 935 est deja utilise par le metier
// Ouvrier avec ces memes decalages : ils sont donc valides a l'ecran.
#define JOBNEW_OBJ_RONDIN          935

// --- Zones de travail du Bucheron ------------------------------------------
// job_City sert d'index de zone : 0 = Blueberry, 1 = Mulholland (Los Santos).
// ATTENTION : l'employeur "Red County" cree par une passe precedente est en
// realite a Mulholland (Los Santos) - 1174/-485 n'est pas dans Red County.
// Le commentaire a ete corrige dans init_Actors, la position n'a pas bouge.
//
// Format : {x, y, z}. Le dernier point de chaque zone est le DEPOT (= la
// position de l'employeur PNJ, donc une coordonnee deja presente dans le
// gamemode et fiable).
#define JOBNEW_BUCHERON_ZONES      2
#define JOBNEW_BUCHERON_ARBRES     5

static const Float:jobnewBucheronArbre[JOBNEW_BUCHERON_ZONES][JOBNEW_BUCHERON_ARBRES][3] = {
	// Zone 0 - Blueberry. Le plateau autour de l'employeur est plat : les
	// points de l'ANPE, de l'employeur Fermier et du depot de paille voisins
	// sont tous a z = 3.1172, on reste sur cette meme altitude.
	{
		{-70.0000,  95.0000, 3.1172},
		{-48.0000,  60.0000, 3.1172},
		{-72.0000,  40.0000, 3.1172},
		{-50.0000,  20.0000, 3.1172},
		{-66.0000,  70.0000, 3.1172}
	},
	// Zone 1 - Mulholland (Los Santos), autour de l'employeur existant.
	{
		{1180.0000, -470.0000, 26.5400},
		{1160.0000, -495.0000, 26.5400},
		{1190.0000, -500.0000, 26.5400},
		{1165.0000, -465.0000, 26.5400},
		{1200.0000, -480.0000, 26.5400}
	}
};

// Depot de chaque zone = position de l'employeur PNJ correspondant.
static const Float:jobnewBucheronDepot[JOBNEW_BUCHERON_ZONES][3] = {
	{ -62.5000,   83.0000,  3.1172},   // Blueberry   (init_Actors)
	{1174.0000, -485.0000, 26.5400}    // Mulholland  (init_Actors)
};

// --- Parcelles du Jardinier ------------------------------------------------
// Coordonnees reprises TELLES QUELLES de gValetPointsLs (LVRP.pwn) : ce sont
// les allees des villas de Richman ou le metier Voiturier gare deja les
// voitures. Elles sont donc certifiees praticables, ce qui evite d'en
// inventer. Un jardinier chez les riches de Richman est en plus coherent.
#define JOBNEW_JARDINIER_PARCELLES 6

static const Float:jobnewJardinierParcelle[JOBNEW_JARDINIER_PARCELLES][3] = {
	{301.0683, -1504.4807, 24.1864},
	{294.5847, -1513.4740, 24.2161},
	{288.3050, -1522.4767, 24.1800},
	{281.6232, -1530.8364, 24.1709},
	{291.4316, -1544.3843, 24.1819},
	{298.4492, -1536.1008, 24.1708}
};

// Employeur Jardinier : a quelques metres de l'employeur Voiturier existant
// (322.1843/-1519.5908/36.0391) et exactement a la meme altitude, donc sur la
// meme terrasse. C'est la seule coordonnee reellement nouvelle de ce module.
#define JOBNEW_JARDINIER_EMPLOYEUR_X   325.5000
#define JOBNEW_JARDINIER_EMPLOYEUR_Y  -1516.5000
#define JOBNEW_JARDINIER_EMPLOYEUR_Z    36.0391

forward JobsNew_Finish(playerid, jobid);

// ---------------------------------------------------------------------------
// Pose le marqueur sur la prochaine tache et dit au joueur quoi faire.
// ---------------------------------------------------------------------------
stock JobsNew_NextSpot(playerid)
{
	new jnJob = PlayerInfo[playerid][pJob];

	if(jnJob == 14)
	{
		new jnZone = job_City[playerid];
		if(jnZone < 0 || jnZone >= JOBNEW_BUCHERON_ZONES) jnZone = 0;

		if(job_ObjectN[playerid] == 1)
		{
			// Rondin en main -> direction le depot.
			gPlayerCheckpoint[playerid] = JOBNEW_CP_BUCHERON_DEPOT;
			SetPlayerCheckpoint(playerid, jobnewBucheronDepot[jnZone][0], jobnewBucheronDepot[jnZone][1], jobnewBucheronDepot[jnZone][2], JOBNEW_CP_SIZE);
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Rapportez le rondin au depot, puis touche 'Y'. (point rouge minimap)");
		}
		else
		{
			// Mains vides -> nouvel arbre tire au hasard dans la zone.
			job_TempVar[playerid] = random(JOBNEW_BUCHERON_ARBRES);
			gPlayerCheckpoint[playerid] = JOBNEW_CP_BUCHERON_ARBRE;
			SetPlayerCheckpoint(playerid, jobnewBucheronArbre[jnZone][job_TempVar[playerid]][0], jobnewBucheronArbre[jnZone][job_TempVar[playerid]][1], jobnewBucheronArbre[jnZone][job_TempVar[playerid]][2], JOBNEW_CP_SIZE);
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Allez a l'arbre marque puis appuyez sur 'Y' pour l'abattre.");
		}
		return 1;
	}

	if(jnJob == 15)
	{
		job_TempVar[playerid] = random(JOBNEW_JARDINIER_PARCELLES);
		gPlayerCheckpoint[playerid] = JOBNEW_CP_JARDINIER;
		SetPlayerCheckpoint(playerid, jobnewJardinierParcelle[job_TempVar[playerid]][0], jobnewJardinierParcelle[job_TempVar[playerid]][1], jobnewJardinierParcelle[job_TempVar[playerid]][2], JOBNEW_CP_SIZE);
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Allez a la parcelle marquee puis appuyez sur 'Y' pour l'entretenir.");
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Demarrage du metier, appele depuis /job debut.
// Renvoie 1 si le metier est gere ici, 0 sinon (pour laisser le code d'origine
// afficher ses propres messages).
// ---------------------------------------------------------------------------
stock JobsNew_Start(playerid)
{
	new jnJob = PlayerInfo[playerid][pJob];
	if(jnJob != 14 && jnJob != 15)
		{return 0;}

	if(job_Start[playerid] == 1)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous etes deja en service.");}

	job_Start[playerid] = 1;
	job_CheckPoints[playerid] = 0;   // taches accomplies depuis le dernier bonus
	job_TempVar[playerid] = 0;       // index du point de travail en cours
	job_ObjectN[playerid] = 0;       // 0 = mains vides, 1 = rondin en main
	job_HoldingObjectSlot[playerid] = -1;

	if(jnJob == 14)
	{
		// On choisit la zone la plus proche du joueur : il commence donc a
		// travailler la ou il vient d'etre embauche, sans traverser la carte.
		new jnZone = 0;
		new Float:jnBest = 999999.0;
		for(new z = 0; z < JOBNEW_BUCHERON_ZONES; z++)
		{
			new Float:jnDist = GetPlayerDistanceFromPoint(playerid, jobnewBucheronDepot[z][0], jobnewBucheronDepot[z][1], jobnewBucheronDepot[z][2]);
			if(jnDist < jnBest) { jnBest = jnDist; jnZone = z; }
		}
		job_City[playerid] = jnZone;
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous prenez votre service de Bucheron.");
	}
	else
	{
		job_City[playerid] = 0;
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous prenez votre service de Jardinier.");
	}

	job_SetSkin(playerid, jnJob);
	job_ShowTexts(playerid);
	JobsNew_NextSpot(playerid);

	if(PlayerInfo[playerid][pJobTime] < JOB_TIME)
	{
		new jnMsg[144];
		format(jnMsg, sizeof(jnMsg), "{78769D} Job {FFFFFF} Vous devez encore travailler %d minute(s) pour obtenir la paye.", JOB_TIME - PlayerInfo[playerid][pJobTime]);
		msg_Client(playerid, COLOR_JOB, jnMsg);
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Entree dans un checkpoint. Renvoie 1 si c'etait un checkpoint de ce module.
// ---------------------------------------------------------------------------
stock JobsNew_OnEnterCheckpoint(playerid)
{
	if(job_Start[playerid] != 1)
		{return 0;}

	switch(gPlayerCheckpoint[playerid])
	{
		case JOBNEW_CP_BUCHERON_ARBRE:
		{
			if(PlayerInfo[playerid][pJob] != 14) return 0;
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous etes au pied de l'arbre : appuyez sur 'Y' pour l'abattre.");
			return 1;
		}
		case JOBNEW_CP_BUCHERON_DEPOT:
		{
			if(PlayerInfo[playerid][pJob] != 14) return 0;
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous etes au depot : appuyez sur 'Y' pour deposer le rondin.");
			return 1;
		}
		case JOBNEW_CP_JARDINIER:
		{
			if(PlayerInfo[playerid][pJob] != 15) return 0;
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous etes sur la parcelle : appuyez sur 'Y' pour l'entretenir.");
			return 1;
		}
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Touche 'Y'. Renvoie 1 si l'action a ete prise en charge ici.
// ---------------------------------------------------------------------------
stock JobsNew_OnKeyYes(playerid)
{
	if(job_Start[playerid] != 1)
		{return 0;}
	// Toutes les actions de ces deux metiers se font a pied.
	if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
		{return 0;}

	new jnJob = PlayerInfo[playerid][pJob];

	// ---------------- Bucheron ----------------
	if(jnJob == 14)
	{
		new jnZone = job_City[playerid];
		if(jnZone < 0 || jnZone >= JOBNEW_BUCHERON_ZONES) jnZone = 0;

		if(job_ObjectN[playerid] == 0)
		{
			// Abattage : il faut etre pres de l'arbre marque.
			new jnIdx = job_TempVar[playerid];
			if(jnIdx < 0 || jnIdx >= JOBNEW_BUCHERON_ARBRES) jnIdx = 0;
			if(!IsPlayerInRangeOfPoint(playerid, JOBNEW_ACTION_RANGE, jobnewBucheronArbre[jnZone][jnIdx][0], jobnewBucheronArbre[jnZone][jnIdx][1], jobnewBucheronArbre[jnZone][jnIdx][2]))
				{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous n'etes pas au pied de l'arbre marque.");}

			// "BOMBER"/"BOM_Plant" est deja utilise par le metier Ouvrier : on
			// sait donc que cette bibliotheque d'animation se charge bien.
			LoopingAnim(playerid, "BOMBER", "BOM_Plant", 4.0, 0, 0, 0, 0, 0);
			GameTextForPlayer(playerid, "~w~Abattage en cours ...", JOBNEW_BUCHERON_DUREE, 6);
			TogglePlayerControllable(playerid, false);
			SetTimerEx("JobsNew_Finish", JOBNEW_BUCHERON_DUREE, 0, "ii", playerid, 14);
			return 1;
		}
		else
		{
			// Depot du rondin.
			if(!IsPlayerInRangeOfPoint(playerid, JOBNEW_ACTION_RANGE, jobnewBucheronDepot[jnZone][0], jobnewBucheronDepot[jnZone][1], jobnewBucheronDepot[jnZone][2]))
				{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous n'etes pas au depot.");}

			if(job_HoldingObjectSlot[playerid] != -1 && IsPlayerAttachedObjectSlotUsed(playerid, job_HoldingObjectSlot[playerid]))
				{RemovePlayerAttachedObject(playerid, job_HoldingObjectSlot[playerid]);}
			job_HoldingObjectSlot[playerid] = -1;
			SetPlayerSpecialAction(playerid, SPECIAL_ACTION_NONE);
			LoopingAnim(playerid, "CARRY", "PUTDWN", 4.1, 0, 1, 1, 0, 0);
			job_ObjectN[playerid] = 0;

			PlayerInfo[playerid][pJobExp]++;
			job_UpdateTexts(playerid);
			job_CheckPoints[playerid]++;
			if(job_CheckPoints[playerid] >= JOBNEW_BUCHERON_PAR_PAYE)
			{
				job_TakePay(playerid, 14);
				job_CheckPoints[playerid] = 0;
			}
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Rondin depose. Retournez couper un autre arbre.");
			JobsNew_NextSpot(playerid);
			return 1;
		}
	}

	// ---------------- Jardinier ----------------
	if(jnJob == 15)
	{
		new jnIdx = job_TempVar[playerid];
		if(jnIdx < 0 || jnIdx >= JOBNEW_JARDINIER_PARCELLES) jnIdx = 0;
		if(!IsPlayerInRangeOfPoint(playerid, JOBNEW_ACTION_RANGE, jobnewJardinierParcelle[jnIdx][0], jobnewJardinierParcelle[jnIdx][1], jobnewJardinierParcelle[jnIdx][2]))
			{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous n'etes pas sur la parcelle marquee.");}

		LoopingAnim(playerid, "BOMBER", "BOM_Plant", 4.0, 0, 0, 0, 0, 0);
		GameTextForPlayer(playerid, "~w~Entretien de la parcelle ...", JOBNEW_JARDINIER_DUREE, 6);
		TogglePlayerControllable(playerid, false);
		SetTimerEx("JobsNew_Finish", JOBNEW_JARDINIER_DUREE, 0, "ii", playerid, 15);
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------
// Fin de l'animation de travail (timer). On revalide tout : le joueur a pu se
// deconnecter, quitter son service ou changer de metier entre-temps.
// ---------------------------------------------------------------------------
public JobsNew_Finish(playerid, jobid)
{
	if(!IsPlayerConnected(playerid))
		{return 0;}

	// On rend toujours le controle, meme si le job a ete quitte pendant
	// l'animation : sinon le joueur resterait fige definitivement.
	TogglePlayerControllable(playerid, true);
	ClearAnimations(playerid);

	if(job_Start[playerid] != 1 || PlayerInfo[playerid][pJob] != jobid)
		{return 0;}

	if(jobid == 14)
	{
		// L'arbre est abattu : le joueur repart avec un rondin sur l'epaule.
		new jnSlot = player_GetSlotObject(playerid);
		if(jnSlot != -1)
		{
			job_HoldingObjectSlot[playerid] = jnSlot;
			SetPlayerAttachedObject(playerid, jnSlot, JOBNEW_OBJ_RONDIN, 1, 0.184976, 0.586401, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 1.000000, 1.000000);
		}
		SetPlayerSpecialAction(playerid, SPECIAL_ACTION_CARRY);
		job_ObjectN[playerid] = 1;
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Arbre abattu, portez le rondin jusqu'au depot.");
		JobsNew_NextSpot(playerid);
		return 1;
	}

	if(jobid == 15)
	{
		PlayerInfo[playerid][pJobExp]++;
		job_UpdateTexts(playerid);
		job_CheckPoints[playerid]++;
		if(job_CheckPoints[playerid] >= JOBNEW_JARDINIER_PAR_PAYE)
		{
			job_TakePay(playerid, 15);
			job_CheckPoints[playerid] = 0;
		}
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Parcelle entretenue. Passez a la suivante.");
		JobsNew_NextSpot(playerid);
		return 1;
	}
	return 0;
}
