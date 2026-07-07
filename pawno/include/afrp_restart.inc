/*
	afrp_restart.inc - Compte a rebours avant le redemarrage quotidien

	Le serveur redemarre chaque jour a 02h00 GMT (via l'hebergeur). Ce module
	NE redemarre PAS lui-meme : il previent les joueurs a l'avance avec des
	annonces chat aux paliers (30/15/10/5/2/1 min, 30s) puis un compte a
	rebours visuel (GameText) sur les 10 dernieres secondes.

	IMPORTANT : gettime() renvoie l'heure LOCALE de la machine serveur. Si ton
	hebergeur est en UTC/GMT (cas le plus courant), RESTART_HOUR=2 = 02h GMT.
	Sinon, ajuste RESTART_HOUR a l'heure LOCALE du serveur correspondant a 2h GMT.
*/

#if defined AFRP_RESTART_INCLUDED
	#endinput
#endif
#define AFRP_RESTART_INCLUDED

#define RESTART_HOUR   2   // heure du redemarrage (serveur/GMT)
#define RESTART_MIN    0

new restartLastLeft = 999999; // secondes restantes au tick precedent (detection de passage)

forward Restart_Tick();

stock Restart_Init()
{
	SetTimer("Restart_Tick", 1000, true);
	return 1;
}

stock Restart_Announce(secondsLeft)
{
	new msg[144];
	if(secondsLeft >= 60)
	{
		new mins = secondsLeft / 60;
		format(msg, sizeof(msg),
			"{FF4444}>> REDEMARRAGE <<{FFFFFF} Le serveur redemarre dans {FFFF00}%d minute%s{FFFFFF}. Mettez-vous en lieu sur (RP).",
			mins, (mins > 1) ? ("s") : (""));
	}
	else
	{
		format(msg, sizeof(msg),
			"{FF4444}>> REDEMARRAGE <<{FFFFFF} Le serveur redemarre dans {FFFF00}%d secondes{FFFFFF} !",
			secondsLeft);
	}
	SendClientMessageToAll(0xFF4444FF, msg);
	// Palier important -> aussi en gros a l'ecran
	if(secondsLeft == 300 || secondsLeft == 60 || secondsLeft == 30)
	{
		for(new p = 0; p < MAX_PLAYERS; p++)
		{
			if(IsPlayerConnected(p) && !IsPlayerNPC(p))
				GameTextForPlayer(p, "~r~Redemarrage bientot", 3000, 3);
		}
	}
	return 1;
}

public Restart_Tick()
{
	new h, m, s;
	gettime(h, m, s);
	new nowSec = h * 3600 + m * 60 + s;
	new target = RESTART_HOUR * 3600 + RESTART_MIN * 60;
	new left = target - nowSec;
	if(left <= 0) left += 86400; // le redemarrage est demain

	// Annonces chat aux paliers, par detection de passage (robuste aux sauts de tick)
	static const miles[] = {1800, 900, 600, 300, 120, 60, 30};
	for(new i = 0; i < sizeof(miles); i++)
	{
		new M = miles[i];
		if(restartLastLeft > M && left <= M)
			Restart_Announce(M);
	}

	// Compte a rebours visuel final (10..1)
	if(left <= 10 && left >= 1 && left != restartLastLeft)
	{
		new gt[48];
		format(gt, sizeof(gt), "~r~REDEMARRAGE~n~~w~%d", left);
		for(new p = 0; p < MAX_PLAYERS; p++)
		{
			if(IsPlayerConnected(p) && !IsPlayerNPC(p))
				GameTextForPlayer(p, gt, 1200, 3);
		}
	}

	restartLastLeft = left;
	return 1;
}
