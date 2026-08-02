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
			for(new td = 0; td < 18; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_HOME[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 18; td++)
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
	// [BUDGET TEXTDRAWS] Appele a l'OUVERTURE du telephone, plus au login.
	// Les 165 elements des apps ne servent que telephone allume : les garder
	// en permanence bouffait les 2/3 des 256 textdraws qu'un client SA-MP
	// accepte, ce qui empechait le sac, le craft ou le HUD de s'afficher.
	// Le groupe NOTIFICATION est exclu : lui doit rester resident (il sert
	// telephone ferme, cf. CreateNotification).
	if(TEXTDRAW_DEFAULT[playerid][0] != PlayerText:INVALID_TEXT_DRAW) return 1;

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

    // Fond d'ecran + cadre : les TextDraws viennent d'etre recreees a neuf,
    // il faut donc reappliquer le theme sauvegarde en DB.
    new foo[80];
    mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `users` WHERE `Username` = '%e'", GetName(playerid));
    mysql_tquery(db_handle, foo, "SQLLoadPhone", "d", playerid);
	return 1;
}

// Detruit tout sauf le groupe NOTIFICATION (4 elements) qui doit survivre
// pour afficher les appels / SMS / virements recus telephone range.
forward DestroyTextDraws(playerid);
public DestroyTextDraws(playerid)
{
	new td;
	for(td = 0; td < sizeof(TEXTDRAW_DEFAULT[]);   td++) if(TEXTDRAW_DEFAULT[playerid][td]   != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_DEFAULT[playerid][td]);   TEXTDRAW_DEFAULT[playerid][td]   = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_BANK[]);      td++) if(TEXTDRAW_BANK[playerid][td]      != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_BANK[playerid][td]);      TEXTDRAW_BANK[playerid][td]      = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_CALLDIAL[]);  td++) if(TEXTDRAW_CALLDIAL[playerid][td]  != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_CALLDIAL[playerid][td]);  TEXTDRAW_CALLDIAL[playerid][td]  = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_CALLLIST[]);  td++) if(TEXTDRAW_CALLLIST[playerid][td]  != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_CALLLIST[playerid][td]);  TEXTDRAW_CALLLIST[playerid][td]  = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_CALLING[]);   td++) if(TEXTDRAW_CALLING[playerid][td]   != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_CALLING[playerid][td]);   TEXTDRAW_CALLING[playerid][td]   = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_HOME[]);      td++) if(TEXTDRAW_HOME[playerid][td]      != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_HOME[playerid][td]);      TEXTDRAW_HOME[playerid][td]      = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_NOTES[]);     td++) if(TEXTDRAW_NOTES[playerid][td]     != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_NOTES[playerid][td]);     TEXTDRAW_NOTES[playerid][td]     = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_NOTESLIST[]); td++) if(TEXTDRAW_NOTESLIST[playerid][td] != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_NOTESLIST[playerid][td]); TEXTDRAW_NOTESLIST[playerid][td] = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_SMS[]);       td++) if(TEXTDRAW_SMS[playerid][td]       != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_SMS[playerid][td]);       TEXTDRAW_SMS[playerid][td]       = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_TIME[]);      td++) if(TEXTDRAW_TIME[playerid][td]      != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_TIME[playerid][td]);      TEXTDRAW_TIME[playerid][td]      = PlayerText:INVALID_TEXT_DRAW; }
	for(td = 0; td < sizeof(TEXTDRAW_TWITTER[]);   td++) if(TEXTDRAW_TWITTER[playerid][td]   != PlayerText:INVALID_TEXT_DRAW) { PlayerTextDrawDestroy(playerid, TEXTDRAW_TWITTER[playerid][td]);   TEXTDRAW_TWITTER[playerid][td]   = PlayerText:INVALID_TEXT_DRAW; }
	return 1;
}

forward CreateShop();
public CreateShop()
{
	for(new ID = 0; ID < sizeof(marketCoordinates); ID++)
	{
		// Pickup : modele 1247 (telephone) au sol pour ramassage
		marketPickupID[ID] = CreatePickup(1247, 1, marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2], -1);

		// 3D label flottant au-dessus
		Create3DTextLabel("{34C759}[TELEPHONE SHOP]{FFFFFF}\nTapez {FFFF00}/market{FFFFFF} ou {FFFF00}/shop\npour acheter / recharger",
			0x33CCFFAA, marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2] + 0.5, 20.0, 0, 1);

		// Icone sur la map/radar : icone 52 (shop/ammunation) - ressemble plus a une boutique
		// pour acheter un telephone que l'icone 6 (combine seul).
		CreateDynamicMapIcon(marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2],
			52, 0, -1, -1, -1, 300.0, MAPICON_GLOBAL);
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
	// Ce groupe reste resident telephone ferme : c'est lui qui previent d'un
	// appel / SMS / virement. S'il manque, la notification serait perdue.
	if(TEXTDRAW_NOTIFICATION[playerid][0] == PlayerText:INVALID_TEXT_DRAW) CreateNotificationTD(playerid);

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

		// [BUDGET TEXTDRAWS] Plus de creation au login : /mobile s'en charge.
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