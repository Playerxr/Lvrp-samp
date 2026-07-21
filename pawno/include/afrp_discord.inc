/*
	afrp_discord.inc - Annonces Discord SANS plugin (via relais HTTP -> HTTPS)

	SA-MP ne fournit qu'un client HTTP/1.0 (natif HTTP(), pas de HTTPS/TLS)
	alors que les webhooks Discord exigent du HTTPS. La solution "zero plugin"
	consiste a passer par un RELAIS : le serveur envoie le message en HTTP
	simple a un petit fichier PHP heberge chez toi, et ce PHP le transmet a
	Discord en HTTPS (avec le ping @everyone).

	=> Voir discord_relay.php (a la racine du depot) : a uploader sur ton
	   hebergement web, et a configurer avec l'URL de ton webhook Discord.

	Configuration cote serveur : scriptfiles/discord.cfg
	  Ligne 1 : URL du relais (ex: http://ton-site.com/discord_relay.php)
	  Ligne 2 : cle secrete partagee (doit etre identique dans le PHP)
	Si le fichier est absent ou l'URL vide, la fonctionnalite est
	silencieusement desactivee (aucune erreur, aucun spam).

	Envoi automatique : "serveur en ligne @everyone" au demarrage du gamemode.
*/

#if defined AFRP_DISCORD_INCLUDED
	#endinput
#endif
#define AFRP_DISCORD_INCLUDED

#define DISCORD_CFG_FILE   "discord.cfg"

new discord_RelayUrl[160];
new discord_Secret[64];
new bool:discord_Enabled = false;

stock Discord_Init()
{
	discord_RelayUrl[0] = 0;
	discord_Secret[0] = 0;
	discord_Enabled = false;

	if(!fexist(DISCORD_CFG_FILE))
	{
		printf("[Discord] discord.cfg absent : annonces Discord desactivees (optionnel).");
		return 0;
	}
	new File:f = fopen(DISCORD_CFG_FILE, io_read);
	if(!f) return 0;
	new line[160];
	// Ligne 1 : URL
	if(fread(f, line))
	{
		Discord_Trim(line);
		strcat(discord_RelayUrl, line, sizeof(discord_RelayUrl));
	}
	// Ligne 2 : secret
	if(fread(f, line))
	{
		Discord_Trim(line);
		strcat(discord_Secret, line, sizeof(discord_Secret));
	}
	fclose(f);

	if(strlen(discord_RelayUrl) > 7) // au moins "http://"
	{
		discord_Enabled = true;
		printf("[Discord] Relais configure, annonces Discord actives.");
	}
	else
	{
		printf("[Discord] URL de relais vide : annonces Discord desactivees.");
	}
	return 1;
}

stock Discord_Trim(string[])
{
	// Retire espaces / retours a la ligne en debut et fin
	new len = strlen(string);
	while(len > 0 && (string[len-1] == '\r' || string[len-1] == '\n' || string[len-1] == ' ' || string[len-1] == '\t'))
		{ string[len-1] = 0; len--; }
	new start = 0;
	while(string[start] == ' ' || string[start] == '\t') start++;
	if(start > 0)
	{
		new i = 0;
		while(string[start + i]) { string[i] = string[start + i]; i++; }
		string[i] = 0;
	}
	return 1;
}

// Encode une chaine pour une URL (les caracteres speciaux -> %XX).
stock Discord_UrlEncode(const src[], dest[], destSize)
{
	new di = 0;
	new hexc[] = "0123456789ABCDEF";
	for(new i = 0; src[i] && di < destSize - 4; i++)
	{
		new c = src[i];
		if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
		   c == '-' || c == '_' || c == '.' || c == '~')
		{
			dest[di++] = c;
		}
		else if(c == ' ')
		{
			dest[di++] = '+';
		}
		else
		{
			dest[di++] = '%';
			dest[di++] = hexc[(c >> 4) & 0x0F];
			dest[di++] = hexc[c & 0x0F];
		}
	}
	dest[di] = 0;
	return di;
}

// Envoie un message texte a Discord via le relais.
// mentionEveryone = 1 pour pinguer @everyone.
stock Discord_Send(const message[], mentionEveryone = 0)
{
	if(!discord_Enabled) return 0;

	new encMsg[400], encSecret[128];
	Discord_UrlEncode(message, encMsg, sizeof(encMsg));
	Discord_UrlEncode(discord_Secret, encSecret, sizeof(encSecret));

	new dcUrl[640];
	format(dcUrl, sizeof(dcUrl), "%s?key=%s&everyone=%d&msg=%s",
		discord_RelayUrl, encSecret, mentionEveryone, encMsg);

	HTTP(0, HTTP_GET, dcUrl, "", "Discord_OnResponse");
	return 1;
}

// Annonce le serveur en ligne avec @everyone (appelee au demarrage).
stock Discord_AnnonceOnline()
{
	if(!discord_Enabled) return 0;
	Discord_Send("Le serveur La Vie RolePlay est EN LIGNE ! Venez jouer des maintenant.", 1);
	return 1;
}

forward Discord_OnResponse(index, response_code, data[]);
public Discord_OnResponse(index, response_code, data[])
{
	// 2xx = OK. On logge juste pour diagnostic, pas d'action necessaire.
	#pragma unused index
	#pragma unused data
	if(response_code >= 200 && response_code < 300)
		printf("[Discord] Annonce transmise (code %d).", response_code);
	else
		printf("[Discord] Echec annonce (code %d). Verifie l'URL du relais / la cle / que le relais est en HTTP accessible.", response_code);
	return 1;
}
