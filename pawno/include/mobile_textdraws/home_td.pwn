/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			textdraws/home_td.pwn
*/

forward CreateHomescreenTD(playerid);
public CreateHomescreenTD(playerid)
{
	TEXTDRAW_HOME[playerid][0] = CreatePlayerTextDraw(playerid, 524.632812, 260.142456, "LD_BEAt:cHIT");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][0], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][0], 0xFFCC00FF); // Jaune iOS (Notes)
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][0], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][0], true);

	TEXTDRAW_HOME[playerid][1] = CreatePlayerTextDraw(playerid, 547.066040, 230.540634, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][1], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][1], 0x007AFFFF); // Bleu iOS (Banque)
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][1], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][1], true);

	// [REBRAND WA] Meme app (SMS) mais rebaptisee "WhatsApp" : vert WhatsApp officiel
	TEXTDRAW_HOME[playerid][2] = CreatePlayerTextDraw(playerid, 569.060668, 230.540634, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][2], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][2], 0x25D366FF); // Vert WhatsApp
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][2], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][2], true);

	// [ICONE TELEPHONE] Tile verte style iPhone (0x34C759FF = vert iOS) pour l'app Telephone
	TEXTDRAW_HOME[playerid][3] = CreatePlayerTextDraw(playerid, 569.060668, 259.342376, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][3], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][3], 885162495); // 0x34C759FF vert iOS
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][3], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][3], true);

	TEXTDRAW_HOME[playerid][4] = CreatePlayerTextDraw(playerid, 547.066040, 259.442382, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][4], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][4], 0x1C1C1EFF); // Noir iOS (Horloge)
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][4], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][4], true);

	TEXTDRAW_HOME[playerid][5] = CreatePlayerTextDraw(playerid, 524.632812, 230.540649, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][5], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][5], 0x1DA1F2FF); // Bleu Twitter (Twitter)
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][5], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][5], true);

	TEXTDRAW_HOME[playerid][6] = CreatePlayerTextDraw(playerid, 532.499206, 267.452148, "hud:radar_qmark");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][6], 7.000000, 12.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][6], 0);

	TEXTDRAW_HOME[playerid][7] = CreatePlayerTextDraw(playerid, 554.499938, 238.703826, "HUD:radar_cash");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][7], 9.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][7], 0);

	TEXTDRAW_HOME[playerid][8] = CreatePlayerTextDraw(playerid, 552.499145, 266.007720, "ld_grav:timer");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][8], 12.000000, 12.479988);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][8], 0);

	TEXTDRAW_HOME[playerid][9] = CreatePlayerTextDraw(playerid, 575.232910, 240.451766, "WA");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][9], 0.157999, 0.720592);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][9], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][9], 1);

	// [ICONE TELEPHONE] Texte "Tel" centre sur la tile verte (au lieu de "C")
	TEXTDRAW_HOME[playerid][10] = CreatePlayerTextDraw(playerid, 575.500000, 264.500000, "Tel");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][10], 0.150000, 0.700000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][10], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][10], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][10], 1);


	TEXTDRAW_HOME[playerid][11] = CreatePlayerTextDraw(playerid, 530.999877, 235.744445, "X");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][11], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][11], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][11], 1);

	// === [APPS AJOUTEES] 3 tiles supplementaires : GPS, Meteo, News ===
	// Tile 12 : GPS (bleu)
	TEXTDRAW_HOME[playerid][12] = CreatePlayerTextDraw(playerid, 524.632812, 293.500000, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][12], 23.000000, 11.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][12], 33023);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][12], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][12], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][12], true);

	// Tile 13 : Meteo (cyan)
	TEXTDRAW_HOME[playerid][13] = CreatePlayerTextDraw(playerid, 547.066040, 293.500000, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][13], 23.000000, 11.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][13], 13434879);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][13], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][13], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][13], true);

	// Tile 14 : News (rouge)
	TEXTDRAW_HOME[playerid][14] = CreatePlayerTextDraw(playerid, 569.060668, 293.500000, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][14], 23.000000, 11.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][14], -2147418367);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][14], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][14], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][14], true);

	// Labels texte sur tiles
	TEXTDRAW_HOME[playerid][15] = CreatePlayerTextDraw(playerid, 528.500000, 295.000000, "GPS");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][15], 0.150000, 0.700000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][15], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][15], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][15], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][15], 1);

	TEXTDRAW_HOME[playerid][16] = CreatePlayerTextDraw(playerid, 549.500000, 295.000000, "Met");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][16], 0.150000, 0.700000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][16], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][16], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][16], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][16], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][16], 1);

	TEXTDRAW_HOME[playerid][17] = CreatePlayerTextDraw(playerid, 572.000000, 295.000000, "New");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][17], 0.150000, 0.700000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][17], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][17], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][17], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][17], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][17], 1);
	return 1;
}