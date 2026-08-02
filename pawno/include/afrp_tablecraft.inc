/*
	afrp_tablecraft.inc - Etabli de craft d'armes pour les gangs (/craft)

	La proprietaire a fourni un mockup textdraw "WEAPON CRAFT" (72 elements,
	style Nickk TextDraw Editor, TextDrawCreate/TextDrawTextSize purs) : trois
	armes de melee avec apercu 3D, nom, deux lignes de texte de couleur gang,
	prix et bouton CRAFT chacune. Le pack ne contient AUCUNE logique de jeu et
	AUCUN systeme de materiaux/ingredients : trois cadres d'apercu avec
	TextDrawSetPreviewModel(..., 0) trainent pres de chaque emplacement mais
	ne portent ni texte ni donnee. Ils sont repris tels quels (decoratifs,
	vides) et RIEN n'a ete invente au-dessus : "fabriquer" ici est un simple
	achat en especes habille en craft, pas un systeme d'ingredients.

	Correspondance mockup -> jeu (relevee sur les elements du pack) :
	  KATANA (haut-gauche) : modele GTA 339, 500$ -> arme /sac arme n8
	  BAT    (haut-droite) : modele GTA 336, 200$ -> arme /sac arme n5
	  GOLF   (bas-gauche)  : modele GTA 333, 350$ -> arme /sac arme n2
	(numeros verifies contre svModeleArme[] dans afrp_sacvisuel.inc, qui sont
	directement les identifiants d'arme SA-MP 1-46 : WEAPON_GOLFCLUB=2,
	WEAPON_BAT=5, WEAPON_KATANA=8.)

	=== ACCES : GANGS SEULEMENT ===
	Le texte de habillage ("Weapon Mafia Vrg", "Weapon Mafia Black", "Weapon
	Gangs use") est explicitement gang/mafia : /craft est reserve aux joueurs
	dans une faction illegale. Ce serveur stocke ca dans PlayerInfo[][pMember],
	>= 200 signifiant gang/mafia (facID = pMember-200, voir afrp_gangcreation.inc
	et afrp_gangtag.inc qui utilisent exactement ce test).

	=== PAS DE LIEU PHYSIQUE ===
	Le mockup ne donne aucune coordonnee de table/etabli, et rien de tel n'existe
	dans le monde a reutiliser : /craft s'ouvre depuis n'importe ou (comme
	/raccourcis), pas de PNJ ni d'objet a inventer.

	=== LIVRAISON DE L'ARME ===
	Comme tout le reste du gamemode, l'arme fabriquee est rangee dans
	PlayerInfo[][pInvWeapon]/[pInvAmmo] (lus par /sac arme), PAS donnee en main
	directement. Le nombre de slots depend du sac (meme formule que /sac arme
	et le depot d'arme existant : 4 slots, +1 si sac >=2, +1 si sac ==3) - aucune
	fonction reutilisable "donner une arme dans un slot libre" n'existait avant
	ce module (le code existant deposait toujours l'arme EN MAIN du joueur, pas
	une arme achetee directement) : TC_DonnerArmeSac() suit la meme formule
	pour rester coherente.

	=== BUDGET TEXTDRAWS ===
	72 elements, crees a l'ouverture et detruits a la fermeture (jamais
	residents), exclusifs du telephone et du sac comme ces deux-la le sont deja
	entre eux (afrp_sacvisuel.inc / vibe_phone.inc appellent maintenant aussi
	TC_Fermer()).

	Hooks a brancher dans LVRP.pwn :
	- OnPlayerConnect              -> TC_OnConnect(playerid)
	- OnPlayerDisconnect           -> TC_OnDisconnect(playerid)
	- OnPlayerClickPlayerTextDraw  -> if(TC_OnClick(playerid, playertextid)) return 1;
	- OnPlayerClickTextDraw        -> if(TC_OnClickHorsZone(playerid, clickedid)) return 1;
	- /craft                       -> TC_Cmd(playerid)
*/

#if defined AFRP_TABLECRAFT_INCLUDED
	#endinput
#endif
#define AFRP_TABLECRAFT_INCLUDED

#define TC_TD_COUNT      72
#define TC_ARMES         3

// Index nommes des elements utiles (les 68 autres ne sont que du decor).
#define TC_TD_CLOSE          57
#define TC_TD_CRAFT_BAT      61
#define TC_TD_CRAFT_KATANA   65
#define TC_TD_CRAFT_GOLF     69

new PlayerText:tcTD[MAX_PLAYERS][TC_TD_COUNT];
new tcOuvert[MAX_PLAYERS];

// Katana, Bat, Golf - dans cet ordre partout (index 0/1/2).
new const tcNom[TC_ARMES][24]    = { "Katana", "Batte de baseball", "Club de golf" };
new const tcPrix[TC_ARMES]       = { 500, 200, 350 };
new const tcNumArme[TC_ARMES]    = { 8, 5, 2 };   // identifiants d'arme SA-MP (= indices svModeleArme[])

// ============================================================================
// OUTILS
// ============================================================================
stock TC_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// Nombre de slots d'armes du sac selon son niveau, meme formule que /sac arme
// (voir "arme" dans OnPlayerCommandText et le dialogue 92 de LVRP.pwn).
stock TC_SlotsArme(playerid)
{
	new slots = 4;
	if(PlayerInfo[playerid][pBag] >= 2) slots++;
	if(PlayerInfo[playerid][pBag] == 3) slots++;
	return slots;
}

// Verifie la place AVANT de faire payer : on ne veut jamais debiter le joueur
// puis echouer a livrer l'arme (meme principe que SacV_APlace dans
// afrp_sacvisuel.inc).
stock TC_APlaceArme(playerid)
{
	new slots = TC_SlotsArme(playerid);
	for(new i = 0; i < slots; i++)
		if(PlayerInfo[playerid][pInvWeapon][i] == 0) return 1;
	return 0;
}

// Range l'arme dans le premier slot libre. A appeler seulement apres avoir
// verifie TC_APlaceArme(), pour rester coherent avec l'ordre annonce
// (verifs -> debit -> livraison).
stock TC_DonnerArmeSac(playerid, numArme)
{
	new slots = TC_SlotsArme(playerid);
	for(new i = 0; i < slots; i++)
	{
		if(PlayerInfo[playerid][pInvWeapon][i] == 0)
		{
			PlayerInfo[playerid][pInvWeapon][i] = numArme;
			PlayerInfo[playerid][pInvAmmo][i] = 1;   // armes blanches : munitions symboliques, comme ailleurs dans le gamemode (ex: SafeGivePlayerWeapon(playerid, 3, 1))
			return 1;
		}
	}
	return 0;
}

// ============================================================================
// CREATION DES TEXTDRAWS - port fidele du mockup (72 elements, coordonnees/
// couleurs/polices inchangees). Trois familles seulement dans ce pack :
//   - sprites (police 4, LD_SPAC:white / LD_BEAT:chit, TextTextSize = largeur/
//     hauteur du sprite, PAS des coordonnees de fin - voir afrp_diagnostic.inc
//     et le commit 9569bcf sur vibe_phone.inc pour ce meme piege)
//   - texte (police 1 ou 2, LetterSize)
//   - apercus 3D d'arme (police 5, SetPreviewModel/Rot/VehCol)
// ============================================================================
stock TC_Sprite(playerid, Float:x, Float:y, Float:w, Float:h, color, sprite[], bool:selectable = false)
{
	new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, sprite);
	PlayerTextDrawTextSize(playerid, t, w, h);
	PlayerTextDrawAlignment(playerid, t, 1);
	PlayerTextDrawColor(playerid, t, color);
	PlayerTextDrawSetShadow(playerid, t, 0);
	PlayerTextDrawSetOutline(playerid, t, 0);
	PlayerTextDrawBackgroundColor(playerid, t, 255);
	PlayerTextDrawFont(playerid, t, 4);
	PlayerTextDrawSetProportional(playerid, t, 1);
	if(selectable) PlayerTextDrawSetSelectable(playerid, t, 1);
	return t;
}

stock TC_Label(playerid, Float:x, Float:y, Float:lx, Float:ly, color, font, txt[])
{
	new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, txt);
	PlayerTextDrawLetterSize(playerid, t, lx, ly);
	PlayerTextDrawAlignment(playerid, t, 1);
	PlayerTextDrawColor(playerid, t, color);
	PlayerTextDrawSetShadow(playerid, t, 1);
	PlayerTextDrawSetOutline(playerid, t, 1);
	PlayerTextDrawBackgroundColor(playerid, t, 1);
	PlayerTextDrawFont(playerid, t, font);
	PlayerTextDrawSetProportional(playerid, t, 1);
	return t;
}

stock TC_Preview(playerid, Float:x, Float:y, Float:w, Float:h, model, Float:rx, Float:ry, Float:rz, Float:zoom, bg)
{
	new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, "_");
	PlayerTextDrawTextSize(playerid, t, w, h);
	PlayerTextDrawAlignment(playerid, t, 1);
	PlayerTextDrawColor(playerid, t, -1);
	PlayerTextDrawSetShadow(playerid, t, 0);
	PlayerTextDrawSetOutline(playerid, t, 0);
	PlayerTextDrawBackgroundColor(playerid, t, bg);
	PlayerTextDrawFont(playerid, t, 5);
	PlayerTextDrawSetProportional(playerid, t, 0);
	PlayerTextDrawSetPreviewModel(playerid, t, model);
	PlayerTextDrawSetPreviewRot(playerid, t, rx, ry, rz, zoom);
	PlayerTextDrawSetPreviewVehCol(playerid, t, 0, 0);
	return t;
}

stock TC_Creer(playerid)
{
	// --- entete / loupe (decor, repris du pack) ---
	tcTD[playerid][0]  = TC_Sprite(playerid, 22.0, 92.0, 2.0, 320.0, -251662081, "LD_SPAC:white");
	tcTD[playerid][1]  = TC_Sprite(playerid, 22.0, 92.0, 2.0, 173.0, -16776961, "LD_SPAC:white");
	tcTD[playerid][2]  = TC_Sprite(playerid, 19.0, 73.0, 88.0, 16.0, -16776961, "LD_SPAC:white");
	tcTD[playerid][3]  = TC_Label(playerid, 36.0, 80.0, -0.419, 0.799, -1, 1, "/");
	tcTD[playerid][4]  = TC_Sprite(playerid, 20.0, 74.0, 86.0, 14.0, 255, "LD_SPAC:white");
	tcTD[playerid][5]  = TC_Label(playerid, 36.0, 74.0, 0.140, 1.299, -1, 2, "SEARCH");
	tcTD[playerid][6]  = TC_Sprite(playerid, 21.0, 73.0, 13.0, 15.0, -1, "LD_BEAT:chit");
	tcTD[playerid][7]  = TC_Sprite(playerid, 22.0, 74.0, 11.0, 13.0, 255, "LD_BEAT:chit");
	tcTD[playerid][8]  = TC_Sprite(playerid, 120.0, 76.0, 9.0, 11.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][9]  = TC_Label(playerid, 130.0, 74.0, 0.300, 1.500, -1, 1, "WEAPON CRAFT");

	// --- cadre GOLF (bas-gauche) ---
	tcTD[playerid][10] = TC_Sprite(playerid, 40.0, 242.0, 75.0, 96.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][11] = TC_Sprite(playerid, 102.0, 323.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][12] = TC_Sprite(playerid, 102.0, 231.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][13] = TC_Sprite(playerid, 32.0, 231.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][14] = TC_Sprite(playerid, 32.0, 323.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][15] = TC_Sprite(playerid, 43.0, 336.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][16] = TC_Sprite(playerid, 43.0, 235.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][17] = TC_Sprite(playerid, 113.0, 246.0, 7.0, 90.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][18] = TC_Sprite(playerid, 36.0, 244.0, 7.0, 90.0, 960051711, "LD_SPAC:white");

	// --- cadre KATANA (haut-gauche) ---
	tcTD[playerid][19] = TC_Sprite(playerid, 40.0, 109.0, 75.0, 96.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][20] = TC_Sprite(playerid, 102.0, 190.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][21] = TC_Sprite(playerid, 102.0, 98.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][22] = TC_Sprite(playerid, 32.0, 98.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][23] = TC_Sprite(playerid, 32.0, 190.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][24] = TC_Sprite(playerid, 43.0, 203.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][25] = TC_Sprite(playerid, 43.0, 102.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][26] = TC_Sprite(playerid, 113.0, 113.0, 7.0, 90.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][27] = TC_Sprite(playerid, 36.0, 111.0, 7.0, 90.0, 960051711, "LD_SPAC:white");

	// --- cadre BAT (haut-droite) ---
	tcTD[playerid][28] = TC_Sprite(playerid, 138.0, 109.0, 75.0, 96.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][29] = TC_Sprite(playerid, 200.0, 190.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][30] = TC_Sprite(playerid, 200.0, 98.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][31] = TC_Sprite(playerid, 130.0, 98.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][32] = TC_Sprite(playerid, 130.0, 190.0, 21.0, 26.0, 960051711, "LD_BEAT:chit");
	tcTD[playerid][33] = TC_Sprite(playerid, 141.0, 203.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][34] = TC_Sprite(playerid, 141.0, 102.0, 70.0, 9.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][35] = TC_Sprite(playerid, 211.0, 113.0, 7.0, 90.0, 960051711, "LD_SPAC:white");
	tcTD[playerid][36] = TC_Sprite(playerid, 134.0, 111.0, 7.0, 90.0, 960051711, "LD_SPAC:white");

	// --- KATANA : apercu 3D, nom, texte de gang, prix ---
	tcTD[playerid][37] = TC_Preview(playerid, 46.0, 96.0, 89.0, 91.0, 339, 380.0, 53.0, -12.0, 2.599, 1);
	tcTD[playerid][38] = TC_Label(playerid, 40.0, 161.0, 0.150, 1.099, -1, 1, "KATANA");
	tcTD[playerid][39] = TC_Label(playerid, 40.0, 170.0, 0.150, 0.999, -2139062017, 1, "Weapon Mafia Vrg");
	tcTD[playerid][40] = TC_Label(playerid, 40.0, 179.0, 0.150, 0.999, -2139062017, 1, "RANCE");
	tcTD[playerid][41] = TC_Label(playerid, 43.0, 197.0, 0.150, 0.999, -1, 1, "500$");

	// --- GOLF : prix (nom/texte/apercu plus bas avec le reste du cadre) ---
	tcTD[playerid][42] = TC_Label(playerid, 43.0, 331.0, 0.150, 0.999, -1, 1, "350$");

	// --- BAT : prix ---
	tcTD[playerid][43] = TC_Label(playerid, 144.0, 197.0, 0.150, 0.999, -1, 1, "200$");

	// --- 3 cadres d'apercu vides (modele 0 dans le pack source) : aucune
	// donnee de materiaux/ingredients associee, purement decoratifs - voir
	// note en tete de fichier.
	tcTD[playerid][44] = TC_Preview(playerid, 43.0, 109.0, 70.0, 49.0, 0, 0.0, 0.0, 0.0, 142.299, 85);
	tcTD[playerid][45] = TC_Preview(playerid, 141.0, 109.0, 70.0, 49.0, 0, 0.0, 0.0, 0.0, 142.299, 85);
	tcTD[playerid][46] = TC_Preview(playerid, 43.0, 241.0, 70.0, 49.0, 0, 0.0, 0.0, 0.0, 142.299, 85);

	// --- BAT : nom, texte de gang ---
	tcTD[playerid][47] = TC_Label(playerid, 140.0, 161.0, 0.150, 1.099, -1, 1, "BAT");
	tcTD[playerid][48] = TC_Label(playerid, 140.0, 170.0, 0.150, 1.099, 1768516095, 1, "Weapon Mafia Black");
	tcTD[playerid][49] = TC_Label(playerid, 140.0, 179.0, 0.150, 1.099, 1768516095, 1, "Shot!!!");

	// --- GOLF : nom, texte de gang ---
	tcTD[playerid][50] = TC_Label(playerid, 41.0, 294.0, 0.150, 1.099, -1, 1, "GOLF");
	tcTD[playerid][51] = TC_Label(playerid, 41.0, 303.0, 0.150, 1.099, 1768516095, 1, "Weapon Gangs use");
	tcTD[playerid][52] = TC_Label(playerid, 41.0, 312.0, 0.150, 1.099, 1768516095, 1, "SamxGM");

	// --- apercus 3D BAT et GOLF ---
	tcTD[playerid][53] = TC_Preview(playerid, 150.0, 110.0, 72.0, 61.0, 336, 0.0, 39.0, 0.0, 1.700, 1);
	tcTD[playerid][54] = TC_Preview(playerid, 46.0, 245.0, 99.0, 72.0, 333, 0.0, 38.0, 0.0, 2.299, 1);

	// --- bouton CLOSE ---
	tcTD[playerid][55] = TC_Sprite(playerid, 219.0, 358.0, 27.0, 30.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][56] = TC_Sprite(playerid, 159.0, 358.0, 27.0, 30.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][TC_TD_CLOSE] = TC_Sprite(playerid, 173.0, 363.0, 60.0, 20.0, -16776961, "LD_SPAC:white", true);
	tcTD[playerid][58] = TC_Label(playerid, 193.0, 366.0, 0.190, 1.399, -1, 1, "CLOSE");

	// --- bouton CRAFT (BAT) ---
	tcTD[playerid][59] = TC_Sprite(playerid, 195.0, 192.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][60] = TC_Sprite(playerid, 161.0, 192.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][TC_TD_CRAFT_BAT] = TC_Sprite(playerid, 171.0, 195.0, 35.0, 13.0, -16776961, "LD_SPAC:white", true);
	tcTD[playerid][62] = TC_Label(playerid, 179.0, 195.0, 0.170, 1.199, -1, 1, "CRAFT");

	// --- bouton CRAFT (KATANA) ---
	tcTD[playerid][63] = TC_Sprite(playerid, 97.0, 192.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][64] = TC_Sprite(playerid, 63.0, 192.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][TC_TD_CRAFT_KATANA] = TC_Sprite(playerid, 73.0, 195.0, 35.0, 13.0, -16776961, "LD_SPAC:white", true);
	tcTD[playerid][66] = TC_Label(playerid, 81.0, 195.0, 0.170, 1.199, -1, 1, "CRAFT");

	// --- bouton CRAFT (GOLF) ---
	tcTD[playerid][67] = TC_Sprite(playerid, 98.0, 325.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][68] = TC_Sprite(playerid, 64.0, 325.0, 21.0, 19.0, -16776961, "LD_BEAT:chit");
	tcTD[playerid][TC_TD_CRAFT_GOLF] = TC_Sprite(playerid, 74.0, 328.0, 35.0, 13.0, -16776961, "LD_SPAC:white", true);
	tcTD[playerid][70] = TC_Label(playerid, 82.0, 328.0, 0.170, 1.199, -1, 1, "CRAFT");

	// --- fond du panneau ---
	// [FIX] La source etirait "LD_BEAT:chit" (une petite icone decorative,
	// jamais prevue pour de tels agrandissements ailleurs dans ce gamemode)
	// a 1409x1176, bien plus grand que l'ecran : au lieu d'un fond discret,
	// ca affichait un gros aplat qui debordait sur les boutons. On reprend
	// "LD_SPAC:white" (la texture plate deja utilisee pour tous les autres
	// grands fonds du gamemode : svPanneau, phone_TD[4], etc.) et on borne
	// la taille au panneau lui-meme au lieu de l'ecran entier.
	tcTD[playerid][71] = TC_Sprite(playerid, 10.0, 65.0, 250.0, 335.0, 103, "LD_SPAC:white");

	return 1;
}

stock TC_Detruire(playerid)
{
	for(new i = 0; i < TC_TD_COUNT; i++) PlayerTextDrawDestroy(playerid, tcTD[playerid][i]);
	return 1;
}

// ============================================================================
// ACHAT (le mockup ne code aucun systeme de materiaux : ceci reste un achat
// en especes habille en "craft", rien de plus)
// ============================================================================
stock TC_Fabriquer(playerid, idx)
{
	if(idx < 0 || idx >= TC_ARMES) return 0;

	// Re-verif : le joueur a pu quitter son gang pendant que le panneau etait
	// ouvert.
	if(PlayerInfo[playerid][pMember] < 200)
	{
		TC_Msg(playerid, "{FF4444}Reserve aux membres d'un gang.");
		return TC_Fermer(playerid);
	}

	new prix = tcPrix[idx];
	if(PlayerInfo[playerid][pCash] < prix)
	{
		new s[128], ps[24];
		FormatMoney(prix, ps, sizeof(ps));
		format(s, sizeof(s), "{FF4444}Il te faut $%s pour fabriquer : %s.", ps, tcNom[idx]);
		return TC_Msg(playerid, s);
	}

	if(!TC_APlaceArme(playerid))
		return TC_Msg(playerid, "{FF4444}Ton sac n'a plus de place pour une arme (/sac arme pour verifier).");

	SafeGivePlayerMoney(playerid, -prix, "Craft d'arme (gang)");
	TC_DonnerArmeSac(playerid, tcNumArme[idx]);

	new msg[144];
	format(msg, sizeof(msg), "{00FF00}Fabrique : %s. Rangee dans ton sac (/sac arme pour la sortir).", tcNom[idx]);
	TC_Msg(playerid, msg);

	TC_Fermer(playerid);
	return 1;
}

// ============================================================================
// OUVRIR / FERMER
// ============================================================================
stock TC_Cmd(playerid)
{
	if(PlayerInfo[playerid][pMember] < 200)
		return TC_Msg(playerid, "{FF4444}Reserve aux membres d'un gang.");
	if(IsPlayerInAnyVehicle(playerid))
		return TC_Msg(playerid, "{FF4444}Impossible dans un vehicule.");

	// [BUDGET TEXTDRAWS] exclusif du telephone et du sac, meme convention que
	// phone_Open()/SacV_Ouvrir() entre eux.
	if(phone_State[playerid] != PHONE_STATE_CLOSED) phone_Close(playerid);
	if(usingPhone[playerid] == 1)
		return TC_Msg(playerid, "{FF8888}Ferme d'abord ton telephone.");
	if(svOuvert[playerid] == 1) SacV_Fermer(playerid);

	if(tcOuvert[playerid] == 1) TC_Fermer(playerid);

	TC_Creer(playerid);
	tcOuvert[playerid] = 1;
	// [FIX] Le fond (indice 71) doit s'afficher EN PREMIER : sur SA-MP, un
	// textdraw affiche plus tard se dessine par-dessus les precedents. En le
	// montrant en dernier (boucle 0..71 dans l'ordre), le fond recouvrait
	// tous les boutons et le texte au lieu de rester derriere.
	PlayerTextDrawShow(playerid, tcTD[playerid][71]);
	for(new i = 0; i < TC_TD_COUNT - 1; i++) PlayerTextDrawShow(playerid, tcTD[playerid][i]);
	SelectTextDraw(playerid, 0x00C8FFAA);

	TC_Msg(playerid, "{AAAAAA}Clique CRAFT sous une arme pour l'acheter, ou CLOSE pour fermer.");
	return 1;
}

stock TC_Fermer(playerid)
{
	if(tcOuvert[playerid] != 1) return 0;
	tcOuvert[playerid] = 0;
	TC_Detruire(playerid);
	CancelSelectTextDraw(playerid);
	return 1;
}

// ============================================================================
// CLICS
// ============================================================================
stock TC_OnClick(playerid, PlayerText:playertextid)
{
	if(tcOuvert[playerid] != 1) return 0;

	if(playertextid == tcTD[playerid][TC_TD_CLOSE])        { TC_Fermer(playerid); return 1; }
	if(playertextid == tcTD[playerid][TC_TD_CRAFT_KATANA]) { TC_Fabriquer(playerid, 0); return 1; }
	if(playertextid == tcTD[playerid][TC_TD_CRAFT_BAT])    { TC_Fabriquer(playerid, 1); return 1; }
	if(playertextid == tcTD[playerid][TC_TD_CRAFT_GOLF])   { TC_Fabriquer(playerid, 2); return 1; }
	return 0;
}

// Echap pendant la selection : SA-MP previent via OnPlayerClickTextDraw avec
// un identifiant invalide (meme mecanisme que SacV_OnClickHorsZone).
stock TC_OnClickHorsZone(playerid, Text:clickedid)
{
	if(tcOuvert[playerid] != 1) return 0;
	if(clickedid != Text:INVALID_TEXT_DRAW) return 0;
	TC_Fermer(playerid);
	return 1;
}

// ============================================================================
// INIT
// ============================================================================
stock TC_OnConnect(playerid)
{
	tcOuvert[playerid] = 0;
	return 1;
}

stock TC_OnDisconnect(playerid)
{
	if(tcOuvert[playerid] == 1) TC_Detruire(playerid);
	tcOuvert[playerid] = 0;
	return 1;
}
