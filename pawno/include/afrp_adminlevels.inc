/*
	afrp_adminlevels.inc - Niveau admin requis pour les commandes sensibles

	Certaines commandes /a (creer, donner) permettaient de creer des
	vehicules ou de donner de l'argent/armes/drogues des le niveau admin 3,
	ce qui a mene a des abus de confiance en l'absence du proprietaire.
	Le niveau minimum requis pour ces commandes est desormais configurable
	via /a niveausensible <niveau>, reserve aux admins niveau 5+, et
	persiste dans scriptfiles/admin_sensible.cfg (survit aux redemarrages).

	Utilisation dans le code : remplacer "PlayerInfo[playerid][pAdmin] >= 3"
	par "PlayerInfo[playerid][pAdmin] >= AdminSensitiveLevel" pour les
	commandes concernees.
*/

#if defined AFRP_ADMINLEVELS_INCLUDED
	#endinput
#endif
#define AFRP_ADMINLEVELS_INCLUDED

#define ADMINLEVEL_FILE     "admin_sensible.cfg"
#define ADMINLEVEL_DEFAULT  5
#define ADMINLEVEL_SETTER   5   // niveau requis pour CHANGER ce seuil (fixe, non configurable)

new AdminSensitiveLevel = ADMINLEVEL_DEFAULT;

stock AdminLevels_Init()
{
	if(!fexist(ADMINLEVEL_FILE))
	{
		AdminSensitiveLevel = ADMINLEVEL_DEFAULT;
		return 1;
	}
	new File:f = fopen(ADMINLEVEL_FILE, io_read);
	if(!f) return 0;
	new line[16];
	if(fread(f, line) && strlen(line) > 0)
	{
		new lvl = strval(line);
		if(lvl >= 1 && lvl <= 6) AdminSensitiveLevel = lvl;
	}
	fclose(f);
	return 1;
}

stock AdminLevels_Save()
{
	if(fexist(ADMINLEVEL_FILE)) fremove(ADMINLEVEL_FILE);
	new File:f = fopen(ADMINLEVEL_FILE, io_write);
	if(!f) return 0;
	new line[16];
	format(line, sizeof(line), "%d", AdminSensitiveLevel);
	fwrite(f, line);
	fclose(f);
	return 1;
}

// /a niveausensible <niveau> : ajuste le niveau requis pour /a creer et
// /a donner. Reserve aux admins niveau ADMINLEVEL_SETTER (5) fixe, pour
// qu'un admin de niveau 3/4 ne puisse jamais s'auto-accorder l'acces.
stock AdminLevels_Cmd(playerid, params[])
{
	if(PlayerInfo[playerid][pAdmin] < ADMINLEVEL_SETTER)
		return msg_Client(playerid, COLOR_NOACCES, "{FF0069}Reserve aux admins niveau 5+.");
	if(!strlen(params))
	{
		new msg[128];
		format(msg, sizeof(msg), "{A98500} Usage {FFFFB2} /a niveausensible <1-6> {FFFFFF}(actuel : niveau %d)", AdminSensitiveLevel);
		return msg_Client(playerid, COLOR_WHITE, msg);
	}
	new lvl = strval(params);
	if(lvl < 1 || lvl > 6)
		return msg_Client(playerid, COLOR_INFO, "{CF9756} Info {FFFFFF} Niveau invalide (1-6).");
	AdminSensitiveLevel = lvl;
	AdminLevels_Save();
	new confirm[128];
	format(confirm, sizeof(confirm), "{9EC73D}[Admin]{FFFFFF} Niveau requis pour /a creer et /a donner : %d.", lvl);
	SendClientMessageToAll(0x9EC73DFF, confirm);
	return 1;
}
