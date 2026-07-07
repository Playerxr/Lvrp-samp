/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			textdraws/bank_td.pwn
*/

forward CreateBankTD(playerid);
public CreateBankTD(playerid)
{
	TEXTDRAW_BANK[playerid][0] = CreatePlayerTextDraw(playerid, 519.000000, 230.237045, "stanje_na_racunu");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][0], 0.195666, 1.052443);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][0], 1);

	TEXTDRAW_BANK[playerid][1] = CreatePlayerTextDraw(playerid, 559.333190, 243.511138, "$900000");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][1], 0.344332, 1.400889);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][1], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_BANK[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][1], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][1], 1);

	TEXTDRAW_BANK[playerid][2] = CreatePlayerTextDraw(playerid, 535.333251, 277.111114, "vas_kredit");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][2], 0.195666, 1.052443);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][2], 1);

	TEXTDRAW_BANK[playerid][3] = CreatePlayerTextDraw(playerid, 559.000183, 288.725982, "$0");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][3], 0.344332, 1.400889);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][3], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][3], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_BANK[playerid][3], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][3], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][3], 1);

	TEXTDRAW_BANK[playerid][4] = CreatePlayerTextDraw(playerid, 521.666625, 309.881530, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][4], 0.000000, 6.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][4], 594.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][4], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_BANK[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][4], 1);

	TEXTDRAW_BANK[playerid][5] = CreatePlayerTextDraw(playerid, 523.999938, 347.888916, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][5], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][5], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][5], 0);

	TEXTDRAW_BANK[playerid][6] = CreatePlayerTextDraw(playerid, 536.666564, 337.518493, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][6], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][6], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][6], 0);

	TEXTDRAW_BANK[playerid][7] = CreatePlayerTextDraw(playerid, 545.999877, 344.570343, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][7], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][7], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][7], 0);

	TEXTDRAW_BANK[playerid][8] = CreatePlayerTextDraw(playerid, 557.666503, 333.370300, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][8], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][8], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][8], 0);

	TEXTDRAW_BANK[playerid][9] = CreatePlayerTextDraw(playerid, 566.999877, 336.274017, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][9], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][9], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][9], 0);

	TEXTDRAW_BANK[playerid][10] = CreatePlayerTextDraw(playerid, 573.299804, 327.477661, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][10], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][10], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][10], 0);

	TEXTDRAW_BANK[playerid][11] = CreatePlayerTextDraw(playerid, 580.266601, 340.692657, "ld_beat:chit"); // Pad
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][11], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][11], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][11], 0);

	TEXTDRAW_BANK[playerid][12] = CreatePlayerTextDraw(playerid, 522.000244, 309.051879, "grafik");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][12], 0.125666, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][12], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][12], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][12], 1);

	TEXTDRAW_BANK[playerid][13] = CreatePlayerTextDraw(playerid, 565.367004, 323.069824, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][13], 16.000000, 26.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][13], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][13], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][13], 19445);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][13], 180.000000, 45.000000, 0.000000, 1.000000);

	TEXTDRAW_BANK[playerid][14] = CreatePlayerTextDraw(playerid, 574.099853, 330.722290, ""); // Pad
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][14], 13.000000, 16.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][14], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][14], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][14], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][14], -90.000000, 0.000000, 30.000000, 1.000000);

	TEXTDRAW_BANK[playerid][15] = CreatePlayerTextDraw(playerid, 541.565368, 332.630126, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][15], 6.000000, 26.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][15], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][15], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][15], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][15], -90.000000, 0.000000, 76.000000, 1.000000);

	TEXTDRAW_BANK[playerid][16] = CreatePlayerTextDraw(playerid, 562.899169, 331.544738, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][16], 6.000000, 16.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][16], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][16], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][16], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][16], -90.000000, 0.000000, -100.000000, 0.800000);

	TEXTDRAW_BANK[playerid][17] = CreatePlayerTextDraw(playerid, 550.066223, 328.310791, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][17], 13.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][17], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][17], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][17], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][17], -90.000000, 0.000000, -65.000000, 1.000000);

	TEXTDRAW_BANK[playerid][18] = CreatePlayerTextDraw(playerid, 526.299682, 333.088562, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][18], 16.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][18], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][18], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][18], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][18], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][18], -90.000000, 0.000000, -65.000000, 1.000000);

	// Rast
	TEXTDRAW_BANK[playerid][19] = CreatePlayerTextDraw(playerid, 580.533264, 317.807312, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][19], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][19], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][19], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][19], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][19], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][19], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][19], 0);

	TEXTDRAW_BANK[playerid][20] = CreatePlayerTextDraw(playerid, 572.632934, 321.633636, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][20], 14.000000, 12.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][20], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][20], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][20], 5);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][20], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][20], -90.000000, 0.000000, -30.000000, 0.800000);
	return 1;
}