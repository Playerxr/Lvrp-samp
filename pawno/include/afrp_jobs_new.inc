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
	  - Benevole d'entreprise (13) : choisir une entreprise employeuse, porter
	    les caisses A LA MAIN jusqu'au camion, rouler, decharger caisse par
	    caisse. Voir le gros pave "BENEVOLE D'ENTREPRISE" plus bas.

	POURQUOI CE FICHIER EST INCLUS APRES afrp_entreprise / afrp_cargomission
	Le Benevole lit entreprises[] (son employeur) et cargoDepot[] (les depots
	poses par un admin avec /cargo creer). Ces deux tableaux n'existent qu'a
	partir de ces deux modules : l'#include de ce fichier a donc ete descendu
	dans LVRP.pwn juste apres <afrp_cargomission>.

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
#define JOBNEW_CP_BENEVOLE_DEPART  34   // depot ou le benevole charge son camion
#define JOBNEW_CP_BENEVOLE_ARRIVEE 35   // depot ou le benevole doit livrer

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

// ===========================================================================
//                        BENEVOLE D'ENTREPRISE (metier 13)
// ===========================================================================
//
// CE QUE FAIT LE JOUEUR
// Il choisit une entreprise pour laquelle bosser, va a un depot cargo, prend
// une caisse DANS SES MAINS, la porte jusqu'au camion, recommence jusqu'a
// remplir le camion, roule jusqu'au depot de destination, puis ressort les
// caisses une par une et les depose. Il n'est paye qu'a la livraison.
//
// POURQUOI ON REUTILISE LE MATERIEL DU /cargo EXISTANT
// Les depots sont ceux poses par un admin avec /cargo creer (fichier
// scriptfiles/cargo_depots.cfg, deja rempli sur le serveur). On ne cree donc
// AUCUN point de travail : si un depot bouge, le metier suit tout seul. La
// destination est tiree au hasard parmi les autres depots, exactement comme le
// fait Cargo_StartMission pour les employes.
//
// CE QU'ON NE TOUCHE PAS
// La mission /cargo du patron et de ses employes reste strictement identique
// (elle verse 100 % en caisse). Le benevole est un chemin PARALLELE.
//
// ---------------------------------------------------------------------------
// LE PARTAGE DE LA PAYE - A LIRE AVANT DE CHANGER LES CHIFFRES
// ---------------------------------------------------------------------------
// Une course rapporte la MEME valeur qu'une course /cargo : la distance entre
// les deux depots x CG_PAY_PER_UNIT (reglage existant de afrp_cargomission).
// Cette valeur est partagee :
//
//   - L'ENTREPRISE touche BENEVOLE_PART_ENTREPRISE % de cette valeur dans sa
//     caisse. C'est moins que les 100 % d'une course faite par un employe :
//     faire travailler un benevole coute donc quelque chose au patron, ce qui
//     est normal puisque le benevole n'est pas salarie de l'entreprise.
//
//   - LE BENEVOLE touche le reste. ATTENTION : il ne le touche PAS en liquide.
//     Une course peut valoir plusieurs dizaines de milliers de dollars ; verser
//     ca directement mettrait ce metier tres au-dessus de tous les autres (voir
//     docs/equilibrage_jobs.md : un metier rapporte quelques dizaines de dollars
//     par tache, plus son salaire horaire). Sa part passe donc par le circuit
//     NORMAL des metiers : job_TakePay() -> bande MOYENNE -> Jobs_ComputeReward
//     (prime de serie + bonus debutant), verse a la paye avec son salaire.
//
//   -> Le vrai bouton pour payer plus le benevole n'est PAS ici : c'est la
//      colonne job13 de la table lvrp_factions_governements (ligne id = 4),
//      exactement comme pour les 12 autres metiers. Voir docs/equilibrage_jobs.md.
#define BENEVOLE_PART_ENTREPRISE   70   // % de la course verse dans la caisse

// Nombre de caisses a porter pour remplir le camion. C'est ce chiffre qui fait
// la duree d'une course : le monter rend le metier plus long et plus physique.
#define BENEVOLE_CAISSES_PAR_COURSE 4

// Une caisse deposee sur deux declenche un bonus de metier. Monter ce chiffre
// paye moins souvent, le descendre a 1 paye a chaque caisse.
#define BENEVOLE_CAISSES_PAR_BONUS  2

// Distance a laquelle on accepte la touche 'Y' pres du camion. Volontairement
// large : un camion est un gros vehicule, on mesure depuis son centre.
#define BENEVOLE_RAYON_CAMION       7.0

// Distance a laquelle on accepte la touche 'Y' pres du depot. CG_DEPOT_RADIUS
// (8.0) est le rayon que le /cargo utilise deja pour reconnaitre un depot : on
// garde la meme valeur pour que les deux systemes reagissent pareil.
#define BENEVOLE_RAYON_DEPOT        CG_DEPOT_RADIUS

// Caisse portee a l'epaule. Modele 935 avec les decalages du metier Ouvrier :
// ils sont deja valides a l'ecran (voir LVRP.pwn, SetPlayerAttachedObject 935).
#define BENEVOLE_OBJ_CAISSE         935

// Dialogue "choisis ton entreprise". 9656 est libre (9655 = banque, 9654 =
// raccourcis, 9657+ non utilises) ; il est inscrit au registre de
// afrp_diagnostic.inc pour que /verif detecte une future collision.
#define JOBNEW_DLG_BENEVOLE         9656

// Employeur PNJ du Benevole : docks de Los Santos, a 6 m de l'employeur
// Camionneur existant (2422.9304/-2075.8716/13.5538) et EXACTEMENT a la meme
// altitude, donc sur le meme quai betonne. C'est la seule coordonnee nouvelle
// de ce metier.
#define BENEVOLE_EMPLOYEUR_X       2428.9304
#define BENEVOLE_EMPLOYEUR_Y      -2075.8716
#define BENEVOLE_EMPLOYEUR_Z         13.5538

// Etapes d'une course.
#define BV_ETAPE_AUCUNE             0   // pas en service
#define BV_ETAPE_CHARGEMENT         1   // au depot de depart, on remplit le camion
#define BV_ETAPE_ROUTE              2   // camion plein, on roule vers la destination
#define BV_ETAPE_DECHARGEMENT       3   // arrive, on ressort les caisses

// Nom de l'entreprise employeuse. On stocke le NOM et pas l'index du tableau :
// si l'entreprise est dissoute pendant le service, Ent_FindByName renvoie -1 et
// on previent proprement le joueur, au lieu de lire un index devenu invalide.
new benevole_Ent[MAX_PLAYERS][64];
new benevole_Etape[MAX_PLAYERS];
new benevole_DepotDepart[MAX_PLAYERS];
new benevole_DepotArrivee[MAX_PLAYERS];
new benevole_Caisses[MAX_PLAYERS];      // caisses chargees dans le camion
new benevole_Livrees[MAX_PLAYERS];      // caisses deja deposees a l'arrivee
new benevole_VersBonus[MAX_PLAYERS];    // compteur vers le prochain job_TakePay

// ---------------------------------------------------------------------------
// Camion utilisable le plus proche du joueur a pied.
// On accepte n'importe quel utilitaire du catalogue de afrp_cargomission
// (Mule, Boxville, Benson, Yankee, Pony, Burrito) : les camionnettes achetees
// par les entreprises en font partie, mais un benevole peut aussi venir avec
// son propre utilitaire. Sinon le metier serait injouable tant qu'un patron
// n'a pas achete de camionnette.
// Renvoie INVALID_VEHICLE_ID si aucun camion assez proche.
// ---------------------------------------------------------------------------
stock Benevole_CamionProche(playerid)
{
	new Float:bvVx, Float:bvVy, Float:bvVz;
	new bvBest = INVALID_VEHICLE_ID;
	new Float:bvBestDist = BENEVOLE_RAYON_CAMION;
	for(new bvV = 1; bvV < MAX_CAR; bvV++)
	{
		if(GetVehicleModel(bvV) == 0) continue;                 // slot vide
		if(Cargo_ModelIndex(GetVehicleModel(bvV)) == -1) continue;
		GetVehiclePos(bvV, bvVx, bvVy, bvVz);
		new Float:bvD = GetPlayerDistanceFromPoint(playerid, bvVx, bvVy, bvVz);
		if(bvD < bvBestDist) { bvBestDist = bvD; bvBest = bvV; }
	}
	return bvBest;
}

// ---------------------------------------------------------------------------
// Enleve la caisse des mains (si elle y est). Appele au depot comme a l'arrivee.
// ---------------------------------------------------------------------------
stock Benevole_LacherCaisse(playerid)
{
	if(job_HoldingObjectSlot[playerid] != -1 && IsPlayerAttachedObjectSlotUsed(playerid, job_HoldingObjectSlot[playerid]))
		{RemovePlayerAttachedObject(playerid, job_HoldingObjectSlot[playerid]);}
	job_HoldingObjectSlot[playerid] = -1;
	SetPlayerSpecialAction(playerid, SPECIAL_ACTION_NONE);
	job_ObjectN[playerid] = 0;
	return 1;
}

// ---------------------------------------------------------------------------
// Met une caisse dans les mains du joueur.
// ---------------------------------------------------------------------------
stock Benevole_PrendreCaisse(playerid)
{
	new bvSlot = player_GetSlotObject(playerid);
	if(bvSlot != -1)
	{
		job_HoldingObjectSlot[playerid] = bvSlot;
		SetPlayerAttachedObject(playerid, bvSlot, BENEVOLE_OBJ_CAISSE, 1, 0.184976, 0.586401, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 1.000000, 1.000000);
	}
	// "liftup" = le geste de soulever une caisse. LoopingAnim force le
	// forcesync a 1, sans quoi une animation declenchee par le joueur lui-meme
	// ne s'affiche pas chez les autres (voir afrp_emotes.inc).
	LoopingAnim(playerid, "CARRY", "liftup", 4.0, 0, 0, 0, 0, 0);
	SetPlayerSpecialAction(playerid, SPECIAL_ACTION_CARRY);
	job_ObjectN[playerid] = 1;
	return 1;
}

// ---------------------------------------------------------------------------
// Pose le marqueur du depot de depart et annonce l'etape de chargement.
// bvDepot = index dans cargoDepot[].
// ---------------------------------------------------------------------------
stock Benevole_CommencerChargement(playerid, bvDepot)
{
	benevole_Etape[playerid]        = BV_ETAPE_CHARGEMENT;
	benevole_DepotDepart[playerid]  = bvDepot;
	benevole_DepotArrivee[playerid] = -1;
	benevole_Caisses[playerid]      = 0;
	benevole_Livrees[playerid]      = 0;

	gPlayerCheckpoint[playerid] = JOBNEW_CP_BENEVOLE_DEPART;
	SetPlayerCheckpoint(playerid, cargoDepot[bvDepot][cdX], cargoDepot[bvDepot][cdY], cargoDepot[bvDepot][cdZ], JOBNEW_CP_SIZE);

	new bvMsg[190];
	format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Depot {FFFFB2}%s{FFFFFF} : gare un utilitaire a cote, puis touche 'Y' pour prendre une caisse ({FFFFB2}%d{FFFFFF} a charger).",
		cargoDepot[bvDepot][cdName], BENEVOLE_CAISSES_PAR_COURSE);
	msg_Client(playerid, COLOR_JOB, bvMsg);
	return 1;
}

// ---------------------------------------------------------------------------
// Camion plein : tirage de la destination parmi les AUTRES depots.
// Meme logique que Cargo_StartMission, recopiee et non appelee pour ne surtout
// pas declencher la mission /cargo des employes en parallele.
// ---------------------------------------------------------------------------
stock Benevole_PartirEnLivraison(playerid)
{
	new bvCandidats[CG_MAX_DEPOT], bvNb = 0;
	for(new bvI = 0; bvI < CG_MAX_DEPOT; bvI++)
	{
		if(cargoDepot[bvI][cdUsed] != 1) continue;
		if(bvI == benevole_DepotDepart[playerid]) continue;
		bvCandidats[bvNb++] = bvI;
	}
	if(bvNb == 0)
	{
		// Un seul depot sur le serveur : on ne peut aller nulle part. On rend
		// la main sans casser le service, l'admin doit poser un 2e depot.
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Il n'y a qu'un seul depot cargo sur le serveur : impossible de livrer. Previens un admin (/cargo creer).");
		return 0;
	}

	new bvDest = bvCandidats[random(bvNb)];
	benevole_DepotArrivee[playerid] = bvDest;
	benevole_Etape[playerid]        = BV_ETAPE_ROUTE;

	gPlayerCheckpoint[playerid] = JOBNEW_CP_BENEVOLE_ARRIVEE;
	SetPlayerCheckpoint(playerid, cargoDepot[bvDest][cdX], cargoDepot[bvDest][cdY], cargoDepot[bvDest][cdZ], JOBNEW_CP_SIZE);

	new bvMsg[190];
	format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Camion charge ! Direction le depot {FFFFB2}%s{FFFFFF} (point rouge minimap).", cargoDepot[bvDest][cdName]);
	msg_Client(playerid, COLOR_JOB, bvMsg);
	return 1;
}

// ---------------------------------------------------------------------------
// Toutes les caisses sont deposees : on paye la caisse de l'entreprise, puis on
// enchaine une nouvelle course a partir du depot ou le joueur se trouve.
// ---------------------------------------------------------------------------
stock Benevole_TerminerCourse(playerid)
{
	new bvDep = benevole_DepotDepart[playerid];
	new bvArr = benevole_DepotArrivee[playerid];

	// Valeur de la course : formule identique a celle du /cargo (distance en
	// metres x CG_PAY_PER_UNIT, plancher 1000 $), pour que les deux systemes
	// restent coherents entre eux.
	new bvValeur = 1000;
	if(bvDep >= 0 && bvDep < CG_MAX_DEPOT && bvArr >= 0 && bvArr < CG_MAX_DEPOT)
	{
		new Float:bvDist = Cargo_Distance2D(cargoDepot[bvDep][cdX], cargoDepot[bvDep][cdY], cargoDepot[bvArr][cdX], cargoDepot[bvArr][cdY]);
		bvValeur = floatround(bvDist) * CG_PAY_PER_UNIT;
		if(bvValeur < 1000) bvValeur = 1000;
	}
	new bvPartEnt = (bvValeur * BENEVOLE_PART_ENTREPRISE) / 100;

	// Ent_CreditByName renvoie 0 si l'entreprise n'existe plus (dissoute
	// pendant le service) : on le dit au joueur au lieu de faire comme si.
	if(Ent_CreditByName(benevole_Ent[playerid], bvPartEnt))
	{
		new bvMsg[200];
		format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Livraison terminee : {FFFFB2}+%d${FFFFFF} verses dans la caisse de {FFFFB2}%s{FFFFFF}.", bvPartEnt, benevole_Ent[playerid]);
		msg_Client(playerid, COLOR_JOB, bvMsg);
	}
	else
	{
		msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Livraison terminee, mais ton entreprise n'existe plus : rien n'a pu etre verse en caisse. Fais /job debut pour en choisir une autre.");
	}

	Obj_Progress(playerid, OBJ_CARGO, 1);   // [OBJECTIFS] compte comme une livraison cargo

	// Nouvelle course : le depot d'arrivee devient le depot de depart.
	if(bvArr >= 0 && bvArr < CG_MAX_DEPOT && cargoDepot[bvArr][cdUsed] == 1)
		{Benevole_CommencerChargement(playerid, bvArr);}
	else
		{benevole_Etape[playerid] = BV_ETAPE_AUCUNE; DisablePlayerCheckpoint(playerid);}
	return 1;
}

// ---------------------------------------------------------------------------
// Dialogue de choix de l'entreprise employeuse (/job debut).
// ---------------------------------------------------------------------------
stock Benevole_ChoisirEntreprise(playerid)
{
	// 30 entreprises max (ENT_MAX) x (64 caracteres de nom + le montant) : on
	// dimensionne large pour que la liste ne soit jamais tronquee.
	new bvBody[2600], bvLigne[128], bvNb = 0;
	for(new bvE = 0; bvE < ENT_MAX; bvE++)
	{
		if(entreprises[bvE][entUsed] != 1) continue;
		format(bvLigne, sizeof(bvLigne), "%s\t{FFFFB2}%d$\n", entreprises[bvE][entName], entreprises[bvE][entCash]);
		strcat(bvBody, bvLigne, sizeof(bvBody));
		bvNb++;
	}
	if(bvNb == 0)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Aucune entreprise n'existe pour l'instant : impossible d'etre benevole. Reviens quand un joueur en aura cree une.");}

	return ShowPlayerDialog(playerid, JOBNEW_DLG_BENEVOLE, DIALOG_STYLE_TABLIST_HEADERS,
		"{78769D}Benevole - Pour qui veux-tu bosser ?\tCaisse",
		bvBody, "Choisir", "Annuler");
}

// ---------------------------------------------------------------------------
// Reponse du dialogue. On reparcourt entreprises[] dans le meme ordre que
// l'affichage : si une entreprise a disparu entre-temps, le joueur tombe sur
// une autre, ce qui est sans danger (le nom retenu reste coherent ensuite).
// ---------------------------------------------------------------------------
stock JobsNew_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid != JOBNEW_DLG_BENEVOLE) return 0;
	if(!response) return 1;
	if(PlayerInfo[playerid][pJob] != 13) return 1;

	new bvRang = 0, bvChoisi = -1;
	for(new bvE = 0; bvE < ENT_MAX; bvE++)
	{
		if(entreprises[bvE][entUsed] != 1) continue;
		if(bvRang == listitem) { bvChoisi = bvE; break; }
		bvRang++;
	}
	if(bvChoisi == -1)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Cette entreprise n'existe plus, refais /job debut.");}

	new bvN = 0;
	while(bvN < 63 && entreprises[bvChoisi][entName][bvN]) { benevole_Ent[playerid][bvN] = entreprises[bvChoisi][entName][bvN]; bvN++; }
	benevole_Ent[playerid][bvN] = 0;

	return Benevole_DemarrerService(playerid);
}

// ---------------------------------------------------------------------------
// Prise de service effective, une fois l'employeur choisi.
// ---------------------------------------------------------------------------
stock Benevole_DemarrerService(playerid)
{
	// Il faut au moins un depot pose par un admin, sinon il n'y a nulle part
	// ou charger.
	new bvDepots = 0;
	for(new bvI = 0; bvI < CG_MAX_DEPOT; bvI++)
		{if(cargoDepot[bvI][cdUsed] == 1) bvDepots++;}
	if(bvDepots < 2)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Il faut au moins 2 depots cargo sur le serveur pour livrer. Previens un admin (/cargo creer).");}

	job_Start[playerid]             = 1;
	job_CheckPoints[playerid]       = 0;
	job_TempVar[playerid]           = 0;
	job_ObjectN[playerid]           = 0;
	job_HoldingObjectSlot[playerid] = -1;
	job_City[playerid]              = 0;
	benevole_VersBonus[playerid]    = 0;

	job_SetSkin(playerid, 13);
	job_ShowTexts(playerid);

	new bvMsg[190];
	format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Tu es benevole chez {FFFFB2}%s{FFFFFF}. Tu n'es paye qu'a la livraison.", benevole_Ent[playerid]);
	msg_Client(playerid, COLOR_JOB, bvMsg);

	// Depot de depart : le plus proche du joueur, sinon le premier existant.
	// Aucun risque de teleportation, on ne fait que poser un marqueur.
	new bvDepot = -1;
	new Float:bvBest = 999999.0;
	for(new bvI = 0; bvI < CG_MAX_DEPOT; bvI++)
	{
		if(cargoDepot[bvI][cdUsed] != 1) continue;
		new Float:bvD = GetPlayerDistanceFromPoint(playerid, cargoDepot[bvI][cdX], cargoDepot[bvI][cdY], cargoDepot[bvI][cdZ]);
		if(bvD < bvBest) { bvBest = bvD; bvDepot = bvI; }
	}
	if(bvDepot == -1)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Aucun depot cargo disponible.");}

	Benevole_CommencerChargement(playerid, bvDepot);

	if(PlayerInfo[playerid][pJobTime] < JOB_TIME)
	{
		new bvMsg2[144];
		format(bvMsg2, sizeof(bvMsg2), "{78769D} Job {FFFFFF} Vous devez encore travailler %d minute(s) pour obtenir la paye.", JOB_TIME - PlayerInfo[playerid][pJobTime]);
		msg_Client(playerid, COLOR_JOB, bvMsg2);
	}
	return 1;
}

// ---------------------------------------------------------------------------
// Touche 'Y' du Benevole. Renvoie 1 si l'action a ete traitee ici.
// ---------------------------------------------------------------------------
stock Benevole_OnKeyYes(playerid)
{
	new bvDep = benevole_DepotDepart[playerid];
	new bvArr = benevole_DepotArrivee[playerid];

	// ---------------- Chargement au depot de depart ----------------
	if(benevole_Etape[playerid] == BV_ETAPE_CHARGEMENT)
	{
		if(bvDep < 0 || bvDep >= CG_MAX_DEPOT || cargoDepot[bvDep][cdUsed] != 1)
			{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Ton depot de depart n'existe plus, refais /job debut.");}

		if(job_ObjectN[playerid] == 0)
		{
			// Le camion est deja plein mais le depart n'a pas pu se faire (un
			// admin a supprime des depots entre-temps) : on retente le depart
			// plutot que de laisser charger une 5e, 6e... caisse.
			if(benevole_Caisses[playerid] >= BENEVOLE_CAISSES_PAR_COURSE)
				{Benevole_PartirEnLivraison(playerid); return 1;}

			// Mains vides : on prend une caisse au depot.
			if(!IsPlayerInRangeOfPoint(playerid, BENEVOLE_RAYON_DEPOT, cargoDepot[bvDep][cdX], cargoDepot[bvDep][cdY], cargoDepot[bvDep][cdZ]))
				{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Approche-toi du depot pour prendre une caisse.");}
			Benevole_PrendreCaisse(playerid);
			new bvMsg[170];
			format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Caisse sur l'epaule ({FFFFB2}%d{FFFFFF}/{FFFFB2}%d{FFFFFF}) : porte-la jusqu'au camion, puis touche 'Y'.",
				benevole_Caisses[playerid] + 1, BENEVOLE_CAISSES_PAR_COURSE);
			return msg_Client(playerid, COLOR_JOB, bvMsg);
		}

		// Caisse en main : on la charge dans le camion.
		if(Benevole_CamionProche(playerid) == INVALID_VEHICLE_ID)
			{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Aucun utilitaire a cote de toi (Mule, Boxville, Benson, Yankee, Pony ou Burrito).");}

		Benevole_LacherCaisse(playerid);
		LoopingAnim(playerid, "CARRY", "PUTDWN", 4.1, 0, 1, 1, 0, 0);
		benevole_Caisses[playerid]++;

		if(benevole_Caisses[playerid] >= BENEVOLE_CAISSES_PAR_COURSE)
			{Benevole_PartirEnLivraison(playerid); return 1;}

		new bvMsg2[170];
		format(bvMsg2, sizeof(bvMsg2), "{78769D} Job {FFFFFF} Caisse chargee ({FFFFB2}%d{FFFFFF}/{FFFFB2}%d{FFFFFF}). Retourne en chercher une autre.",
			benevole_Caisses[playerid], BENEVOLE_CAISSES_PAR_COURSE);
		return msg_Client(playerid, COLOR_JOB, bvMsg2);
	}

	// ---------------- Dechargement au depot d'arrivee ----------------
	if(benevole_Etape[playerid] == BV_ETAPE_DECHARGEMENT)
	{
		if(bvArr < 0 || bvArr >= CG_MAX_DEPOT || cargoDepot[bvArr][cdUsed] != 1)
			{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Ton depot de livraison n'existe plus, refais /job debut.");}

		if(job_ObjectN[playerid] == 0)
		{
			// Mains vides : on sort une caisse du camion.
			if(Benevole_CamionProche(playerid) == INVALID_VEHICLE_ID)
				{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Retourne au camion pour sortir une caisse.");}
			Benevole_PrendreCaisse(playerid);
			new bvMsg[170];
			format(bvMsg, sizeof(bvMsg), "{78769D} Job {FFFFFF} Caisse sortie du camion ({FFFFB2}%d{FFFFFF}/{FFFFB2}%d{FFFFFF}) : porte-la au depot, puis touche 'Y'.",
				benevole_Livrees[playerid] + 1, benevole_Caisses[playerid]);
			return msg_Client(playerid, COLOR_JOB, bvMsg);
		}

		// Caisse en main : on la depose au depot.
		if(!IsPlayerInRangeOfPoint(playerid, BENEVOLE_RAYON_DEPOT, cargoDepot[bvArr][cdX], cargoDepot[bvArr][cdY], cargoDepot[bvArr][cdZ]))
			{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Tu n'es pas au depot de livraison.");}

		Benevole_LacherCaisse(playerid);
		LoopingAnim(playerid, "CARRY", "PUTDWN", 4.1, 0, 1, 1, 0, 0);
		benevole_Livrees[playerid]++;

		PlayerInfo[playerid][pJobExp]++;
		job_UpdateTexts(playerid);

		// Bonus de metier une caisse sur BENEVOLE_CAISSES_PAR_BONUS. C'est le
		// circuit normal de tous les metiers (bande MOYENNE + prime de serie).
		benevole_VersBonus[playerid]++;
		if(benevole_VersBonus[playerid] >= BENEVOLE_CAISSES_PAR_BONUS)
		{
			job_TakePay(playerid, 13);
			benevole_VersBonus[playerid] = 0;
		}

		if(benevole_Livrees[playerid] >= benevole_Caisses[playerid])
			{return Benevole_TerminerCourse(playerid);}

		new bvMsg2[170];
		format(bvMsg2, sizeof(bvMsg2), "{78769D} Job {FFFFFF} Caisse livree ({FFFFB2}%d{FFFFFF}/{FFFFB2}%d{FFFFFF}). Retourne en chercher une autre dans le camion.",
			benevole_Livrees[playerid], benevole_Caisses[playerid]);
		return msg_Client(playerid, COLOR_JOB, bvMsg2);
	}

	// ---------------- En route ----------------
	if(benevole_Etape[playerid] == BV_ETAPE_ROUTE)
	{
		// Filet de secours : si le joueur s'est gare a cote du depot sans
		// passer dans le checkpoint (il fait 5 m de rayon, on peut le rater),
		// la touche 'Y' sur place fait quand meme basculer au dechargement.
		if(bvArr >= 0 && bvArr < CG_MAX_DEPOT && cargoDepot[bvArr][cdUsed] == 1
			&& IsPlayerInRangeOfPoint(playerid, BENEVOLE_RAYON_DEPOT, cargoDepot[bvArr][cdX], cargoDepot[bvArr][cdY], cargoDepot[bvArr][cdZ]))
		{
			benevole_Etape[playerid] = BV_ETAPE_DECHARGEMENT;
			benevole_Livrees[playerid] = 0;
			return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Arrive ! Touche 'Y' au camion pour sortir une caisse, puis 'Y' au depot pour la deposer.");
		}
		return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Le camion est charge : conduis-le jusqu'au point rouge.");
	}

	return 0;
}

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
	if(jnJob != 13 && jnJob != 14 && jnJob != 15)
		{return 0;}

	if(job_Start[playerid] == 1)
		{return msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Vous etes deja en service.");}

	// Le Benevole doit d'abord dire pour quelle entreprise il bosse : la prise
	// de service se fait dans la reponse du dialogue, pas ici.
	if(jnJob == 13)
	{
		benevole_Etape[playerid] = BV_ETAPE_AUCUNE;
		benevole_Ent[playerid][0] = 0;
		return Benevole_ChoisirEntreprise(playerid);
	}

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
		case JOBNEW_CP_BENEVOLE_DEPART:
		{
			if(PlayerInfo[playerid][pJob] != 13) return 0;
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Tu es au depot : touche 'Y' pour prendre une caisse, porte-la au camion puis 'Y' a nouveau.");
			return 1;
		}
		case JOBNEW_CP_BENEVOLE_ARRIVEE:
		{
			if(PlayerInfo[playerid][pJob] != 13) return 0;
			// On ne bascule qu'une fois : re-entrer dans le meme checkpoint ne
			// doit pas remettre le compteur de caisses livrees a zero.
			if(benevole_Etape[playerid] != BV_ETAPE_ROUTE) return 1;
			benevole_Etape[playerid] = BV_ETAPE_DECHARGEMENT;
			benevole_Livrees[playerid] = 0;
			msg_Client(playerid, COLOR_JOB, "{78769D} Job {FFFFFF} Arrive ! Descends, touche 'Y' au camion pour sortir une caisse, puis 'Y' au depot pour la deposer.");
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

	// ---------------- Benevole d'entreprise ----------------
	if(jnJob == 13)
		{return Benevole_OnKeyYes(playerid);}

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
