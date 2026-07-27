/*
	afrp_hud.inc - HUD personnalise : barres vie/armure stylees + icones + ping

	A inclure dans LVRP.pwn apres PlayerInfo.

	=== LAYOUT (depuis 2026-07) ===
	Tout le bloc d'etat du joueur est regroupe EN BAS A GAUCHE, a droite du radar,
	pour liberer le haut de l'ecran et faciliter la lecture :

	     [coeur]  Vie:100     [====================]   <- ici (afrp_hud.inc)
	              Armure:50   [==========          ]   <- ici (afrp_hud.inc)
	     [burger] Soif:80     [================    ]   <- LVRP.pwn (legacy)
	              Faim:75     [===============     ]   <- LVRP.pwn (legacy)

	Le radar de GTA occupe environ X 25..135. Le bloc commence a X 146 : il ne
	masque donc jamais le radar.

	Les barres faim/soif et les textes "Vie:" / "Armure:" restent geres par
	LVRP.pwn (UpdateStatsTextsPlayers). Ce module ne gere que :
	  - les barres graphiques vie/armure
	  - les 2 icones sprite (coeur + burger)
	  - le ping

	Hooks a brancher :
	- OnPlayerConnect    -> HUD_OnConnect(playerid)
	- OnPlayerDisconnect -> HUD_OnDisconnect(playerid)
	- OnPlayerSpawn      -> HUD_OnSpawn(playerid)  (re-show apres spawn)
	- Optionnel : commande /hud pour toggle
*/

#if defined AFRP_HUD_INCLUDED
	#endinput
#endif
#define AFRP_HUD_INCLUDED

// --- Position des barres vie / armure (bas gauche, a droite du radar) ---
#define HUD_BAR_X        174.0
#define HUD_BAR_Y_HP     374.0
#define HUD_BAR_Y_AR     386.0
#define HUD_BAR_W_MAX    62.0
#define HUD_BAR_H        4.0

// --- Icones sprite (font 4) ---
// Si une icone ne s'affiche pas en jeu, il suffit de changer le nom ici.
// "hud:radar_girlfriend" = le blip coeur de GTA SA
// "hud:radar_burgerShot" = le blip burger de GTA SA
#define HUD_SPR_COEUR    "hud:radar_girlfriend"
#define HUD_SPR_BURGER   "hud:radar_burgerShot"
#define HUD_ICON_X       150.0
#define HUD_ICON_Y_VIE   371.0
#define HUD_ICON_Y_FAIM  398.0
#define HUD_ICON_W       16.0
#define HUD_ICON_H       16.0

// --- Ping (inchange, haut droite) ---
#define HUD_PING_X       540.0
#define HUD_PING_Y       100.0

new PlayerText:hudHpBg[MAX_PLAYERS];
new PlayerText:hudHpFg[MAX_PLAYERS];
new PlayerText:hudArBg[MAX_PLAYERS];
new PlayerText:hudArFg[MAX_PLAYERS];
new PlayerText:hudIcoVie[MAX_PLAYERS];
new PlayerText:hudIcoFaim[MAX_PLAYERS];
new PlayerText:hudPing[MAX_PLAYERS];
new hudVisible[MAX_PLAYERS];

stock HUD_Create(playerid)
{
	if(hudHpBg[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	// === Barre VIE (fond gris + foreground rouge dynamique) ===
	hudHpBg[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X, HUD_BAR_Y_HP, "_");
	PlayerTextDrawFont(playerid, hudHpBg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, hudHpBg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, hudHpBg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, hudHpBg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, hudHpBg[playerid], HUD_BAR_X + HUD_BAR_W_MAX, HUD_BAR_Y_HP + HUD_BAR_H);
	PlayerTextDrawUseBox(playerid, hudHpBg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, hudHpBg[playerid], 0x000000AA);

	// Foreground (largeur variable selon HP)
	hudHpFg[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X, HUD_BAR_Y_HP, "_");
	PlayerTextDrawFont(playerid, hudHpFg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, hudHpFg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, hudHpFg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, hudHpFg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, hudHpFg[playerid], HUD_BAR_X + HUD_BAR_W_MAX, HUD_BAR_Y_HP + HUD_BAR_H);
	PlayerTextDrawUseBox(playerid, hudHpFg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, hudHpFg[playerid], 0xCC0000FF);

	// === Barre ARMURE ===
	hudArBg[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X, HUD_BAR_Y_AR, "_");
	PlayerTextDrawFont(playerid, hudArBg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, hudArBg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, hudArBg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, hudArBg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, hudArBg[playerid], HUD_BAR_X + HUD_BAR_W_MAX, HUD_BAR_Y_AR + HUD_BAR_H);
	PlayerTextDrawUseBox(playerid, hudArBg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, hudArBg[playerid], 0x000000AA);

	hudArFg[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X, HUD_BAR_Y_AR, "_");
	PlayerTextDrawFont(playerid, hudArFg[playerid], 1);
	PlayerTextDrawLetterSize(playerid, hudArFg[playerid], 0.0, 0.0);
	PlayerTextDrawColor(playerid, hudArFg[playerid], 0x00000000);
	PlayerTextDrawBackgroundColor(playerid, hudArFg[playerid], 0x00000000);
	PlayerTextDrawTextSize(playerid, hudArFg[playerid], HUD_BAR_X + HUD_BAR_W_MAX, HUD_BAR_Y_AR + HUD_BAR_H);
	PlayerTextDrawUseBox(playerid, hudArFg[playerid], 1);
	PlayerTextDrawBoxColor(playerid, hudArFg[playerid], 0x00CCFFFF);

	// === Icone COEUR (vie / armure) ===
	hudIcoVie[playerid] = CreatePlayerTextDraw(playerid, HUD_ICON_X, HUD_ICON_Y_VIE, HUD_SPR_COEUR);
	PlayerTextDrawFont(playerid, hudIcoVie[playerid], 4);
	PlayerTextDrawTextSize(playerid, hudIcoVie[playerid], HUD_ICON_W, HUD_ICON_H);
	PlayerTextDrawAlignment(playerid, hudIcoVie[playerid], 1);
	PlayerTextDrawColor(playerid, hudIcoVie[playerid], 0xFFFFFFFF);
	PlayerTextDrawBackgroundColor(playerid, hudIcoVie[playerid], 0x00000000);
	PlayerTextDrawSetShadow(playerid, hudIcoVie[playerid], 0);
	PlayerTextDrawSetOutline(playerid, hudIcoVie[playerid], 0);
	PlayerTextDrawSetProportional(playerid, hudIcoVie[playerid], 0);

	// === Icone BURGER (faim / soif) ===
	hudIcoFaim[playerid] = CreatePlayerTextDraw(playerid, HUD_ICON_X, HUD_ICON_Y_FAIM, HUD_SPR_BURGER);
	PlayerTextDrawFont(playerid, hudIcoFaim[playerid], 4);
	PlayerTextDrawTextSize(playerid, hudIcoFaim[playerid], HUD_ICON_W, HUD_ICON_H);
	PlayerTextDrawAlignment(playerid, hudIcoFaim[playerid], 1);
	PlayerTextDrawColor(playerid, hudIcoFaim[playerid], 0xFFFFFFFF);
	PlayerTextDrawBackgroundColor(playerid, hudIcoFaim[playerid], 0x00000000);
	PlayerTextDrawSetShadow(playerid, hudIcoFaim[playerid], 0);
	PlayerTextDrawSetOutline(playerid, hudIcoFaim[playerid], 0);
	PlayerTextDrawSetProportional(playerid, hudIcoFaim[playerid], 0);

	// === Ping ===
	hudPing[playerid] = CreatePlayerTextDraw(playerid, HUD_PING_X, HUD_PING_Y, "Ping: ---");
	PlayerTextDrawFont(playerid, hudPing[playerid], 2);
	PlayerTextDrawLetterSize(playerid, hudPing[playerid], 0.20, 1.0);
	PlayerTextDrawColor(playerid, hudPing[playerid], 0x00FF00FF);
	PlayerTextDrawSetShadow(playerid, hudPing[playerid], 1);
	PlayerTextDrawSetOutline(playerid, hudPing[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, hudPing[playerid], 0x000000FF);

	hudVisible[playerid] = 1;
	return 1;
}

stock HUD_Show(playerid)
{
	if(hudHpBg[playerid] == PlayerText:INVALID_TEXT_DRAW) HUD_Create(playerid);
	PlayerTextDrawShow(playerid, hudHpBg[playerid]);
	PlayerTextDrawShow(playerid, hudHpFg[playerid]);
	PlayerTextDrawShow(playerid, hudArBg[playerid]);
	PlayerTextDrawShow(playerid, hudArFg[playerid]);
	PlayerTextDrawShow(playerid, hudIcoVie[playerid]);
	PlayerTextDrawShow(playerid, hudIcoFaim[playerid]);
	PlayerTextDrawShow(playerid, hudPing[playerid]);
	hudVisible[playerid] = 1;
	return 1;
}

stock HUD_Hide(playerid)
{
	if(hudHpBg[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;
	PlayerTextDrawHide(playerid, hudHpBg[playerid]);
	PlayerTextDrawHide(playerid, hudHpFg[playerid]);
	PlayerTextDrawHide(playerid, hudArBg[playerid]);
	PlayerTextDrawHide(playerid, hudArFg[playerid]);
	PlayerTextDrawHide(playerid, hudIcoVie[playerid]);
	PlayerTextDrawHide(playerid, hudIcoFaim[playerid]);
	PlayerTextDrawHide(playerid, hudPing[playerid]);
	hudVisible[playerid] = 0;
	return 1;
}

// Update les valeurs (appele par timer 1s)
stock HUD_Update(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	if(hudVisible[playerid] != 1) return 0;
	if(hudHpFg[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;

	// HP : redimensionne le foreground
	new Float:hp, Float:ar;
	GetPlayerHealth(playerid, hp);
	GetPlayerArmour(playerid, ar);
	if(hp < 0.0) hp = 0.0;
	if(hp > 100.0) hp = 100.0;
	if(ar < 0.0) ar = 0.0;
	if(ar > 100.0) ar = 100.0;

	new Float:hpW = HUD_BAR_W_MAX * (hp / 100.0);
	new Float:arW = HUD_BAR_W_MAX * (ar / 100.0);
	if(hpW < 1.0) hpW = 1.0; // garde 1px pour la box visible
	if(arW < 1.0) arW = 1.0;

	// Redimensionne via PlayerTextDrawTextSize (hide/show pour refresh)
	PlayerTextDrawTextSize(playerid, hudHpFg[playerid], HUD_BAR_X + hpW, HUD_BAR_Y_HP + HUD_BAR_H);
	PlayerTextDrawTextSize(playerid, hudArFg[playerid], HUD_BAR_X + arW, HUD_BAR_Y_AR + HUD_BAR_H);

	// Couleur HP dynamique : vert si > 60, jaune si 30-60, rouge si < 30
	new hpColor;
	if(hp > 60.0)      hpColor = 0x44CC44FF;
	else if(hp > 30.0) hpColor = 0xFFAA00FF;
	else               hpColor = 0xCC0000FF;
	PlayerTextDrawBoxColor(playerid, hudHpFg[playerid], hpColor);

	// Cache armure si 0
	if(ar < 1.0)
	{
		PlayerTextDrawBoxColor(playerid, hudArFg[playerid], 0x00000000);
	}
	else
	{
		PlayerTextDrawBoxColor(playerid, hudArFg[playerid], 0x00CCFFFF);
	}

	// Re-show pour appliquer les changements visuels
	PlayerTextDrawShow(playerid, hudHpFg[playerid]);
	PlayerTextDrawShow(playerid, hudArFg[playerid]);

	// Ping : update avec couleur selon valeur
	new ping = GetPlayerPing(playerid);
	new pingStr[20];
	format(pingStr, sizeof(pingStr), "Ping: %d", ping);
	PlayerTextDrawSetString(playerid, hudPing[playerid], pingStr);
	new pingColor;
	if(ping < 100)      pingColor = 0x00FF00FF;
	else if(ping < 200) pingColor = 0xFFAA00FF;
	else                pingColor = 0xFF3333FF;
	PlayerTextDrawColor(playerid, hudPing[playerid], pingColor);
	PlayerTextDrawShow(playerid, hudPing[playerid]);

	return 1;
}

stock HUD_OnConnect(playerid)
{
	hudHpBg[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudHpFg[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudArBg[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudArFg[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudIcoVie[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudIcoFaim[playerid] = PlayerText:INVALID_TEXT_DRAW;
	hudPing[playerid]    = PlayerText:INVALID_TEXT_DRAW;
	hudVisible[playerid] = 0;
	return 1;
}

stock HUD_OnDisconnect(playerid)
{
	if(hudHpBg[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, hudHpBg[playerid]);
		PlayerTextDrawDestroy(playerid, hudHpFg[playerid]);
		PlayerTextDrawDestroy(playerid, hudArBg[playerid]);
		PlayerTextDrawDestroy(playerid, hudArFg[playerid]);
		PlayerTextDrawDestroy(playerid, hudIcoVie[playerid]);
		PlayerTextDrawDestroy(playerid, hudIcoFaim[playerid]);
		PlayerTextDrawDestroy(playerid, hudPing[playerid]);
	}
	HUD_OnConnect(playerid); // reset les vars
	return 1;
}

stock HUD_OnSpawn(playerid)
{
	HUD_Show(playerid);
	return 1;
}

// Timer global qui refresh tous les HUD une fois par seconde
forward HUD_GlobalTick();
public HUD_GlobalTick()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		HUD_Update(i);
	}
	return 1;
}

stock HUD_Init()
{
	SetTimer("HUD_GlobalTick", 1000, true);
	return 1;
}

// Toggle commande /hud
stock HUD_CmdToggle(playerid)
{
	if(hudVisible[playerid] == 1)
	{
		HUD_Hide(playerid);
		SendClientMessage(playerid, 0xAAAAAAFF, "{AAAAAA}[HUD]{FFFFFF} Cache. /hud pour reafficher.");
	}
	else
	{
		HUD_Show(playerid);
		SendClientMessage(playerid, 0xAAAAAAFF, "{AAAAAA}[HUD]{FFFFFF} Affiche.");
	}
	return 1;
}
