/*
	mysql/connection.pwn - adapte pour LVRP inline

	On utilise le handle MYSQL deja ouvert par LVRP (gamemodes/LVRP.pwn:1424
	'new MySQL:MYSQL;' + MySQLConnect dans OnGameModeInit).
	Plus de db_handle separe, plus de connexion redondante, plus de fuite de log.
*/

#define db_handle MYSQL

// Stubs vides : LVRP gere la connexion. Garde les noms pour compat code.
stock MySQL_Init() { return 1; }
stock MySQL_Exit() { return 1; }
