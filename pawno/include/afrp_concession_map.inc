/*
	afrp_concession_map.inc - Objets de la 4e concession (map communautaire
	de garage mecanicien, fournie par le proprietaire, reconvertie en
	concession auto d'occasion).

	POURQUOI UN FICHIER A PART : meme convention que afrp_prison_map.inc -
	ce fichier ne contient QUE les ~230 appels CreateDynamicObject/
	SetDynamicObjectMaterial(Text) d'origine, portes tels quels dans une
	seule fonction stock appelee une fois au demarrage. Separer les donnees
	brutes de la logique (systeme de concession deja existant dans
	LVRP.pwn : dealerShip_Load/Save, dealerShip_ShowDialog, GetDealerShipName)
	garde tout lisible.

	CE FICHIER NE CREE PAS DE CONCESSION FONCTIONNELLE A LUI SEUL : il pose
	juste le decor. La concession en elle-meme (pickup 'N', catalogue de
	vehicules) vient de la ligne MySQL a inserer dans lvrp_server_dealership
	(Type=7, City=2) + lvrp_server_vehicles_prices (dealerShip=7) - voir le
	rapport de session pour les INSERT exacts a executer sur la base live.
	GetDealerShipName() (LVRP.pwn) a recu un cas 7 = "Occasion" pour que ce
	nouveau point s'affiche comme une concession a part entiere dans le
	libelle du pickup, distincte des concessions Sport/Auto existantes.

	REBRANDING : la map source ("Mechanic_by_splux") etait un garage
	mecanicien anglophone (enseignes "SERVICE"/"TUNE UP"/"SPOORING"/
	"BALANCING"/"JOKS"/"MOTOR") credite a un auteur tiers dans les
	commentaires d'en-tete (non affiches en jeu, non repris ici). Comme il
	s'agit maintenant d'une concession et non d'un garage de reparation, et
	que le serveur est francophone, les 6 enseignes 3D (SetDynamicObjectMaterialText)
	et le panneau "OFFICE" ont ete retextes en francais/marque AFRP - la
	structure (positions, tailles, modeles) n'a pas bouge.

	Coordonnees d'origine : Las Venturas, pres de l'aeroport (x~1013-1038,
	y~-1946 a -1970). World/interior -1/-1 partout dans le fichier source
	(donc deja "monde ouvert", rien a changer de ce cote).

	Donnees portees fidelement depuis le fichier fourni (245 lignes) ;
	aucune n'a ete reordonnee/nettoyee au-dela des textes d'enseignes
	ci-dessus. Verification effectuee : toutes les lignes sont bien
	formees (parentheses equilibrees, arguments complets) - aucune
	correction n'a ete necessaire.

	A VERIFIER EN JEU (non verifiable depuis ce sandbox, pas de client
	SA-MP ici) : collision avec la geometrie GTA existante a ces
	coordonnees (cas deja rencontre sur d'autres maps communautaires
	ajoutees cette session), et que le point de pickup choisi pour la
	concession (voir INSERT dans le rapport) tombe bien au sol/hors mur.
*/

#if defined AFRP_CONCESSION_MAP_INCLUDED
	#endinput
#endif
#define AFRP_CONCESSION_MAP_INCLUDED

// Cree les objets de la map. Appelee une seule fois depuis OnGameModeInit
// (voir gamemodes/LVRP.pwn). Monde/interieur -1/-1 (monde ouvert, visible
// par tous) comme dans le fichier source -> pas de constantes a exposer
// ici (contrairement a afrp_prison_map.inc qui vit dans un interieur dedie).
stock Concession_CreateMap()
{
	new tmpobjid;
	// Voir afrp_mapmove.inc : coordonnees d'origine intactes, la transformation
	// eventuelle de /a deplacer est appliquee par MM_Obj.
	MM_Begin(MM_CONCESSION);
	tmpobjid = MM_Obj(18981, 1026.063842, -1958.130249, 11.629739, 0.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18980, 1023.401794, -1958.110595, 16.231557, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 5392, "eastshops1_lae", "blueshop2_LAe", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1965.622070, 11.632725, 90.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1955.622436, 11.632725, 90.000000, 90.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18980, 1026.072265, -1970.150756, 11.911555, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18980, 1026.072265, -1946.070678, 11.911555, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1965.622070, 11.632725, 89.999992, 180.000000, -89.999992, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1955.622436, 11.632725, 89.999992, 180.000000, -89.999992, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18981, 1013.093933, -1958.130249, 4.219738, 0.000000, 180.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1965.622070, 16.202737, 89.999992, 180.000000, -89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1955.622436, 16.202737, 89.999992, 180.000000, -89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1965.622070, 16.202737, 89.999992, 180.000015, -89.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1955.622436, 16.202737, 89.999992, 180.000015, -89.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1951.479614, 11.632725, 89.999992, 180.000000, -89.999992, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1951.479614, 11.632725, 89.999992, 180.000000, -89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 1714, "cj_office", "BLUE_FABRIC", 0x00000000);
	tmpobjid = MM_Obj(18766, 1016.073181, -1951.479614, 16.202737, 89.999992, 180.000015, -89.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1021.072753, -1951.479614, 16.202737, 89.999992, 180.000030, -89.999961, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 16640, "a51", "ws_metalpanel1", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.913757, -1970.173583, 14.262741, 180.000000, 360.000000, 0.000022, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.473388, -1970.183593, 14.242740, 180.000000, 360.000000, 0.000022, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.913757, -1970.173583, 9.272742, 0.000000, 540.000000, -179.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.473388, -1970.183593, 9.252741, 0.000000, 540.000000, -179.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.913757, -1946.022705, 14.262741, 0.000000, 540.000000, -179.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.473388, -1946.032714, 14.242740, 0.000000, 540.000000, -179.999969, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.913757, -1946.022705, 9.272742, -0.000000, 540.000000, -179.999923, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(18766, 1018.473388, -1946.032714, 9.252741, -0.000000, 540.000000, -179.999923, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(19463, 1023.483764, -1948.239624, 13.819200, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(19463, 1023.483764, -1951.740112, 13.819200, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(19463, 1023.363647, -1948.249633, 13.819200, 89.999992, 89.999992, -89.999992, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(19463, 1023.363647, -1951.750122, 13.819200, 89.999992, 89.999992, -89.999992, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(19388, 1015.187438, -1953.393432, 13.829356, 0.000007, 0.000000, 89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(1499, 1014.429016, -1953.445312, 12.088800, 0.000000, 0.000007, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 2755, "ab_dojowall", "ab_trellis", 0x00000000);
	tmpobjid = MM_Obj(19434, 1015.350769, -1953.422973, 15.595175, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(19434, 1018.850646, -1953.422973, 15.595175, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(19434, 1021.710998, -1953.422973, 15.575175, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(2008, 1018.289123, -1952.981689, 12.105179, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 4, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = MM_Obj(2008, 1022.779479, -1952.981689, 12.105179, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 4, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = MM_Obj(2008, 1020.519226, -1952.981689, 12.105179, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 4, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = MM_Obj(2163, 1023.297424, -1947.186889, 12.799434, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = MM_Obj(2163, 1023.297424, -1950.276489, 12.799434, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14415, "carter_block_2", "mp_shop_floor2", 0x00000000);
	tmpobjid = MM_Obj(3077, 1019.850280, -1947.233764, 12.097772, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 1, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	// [REBRAND] "OFFICE" -> "BUREAU"
	tmpobjid = MM_Obj(19482, 1019.919982, -1953.547729, 15.115763, 0.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000} BUREAU", 140, "Ariel", 35, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(18762, 1016.088562, -1969.160400, 11.645633, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1021.088317, -1969.160400, 11.645633, 90.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1016.088562, -1957.209716, 11.645633, 89.999992, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1021.088317, -1957.209716, 11.645633, 89.999992, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1016.088562, -1963.189575, 11.645633, 89.999992, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1021.088317, -1963.189575, 11.645633, 89.999992, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1023.119873, -1966.189941, 11.645633, 89.999992, 224.999984, -44.999996, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1023.119873, -1960.209838, 11.645633, 89.999992, 224.999984, -44.999996, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 10765, "airportgnd_sfse", "white", 0x00000000);
	tmpobjid = MM_Obj(18762, 1014.118347, -1969.160400, 13.385636, 180.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(18762, 1023.238281, -1969.160400, 13.385636, 180.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(18762, 1014.118347, -1963.180786, 13.385636, 0.000000, 179.999984, -90.000007, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(18762, 1023.238281, -1963.180786, 13.385636, 0.000000, 179.999984, -90.000007, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(18762, 1014.118347, -1957.180175, 13.385636, -0.000007, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(18762, 1023.238281, -1957.180175, 13.385636, -0.000007, 179.999984, -89.999984, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 2567, "ab", "ab_metalholes", 0x00000000);
	tmpobjid = MM_Obj(19482, 1019.060668, -1966.374511, 12.134600, 540.000000, 90.000000, 540.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} 1", 140, "Ariel", 170, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1019.060668, -1960.393798, 12.134600, 540.000000, 90.000000, 540.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} 2", 140, "Ariel", 170, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(1483, 1028.496582, -1947.477050, 14.049173, -0.000007, -0.000000, -89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14534, "ab_wooziea", "mcstraps_window", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(1483, 1035.185791, -1947.477050, 14.049173, -0.000007, -0.000000, -89.999977, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 14534, "ab_wooziea", "mcstraps_window", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 18646, "matcolours", "grey-90-percent", 0x00000000);
	tmpobjid = MM_Obj(19448, 1029.998779, -1946.655517, 10.382500, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	tmpobjid = MM_Obj(19448, 1033.709472, -1946.665527, 10.392500, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	tmpobjid = MM_Obj(19435, 1025.259765, -1948.473022, 11.329799, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	tmpobjid = MM_Obj(19435, 1038.420410, -1948.473022, 11.329799, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	tmpobjid = MM_Obj(19435, 1031.850097, -1948.473022, 11.329799, 90.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	tmpobjid = MM_Obj(640, 1035.865356, -1970.109130, 12.979750, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 4830, "airport2", "bevflower2", 0x00000000);
	tmpobjid = MM_Obj(640, 1030.485717, -1970.109130, 12.979750, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 4830, "airport2", "bevflower2", 0x00000000);
	tmpobjid = MM_Obj(640, 1025.115600, -1970.109130, 12.979750, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 18646, "matcolours", "grey-90-percent", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 4830, "airport2", "bevflower2", 0x00000000);
	tmpobjid = MM_Obj(640, 1023.924926, -1949.287841, 12.719749, 0.000000, 0.000000, 360.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 4830, "airport2", "bevflower2", 0x00000000);
	tmpobjid = MM_Obj(640, 1023.934936, -1950.798583, 12.749750, 0.000000, 0.000000, 360.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterial(tmpobjid, 0, 9514, "711_sfw", "ws_carpark2", 0x00000000);
	SetDynamicObjectMaterial(tmpobjid, 1, 4830, "airport2", "bevflower2", 0x00000000);
	// [REBRAND] enseignes de garage ("SERVICE"/"TUNE UP"/"SPOORING"/"BALANCING")
	// -> vocabulaire concession auto en francais, branding AFRP.
	tmpobjid = MM_Obj(19482, 1023.911193, -1968.644653, 15.937297, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} AFRP", 140, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1023.911193, -1966.214233, 15.937297, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} CONCESSION", 140, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1023.911193, -1963.653320, 15.937297, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} OCCASION", 140, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1023.911193, -1960.593261, 15.937297, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{FFFAFA} GARANTIE", 140, "Ariel", 50, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1023.611328, -1952.423950, 17.057296, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000} VENTE", 140, "Ariel", 80, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(19482, 1023.611328, -1949.803710, 17.057296, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	SetDynamicObjectMaterialText(tmpobjid, 0, "{000000} AUTO", 140, "Ariel", 80, 1, 0x00000000, 0x00000000, 1);
	tmpobjid = MM_Obj(1723, 1014.480285, -1947.096923, 12.083986, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(2002, 1017.505737, -1947.098388, 12.095965, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(2254, 1013.601562, -1951.930664, 14.340389, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(2254, 1013.601562, -1947.860595, 14.340389, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1722, 1022.205749, -1954.787353, 12.107660, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1722, 1019.955627, -1954.787353, 12.107660, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1722, 1017.695495, -1954.787353, 12.107660, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1806, 1022.039978, -1951.839111, 12.092888, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1806, 1019.749938, -1951.839111, 12.092888, 0.000000, 0.000000, -168.599990, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1806, 1017.449279, -1951.813598, 12.092888, 0.000000, 0.000000, 164.700012, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19899, 1014.070800, -1966.112426, 12.058495, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19899, 1018.760925, -1969.163085, 13.108504, 180.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19899, 1018.760925, -1957.212280, 13.108504, 180.000000, 0.000000, 270.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19903, 1018.464050, -1969.084228, 13.090775, 0.000000, 0.000000, 25.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19903, 1018.464050, -1957.133911, 13.090775, 0.000000, 0.000000, -25.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19903, 1018.464050, -1963.113891, 12.130770, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19815, 1013.593566, -1960.899536, 13.056534, 0.000000, 0.000000, 90.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1081, 1013.703735, -1961.858276, 14.741557, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1083, 1013.680847, -1958.625000, 14.779560, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1098, 1013.703674, -1960.177856, 14.771246, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1082, 1013.711853, -1958.629882, 13.257081, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1010, 1014.054138, -1965.046997, 13.302688, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1010, 1014.054138, -1966.267089, 13.302688, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1010, 1014.054138, -1965.646728, 13.302688, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1010, 1014.054138, -1965.957031, 13.302688, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1010, 1014.054138, -1965.336425, 13.302688, 0.000000, 0.000000, 0.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1968.161499, 15.812053, -44.799987, 179.999969, 179.999938, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1968.161499, 15.812053, -44.799987, 179.999969, 179.999938, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1968.161499, 15.812053, -44.799987, 179.999969, 179.999938, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1966.161865, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1966.161865, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1966.161865, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1964.120727, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1964.120727, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1964.120727, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1968.161499, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1968.161499, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1968.161499, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1966.161865, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1966.161865, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1966.161865, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1964.120727, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1964.120727, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1964.120727, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1962.171020, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1962.171020, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1962.171020, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1960.171386, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1960.171386, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1960.171386, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1958.130249, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1958.130249, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1958.130249, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1962.171020, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1962.171020, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1962.171020, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1960.171386, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1960.171386, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1960.171386, 15.812053, -44.799987, 179.999923, 179.999801, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1958.130249, 15.812053, -44.799987, 179.999908, 179.999755, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1958.130249, 15.812053, -44.799987, 179.999908, 179.999755, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1958.130249, 15.812053, -44.799987, 179.999908, 179.999755, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1021.272827, -1950.009887, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1020.592529, -1950.009887, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1019.912292, -1950.009887, 15.812053, -44.799987, 179.999954, 179.999893, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.822143, -1950.009887, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1016.141845, -1950.009887, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(19279, 1015.461608, -1950.009887, 15.812053, -44.799987, 179.999938, 179.999847, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1290, 1037.281372, -1970.088378, 13.514026, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	tmpobjid = MM_Obj(1290, 1027.890747, -1970.088378, 13.514026, 0.000000, 0.000000, 180.000000, -1, -1, -1, 300.00, 300.00);
	MM_End();
	#pragma unused tmpobjid
	return 1;
}

// ============================================================================
// VEHICULES D'EXPOSITION
// ============================================================================
/*
	Le proprietaire voulait des voitures garees dans la concession, purement
	pour l'ambiance : pas conduisibles, pas achetables, jamais melangees aux
	vehicules des joueurs.

	POURQUOI DE VRAIS VEHICULES ET PAS DES OBJETS : on peut donner un modele de
	vehicule a CreateDynamicObject, c'est meme la ruse habituelle pour ce genre
	de vitrine. Sauf que les roues d'un vehicule GTA ne font pas partie de son
	modele (elles sont posees a l'execution sur des points d'ancrage par le
	moteur de vehicules) : en objet, on obtient des carrosseries posees sur
	rien. Une vitrine de concession avec des voitures sans roues est pire que
	pas de vitrine du tout. On cree donc de vrais vehicules, et on les neutralise.

	COMMENT ILS SONT NEUTRALISES, ET POURQUOI CA SUFFIT :
	- portieres verrouillees (SetVehicleParamsEx doors = 1) : SA-MP interdit
	  alors l'entree a TOUT LE MONDE, il n'y a pas d'exception proprietaire.
	  C'est ce qui garantit "jamais conduisible", pas un test dans un callback
	  qu'on pourrait oublier de brancher ailleurs ;
	- delai de reapparition -1 : ils ne repassent jamais par OnVehicleSpawn,
	  donc jamais de retour a une position "officielle" qui n'existe pas ;
	- leur ligne dans le tableau vehicle[] est explicitement marquee vide
	  (used = 0, SQLID = -1). Toutes les boucles du gamemode qui parcourent
	  1..totalVehicles testent l'un ou l'autre avant d'agir : sauvegarde auto,
	  reparation d'essence, payday... aucune ne les verra. Sans ce marquage
	  elles risqueraient d'ecrire des lignes fantomes dans lvrp_server_vehicles.

	POURQUOI LES CREER PENDANT OnGameModeInit : vehicle_Load() arrive plus tard
	(reponse MySQL asynchrone). Ces vehicules prennent donc les tout premiers
	IDs SA-MP, et vehicle_Load sait deja gerer le cas ou SA-MP lui rend un ID
	different de son compteur (vehicle_Transfer + recalage de totalVehicles).
	Les creer apres coup, sans savoir quand la reponse MySQL tombe, serait au
	contraire une course entre les deux.

	POSITIONS : posees sur la dalle exterieure de la concession, la seule zone
	franchement vide des donnees de la map (x 1027-1037, y -1952 a -1966 : rien
	d'autre n'y est cree). L'interieur (x 1014-1023) est occupe par le
	comptoir, les chaises et les rayonnages du garage d'origine - y poser des
	voitures les aurait plantees dans le mobilier.
	Le z est volontairement pris AU-DESSUS du sol estime : un vehicule tombe
	et se pose tout seul, alors qu'un vehicule cree trop bas reste plante dans
	le decor. A CONFIRMER A L'OEIL en jeu, comme le reste de cette map.
*/

#define CONCESSION_MAX_EXPO 5

new concessionExpoVeh[CONCESSION_MAX_EXPO];
new concessionExpoNb = 0;

// Modeles choisis pour une concession d'occasion : de la berline banale au
// coupe, rien d'exotique ni de faction.
new const concessionExpoModel[CONCESSION_MAX_EXPO] = { 560, 405, 445, 562, 429 };
new const Float:concessionExpoX[CONCESSION_MAX_EXPO] = { 1029.0, 1032.5, 1036.0, 1029.0, 1032.5 };
new const Float:concessionExpoY[CONCESSION_MAX_EXPO] = { -1953.0, -1953.0, -1953.0, -1960.5, -1960.5 };
new const Float:concessionExpoZ[CONCESSION_MAX_EXPO] = { 13.0, 13.0, 13.0, 13.0, 13.0 };
new const Float:concessionExpoA[CONCESSION_MAX_EXPO] = { 0.0, 0.0, 0.0, 180.0, 180.0 };

stock Concession_CreateExpo()
{
	concessionExpoNb = 0;
	for(new i = 0; i < CONCESSION_MAX_EXPO; i++)
	{
		concessionExpoVeh[i] = -1;
		// Position passee par MM_Point : si la concession a deja ete deplacee
		// avant ce demarrage (mapdeplace.cfg relu par MM_Init), la vitrine doit
		// naitre avec le batiment et pas a l'ancienne adresse.
		new Float:nx, Float:ny, Float:nz;
		MM_Point(MM_CONCESSION, concessionExpoX[i], concessionExpoY[i], concessionExpoZ[i], nx, ny, nz);
		// -1 = ne reapparait jamais.
		new vid = CreateVehicle(concessionExpoModel[i], nx, ny, nz, MM_Angle(MM_CONCESSION, concessionExpoA[i]), 1, 1, -1);
		if(vid == INVALID_VEHICLE_ID || vid < 1 || vid >= MAX_CAR)
		{
			printf("[CONCESSION] Vehicule d'exposition %d : creation refusee par SA-MP.", i);
			continue;
		}
		concessionExpoVeh[i] = vid;
		concessionExpoNb++;

		// Moteur coupe, phares eteints, PORTIERES VERROUILLEES : c'est cette
		// derniere qui rend le vehicule definitivement non conduisible.
		SetVehicleParamsEx(vid, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_ON, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF, VEHICLE_PARAMS_OFF);
		SetVehicleVirtualWorld(vid, 0);
		LinkVehicleToInterior(vid, 0);

		// Ligne marquee vide : voir l'explication ci-dessus. Sans ca les
		// boucles de sauvegarde pourraient les prendre pour des vrais.
		vehicle[vid][used] = 0;
		vehicle[vid][SQLID] = -1;
		vehicle[vid][cOwned] = 0;
		vehicle[vid][cEngine] = false;
	}
	printf("[CONCESSION] %d vehicule(s) d'exposition (verrouilles, hors systeme vehicule).", concessionExpoNb);
	return concessionExpoNb;
}

// ============================================================================
// CE QUI DOIT SUIVRE LA MAP QUAND ELLE EST DEPLACEE (/a deplacer concession)
// ============================================================================
// Repositionne les vehicules de vitrine. On repart toujours des coordonnees
// d'origine (les tableaux const ci-dessus) et jamais de la position actuelle :
// sinon chaque deplacement s'ajouterait au precedent et la vitrine partirait
// en vrille au bout de trois essais.
stock Concession_MoveExpo()
{
	new Float:nx, Float:ny, Float:nz;
	for(new i = 0; i < CONCESSION_MAX_EXPO; i++)
	{
		if(concessionExpoVeh[i] == -1) continue;
		MM_Point(MM_CONCESSION, concessionExpoX[i], concessionExpoY[i], concessionExpoZ[i], nx, ny, nz);
		SetVehiclePos(concessionExpoVeh[i], nx, ny, nz);
		SetVehicleZAngle(concessionExpoVeh[i], MM_Angle(MM_CONCESSION, concessionExpoA[i]));
	}
	return 1;
}

// Le pickup 'N' de la 4e concession (Type=7) vit dans lvrp_server_dealership,
// pas dans un fichier de map : deplacer le batiment sans lui laisserait la
// concession achetable depuis un trottoir a l'autre bout de la ville.
//
// LA BASE N'EST VOLONTAIREMENT PAS REECRITE : la ligne SQL garde la position
// d'origine et le decalage vit dans mapdeplace.cfg, comme pour le reste. Ca
// evite au proprietaire d'avoir a lancer une requete SQL apres chaque essai,
// et surtout ca rend /a deplacer reset concession vraiment reversible.
// La position d'origine est memorisee au premier appel, c'est-a-dire juste
// apres dealerShip_Load, quand la valeur lue en base est encore intacte.
new Float:concessionPickupOrig[3];
new bool:concessionPickupCapture = false;

stock Concession_MovePickup()
{
	new id = -1;
	for(new i = 0; i < totalDealerShip && i < MAX_DEALSHIP; i++)
	{
		if(dealerShip[i][used] == 1 && dealerShip[i][typeZ] == 7) { id = i; break; }
	}
	// Tant que la ligne Type=7 n'existe pas en base, il n'y a rien a suivre.
	if(id == -1) return 0;

	if(!concessionPickupCapture)
	{
		concessionPickupOrig[0] = dealerShip[id][pos][0];
		concessionPickupOrig[1] = dealerShip[id][pos][1];
		concessionPickupOrig[2] = dealerShip[id][pos][2];
		concessionPickupCapture = true;
	}

	new Float:nx, Float:ny, Float:nz;
	MM_Point(MM_CONCESSION, concessionPickupOrig[0], concessionPickupOrig[1], concessionPickupOrig[2], nx, ny, nz);
	dealerShip[id][pos][0] = nx;
	dealerShip[id][pos][1] = ny;
	dealerShip[id][pos][2] = nz;
	dealerShip_Update(id);
	return 1;
}
