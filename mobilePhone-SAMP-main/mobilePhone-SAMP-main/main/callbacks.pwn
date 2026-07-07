/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			main/callbacks.pwn
*/

public OnFilterScriptInit()
{
	MySQL_Init();
	#if DEBUG == 1
	printf(""MOBILE" | "COUNT"", tdCount);
    #endif

    CreateShop();
    return 1;
}

public OnFilterScriptExit()
{
    MySQL_Exit();
    return 1;
}

public OnPlayerConnect(playerid)
{
    new foo[80];
    mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `users` WHERE `Username` = '%e'", GetName(playerid));
    mysql_tquery(db_handle, foo, "SQLLoadUser", "d", playerid);
    mysql_tquery(db_handle, foo, "SQLLoadPhone", "d", playerid);

    CreateNotificationTD(playerid);
	return 1;
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
	// Notes
    if(playertextid == TEXTDRAW_HOME[playerid][0]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, NOTES, SHOW, NOTESLISTTD);
    }
    // SMS
    if(playertextid == TEXTDRAW_HOME[playerid][1]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, BANK, SHOW);
    }
    // SMS
    if(playertextid == TEXTDRAW_HOME[playerid][2]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, SMS, SHOW);
    }
    // Dial
    if(playertextid == TEXTDRAW_HOME[playerid][3]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, CALL, SHOW, CALLDIAL);
    }
    // Clock
    if(playertextid == TEXTDRAW_HOME[playerid][4]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, TIME, SHOW);
    }
    // Twitter
    if(playertextid == TEXTDRAW_HOME[playerid][5]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, TWITTER, SHOW);
    }

    // Putting away phone
    if(playertextid == TEXTDRAW_DEFAULT[playerid][12]) 
    {
        HidePhone(playerid);
        CancelSelectTextDraw(playerid);
        UseMobile(playerid, NOAPPS, HIDE);
        usingPhone[playerid] = false;
    }
    // One step back
    if(playertextid == TEXTDRAW_DEFAULT[playerid][13]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, HOME, SHOW);
        UseMobile(playerid, NOAPPS, SHOW);
    }
    // Back to homepage
    if(playertextid == TEXTDRAW_DEFAULT[playerid][14]) 
    {
        HidePhone(playerid);
        UseMobile(playerid, HOME, SHOW);
        UseMobile(playerid, NOAPPS, SHOW);
    }

    // Tweet button
    if(playertextid == TEXTDRAW_TWITTER[playerid][5])
    {
    	if(gettime() < twitterDelay) return SendClientMessage(playerid, -1, "Mora proci 30 sekundi izmedu objavljivanje tweetova.");
    	writingTweet[playerid] = 1;
    	CancelSelectTextDraw(playerid);

    	SendClientMessage(playerid, -1, " ");
    	SendClientMessage(playerid, -1, "Napisite tweet u chat koji zelite objaviti - (tweet_exit ukoliko zelite prekinuti radnju)");
    }
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case MARKET_DIALOG:
        {
            if(response)
            {
                switch(listitem)
                {
                    case 0:
                    {
                    	if(hasPhone[playerid] == 1) return SendPlayerNotification(playerid, -1, HAS_PHONE);

                    	playerNumber[playerid] = 100000 + random(899000);
                    	playerCredit[playerid] = 100;
                        hasPhone[playerid] = 1;

                        SendClientMessage(playerid, -1, ""SUCCESS"");
                        SendPlayerNotification(playerid, -1, READY_TO_USE);
                        CreateTextDraws(playerid);

                        new foo[256];
				        mysql_format(db_handle, foo, sizeof(foo), 
				        			"INSERT INTO `users` (`Username`, `HasPhone`, `Number`, `Credit`, `Frame`, `Background`) VALUES ('%e', %d, %d, %d, 0, 0)", 
				        			GetName(playerid), hasPhone[playerid], playerNumber[playerid], playerCredit[playerid]);
				        mysql_tquery(db_handle, foo);
                    }

                    case 1..4:
                    {

                    	if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE);
                    	if(playerCredit[playerid] > 1000) return SendClientMessage(playerid, -1, CREDIT_MAX);

                    	switch(listitem)
                    	{
                    		case 1:
                    			playerCredit[playerid] += 10;
                    		case 2:
                    			playerCredit[playerid] += 20;
                    		case 3:
                    			playerCredit[playerid] += 50;
                    		case 4:
                    			playerCredit[playerid] += 100;
                    	}

                    	new foo[128];
				        mysql_format(db_handle, foo, sizeof(foo), 
				        			"UPDATE `users` SET `Credit` = %d WHERE `Username` = '%e'", 
				        			playerCredit[playerid], GetName(playerid));
				        mysql_tquery(db_handle, foo);

				        format(foo, sizeof(foo), CREDIT_MARKET, playerCredit[playerid]);
				        SendClientMessage(playerid, -1, foo);
                    }
                }
            }
            else OnPlayerEnterShop(playerid, EXITING);
        }

        case SETTINGS_DIALOG:
        {
        	if(response)
        	{
        		switch(listitem)
        		{
        			case 0..2:
        				ChangeBackground(playerid, listitem);
        			case 3..9:
        				ChangeFrame(playerid, listitem-3);
        		}

        		HidePhone(playerid);
		        UseMobile(playerid, HOME, SHOW);
		        UseMobile(playerid, NOAPPS, SHOW);
        		SelectTextDraw(playerid, SELECTION_COLOR);
        	}
        }
    }
    return 1;
}

public OnPlayerText(playerid, text[])
{
	if(playerOccupied[playerid] == 1)
	{
		if(strfind(text, "sms_exit", true) != -1) 
			return SendClientMessage(playerid, -1, "Odustali ste od pisanja SMSa"), playerOccupied[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR);

		// Zavrsit

		return 0;
	}

	if(writingTweet[playerid] == 1)
	{
		if(strfind(text, "tweet_exit", true) != -1) 
			return SendClientMessage(playerid, -1, "Odustali ste od pisanja tweeta."), writingTweet[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR);

		if(strlen(text) > 92) 
			return SendClientMessage(playerid, -1, "Tekst je predug, pokusajte ponovo."), writingTweet[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR);

		for(new i = 0; i < strlen(text); i++)
		{
			if(text[i] == ' ') text[i] = '_';
		}

		new string[92];
		format(string, 92, "%s", text);

		if(strlen(string) > 21) strins(string, "~n~", 21, 92);
		if(strlen(string) > 45) strins(string, "~n~", 45, 92);
		if(strlen(string) > 68) strins(string, "~n~", 68, 92);

		new foo[160];
	    mysql_format(db_handle, foo, sizeof(foo), 
	    			"UPDATE `twitter` SET `TweetString` = '%s', `User` = '%e', `Time` = NOW() WHERE `TweetID` = %d", 
	    			string, GetName(playerid), tweetID);
	    mysql_tquery(db_handle, foo);

	    PlayerTextDrawSetString(playerid, TEXTDRAW_TWITTER[playerid][9+tweetID], string);

	    tweetID++;
	    if(tweetID > 3) tweetID = 1;

	    SendClientMessage(playerid, -1, " ");
    	SendClientMessage(playerid, -1, "Zavrsili ste sa pisanjem tweeta.");

	    writingTweet[playerid] = 0;
	    twitterDelay = gettime() + 30;

	    SelectTextDraw(playerid, SELECTION_COLOR);
	    return 0;
	}
	return 1;
}