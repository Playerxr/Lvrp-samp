/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			main/commands.pwn
*/

CMD:mobile(playerid)
{
	if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE);
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);

	if(usingPhone[playerid] == 0)
	{
		SelectTextDraw(playerid, SELECTION_COLOR);
		UseMobile(playerid, HOME, SHOW);
		UseMobile(playerid, NOAPPS, SHOW);
		UpdateTimeDate(playerid, 2);

		usingPhone[playerid] = 1;
	} 
	else
	{
		CancelSelectTextDraw(playerid);
		HidePhone(playerid);
		ShowNoApps(playerid, HIDE);

		usingPhone[playerid] = 0;
	}
	return 1;
}

alias:mobile("mobitel")

CMD:shop(playerid)
{
	if(!IsPlayerNearMarket(playerid)) return SendPlayerNotification(playerid, -1, NOT_IN_MARKET);
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);

	OnPlayerEnterShop(playerid, BUYING);
	return 1;
}

alias:shop("market")

CMD:mobilesettings(playerid)
{
	if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE);
	if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, TWEET_ERROR);
	if(usingPhone[playerid] == 0) return SendClientMessage(playerid, -1, "Morate imati upaljen telefon.");
	if(playerOccupied[playerid] == 1) return SendClientMessage(playerid, -1, "Morate biti van poziva.");

	ShowPlayerDialog(playerid, SETTINGS_DIALOG, DIALOG_STYLE_LIST, "Settings", "Pozadina - Zvjezdano nebo\nPozadina - 2-Color\nPozadina - Nebo\n\
																				Okvir - Default\nOkvir - Plavi\nOkvir - Crveni\nOkvir - Zeleni\nOkvir - Zuti\nOkvir - Ljubicasti\nOkvir - Rozi", "Odaberi", CANCEL);
	return 1;
}

//
#if DEBUG == 1
new test_id=0;
CMD:test(playerid)
{
	switch(test_id)
    {
    	case 0:
    		SetPlayerPos(playerid, 2284.8750, -1326.1179, 24.6223+5), test_id = 1;
    	case 1:
    		SetPlayerPos(playerid, 1152.3308, -1657.2321, 13.9058+5), test_id = 2;
    	default:
    		SetPlayerPos(playerid, 1340.5618, -1318.0380, 14.0+5), test_id = 0;
    }
}

CMD:notif(playerid)
{
	playerOccupied[playerid] = 1;
	return 1;
}

CMD:bank(playerid)
{
	switch(test_id)
    {
    	case 0: // Pad
	    	PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][19]),
			PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][20]),
			PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][11]),
			PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][14]), test_id = 1;
		case 1: // Rast
			PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][19]),
			PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][20]),
			PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][11]),
			PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][14]), test_id = 0;
	}
	return 1;
}
#endif