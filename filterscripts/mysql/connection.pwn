/*
	                  __ __   _       _ 
	                 /_ /_ | | |     | |
	 __   _____  _ __ | || | | | ___ | |
	 \ \ / / _ \| '_ \| || | | |/ _ \| |
	  \ V / (_) | | | | || |_| | (_) | |
	   \_/ \___/|_| |_|_||_(_)_|\___/|_|

			mysql/connection.pwn
*/

new MySQL:db_handle;

stock MySQL_Init()
{
	// [FIX MEMOIRE] mysql_log(ALL) logue chaque requete -> 200MB en 2min.
	// On garde seulement les erreurs et warnings reels.
	mysql_log(ERROR | WARNING);
	db_handle = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB);
	if(mysql_errno(db_handle) != 0)
    {
        printf("MySQL | "NO_CONNECTION" (%d).", mysql_errno(db_handle));
        SendRconCommand("exit");
    }
    else
    {
    	#if DEBUG == 1
        printf("MySQL | "CONNECTED" (%d).", _:db_handle);
        #endif
    }
	return 1;
}

stock MySQL_Exit()
{
    if(db_handle)
    {
        mysql_close(db_handle);
    }
    return 1;
}