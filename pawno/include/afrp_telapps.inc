/*
	afrp_telapps.inc - Les 8 applications du smartphone

	Le smartphone du serveur (vibe_phone) etait une belle coquille vide :
	ecran de verrouillage, barre d'etat, huit icones sur l'accueil, et derriere
	chacune un message "App 'X' (en developpement - P2-P4)" avant de revenir a
	l'accueil. Les commandes /tel /phone /telephone avaient donc ete
	desactivees, en renvoyant les joueurs vers /mobile.

	Ce module donne un contenu aux huit applications et rouvre le telephone.

	=== IL NE REECRIT AUCUN SYSTEME ===
	Chaque application appelle ce que le gamemode sait deja faire :
	  Telephone -> /call, qui gere deja le VOIP, la recherche par numero,
	               le credit et la verification du telephone
	  Contacts  -> le repertoire existant (PhoneContact_*)
	  Urgences  -> /911 /sos /feu /ambulance /mecano
	  Plan      -> /position, le partage de position deja en place
	  Banque    -> lit pCash et pAccount
	Seuls les SMS n'existaient nulle part : ils sont ecrits ici, avec le meme
	cout et les memes conditions qu'un appel.

	=== DEUX ICONES REAFFECTEES ===
	"Galerie" et "Agenda" ne pouvaient rien afficher d'utile : un serveur SA-MP
	n'a ni photos ni rendez-vous. Elles deviennent "Urgences" et "Infos". Garder
	deux icones decoratives sur huit aurait ete deux occasions de decevoir.

	=== TOUT PASSE PAR DES DIALOGUES ===
	Un client SA-MP n'accepte que 256 textdraws par joueur et ce gamemode en
	tient deja pres de 200. Dessiner huit ecrans en textdraws aurait demande
	une centaine d'elements de plus, et rien ne se serait affiche. Les
	applications utilisent donc des dialogues, qui ne coutent rien.

	Hooks a brancher dans LVRP.pwn :
	- phone_OpenApp     -> remplace par TelApp_Ouvrir(playerid, app)
	- phone_Close       -> phone_Destroy(playerid) pour liberer les 50 elements
	- OnDialogResponse  -> if(TelApp_OnDialog(playerid, dialogid, response, listitem, inputtext)) return 1;
	- /tel /phone /telephone -> reactivees vers phone_Open(playerid)
*/

#if defined AFRP_TELAPPS_INCLUDED
	#endinput
#endif
#define AFRP_TELAPPS_INCLUDED

#define DIALOG_TA_TEL      9641
#define DIALOG_TA_NUMERO   9642
#define DIALOG_TA_SMS      9643
#define DIALOG_TA_CONTACT  9644
#define DIALOG_TA_BANQUE   9645
#define DIALOG_TA_URGENCE  9646
#define DIALOG_TA_REGLAGES 9647
#define DIALOG_TA_PLAN     9648
#define DIALOG_TA_INFOS    9649

#define TA_COUT_SMS        10      // meme tarif qu'un appel

// Les six fonds d'ecran. vibe_phone a la meme table, mais declaree static :
// elle n'est pas visible depuis ce fichier, on la reprend telle quelle.
new taFonds[6] = {
	0x1C1C1EFF, 0x0A2540FF, 0x2D0A4FFF,
	0x3A2317FF, 0x0F3D2EFF, 0x4A0E1AFF
};

// ============================================================================
// OUTILS
// ============================================================================
stock TelApp_Msg(playerid, const text[])
{
	SendClientMessage(playerid, 0xFFFFFFFF, text);
	return 1;
}

// Retour a l'accueil du telephone apres une application.
stock TelApp_Accueil(playerid)
{
	if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
	return phone_ShowHome(playerid);
}

// Executer une commande existante plutot que recopier son code : /call fait
// deja 80 lignes de verifications qu'il serait absurde de dupliquer.
stock TelApp_Commande(playerid, const cmd[])
{
	return CallLocalFunction("OnPlayerCommandText", "is", playerid, cmd);
}

stock TelApp_JoueurParNumero(numero)
{
	if(numero <= 0) return -1;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		if(PlayerInfo[i][pNumber] == numero) return i;
	}
	return -1;
}

// ============================================================================
// TELEPHONE
// ============================================================================
stock TelApp_Tel(playerid)
{
	new body[400];
	format(body, sizeof(body), "{FFFFFF}Appeler un numero\n{FFFFFF}Mon repertoire\n{FF8888}Raccrocher\n{AAAAAA}Mon numero : %d", PlayerInfo[playerid][pNumber]);
	return ShowPlayerDialog(playerid, DIALOG_TA_TEL, DIALOG_STYLE_LIST, "Telephone", body, "Choisir", "Retour");
}

stock TelApp_DemanderNumero(playerid)
{
	return ShowPlayerDialog(playerid, DIALOG_TA_NUMERO, DIALOG_STYLE_INPUT, "Appeler",
		"{FFFFFF}Tape le numero a appeler.\n{AAAAAA}Un appel coute 10$ de credit.", "Appeler", "Retour");
}

// ============================================================================
// MESSAGES (SMS)
// ============================================================================
// Aucun systeme de SMS n'existait dans le gamemode. Il est ecrit ici avec les
// memes conditions qu'un appel : telephone requis des deux cotes, 10$, et le
// destinataire doit etre en ligne.
stock TelApp_Sms(playerid)
{
	return ShowPlayerDialog(playerid, DIALOG_TA_SMS, DIALOG_STYLE_INPUT, "Nouveau message",
		"{FFFFFF}Tape le numero, un espace, puis ton message.\n{AAAAAA}Exemple : 555123 salut, tu es ou ?\n{AAAAAA}Un SMS coute 10$ de credit.", "Envoyer", "Retour");
}

stock TelApp_EnvoyerSms(playerid, const texte[])
{
	if(hasPhone[playerid] == 0) return TelApp_Msg(playerid, "{FF4444}Tu n'as pas de telephone.");

	// Separation numero / message.
	new p = 0, buf[16];
	while(texte[p] == ' ') p++;
	new n = 0;
	while(texte[p] && texte[p] != ' ' && n < 15) buf[n++] = texte[p++];
	buf[n] = 0;
	while(texte[p] == ' ') p++;

	new numero = strval(buf);
	if(numero <= 0 || !strlen(texte[p]))
		return TelApp_Msg(playerid, "{FF4444}Format attendu : numero puis message. Exemple : 555123 salut.");

	if(PlayerInfo[playerid][pCash] < TA_COUT_SMS)
		return TelApp_Msg(playerid, "{FF4444}Pas assez de credit. 10$ minimum pour envoyer un SMS.");

	new cible = TelApp_JoueurParNumero(numero);
	if(cible == -1) return TelApp_Msg(playerid, "{FF8888}Ce numero n'est pas joignable.");
	if(cible == playerid) return TelApp_Msg(playerid, "{FF8888}Tu ne peux pas t'ecrire a toi-meme.");
	if(hasPhone[cible] == 0) return TelApp_Msg(playerid, "{FF8888}Le destinataire n'a pas de telephone.");

	// Le message peut etre long : on le tronque plutot que de deborder.
	new msg[144];
	format(msg, sizeof(msg), "%s", texte[p]);

	SafeGivePlayerMoney(playerid, -TA_COUT_SMS, "SMS");

	new s[220];
	format(s, sizeof(s), "{32D74B}SMS a %d{FFFFFF} : %s", numero, msg);
	TelApp_Msg(playerid, s);
	format(s, sizeof(s), "{32D74B}SMS de %d{FFFFFF} : %s", PlayerInfo[playerid][pNumber], msg);
	TelApp_Msg(cible, s);
	TelApp_Msg(cible, "{AAAAAA}Reponds avec /tel, application Messages.");
	PlayerPlaySound(cible, 1052, 0.0, 0.0, 0.0);
	return 1;
}

// ============================================================================
// CONTACTS
// ============================================================================
stock TelApp_Contacts(playerid)
{
	if(PhoneContact_Count[playerid] == 0)
	{
		TelApp_Msg(playerid, "{FF8888}Ton repertoire est vide.");
		return TelApp_Accueil(playerid);
	}
	new body[700], line[100];
	body[0] = 0;
	for(new i = 0; i < PhoneContact_Count[playerid]; i++)
	{
		format(line, sizeof(line), "{FFD700}%s{FFFFFF} - %d\n", PhoneContact_Name[playerid][i], PhoneContact_Num[playerid][i]);
		strcat(body, line);
	}
	return ShowPlayerDialog(playerid, DIALOG_TA_CONTACT, DIALOG_STYLE_LIST, "Repertoire - clique pour appeler", body, "Appeler", "Retour");
}

// ============================================================================
// BANQUE
// ============================================================================
// [LIEN REEL] Ouvre le dialogue 86, exactement celui du PNJ de banque (voir
// OnPlayerClickPlayer, actor[tmpId][variable] == 5) : aucune verification de
// proximite n'y est faite cote serveur, donc le rouvrir depuis le telephone
// est sans risque et evite un deuxieme systeme de banque qui divergerait du
// vrai (depot/retrait/virement/chequier au lieu d'un simple solde en lecture
// seule comme avant).
stock TelApp_Banque(playerid)
{
	phone_Close(playerid);
	player_Variable[playerid] = 0;
	new body[400];
	format(body, sizeof(body), "Infos sur le compte \nDeposer du liquide (MAX:$%d) \nRetirer du liquide (MAX:$%d)\nVirement Bancaire\nDemander un chequier", PlayerInfo[playerid][pCash], PlayerInfo[playerid][pAccount]);
	return ShowPlayerDialog(playerid, 86, DIALOG_STYLE_LIST, "{AADD66} Banque {FFFFFF} Operations disponibles", body, "Valider", "Quitter");
}

// ============================================================================
// URGENCES
// ============================================================================
stock TelApp_Urgences(playerid)
{
	new body[400];
	format(body, sizeof(body), "{007AFF}Police{FFFFFF} - 911\n{FF453A}Pompiers{FFFFFF} - les secours incendie\n{32D74B}Ambulance{FFFFFF} - le SAMU\n{FFAA00}Depanneur{FFFFFF} - un mecanicien\n{AAAAAA}SOS - alerte generale");
	return ShowPlayerDialog(playerid, DIALOG_TA_URGENCE, DIALOG_STYLE_LIST, "Urgences", body, "Appeler", "Retour");
}

// ============================================================================
// REGLAGES
// ============================================================================
stock TelApp_Reglages(playerid)
{
	new body[500];
	format(body, sizeof(body), "{FFFFFF}Fond - Bleu nuit\n{FFFFFF}Fond - Violet\n{FFFFFF}Fond - Vert sombre\n{FFFFFF}Fond - Brun\n{FFFFFF}Fond - Vert fonce\n{FFFFFF}Fond - Bordeaux\n{AAAAAA}Mon numero : %d", PlayerInfo[playerid][pNumber]);
	return ShowPlayerDialog(playerid, DIALOG_TA_REGLAGES, DIALOG_STYLE_LIST, "Reglages", body, "Choisir", "Retour");
}

// ============================================================================
// PLAN
// ============================================================================
stock TelApp_Plan(playerid)
{
	new body[400], Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);
	format(body, sizeof(body), "{FFFFFF}Partager ma position\n{AAAAAA}Ma position : %.0f %.0f %.0f", x, y, z);
	return ShowPlayerDialog(playerid, DIALOG_TA_PLAN, DIALOG_STYLE_LIST, "Plan", body, "Choisir", "Retour");
}

// ============================================================================
// INFOS
// ============================================================================
stock TelApp_Infos(playerid)
{
	new enligne = 0;
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i) || IsPlayerNPC(i)) continue;
		enligne++;
	}

	new heure, minute;
	gettime(heure, minute);

	new body[500];
	format(body, sizeof(body), "{FFFFFF}Heure : {FFFF00}%02d:%02d{FFFFFF}\nJoueurs connectes : {FFFF00}%d{FFFFFF}\n\nMon numero : {FFFF00}%d{FFFFFF}\nMon niveau : {FFFF00}%d{FFFFFF}\nLiquide : {32D74B}%d${FFFFFF}\nBanque : {32D74B}%d$",
		heure, minute, enligne, PlayerInfo[playerid][pNumber], PlayerInfo[playerid][pLevel],
		PlayerInfo[playerid][pCash], PlayerInfo[playerid][pAccount]);
	return ShowPlayerDialog(playerid, DIALOG_TA_INFOS, DIALOG_STYLE_MSGBOX, "Infos", body, "Retour", "");
}

// ============================================================================
// AIGUILLAGE DEPUIS L'ACCUEIL
// ============================================================================
stock TelApp_Ouvrir(playerid, app)
{
	switch(app)
	{
		case PHONE_STATE_APP_TEL: return TelApp_Tel(playerid);
		case PHONE_STATE_APP_SMS: return TelApp_Sms(playerid);
		case PHONE_STATE_APP_CTC: return TelApp_Contacts(playerid);
		case PHONE_STATE_APP_BNK: return TelApp_Banque(playerid);
		case PHONE_STATE_APP_GAL: return TelApp_Urgences(playerid);
		case PHONE_STATE_APP_SET: return TelApp_Reglages(playerid);
		case PHONE_STATE_APP_MAP: return TelApp_Plan(playerid);
		case PHONE_STATE_APP_CAL: return TelApp_Infos(playerid);
	}
	return TelApp_Accueil(playerid);
}

// ============================================================================
// REPONSES AUX DIALOGUES
// ============================================================================
stock TelApp_OnDialog(playerid, dialogid, response, listitem, const inputtext[])
{
	if(dialogid < DIALOG_TA_TEL || dialogid > DIALOG_TA_INFOS) return 0;

	// Bouton droit : on remonte a l'accueil du telephone, jamais dans le vide.
	if(!response && dialogid != DIALOG_TA_INFOS)
	{
		TelApp_Accueil(playerid);
		return 1;
	}

	switch(dialogid)
	{
		case DIALOG_TA_INFOS:
		{
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_TEL:
		{
			switch(listitem)
			{
				case 0: TelApp_DemanderNumero(playerid);
				case 1: TelApp_Contacts(playerid);
				case 2:
				{
					TelApp_Commande(playerid, "/call 0");
					TelApp_Accueil(playerid);
				}
				default: TelApp_Tel(playerid);   // la ligne d'information
			}
			return 1;
		}
		case DIALOG_TA_NUMERO:
		{
			new numero = strval(inputtext);
			if(numero <= 0)
			{
				TelApp_Msg(playerid, "{FF4444}Numero invalide.");
				TelApp_DemanderNumero(playerid);
				return 1;
			}
			new cmd[32];
			format(cmd, sizeof(cmd), "/call %d", numero);
			TelApp_Commande(playerid, cmd);
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_SMS:
		{
			TelApp_EnvoyerSms(playerid, inputtext);
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_CONTACT:
		{
			if(listitem < 0 || listitem >= PhoneContact_Count[playerid])
			{
				TelApp_Accueil(playerid);
				return 1;
			}
			new cmd[32];
			format(cmd, sizeof(cmd), "/call %d", PhoneContact_Num[playerid][listitem]);
			TelApp_Commande(playerid, cmd);
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_URGENCE:
		{
			switch(listitem)
			{
				case 0: TelApp_Commande(playerid, "/911");
				case 1: TelApp_Commande(playerid, "/feu");
				case 2: TelApp_Commande(playerid, "/ambulance");
				case 3: TelApp_Commande(playerid, "/mecano");
				case 4: TelApp_Commande(playerid, "/sos");
			}
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_REGLAGES:
		{
			if(listitem >= 0 && listitem <= 5)
			{
				phone_Wallpaper[playerid] = listitem;
				// La couleur du fond est fixee a la creation du textdraw et
				// n'est jamais relue : sans cette ligne, changer de fond
				// n'aurait eu aucun effet visible.
				// [HABILLAGE HANDPHONE] L'indice 8 (ancien design) est devenu
				// PHONE_TD_WALLPAPER (vibe_phone.inc) : c'est maintenant le
				// fond d'ecran visible derriere les icones du nouveau design.
				PlayerTextDrawColor(playerid, phone_TD[playerid][PHONE_TD_WALLPAPER], taFonds[listitem]);
				TelApp_Msg(playerid, "{00FF00}Fond d'ecran change.");
			}
			TelApp_Accueil(playerid);
			return 1;
		}
		case DIALOG_TA_PLAN:
		{
			if(listitem == 0) TelApp_Commande(playerid, "/position");
			TelApp_Accueil(playerid);
			return 1;
		}
	}
	return 1;
}

stock TelApp_OnConnect(playerid)
{
	// Rien a remettre a zero pour l'instant : le crochet existe pour que le
	// jour ou ce module gardera un etat par joueur, il soit deja branche.
	#pragma unused playerid
	return 1;
}
