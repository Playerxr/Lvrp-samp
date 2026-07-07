/*
	afrp_motd.inc - Message of the day + live admin counter

	MOTD :
	- Persistant en fichier scriptfiles/motd.txt
	- /setmotd <texte> (admin lvl 3+)
	- Affiche au login

	Live Admin Counter :
	- TextDraw global "Admins online: X" en bas-gauche
	- Refresh toutes les 30s
*/

#if defined AFRP_MOTD_INCLUDED
	#endinput
#endif
#define AFRP_MOTD_INCLUDED

#define MOTD_FILE "motd.txt"
#define MOTD_MAX_LEN 256

new motdText[MOTD_MAX_LEN] = "Bienvenue sur AFRP-RP ! Tape /tuto pour le tutoriel et /aide pour les commandes.";
new Text:adminCounterTD = Text:INVALID_TEXT_DRAW;

stock MOTD_Load()
{
	new File:f = fopen(MOTD_FILE, io_read);
	if(!f) return 0;
	new line[MOTD_MAX_LEN];
	if(fread(f, line, sizeof(line)))
	{
		new len = strlen(line);
		while(len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) { line[len-1] = 0; len--; }
		if(strlen(line) > 0) strmid(motdText, line, 0, strlen(line), MOTD_MAX_LEN);
	}
	fclose(f);
	return 1;
}

stock MOTD_Save()
{
	new File:f = fopen(MOTD_FILE, io_write);
	if(!f) return 0;
	fwrite(f, motdText);
	fclose(f);
	return 1;
}

stock MOTD_ShowToPlayer(playerid)
{
	if(!IsPlayerConnected(playerid)) return 0;
	new msg[MOTD_MAX_LEN + 32];
	format(msg, sizeof(msg), "{FFD700}[MOTD]{FFFFFF} %s", motdText);
	SendClientMessage(playerid, -1, msg);
	return 1;
}

stock MOTD_CmdSet(playerid, const newText[])
{
	if(PlayerInfo[playerid][pAdmin] < 3)
		return SendClientMessage(playerid, -1, "{FF4444}Reserve admin niveau 3+.");
	if(strlen(newText) < 5)
		return SendClientMessage(playerid, -1, "{FFFFFF}Usage: /setmotd <message>");
	if(strlen(newText) >= MOTD_MAX_LEN)
		return SendClientMessage(playerid, -1, "{FF4444}Message trop long (max 256 chars).");
	strmid(motdText, newText, 0, strlen(newText), MOTD_MAX_LEN);
	MOTD_Save();
	new ack[100];
	format(ack, sizeof(ack), "{00FF00}[MOTD]{FFFFFF} Nouveau message defini. Visible au prochain login.");
	SendClientMessage(playerid, -1, ack);
	// Broadcast immediate
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(IsPlayerConnected(i) && !IsPlayerNPC(i)) MOTD_ShowToPlayer(i);
	}
	return 1;
}

// === ADMIN COUNTER ===
stock MOTD_CountAdmins()
{
	new c = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pAdmin] >= 1) c++;
	}
	return c;
}

forward MOTD_AdminCounterTick();
public MOTD_AdminCounterTick()
{
	if(adminCounterTD == Text:INVALID_TEXT_DRAW) return 0;
	new buf[40];
	format(buf, sizeof(buf), "Admins online: ~y~%d", MOTD_CountAdmins());
	TextDrawSetString(adminCounterTD, buf);
	return 1;
}

stock MOTD_Init()
{
	MOTD_Load();
	// TextDraw global en bas-gauche
	adminCounterTD = TextDrawCreate(10.0, 426.0, "Admins online: ~y~0");
	TextDrawFont(adminCounterTD, 2);
	TextDrawLetterSize(adminCounterTD, 0.25, 1.1);
	TextDrawColor(adminCounterTD, 0xFFFFFFFF);
	TextDrawSetShadow(adminCounterTD, 1);
	TextDrawSetOutline(adminCounterTD, 0);
	TextDrawBackgroundColor(adminCounterTD, 0x000000CC);
	TextDrawUseBox(adminCounterTD, 0);
	// Refresh toutes les 30s
	SetTimer("MOTD_AdminCounterTick", 30 * 1000, true);
	return 1;
}

stock MOTD_ShowCounter(playerid)
{
	if(adminCounterTD != Text:INVALID_TEXT_DRAW)
		TextDrawShowForPlayer(playerid, adminCounterTD);
	return 1;
}

stock MOTD_HideCounter(playerid)
{
	if(adminCounterTD != Text:INVALID_TEXT_DRAW)
		TextDrawHideForPlayer(playerid, adminCounterTD);
	return 1;
}
