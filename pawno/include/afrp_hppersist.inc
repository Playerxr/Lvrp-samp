/*
	afrp_hppersist.inc - Persistance de la vie entre les sessions

	Avant : un joueur blesse pouvait se deconnecter puis se reconnecter avec
	100 HP -> il "soignait" gratuitement en se relog, rendant l'hopital inutile.

	Maintenant : la vie (et l'armure) reelles sont sauvees a la deconnexion et
	restaurees exactement au login. Un joueur a 30 HP se reconnecte a 30 HP -> il
	DOIT se faire soigner a l'hopital (ou par un medecin).

	Le systeme de COMA est gere a part (colonne ComaTime) : on ne touche pas a la
	vie d'un joueur en coma, son etat est deja persistant.

	Colonnes utilisees : lvrp_users.Health et lvrp_users.Armour (deja ecrites par
	l'ancien anti-combat-log). Si elles n'existent pas sur une base vierge,
	decommente la migration dans OnGameModeInit (voir LVRP.pwn).
*/

#if defined AFRP_HPPERSIST_INCLUDED
	#endinput
#endif
#define AFRP_HPPERSIST_INCLUDED

new bool:hpRestorePending[MAX_PLAYERS];

stock HpPersist_OnConnect(playerid)
{
	hpRestorePending[playerid] = false;
	return 1;
}

// A appeler DANS le callback de login (cache MySQL encore actif), apres les
// autres cache_get_value_name. Lit Health/Armour et prepare la restauration.
stock HpPersist_LoadFromCache(playerid)
{
	new Float:hp = 100.0, Float:ar = 0.0;
	cache_get_value_name_float(0, "Health", hp);
	cache_get_value_name_float(0, "Armour", ar);
	// Valeur invalide/absente (colonne manquante -> 0) => defaut sur = 100
	if(hp < 1.0 || hp > 100.0) hp = 100.0;
	if(ar < 0.0 || ar > 100.0) ar = 0.0;
	PlayerInfo[playerid][pHealth] = hp;
	PlayerInfo[playerid][pArmour] = ar;
	// On ne restaure pas la vie d'un joueur en coma (le systeme coma s'en charge)
	if(PlayerInfo[playerid][pComaTime] <= 0)
		hpRestorePending[playerid] = true;
	return 1;
}

// A appeler au 1er spawn apres login. Renvoie 1 si la vie a ete restauree
// (dans ce cas, ne PAS forcer 100 ensuite), 0 sinon (respawn normal).
stock HpPersist_ApplyOnSpawn(playerid)
{
	if(!hpRestorePending[playerid]) return 0;
	hpRestorePending[playerid] = false;

	new Float:hp = PlayerInfo[playerid][pHealth];
	if(hp < 1.0 || hp > 100.0) hp = 100.0;
	SafeSetPlayerHealth(playerid, hp);

	new Float:ar = PlayerInfo[playerid][pArmour];
	if(ar > 0.0 && ar <= 100.0)
		SafeSetPlayerArmour(playerid, ar);

	if(hp < 100.0)
		SendClientMessage(playerid, 0xFF8282FF, "{FF8282}[Sante]{FFFFFF} Vous vous reconnectez blesse. Rendez-vous a l'hopital ou appelez un medecin ({FFFF00}/ambulance{FFFFFF}).");
	return 1;
}

// A appeler dans OnPlayerDisconnect. Sauve la vie/armure reelles (hors coma).
stock HpPersist_SaveOnDisconnect(playerid)
{
	if(gPlayerLogged[playerid] != 1) return 0;
	if(IsPlayerNPC(playerid)) return 0;
	// En coma -> gere par ComaTime, on ne sauve pas une vie a ~0
	if(PlayerInfo[playerid][pComaTime] > 0 || medic_PlayerNeedMedic[playerid] == 2) return 0;

	new Float:hp, Float:ar;
	GetPlayerHealth(playerid, hp);
	GetPlayerArmour(playerid, ar);
	if(hp < 0.0) hp = 0.0;
	if(hp > 100.0) hp = 100.0;
	if(ar < 0.0) ar = 0.0;
	if(ar > 100.0) ar = 100.0;
	PlayerInfo[playerid][pHealth] = hp;
	PlayerInfo[playerid][pArmour] = ar;

	new q[160];
	format(q, sizeof(q), "UPDATE lvrp_users SET Health=%.1f, Armour=%.1f WHERE id=%d",
		hp, ar, PlayerInfo[playerid][pSQLID]);
	mysql_tquery(MYSQL, q, "", "");
	return 1;
}
