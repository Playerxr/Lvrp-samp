/*
	ngg_compat.inc - Couche de compatibilite NGG -> LVRP

	Permet d'inclure du code NGG (ex: phone_new.pwn) sans le modifier.
	Tous les symboles NGG sont mappes vers les equivalents LVRP, ou stubbes
	quand le systeme n'existe pas (PayPhones, Businesses-tel, etc.).

	A inclure APRES les declarations de PlayerInfo[] / MYSQL / variables LVRP.
*/

#if defined NGG_COMPAT_INCLUDED
	#endinput
#endif
#define NGG_COMPAT_INCLUDED

// =====================================================================
// MAPPINGS DIRECTS (NGG name -> LVRP name)
// =====================================================================

#define MainPipeline                MYSQL
#define pPnumber                    pNumber
#define gPlayerLogged               gPlayerLogged       // existe deja dans LVRP

// =====================================================================
// CONSTANTES NGG (a definir avant inclusion du phone NGG)
// =====================================================================

#define MAX_PAYPHONES               64
#define MAX_BUSINESSES              200
#define MAX_GROUPS                  32
#define MAX_PLAYERVEHICLES          5
#define INVALID_GROUP_ID            -1
#define INVALID_PLAYER_ID2          0xFFFF
#define SENDDATA_THREAD             0

// Dialog IDs NGG (decales pour eviter collision avec LVRP 9100-9410)
#define DIALOG_911MENU              9600
#define DIALOG_HOTLINE              9601
#define DIALOG_HELPCATOTHER1        9602
#define RTONEMENU                   9603

// =====================================================================
// COULEURS (NGG utilise des noms differents)
// =====================================================================

#if !defined COLOR_GREEN
	#define COLOR_GREEN             0x33AA33FF
#endif
#if !defined COLOR_WHITE
	#define COLOR_WHITE             0xFFFFFFAA
#endif
#if !defined COLOR_GRAD1
	#define COLOR_GRAD1             0xB4B5B7FF
#endif
#if !defined COLOR_GRAD2
	#define COLOR_GRAD2             0xBFC0C2FF
#endif
#if !defined COLOR_GRAD3
	#define COLOR_GRAD3             0xCBCCCEFF
#endif
#if !defined COLOR_PURPLE
	#define COLOR_PURPLE            0xC2A2DAAA
#endif
#if !defined COLOR_YELLOW
	#define COLOR_YELLOW            0xFFFF00AA
#endif
#if !defined COLOR_PINK
	#define COLOR_PINK              0xFF66FFAA
#endif
#if !defined COLOR_GREY
	#define COLOR_GREY              0xAFAFAFAA
#endif

// =====================================================================
// VARIABLES GLOBALES MANQUANTES (LVRP ne les a pas)
// =====================================================================

new PhoneOnline[MAX_PLAYERS];                // 0 = on, >0 = off
new Spectating[MAX_PLAYERS];                 // 0 = pas en spec
new PlayerCuffed[MAX_PLAYERS];               // 0 = pas menotte
new bJailPhoneUse[MAX_PLAYERS];              // jail phone occupe (par index prison)
new TempNumber[MAX_PLAYERS];                 // numero temporaire actif

// Buffers globaux NGG
new szMiscArray[256];

// =====================================================================
// HELPERS NGG -> LVRP
// =====================================================================

stock GetPlayerNameEx(playerid)
{
	static name[MAX_PLAYER_NAME];
	name[0] = 0;
	if(IsPlayerConnected(playerid))
		GetPlayerName(playerid, name, sizeof(name));
	return name;
}

stock GetPlayerNameExt(playerid)
{
	return GetPlayerNameEx(playerid);
}

stock GetPlayerSQLId(playerid)
{
	return PlayerInfo[playerid][pSQLID];
}

stock SendClientMessageEx(playerid, color, const message[])
{
	return SendClientMessage(playerid, color, message);
}

// Variante avec format en place (NGG utilise SendClientMessageEx avec format)
stock ChatTrafficProcess(playerid, color, const message[], type=0)
{
	#pragma unused type
	return SendClientMessage(playerid, color, message);
}

// Diffuse un message aux joueurs dans un rayon Float:distance de playerid
stock ProxDetector(Float:radi, playerid, const string[], col1, col2, col3, col4, col5)
{
	#pragma unused col2, col3, col4, col5
	if(!IsPlayerConnected(playerid)) return 0;
	new Float:posx, Float:posy, Float:posz;
	GetPlayerPos(playerid, posx, posy, posz);
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(GetPlayerVirtualWorld(i) != GetPlayerVirtualWorld(playerid)) continue;
		new Float:tx, Float:ty, Float:tz;
		GetPlayerPos(i, tx, ty, tz);
		new Float:d = floatsqroot((posx-tx)*(posx-tx) + (posy-ty)*(posy-ty) + (posz-tz)*(posz-tz));
		if(d <= radi)
			SendClientMessage(i, col1, string);
	}
	return 1;
}

stock Group_NumToDialogHex(color)
{
	static buf[12];
	format(buf, sizeof(buf), "%06x", color & 0xFFFFFF);
	return buf;
}

stock ShowPlayerDialogEx(playerid, dialogid, style, const caption[], const info[], const button1[], const button2[])
{
	return ShowPlayerDialog(playerid, dialogid, style, caption, info, button1, button2);
}

// Stubs pour systemes NGG non-portes
stock IsValidBusinessID(id) { #pragma unused id  return 0; }
stock IsAHitman(playerid)   { #pragma unused playerid  return 0; }

// =====================================================================
// STUBS pour systemes lourds NGG (PayPhones, Businesses, Groups)
// Stubbes ici pour que phone_new.pwn compile. Si vous voulez ces features,
// il faut implementer les structures + commandes admin pour les creer.
// =====================================================================

enum eNGG_PayPhoneStub {
	pp_iNumber,
	pp_iAreaID,
	pp_iCallerID
};
new arrPayPhoneData[MAX_PAYPHONES][eNGG_PayPhoneStub];

enum eNGG_BusinessStub {
	bName[64],
	bPhoneNr
};
new Businesses[MAX_BUSINESSES][eNGG_BusinessStub];

enum eNGG_GroupStub {
	g_szGroupName[64],
	g_hDutyColour,
	gTempNum
};
new arrGroupData[MAX_GROUPS][eNGG_GroupStub];

stock PayPhone_UpdateTextLabel(id, state)
{
	#pragma unused id, state
	return 0;
}

// Init stubs : reset arrays au demarrage
stock NGG_Compat_Init()
{
	for(new i = 0; i < MAX_PAYPHONES; i++)
	{
		arrPayPhoneData[i][pp_iCallerID] = INVALID_PLAYER_ID;
		arrPayPhoneData[i][pp_iNumber] = 0;
		arrPayPhoneData[i][pp_iAreaID] = -1;
	}
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		Mobile[i] = INVALID_PLAYER_ID;
		PhoneOnline[i] = 0;
		Spectating[i] = 0;
		PlayerCuffed[i] = 0;
		bJailPhoneUse[i] = 0;
		TempNumber[i] = 0;
	}
	return 1;
}
