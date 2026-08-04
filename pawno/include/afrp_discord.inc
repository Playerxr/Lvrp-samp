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
	  Ligne 1 : adresse du relais, AVEC OU SANS http:// (les deux marchent
	            depuis le fix ci-dessous ; en interne le prefixe est retire
	            car le natif HTTP() de SA-MP veut un hote nu)
	            ex: ton-site.com/discord_relay.php
	  Ligne 2 : cle secrete partagee (doit etre identique dans le PHP)

	/!\ Le relais doit repondre en HTTP SIMPLE (port 80), sans redirection
	    automatique vers HTTPS : SA-MP ne sait pas parler TLS, une redirection
	    301/302 vers https:// fait echouer l'envoi.
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

	// [FIX] Retire http:// ou https:// AVANT tout le reste : voir
	// Discord_StripScheme ci-dessous, c'est la cause de "Echec annonce (code 1)".
	new schemeRetire = Discord_StripScheme(discord_RelayUrl);

	if(strlen(discord_RelayUrl) > 4) // au moins "a.fr"
	{
		discord_Enabled = true;
		printf("[Discord] Relais configure (%s), annonces Discord actives.", discord_RelayUrl);
		if(schemeRetire == 8)
			printf("[Discord] ATTENTION : discord.cfg indique https://. SA-MP ne sait pas faire de HTTPS : la requete partira en HTTP simple (port 80). Si ton hebergement ne repond qu'en HTTPS, le relais doit etre joignable en http:// sans redirection.");
	}
	else
	{
		printf("[Discord] URL de relais vide : annonces Discord desactivees.");
	}
	return 1;
}

// [FIX ANNONCE JAMAIS PARTIE] Le natif HTTP() de SA-MP veut un hote NU
// ("mon-site.com/relay.php"), PAS une URL complete : lui passer
// "http://mon-site.com/..." echoue avec HTTP_ERROR_BAD_HOST (code 1). C'est
// exactement ce que faisait ce module, parce que sa propre doc de config
// demandait de mettre le http:// dans discord.cfg - l'annonce "serveur en
// ligne" n'est donc JAMAIS partie depuis l'installation du relais, et le log
// le repetait a chaque demarrage. On accepte les deux formes et on retire le
// prefixe ici, pour ne pas avoir a corriger les discord.cfg deja ecrits.
// (Reference : l'appel HTTP() de FCNPC.inc, qui passe bien un hote nu.)
stock Discord_StripScheme(string[])
{
	new decale = 0;
	if(strfind(string, "https://", true) == 0)      decale = 8;
	else if(strfind(string, "http://", true) == 0)  decale = 7;
	if(decale == 0) return 0;

	new i = 0;
	while(string[decale + i]) { string[i] = string[decale + i]; i++; }
	string[i] = 0;
	return decale;
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
	// Codes 1-6 = erreurs CLIENT de SA-MP (a_http.inc), pas des codes HTTP :
	// les distinguer evite de chercher un probleme cote Discord alors que la
	// requete n'est jamais sortie du serveur.
	else if(response_code == 1)
		printf("[Discord] Echec annonce : hote introuvable (BAD_HOST). Verifie l'adresse en ligne 1 de discord.cfg (nom de domaine correct, joignable en HTTP simple).");
	else if(response_code == 2 || response_code == 3)
		printf("[Discord] Echec annonce : connexion impossible au relais (code %d). Le site repond-il bien en http:// sans redirection vers https:// ?", response_code);
	else if(response_code >= 4 && response_code <= 6)
		printf("[Discord] Echec annonce : reponse du relais invalide (code %d). Le fichier discord_relay.php est-il bien en place et renvoie-t-il du texte simple ?", response_code);
	else
		printf("[Discord] Echec annonce (code HTTP %d). Verifie la cle secrete (ligne 2 de discord.cfg = $SECRET du PHP) et l'URL du webhook dans discord_relay.php.", response_code);
	return 1;
}
