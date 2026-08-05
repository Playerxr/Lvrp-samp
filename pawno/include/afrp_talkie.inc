/*
	afrp_talkie.inc - Talkie-walkie a l'ecran pour les factions legales

	=== POURQUOI CE MODULE EXISTE (ET CE QU'IL REMPLACE) ===
	La proprietaire a fourni un export de textdraws (radinholapa.txt) : 23
	formes qui dessinent un talkie-walkie tenu en bas de l'ecran (antenne en
	haut, boitier qui sort par le bas de l'ecran). Cet export ne contient
	AUCUN texte - c'est uniquement la carrosserie.

	Ce meme export avait deja ete porte tel quel dans afrp_radiowidget.inc,
	ou il servait de simple voyant "je suis en radio". Ce module le REMPLACE
	(afrp_radiowidget.inc est supprime) au lieu de s'ajouter a lui : les deux
	auraient dessine exactement les memes 23 formes aux memes coordonnees,
	soit 23 textdraws globaux gaspilles et deux talkies superposes au pixel
	pres, impossibles a distinguer a l'oeil mais impossibles a cacher
	proprement (chacun ne cache que les siens).

	=== CE QUE CE MODULE AJOUTE PAR RAPPORT AU VOYANT PRECEDENT ===
	L'ecran du talkie affiche maintenant deux lignes utiles :
	  - le nom de la faction (un flic voit "LSPD", un medecin voit "Hopital")
	  - une ligne d'etat, qui dit HONNETEMENT si la voix passe ou pas
	Sans ces deux lignes l'objet ne servait a rien d'autre qu'a decorer.

	=== POURQUOI Text: GLOBAL POUR LA CARROSSERIE, PlayerText: POUR LE TEXTE ===
	Les 23 formes sont IDENTIQUES pour tout le monde : un seul jeu de Text:
	cree une fois a OnGameModeInit, montre/cache par joueur
	(TextDrawShowForPlayer/HideForPlayer). C'est la convention deja utilisee
	dans ce gamemode pour les elements globaux (afrp_announce.inc,
	afrp_banque.inc, pay_TextButton1-3, phone_Buton3...).
	Les deux lignes de texte, elles, DEPENDENT DU JOUEUR (sa faction, son
	etat). Un Text: global les rendrait fausses : TextDrawSetString ecrit pour
	tous ceux qui l'affichent au meme instant - un flic et un medecin en radio
	en meme temps verraient tous les deux la derniere faction ecrite. D'ou des
	PlayerText:, seule facon d'avoir un contenu par joueur.

	=== POURQUOI ON CREE/DETRUIT LES PlayerText: A CHAQUE AFFICHAGE ===
	La limite SA-MP est de 256 textdraws par joueur, et ce gamemode en
	consomme deja beaucoup (HUD, compteur, telephone, sac, mSelection...).
	Le talkie est cache 95% du temps : le creer a l'affichage et le detruire
	a la fermeture (comme afrp_sacvisuel.inc et afrp_tablecraft.inc) lui fait
	occuper 0 slot quand il ne sert pas, au lieu de 2 en permanence.

	=== OU EST PLACE LE TEXTE (verifie sur les coordonnees de l'export) ===
	Les 23 formes ont ete recalculees une par une (position + TextDrawTextSize
	= largeur/hauteur pour les sprites en font 4). L'ecran du talkie est le
	groupe turquoise (0x23B2A7FF) : elements 4, 5, 6 et 7. L'element 4 couvre
	x 367->459 a partir de y 355.4 et descend au-dela du bas de l'ecran
	(la zone visible SA-MP s'arrete a y=448) : c'est donc une plage turquoise
	pleine largeur, sans trou, de y 355.4 a y 448.
	Les petits reperes blancs de l'export (elements 17 a 21) occupent
	y 349.5 -> 356.5 : les deux lignes sont posees PLUS BAS (y 362 et y 376)
	pour ne pas les recouvrir, et bien au-dessus de y=448.
	x = 372 laisse 5 px de marge apres le bord gauche de l'ecran (x=367).
	Couleur du texte : le meme noir que la carrosserie (0x0A0A0AFF) - du texte
	sombre sur un ecran turquoise clair, comme un vrai afficheur LCD.
	ATTENTION : ce placement est calcule, PAS constate. Personne n'a vu le
	rendu en jeu. Les 5 constantes TALKIE_TXT_* ci-dessous existent justement
	pour corriger sans toucher au reste si l'affichage tombe a cote.

	=== POURQUOI PAS DE LIGNE "TRANSMISSION" QUI CLIGNOTE ===
	Le vrai filterscript (sampvoice.pwn) appelle bien OnPlayerVoiceStart/Stop
	a chaque appui/relachement de la touche talkie (OnPlayerActivationKeyPress/
	Release cote filterscript, via CallRemoteFunction) : le serveur SAIT donc
	quand quelqu'un parle vraiment. Mais ce module ne distingue pas encore
	"micro ouvert" de "en train de parler la seconde presente" - Talkie_Show/
	Hide suivent le mode (radio actif ou non), pas l'instant du appui. Ecrire
	"MICRO OUVERT" reste donc vrai : le canal est ouvert, prêt a transmettre
	des que le joueur appuie sur la touche talkie.

	=== JOUEUR SANS PLUGIN OU SANS MICRO ===
	Il voit quand meme le talkie, avec "TEXTE SEUL" en ligne d'etat. Le cacher
	aurait donne l'impression que /radio est casse chez lui, alors que la
	radio ECRITE (/radio <message>, msg_Radio) marche parfaitement. Le mot
	"TEXTE SEUL" dit la verite : l'objet est la, la voix non.

	=== FACTIONS LEGALES : REGLE NON DUPLIQUEE ===
	On appelle RadioVoice_IsLegalFaction() (afrp_radiovoice.inc), qui est la
	source de verite unique partagee avec /radio. Recopier la condition ici
	aurait garanti qu'un jour les deux ne soient plus d'accord.

	=== NOMS DE FACTION : GetFactionName() + REPARATION DES ACCENTS ===
	GetFactionName() (LVRP.pwn ~18659) est la source des noms. Mais dans ce
	depot les accents ont ete perdus a un moment de l'historique : la fonction
	renvoie "Hpital", "Prsidence", "Arme de Terre". Illisible sur un ecran de
	radio. On repare ces cas connus en ASCII ("Hopital", "Presidence",
	"Armee de Terre") par comparaison exacte : si GetFactionName change un
	jour, la comparaison echoue simplement et on affiche ce qu'elle renvoie -
	jamais de plantage, jamais de nom invente.

	=== CHANGEMENT DE FACTION PENDANT UN /radio ACTIF ===
	Pour les joueurs qui ont vraiment la voix, afrp_radiovoice.inc detache
	deja le canal (RadioVoice_ReconcileTick) et cela ferme le talkie via
	OnPlayerVoiceStop. Mais ce tick-la ignore les joueurs SANS voix
	(radioVoiceAttachedTo == 0), qui garderaient un talkie affichant leur
	ancienne faction. On a donc notre propre verification, volontairement
	separee pour ne pas modifier la logique vocale : elle ne regarde que les
	joueurs qui affichent le talkie, c'est-a-dire presque personne.

	Hooks a brancher dans LVRP.pwn :
	- OnGameModeInit     -> Talkie_Init()
	- OnPlayerVoiceStart -> Talkie_OnVoiceStart(playerid)
	- OnPlayerVoiceStop  -> Talkie_OnVoiceStop(playerid)
	- OnPlayerDisconnect -> Talkie_OnDisconnect(playerid)
	- /radio (sans texte) -> Talkie_OnRadioMode(playerid) apres talkStats = 3
	- /local              -> Talkie_Hide(playerid) apres VoixLocale_Toggle
*/

#if defined AFRP_TALKIE_INCLUDED
	#endinput
#endif
#define AFRP_TALKIE_INCLUDED

// Nombre de formes portees depuis radinholapa.txt (23 TextDrawCreate).
#define TALKIE_COUNT        23

// Verification de coherence faction/affichage. Meme cadence que la
// reconciliation vocale (8 s) : au-dela ca se verrait, en dessous ca ne sert
// a rien, un changement de faction n'est pas une urgence a la seconde.
#define TALKIE_TICK_MS      8000

// --- Placement des deux lignes de texte sur l'ecran turquoise ---
// Ce sont les SEULES valeurs a retoucher si le rendu tombe a cote (voir
// l'en-tete : calcul sur les coordonnees de l'export, jamais vu en jeu).
#define TALKIE_TXT_X            372.0   // 5 px apres le bord gauche de l'ecran (367)
#define TALKIE_TXT_Y_FACTION    362.0   // sous les reperes blancs (qui finissent a 356.5)
#define TALKIE_TXT_Y_STATUT     376.0   // une ligne plus bas
#define TALKIE_TXT_BORD_DROIT   459.0   // bord droit de l'ecran : coupure des lignes
#define TALKIE_TXT_COULEUR      168430335 // 0x0A0A0AFF, le noir deja utilise par la carrosserie

new Text:talkieTD[TALKIE_COUNT];              // carrosserie : identique pour tous
new PlayerText:talkieTdFaction[MAX_PLAYERS];  // ligne 1 : nom de la faction
new PlayerText:talkieTdStatut[MAX_PLAYERS];   // ligne 2 : etat de la voix
new bool:talkieAffiche[MAX_PLAYERS];          // le talkie est-il a l'ecran pour ce joueur ?
new talkieFactionAff[MAX_PLAYERS];            // faction affichee, pour detecter un changement

// Nom de faction pret a afficher sur un ecran de radio : GetFactionName()
// reste la source, on ne fait que remettre en ASCII les accents perdus dans
// l'historique du depot (voir l'en-tete). Comparaison exacte : si le nom
// source change, on retombe simplement sur la valeur d'origine.
stock Talkie_NomFaction(talkiePm, talkieDest[], talkieMax)
{
	new talkieBrut[32];
	strmid(talkieBrut, GetFactionName(talkiePm), 0, 32, 32);

	if(!strcmp(talkieBrut, "Hpital", true))              format(talkieDest, talkieMax, "Hopital");
	else if(!strcmp(talkieBrut, "Prsidence", true))      format(talkieDest, talkieMax, "Presidence");
	else if(!strcmp(talkieBrut, "Arme de Terre", true))  format(talkieDest, talkieMax, "Armee de Terre");
	else if(!strcmp(talkieBrut, "Arme de l'Air", true))  format(talkieDest, talkieMax, "Armee de l'Air");
	else format(talkieDest, talkieMax, "%s", talkieBrut);
	return 1;
}

stock Talkie_Init()
{
	// --- Port fidele des 23 formes de radinholapa.txt : positions, tailles,
	// couleurs et polices repris a l'identique, rien n'a ete retouche. ---
	talkieTD[0] = TextDrawCreate(358.000, 334.000, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[0], 111.000, 148.000);
	TextDrawAlignment(talkieTD[0], 1);
	TextDrawColor(talkieTD[0], 168430335);
	TextDrawSetShadow(talkieTD[0], 0);
	TextDrawSetOutline(talkieTD[0], 0);
	TextDrawBackgroundColor(talkieTD[0], 255);
	TextDrawFont(talkieTD[0], 4);
	TextDrawSetProportional(talkieTD[0], 1);

	talkieTD[1] = TextDrawCreate(353.899, 320.000, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[1], 25.000, 25.000);
	TextDrawAlignment(talkieTD[1], 1);
	TextDrawColor(talkieTD[1], 168430335);
	TextDrawSetShadow(talkieTD[1], 0);
	TextDrawSetOutline(talkieTD[1], 0);
	TextDrawBackgroundColor(talkieTD[1], 255);
	TextDrawFont(talkieTD[1], 4);
	TextDrawSetProportional(talkieTD[1], 1);

	talkieTD[2] = TextDrawCreate(448.000, 320.000, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[2], 25.100, 25.000);
	TextDrawAlignment(talkieTD[2], 1);
	TextDrawColor(talkieTD[2], 168430335);
	TextDrawSetShadow(talkieTD[2], 0);
	TextDrawSetOutline(talkieTD[2], 0);
	TextDrawBackgroundColor(talkieTD[2], 255);
	TextDrawFont(talkieTD[2], 4);
	TextDrawSetProportional(talkieTD[2], 1);

	talkieTD[3] = TextDrawCreate(367.000, 324.398, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[3], 92.000, 148.000);
	TextDrawAlignment(talkieTD[3], 1);
	TextDrawColor(talkieTD[3], 168430335);
	TextDrawSetShadow(talkieTD[3], 0);
	TextDrawSetOutline(talkieTD[3], 0);
	TextDrawBackgroundColor(talkieTD[3], 255);
	TextDrawFont(talkieTD[3], 4);
	TextDrawSetProportional(talkieTD[3], 1);

	talkieTD[4] = TextDrawCreate(367.000, 355.398, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[4], 92.000, 148.000);
	TextDrawAlignment(talkieTD[4], 1);
	TextDrawColor(talkieTD[4], 598910975);
	TextDrawSetShadow(talkieTD[4], 0);
	TextDrawSetOutline(talkieTD[4], 0);
	TextDrawBackgroundColor(talkieTD[4], 255);
	TextDrawFont(talkieTD[4], 4);
	TextDrawSetProportional(talkieTD[4], 1);

	talkieTD[5] = TextDrawCreate(435.799, 338.700, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[5], 28.100, 31.000);
	TextDrawAlignment(talkieTD[5], 1);
	TextDrawColor(talkieTD[5], 598910975);
	TextDrawSetShadow(talkieTD[5], 0);
	TextDrawSetOutline(talkieTD[5], 0);
	TextDrawBackgroundColor(talkieTD[5], 255);
	TextDrawFont(talkieTD[5], 4);
	TextDrawSetProportional(talkieTD[5], 1);

	talkieTD[6] = TextDrawCreate(362.199, 338.700, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[6], 28.100, 31.000);
	TextDrawAlignment(talkieTD[6], 1);
	TextDrawColor(talkieTD[6], 598910975);
	TextDrawSetShadow(talkieTD[6], 0);
	TextDrawSetOutline(talkieTD[6], 0);
	TextDrawBackgroundColor(talkieTD[6], 255);
	TextDrawFont(talkieTD[6], 4);
	TextDrawSetProportional(talkieTD[6], 1);

	talkieTD[7] = TextDrawCreate(375.700, 343.998, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[7], 75.000, 84.000);
	TextDrawAlignment(talkieTD[7], 1);
	TextDrawColor(talkieTD[7], 598910975);
	TextDrawSetShadow(talkieTD[7], 0);
	TextDrawSetOutline(talkieTD[7], 0);
	TextDrawBackgroundColor(talkieTD[7], 255);
	TextDrawFont(talkieTD[7], 4);
	TextDrawSetProportional(talkieTD[7], 1);

	talkieTD[8] = TextDrawCreate(434.000, 313.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[8], 20.000, 13.000);
	TextDrawAlignment(talkieTD[8], 1);
	TextDrawColor(talkieTD[8], 168430335);
	TextDrawSetShadow(talkieTD[8], 0);
	TextDrawSetOutline(talkieTD[8], 0);
	TextDrawBackgroundColor(talkieTD[8], 255);
	TextDrawFont(talkieTD[8], 4);
	TextDrawSetProportional(talkieTD[8], 1);

	talkieTD[9] = TextDrawCreate(428.997, 303.299, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[9], 30.100, 22.000);
	TextDrawAlignment(talkieTD[9], 1);
	TextDrawColor(talkieTD[9], 168430335);
	TextDrawSetShadow(talkieTD[9], 0);
	TextDrawSetOutline(talkieTD[9], 0);
	TextDrawBackgroundColor(talkieTD[9], 255);
	TextDrawFont(talkieTD[9], 4);
	TextDrawSetProportional(talkieTD[9], 1);

	talkieTD[10] = TextDrawCreate(435.200, 238.399, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[10], 17.698, 76.000);
	TextDrawAlignment(talkieTD[10], 1);
	TextDrawColor(talkieTD[10], 168430335);
	TextDrawSetShadow(talkieTD[10], 0);
	TextDrawSetOutline(talkieTD[10], 0);
	TextDrawBackgroundColor(talkieTD[10], 255);
	TextDrawFont(talkieTD[10], 4);
	TextDrawSetProportional(talkieTD[10], 1);

	talkieTD[11] = TextDrawCreate(430.697, 225.600, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[11], 26.500, 24.000);
	TextDrawAlignment(talkieTD[11], 1);
	TextDrawColor(talkieTD[11], 168430335);
	TextDrawSetShadow(talkieTD[11], 0);
	TextDrawSetOutline(talkieTD[11], 0);
	TextDrawBackgroundColor(talkieTD[11], 255);
	TextDrawFont(talkieTD[11], 4);
	TextDrawSetProportional(talkieTD[11], 1);

	talkieTD[12] = TextDrawCreate(365.000, 343.000, "PARTICLE:lamp_shad_64");
	TextDrawTextSize(talkieTD[12], 94.000, 138.000);
	TextDrawAlignment(talkieTD[12], 1);
	TextDrawColor(talkieTD[12], -16777318);
	TextDrawSetShadow(talkieTD[12], 0);
	TextDrawSetOutline(talkieTD[12], 0);
	TextDrawBackgroundColor(talkieTD[12], 255);
	TextDrawFont(talkieTD[12], 4);
	TextDrawSetProportional(talkieTD[12], 1);

	talkieTD[13] = TextDrawCreate(401.000, 308.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[13], 20.000, 18.000);
	TextDrawAlignment(talkieTD[13], 1);
	TextDrawColor(talkieTD[13], 168430335);
	TextDrawSetShadow(talkieTD[13], 0);
	TextDrawSetOutline(talkieTD[13], 0);
	TextDrawBackgroundColor(talkieTD[13], 255);
	TextDrawFont(talkieTD[13], 4);
	TextDrawSetProportional(talkieTD[13], 1);

	talkieTD[14] = TextDrawCreate(395.997, 299.299, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[14], 30.100, 22.000);
	TextDrawAlignment(talkieTD[14], 1);
	TextDrawColor(talkieTD[14], 168430335);
	TextDrawSetShadow(talkieTD[14], 0);
	TextDrawSetOutline(talkieTD[14], 0);
	TextDrawBackgroundColor(talkieTD[14], 255);
	TextDrawFont(talkieTD[14], 4);
	TextDrawSetProportional(talkieTD[14], 1);

	talkieTD[15] = TextDrawCreate(369.000, 319.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[15], 20.000, 13.000);
	TextDrawAlignment(talkieTD[15], 1);
	TextDrawColor(talkieTD[15], 168430335);
	TextDrawSetShadow(talkieTD[15], 0);
	TextDrawSetOutline(talkieTD[15], 0);
	TextDrawBackgroundColor(talkieTD[15], 255);
	TextDrawFont(talkieTD[15], 4);
	TextDrawSetProportional(talkieTD[15], 1);

	talkieTD[16] = TextDrawCreate(363.997, 311.299, "LD_BEAT:chit");
	TextDrawTextSize(talkieTD[16], 30.100, 22.000);
	TextDrawAlignment(talkieTD[16], 1);
	TextDrawColor(talkieTD[16], 168430335);
	TextDrawSetShadow(talkieTD[16], 0);
	TextDrawSetOutline(talkieTD[16], 0);
	TextDrawBackgroundColor(talkieTD[16], 255);
	TextDrawFont(talkieTD[16], 4);
	TextDrawSetProportional(talkieTD[16], 1);

	talkieTD[17] = TextDrawCreate(439.000, 350.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[17], 9.000, 6.000);
	TextDrawAlignment(talkieTD[17], 1);
	TextDrawColor(talkieTD[17], -1);
	TextDrawSetShadow(talkieTD[17], 0);
	TextDrawSetOutline(talkieTD[17], 0);
	TextDrawBackgroundColor(talkieTD[17], 255);
	TextDrawFont(talkieTD[17], 4);
	TextDrawSetProportional(talkieTD[17], 1);

	talkieTD[18] = TextDrawCreate(448.200, 351.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[18], 1.600, 4.000);
	TextDrawAlignment(talkieTD[18], 1);
	TextDrawColor(talkieTD[18], -1);
	TextDrawSetShadow(talkieTD[18], 0);
	TextDrawSetOutline(talkieTD[18], 0);
	TextDrawBackgroundColor(talkieTD[18], 255);
	TextDrawFont(talkieTD[18], 4);
	TextDrawSetProportional(talkieTD[18], 1);

	talkieTD[19] = TextDrawCreate(375.200, 350.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[19], 1.600, 5.000);
	TextDrawAlignment(talkieTD[19], 1);
	TextDrawColor(talkieTD[19], -1);
	TextDrawSetShadow(talkieTD[19], 0);
	TextDrawSetOutline(talkieTD[19], 0);
	TextDrawBackgroundColor(talkieTD[19], 255);
	TextDrawFont(talkieTD[19], 4);
	TextDrawSetProportional(talkieTD[19], 1);

	talkieTD[20] = TextDrawCreate(378.200, 349.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[20], 1.600, 6.000);
	TextDrawAlignment(talkieTD[20], 1);
	TextDrawColor(talkieTD[20], -1);
	TextDrawSetShadow(talkieTD[20], 0);
	TextDrawSetOutline(talkieTD[20], 0);
	TextDrawBackgroundColor(talkieTD[20], 255);
	TextDrawFont(talkieTD[20], 4);
	TextDrawSetProportional(talkieTD[20], 1);

	talkieTD[21] = TextDrawCreate(371.200, 352.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[21], 2.599, 3.000);
	TextDrawAlignment(talkieTD[21], 1);
	TextDrawColor(talkieTD[21], -1);
	TextDrawSetShadow(talkieTD[21], 0);
	TextDrawSetOutline(talkieTD[21], 0);
	TextDrawBackgroundColor(talkieTD[21], 255);
	TextDrawFont(talkieTD[21], 4);
	TextDrawSetProportional(talkieTD[21], 1);

	talkieTD[22] = TextDrawCreate(368.000, 329.500, "LD_SPAC:white");
	TextDrawTextSize(talkieTD[22], 90.000, 4.000);
	TextDrawAlignment(talkieTD[22], 1);
	TextDrawColor(talkieTD[22], 421075455);
	TextDrawSetShadow(talkieTD[22], 0);
	TextDrawSetOutline(talkieTD[22], 0);
	TextDrawBackgroundColor(talkieTD[22], 255);
	TextDrawFont(talkieTD[22], 4);
	TextDrawSetProportional(talkieTD[22], 1);

	for(new talkieI = 0; talkieI < MAX_PLAYERS; talkieI++)
	{
		talkieTdFaction[talkieI] = PlayerText:INVALID_TEXT_DRAW;
		talkieTdStatut[talkieI]  = PlayerText:INVALID_TEXT_DRAW;
		talkieAffiche[talkieI]   = false;
		talkieFactionAff[talkieI] = 0;
	}

	SetTimer("Talkie_Tick", TALKIE_TICK_MS, true);
	printf("[TALKIE] Interface talkie-walkie prete (%d formes globales, 2 lignes par joueur).", TALKIE_COUNT);
	return 1;
}

// Ecrit les deux lignes de l'ecran pour CE joueur. Suppose que les
// PlayerText: existent deja (crees par Talkie_Show).
stock Talkie_MajTexte(playerid)
{
	new talkiePm = PlayerInfo[playerid][pMember];

	new talkieNom[32];
	Talkie_NomFaction(talkiePm, talkieNom, sizeof(talkieNom));
	PlayerTextDrawSetString(playerid, talkieTdFaction[playerid], talkieNom);

	// Etat : on ne dit que ce qui est verifiable (voir l'en-tete).
	// RadioVoice_IsCapable lit le PVar "hasVoiceOnClient", pose par sampvoice.pwn.
	if(RadioVoice_IsCapable(playerid))
		PlayerTextDrawSetString(playerid, talkieTdStatut[playerid], "MICRO OUVERT");
	else
		PlayerTextDrawSetString(playerid, talkieTdStatut[playerid], "TEXTE SEUL");

	talkieFactionAff[playerid] = talkiePm;
	return 1;
}

// Affiche le talkie. Retourne 0 si l'affichage a ete refuse (faction non
// legale, ou un autre panneau plein ecran deja ouvert).
stock Talkie_Show(playerid)
{
	if(!IsPlayerConnected(playerid) || IsPlayerNPC(playerid)) return 0;
	if(talkieAffiche[playerid]) { Talkie_MajTexte(playerid); return 1; }

	// [SOURCE DE VERITE UNIQUE] meme regle que /radio et que les canaux vocaux.
	if(!RadioVoice_IsLegalFaction(PlayerInfo[playerid][pMember])) return 0;

	// Le talkie occupe x354-473 / y226-448, ce qui recouvre largement le
	// panneau du sac (x347-402 / y168-339) et le telephone. Regle heritee du
	// voyant precedent : on n'affiche jamais deux panneaux l'un sur l'autre.
	if(svOuvert[playerid] == 1) return 0;
	if(phone_State[playerid] != PHONE_STATE_CLOSED) return 0;

	talkieTdFaction[playerid] = CreatePlayerTextDraw(playerid, TALKIE_TXT_X, TALKIE_TXT_Y_FACTION, "_");
	PlayerTextDrawFont(playerid, talkieTdFaction[playerid], 1);
	PlayerTextDrawLetterSize(playerid, talkieTdFaction[playerid], 0.200000, 1.000000);
	PlayerTextDrawTextSize(playerid, talkieTdFaction[playerid], TALKIE_TXT_BORD_DROIT, 17.000000);
	PlayerTextDrawAlignment(playerid, talkieTdFaction[playerid], 1);
	PlayerTextDrawColor(playerid, talkieTdFaction[playerid], TALKIE_TXT_COULEUR);
	PlayerTextDrawSetShadow(playerid, talkieTdFaction[playerid], 0);
	PlayerTextDrawSetOutline(playerid, talkieTdFaction[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, talkieTdFaction[playerid], 255);
	PlayerTextDrawSetProportional(playerid, talkieTdFaction[playerid], 1);

	talkieTdStatut[playerid] = CreatePlayerTextDraw(playerid, TALKIE_TXT_X, TALKIE_TXT_Y_STATUT, "_");
	PlayerTextDrawFont(playerid, talkieTdStatut[playerid], 1);
	PlayerTextDrawLetterSize(playerid, talkieTdStatut[playerid], 0.170000, 0.900000);
	PlayerTextDrawTextSize(playerid, talkieTdStatut[playerid], TALKIE_TXT_BORD_DROIT, 17.000000);
	PlayerTextDrawAlignment(playerid, talkieTdStatut[playerid], 1);
	PlayerTextDrawColor(playerid, talkieTdStatut[playerid], TALKIE_TXT_COULEUR);
	PlayerTextDrawSetShadow(playerid, talkieTdStatut[playerid], 0);
	PlayerTextDrawSetOutline(playerid, talkieTdStatut[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, talkieTdStatut[playerid], 255);
	PlayerTextDrawSetProportional(playerid, talkieTdStatut[playerid], 1);

	talkieAffiche[playerid] = true;
	Talkie_MajTexte(playerid);

	for(new talkieK = 0; talkieK < TALKIE_COUNT; talkieK++)
		TextDrawShowForPlayer(playerid, talkieTD[talkieK]);
	PlayerTextDrawShow(playerid, talkieTdFaction[playerid]);
	PlayerTextDrawShow(playerid, talkieTdStatut[playerid]);
	return 1;
}

// Cache le talkie et LIBERE les deux slots par joueur (voir l'en-tete :
// limite de 256 textdraws par joueur, deja bien entamee par le HUD).
stock Talkie_Hide(playerid)
{
	if(!talkieAffiche[playerid]) return 0;
	talkieAffiche[playerid] = false;
	talkieFactionAff[playerid] = 0;

	for(new talkieK = 0; talkieK < TALKIE_COUNT; talkieK++)
		TextDrawHideForPlayer(playerid, talkieTD[talkieK]);

	if(talkieTdFaction[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, talkieTdFaction[playerid]);
		talkieTdFaction[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	if(talkieTdStatut[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, talkieTdStatut[playerid]);
		talkieTdStatut[playerid] = PlayerText:INVALID_TEXT_DRAW;
	}
	return 1;
}

// Appele par OnPlayerVoiceStart, que afrp_radiovoice.inc declenche au moment
// exact ou le joueur rejoint le canal vocal de sa faction.
stock Talkie_OnVoiceStart(playerid)
{
	if(GetPVarInt(playerid, "talkStats") != 3) return 0; // 3 = mode radio
	return Talkie_Show(playerid);
}

stock Talkie_OnVoiceStop(playerid)
{
	return Talkie_Hide(playerid);
}

// Appele par /radio (sans texte) APRES le passage de talkStats a 3.
// Indispensable pour les joueurs sans plugin/micro : chez eux
// RadioVoice_Enable sort avant d'appeler OnPlayerVoiceStart, donc le talkie
// ne s'ouvrirait jamais alors que la radio ECRITE leur marche tres bien.
stock Talkie_OnRadioMode(playerid)
{
	return Talkie_Show(playerid);
}

stock Talkie_OnDisconnect(playerid)
{
	Talkie_Hide(playerid);
	return 1;
}

// Le talkie ne doit jamais afficher une faction que le joueur a quittee.
// On ne parcourt que les joueurs qui l'ont reellement a l'ecran : en pratique
// ce timer ne fait rien du tout la plupart du temps.
forward Talkie_Tick();
public Talkie_Tick()
{
	for(new talkieI = 0; talkieI < MAX_PLAYERS_CURRENT + 1; talkieI++)
	{
		if(!talkieAffiche[talkieI])
		{
			// Rattrapage : le joueur EST en mode radio mais n'a pas le talkie.
			// Cas normal - il avait le sac ou le telephone ouvert au moment du
			// /radio, Talkie_Show a refuse pour ne pas superposer deux
			// panneaux. Des qu'il les referme, le talkie apparait enfin au
			// lieu de rester absent jusqu'au prochain /local puis /radio.
			if(IsPlayerConnected(talkieI) && GetPVarInt(talkieI, "talkStats") == 3)
				Talkie_Show(talkieI);
			continue;
		}
		if(!IsPlayerConnected(talkieI)) { Talkie_Hide(talkieI); continue; }

		new talkiePm = PlayerInfo[talkieI][pMember];
		if(talkiePm == talkieFactionAff[talkieI]) continue;

		// Plus dans une faction a radio : on ferme au lieu de mentir.
		if(!RadioVoice_IsLegalFaction(talkiePm))
		{
			Talkie_Hide(talkieI);
			if(GetPVarInt(talkieI, "talkStats") == 3) SetPVarInt(talkieI, "talkStats", 0);
			continue;
		}
		Talkie_MajTexte(talkieI);
	}
	return 1;
}
