/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			main/functions.pwn
*/

forward ShowCall(playerid, type, status);
public ShowCall(playerid, type, status)
{
	switch(type)
	{
		case CALLDIAL:
			ShowCallDialTD(playerid, status);
		case CALLLIST:
			ShowCallListTD(playerid, status);
		case CALLING:
			ShowCallingTD(playerid, status);
		default: 
			return 0;
	}
	return 1;
}

forward ShowBank(playerid, status);
public ShowBank(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 21; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 19; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowHome(playerid, status);
public ShowHome(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 12; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_HOME[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 12; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_HOME[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowNoApps(playerid, status);
public ShowNoApps(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 19; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_DEFAULT[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 19; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_DEFAULT[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowNotes(playerid, type, status);
public ShowNotes(playerid, type, status)
{
	switch(type)
	{
		case NOTESTD:
			ShowNotesTD(playerid, status);
		case NOTESLISTTD:
			ShowNotesListTD(playerid, status);
		default: 
			return 0;
	}
	return 1;
}

forward ShowSMS(playerid, status);
public ShowSMS(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 3; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_SMS[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 3; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_SMS[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowTime(playerid, status);
public ShowTime(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 5; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_TIME[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 5; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_TIME[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowTwitter(playerid, status);
public ShowTwitter(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 16; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_TWITTER[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 16; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_TWITTER[playerid][td]);
			}
			
			new foo[160];
		    mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `twitter`");
		    mysql_tquery(db_handle, foo, "SQLLoadTwitter", "d", playerid);
		}
	}
	return 1;
}

forward ShowNotification(playerid, status);
public ShowNotification(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 4; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_NOTIFICATION[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 4; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_NOTIFICATION[playerid][td]);
			}
		}
	}
	return 1;
}

forward HidePhone(playerid);
public HidePhone(playerid)
{
	new foo[80];
    mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `users` WHERE `Username` = '%s'", GetName(playerid));
    mysql_tquery(db_handle, foo, "SQLLoadPhone", "d", playerid);
	
	ShowBank(playerid, HIDE);
	ShowCall(playerid, CALLLIST, HIDE);
	ShowCall(playerid, CALLING, HIDE);
	ShowCall(playerid, CALLDIAL, HIDE);
	ShowHome(playerid, HIDE);
	ShowNotes(playerid, NOTESTD, HIDE);
	ShowNotes(playerid, NOTESLISTTD, HIDE);
	ShowSMS(playerid, HIDE);
	ShowTime(playerid, HIDE);
	ShowTwitter(playerid, HIDE);
	UpdateTimeDate(playerid, 2);
	return 1;
}

forward CreateTextDraws(playerid);
public CreateTextDraws(playerid)
{
	CreatePhoneTD(playerid);
    CreateBankTD(playerid);
    CreateCallDialTD(playerid);
    CreateCallListTD(playerid);
    CreateCallingTD(playerid);
    CreateHomescreenTD(playerid);
    CreateNotesTD(playerid);
    CreateNotesListTD(playerid);
    CreateSMSTD(playerid);
    CreateTimeTD(playerid);
    CreateTwitterTD(playerid);
	return 1;
}

forward CreateShop();
public CreateShop()
{
	for(new ID = 0; ID < sizeof(marketCoordinates); ID++) 
	{
		marketPickupID[ID] = CreatePickup(1239, 1, marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2], -1);
		Create3DTextLabel(""SHOP" "SHOP_INFO"", 0x33CCFFAA, marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2], 25, 0, 1);
	}

	#if DEBUG == 1
	printf(""SHOP" | "CONSOLE_SHOP"", sizeof(marketCoordinates));
	#endif
	return 1;
}

forward OnPlayerEnterShop(playerid, code);
public OnPlayerEnterShop(playerid, code)
{
    switch(code)
    {
        case BUYING:
        {
            ShowPlayerDialog(playerid, MARKET_DIALOG, DIALOG_STYLE_LIST, SHOP, ""MOBILE"\n$10 "CREDIT"\n$20 "CREDIT"\n$50 "CREDIT"\n$100 "CREDIT"", BUY, CANCEL);
        }

        case EXITING:
        {
            SendClientMessage(playerid, -1, ""DIALOG_CLOSED"");
        }

        default:
            return 0;
    }
    return 1;
}

forward SendPlayerNotification(playerid, receiverid, type);
public SendPlayerNotification(playerid, receiverid, type)
{
	switch(type)
	{
		case BANK_PAYMENT:
		{
			CreateNotification(playerid, PAYMENT_RECEIVED, CHECK_PHONE, -1);
		}

		case CALL_RECEIVED:
		{
			new string[64];
			format(string, 64, CALL_NOTIFICATION, GetName(receiverid));
			CreateNotification(playerid, string, CHECK_PHONE, receiverid);
		}

		case UNAVAILABLE:
		{
			CreateNotification(playerid, UNAVAILABLE_TEXT, TRY_AGAIN, -1);
		}

		case SMS_RECEIVED:
		{
			new string[64];
			format(string, 64, SMS_TEXT_RECEIVED, GetName(receiverid));
			CreateNotification(playerid, string, CHECK_PHONE, receiverid);
		}

		case NEW_TWEET:
		{
			CreateNotification(playerid, TWEET_NOTIFICATION, CHECK_PHONE, -1);
		}

		case PHONE_CREDIT:
		{
			CreateNotification(playerid, CREDIT_RECEIVED, CHECK_PHONE, -1);
		}

		case READY_TO_USE:
		{
			CreateNotification(playerid, PHONE_READY, PHONE_COMMAND, -1);
		}

		case NO_PHONE:
		{
			CreateNotification(playerid, ERROR_PHONE, FORCE_BUY, -1);
		}

		case HAS_PHONE:
		{
			CreateNotification(playerid, HAS_PHONE_TEXT, PHONE_COMMAND, -1);
		}

		case NOT_IN_MARKET:
		{
			CreateNotification(playerid, MARKET_ERROR, TRY_AGAIN, -1);
		}
	}
	return 1;
}

forward CreateNotification(playerid, lineone[], linetwo[], receiverid);
public CreateNotification(playerid, lineone[], linetwo[], receiverid)
{
	new string[80];
	format(string, sizeof(string), "%s~n~%s", lineone, linetwo);
	PlayerTextDrawSetString(playerid, TEXTDRAW_NOTIFICATION[playerid][3], string);
	
	for(new i = 0; i < 4; i++) PlayerTextDrawShow(playerid, TEXTDRAW_NOTIFICATION[playerid][i]);

	SetTimerEx("HideNotification", 2000, false, "d", playerid);
	return 1;
}

forward UpdateTimeDate(playerid, type);
public UpdateTimeDate(playerid, type)
{
	switch(type)
	{
		case 1:
		{
			new Year, Month, Day;
			getdate(Year, Month, Day);

			new string[12];
			format(string, sizeof(string), "%02d/%02d/%d", Day, Month, Year);
			PlayerTextDrawSetString(playerid, TEXTDRAW_TIME[playerid][1], string);

			//

			new Hour, Minute, Second;
			gettime(Hour, Minute, Second);

			format(string, sizeof(string), "%02d:%02d:%02d", Hour, Minute, Second);
			PlayerTextDrawSetString(playerid, TEXTDRAW_TIME[playerid][2], string);
		}
		case 2:
		{
			new Hour, Minute;
			gettime(Hour, Minute);

			new string[10];
			format(string, sizeof(string), "%02d:%02d", Hour, Minute);
			PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][15], string);
		}
	}
	return 1;
}

forward ChangeBackground(playerid, background);
public ChangeBackground(playerid, background)
{
	switch(background)
	{
		case 0:
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_SHTR:bstars"),
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_SHTR:bstars");
    	case 1:
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_OTB2:backbet"),
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_OTB2:backbet");
    	case 2:
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_GRAV:sky"),
    		PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_GRAV:sky");
	}

	new foo[80];
    mysql_format(db_handle, foo, sizeof(foo), 
    			"UPDATE `users` SET `Background` = %d WHERE `Username` = '%s'", 
    			background, GetName(playerid));
    mysql_tquery(db_handle, foo);
	return 1;
}

forward ChangeFrame(playerid, frame);
public ChangeFrame(playerid, frame)
{
	switch(frame)
	{
		case 0:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], -1);
			}
		}
		case 1:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0x007BFFFF);
			}
		}
		case 2:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0xFF0000FF);
			}
		}
		case 3:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0x0FF702FF);
			}
		}
		case 4:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0xF8CE02FF);
			}
		}
		case 5:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0xAA02F7FF);
			}
		}
		case 6:
		{
			for(new i = 4; i < 12; i++)
			{
				PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], 0xF702BEFF);
			}
		}
	}

	new foo[80];
    mysql_format(db_handle, foo, sizeof(foo), 
    			"UPDATE `users` SET `Frame` = %d  WHERE `Username` = '%s'", 
    			frame, GetName(playerid));
    mysql_tquery(db_handle, foo);
	return 1;
}
//

forward SQLLoadUser(playerid);
public SQLLoadUser(playerid)
{
	new rows = cache_num_rows();

	if(rows)
	{
		cache_get_value_name_int(0, "HasPhone", hasPhone[playerid]);
		cache_get_value_name_int(0, "Number", playerNumber[playerid]);
		cache_get_value_name_int(0, "Credit", playerCredit[playerid]);

		if(hasPhone[playerid] == 1) CreateTextDraws(playerid);
	}
	return 1;
}

forward SQLLoadTwitter(playerid);
public SQLLoadTwitter(playerid)
{
	new rows = cache_num_rows(), string[92], tweet = 1;

	if(rows)
	{
		for(new i = 0; i < rows; i++) 
		{ 
            cache_get_value_name_int(i, "TweetID", tweet);
            cache_get_value_name(i, "TweetString", string);
        	
            PlayerTextDrawSetString(playerid, TEXTDRAW_TWITTER[playerid][9+tweet], string);
        }
	}
	return 1;
}

forward SQLLoadPhone(playerid);
public SQLLoadPhone(playerid)
{
	new rows = cache_num_rows(), frameID[MAX_PLAYERS], backgroundID[MAX_PLAYERS];

	if(rows)
	{
		cache_get_value_name_int(0, "Frame", frameID[playerid]);
		cache_get_value_name_int(0, "Background", backgroundID[playerid]);

		ChangeBackground(playerid, backgroundID[playerid]);
		ChangeFrame(playerid, frameID[playerid]);
	}
	return 1;
}

//

UseMobile(playerid, type, status, additional = 0)
{
	switch(type)
	{
		case BANK:
		{
			if(status == 0) ShowBank(playerid, HIDE);
			else ShowBank(playerid, SHOW);
		}
		case CALL:
		{
			if(status == 0) ShowCall(playerid, additional, HIDE);
			else ShowCall(playerid, additional, SHOW);
		}
		case HOME:
		{
			if(status == 0) ShowHome(playerid, HIDE);
			else ShowHome(playerid, SHOW);
		}
		case NOAPPS:
		{
			if(status == 0) ShowNoApps(playerid, HIDE);
			else ShowNoApps(playerid, SHOW);
		}
		case NOTES:
		{
			if(status == 0) ShowNotes(playerid, additional, HIDE);
			else ShowNotes(playerid, additional, SHOW);
		}
		case SMS:
		{
			if(status == 0) ShowSMS(playerid, HIDE);
			else ShowSMS(playerid, SHOW);
		}
		case TIME:
		{
			if(status == 0) ShowTime(playerid, HIDE);
			else ShowTime(playerid, SHOW), UpdateTimeDate(playerid, 1);
		}
		case TWITTER:
		{
			if(status == 0) ShowTwitter(playerid, HIDE);
			else ShowTwitter(playerid, SHOW);
		}
	}
}

IsPlayerNearMarket(playerid, status = 0)
{
	for(new ID = 0; ID < sizeof(marketCoordinates); ID++) 
	{
		if(IsPlayerInRangeOfPoint(playerid, 3.0, marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2])) status = 1;
	}
	return status;
}

GetName(playerid)
{
	new PlayerName[MAX_PLAYER_NAME]; 
	GetPlayerName(playerid, PlayerName, sizeof(PlayerName));
	return PlayerName;
}