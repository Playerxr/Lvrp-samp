/*
	mapedit_dbcompat.inc - Wrappers DB manquants pour Map Editor V3

	Map Editor V3 (simonseidel) est ecrit contre "sampctl/samp-stdlib", un
	fork de a_samp.inc qui ajoute des wrappers types autour des natifs DB
	de base (db_get_field / db_get_field_assoc, qui ne renvoient que des
	chaines). Notre a_sampdb.inc est la version vanilla SA-MP et ne les
	fournit pas, d'ou les erreurs "undefined symbol" a la compilation.
	Ce fichier fournit des equivalents fonctionnels a base des natifs deja
	presents, sans avoir a remplacer tout le stdlib.
*/

#if defined MAPEDIT_DBCOMPAT_INCLUDED
	#endinput
#endif
#define MAPEDIT_DBCOMPAT_INCLUDED

stock bool:isempty(const string[])
{
	for(new i = 0; string[i]; i++)
	{
		if(string[i] > ' ') return false;
	}
	return true;
}

stock db_get_field_int(DBResult:dbresult, field = 0)
{
	new str[32];
	db_get_field(dbresult, field, str, sizeof(str));
	return strval(str);
}

stock db_get_field_assoc_int(DBResult:dbresult, const field[])
{
	new str[32];
	db_get_field_assoc(dbresult, field, str, sizeof(str));
	return strval(str);
}

stock Float:db_get_field_assoc_float(DBResult:dbresult, const field[])
{
	new str[32];
	db_get_field_assoc(dbresult, field, str, sizeof(str));
	return floatstr(str);
}

stock bool:mapedit_CharIn(c, const chars[])
{
	for(new i = 0; chars[i]; i++)
	{
		if(chars[i] == c) return true;
	}
	return false;
}

// Enleve les caracteres de "chars" (espace par defaut) au debut/fin de "string"
stock strtrim(string[], const chars[] = " \t\r\n")
{
	new len = strlen(string);
	new start = 0;
	while(start < len && mapedit_CharIn(string[start], chars)) start++;
	new last = len - 1;
	while(last >= start && mapedit_CharIn(string[last], chars)) last--;
	new newlen = last - start + 1;
	if(newlen < 0) newlen = 0;
	if(start > 0)
	{
		for(new i = 0; i < newlen; i++) string[i] = string[start + i];
	}
	string[newlen] = 0;
	return newlen;
}

stock ret_GetPlayerName(playerid)
{
	new name[MAX_PLAYER_NAME];
	GetPlayerName(playerid, name, sizeof(name));
	return name;
}
