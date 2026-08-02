/*
	afrp_hopital_map.inc - Objets du QG de la faction Hopital (map
	communautaire fournie par le proprietaire, portee telle quelle).

	POURQUOI UN FICHIER A PART : meme convention que afrp_prison_map.inc /
	afrp_concession_map.inc - ce fichier ne contient QUE les ~270 appels
	CreateDynamicObject/SetDynamicObjectMaterial(Text) d'origine, portes
	dans une seule fonction stock appelee une fois au demarrage, plus les
	7 RemoveBuildingForPlayer d'origine (isoles dans leur propre fonction,
	voir plus bas - explique pourquoi).

	DECISION DEJA PRISE (proprietaire) : ceci est un batiment de faction
	FIXE (QG officiel Hopital), pas un local achetable via /entreprise.
	Le catalogue EntInteriors (afrp_entreprise.inc) ne reutilise que des
	interieurs GTA:SA preexistants pour des locaux joueurs - architecture
	non adaptee a une map monde-ouvert custom liee a une faction officielle.
	Ce fichier ne touche donc PAS afrp_entreprise.inc.

	CE FICHIER NE CREE PAS LE DUTY-TOGGLE A LUI SEUL : il pose le decor.
	Hopital_ToggleDuty() (LVRP.pwn) a recu une 4e zone reconnue
	(HOPITAL_MAP_ZONE_X/Y/Z/RADIUS, definis ci-dessous) en plus des 3
	batiments vanilla GTA (LS/SF/LV) deja geres - les 3 zones vanilla
	n'ont pas ete touchees.

	REBRANDING : les 2 seules enseignes 3D (SetDynamicObjectMaterialText)
	du fichier source etaient en anglais ("HOSPITAL" / "Medical Center") -
	retextees en francais + branding AFRP pour rester coherent avec le
	reste du serveur. Le commentaire d'en-tete du fichier fourni (credits
	discord/youtube d'une communaute tierce, en arabe/darija) n'est pas
	affiche en jeu : non repris ici.

	RemoveBuildingForPlayer ISOLE DANS SA PROPRE FONCTION (Hopital_MapRemoveBuildings) :
	contrairement a CreateDynamicObject, RemoveBuildingForPlayer prend un
	playerid et ne peut pas s'appeler une fois pour tout le monde depuis
	OnGameModeInit (qui n'a pas de playerid) - SA-MP l'exige a la connexion
	de CHAQUE joueur, sinon le batiment GTA d'origine reste visible pour lui
	en superposition de la map custom (meme pattern que MapPack_Retraits
	dans afrp_mappack.inc). Appelee depuis OnPlayerConnect (LVRP.pwn).

	Coordonnees d'origine : x~383-404, y~-1798 a -1835 (zone Los Santos,
	quartier hopital vanilla remplace). World/interior -1/-1 partout dans
	le fichier source (monde ouvert, deja correct, rien a changer).

	Donnees portees fidelement depuis le fichier fourni (546 lignes) ;
	aucune n'a ete reordonnee/nettoyee au-dela des 2 textes d'enseignes
	ci-dessus. Verification effectuee : toutes les lignes sont bien
	formees (parentheses equilibrees, arguments complets) - aucune
	correction n'a ete necessaire.

	A VERIFIER EN JEU (non verifiable depuis ce sandbox, pas de client
	SA-MP ici) : collision avec la geometrie GTA existante a ces
	coordonnees (cas deja rencontre sur d'autres maps communautaires
	ajoutees cette session), et que la 4e zone de duty-toggle (centre +
	rayon ci-dessous) couvre bien tout le batiment sans deborder sur la
	rue/un batiment voisin.
*/

#if defined AFRP_HOPITAL_MAP_INCLUDED
	#endinput
#endif
#define AFRP_HOPITAL_MAP_INCLUDED

// Centre approximatif (centroide des objets) + rayon de la 4e zone de
// duty-toggle Hopital_ToggleDuty() - meme convention que les 3 zones
// vanilla existantes (rayon 30.0). Voir LVRP.pwn pour l'utilisation.
#define HOPITAL_MAP_ZONE_X         394.0
#define HOPITAL_MAP_ZONE_Y         -1816.0
#define HOPITAL_MAP_ZONE_Z         7.0
#define HOPITAL_MAP_ZONE_RADIUS    30.0

// Cree les objets de la map. Appelee une seule fois depuis OnGameModeInit
// (voir gamemodes/LVRP.pwn). Monde/interieur -1/-1 (monde ouvert) comme
// dans le fichier source.
stock Hopital_CreateMap()
{
	new hospbiloox;
	hospbiloox = CreateDynamicObject(18765, 399.098449, -1802.580932, 4.340404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1812.580932, 4.341404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1816.482421, 4.340404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.080444, -1816.482421, 4.339406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-80-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 384.617004, -1808.097778, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 384.617004, -1809.097534, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 403.593444, -1812.598388, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 401.173126, -1820.932739, 8.635934, 0.000000, 89.999992, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 389.112670, -1820.968872, 8.335934, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 384.604125, -1815.096069, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 384.616027, -1817.596679, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(19464, 384.737335, -1812.060302, 8.132106, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(19441, 384.770721, -1819.327880, 8.935256, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 1419, "break_fence3", "CJ_FRAME_Glass", 0x00000000);
	hospbiloox = CreateDynamicObject(1499, 384.309539, -1815.602783, 6.834140, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10101, "2notherbuildsfe", "Bow_Abpave_Gen", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.062438, -1812.535766, 12.848403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.062438, -1812.535766, 12.839405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.071441, -1816.437255, 12.848402, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.044433, -1816.437255, 12.839405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 400.072998, -1809.068237, 13.335935, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 398.335449, -1808.202880, 8.528125, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 393.617004, -1808.097778, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 389.617004, -1808.097778, 8.240406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 387.435760, -1807.893920, 8.528125, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 386.335693, -1807.902954, 8.528125, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 401.535644, -1808.202880, 8.528125, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 395.135253, -1808.202880, 8.528125, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(1499, 390.109527, -1808.102661, 6.834139, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10101, "2notherbuildsfe", "Bow_Abpave_Gen", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(1499, 393.109710, -1808.102661, 6.834139, 0.000000, 0.000000, -180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10101, "2notherbuildsfe", "Bow_Abpave_Gen", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 391.635040, -1807.902954, 11.082135, 180.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 391.635040, -1807.902954, 11.882138, 180.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 3979, "civic01_lan", "sl_laexporail", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 399.109008, -1808.076049, 4.435936, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 389.109008, -1808.076049, 4.345938, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 403.575439, -1816.461303, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 399.000976, -1820.932128, 4.347935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 399.072998, -1820.968261, 12.635935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 402.610931, -1812.250122, 6.840404, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 401.683868, -1812.250122, 6.840404, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2434, 400.754516, -1812.251831, 6.840404, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1813.384765, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1814.311645, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1815.239135, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1816.166625, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1817.094726, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2435, 400.575805, -1818.019775, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2164, 403.061035, -1813.415405, 7.040404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(2164, 403.061035, -1816.814575, 7.040404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-90-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2388, 390.200744, -1818.575195, 6.841403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(19787, 389.958007, -1818.814331, 8.141404, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 19173, "samppictures", "samppicture3", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 14668, "711c", "gun_ceiling1128", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.941711, -1816.117553, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.041656, -1816.117553, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 389.141601, -1816.117553, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 391.841766, -1816.117553, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 388.241546, -1816.117553, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.941711, -1814.617675, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.041656, -1814.617675, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 389.141601, -1814.617675, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 391.841766, -1814.617675, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 388.241546, -1814.617675, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.941711, -1813.218017, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.041656, -1813.218017, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 389.141601, -1813.218017, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 391.841766, -1813.218017, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 388.241546, -1813.218017, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.941711, -1811.918334, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 390.041656, -1811.918334, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 389.141601, -1811.918334, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 391.841766, -1811.918334, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 388.241546, -1811.918334, 6.841403, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 385.558868, -1816.751831, 10.434620, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 385.558868, -1811.953002, 10.434620, 0.000000, 90.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 388.640533, -1819.916748, 10.453622, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 393.439422, -1819.916748, 10.453622, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 394.640899, -1819.916748, 10.453622, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 399.439788, -1819.916748, 10.453622, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 394.640899, -1819.816772, 14.953639, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 399.439788, -1819.816772, 14.953639, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 388.640533, -1819.816772, 14.953639, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 393.439422, -1819.816772, 14.953639, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 388.640533, -1816.517578, 14.253638, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 393.439422, -1816.517578, 14.253638, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 388.640533, -1813.418334, 13.553634, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 393.439422, -1813.418334, 13.553634, 0.000041, 90.000000, -89.999755, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 388.640533, -1810.219116, 12.653632, 0.000041, 90.000000, -89.999748, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 393.439422, -1810.219116, 12.653632, 0.000041, 90.000000, -89.999748, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.098449, -1812.580932, 12.749405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1812.580932, 12.740406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.107452, -1816.482421, 12.749403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.080444, -1816.482421, 12.740406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-60-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.071441, -1812.608032, 12.848403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.071441, -1812.608032, 12.839405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.080444, -1816.509521, 12.848402, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.053436, -1816.509521, 12.839405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 403.593444, -1826.398437, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 403.575439, -1830.261352, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 384.593383, -1826.398437, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 384.575378, -1830.261352, 8.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 389.112670, -1834.768920, 8.335933, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 398.412811, -1834.768920, 8.335933, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 392.772613, -1820.932739, 8.635934, 0.000000, 89.999992, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.098449, -1825.580932, 4.340404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1825.580932, 4.341403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1829.482421, 4.340404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.080444, -1829.482421, 4.339406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-80-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 398.998443, -1826.380126, 12.840405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 388.998443, -1826.380126, 12.841387, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 388.998443, -1830.281372, 12.840408, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 398.980438, -1830.281372, 12.839425, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-80-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2309, 388.241546, -1811.954467, 6.850401, 0.000000, 0.000000, 900.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(19373, 384.306793, -1816.233764, 11.096416, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.765899, -1822.938476, 8.040405, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 399.175170, -1823.047485, 8.040405, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 389.512695, -1820.986938, 10.935943, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2132, 400.206420, -1822.919067, 6.840404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 3, 18646, "matcolours", "grey-20-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 4, 18646, "matcolours", "grey-20-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 5, 18646, "matcolours", "grey-20-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 6, 18646, "matcolours", "grey-20-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 399.982452, -1825.060058, 8.440404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 402.482604, -1825.060058, 8.440403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(1499, 400.472839, -1825.184448, 6.840404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 401.482543, -1825.060058, 11.840416, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 400.782501, -1825.060058, 11.840416, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 384.611389, -1826.390014, 10.940423, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 384.693389, -1830.152954, 10.940406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 385.558868, -1830.651489, 10.434617, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19452, 385.558868, -1825.852905, 10.434622, 0.000000, 90.000000, 179.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.874908, -1830.770263, 8.349406, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.874908, -1831.770019, 8.249403, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 400.775268, -1829.370605, 8.349406, 0.000000, -89.999992, 179.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 400.475250, -1829.370605, 8.349406, 0.000000, -89.999992, 179.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 396.032348, -1829.362426, 8.840407, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(1499, 396.491546, -1829.382446, 6.840404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 395.232299, -1829.362426, 8.840407, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 396.732391, -1829.362426, 11.840417, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 397.532440, -1829.362426, 11.840417, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.765899, -1826.137695, 14.340429, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 389.112670, -1834.759277, 5.939940, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 398.412811, -1834.759277, 5.939940, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.847900, -1830.761230, 3.432410, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.847900, -1831.760986, 3.431410, 180.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 394.757873, -1822.948486, 3.413410, 0.000000, 90.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 19267, "mapmarkers", "red-2", 0x00000000);
	hospbiloox = CreateDynamicObject(2165, 399.975280, -1833.741210, 6.839406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 3, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 4, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(1369, 399.956176, -1810.601318, 7.440404, 0.000000, 0.000000, -30.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 4, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 5, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 6, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(19994, 398.776580, -1832.761718, 6.839406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2161, 402.995056, -1830.841552, 7.939404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(2161, 402.995056, -1832.241210, 7.502404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-60-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(1742, 395.235687, -1833.647827, 6.939404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 3, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(1742, 395.244689, -1832.248168, 6.948404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 3, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(1742, 395.235687, -1830.848510, 6.939404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 10765, "airportgnd_sfse", "black64", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 3, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(1800, 388.859252, -1832.666015, 6.840404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16093, "a51_ext", "ws_whitewall2_top", 0x00000000);
	hospbiloox = CreateDynamicObject(19903, 385.581451, -1827.626342, 6.840404, 0.000000, 0.000000, -15.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(1800, 388.859252, -1829.266845, 6.840404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16093, "a51_ext", "ws_whitewall2_top", 0x00000000);
	hospbiloox = CreateDynamicObject(19903, 385.581451, -1830.725585, 6.840404, 0.000000, 0.000000, -15.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(1800, 388.859252, -1824.168090, 6.840404, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16093, "a51_ext", "ws_whitewall2_top", 0x00000000);
	hospbiloox = CreateDynamicObject(19903, 385.581451, -1822.427124, 6.840404, 0.000000, 0.000000, -15.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(2146, 391.186096, -1822.477661, 7.341403, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 1239, "icons6", "lyellow32", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(2133, 385.879028, -1825.841674, 6.841403, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-20-percent", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 2, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(1800, 390.581634, -1830.725585, 6.840401, -0.000003, -0.000057, -89.999603, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 16093, "a51_ext", "ws_whitewall2_top", 0x00000000);
	hospbiloox = CreateDynamicObject(19903, 393.859039, -1832.666015, 6.840405, 0.000051, 0.000023, 165.000289, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 1, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 398.989440, -1826.280151, 12.844402, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 388.989440, -1826.280151, 12.845385, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 388.989440, -1830.181396, 12.844406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 398.971435, -1830.181396, 12.843420, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 18646, "matcolours", "grey-80-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 399.098449, -1812.580932, 4.340404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1812.580932, 4.341403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "grey-70-percent", 0x00000000);
	hospbiloox = CreateDynamicObject(18765, 389.098449, -1802.580932, 4.341403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 7509, "vgwestretail1", "hedge2_256", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 403.593444, -1802.598266, 5.240406, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 399.093170, -1798.099365, 5.231406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 386.592407, -1798.099365, 2.831408, 0.000000, 89.999992, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18766, 384.593261, -1802.571166, 5.321403, 0.000000, 0.000000, -90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1806.797363, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 390.916900, -1806.797363, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 390.916900, -1804.997802, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 390.916900, -1803.098266, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 390.916900, -1801.198730, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 390.916900, -1799.399169, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1806.797363, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1804.997802, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1803.098266, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1801.198730, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18762, 392.316986, -1799.399169, 4.367405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(2370, 396.857727, -1801.131225, 6.741404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18882, "hugebowls", "woodpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(2370, 399.557891, -1805.630126, 6.641404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18882, "hugebowls", "woodpanel1", 0x00000000);
	hospbiloox = CreateDynamicObject(18764, 385.848907, -1826.705444, 13.835935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "lightblue", 0x00000000);
	hospbiloox = CreateDynamicObject(18764, 385.848907, -1830.304565, 13.835935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 18646, "matcolours", "lightblue", 0x00000000);
	hospbiloox = CreateDynamicObject(2679, 383.372802, -1828.201049, 14.835949, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	hospbiloox = CreateDynamicObject(2679, 383.372802, -1828.700927, 14.535948, 0.000000, 90.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	SetDynamicObjectMaterial(hospbiloox, 1, 1714, "cj_office", "white32", 0x00000000);
	hospbiloox = CreateDynamicObject(19482, 383.286010, -1828.618652, 13.364957, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(hospbiloox, 0, "{fffafa} HOPITAL", 90, "Ariel", 30, 1, 0x00000000, 0x00000000, 1);
	hospbiloox = CreateDynamicObject(19482, 383.286010, -1828.618652, 12.364957, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(hospbiloox, 0, "{fffafa} Centre Medical AFRP", 90, "Ariel", 40, 1, 0x00000000, 0x00000000, 1);
	hospbiloox = CreateDynamicObject(18764, 385.866912, -1826.578979, 13.735935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	hospbiloox = CreateDynamicObject(18764, 385.866912, -1830.378051, 13.735935, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(hospbiloox, 0, 10765, "airportgnd_sfse", "black64", 0x00000000);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	hospbiloox = CreateDynamicObject(638, 384.455505, -1810.941650, 7.531715, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(638, 384.457519, -1813.541015, 7.531715, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(638, 384.462432, -1819.439575, 7.531715, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1671, 399.204528, -1814.908325, 7.240405, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1671, 399.304534, -1817.007812, 7.240405, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19893, 400.814880, -1816.800048, 7.896408, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19893, 400.814880, -1815.200439, 7.896408, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2002, 393.458801, -1819.931152, 6.841403, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1736, 402.770233, -1815.491699, 9.240408, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1671, 401.904693, -1814.908325, 7.240405, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1671, 401.904693, -1816.807861, 7.240405, 0.000000, 0.000000, 255.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19806, 396.670928, -1814.280883, 10.240412, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19806, 390.270538, -1814.280883, 10.240412, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(3812, 392.269744, -1820.260131, 9.841403, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(3812, 402.848510, -1817.174072, 9.740410, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(3812, 402.948516, -1811.475463, 9.740410, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2528, 402.063049, -1821.917602, 6.840404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2690, 399.798675, -1824.373657, 8.540406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 401.510925, -1823.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1523, 394.961730, -1828.329467, 6.831405, 360.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1523, 394.889709, -1825.312133, 6.840404, 360.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2690, 395.398406, -1824.473632, 8.540406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2690, 394.170349, -1824.482666, 8.540406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2690, 403.006591, -1810.915161, 8.239402, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2714, 395.328308, -1826.799316, 9.640410, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2685, 402.567535, -1825.597045, 8.840407, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11707, 403.070800, -1822.831420, 8.540406, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11711, 394.287353, -1826.760009, 9.541407, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11713, 395.262603, -1823.000732, 8.540406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11738, 400.729614, -1818.051757, 7.939404, 0.000000, 0.000000, 35.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11705, 400.934234, -1812.433837, 7.840404, 0.000000, 0.000000, 30.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19999, 401.125579, -1832.902709, 6.839406, 0.000000, 0.000000, 260.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(11706, 394.592285, -1820.048706, 6.839406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(3810, 402.389556, -1828.179077, 8.740406, 0.000000, 0.000000, 450.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(948, 394.401123, -1808.896240, 6.840404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(19825, 394.363647, -1829.208496, 9.741407, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1661, 390.758850, -1825.384887, 10.241411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1661, 390.758850, -1829.983764, 10.241411, 0.000000, 0.000000, 29.999998, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1778, 393.575042, -1823.529296, 6.841403, 0.000000, 0.000000, 450.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2007, 389.951416, -1833.778808, 6.740404, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2007, 389.952423, -1833.787841, 7.440402, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 387.593200, -1830.890136, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1830.890136, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1828.390747, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1825.591430, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1822.892089, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1828.390747, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1825.591430, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1822.892089, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1816.390747, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1813.591430, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1810.892089, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 391.193481, -1816.390747, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 391.193481, -1813.591430, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 391.193481, -1810.892089, 10.332410, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 401.193481, -1819.390747, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 401.193481, -1816.591430, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 401.193481, -1813.892089, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1819.390747, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1816.591430, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 392.193481, -1813.892089, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1819.390747, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1816.591430, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 386.193481, -1813.892089, 10.232411, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 401.510925, -1827.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 398.510925, -1827.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 396.510925, -1827.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 396.510925, -1824.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2707, 396.510925, -1822.162841, 10.331405, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2370, 387.592834, -1803.457031, 6.941402, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1705, 387.321868, -1801.187866, 6.841403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1705, 388.321929, -1805.086914, 6.841403, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1704, 398.992431, -1800.283447, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1704, 395.192199, -1801.183227, 6.840404, 0.000000, 0.000000, 450.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1708, 401.797058, -1804.832763, 6.840404, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1708, 397.896820, -1805.732543, 6.840404, 0.000000, 0.000000, 450.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1568, 403.496795, -1798.161254, 5.040406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1568, 395.196289, -1798.161254, 5.040406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1568, 388.495880, -1798.061279, 5.040406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(1568, 384.695648, -1798.061279, 5.040406, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2245, 387.962615, -1803.081054, 8.041401, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2244, 397.065338, -1800.790527, 7.840404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(2241, 399.835937, -1805.247070, 7.640404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(718, 401.572143, -1800.382202, 6.540404, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	hospbiloox = CreateDynamicObject(738, 386.596099, -1799.774780, 6.886403, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	#pragma unused hospbiloox
	return 1;
}

// Retire les 7 batiments GTA d'origine remplaces par cette map, pour UN
// joueur (RemoveBuildingForPlayer ne peut pas etre global, voir l'en-tete
// de ce fichier). A appeler depuis OnPlayerConnect (LVRP.pwn).
stock Hopital_MapRemoveBuildings(playerid)
{
	RemoveBuildingForPlayer(playerid, 6288, 397.539, -1848.489, 12.148, 0.250);
	RemoveBuildingForPlayer(playerid, 6456, 397.539, -1848.489, 12.148, 0.250);
	RemoveBuildingForPlayer(playerid, 1371, 387.132, -1831.449, 7.593, 0.250);
	RemoveBuildingForPlayer(playerid, 1368, 381.796, -1834.920, 7.476, 0.250);
	RemoveBuildingForPlayer(playerid, 1215, 384.429, -1837.979, 7.375, 0.250);
	RemoveBuildingForPlayer(playerid, 1215, 384.429, -1855.520, 7.375, 0.250);
	RemoveBuildingForPlayer(playerid, 1500, 387.562, -1817.089, 6.820, 0.250);
	return 1;
}
