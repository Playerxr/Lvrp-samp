/*
	afrp_mobile_apps.inc - 3 nouvelles apps pour le /mobile

	Apps :
	- GPS  : liste de 18 destinations, click = checkpoint sur la map
	- Met  : meteo + heure + date (info live)
	- New  : flux des 10 derniers events/annonces admin

	Tiles :
	- TEXTDRAW_HOME[playerid][12] = GPS tile
	- TEXTDRAW_HOME[playerid][13] = Meteo tile
	- TEXTDRAW_HOME[playerid][14] = News tile

	Dialogs :
	- DIALOG_APP_GPS    = 9700 (liste destinations)
	- DIALOG_APP_METEO  = 9701 (msgbox info)
	- DIALOG_APP_NEWS   = 9702 (msgbox feed)
*/

#if defined AFRP_MOBILE_APPS_INCLUDED
	#endinput
#endif
#define AFRP_MOBILE_APPS_INCLUDED

#define DIALOG_APP_GPS    9700
#define DIALOG_APP_METEO  9701
#define DIALOG_APP_NEWS   9702

#define NEWS_FEED_SIZE    10

// === GPS : destinations utiles ===
static const Float:gpsCoords[][3] = {
	{1456.8, -1023.6, 23.8},   // Banque Centre LS
	{-1990.0, 1107.0, 53.1},   // Banque SF
	{2438.5, -2113.5, 13.5},   // Hopital LS
	{1175.0, -1323.5, 14.0},   // Hopital All Saints
	{1545.3, -1675.6, 16.2},   // LSPD HQ
	{-1605.5, 711.0, 14.1},    // SFPD HQ
	{2287.8, 2433.8, 10.8},    // LVPD HQ
	{-1450.0, 522.0, 17.0},    // FBI HQ SF
	{1487.4, 1305.0, 10.8},    // Pompiers LV
	{2117.32, -1790.55, 13.55},// Pizza
	{2276.65, -2226.90, 13.54},// Eboueur
	{1969.42, -2189.07, 13.54},// Pilote/Aero LS
	{1683.0, 1447.0, 10.7},    // Aero LV
	{-1655.0, 1213.0, 7.2},    // Aero SF
	{2284.8, -1326.1, 25.5},   // Marche 24/7 LS
	{1152.3, -1657.2, 14.5},   // Marche 24/7 LV
	{1340.6, -1318.0, 14.0},   // Marche 24/7 Vinewood
	{683.6, -464.7, 16.5}      // Mont Chiliad
};
static const gpsNames[][32] = {
	"Banque - Centre LS",
	"Banque - San Fierro",
	"Hopital - LS Centre",
	"Hopital - All Saints",
	"LSPD HQ",
	"SFPD HQ",
	"LVPD HQ",
	"FBI HQ",
	"Caserne Pompiers LV",
	"Job: Pizza",
	"Job: Eboueur",
	"Aeroport LS",
	"Aeroport LV",
	"Aeroport SF",
	"Marche 24/7 LS",
	"Marche 24/7 LV",
	"Marche 24/7 Vinewood",
	"Mont Chiliad (vue)"
};

stock MobileApps_OpenGPS(playerid)
{
	new list[1024];
	list[0] = 0;
	for(new i = 0; i < sizeof(gpsCoords); i++)
	{
		new tmp[48];
		format(tmp, sizeof(tmp), "{FFFFFF}%s\n", gpsNames[i]);
		strcat(list, tmp, sizeof(list));
	}
	ShowPlayerDialog(playerid, DIALOG_APP_GPS, DIALOG_STYLE_LIST,
		"{007AFF}>> GPS - Choisir destination <<", list, "Aller", "Retour");
	return 1;
}

// === METEO + HEURE ===
stock MobileApps_OpenMeteo(playerid)
{
	new h, m, s, day, mo, y;
	gettime(h, m, s);
	getdate(y, mo, day);

	new const moisNames[13][12] = {
		"", "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin",
		"Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"
	};

	// Determine "meteo" selon heure (simulation simple)
	new meteoStr[32];
	if(h >= 22 || h < 6) meteoStr = "Nuit claire, 15 C";
	else if(h < 10) meteoStr = "Brouillard matinal, 18 C";
	else if(h < 16) meteoStr = "Ensoleille, 26 C";
	else meteoStr = "Couvert, 22 C";

	new info[400];
	format(info, sizeof(info),
		"{FFFFFF}>>> Meteo Los Santos <<<\n\n{8C8C8C}Date:{FFFFFF} %d %s %d\n{8C8C8C}Heure:{FFFFFF} %02d:%02d:%02d\n{8C8C8C}Conditions:{FFFFFF} %s\n\n{8C8C8C}Previsions 6h:\n{FFFFFF}Sec, pas de pluie attendue.\n\n{00CCFF}Tape /event si un event est en cours.",
		day, moisNames[mo], y, h, m, s, meteoStr);

	ShowPlayerDialog(playerid, DIALOG_APP_METEO, DIALOG_STYLE_MSGBOX,
		"{00CCFF}>> Meteo & Heure <<", info, "OK", "");
	return 1;
}

// === NEWS / FLUX ===
new newsFeed[NEWS_FEED_SIZE][140];
new newsCount = 0;

stock News_Push(const text[])
{
	// Decale le buffer (FIFO inverse - les recents en haut)
	for(new i = NEWS_FEED_SIZE - 1; i > 0; i--)
	{
		strmid(newsFeed[i], newsFeed[i-1], 0, strlen(newsFeed[i-1]), 140);
	}
	strmid(newsFeed[0], text, 0, strlen(text), 140);
	if(newsCount < NEWS_FEED_SIZE) newsCount++;
	return 1;
}

stock MobileApps_OpenNews(playerid)
{
	new list[1500];
	list[0] = 0;
	if(newsCount == 0)
	{
		strcat(list, "{8C8C8C}Aucune annonce pour l'instant.\n\nReviens plus tard pour voir les events admin et les annonces.", sizeof(list));
	}
	else
	{
		new h, m, s; gettime(h, m, s);
		for(new i = 0; i < newsCount; i++)
		{
			strcat(list, "{FFFFFF}- ", sizeof(list));
			strcat(list, newsFeed[i], sizeof(list));
			strcat(list, "\n", sizeof(list));
		}
	}
	ShowPlayerDialog(playerid, DIALOG_APP_NEWS, DIALOG_STYLE_MSGBOX,
		"{FF6666}>> News & Annonces <<", list, "OK", "");
	return 1;
}

// === Dispatch clic depuis OnPlayerClickPlayerTextDraw ===
// Appele AVANT le dispatch existant mobile_system pour intercepter slots 12/13/14
stock bool:MobileApps_OnClick(playerid, PlayerText:playertextid)
{
	if(playertextid == TEXTDRAW_HOME[playerid][12]) { MobileApps_OpenGPS(playerid);   return true; }
	if(playertextid == TEXTDRAW_HOME[playerid][13]) { MobileApps_OpenMeteo(playerid); return true; }
	if(playertextid == TEXTDRAW_HOME[playerid][14]) { MobileApps_OpenNews(playerid);  return true; }
	return false;
}

// === Dispatch dialog (a appeler depuis OnDialogResponse) ===
stock MobileApps_OnDialogResponse(playerid, dialogid, response, listitem)
{
	if(dialogid == DIALOG_APP_GPS)
	{
		if(!response) return 1;
		if(listitem < 0 || listitem >= sizeof(gpsCoords)) return 1;
		SetPlayerCheckpoint(playerid,
			gpsCoords[listitem][0], gpsCoords[listitem][1], gpsCoords[listitem][2],
			5.0);
		new msg[140];
		format(msg, sizeof(msg),
			"{007AFF}[GPS]{FFFFFF} Destination {FFFF00}%s{FFFFFF} marquee sur ta carte.",
			gpsNames[listitem]);
		SendClientMessage(playerid, -1, msg);
		return 1;
	}
	if(dialogid == DIALOG_APP_METEO || dialogid == DIALOG_APP_NEWS)
	{
		// Juste OK -> rien a faire
		return 1;
	}
	return 0;
}

// Init : push quelques entrees demo dans le news feed
stock MobileApps_Init()
{
	News_Push("Bienvenue sur AFRP-RP ! Tape /aide pour les commandes.");
	News_Push("Events automatiques toutes les 30-60 min. Tape /event.");
	News_Push("Nouveau telephone disponible : /shop puis /mobile.");
	return 1;
}
