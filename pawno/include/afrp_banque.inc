/*
	afrp_banque.inc - Panneau bancaire visuel (bancoverde.pwn) + vrai systeme d'emprunt

	La proprietaire a fourni un mockup textdraw portugais "Banco Verde" (79
	Text:PublicTD[], style Nickk TextDraw Editor) et a demande "un vrai systeme
	pour la banque" : pas juste un habillage, mais aussi une fonctionnalite
	d'emprunt qui n'existait pas du tout dans le gamemode. Ce module fait les
	deux : il remplace le menu liste (dialog 86) ouvert par le PNJ banquier par
	ce panneau, ET ajoute /Emprestimo/ comme vrai systeme de pret/remboursement.

	=== CORRESPONDANCE MOCKUP -> JEU (relevee sur les 79 elements) ===
	Sur les 79, exactement 8 sont SetSelectable(...,1) (vrais boutons) :
	  32 "Deposito", 33 "Saque", 34 "Tranferencia"  -> onglets (bascule laquelle
	      des 3 operations le sous-panneau "Qual valor desejado ?" concerne)
	  39 "R$" et 41 "DEPOSITAR"                      -> ouvrent la saisie du
	      montant pour l'onglet actuellement selectionne (voir plus bas pourquoi
	      un dialogue, et pourquoi ces deux boutons font la meme chose)
	  64 et 70 "R$ 300,00"                           -> ARTEFACT D'EXPORT : ce
	      sont les valeurs d'un toast de confirmation demo ("Depositado" /
	      "Retirado"), pas des boutons. Repris en TextDrawSetSelectable(...,0)
	      et pilotes comme de vrais toasts (montant reel, ~3s, voir plus bas).
	  77 "Realizar Emprestimo"                       -> ouvre l'emprunt
	Le reste (labels, fonds, decor de carte bancaire "VISA") est decoratif et
	repris tel quel, SetSelectable(...,0) comme dans la source.
	Deux textes source ("\") n'etaient pas echappes et auraient casse la
	compilation (backslash brut dans une chaine Pawn) : corriges en "\\" sans
	rien changer au rendu (elements 62 et 68, un petit "check" en ASCII-art).

	=== TEXTDRAWS GLOBAUX, PAS PAR JOUEUR ===
	Contrairement au sac (afrp_sacvisuel.inc) ou au craft (afrp_tablecraft.inc)
	qui creent/detruisent des PlayerText a chaque ouverture, ce panneau reprend
	le style DEJA utilise ailleurs dans ce gamemode pour des elements globaux
	affiches/caches par joueur (pay_TextButton1-3, phone_Buton3, et surtout
	radioWidgetTD dans afrp_radiowidget.inc) : un seul jeu de Text: cree une
	fois a OnGameModeInit, montre/cache par joueur avec TextDrawShowForPlayer/
	TextDrawHideForPlayer. LIMITE CONNUE (deja presente pour tous ces autres
	elements globaux) : TextDrawSetString/TextDrawColor changent le contenu
	pour TOUT LE MONDE qui le voit actuellement affiche, pas juste le joueur
	cible. Comme la banque n'est consultee que par un joueur a la fois devant
	le meme PNJ dans l'immense majorite des cas, et que chaque ouverture/action
	reecrit immediatement le bon contenu pour son joueur, le risque est un
	affichage errone d'une fraction de seconde dans le rare cas de deux joueurs
	simultanes, jamais une perte de donnees (PlayerInfo reste la seule verite).

	=== POURQUOI UN DIALOGUE POUR LE MONTANT ===
	SA-MP ne permet pas de taper du texte libre dans un textdraw. L'approche
	etablie (dej utilisee par le reste du gamemode pour /sac jeter, /sac
	donner, etc.) : un ShowPlayerDialog(DIALOG_STYLE_INPUT) par-dessus le
	panneau encore affiche et encore en SelectTextDraw. D'ou le choix de faire
	de "R$" (39) ET "DEPOSITAR" (41) le meme raccourci "ouvrir la saisie pour
	l'onglet actif" : le pack n'a qu'un seul sous-panneau de saisie partage
	entre les 3 operations (Deposito/Saque/Tranferencia), donc les deux boutons
	qui y vivent font logiquement la meme chose plutot que d'inventer un sens
	different a chacun.
	DIALOG_BANQUE = 9655, libre entre DIALOG_RACCOURCIS (9654, dernier de la
	plage 9600-9654 utilisee par event/armeebase/assurance/braquage/gps/saison/
	mapedit/mappack/sacvisuel/telapps/portes/residence/raccourcis) et
	DIALOG_APP_GPS (9700, debut de la plage mobile_apps/stats/genmiracle/
	economy). Verifie par grep sur tous les #define DIALOG_* et tous les
	ShowPlayerDialog(...) numeriques du gamemode avant de le choisir.

	=== DEPOT / RETRAIT / VIREMENT : LOGIQUE REPRISE, PAS REINVENTEE ===
	Les regles de validation (montant max = liquide/solde, anti virement a
	soi-meme, syntaxe "montant/id") sont copiees de l'ancien handler du dialog
	86 (OnDialogResponse, ex-lignes ~40734-40917 du gamemode), pas redevinees.
	Seul ajout : le virement verifie maintenant IsPlayerConnected(cible) avant
	de crediter - l'ancien code ne le faisait pas et pouvait donc silencieusement
	ecrire dans PlayerInfo[] d'un ID deconnecte ou invalide. Comme l'ancien
	code, deposit/retrait/virement ne forcent PAS de sauvegarde MySQL immediate
	pour "Bank" : ce champ est deja couvert par la sauvegarde periodique
	generale (OnPlayerUpdateSQL), exactement comme avant ce module.
	Le handler du dialog 86 original reste EN PLACE mais mort : plus aucun
	point d'entree (le PNJ ouvre desormais Banque_Ouvrir) n'y mene. Le
	supprimer proprement aurait touche a un bloc OnDialogResponse tres imbrique
	pour un gain nul ; le laisser inerte est le choix le plus sur.

	=== EMPRUNT : VRAI NOUVEAU SYSTEME, VALEURS PAR DEFAUT A AJUSTER ===
	Rien n'existait avant (verifie : aucun pLoanAmount/Loan/Emprunt dans tout le
	gamemode). La demande de la proprietaire etait volontairement sans chiffres
	("un vrai systeme"), donc les valeurs ci-dessous sont des choix par defaut
	raisonnables, PAS des chiffres communiques par elle - a ajuster ici si besoin :
	  BANQ_PRET_MAX = $50 000 (plafond emprunte, pas plafond du) ;
	  BANQ_PRET_FRAIS_POURCENT = 10% fixe, preleve UNE FOIS a l'emprunt (pas
	    d'interet qui court dans le temps, pour rester simple et previsible :
	    emprunter $1000 => devoir $1100, point final) ;
	  un seul emprunt actif a la fois (il faut le solder avant d'en reprendre
	    un) - evite toute logique d'emprunts multiples/cumules ;
	  l'argent emprunte est credite sur pAccount (solde bancaire), jamais sur
	    pCash ; le remboursement part uniquement de pAccount (jamais de pCash),
	    partiel ou total, jamais plus que ce qui est du ni plus que le solde.
	AUCUNE penalite/relance automatique en cas de non-remboursement : hors
	perimetre demande, volontairement pas construit.
	pLoanAmount se sauvegarde immediatement (MySQLUpdatePlayerIntSingle) a
	CHAQUE changement, contrairement a Bank plus haut : OnPlayerUpdateSQL est
	une requete a colonnes fixes deja tres longue, y ajouter Loan aurait ete un
	changement plus risque pour un gain minime face a une sauvegarde immediate.

	*** ACTION MANUELLE REQUISE COTE BASE DE DONNEES DE PRODUCTION ***
	Ce sandbox n'a AUCUNE connexion a la base MySQL live : la colonne ne peut
	pas etre creee depuis ici. A executer une fois, a la main, sur la base
	de production avant de mettre ce module en ligne :
		ALTER TABLE lvrp_users ADD COLUMN Loan INT(11) NOT NULL DEFAULT 0;
	(lvrp.sql et lvrp_vierge.sql sont deja mis a jour pour toute FUTURE
	reinstallation, mais ca ne touche pas une base qui existe deja.)

	=== BOUTON "REMBOURSER" : SEUL AJOUT AU-DELA DES 79 ELEMENTS SOURCE ===
	Le pack n'a aucun bouton de remboursement (verifie sur les elements 71-78 :
	juste des icones de toast, le fond/titre/texte marketing et le bouton
	d'emprunt). Ajoute en banqueTD[79], clone exact du style du bouton 77
	(meme police/alignement/taille), place juste en dessous, texte "Rembourser"
	en blanc au lieu du bleu de 77 pour le distinguer visuellement comme action
	secondaire. Les textes 75/76 (accroche marketing) sont aussi rendus
	dynamiques : ils affichent l'emprunt en cours a la place du texte marketing
	des qu'un emprunt existe, cote a cote avec ce nouveau bouton.

	=== CHEQUIER : PAS DE VISUEL DANS LE PACK, COMMANDE DE SECOURS ===
	bancoverde.pwn ne prevoit aucun element pour "Demander un chequier" (qui
	existait dans l'ancien menu liste). Plutot que de forcer un element en
	trop dans une mise en page qui n'a pas ete pensee pour, /chequier reste
	joignable en commande, avec exactement la meme logique que l'ancien
	dialog 86 (niveau 3 minimum, un seul chequier a la fois).

	Hooks branches dans LVRP.pwn :
	- OnGameModeInit              -> Banque_Init()
	- OnPlayerConnect             -> Banque_OnConnect(playerid)
	- OnPlayerDisconnect          -> Banque_OnDisconnect(playerid)
	- OnPlayerClickTextDraw       -> if(Banque_OnClick(playerid, clickedid)) return 1;
	- OnDialogResponse            -> if(Banque_OnDialog(playerid, dialogid, response, inputtext)) return 1;
	- PNJ banquier (actor[][variable]==5) -> Banque_Ouvrir(playerid)
	- /chequier                   -> Banque_CommandeChequier(playerid)
*/

#if defined AFRP_BANQUE_INCLUDED
	#endinput
#endif
#define AFRP_BANQUE_INCLUDED

#define BANQ_TD_COUNT               80   // 79 elements portes de bancoverde.pwn + 1 nouveau (79 = "Rembourser")
#define DIALOG_BANQUE              9655  // libre : voir le detail du grep dans le commentaire d'en-tete

// --- Emprunt : reglages par defaut (voir en-tete), a ajuster ici si besoin ---
#define BANQ_PRET_MAX              50000 // plafond du montant EMPRUNTE (pas du montant du apres frais)
#define BANQ_PRET_FRAIS_POURCENT   10    // frais fixe preleve a l'emprunt, pas d'interet dans le temps

// --- etapes du dialogue de saisie (montant), et onglets actifs 1-3 partages avec elles ---
#define BANQ_ETAPE_DEPOT           1
#define BANQ_ETAPE_RETRAIT         2
#define BANQ_ETAPE_VIREMENT        3
#define BANQ_ETAPE_EMPRUNT         4
#define BANQ_ETAPE_REMBOURSER      5

#define BANQ_TOAST_DUREE_MS        3000  // duree d'affichage des toasts Depositado/Retirado

// Vert "Banco Verde" du pack (onglet actif) / gris clair du pack (onglet au repos) -
// memes valeurs que celles deja utilisees par la source pour Deposito vs Saque/Tranferencia.
#define BANQ_COULEUR_ACTIF         16711935
#define BANQ_COULEUR_INACTIF       -1094795521

// Overlay de SelectTextDraw : vert fonce translucide, coherent avec le theme "Banco Verde".
#define BANQ_SELECT_COULEUR        0x0A2A0AAA

new Text:banqueTD[BANQ_TD_COUNT];

new bool:banqOuvert[MAX_PLAYERS];
new banqEtape[MAX_PLAYERS];              // etape en attente de reponse du DIALOG_BANQUE, 0 = aucune
new banqPending[MAX_PLAYERS];            // onglet actif (Deposito/Saque/Tranferencia) pour R$/DEPOSITAR
new banqToastExpire[MAX_PLAYERS];        // GetTickCount() auquel le toast en cours doit se cacher, 0 = aucun

// Elements du toast "Depositado" / "Retirado" (fond partage 58, icones, label, valeur, fleche) :
// caches par defaut a l'ouverture du panneau, montres brievement apres une operation reussie.
new const banqIdxToastDepot[8]   = {58,59,60,61,62,63,64,71};
new const banqIdxToastRetrait[8] = {58,65,66,67,68,69,70,72};

// ============================================================================
// CREATION (une seule fois, a OnGameModeInit)
// ============================================================================
stock Banque_Init()
{
	banqueTD[0] = TextDrawCreate(153.000000, 75.000000, "INTRO3:intro3");
	TextDrawFont(banqueTD[0], 4);
	TextDrawLetterSize(banqueTD[0], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[0], 327.500000, 252.500000);
	TextDrawSetOutline(banqueTD[0], 1);
	TextDrawSetShadow(banqueTD[0], 0);
	TextDrawAlignment(banqueTD[0], 1);
	TextDrawColor(banqueTD[0], 1296911871);
	TextDrawBackgroundColor(banqueTD[0], 1296911871);
	TextDrawBoxColor(banqueTD[0], 1296911666);
	TextDrawUseBox(banqueTD[0], 1);
	TextDrawSetProportional(banqueTD[0], 1);
	TextDrawSetSelectable(banqueTD[0], 0);

	banqueTD[1] = TextDrawCreate(355.000000, 261.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[1], 4);
	TextDrawLetterSize(banqueTD[1], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[1], 127.000000, 22.000000);
	TextDrawSetOutline(banqueTD[1], 1);
	TextDrawSetShadow(banqueTD[1], 0);
	TextDrawAlignment(banqueTD[1], 1);
	TextDrawColor(banqueTD[1], 255);
	TextDrawBackgroundColor(banqueTD[1], 255);
	TextDrawBoxColor(banqueTD[1], 50);
	TextDrawUseBox(banqueTD[1], 1);
	TextDrawSetProportional(banqueTD[1], 1);
	TextDrawSetSelectable(banqueTD[1], 0);

	banqueTD[2] = TextDrawCreate(165.000000, 81.000000, "/");
	TextDrawFont(banqueTD[2], 1);
	TextDrawLetterSize(banqueTD[2], 0.454164, 2.299994);
	TextDrawTextSize(banqueTD[2], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[2], 0);
	TextDrawSetShadow(banqueTD[2], 0);
	TextDrawAlignment(banqueTD[2], 1);
	TextDrawColor(banqueTD[2], 16711935);
	TextDrawBackgroundColor(banqueTD[2], 255);
	TextDrawBoxColor(banqueTD[2], 50);
	TextDrawUseBox(banqueTD[2], 0);
	TextDrawSetProportional(banqueTD[2], 1);
	TextDrawSetSelectable(banqueTD[2], 0);

	banqueTD[3] = TextDrawCreate(167.000000, 102.000000, "/");
	TextDrawFont(banqueTD[3], 1);
	TextDrawLetterSize(banqueTD[3], 1.524994, -1.850005);
	TextDrawTextSize(banqueTD[3], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[3], 0);
	TextDrawSetShadow(banqueTD[3], 0);
	TextDrawAlignment(banqueTD[3], 1);
	TextDrawColor(banqueTD[3], 16711935);
	TextDrawBackgroundColor(banqueTD[3], 255);
	TextDrawBoxColor(banqueTD[3], 50);
	TextDrawUseBox(banqueTD[3], 0);
	TextDrawSetProportional(banqueTD[3], 1);
	TextDrawSetSelectable(banqueTD[3], 0);

	banqueTD[4] = TextDrawCreate(156.000000, 88.000000, "/");
	TextDrawFont(banqueTD[4], 1);
	TextDrawLetterSize(banqueTD[4], 2.420831, 1.299994);
	TextDrawTextSize(banqueTD[4], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[4], 0);
	TextDrawSetShadow(banqueTD[4], 0);
	TextDrawAlignment(banqueTD[4], 1);
	TextDrawColor(banqueTD[4], 16711935);
	TextDrawBackgroundColor(banqueTD[4], 255);
	TextDrawBoxColor(banqueTD[4], 50);
	TextDrawUseBox(banqueTD[4], 0);
	TextDrawSetProportional(banqueTD[4], 1);
	TextDrawSetSelectable(banqueTD[4], 0);

	banqueTD[5] = TextDrawCreate(202.000000, 88.000000, "HUD:radar_propertyg");
	TextDrawFont(banqueTD[5], 4);
	TextDrawLetterSize(banqueTD[5], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[5], 12.000000, 12.000000);
	TextDrawSetOutline(banqueTD[5], 1);
	TextDrawSetShadow(banqueTD[5], 0);
	TextDrawAlignment(banqueTD[5], 1);
	TextDrawColor(banqueTD[5], -1);
	TextDrawBackgroundColor(banqueTD[5], 255);
	TextDrawBoxColor(banqueTD[5], 50);
	TextDrawUseBox(banqueTD[5], 1);
	TextDrawSetProportional(banqueTD[5], 1);
	TextDrawSetSelectable(banqueTD[5], 0);

	banqueTD[6] = TextDrawCreate(215.000000, 91.000000, "BANCO");
	TextDrawFont(banqueTD[6], 1);
	TextDrawLetterSize(banqueTD[6], 0.204166, 0.899999);
	TextDrawTextSize(banqueTD[6], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[6], 0);
	TextDrawSetShadow(banqueTD[6], 4);
	TextDrawAlignment(banqueTD[6], 1);
	TextDrawColor(banqueTD[6], -1);
	TextDrawBackgroundColor(banqueTD[6], 255);
	TextDrawBoxColor(banqueTD[6], 50);
	TextDrawUseBox(banqueTD[6], 1);
	TextDrawSetProportional(banqueTD[6], 1);
	TextDrawSetSelectable(banqueTD[6], 0);

	banqueTD[7] = TextDrawCreate(248.000000, 108.000000, "_");
	TextDrawFont(banqueTD[7], 1);
	TextDrawLetterSize(banqueTD[7], 0.987500, 3.300003);
	TextDrawTextSize(banqueTD[7], 334.500000, 88.500000);
	TextDrawSetOutline(banqueTD[7], 1);
	TextDrawSetShadow(banqueTD[7], 0);
	TextDrawAlignment(banqueTD[7], 2);
	TextDrawColor(banqueTD[7], -1);
	TextDrawBackgroundColor(banqueTD[7], 255);
	TextDrawBoxColor(banqueTD[7], 1296911751);
	TextDrawUseBox(banqueTD[7], 1);
	TextDrawSetProportional(banqueTD[7], 1);
	TextDrawSetSelectable(banqueTD[7], 0);

	banqueTD[8] = TextDrawCreate(212.000000, 128.000000, "Voce nao possui multas.");
	TextDrawFont(banqueTD[8], 1);
	TextDrawLetterSize(banqueTD[8], 0.183333, 0.850000);
	TextDrawTextSize(banqueTD[8], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[8], 0);
	TextDrawSetShadow(banqueTD[8], 0);
	TextDrawAlignment(banqueTD[8], 1);
	TextDrawColor(banqueTD[8], -1);
	TextDrawBackgroundColor(banqueTD[8], 255);
	TextDrawBoxColor(banqueTD[8], 50);
	TextDrawUseBox(banqueTD[8], 0);
	TextDrawSetProportional(banqueTD[8], 1);
	TextDrawSetSelectable(banqueTD[8], 0);

	banqueTD[9] = TextDrawCreate(201.000000, 104.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[9], 4);
	TextDrawLetterSize(banqueTD[9], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[9], 23.000000, 25.500000);
	TextDrawSetOutline(banqueTD[9], 3);
	TextDrawSetShadow(banqueTD[9], 0);
	TextDrawAlignment(banqueTD[9], 1);
	TextDrawColor(banqueTD[9], 1296911871);
	TextDrawBackgroundColor(banqueTD[9], 255);
	TextDrawBoxColor(banqueTD[9], 50);
	TextDrawUseBox(banqueTD[9], 1);
	TextDrawSetProportional(banqueTD[9], 1);
	TextDrawSetSelectable(banqueTD[9], 0);

	banqueTD[10] = TextDrawCreate(205.000000, 114.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[10], 4);
	TextDrawLetterSize(banqueTD[10], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[10], 14.500000, 9.500000);
	TextDrawSetOutline(banqueTD[10], 3);
	TextDrawSetShadow(banqueTD[10], 0);
	TextDrawAlignment(banqueTD[10], 1);
	TextDrawColor(banqueTD[10], -1094795521);
	TextDrawBackgroundColor(banqueTD[10], 255);
	TextDrawBoxColor(banqueTD[10], 50);
	TextDrawUseBox(banqueTD[10], 1);
	TextDrawSetProportional(banqueTD[10], 1);
	TextDrawSetSelectable(banqueTD[10], 0);

	banqueTD[11] = TextDrawCreate(207.000000, 108.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[11], 4);
	TextDrawLetterSize(banqueTD[11], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[11], 10.500000, 9.500000);
	TextDrawSetOutline(banqueTD[11], 3);
	TextDrawSetShadow(banqueTD[11], 0);
	TextDrawAlignment(banqueTD[11], 1);
	TextDrawColor(banqueTD[11], -1094795521);
	TextDrawBackgroundColor(banqueTD[11], 255);
	TextDrawBoxColor(banqueTD[11], 50);
	TextDrawUseBox(banqueTD[11], 1);
	TextDrawSetProportional(banqueTD[11], 1);
	TextDrawSetSelectable(banqueTD[11], 0);

	banqueTD[12] = TextDrawCreate(223.000000, 111.000000, "LEH SAMP");
	TextDrawFont(banqueTD[12], 1);
	TextDrawLetterSize(banqueTD[12], 0.212499, 1.000000);
	TextDrawTextSize(banqueTD[12], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[12], 0);
	TextDrawSetShadow(banqueTD[12], 0);
	TextDrawAlignment(banqueTD[12], 1);
	TextDrawColor(banqueTD[12], -1094795521);
	TextDrawBackgroundColor(banqueTD[12], 255);
	TextDrawBoxColor(banqueTD[12], 50);
	TextDrawUseBox(banqueTD[12], 0);
	TextDrawSetProportional(banqueTD[12], 1);
	TextDrawSetSelectable(banqueTD[12], 0);

	banqueTD[13] = TextDrawCreate(160.000000, 175.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[13], 4);
	TextDrawLetterSize(banqueTD[13], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[13], 23.500000, 25.500000);
	TextDrawSetOutline(banqueTD[13], 1);
	TextDrawSetShadow(banqueTD[13], 0);
	TextDrawAlignment(banqueTD[13], 1);
	TextDrawColor(banqueTD[13], 1296911871);
	TextDrawBackgroundColor(banqueTD[13], 255);
	TextDrawBoxColor(banqueTD[13], 50);
	TextDrawUseBox(banqueTD[13], 1);
	TextDrawSetProportional(banqueTD[13], 1);
	TextDrawSetSelectable(banqueTD[13], 0);

	banqueTD[14] = TextDrawCreate(160.000000, 154.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[14], 4);
	TextDrawLetterSize(banqueTD[14], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[14], 23.500000, 25.500000);
	TextDrawSetOutline(banqueTD[14], 1);
	TextDrawSetShadow(banqueTD[14], 0);
	TextDrawAlignment(banqueTD[14], 1);
	TextDrawColor(banqueTD[14], 1296911871);
	TextDrawBackgroundColor(banqueTD[14], 255);
	TextDrawBoxColor(banqueTD[14], 50);
	TextDrawUseBox(banqueTD[14], 1);
	TextDrawSetProportional(banqueTD[14], 1);
	TextDrawSetSelectable(banqueTD[14], 0);

	banqueTD[15] = TextDrawCreate(160.000000, 196.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[15], 4);
	TextDrawLetterSize(banqueTD[15], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[15], 23.500000, 25.500000);
	TextDrawSetOutline(banqueTD[15], 1);
	TextDrawSetShadow(banqueTD[15], 0);
	TextDrawAlignment(banqueTD[15], 1);
	TextDrawColor(banqueTD[15], 1296911871);
	TextDrawBackgroundColor(banqueTD[15], 255);
	TextDrawBoxColor(banqueTD[15], 50);
	TextDrawUseBox(banqueTD[15], 1);
	TextDrawSetProportional(banqueTD[15], 1);
	TextDrawSetSelectable(banqueTD[15], 0);

	banqueTD[16] = TextDrawCreate(160.000000, 217.000000, "ld_beat:chit");
	TextDrawFont(banqueTD[16], 4);
	TextDrawLetterSize(banqueTD[16], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[16], 23.500000, 25.500000);
	TextDrawSetOutline(banqueTD[16], 1);
	TextDrawSetShadow(banqueTD[16], 0);
	TextDrawAlignment(banqueTD[16], 1);
	TextDrawColor(banqueTD[16], 1296911871);
	TextDrawBackgroundColor(banqueTD[16], 255);
	TextDrawBoxColor(banqueTD[16], 50);
	TextDrawUseBox(banqueTD[16], 1);
	TextDrawSetProportional(banqueTD[16], 1);
	TextDrawSetSelectable(banqueTD[16], 0);

	banqueTD[17] = TextDrawCreate(169.000000, 162.000000, "u");
	TextDrawFont(banqueTD[17], 2);
	TextDrawLetterSize(banqueTD[17], 0.216666, 1.200000);
	TextDrawTextSize(banqueTD[17], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[17], 0);
	TextDrawSetShadow(banqueTD[17], 0);
	TextDrawAlignment(banqueTD[17], 1);
	TextDrawColor(banqueTD[17], -1);
	TextDrawBackgroundColor(banqueTD[17], 255);
	TextDrawBoxColor(banqueTD[17], 50);
	TextDrawUseBox(banqueTD[17], 0);
	TextDrawSetProportional(banqueTD[17], 1);
	TextDrawSetSelectable(banqueTD[17], 0);

	banqueTD[18] = TextDrawCreate(167.000000, 158.000000, "A");
	TextDrawFont(banqueTD[18], 2);
	TextDrawLetterSize(banqueTD[18], 0.312500, 1.350000);
	TextDrawTextSize(banqueTD[18], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[18], 0);
	TextDrawSetShadow(banqueTD[18], 0);
	TextDrawAlignment(banqueTD[18], 1);
	TextDrawColor(banqueTD[18], -1);
	TextDrawBackgroundColor(banqueTD[18], 255);
	TextDrawBoxColor(banqueTD[18], 50);
	TextDrawUseBox(banqueTD[18], 0);
	TextDrawSetProportional(banqueTD[18], 1);
	TextDrawSetSelectable(banqueTD[18], 0);

	banqueTD[19] = TextDrawCreate(166.000000, 182.000000, "l");
	TextDrawFont(banqueTD[19], 0);
	TextDrawLetterSize(banqueTD[19], 0.312500, 1.150000);
	TextDrawTextSize(banqueTD[19], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[19], 0);
	TextDrawSetShadow(banqueTD[19], 0);
	TextDrawAlignment(banqueTD[19], 1);
	TextDrawColor(banqueTD[19], -1);
	TextDrawBackgroundColor(banqueTD[19], 255);
	TextDrawBoxColor(banqueTD[19], 50);
	TextDrawUseBox(banqueTD[19], 0);
	TextDrawSetProportional(banqueTD[19], 1);
	TextDrawSetSelectable(banqueTD[19], 0);

	banqueTD[20] = TextDrawCreate(169.000000, 179.000000, "l");
	TextDrawFont(banqueTD[20], 0);
	TextDrawLetterSize(banqueTD[20], 0.312500, 1.700000);
	TextDrawTextSize(banqueTD[20], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[20], 0);
	TextDrawSetShadow(banqueTD[20], 0);
	TextDrawAlignment(banqueTD[20], 1);
	TextDrawColor(banqueTD[20], -1);
	TextDrawBackgroundColor(banqueTD[20], 255);
	TextDrawBoxColor(banqueTD[20], 50);
	TextDrawUseBox(banqueTD[20], 0);
	TextDrawSetProportional(banqueTD[20], 1);
	TextDrawSetSelectable(banqueTD[20], 0);

	banqueTD[21] = TextDrawCreate(172.000000, 186.000000, "l");
	TextDrawFont(banqueTD[21], 0);
	TextDrawLetterSize(banqueTD[21], 0.312500, 0.850000);
	TextDrawTextSize(banqueTD[21], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[21], 0);
	TextDrawSetShadow(banqueTD[21], 0);
	TextDrawAlignment(banqueTD[21], 1);
	TextDrawColor(banqueTD[21], -1);
	TextDrawBackgroundColor(banqueTD[21], 255);
	TextDrawBoxColor(banqueTD[21], 50);
	TextDrawUseBox(banqueTD[21], 0);
	TextDrawSetProportional(banqueTD[21], 1);
	TextDrawSetSelectable(banqueTD[21], 0);

	banqueTD[22] = TextDrawCreate(166.000000, 201.000000, "O");
	TextDrawFont(banqueTD[22], 1);
	TextDrawLetterSize(banqueTD[22], 0.416666, 1.700000);
	TextDrawTextSize(banqueTD[22], 404.500000, 27.500000);
	TextDrawSetOutline(banqueTD[22], 0);
	TextDrawSetShadow(banqueTD[22], 0);
	TextDrawAlignment(banqueTD[22], 1);
	TextDrawColor(banqueTD[22], -1);
	TextDrawBackgroundColor(banqueTD[22], 255);
	TextDrawBoxColor(banqueTD[22], 50);
	TextDrawUseBox(banqueTD[22], 0);
	TextDrawSetProportional(banqueTD[22], 1);
	TextDrawSetSelectable(banqueTD[22], 0);

	banqueTD[23] = TextDrawCreate(168.000000, 207.000000, "/");
	TextDrawFont(banqueTD[23], 1);
	TextDrawLetterSize(banqueTD[23], 0.500000, 0.449999);
	TextDrawTextSize(banqueTD[23], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[23], 0);
	TextDrawSetShadow(banqueTD[23], 0);
	TextDrawAlignment(banqueTD[23], 1);
	TextDrawColor(banqueTD[23], -1);
	TextDrawBackgroundColor(banqueTD[23], 255);
	TextDrawBoxColor(banqueTD[23], 50);
	TextDrawUseBox(banqueTD[23], 0);
	TextDrawSetProportional(banqueTD[23], 1);
	TextDrawSetSelectable(banqueTD[23], 0);

	banqueTD[24] = TextDrawCreate(171.000000, 229.000000, "_");
	TextDrawFont(banqueTD[24], 1);
	TextDrawLetterSize(banqueTD[24], 0.600000, 0.450002);
	TextDrawTextSize(banqueTD[24], 303.500000, -12.500000);
	TextDrawSetOutline(banqueTD[24], 1);
	TextDrawSetShadow(banqueTD[24], 0);
	TextDrawAlignment(banqueTD[24], 2);
	TextDrawColor(banqueTD[24], -1);
	TextDrawBackgroundColor(banqueTD[24], 255);
	TextDrawBoxColor(banqueTD[24], -121);
	TextDrawUseBox(banqueTD[24], 1);
	TextDrawSetProportional(banqueTD[24], 1);
	TextDrawSetSelectable(banqueTD[24], 0);

	banqueTD[25] = TextDrawCreate(171.000000, 226.000000, "_");
	TextDrawFont(banqueTD[25], 3);
	TextDrawLetterSize(banqueTD[25], 0.600000, -0.149997);
	TextDrawTextSize(banqueTD[25], 303.500000, -12.500000);
	TextDrawSetOutline(banqueTD[25], 1);
	TextDrawSetShadow(banqueTD[25], 0);
	TextDrawAlignment(banqueTD[25], 2);
	TextDrawColor(banqueTD[25], -1);
	TextDrawBackgroundColor(banqueTD[25], 255);
	TextDrawBoxColor(banqueTD[25], -121);
	TextDrawUseBox(banqueTD[25], 1);
	TextDrawSetProportional(banqueTD[25], 1);
	TextDrawSetSelectable(banqueTD[25], 0);

	banqueTD[26] = TextDrawCreate(202.000000, 157.000000, "Saldo Bancario");
	TextDrawFont(banqueTD[26], 1);
	TextDrawLetterSize(banqueTD[26], 0.179166, 0.900000);
	TextDrawTextSize(banqueTD[26], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[26], 0);
	TextDrawSetShadow(banqueTD[26], 0);
	TextDrawAlignment(banqueTD[26], 1);
	TextDrawColor(banqueTD[26], -1094795521);
	TextDrawBackgroundColor(banqueTD[26], 255);
	TextDrawBoxColor(banqueTD[26], 50);
	TextDrawUseBox(banqueTD[26], 0);
	TextDrawSetProportional(banqueTD[26], 1);
	TextDrawSetSelectable(banqueTD[26], 0);

	banqueTD[27] = TextDrawCreate(202.000000, 166.000000, "R$");
	TextDrawFont(banqueTD[27], 1);
	TextDrawLetterSize(banqueTD[27], 0.337500, 1.349999);
	TextDrawTextSize(banqueTD[27], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[27], 1);
	TextDrawSetShadow(banqueTD[27], 0);
	TextDrawAlignment(banqueTD[27], 1);
	TextDrawColor(banqueTD[27], 16711935);
	TextDrawBackgroundColor(banqueTD[27], 255);
	TextDrawBoxColor(banqueTD[27], 50);
	TextDrawUseBox(banqueTD[27], 1);
	TextDrawSetProportional(banqueTD[27], 1);
	TextDrawSetSelectable(banqueTD[27], 0);

	banqueTD[28] = TextDrawCreate(219.000000, 166.000000, "0,00");
	TextDrawFont(banqueTD[28], 1);
	TextDrawLetterSize(banqueTD[28], 0.337500, 1.349999);
	TextDrawTextSize(banqueTD[28], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[28], 1);
	TextDrawSetShadow(banqueTD[28], 0);
	TextDrawAlignment(banqueTD[28], 1);
	TextDrawColor(banqueTD[28], -1);
	TextDrawBackgroundColor(banqueTD[28], 255);
	TextDrawBoxColor(banqueTD[28], 50);
	TextDrawUseBox(banqueTD[28], 1);
	TextDrawSetProportional(banqueTD[28], 1);
	TextDrawSetSelectable(banqueTD[28], 0);

	banqueTD[29] = TextDrawCreate(202.000000, 203.000000, "Saldo Carteira");
	TextDrawFont(banqueTD[29], 1);
	TextDrawLetterSize(banqueTD[29], 0.179166, 0.900000);
	TextDrawTextSize(banqueTD[29], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[29], 0);
	TextDrawSetShadow(banqueTD[29], 0);
	TextDrawAlignment(banqueTD[29], 1);
	TextDrawColor(banqueTD[29], -1094795521);
	TextDrawBackgroundColor(banqueTD[29], 255);
	TextDrawBoxColor(banqueTD[29], 50);
	TextDrawUseBox(banqueTD[29], 0);
	TextDrawSetProportional(banqueTD[29], 1);
	TextDrawSetSelectable(banqueTD[29], 0);

	banqueTD[30] = TextDrawCreate(202.000000, 212.000000, "R$");
	TextDrawFont(banqueTD[30], 1);
	TextDrawLetterSize(banqueTD[30], 0.337500, 1.349999);
	TextDrawTextSize(banqueTD[30], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[30], 1);
	TextDrawSetShadow(banqueTD[30], 0);
	TextDrawAlignment(banqueTD[30], 1);
	TextDrawColor(banqueTD[30], 16711935);
	TextDrawBackgroundColor(banqueTD[30], 255);
	TextDrawBoxColor(banqueTD[30], 50);
	TextDrawUseBox(banqueTD[30], 1);
	TextDrawSetProportional(banqueTD[30], 1);
	TextDrawSetSelectable(banqueTD[30], 0);

	banqueTD[31] = TextDrawCreate(219.000000, 212.000000, "0,00");
	TextDrawFont(banqueTD[31], 1);
	TextDrawLetterSize(banqueTD[31], 0.337500, 1.349999);
	TextDrawTextSize(banqueTD[31], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[31], 1);
	TextDrawSetShadow(banqueTD[31], 0);
	TextDrawAlignment(banqueTD[31], 1);
	TextDrawColor(banqueTD[31], -1);
	TextDrawBackgroundColor(banqueTD[31], 255);
	TextDrawBoxColor(banqueTD[31], 50);
	TextDrawUseBox(banqueTD[31], 1);
	TextDrawSetProportional(banqueTD[31], 1);
	TextDrawSetSelectable(banqueTD[31], 0);

	banqueTD[32] = TextDrawCreate(216.000000, 238.000000, "Deposito");
	TextDrawFont(banqueTD[32], 1);
	TextDrawLetterSize(banqueTD[32], 0.174999, 1.000000);
	TextDrawTextSize(banqueTD[32], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[32], 0);
	TextDrawSetShadow(banqueTD[32], 0);
	TextDrawAlignment(banqueTD[32], 2);
	TextDrawColor(banqueTD[32], 16711935);
	TextDrawBackgroundColor(banqueTD[32], 255);
	TextDrawBoxColor(banqueTD[32], 200);
	TextDrawUseBox(banqueTD[32], 0);
	TextDrawSetProportional(banqueTD[32], 1);
	TextDrawSetSelectable(banqueTD[32], 1);

	banqueTD[33] = TextDrawCreate(256.000000, 238.000000, "Saque");
	TextDrawFont(banqueTD[33], 1);
	TextDrawLetterSize(banqueTD[33], 0.174999, 1.000000);
	TextDrawTextSize(banqueTD[33], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[33], 0);
	TextDrawSetShadow(banqueTD[33], 0);
	TextDrawAlignment(banqueTD[33], 2);
	TextDrawColor(banqueTD[33], -1094795521);
	TextDrawBackgroundColor(banqueTD[33], 255);
	TextDrawBoxColor(banqueTD[33], 200);
	TextDrawUseBox(banqueTD[33], 0);
	TextDrawSetProportional(banqueTD[33], 1);
	TextDrawSetSelectable(banqueTD[33], 1);

	banqueTD[34] = TextDrawCreate(299.000000, 238.000000, "Tranferencia");
	TextDrawFont(banqueTD[34], 1);
	TextDrawLetterSize(banqueTD[34], 0.174999, 1.000000);
	TextDrawTextSize(banqueTD[34], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[34], 0);
	TextDrawSetShadow(banqueTD[34], 0);
	TextDrawAlignment(banqueTD[34], 2);
	TextDrawColor(banqueTD[34], -1094795521);
	TextDrawBackgroundColor(banqueTD[34], 255);
	TextDrawBoxColor(banqueTD[34], 200);
	TextDrawUseBox(banqueTD[34], 0);
	TextDrawSetProportional(banqueTD[34], 1);
	TextDrawSetSelectable(banqueTD[34], 1);

	banqueTD[35] = TextDrawCreate(237.000000, 234.000000, "l");
	TextDrawFont(banqueTD[35], 1);
	TextDrawLetterSize(banqueTD[35], 0.154166, 1.750000);
	TextDrawTextSize(banqueTD[35], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[35], 1);
	TextDrawSetShadow(banqueTD[35], 0);
	TextDrawAlignment(banqueTD[35], 1);
	TextDrawColor(banqueTD[35], -1094795521);
	TextDrawBackgroundColor(banqueTD[35], 255);
	TextDrawBoxColor(banqueTD[35], 50);
	TextDrawUseBox(banqueTD[35], 1);
	TextDrawSetProportional(banqueTD[35], 1);
	TextDrawSetSelectable(banqueTD[35], 0);

	banqueTD[36] = TextDrawCreate(273.000000, 234.000000, "l");
	TextDrawFont(banqueTD[36], 1);
	TextDrawLetterSize(banqueTD[36], 0.154166, 1.750000);
	TextDrawTextSize(banqueTD[36], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[36], 1);
	TextDrawSetShadow(banqueTD[36], 0);
	TextDrawAlignment(banqueTD[36], 1);
	TextDrawColor(banqueTD[36], -1094795521);
	TextDrawBackgroundColor(banqueTD[36], 255);
	TextDrawBoxColor(banqueTD[36], 50);
	TextDrawUseBox(banqueTD[36], 1);
	TextDrawSetProportional(banqueTD[36], 1);
	TextDrawSetSelectable(banqueTD[36], 0);

	banqueTD[37] = TextDrawCreate(259.000000, 257.000000, "_");
	TextDrawFont(banqueTD[37], 1);
	TextDrawLetterSize(banqueTD[37], 0.600000, 3.000003);
	TextDrawTextSize(banqueTD[37], 288.500000, 112.500000);
	TextDrawSetOutline(banqueTD[37], 1);
	TextDrawSetShadow(banqueTD[37], 0);
	TextDrawAlignment(banqueTD[37], 2);
	TextDrawColor(banqueTD[37], 1296911871);
	TextDrawBackgroundColor(banqueTD[37], 255);
	TextDrawBoxColor(banqueTD[37], 1296911751);
	TextDrawUseBox(banqueTD[37], 1);
	TextDrawSetProportional(banqueTD[37], 1);
	TextDrawSetSelectable(banqueTD[37], 0);

	banqueTD[38] = TextDrawCreate(204.000000, 255.000000, "Qual valor desejado ?");
	TextDrawFont(banqueTD[38], 1);
	TextDrawLetterSize(banqueTD[38], 0.183333, 1.000000);
	TextDrawTextSize(banqueTD[38], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[38], 0);
	TextDrawSetShadow(banqueTD[38], 0);
	TextDrawAlignment(banqueTD[38], 1);
	TextDrawColor(banqueTD[38], -1094795521);
	TextDrawBackgroundColor(banqueTD[38], 255);
	TextDrawBoxColor(banqueTD[38], 50);
	TextDrawUseBox(banqueTD[38], 0);
	TextDrawSetProportional(banqueTD[38], 1);
	TextDrawSetSelectable(banqueTD[38], 0);

	banqueTD[39] = TextDrawCreate(213.000000, 266.000000, "R$");
	TextDrawFont(banqueTD[39], 1);
	TextDrawLetterSize(banqueTD[39], 0.258332, 1.750000);
	TextDrawTextSize(banqueTD[39], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[39], 0);
	TextDrawSetShadow(banqueTD[39], 0);
	TextDrawAlignment(banqueTD[39], 2);
	TextDrawColor(banqueTD[39], -1);
	TextDrawBackgroundColor(banqueTD[39], 255);
	TextDrawBoxColor(banqueTD[39], 200);
	TextDrawUseBox(banqueTD[39], 0);
	TextDrawSetProportional(banqueTD[39], 1);
	TextDrawSetSelectable(banqueTD[39], 1);

	banqueTD[40] = TextDrawCreate(259.000000, 295.000000, "_");
	TextDrawFont(banqueTD[40], 1);
	TextDrawLetterSize(banqueTD[40], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[40], 286.000000, 113.000000);
	TextDrawSetOutline(banqueTD[40], 1);
	TextDrawSetShadow(banqueTD[40], 0);
	TextDrawAlignment(banqueTD[40], 2);
	TextDrawColor(banqueTD[40], -1);
	TextDrawBackgroundColor(banqueTD[40], 255);
	TextDrawBoxColor(banqueTD[40], 16711815);
	TextDrawUseBox(banqueTD[40], 1);
	TextDrawSetProportional(banqueTD[40], 1);
	TextDrawSetSelectable(banqueTD[40], 0);

	banqueTD[41] = TextDrawCreate(257.000000, 298.000000, "DEPOSITAR");
	TextDrawFont(banqueTD[41], 1);
	TextDrawLetterSize(banqueTD[41], 0.233332, 1.200000);
	TextDrawTextSize(banqueTD[41], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[41], 0);
	TextDrawSetShadow(banqueTD[41], 0);
	TextDrawAlignment(banqueTD[41], 2);
	TextDrawColor(banqueTD[41], 255);
	TextDrawBackgroundColor(banqueTD[41], 255);
	TextDrawBoxColor(banqueTD[41], 200);
	TextDrawUseBox(banqueTD[41], 0);
	TextDrawSetProportional(banqueTD[41], 1);
	TextDrawSetSelectable(banqueTD[41], 1);

	banqueTD[42] = TextDrawCreate(395.000000, 96.000000, "_");
	TextDrawFont(banqueTD[42], 2);
	TextDrawLetterSize(banqueTD[42], 0.600000, 8.800003);
	TextDrawTextSize(banqueTD[42], 328.500000, 125.000000);
	TextDrawSetOutline(banqueTD[42], 4);
	TextDrawSetShadow(banqueTD[42], 2);
	TextDrawAlignment(banqueTD[42], 2);
	TextDrawColor(banqueTD[42], 16711935);
	TextDrawBackgroundColor(banqueTD[42], 255);
	TextDrawBoxColor(banqueTD[42], 16711815);
	TextDrawUseBox(banqueTD[42], 1);
	TextDrawSetProportional(banqueTD[42], 1);
	TextDrawSetSelectable(banqueTD[42], 0);

	banqueTD[43] = TextDrawCreate(335.000000, 96.000000, "VISA");
	TextDrawFont(banqueTD[43], 1);
	TextDrawLetterSize(banqueTD[43], 0.270833, 1.250000);
	TextDrawTextSize(banqueTD[43], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[43], 0);
	TextDrawSetShadow(banqueTD[43], 0);
	TextDrawAlignment(banqueTD[43], 1);
	TextDrawColor(banqueTD[43], 255);
	TextDrawBackgroundColor(banqueTD[43], 255);
	TextDrawBoxColor(banqueTD[43], 50);
	TextDrawUseBox(banqueTD[43], 0);
	TextDrawSetProportional(banqueTD[43], 1);
	TextDrawSetSelectable(banqueTD[43], 0);

	banqueTD[44] = TextDrawCreate(442.000000, 104.000000, "_");
	TextDrawFont(banqueTD[44], 2);
	TextDrawLetterSize(banqueTD[44], 0.904166, 6.500003);
	TextDrawTextSize(banqueTD[44], 390.500000, 12.500000);
	TextDrawSetOutline(banqueTD[44], 4);
	TextDrawSetShadow(banqueTD[44], 2);
	TextDrawAlignment(banqueTD[44], 2);
	TextDrawColor(banqueTD[44], 16711935);
	TextDrawBackgroundColor(banqueTD[44], 255);
	TextDrawBoxColor(banqueTD[44], 135);
	TextDrawUseBox(banqueTD[44], 1);
	TextDrawSetProportional(banqueTD[44], 1);
	TextDrawSetSelectable(banqueTD[44], 0);

	banqueTD[45] = TextDrawCreate(442.000000, 104.000000, "_");
	TextDrawFont(banqueTD[45], 2);
	TextDrawLetterSize(banqueTD[45], 0.904166, 6.500003);
	TextDrawTextSize(banqueTD[45], 390.500000, 12.500000);
	TextDrawSetOutline(banqueTD[45], 4);
	TextDrawSetShadow(banqueTD[45], 2);
	TextDrawAlignment(banqueTD[45], 2);
	TextDrawColor(banqueTD[45], 16711935);
	TextDrawBackgroundColor(banqueTD[45], 255);
	TextDrawBoxColor(banqueTD[45], 135);
	TextDrawUseBox(banqueTD[45], 1);
	TextDrawSetProportional(banqueTD[45], 1);
	TextDrawSetSelectable(banqueTD[45], 0);

	banqueTD[46] = TextDrawCreate(442.000000, 104.000000, "_");
	TextDrawFont(banqueTD[46], 2);
	TextDrawLetterSize(banqueTD[46], 0.904166, 6.500003);
	TextDrawTextSize(banqueTD[46], 390.500000, 12.500000);
	TextDrawSetOutline(banqueTD[46], 4);
	TextDrawSetShadow(banqueTD[46], 2);
	TextDrawAlignment(banqueTD[46], 2);
	TextDrawColor(banqueTD[46], 16711935);
	TextDrawBackgroundColor(banqueTD[46], 255);
	TextDrawBoxColor(banqueTD[46], 135);
	TextDrawUseBox(banqueTD[46], 1);
	TextDrawSetProportional(banqueTD[46], 1);
	TextDrawSetSelectable(banqueTD[46], 0);

	banqueTD[47] = TextDrawCreate(442.000000, 104.000000, "_");
	TextDrawFont(banqueTD[47], 2);
	TextDrawLetterSize(banqueTD[47], 0.904166, 6.500003);
	TextDrawTextSize(banqueTD[47], 390.500000, 12.500000);
	TextDrawSetOutline(banqueTD[47], 4);
	TextDrawSetShadow(banqueTD[47], 2);
	TextDrawAlignment(banqueTD[47], 2);
	TextDrawColor(banqueTD[47], 16711935);
	TextDrawBackgroundColor(banqueTD[47], 255);
	TextDrawBoxColor(banqueTD[47], 135);
	TextDrawUseBox(banqueTD[47], 1);
	TextDrawSetProportional(banqueTD[47], 1);
	TextDrawSetSelectable(banqueTD[47], 0);

	banqueTD[48] = TextDrawCreate(444.000000, 123.000000, ")");
	TextDrawFont(banqueTD[48], 1);
	TextDrawLetterSize(banqueTD[48], 0.262500, 1.700000);
	TextDrawTextSize(banqueTD[48], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[48], 0);
	TextDrawSetShadow(banqueTD[48], 0);
	TextDrawAlignment(banqueTD[48], 1);
	TextDrawColor(banqueTD[48], -1);
	TextDrawBackgroundColor(banqueTD[48], 255);
	TextDrawBoxColor(banqueTD[48], 50);
	TextDrawUseBox(banqueTD[48], 0);
	TextDrawSetProportional(banqueTD[48], 1);
	TextDrawSetSelectable(banqueTD[48], 0);

	banqueTD[49] = TextDrawCreate(442.000000, 125.000000, ")");
	TextDrawFont(banqueTD[49], 1);
	TextDrawLetterSize(banqueTD[49], 0.212500, 1.400000);
	TextDrawTextSize(banqueTD[49], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[49], 0);
	TextDrawSetShadow(banqueTD[49], 0);
	TextDrawAlignment(banqueTD[49], 1);
	TextDrawColor(banqueTD[49], -1);
	TextDrawBackgroundColor(banqueTD[49], 255);
	TextDrawBoxColor(banqueTD[49], 50);
	TextDrawUseBox(banqueTD[49], 0);
	TextDrawSetProportional(banqueTD[49], 1);
	TextDrawSetSelectable(banqueTD[49], 0);

	banqueTD[50] = TextDrawCreate(440.000000, 127.000000, ")");
	TextDrawFont(banqueTD[50], 1);
	TextDrawLetterSize(banqueTD[50], 0.212500, 1.100000);
	TextDrawTextSize(banqueTD[50], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[50], 0);
	TextDrawSetShadow(banqueTD[50], 0);
	TextDrawAlignment(banqueTD[50], 1);
	TextDrawColor(banqueTD[50], -1);
	TextDrawBackgroundColor(banqueTD[50], 255);
	TextDrawBoxColor(banqueTD[50], 50);
	TextDrawUseBox(banqueTD[50], 0);
	TextDrawSetProportional(banqueTD[50], 1);
	TextDrawSetSelectable(banqueTD[50], 0);

	banqueTD[51] = TextDrawCreate(438.000000, 129.000000, ")");
	TextDrawFont(banqueTD[51], 1);
	TextDrawLetterSize(banqueTD[51], 0.212500, 0.750000);
	TextDrawTextSize(banqueTD[51], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[51], 0);
	TextDrawSetShadow(banqueTD[51], 0);
	TextDrawAlignment(banqueTD[51], 1);
	TextDrawColor(banqueTD[51], -1);
	TextDrawBackgroundColor(banqueTD[51], 255);
	TextDrawBoxColor(banqueTD[51], 50);
	TextDrawUseBox(banqueTD[51], 0);
	TextDrawSetProportional(banqueTD[51], 1);
	TextDrawSetSelectable(banqueTD[51], 0);

	banqueTD[52] = TextDrawCreate(410.000000, 161.000000, "02/25");
	TextDrawFont(banqueTD[52], 2);
	TextDrawLetterSize(banqueTD[52], 0.179166, 1.000000);
	TextDrawTextSize(banqueTD[52], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[52], 0);
	TextDrawSetShadow(banqueTD[52], 0);
	TextDrawAlignment(banqueTD[52], 1);
	TextDrawColor(banqueTD[52], 255);
	TextDrawBackgroundColor(banqueTD[52], 255);
	TextDrawBoxColor(banqueTD[52], 50);
	TextDrawUseBox(banqueTD[52], 0);
	TextDrawSetProportional(banqueTD[52], 1);
	TextDrawSetSelectable(banqueTD[52], 0);

	banqueTD[53] = TextDrawCreate(337.000000, 161.000000, "LEH SAMP");
	TextDrawFont(banqueTD[53], 1);
	TextDrawLetterSize(banqueTD[53], 0.270833, 1.000000);
	TextDrawTextSize(banqueTD[53], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[53], 0);
	TextDrawSetShadow(banqueTD[53], 0);
	TextDrawAlignment(banqueTD[53], 1);
	TextDrawColor(banqueTD[53], 255);
	TextDrawBackgroundColor(banqueTD[53], 255);
	TextDrawBoxColor(banqueTD[53], 50);
	TextDrawUseBox(banqueTD[53], 0);
	TextDrawSetProportional(banqueTD[53], 1);
	TextDrawSetSelectable(banqueTD[53], 0);

	banqueTD[54] = TextDrawCreate(335.000000, 128.000000, "oooo");
	TextDrawFont(banqueTD[54], 1);
	TextDrawLetterSize(banqueTD[54], 0.225000, 1.500000);
	TextDrawTextSize(banqueTD[54], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[54], 0);
	TextDrawSetShadow(banqueTD[54], 0);
	TextDrawAlignment(banqueTD[54], 1);
	TextDrawColor(banqueTD[54], 255);
	TextDrawBackgroundColor(banqueTD[54], 255);
	TextDrawBoxColor(banqueTD[54], 50);
	TextDrawUseBox(banqueTD[54], 0);
	TextDrawSetProportional(banqueTD[54], 1);
	TextDrawSetSelectable(banqueTD[54], 0);

	banqueTD[55] = TextDrawCreate(359.000000, 128.000000, "oooo");
	TextDrawFont(banqueTD[55], 1);
	TextDrawLetterSize(banqueTD[55], 0.225000, 1.500000);
	TextDrawTextSize(banqueTD[55], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[55], 0);
	TextDrawSetShadow(banqueTD[55], 0);
	TextDrawAlignment(banqueTD[55], 1);
	TextDrawColor(banqueTD[55], 255);
	TextDrawBackgroundColor(banqueTD[55], 255);
	TextDrawBoxColor(banqueTD[55], 50);
	TextDrawUseBox(banqueTD[55], 0);
	TextDrawSetProportional(banqueTD[55], 1);
	TextDrawSetSelectable(banqueTD[55], 0);

	banqueTD[56] = TextDrawCreate(383.000000, 128.000000, "oooo");
	TextDrawFont(banqueTD[56], 1);
	TextDrawLetterSize(banqueTD[56], 0.225000, 1.500000);
	TextDrawTextSize(banqueTD[56], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[56], 0);
	TextDrawSetShadow(banqueTD[56], 0);
	TextDrawAlignment(banqueTD[56], 1);
	TextDrawColor(banqueTD[56], 255);
	TextDrawBackgroundColor(banqueTD[56], 255);
	TextDrawBoxColor(banqueTD[56], 50);
	TextDrawUseBox(banqueTD[56], 0);
	TextDrawSetProportional(banqueTD[56], 1);
	TextDrawSetSelectable(banqueTD[56], 0);

	banqueTD[57] = TextDrawCreate(408.000000, 128.000000, "oooo");
	TextDrawFont(banqueTD[57], 1);
	TextDrawLetterSize(banqueTD[57], 0.225000, 1.500000);
	TextDrawTextSize(banqueTD[57], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[57], 0);
	TextDrawSetShadow(banqueTD[57], 0);
	TextDrawAlignment(banqueTD[57], 1);
	TextDrawColor(banqueTD[57], 255);
	TextDrawBackgroundColor(banqueTD[57], 255);
	TextDrawBoxColor(banqueTD[57], 50);
	TextDrawUseBox(banqueTD[57], 0);
	TextDrawSetProportional(banqueTD[57], 1);
	TextDrawSetSelectable(banqueTD[57], 0);

	banqueTD[58] = TextDrawCreate(395.000000, 188.000000, "_");
	TextDrawFont(banqueTD[58], 1);
	TextDrawLetterSize(banqueTD[58], 0.633333, 4.700004);
	TextDrawTextSize(banqueTD[58], 323.500000, 126.000000);
	TextDrawSetOutline(banqueTD[58], 1);
	TextDrawSetShadow(banqueTD[58], 0);
	TextDrawAlignment(banqueTD[58], 2);
	TextDrawColor(banqueTD[58], -1);
	TextDrawBackgroundColor(banqueTD[58], 255);
	TextDrawBoxColor(banqueTD[58], 1296911751);
	TextDrawUseBox(banqueTD[58], 1);
	TextDrawSetProportional(banqueTD[58], 1);
	TextDrawSetSelectable(banqueTD[58], 0);

	banqueTD[59] = TextDrawCreate(344.000000, 199.000000, "_");
	TextDrawFont(banqueTD[59], 1);
	TextDrawLetterSize(banqueTD[59], -0.145833, 1.400004);
	TextDrawTextSize(banqueTD[59], 189.500000, 12.000000);
	TextDrawSetOutline(banqueTD[59], 1);
	TextDrawSetShadow(banqueTD[59], 0);
	TextDrawAlignment(banqueTD[59], 2);
	TextDrawColor(banqueTD[59], -1);
	TextDrawBackgroundColor(banqueTD[59], 255);
	TextDrawBoxColor(banqueTD[59], 16711815);
	TextDrawUseBox(banqueTD[59], 1);
	TextDrawSetProportional(banqueTD[59], 1);
	TextDrawSetSelectable(banqueTD[59], 0);

	banqueTD[60] = TextDrawCreate(342.000000, 201.000000, "l");
	TextDrawFont(banqueTD[60], 1);
	TextDrawLetterSize(banqueTD[60], 0.308333, 1.500000);
	TextDrawTextSize(banqueTD[60], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[60], 0);
	TextDrawSetShadow(banqueTD[60], 0);
	TextDrawAlignment(banqueTD[60], 1);
	TextDrawColor(banqueTD[60], -1);
	TextDrawBackgroundColor(banqueTD[60], 255);
	TextDrawBoxColor(banqueTD[60], 50);
	TextDrawUseBox(banqueTD[60], 0);
	TextDrawSetProportional(banqueTD[60], 1);
	TextDrawSetSelectable(banqueTD[60], 0);

	banqueTD[61] = TextDrawCreate(340.000000, 201.000000, "/");
	TextDrawFont(banqueTD[61], 1);
	TextDrawLetterSize(banqueTD[61], 0.279166, 0.700000);
	TextDrawTextSize(banqueTD[61], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[61], 0);
	TextDrawSetShadow(banqueTD[61], 0);
	TextDrawAlignment(banqueTD[61], 1);
	TextDrawColor(banqueTD[61], -1);
	TextDrawBackgroundColor(banqueTD[61], 255);
	TextDrawBoxColor(banqueTD[61], 50);
	TextDrawUseBox(banqueTD[61], 0);
	TextDrawSetProportional(banqueTD[61], 1);
	TextDrawSetSelectable(banqueTD[61], 0);

	// [FIX] source exportait un backslash non echappe (cassait la compilation).
	// Un commentaire en fin de cette ligne precise casse aussi pawncc pour une
	// raison indeterminee : commentaire mis au-dessus, pas au bout de la ligne.
	banqueTD[62] = TextDrawCreate(343.000000, 201.000000, "\\");
	TextDrawFont(banqueTD[62], 1);
	TextDrawLetterSize(banqueTD[62], 0.279166, 0.700000);
	TextDrawTextSize(banqueTD[62], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[62], 0);
	TextDrawSetShadow(banqueTD[62], 0);
	TextDrawAlignment(banqueTD[62], 1);
	TextDrawColor(banqueTD[62], -1);
	TextDrawBackgroundColor(banqueTD[62], 255);
	TextDrawBoxColor(banqueTD[62], 50);
	TextDrawUseBox(banqueTD[62], 0);
	TextDrawSetProportional(banqueTD[62], 1);
	TextDrawSetSelectable(banqueTD[62], 0);

	banqueTD[63] = TextDrawCreate(354.000000, 196.000000, "Depositado");
	TextDrawFont(banqueTD[63], 1);
	TextDrawLetterSize(banqueTD[63], 0.145833, 0.800000);
	TextDrawTextSize(banqueTD[63], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[63], 0);
	TextDrawSetShadow(banqueTD[63], 0);
	TextDrawAlignment(banqueTD[63], 1);
	TextDrawColor(banqueTD[63], -1094795521);
	TextDrawBackgroundColor(banqueTD[63], 255);
	TextDrawBoxColor(banqueTD[63], 50);
	TextDrawUseBox(banqueTD[63], 0);
	TextDrawSetProportional(banqueTD[63], 1);
	TextDrawSetSelectable(banqueTD[63], 0);

	banqueTD[64] = TextDrawCreate(366.000000, 203.000000, "R$ 300,00");
	TextDrawFont(banqueTD[64], 1);
	TextDrawLetterSize(banqueTD[64], 0.137499, 1.250000);
	TextDrawTextSize(banqueTD[64], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[64], 0);
	TextDrawSetShadow(banqueTD[64], 0);
	TextDrawAlignment(banqueTD[64], 2);
	TextDrawColor(banqueTD[64], -1);
	TextDrawBackgroundColor(banqueTD[64], 255);
	TextDrawBoxColor(banqueTD[64], 200);
	TextDrawUseBox(banqueTD[64], 0);
	TextDrawSetProportional(banqueTD[64], 1);
	TextDrawSetSelectable(banqueTD[64], 0); // [FIX] source le laissait cliquable par erreur (artefact d'export, toast demo "$300,00") : repris comme toast de confirmation, jamais un bouton

	banqueTD[65] = TextDrawCreate(407.000000, 199.000000, "_");
	TextDrawFont(banqueTD[65], 1);
	TextDrawLetterSize(banqueTD[65], -0.145833, 1.400004);
	TextDrawTextSize(banqueTD[65], 189.500000, 12.000000);
	TextDrawSetOutline(banqueTD[65], 1);
	TextDrawSetShadow(banqueTD[65], 0);
	TextDrawAlignment(banqueTD[65], 2);
	TextDrawColor(banqueTD[65], -1);
	TextDrawBackgroundColor(banqueTD[65], 255);
	TextDrawBoxColor(banqueTD[65], 16711815);
	TextDrawUseBox(banqueTD[65], 1);
	TextDrawSetProportional(banqueTD[65], 1);
	TextDrawSetSelectable(banqueTD[65], 0);

	banqueTD[66] = TextDrawCreate(405.000000, 195.000000, "l");
	TextDrawFont(banqueTD[66], 1);
	TextDrawLetterSize(banqueTD[66], 0.308333, 1.500000);
	TextDrawTextSize(banqueTD[66], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[66], 0);
	TextDrawSetShadow(banqueTD[66], 0);
	TextDrawAlignment(banqueTD[66], 1);
	TextDrawColor(banqueTD[66], -1);
	TextDrawBackgroundColor(banqueTD[66], 255);
	TextDrawBoxColor(banqueTD[66], 50);
	TextDrawUseBox(banqueTD[66], 0);
	TextDrawSetProportional(banqueTD[66], 1);
	TextDrawSetSelectable(banqueTD[66], 0);

	banqueTD[67] = TextDrawCreate(406.000000, 200.000000, "/");
	TextDrawFont(banqueTD[67], 1);
	TextDrawLetterSize(banqueTD[67], 0.279166, 0.700000);
	TextDrawTextSize(banqueTD[67], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[67], 0);
	TextDrawSetShadow(banqueTD[67], 0);
	TextDrawAlignment(banqueTD[67], 1);
	TextDrawColor(banqueTD[67], -1);
	TextDrawBackgroundColor(banqueTD[67], 255);
	TextDrawBoxColor(banqueTD[67], 50);
	TextDrawUseBox(banqueTD[67], 0);
	TextDrawSetProportional(banqueTD[67], 1);
	TextDrawSetSelectable(banqueTD[67], 0);

	// [FIX] idem qu'element 62 : meme icone "check" en ASCII (l / \), commentaire au-dessus pour la meme raison.
	banqueTD[68] = TextDrawCreate(403.000000, 200.000000, "\\");
	TextDrawFont(banqueTD[68], 1);
	TextDrawLetterSize(banqueTD[68], 0.279166, 0.700000);
	TextDrawTextSize(banqueTD[68], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[68], 0);
	TextDrawSetShadow(banqueTD[68], 0);
	TextDrawAlignment(banqueTD[68], 1);
	TextDrawColor(banqueTD[68], -1);
	TextDrawBackgroundColor(banqueTD[68], 255);
	TextDrawBoxColor(banqueTD[68], 50);
	TextDrawUseBox(banqueTD[68], 0);
	TextDrawSetProportional(banqueTD[68], 1);
	TextDrawSetSelectable(banqueTD[68], 0);

	banqueTD[69] = TextDrawCreate(417.000000, 196.000000, "Retirado");
	TextDrawFont(banqueTD[69], 1);
	TextDrawLetterSize(banqueTD[69], 0.145833, 0.800000);
	TextDrawTextSize(banqueTD[69], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[69], 0);
	TextDrawSetShadow(banqueTD[69], 0);
	TextDrawAlignment(banqueTD[69], 1);
	TextDrawColor(banqueTD[69], -1094795521);
	TextDrawBackgroundColor(banqueTD[69], 255);
	TextDrawBoxColor(banqueTD[69], 50);
	TextDrawUseBox(banqueTD[69], 0);
	TextDrawSetProportional(banqueTD[69], 1);
	TextDrawSetSelectable(banqueTD[69], 0);

	banqueTD[70] = TextDrawCreate(429.000000, 203.000000, "R$ 300,00");
	TextDrawFont(banqueTD[70], 1);
	TextDrawLetterSize(banqueTD[70], 0.137499, 1.250000);
	TextDrawTextSize(banqueTD[70], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[70], 0);
	TextDrawSetShadow(banqueTD[70], 0);
	TextDrawAlignment(banqueTD[70], 2);
	TextDrawColor(banqueTD[70], -1);
	TextDrawBackgroundColor(banqueTD[70], 255);
	TextDrawBoxColor(banqueTD[70], 200);
	TextDrawUseBox(banqueTD[70], 0);
	TextDrawSetProportional(banqueTD[70], 1);
	TextDrawSetSelectable(banqueTD[70], 0); // [FIX] idem : toast "Retirado", jamais un bouton

	banqueTD[71] = TextDrawCreate(381.000000, 196.000000, "ld_beat:up");
	TextDrawFont(banqueTD[71], 4);
	TextDrawLetterSize(banqueTD[71], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[71], 7.000000, 9.500000);
	TextDrawSetOutline(banqueTD[71], 1);
	TextDrawSetShadow(banqueTD[71], 0);
	TextDrawAlignment(banqueTD[71], 1);
	TextDrawColor(banqueTD[71], 16711935);
	TextDrawBackgroundColor(banqueTD[71], 255);
	TextDrawBoxColor(banqueTD[71], 50);
	TextDrawUseBox(banqueTD[71], 1);
	TextDrawSetProportional(banqueTD[71], 1);
	TextDrawSetSelectable(banqueTD[71], 0);

	banqueTD[72] = TextDrawCreate(438.000000, 195.000000, "ld_beat:down");
	TextDrawFont(banqueTD[72], 4);
	TextDrawLetterSize(banqueTD[72], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[72], 7.000000, 9.500000);
	TextDrawSetOutline(banqueTD[72], 1);
	TextDrawSetShadow(banqueTD[72], 0);
	TextDrawAlignment(banqueTD[72], 1);
	TextDrawColor(banqueTD[72], -16776961);
	TextDrawBackgroundColor(banqueTD[72], 255);
	TextDrawBoxColor(banqueTD[72], 50);
	TextDrawUseBox(banqueTD[72], 1);
	TextDrawSetProportional(banqueTD[72], 1);
	TextDrawSetSelectable(banqueTD[72], 0);

	banqueTD[73] = TextDrawCreate(395.000000, 239.000000, "_");
	TextDrawFont(banqueTD[73], 1);
	TextDrawLetterSize(banqueTD[73], 0.583333, 8.350016);
	TextDrawTextSize(banqueTD[73], 343.000000, 125.500000);
	TextDrawSetOutline(banqueTD[73], 1);
	TextDrawSetShadow(banqueTD[73], 0);
	TextDrawAlignment(banqueTD[73], 2);
	TextDrawColor(banqueTD[73], -1);
	TextDrawBackgroundColor(banqueTD[73], 255);
	TextDrawBoxColor(banqueTD[73], 1296911751);
	TextDrawUseBox(banqueTD[73], 1);
	TextDrawSetProportional(banqueTD[73], 1);
	TextDrawSetSelectable(banqueTD[73], 0);

	banqueTD[74] = TextDrawCreate(334.000000, 239.000000, "Emprestimo");
	TextDrawFont(banqueTD[74], 1);
	TextDrawLetterSize(banqueTD[74], 0.216666, 1.250000);
	TextDrawTextSize(banqueTD[74], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[74], 0);
	TextDrawSetShadow(banqueTD[74], 0);
	TextDrawAlignment(banqueTD[74], 1);
	TextDrawColor(banqueTD[74], -1);
	TextDrawBackgroundColor(banqueTD[74], 255);
	TextDrawBoxColor(banqueTD[74], 50);
	TextDrawUseBox(banqueTD[74], 0);
	TextDrawSetProportional(banqueTD[74], 1);
	TextDrawSetSelectable(banqueTD[74], 0);

	banqueTD[75] = TextDrawCreate(335.000000, 253.000000, "Esta apertado sem dinheiro ?");
	TextDrawFont(banqueTD[75], 1);
	TextDrawLetterSize(banqueTD[75], 0.129166, 0.800000);
	TextDrawTextSize(banqueTD[75], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[75], 0);
	TextDrawSetShadow(banqueTD[75], 0);
	TextDrawAlignment(banqueTD[75], 1);
	TextDrawColor(banqueTD[75], -1094795521);
	TextDrawBackgroundColor(banqueTD[75], 255);
	TextDrawBoxColor(banqueTD[75], 50);
	TextDrawUseBox(banqueTD[75], 0);
	TextDrawSetProportional(banqueTD[75], 1);
	TextDrawSetSelectable(banqueTD[75], 0);

	banqueTD[76] = TextDrawCreate(335.000000, 260.000000, "Voce ja pode realizar emprestimos no nosso banco com total seguranca e sem problemas");
	TextDrawFont(banqueTD[76], 1);
	TextDrawLetterSize(banqueTD[76], 0.149999, 0.650000);
	TextDrawTextSize(banqueTD[76], 400.000000, 17.000000);
	TextDrawSetOutline(banqueTD[76], 0);
	TextDrawSetShadow(banqueTD[76], 0);
	TextDrawAlignment(banqueTD[76], 1);
	TextDrawColor(banqueTD[76], -1094795521);
	TextDrawBackgroundColor(banqueTD[76], 255);
	TextDrawBoxColor(banqueTD[76], 50);
	TextDrawUseBox(banqueTD[76], 0);
	TextDrawSetProportional(banqueTD[76], 1);
	TextDrawSetSelectable(banqueTD[76], 0);

	banqueTD[77] = TextDrawCreate(395.000000, 297.000000, "Realizar Emprestimo");
	TextDrawFont(banqueTD[77], 1);
	TextDrawLetterSize(banqueTD[77], 0.199999, 1.350000);
	TextDrawTextSize(banqueTD[77], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[77], 0);
	TextDrawSetShadow(banqueTD[77], 0);
	TextDrawAlignment(banqueTD[77], 2);
	TextDrawColor(banqueTD[77], 255);
	TextDrawBackgroundColor(banqueTD[77], 255);
	TextDrawBoxColor(banqueTD[77], 200);
	TextDrawUseBox(banqueTD[77], 0);
	TextDrawSetProportional(banqueTD[77], 1);
	TextDrawSetSelectable(banqueTD[77], 1);

	banqueTD[78] = TextDrawCreate(395.000000, 295.000000, "_");
	TextDrawFont(banqueTD[78], 1);
	TextDrawLetterSize(banqueTD[78], 0.600000, 2.000000);
	TextDrawTextSize(banqueTD[78], 286.000000, 113.000000);
	TextDrawSetOutline(banqueTD[78], 1);
	TextDrawSetShadow(banqueTD[78], 0);
	TextDrawAlignment(banqueTD[78], 2);
	TextDrawColor(banqueTD[78], -1);
	TextDrawBackgroundColor(banqueTD[78], 255);
	TextDrawBoxColor(banqueTD[78], 16711815);
	TextDrawUseBox(banqueTD[78], 1);
	TextDrawSetProportional(banqueTD[78], 1);
	TextDrawSetSelectable(banqueTD[78], 0);



	// [NOUVEAU] banqueTD[79] "Rembourser" - seul element hors des 79 de la source
	// (voir en-tete). Clone exact du style du bouton 77, texte + couleur changes
	// pour le distinguer comme action secondaire, place juste en dessous.
	banqueTD[79] = TextDrawCreate(395.000000, 313.000000, "Rembourser");
	TextDrawFont(banqueTD[79], 1);
	TextDrawLetterSize(banqueTD[79], 0.199999, 1.350000);
	TextDrawTextSize(banqueTD[79], 16.500000, 90.500000);
	TextDrawSetOutline(banqueTD[79], 0);
	TextDrawSetShadow(banqueTD[79], 0);
	TextDrawAlignment(banqueTD[79], 2);
	TextDrawColor(banqueTD[79], -1);
	TextDrawBackgroundColor(banqueTD[79], 255);
	TextDrawBoxColor(banqueTD[79], 200);
	TextDrawUseBox(banqueTD[79], 0);
	TextDrawSetProportional(banqueTD[79], 1);
	TextDrawSetSelectable(banqueTD[79], 1);

	// Timer de masquage des toasts (voir Banque_ToastTick) : un seul timer
	// partage qui scanne les joueurs, comme Cv_Tick/Gps_Tick ailleurs dans ce
	// gamemode, plutot qu'un SetTimerEx par transaction.
	SetTimer("Banque_ToastTick", 500, true);
	return 1;
}

// ============================================================================
// OUTILS
// ============================================================================
stock Banque_Msg(playerid, const text[])
{
	return SendClientMessage(playerid, 0xFFFFFFFF, text);
}

// Solde banque (28) / solde liquide (31) : format "R$ 1.234,00". Ce gamemode
// ne gere pas les centimes, donc ",00" est un suffixe statique qui colle au
// visuel du pack sans inventer une vraie gestion decimale.
stock Banque_MettreAJourSoldes(playerid)
{
	new montantStr[24], s[32];

	FormatMoney(PlayerInfo[playerid][pAccount], montantStr, sizeof(montantStr));
	format(s, sizeof(s), "%s,00", montantStr);
	TextDrawSetString(banqueTD[28], s);
	if(banqOuvert[playerid]) TextDrawShowForPlayer(playerid, banqueTD[28]);

	FormatMoney(PlayerInfo[playerid][pCash], montantStr, sizeof(montantStr));
	format(s, sizeof(s), "%s,00", montantStr);
	TextDrawSetString(banqueTD[31], s);
	if(banqOuvert[playerid]) TextDrawShowForPlayer(playerid, banqueTD[31]);
	return 1;
}

// Textes 75/76 : accroche marketing du pack par defaut, remplacee par l'etat
// reel de l'emprunt en cours des qu'il y en a un.
stock Banque_MettreAJourEmprunt(playerid)
{
	new montant = PlayerInfo[playerid][pLoanAmount];

	if(montant <= 0)
	{
		TextDrawSetString(banqueTD[75], "Esta apertado sem dinheiro ?");
		TextDrawSetString(banqueTD[76], "Voce ja pode realizar emprestimos no nosso banco com total seguranca e sem problemas");
	}
	else
	{
		new montantStr[24], s[144];
		FormatMoney(montant, montantStr, sizeof(montantStr));
		format(s, sizeof(s), "Emprunt en cours : $%s", montantStr);
		TextDrawSetString(banqueTD[75], s);
		TextDrawSetString(banqueTD[76], "Cliquez Rembourser pour regler depuis votre solde bancaire.");
	}

	if(banqOuvert[playerid])
	{
		TextDrawShowForPlayer(playerid, banqueTD[75]);
		TextDrawShowForPlayer(playerid, banqueTD[76]);
	}
	return 1;
}

// Recolore les 3 onglets (32/33/34) et retient lequel est actif pour R$/DEPOSITAR.
stock Banque_SelectionnerOnglet(playerid, onglet)
{
	banqPending[playerid] = onglet;

	TextDrawColor(banqueTD[32], (onglet == BANQ_ETAPE_DEPOT)    ? (BANQ_COULEUR_ACTIF) : (BANQ_COULEUR_INACTIF));
	TextDrawColor(banqueTD[33], (onglet == BANQ_ETAPE_RETRAIT)  ? (BANQ_COULEUR_ACTIF) : (BANQ_COULEUR_INACTIF));
	TextDrawColor(banqueTD[34], (onglet == BANQ_ETAPE_VIREMENT) ? (BANQ_COULEUR_ACTIF) : (BANQ_COULEUR_INACTIF));

	if(banqOuvert[playerid])
	{
		TextDrawShowForPlayer(playerid, banqueTD[32]);
		TextDrawShowForPlayer(playerid, banqueTD[33]);
		TextDrawShowForPlayer(playerid, banqueTD[34]);
	}
	return 1;
}

stock Banque_CacherToasts(playerid)
{
	for(new i = 0; i < sizeof(banqIdxToastDepot); i++)   TextDrawHideForPlayer(playerid, banqueTD[banqIdxToastDepot[i]]);
	for(new i = 0; i < sizeof(banqIdxToastRetrait); i++) TextDrawHideForPlayer(playerid, banqueTD[banqIdxToastRetrait[i]]);
	banqToastExpire[playerid] = 0;
	return 1;
}

stock Banque_ToastDepot(playerid, montant)
{
	new montantStr[24], s[32];
	FormatMoney(montant, montantStr, sizeof(montantStr));
	format(s, sizeof(s), "R$ %s,00", montantStr);
	TextDrawSetString(banqueTD[64], s);

	for(new i = 0; i < sizeof(banqIdxToastRetrait); i++) TextDrawHideForPlayer(playerid, banqueTD[banqIdxToastRetrait[i]]); // un seul toast a la fois
	for(new i = 0; i < sizeof(banqIdxToastDepot); i++)   TextDrawShowForPlayer(playerid, banqueTD[banqIdxToastDepot[i]]);
	banqToastExpire[playerid] = GetTickCount() + BANQ_TOAST_DUREE_MS;
	return 1;
}

stock Banque_ToastRetrait(playerid, montant)
{
	new montantStr[24], s[32];
	FormatMoney(montant, montantStr, sizeof(montantStr));
	format(s, sizeof(s), "R$ %s,00", montantStr);
	TextDrawSetString(banqueTD[70], s);

	for(new i = 0; i < sizeof(banqIdxToastDepot); i++)   TextDrawHideForPlayer(playerid, banqueTD[banqIdxToastDepot[i]]); // un seul toast a la fois
	for(new i = 0; i < sizeof(banqIdxToastRetrait); i++) TextDrawShowForPlayer(playerid, banqueTD[banqIdxToastRetrait[i]]);
	banqToastExpire[playerid] = GetTickCount() + BANQ_TOAST_DUREE_MS;
	return 1;
}

forward Banque_ToastTick();
public Banque_ToastTick()
{
	new tick = GetTickCount();
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		if(!banqOuvert[i]) continue;
		if(banqToastExpire[i] == 0) continue;
		if(tick < banqToastExpire[i]) continue;
		Banque_CacherToasts(i);
	}
	return 1;
}

// ============================================================================
// OUVRIR / FERMER
// ============================================================================
stock Banque_Ouvrir(playerid)
{
	// Budget de textdraws partage avec le telephone / le sac / le craft, comme
	// SacV_Ouvrir le fait deja pour les memes raisons : on ferme ce qui peut
	// l'etre, on refuse si le vieux telephone (usingPhone) est ouvert.
	if(phone_State[playerid] != PHONE_STATE_CLOSED) phone_Close(playerid);
	if(usingPhone[playerid] == 1)
	{
		Banque_Msg(playerid, "{FF8888}Fermez d'abord votre telephone.");
		return 0;
	}
	if(svOuvert[playerid] == 1) SacV_Fermer(playerid);
	TC_Fermer(playerid);

	banqOuvert[playerid] = true;
	banqEtape[playerid] = 0;

	for(new i = 0; i < BANQ_TD_COUNT; i++) TextDrawShowForPlayer(playerid, banqueTD[i]);
	Banque_CacherToasts(playerid); // les toasts ne s'affichent qu'apres une operation reussie
	Banque_SelectionnerOnglet(playerid, BANQ_ETAPE_DEPOT); // Deposito actif par defaut, comme dans le pack
	Banque_MettreAJourSoldes(playerid);
	Banque_MettreAJourEmprunt(playerid);

	SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
	return 1;
}

stock Banque_Fermer(playerid)
{
	if(!banqOuvert[playerid]) return 0;
	banqOuvert[playerid] = false;
	banqEtape[playerid] = 0;

	for(new i = 0; i < BANQ_TD_COUNT; i++) TextDrawHideForPlayer(playerid, banqueTD[i]);
	banqToastExpire[playerid] = 0;

	CancelSelectTextDraw(playerid);
	return 1;
}

// ============================================================================
// SAISIE DU MONTANT (dialogue par-dessus le panneau, voir en-tete)
// ============================================================================
stock Banque_DemanderMontant(playerid, etape)
{
	new corps[160], titre[48];

	switch(etape)
	{
		case BANQ_ETAPE_DEPOT:
		{
			format(titre, sizeof(titre), "{AADD66} Banque {FFFFFF} Depot");
			format(corps, sizeof(corps), "Depot maximum : $%d.\n\nEntrez le montant que vous souhaitez deposer :", PlayerInfo[playerid][pCash]);
		}
		case BANQ_ETAPE_RETRAIT:
		{
			format(titre, sizeof(titre), "{AADD66} Banque {FFFFFF} Retrait");
			format(corps, sizeof(corps), "Retrait maximum : $%d.\n\nEntrez le montant que vous souhaitez retirer :", PlayerInfo[playerid][pAccount]);
		}
		case BANQ_ETAPE_VIREMENT:
		{
			format(titre, sizeof(titre), "{AADD66} Banque {FFFFFF} Virement");
			format(corps, sizeof(corps), "Virement maximum : $%d.\n\nEntrez le montant puis l'ID du joueur separes d'un '/'.\nEx : 3600/2 :", PlayerInfo[playerid][pAccount]);
		}
		case BANQ_ETAPE_EMPRUNT:
		{
			if(PlayerInfo[playerid][pLoanAmount] > 0)
			{
				Banque_Msg(playerid, "{FF4444}Vous avez deja un emprunt en cours. Remboursez-le avant d'en redemander un.");
				return 0;
			}
			format(titre, sizeof(titre), "{AADD66} Banque {FFFFFF} Emprunt");
			format(corps, sizeof(corps), "Emprunt maximum : $%d (frais de %d%% inclus).\n\nEntrez le montant que vous souhaitez emprunter :", BANQ_PRET_MAX, BANQ_PRET_FRAIS_POURCENT);
		}
		case BANQ_ETAPE_REMBOURSER:
		{
			if(PlayerInfo[playerid][pLoanAmount] <= 0)
			{
				Banque_Msg(playerid, "{FF4444}Vous n'avez aucun emprunt en cours.");
				return 0;
			}
			format(titre, sizeof(titre), "{AADD66} Banque {FFFFFF} Remboursement");
			format(corps, sizeof(corps), "Vous devez : $%d.\n\nEntrez le montant a rembourser (max : votre solde bancaire) :", PlayerInfo[playerid][pLoanAmount]);
		}
		default: return 0;
	}

	banqEtape[playerid] = etape;
	ShowPlayerDialog(playerid, DIALOG_BANQUE, DIALOG_STYLE_INPUT, titre, corps, "Valider", "Annuler");
	return 1;
}

// ============================================================================
// CLICS
// ============================================================================
stock Banque_OnClick(playerid, Text:clickedid)
{
	if(!banqOuvert[playerid]) return 0;

	if(clickedid == Text:INVALID_TEXT_DRAW)
	{
		Banque_Fermer(playerid);
		return 1;
	}
	if(clickedid == banqueTD[32]) { Banque_SelectionnerOnglet(playerid, BANQ_ETAPE_DEPOT); return 1; }
	if(clickedid == banqueTD[33]) { Banque_SelectionnerOnglet(playerid, BANQ_ETAPE_RETRAIT); return 1; }
	if(clickedid == banqueTD[34]) { Banque_SelectionnerOnglet(playerid, BANQ_ETAPE_VIREMENT); return 1; }

	if(clickedid == banqueTD[39] || clickedid == banqueTD[41])
	{
		Banque_DemanderMontant(playerid, banqPending[playerid]);
		if(banqOuvert[playerid]) SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
		return 1;
	}
	if(clickedid == banqueTD[77])
	{
		Banque_DemanderMontant(playerid, BANQ_ETAPE_EMPRUNT);
		if(banqOuvert[playerid]) SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
		return 1;
	}
	if(clickedid == banqueTD[79])
	{
		Banque_DemanderMontant(playerid, BANQ_ETAPE_REMBOURSER);
		if(banqOuvert[playerid]) SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
		return 1;
	}
	return 0; // pas un des notres (ex: pay_TextButton en select mode simultane) : laisser passer
}

// ============================================================================
// REPONSE AU DIALOGUE DE MONTANT
// ============================================================================
stock Banque_OnDialog(playerid, dialogid, response, const inputtext[])
{
	if(dialogid != DIALOG_BANQUE) return 0;

	if(!response)
	{
		banqEtape[playerid] = 0;
		if(banqOuvert[playerid]) SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
		return 1;
	}

	new etape = banqEtape[playerid];
	banqEtape[playerid] = 0;

	switch(etape)
	{
		case BANQ_ETAPE_DEPOT:      Banque_TraiterDepot(playerid, inputtext);
		case BANQ_ETAPE_RETRAIT:    Banque_TraiterRetrait(playerid, inputtext);
		case BANQ_ETAPE_VIREMENT:   Banque_TraiterVirement(playerid, inputtext);
		case BANQ_ETAPE_EMPRUNT:    Banque_TraiterEmprunt(playerid, inputtext);
		case BANQ_ETAPE_REMBOURSER: Banque_TraiterRemboursement(playerid, inputtext);
	}

	if(banqOuvert[playerid]) SelectTextDraw(playerid, BANQ_SELECT_COULEUR);
	return 1;
}

// ============================================================================
// DEPOT / RETRAIT / VIREMENT - logique reprise de l'ancien dialog 86 (voir en-tete)
// ============================================================================
stock Banque_TraiterDepot(playerid, const inputtext[])
{
	if(!strlen(inputtext) || strlen(inputtext) > 9)
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_DEPOT);
	}
	new montant = strval(inputtext);
	if(montant < 1 || montant > PlayerInfo[playerid][pCash])
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide (vous n'avez pas ce montant en liquide).");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_DEPOT);
	}

	SafeGivePlayerMoney(playerid, -montant, "Depot argent sur son compte banquaire");
	PlayerInfo[playerid][pAccount] += montant;

	Banque_MettreAJourSoldes(playerid);
	Banque_ToastDepot(playerid, montant);

	new s[144];
	format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Depot de $%d effectue. Solde bancaire : $%d.", montant, PlayerInfo[playerid][pAccount]);
	Banque_Msg(playerid, s);
	return 1;
}

stock Banque_TraiterRetrait(playerid, const inputtext[])
{
	if(!strlen(inputtext) || strlen(inputtext) > 9)
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_RETRAIT);
	}
	new montant = strval(inputtext);
	if(montant < 1 || montant > PlayerInfo[playerid][pAccount])
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide (solde bancaire insuffisant).");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_RETRAIT);
	}

	SafeGivePlayerMoney(playerid, montant, "Retire argent de son compte banquaire");
	PlayerInfo[playerid][pAccount] -= montant;

	Banque_MettreAJourSoldes(playerid);
	Banque_ToastRetrait(playerid, montant);

	new s[144];
	format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Retrait de $%d effectue. Solde bancaire : $%d.", montant, PlayerInfo[playerid][pAccount]);
	Banque_Msg(playerid, s);
	return 1;
}

stock Banque_TraiterVirement(playerid, const inputtext[])
{
	if(!strlen(inputtext) || strlen(inputtext) > 12 || strfind(inputtext, "/", true) == -1)
	{
		Banque_Msg(playerid, "{FF4444}Format invalide. Ex : 3600/2");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_VIREMENT);
	}

	// [12 lignes, pas 3] split() ne verifie pas les bornes du tableau de
	// destination (voir stock.inc / le split() du gamemode) : avec la limite de
	// 12 caracteres ci-dessus, un texte comme "1/2/3/4/5/2" tient et donnerait
	// 6 tokens. Meme marge que l'ancien dialog 86, qui utilisait deja
	// virement[12][16].
	new virement[12][16];
	split(inputtext, virement, '/');
	if(!strlen(virement[0]) || !strlen(virement[1]) || strlen(virement[2]) > 0)
	{
		Banque_Msg(playerid, "{FF4444}Format invalide. Ex : 3600/2");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_VIREMENT);
	}

	new montant = strval(virement[0]);
	new cible = strval(virement[1]);

	if(cible == playerid)
	{
		Banque_Msg(playerid, "{FF4444}Vous ne pouvez pas vous virer a vous-meme.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_VIREMENT);
	}
	// [FIX] l'ancien dialog 86 ne verifiait pas la connexion de la cible avant
	// de crediter : un ID deconnecte/invalide aurait ecrit dans une case memoire
	// au hasard. Voir en-tete.
	if(cible < 0 || cible >= MAX_PLAYERS || !IsPlayerConnected(cible))
	{
		Banque_Msg(playerid, "{FF4444}Joueur introuvable.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_VIREMENT);
	}
	if(montant < 1 || montant > PlayerInfo[playerid][pAccount])
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide (solde bancaire insuffisant).");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_VIREMENT);
	}

	PlayerInfo[playerid][pAccount] -= montant;
	PlayerInfo[cible][pAccount] += montant;

	new sendername[MAX_PLAYER_NAME+1], s[144];
	GetPlayerName(playerid, sendername, sizeof(sendername));
	format(s, sizeof(s), "* %s fait un virement.", sendername);
	ProxDetector(30.0, playerid, s, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, COLOR_PURPLE, true);

	format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Virement de $%d envoye. Solde bancaire : $%d.", montant, PlayerInfo[playerid][pAccount]);
	Banque_Msg(playerid, s);
	format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Vous recevez un virement de %s : $%d.", sendername, montant);
	Banque_Msg(cible, s);

	Banque_MettreAJourSoldes(playerid);
	return 1;
}

// ============================================================================
// EMPRUNT - vrai nouveau systeme (voir en-tete pour les valeurs par defaut)
// ============================================================================
stock Banque_TraiterEmprunt(playerid, const inputtext[])
{
	// Re-verifie (le joueur a pu obtenir un emprunt par un autre moyen pendant
	// que le dialogue etait ouvert) plutot que de faire confiance a l'etat vu
	// au clic initial.
	if(PlayerInfo[playerid][pLoanAmount] > 0)
	{
		Banque_Msg(playerid, "{FF4444}Vous avez deja un emprunt en cours.");
		return 1;
	}
	if(!strlen(inputtext) || strlen(inputtext) > 9)
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_EMPRUNT);
	}
	new montant = strval(inputtext);
	if(montant < 1 || montant > BANQ_PRET_MAX)
	{
		new s[96];
		format(s, sizeof(s), "{FF4444}Montant invalide (max $%d).", BANQ_PRET_MAX);
		Banque_Msg(playerid, s);
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_EMPRUNT);
	}

	// Frais fixe de BANQ_PRET_FRAIS_POURCENT% preleve une seule fois a l'emprunt,
	// pas d'interet qui court dans le temps (voir en-tete).
	new du = montant + ((montant * BANQ_PRET_FRAIS_POURCENT) / 100);
	PlayerInfo[playerid][pAccount] += montant;   // credite sur le compte, jamais en liquide
	PlayerInfo[playerid][pLoanAmount] = du;

	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Bank", PlayerInfo[playerid][pAccount]);
	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Loan", PlayerInfo[playerid][pLoanAmount]);

	Banque_MettreAJourSoldes(playerid);
	Banque_MettreAJourEmprunt(playerid);

	new s[160];
	format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Emprunt de $%d accorde (frais %d%% inclus, vous devez $%d). Credite sur votre compte bancaire.", montant, BANQ_PRET_FRAIS_POURCENT, du);
	Banque_Msg(playerid, s);
	return 1;
}

stock Banque_TraiterRemboursement(playerid, const inputtext[])
{
	if(PlayerInfo[playerid][pLoanAmount] <= 0)
	{
		Banque_Msg(playerid, "{FF4444}Vous n'avez aucun emprunt en cours.");
		return 1;
	}
	if(!strlen(inputtext) || strlen(inputtext) > 9)
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide.");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_REMBOURSER);
	}
	new montant = strval(inputtext);
	if(montant < 1 || montant > PlayerInfo[playerid][pLoanAmount] || montant > PlayerInfo[playerid][pAccount])
	{
		Banque_Msg(playerid, "{FF4444}Montant invalide (max : ce que vous devez ET votre solde bancaire).");
		return Banque_DemanderMontant(playerid, BANQ_ETAPE_REMBOURSER);
	}

	PlayerInfo[playerid][pAccount] -= montant;    // uniquement depuis le solde bancaire, jamais le liquide
	PlayerInfo[playerid][pLoanAmount] -= montant;

	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Bank", PlayerInfo[playerid][pAccount]);
	MySQLUpdatePlayerIntSingle(PlayerInfo[playerid][pSQLID], "Loan", PlayerInfo[playerid][pLoanAmount]);

	Banque_MettreAJourSoldes(playerid);
	Banque_MettreAJourEmprunt(playerid);

	new s[144];
	if(PlayerInfo[playerid][pLoanAmount] <= 0)
		format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Remboursement de $%d effectue. Emprunt solde !", montant);
	else
		format(s, sizeof(s), "{AADD66} Banque {FFFFFF} Remboursement de $%d effectue. Reste du : $%d.", montant, PlayerInfo[playerid][pLoanAmount]);
	Banque_Msg(playerid, s);
	return 1;
}

// ============================================================================
// CHEQUIER - pas de visuel dans le pack, commande de secours (voir en-tete)
// ============================================================================
stock Banque_CommandeChequier(playerid)
{
	if(PlayerInfo[playerid][pCheck] == 1)
		return Banque_Msg(playerid, "{AADD66} Banque {FFFFFF} Vous avez deja un chequier.");
	if(PlayerInfo[playerid][pLevel] <= 3)
		return Banque_Msg(playerid, "{CF9756} Info {FFFFFF} Vous devez etre level 3 minimum.");

	PlayerInfo[playerid][pCheck] = 1;
	PlayerInfo[playerid][pCheckNumber] = 20;
	return Banque_Msg(playerid, "{AADD66} Banque {FFFFFF} Votre chequier vous a ete delivre.");
}

// ============================================================================
// CONNEXION / DECONNEXION
// ============================================================================
stock Banque_OnConnect(playerid)
{
	// Le slot a pu servir a quelqu'un d'autre : sans remise a zero, on croirait
	// le panneau encore ouvert pour ce nouvel occupant.
	banqOuvert[playerid] = false;
	banqEtape[playerid] = 0;
	banqPending[playerid] = BANQ_ETAPE_DEPOT;
	banqToastExpire[playerid] = 0;
	return 1;
}

stock Banque_OnDisconnect(playerid)
{
	// Textdraws globaux, rien a detruire (contrairement au sac/craft) : juste
	// les flags, pour la meme raison qu'a la connexion.
	banqOuvert[playerid] = false;
	banqEtape[playerid] = 0;
	banqToastExpire[playerid] = 0;
	return 1;
}
