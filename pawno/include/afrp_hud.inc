/*
	afrp_hud.inc - HUD personnalise : barres vie/armure stylees + ping affiche

	A inclure dans LVRP.pwn apres PlayerInfo.

	- Barres vie/armure custom (style boxe couleur) sous les bars faim/soif existantes
	- Ping affiche en haut a droite, format "Ping: XX"
	- Timer 1s pour rafraichir
	- Init au connect, destroy au disconnect

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

// Layout : barres vie/armure a Y=60 (sous faim/soif), ping a Y=10 (au-dessus de tout)
#define HUD_BAR_X        548.0
#define HUD_BAR_Y_HP     60.0
#define HUD_BAR_Y_AR     70.0
#define HUD_BAR_W_MAX    85.0
#define HUD_BAR_H        4.0
#define HUD_PING_X       540.0
#define HUD_PING_Y       100.0

new PlayerText:hudHpBg[MAX_PLAYERS];
new PlayerText:hudHpFg[MAX_PLAYERS];
new PlayerText:hudHpLbl[MAX_PLAYERS];
new PlayerText:hudArBg[MAX_PLAYERS];
new PlayerText:hudArFg[MAX_PLAYERS];
new PlayerText:hudArLbl[MAX_PLAYERS];
new PlayerText:hudPing[MAX_PLAYERS];
new hudVisible[MAX_PLAYERS];

stock HUD_Create(playerid)
{
	if(hudHpBg[playerid] != PlayerText:INVALID_TEXT_DRAW) return 1;

	// === Barre VIE (fond gris + foreground rouge dynamique + label) ===
	// Fond (background bar)
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

	// Label
	hudHpLbl[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X - 18.0, HUD_BAR_Y_HP - 1.0, "VIE");
	PlayerTextDrawFont(playerid, hudHpLbl[playerid], 2);
	PlayerTextDrawLetterSize(playerid, hudHpLbl[playerid], 0.18, 0.9);
	PlayerTextDrawColor(playerid, hudHpLbl[playerid], 0xFFFFFFFF);
	PlayerTextDrawSetShadow(playerid, hudHpLbl[playerid], 1);

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

	hudArLbl[playerid] = CreatePlayerTextDraw(playerid, HUD_BAR_X - 18.0, HUD_BAR_Y_AR - 1.0, "ARM");
	PlayerTextDrawFont(playerid, hudArLbl[playerid], 2);
	PlayerTextDrawLetterSize(playerid, hudArLbl[playerid], 0.18, 0.9);
	PlayerTextDrawColor(playerid, hudArLbl[playerid], 0xFFFFFFFF);
	PlayerTextDrawSetShadow(playerid, hudArLbl[playerid], 1);

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
	PlayerTextDrawShow(playerid, hudHpLbl[playerid]);
	PlayerTextDrawShow(playerid, hudArBg[playerid]);
	PlayerTextDrawShow(playerid, hudArFg[playerid]);
	PlayerTextDrawShow(playerid, hudArLbl[playerid]);
	PlayerTextDrawShow(playerid, hudPing[playerid]);
	hudVisible[playerid] = 1;
	return 1;
}

stock HUD_Hide(playerid)
{
	if(hudHpBg[playerid] == PlayerText:INVALID_TEXT_DRAW) return 0;
	PlayerTextDrawHide(playerid, hudHpBg[playerid]);
	PlayerTextDrawHide(playerid, hudHpFg[playerid]);
	PlayerTextDrawHide(playerid, hudHpLbl[playerid]);
	PlayerTextDrawHide(playerid, hudArBg[playerid]);
	PlayerTextDrawHide(playerid, hudArFg[playerid]);
	PlayerTextDrawHide(playerid, hudArLbl[playerid]);
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
	hudHpBg[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudHpFg[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudHpLbl[playerid] = PlayerText:INVALID_TEXT_DRAW;
	hudArBg[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudArFg[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudArLbl[playerid] = PlayerText:INVALID_TEXT_DRAW;
	hudPing[playerid]  = PlayerText:INVALID_TEXT_DRAW;
	hudVisible[playerid] = 0;
	return 1;
}

stock HUD_OnDisconnect(playerid)
{
	if(hudHpBg[playerid] != PlayerText:INVALID_TEXT_DRAW)
	{
		PlayerTextDrawDestroy(playerid, hudHpBg[playerid]);
		PlayerTextDrawDestroy(playerid, hudHpFg[playerid]);
		PlayerTextDrawDestroy(playerid, hudHpLbl[playerid]);
		PlayerTextDrawDestroy(playerid, hudArBg[playerid]);
		PlayerTextDrawDestroy(playerid, hudArFg[playerid]);
		PlayerTextDrawDestroy(playerid, hudArLbl[playerid]);
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
