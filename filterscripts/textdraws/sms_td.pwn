/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			textdraws/sms_td.pwn
*/

forward CreateSMSTD(playerid);
public CreateSMSTD(playerid)
{
	TEXTDRAW_SMS[playerid][0] = CreatePlayerTextDraw(playerid, 559.666870, 289.140808, "upisite_u_chat~n~poruku_koju_zelite~n~poslati_odabranom_broju");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][0], 0.113333, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][0], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][0], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][0], 1);

	TEXTDRAW_SMS[playerid][1] = CreatePlayerTextDraw(playerid, 559.666870, 238.118499, "06060606");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][1], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][1], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][1], 1);

	TEXTDRAW_SMS[playerid][2] = CreatePlayerTextDraw(playerid, 560.666625, 324.400085, "ukoliko_zelite_prekinuti~n~slanje_poruke_napisite~n~'sms_exit'");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][2], 0.113333, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][2], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][2], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][2], 1);
	return 1;
}

// TEXTDRAW_SMS[playerid][1] da bude clickable i da mozes upisati broj