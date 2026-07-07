/*
 *  AFRP Mobile Phone System - Filterscript
 *  Adapte en francais depuis mobilePhone-SAMP (2025)
 *  Dependances: YSI (y_hooks, y_timers), Pawn.CMD
 *  BDD: s114424_Rp-ci (table lvrp_afrp_phone)
 *
 *  Installation:
 *  1. Copier ce fichier dans filterscripts/afrp_phone.pwn
 *  2. Compiler : pawn afrp_phone.pwn
 *  3. Ajouter dans server.cfg : filterscripts afrp_phone
 *  4. Executer le SQL dans phpMyAdmin (voir instructions)
 *
 *  Commandes:
 *  /mobile ou /tel  -> Ouvrir/fermer le telephone
 *  /shop ou /market -> Acheter un telephone (dans le magasin)
 *  /mobilesettings  -> Personnaliser fond et couleur de cadre
 */

#include <a_samp>
#include <a_mysql>

#define FILTERSCRIPT

// ============================================================
// === CONFIG
// ============================================================
// BDD : meme connexion que le gamemode AFRP
// Remplir avec les infos de LemeHost :
#define MYSQL_HOST      "51.38.205.167"
#define MYSQL_USER      "u114424_4eTuHFdLws"
#define MYSQL_PASS      "bcLM+N9!qp2b!824pmmdaYAI"
#define MYSQL_DB        "s114424_Rp-ci"

#define DEBUG           0   // 1=affiche debug dans console, 0=silencieux
#define SELECTION_COLOR 0xE3E3E3AA

// Apps index
#define CALLDIAL        1
#define CALLLIST        2
#define CALLING         3
#define NOTESTD         1
#define NOTESLISTTD     2
#define HIDE            0
#define SHOW            1
#define BANK            0
#define CALL            1
#define HOME            2
#define NOAPPS          3
#define NOTES           4
#define SMS             5
#define TIMELINE        6
#define TWITTER         7
#define BUYING          1
#define EXITING         2

// Dialog IDs (plage haute pour eviter conflits avec LVRP)
#define MARKET_DIALOG   16261
#define SETTINGS_DIALOG 17214

// Notifications
#define BANK_PAYMENT    1
#define CALL_RECEIVED   2
#define UNAVAILABLE     3
#define SMS_RECEIVED    4
#define NEW_TWEET       5
#define PHONE_CREDIT    6
#define READY_TO_USE    7
#define NO_PHONE        8
#define HAS_PHONE       9
#define NOT_IN_MARKET   10

// Messages francais
#define MSG_NO_CONNECTION   "Impossible de se connecter a la base de donnees."
#define MSG_CONNECTED       "Base de donnees connectee."
#define MSG_SHOP            "BOUTIQUE"
#define MSG_SHOP_INFO       "\nTapez /market pour acheter"
#define MSG_CONSOLE_SHOP    "Cree %d boutiques."
#define MSG_DIALOG_CLOSED   "Vous avez ferme le dialogue."
#define MSG_MOBILE          "Telephone Mobile"
#define MSG_BUY             "Acheter"
#define MSG_CANCEL          "Annuler"
#define MSG_SUCCESS         "Telephone achete ! Utilisez /mobile pour l ouvrir."
#define MSG_COUNT           "Nombre total de TextDraws Mobile: %d"
#define MSG_UNAVAILABLE     "Ce joueur est actuellement indisponible."
#define MSG_TRY_AGAIN       "Reessayez plus tard."
#define MSG_SMS_RECEIVED    "Le joueur %s vous a envoye un SMS."
#define MSG_CALL_NOTIF      "Le joueur %s vous appelle."
#define MSG_CHECK_PHONE     "Consultez votre telephone."
#define MSG_PAYMENT         "Vous avez recu un virement bancaire."
#define MSG_NEW_TWEET       "Quelqu un a publie un nouveau tweet."
#define MSG_CREDIT_OK       "Credit recharge avec succes."
#define MSG_PHONE_READY     "Telephone achete avec succes !"
#define MSG_PHONE_CMD       "Utilisez /mobile pour l ouvrir"
#define MSG_ERROR_PHONE     "Vous n avez pas de telephone."
#define MSG_FORCE_BUY       "Achetez-en un en boutique (/market)."
#define MSG_HAS_PHONE       "Vous avez deja un telephone."
#define MSG_MARKET_ERROR    "Vous n etes pas dans une boutique."
#define MSG_CREDIT          "Credit"
#define MSG_CREDIT_MARKET   "Vous avez maintenant $%d de credit."
#define MSG_CREDIT_MAX      "Vous avez atteint le credit maximum."
#define MSG_TWEET_ERROR     "Finissez ou annulez d abord votre tweet."
#define MSG_TWEET_EXIT      "Vous avez annule la publication du tweet."
#define MSG_SMS_EXIT        "Vous avez annule l envoi du SMS."
#define MSG_TWEET_DONE      "Votre tweet a ete publie !"
#define MSG_TWEET_TOO_LONG  "Message trop long, reessayez."

// Variables
// [FIX] Tous les magasins 24/7 de Los Santos + rayon augmente a 15m
new Float:marketCoordinates[10][3] = {
    {1152.3308, -1657.2321, 14.5},  // Ganton 24/7
    {2201.4375, -1653.9713, 14.5},  // Idlewood 24/7
    {1959.1799, -1779.1599, 13.4},  // El Corona 24/7
    {2697.0166, -1704.2478, 9.8},   // East LS 24/7
    {1361.5000, -1289.0000, 13.4},  // Unity Station 24/7
    {2684.9375, -2095.4000, 13.5},  // Playa del Seville 24/7
    {1937.4000, -1396.6000, 13.4},  // Jefferson 24/7
    {1911.5000, -1786.9000, 13.4},  // Willowfield 24/7
    {2538.0000, -1743.0000, 13.4},  // Balla territory 24/7
    {2155.9900, -2095.4000, 13.5}   // Las Colinas 24/7
};

new usingPhone[MAX_PLAYERS] = 0,
    hasPhone[MAX_PLAYERS] = 0,
    playerNumber[MAX_PLAYERS] = 0,
    playerCredit[MAX_PLAYERS] = 0,
    playerOccupied[MAX_PLAYERS] = 0,
    writingTweet[MAX_PLAYERS] = 0,
    twitterDelay = 0,
    tweetID = 1,
    marketPickupID[10],

    PlayerText:TEXTDRAW_BANK[MAX_PLAYERS][21],
    PlayerText:TEXTDRAW_CALLDIAL[MAX_PLAYERS][22],
    PlayerText:TEXTDRAW_CALLLIST[MAX_PLAYERS][29],
    PlayerText:TEXTDRAW_CALLING[MAX_PLAYERS][5],
    PlayerText:TEXTDRAW_HOME[MAX_PLAYERS][12],
    PlayerText:TEXTDRAW_DEFAULT[MAX_PLAYERS][19],
    PlayerText:TEXTDRAW_NOTES[MAX_PLAYERS][11],
    PlayerText:TEXTDRAW_NOTESLIST[MAX_PLAYERS][16],
    PlayerText:TEXTDRAW_NOTIFICATION[MAX_PLAYERS][4],
    PlayerText:TEXTDRAW_SMS[MAX_PLAYERS][3],
    PlayerText:TEXTDRAW_TIME[MAX_PLAYERS][5],
    PlayerText:TEXTDRAW_TWITTER[MAX_PLAYERS][16];

new MySQL:db_handle;

// ============================================================
// === CONNEXION BDD
// ============================================================
public OnFilterScriptInit()
{
    print("=========================================");
    print("[AFRP PHONE] >>> FILTERSCRIPT EN COURS DE CHARGEMENT");
    print("=========================================");
    db_handle = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB);
    if(db_handle && mysql_errno(db_handle) == 0)
        printf("[AFRP PHONE] "MSG_CONNECTED" (handle: %d)", _:db_handle);
    else
        printf("[AFRP PHONE] ERREUR connexion BDD (%d)", mysql_errno(db_handle));
    CreateShop();
    // [FIX REAPPEAR] Charger les donnees telephone pour TOUS les joueurs deja connectes
    // (sinon les textdraws ne sont jamais crees pour eux et /mobile ne fait rien)
    for(new p = 0; p < GetMaxPlayers(); p++)
    {
        if(IsPlayerConnected(p) && !IsPlayerNPC(p))
        {
            new foo[120];
            mysql_format(db_handle, foo, sizeof(foo),
                "SELECT * FROM `lvrp_afrp_phone` WHERE `Username` = '%e'", GetName(p));
            mysql_tquery(db_handle, foo, "Phone_LoadUser", "d", p);
            mysql_tquery(db_handle, foo, "Phone_LoadSettings", "d", p);
            CreateNotificationTD(p);
        }
    }
    return 1;
}

public OnFilterScriptExit()
{
    // [FIX] Ne pas fermer le handle MySQL - il est partage avec le gamemode principal
    // mysql_close(db_handle); // DESACTIVE
    return 1;
}

// ============================================================
// === TIMER
// ============================================================
// Timer sans YSI : utilise SetTimerEx
forward Phone_HideNotification(playerid);
public Phone_HideNotification(playerid)
{
    for(new i = 0; i < 4; i++)
        PlayerTextDrawHide(playerid, TEXTDRAW_NOTIFICATION[playerid][i]);
    return 1;
}

// ============================================================
// === COMMANDES
// ============================================================
// Commandes via OnPlayerCommandText (sans Pawn.CMD)
public OnPlayerCommandText(playerid, cmdtext[])
{
    new cmd[32];
    cmd[0] = EOS;
    // Extraire la commande
    new k = 0;
    while(cmdtext[k] > ' ' && k < 31) { cmd[k] = cmdtext[k]; k++; }
    cmd[k] = EOS;

    if(strcmp(cmd, "/mobile", true) == 0 || strcmp(cmd, "/mobitel", true) == 0)
    {
        printf("[AFRP PHONE DEBUG] /mobile recu par player %d (hasPhone=%d)", playerid, hasPhone[playerid]);
        SendClientMessage(playerid, -1, "{00FF00}>> DEBUG <<{FFFFFF} /mobile capture par le filterscript afrp_phone.");
        if(hasPhone[playerid] == 0)
        {
            // [FIX REAPPEAR] Tenter de recharger les donnees depuis la BDD au cas ou
            // le filterscript a ete redemarre apres connexion du joueur
            new foo[120];
            mysql_format(db_handle, foo, sizeof(foo),
                "SELECT * FROM `lvrp_afrp_phone` WHERE `Username` = '%e'", GetName(playerid));
            mysql_tquery(db_handle, foo, "Phone_LoadUser", "d", playerid);
            // Info au joueur : si tu as un telephone et que rien ne s'affiche, attends 1s et reessaye
            SendClientMessage(playerid, -1, "{CF9756}>> Phone <<{FFFFFF} Verification... Si tu as un telephone retape /mobile. Sinon /market pour acheter.");
            return 1;
        }
        if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, MSG_TWEET_ERROR), 1;
        if(usingPhone[playerid] == 0)
        {
            SelectTextDraw(playerid, SELECTION_COLOR);
            UseMobile(playerid, HOME, SHOW);
            UseMobile(playerid, NOAPPS, SHOW);
            UpdateTimeDate(playerid, 2);
            usingPhone[playerid] = 1;
        }
        else
        {
            CancelSelectTextDraw(playerid);
            HidePhone(playerid);
            ShowNoApps(playerid, HIDE);
            usingPhone[playerid] = 0;
        }
        return 1;
    }
    if(strcmp(cmd, "/shop", true) == 0 || strcmp(cmd, "/market", true) == 0)
    {
        // [FIX] Achat possible partout (plus besoin d'etre dans un magasin)
        if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, MSG_TWEET_ERROR), 1;
        OnPlayerEnterShop(playerid, BUYING);
        return 1;
    }
    // [APP WHATSAPP] /wa - vraie app avec chat et appels vocaux
    if(strcmp(cmd, "/wa", true) == 0 || strcmp(cmd, "/whatsapp", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9200, DIALOG_STYLE_LIST, "{00BA00}WhatsApp AFRP",
            "{FFFFFF}1. Discussion de groupe (chat global)\n{FFFFFF}2. Envoyer un message prive\n{00FF00}3. Appel vocal a un contact\n{FFFFFF}4. Mon credit : voir",
            "Choisir", "Fermer");
        return 1;
    }
    // [APP TRADING] /trading - vraie app avec achat/vente
    if(strcmp(cmd, "/trading", true) == 0 || strcmp(cmd, "/trade", true) == 0 || strcmp(cmd, "/bourse", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9210, DIALOG_STYLE_LIST, "{FFAA00}AFRP Trading",
            "{FFFFFF}1. Voir les cours en direct\n{00FF00}2. Acheter une crypto/action\n{FF0000}3. Vendre une position\n{FFFF00}4. Mon portefeuille",
            "Choisir", "Fermer");
        return 1;
    }
    if(strcmp(cmd, "/mobilesettings", true) == 0 || strcmp(cmd, "/parametres", true) == 0 || strcmp(cmd, "/settings", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        if(writingTweet[playerid] == 1) return SendClientMessage(playerid, -1, MSG_TWEET_ERROR), 1;
        if(playerOccupied[playerid] == 1) return SendClientMessage(playerid, -1, "Terminez d abord votre appel."), 1;
        // [FIX] Ne plus exiger que le telephone soit ouvert - parametres accessibles partout
        ShowPlayerDialog(playerid, SETTINGS_DIALOG, DIALOG_STYLE_LIST, "{2B6AFF}Parametres Telephone",
            "{FFFFFF}Fond - Ciel etoile\n{FFFFFF}Fond - Bicolore\n{FFFFFF}Fond - Ciel bleu\n{FFFFFF}Fond - Battement\n{FFFFFF}Fond - Lignes vertes\n{FFFFFF}Fond - Course\n{888888}-------------------\n{FFFFFF}Cadre - Blanc\n{4488FF}Cadre - Bleu\n{FF4444}Cadre - Rouge\n{44FF44}Cadre - Vert\n{FFFF44}Cadre - Jaune\n{AA44FF}Cadre - Violet\n{FF88AA}Cadre - Rose",
            "Choisir", MSG_CANCEL);
        return 1;
    }
    // [APP AIDE] /phonehelp - liste de toutes les commandes du telephone
    if(strcmp(cmd, "/phonehelp", true) == 0 || strcmp(cmd, "/phoneaide", true) == 0 || strcmp(cmd, "/aidephone", true) == 0)
    {
        ShowPlayerDialog(playerid, 9220, DIALOG_STYLE_MSGBOX, "{2B6AFF}AFRP Phone - Aide",
            "{FFFFFF}Commandes disponibles :\n\n{00FF00}/mobile{FFFFFF} - Ouvrir/fermer le telephone\n{00FF00}/market{FFFFFF} - Acheter un telephone (500$)\n{00BA00}/wa{FFFFFF} - App WhatsApp (chat + vocal)\n{FFAA00}/trading{FFFFFF} - App Trading (crypto/actions)\n{888888}/mobilesettings{FFFFFF} - Parametres et fonds d'ecran\n{888888}/phonehelp{FFFFFF} - Cette aide\n\n{FFFF00}Astuce : /wa et /trading marchent partout, pas besoin d'ouvrir le telephone !",
            "OK", "");
        return 1;
    }
    // ====================================================
    // === [APP GOUVERNEMENT] /gouv - Regles, devoirs, casier civique
    // ====================================================
    if(strcmp(cmd, "/gouv", true) == 0 || strcmp(cmd, "/gouvernement", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9300, DIALOG_STYLE_LIST, "{0066CC}Gouvernement AFRP",
            "{FFFFFF}1. Regles du serveur\n{FFFFFF}2. Devoirs du citoyen\n{FFFF00}3. Mon casier civique\n{00FF00}4. Lois en vigueur",
            "Voir", "Fermer");
        return 1;
    }
    // ====================================================
    // === [APP TRAVAIL] /travail - Mon job
    // ====================================================
    if(strcmp(cmd, "/travail", true) == 0 || strcmp(cmd, "/job", true) == 0 || strcmp(cmd, "/jobs", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9310, DIALOG_STYLE_LIST, "{FF6600}App Travail",
            "{FFFFFF}1. Mon job actuel\n{FFFFFF}2. Jobs disponibles\n{FFFFFF}3. Salaires des factions\n{FFFF00}4. Mon statut professionnel",
            "Voir", "Fermer");
        return 1;
    }
    // ====================================================
    // === [APP ANNUAIRE] /annuaire - Numeros utiles
    // ====================================================
    if(strcmp(cmd, "/annuaire", true) == 0 || strcmp(cmd, "/contacts", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9320, DIALOG_STYLE_MSGBOX, "{2B6AFF}Annuaire AFRP",
            "{FFFFFF}Numeros utiles :\n\n{0066FF}Police LSPD{FFFFFF}        : {00FF00}911\n{FF0000}Pompiers{FFFFFF}           : {00FF00}912\n{FF1493}Hopital / SAMU{FFFFFF}    : {00FF00}913\n{FFAA00}Taxi AFRP{FFFFFF}         : {00FF00}914\n{00BFFF}Marine{FFFFFF}              : {00FF00}915\n{888888}Armee Terre{FFFFFF}        : {00FF00}916\n{4B0082}Justice / Tribunal{FFFFFF} : {00FF00}917\n{FFFF00}Mairie{FFFFFF}              : {00FF00}918\n\n{FFFFFF}Tapez /call <numero> pour appeler",
            "OK", "");
        return 1;
    }
    // ====================================================
    // === [APP METEO] /meteo - Heure et meteo
    // ====================================================
    if(strcmp(cmd, "/meteo", true) == 0 || strcmp(cmd, "/heure", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        new h, m, s, jour, mois, annee, infoM[400];
        gettime(h, m, s);
        getdate(annee, mois, jour);
        new const conditions[5][] = {"Ensoleille", "Nuageux", "Pluie legere", "Brouillard", "Tempete"};
        new const periodes[3][] = {"Matin", "Apres-midi", "Nuit"};
        new periode = (h < 12) ? 0 : ((h < 18) ? 1 : 2);
        new cond = (jour + mois) % 5; // pseudo-aleatoire base sur la date
        new temp = 18 + random(15); // 18-32 degres
        format(infoM, sizeof(infoM),
            "{FFFFFF}Date     : {FFFF00}%02d/%02d/%d{FFFFFF}\nHeure    : {FFFF00}%02d:%02d:%02d{FFFFFF}\nPeriode  : {FFFF00}%s{FFFFFF}\n\n{00FFFF}Meteo Los Santos :\n{FFFFFF}Condition : {00FF00}%s{FFFFFF}\nTemperature : {00FF00}%d C{FFFFFF}\n\n{888888}Bon RP a tous !",
            jour, mois, annee, h, m, s, periodes[periode], conditions[cond], temp);
        ShowPlayerDialog(playerid, 9330, DIALOG_STYLE_MSGBOX, "{00BFFF}Meteo AFRP", infoM, "OK", "");
        return 1;
    }
    // ====================================================
    // === [APP POLITIQUE] /politique - Elections, maire, lois
    // ====================================================
    if(strcmp(cmd, "/politique", true) == 0 || strcmp(cmd, "/maire", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9340, DIALOG_STYLE_LIST, "{FFD700}Politique AFRP",
            "{FFFFFF}1. Maire actuel et conseil\n{FFFFFF}2. Lois en vigueur\n{FFFFFF}3. Prochaines elections\n{00FF00}4. Voter (si election ouverte)",
            "Voir", "Fermer");
        return 1;
    }
    // ====================================================
    // === [APP MARKETPLACE] /troc - Marketplace joueurs
    // ====================================================
    if(strcmp(cmd, "/troc", true) == 0 || strcmp(cmd, "/marketplace", true) == 0 || strcmp(cmd, "/annonces", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9350, DIALOG_STYLE_LIST, "{32CD32}Marketplace AFRP",
            "{FFFFFF}1. Voir les annonces\n{FFFFFF}2. Poster une annonce (50 francs)\n{FFFFFF}3. Mes annonces\n{FF0000}4. Supprimer une annonce",
            "Choisir", "Fermer");
        return 1;
    }
    // ====================================================
    // === [APP CARTE] /carte - Carte interactive
    // ====================================================
    if(strcmp(cmd, "/carte", true) == 0 || strcmp(cmd, "/map", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        ShowPlayerDialog(playerid, 9360, DIALOG_STYLE_LIST, "{00BFFF}Carte AFRP",
            "{FFFFFF}1. Territoires de gangs\n{FFFFFF}2. Zones protegees (police/hopital)\n{FFFFFF}3. Hopitaux et postes\n{FFFFFF}4. Magasins 24/7",
            "Voir", "Fermer");
        return 1;
    }
    // ====================================================
    // === [APP SANTE] /sante - Carnet medical
    // ====================================================
    if(strcmp(cmd, "/sante", true) == 0 || strcmp(cmd, "/medical", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        new Float:hp, Float:armor, sanInfo[300];
        GetPlayerHealth(playerid, hp);
        GetPlayerArmour(playerid, armor);
        new etat[30] = "Bonne sante";
        if(hp < 30) etat = "{FF0000}Critique";
        else if(hp < 60) etat = "{FFAA00}Blesse";
        else etat = "{00FF00}En forme";
        format(sanInfo, sizeof(sanInfo),
            "{FFFFFF}Mon carnet medical :\n\nVie : {00FF00}%.0f / 100\n{FFFFFF}Armure : {00FFFF}%.0f / 100\n{FFFFFF}Etat : %s\n\n{FF1493}Hopital LS : Centre Ville\n{FF1493}SAMU : tapez /call 913\n\n{888888}En cas d'urgence, contactez un medecin.",
            hp, armor, etat);
        ShowPlayerDialog(playerid, 9370, DIALOG_STYLE_MSGBOX, "{FF1493}App Sante", sanInfo, "OK", "");
        return 1;
    }
    // ====================================================
    // === [APP IMMO] /immo - Annonces immobilieres
    // ====================================================
    if(strcmp(cmd, "/immo", true) == 0 || strcmp(cmd, "/immobilier", true) == 0)
    {
        if(hasPhone[playerid] == 0) return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
        // Recuperer les maisons a vendre
        new immoSql[200];
        format(immoSql, sizeof(immoSql), "SELECT id, Cout, Pos_x, Pos_y FROM lvrp_server_houses WHERE Owned=0 LIMIT 15");
        mysql_tquery(db_handle, immoSql, "Immo_ListHouses", "d", playerid);
        return 1;
    }

    return 0; // Commande non geree par ce filterscript
}

// ============================================================
// === CALLBACKS
// ============================================================
public OnPlayerConnect(playerid)
{
    new foo[120];
    mysql_format(db_handle, foo, sizeof(foo),
        "SELECT * FROM `lvrp_afrp_phone` WHERE `Username` = '%e'", GetName(playerid));
    mysql_tquery(db_handle, foo, "Phone_LoadUser", "d", playerid);
    mysql_tquery(db_handle, foo, "Phone_LoadSettings", "d", playerid);
    CreateNotificationTD(playerid);
    return 1;
}

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
    // Notes
    if(playertextid == TEXTDRAW_HOME[playerid][0])
    { HidePhone(playerid); UseMobile(playerid, NOTES, SHOW, NOTESLISTTD); }
    // Banque
    if(playertextid == TEXTDRAW_HOME[playerid][1])
    {
        HidePhone(playerid);
        // Update solde depuis les infos LVRP si dispo
        new bankSql[120];
        mysql_format(db_handle, bankSql, sizeof(bankSql),
            "SELECT Cash, Bank FROM `lvrp_users` WHERE `Name` = '%e'", GetName(playerid));
        mysql_tquery(db_handle, bankSql, "Phone_LoadBalance", "d", playerid);
        UseMobile(playerid, BANK, SHOW);
    }
    // SMS
    if(playertextid == TEXTDRAW_HOME[playerid][2])
    { HidePhone(playerid); UseMobile(playerid, SMS, SHOW); }
    // Appel
    if(playertextid == TEXTDRAW_HOME[playerid][3])
    { HidePhone(playerid); UseMobile(playerid, CALL, SHOW, CALLDIAL); }
    // Horloge
    if(playertextid == TEXTDRAW_HOME[playerid][4])
    { HidePhone(playerid); UseMobile(playerid, TIMELINE, SHOW); }
    // Twitter
    if(playertextid == TEXTDRAW_HOME[playerid][5])
    { HidePhone(playerid); UseMobile(playerid, TWITTER, SHOW); }

    // [APP PARAMETRES] Icone ? - clic ouvre les parametres
    if(playertextid == TEXTDRAW_HOME[playerid][6])
    {
        HidePhone(playerid);
        CancelSelectTextDraw(playerid);
        usingPhone[playerid] = 0;
        ShowPlayerDialog(playerid, SETTINGS_DIALOG, DIALOG_STYLE_LIST, "{2B6AFF}Parametres Telephone",
            "{FFFFFF}Fond - Ciel etoile\n{FFFFFF}Fond - Bicolore\n{FFFFFF}Fond - Ciel bleu\n{FFFFFF}Fond - Battement\n{FFFFFF}Fond - Lignes vertes\n{FFFFFF}Fond - Course\n{888888}-------------------\n{FFFFFF}Cadre - Blanc\n{4488FF}Cadre - Bleu\n{FF4444}Cadre - Rouge\n{44FF44}Cadre - Vert\n{FFFF44}Cadre - Jaune\n{AA44FF}Cadre - Violet\n{FF88AA}Cadre - Rose",
            "Choisir", "Fermer");
    }
    // [APP TRADING] Icone $ - clic ouvre l'app trading
    if(playertextid == TEXTDRAW_HOME[playerid][7])
    {
        HidePhone(playerid);
        CancelSelectTextDraw(playerid);
        usingPhone[playerid] = 0;
        ShowPlayerDialog(playerid, 9210, DIALOG_STYLE_LIST, "{FFAA00}AFRP Trading",
            "{FFFFFF}1. Voir les cours en direct\n{00FF00}2. Acheter une crypto/action\n{FF0000}3. Vendre une position\n{FFFF00}4. Mon portefeuille",
            "Choisir", "Fermer");
    }

    // Fermer telephone (X)
    if(playertextid == TEXTDRAW_DEFAULT[playerid][12])
    {
        HidePhone(playerid);
        CancelSelectTextDraw(playerid);
        UseMobile(playerid, NOAPPS, HIDE);
        usingPhone[playerid] = false;
    }
    // Retour (<)
    if(playertextid == TEXTDRAW_DEFAULT[playerid][13])
    { HidePhone(playerid); UseMobile(playerid, HOME, SHOW); UseMobile(playerid, NOAPPS, SHOW); }
    // Accueil (O)
    if(playertextid == TEXTDRAW_DEFAULT[playerid][14])
    { HidePhone(playerid); UseMobile(playerid, HOME, SHOW); UseMobile(playerid, NOAPPS, SHOW); }

    // Tweet - bouton publier
    if(playertextid == TEXTDRAW_TWITTER[playerid][5])
    {
        if(gettime() < twitterDelay)
            return SendClientMessage(playerid, -1, "Attendez 30 secondes entre deux tweets."), 1;
        writingTweet[playerid] = 1;
        CancelSelectTextDraw(playerid);
        SendClientMessage(playerid, -1, " ");
        SendClientMessage(playerid, -1, "Ecrivez votre tweet dans le chat (tweet_exit pour annuler)");
    }
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    // [OPT MYSQL] Sauvegarder le credit telephone seulement a la deconnexion
    // (au lieu de a chaque message ou appel - reduit drastiquement les requetes)
    if(hasPhone[playerid] == 1)
    {
        new sqlDisc[200];
        format(sqlDisc, sizeof(sqlDisc),
            "UPDATE lvrp_afrp_phone SET Credit=%d WHERE Username='%e'",
            playerCredit[playerid], GetName(playerid));
        mysql_tquery(db_handle, sqlDisc, "", "");
    }
    // Reset variables
    hasPhone[playerid] = 0;
    playerNumber[playerid] = 0;
    playerCredit[playerid] = 0;
    usingPhone[playerid] = 0;
    writingTweet[playerid] = 0;
    playerOccupied[playerid] = 0;
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    
    // [APP WHATSAPP] Menu principal
    if(dialogid == 9200)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0: ShowPlayerDialog(playerid, 9201, DIALOG_STYLE_INPUT, "{00BA00}WhatsApp - Discussion de groupe", "{FFFFFF}Message pour le groupe AFRP Officiel\n(visible par tous les joueurs)\n{FF0000}Cout : 10 francs", "Envoyer", "Retour");
            case 1: ShowPlayerDialog(playerid, 9202, DIALOG_STYLE_INPUT, "{00BA00}WhatsApp - Message prive", "{FFFFFF}Format : ID_DU_JOUEUR Message\nExemple : 5 Salut comment va\n{FF0000}Cout : 5 francs", "Envoyer", "Retour");
            case 2: ShowPlayerDialog(playerid, 9203, DIALOG_STYLE_INPUT, "{00BA00}WhatsApp - Appel vocal", "{FFFFFF}ID du joueur a appeler\n(il doit avoir un telephone)\n{FF0000}Cout : 20 francs", "Appeler", "Retour");
            case 3:
            {
                new cinfo[80];
                format(cinfo, sizeof(cinfo), "{FFFFFF}Vous avez {00FF00}%d francs{FFFFFF} de credit.\nRecharger via /market ou un 24/7.", playerCredit[playerid]);
                ShowPlayerDialog(playerid, 9209, DIALOG_STYLE_MSGBOX, "{00BA00}Credit", cinfo, "OK", "");
            }
        }
        return 1;
    }
    // [WA] Message groupe
    if(dialogid == 9201 && response && strlen(inputtext) > 0)
    {
        if(playerCredit[playerid] < 10) { SendClientMessage(playerid, -1, "{FF0000}>> WhatsApp <<{FFFFFF} Pas assez de credit."); return 1; }
        playerCredit[playerid] -= 10;
        new wamsg[200];
        format(wamsg, sizeof(wamsg), "{00BA00}[WA Groupe AFRP] {FFFFFF}%s : {DDDDDD}%s", GetName(playerid), inputtext);
        for(new p = 0; p < GetMaxPlayers(); p++)
            if(IsPlayerConnected(p) && hasPhone[p] == 1) SendClientMessage(p, -1, wamsg);
        // [OPT MYSQL] credit sauve a la deconnexion
        return 1;
    }
    // [WA] Message prive (format: ID texte)
    if(dialogid == 9202 && response && strlen(inputtext) > 0)
    {
        new targetID = -1, k = 0;
        while(inputtext[k] >= '0' && inputtext[k] <= '9') { targetID = (targetID < 0 ? 0 : targetID) * 10 + (inputtext[k] - '0'); k++; }
        if(targetID < 0 || !IsPlayerConnected(targetID)) { SendClientMessage(playerid, -1, "{FF0000}>> WA <<{FFFFFF} Joueur introuvable. Format : ID Message"); return 1; }
        while(inputtext[k] == ' ') k++;
        if(strlen(inputtext[k]) == 0) { SendClientMessage(playerid, -1, "{FF0000}>> WA <<{FFFFFF} Message vide."); return 1; }
        if(hasPhone[targetID] == 0) { SendClientMessage(playerid, -1, "{FF0000}>> WA <<{FFFFFF} Le destinataire n'a pas de telephone."); return 1; }
        if(playerCredit[playerid] < 5) { SendClientMessage(playerid, -1, "{FF0000}>> WA <<{FFFFFF} Pas assez de credit."); return 1; }
        playerCredit[playerid] -= 5;
        new wamsg[200];
        format(wamsg, sizeof(wamsg), "{00BA00}[WA Prive] {FFFFFF}%s : {DDDDDD}%s", GetName(playerid), inputtext[k]);
        SendClientMessage(targetID, -1, wamsg);
        format(wamsg, sizeof(wamsg), "{00BA00}[WA Prive a %s] {DDDDDD}%s", GetName(targetID), inputtext[k]);
        SendClientMessage(playerid, -1, wamsg);
        // [OPT MYSQL] credit sauve a la deconnexion
        return 1;
    }
    // [WA] Appel vocal
    if(dialogid == 9203 && response && strlen(inputtext) > 0)
    {
        new targetID = -1, k = 0;
        while(inputtext[k] >= '0' && inputtext[k] <= '9') { targetID = (targetID < 0 ? 0 : targetID) * 10 + (inputtext[k] - '0'); k++; }
        if(targetID < 0 || !IsPlayerConnected(targetID)) { SendClientMessage(playerid, -1, "{FF0000}>> WA Vocal <<{FFFFFF} Joueur introuvable."); return 1; }
        if(hasPhone[targetID] == 0) { SendClientMessage(playerid, -1, "{FF0000}>> WA Vocal <<{FFFFFF} Le destinataire n'a pas de telephone."); return 1; }
        if(playerCredit[playerid] < 20) { SendClientMessage(playerid, -1, "{FF0000}>> WA Vocal <<{FFFFFF} Pas assez de credit (20 francs)."); return 1; }
        playerCredit[playerid] -= 20;
        new callmsg[180];
        format(callmsg, sizeof(callmsg), "{00BA00}[WA Vocal] {FFFFFF}%s souhaite vous appeler. Utilisez le chat vocal SAMP pour repondre.", GetName(playerid));
        SendClientMessage(targetID, -1, callmsg);
        format(callmsg, sizeof(callmsg), "{00BA00}[WA Vocal] {FFFFFF}Appel envoye a %s. Utilisez le chat vocal SAMP (touche N par defaut).", GetName(targetID));
        SendClientMessage(playerid, -1, callmsg);
        // [OPT MYSQL] credit sauve a la deconnexion
        return 1;
    }
    // ====================================================
    // === HANDLERS APPS GOUV/JOB/POLITIQUE/MARKET/CARTE
    // ====================================================
    // [GOUV] Sous-menu
    if(dialogid == 9300)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0: ShowPlayerDialog(playerid, 9301, DIALOG_STYLE_MSGBOX, "{0066CC}Regles AFRP",
                "{FFFFFF}1. Pas de cheats ni hacks\n2. Pas d'insultes envers les autres joueurs\n3. Respectez le RP en permanence\n4. Pas de MetaGaming / PowerGaming\n5. Pas de DeathMatch sans raison RP\n6. Respectez les admins\n7. Pas de spam dans le chat\n8. Une vie = un personnage\n9. RP your level (pas Hollywood)\n10. Bug abuse interdit\n\n{FFFF00}/aide pour plus d'infos", "OK", "");
            case 1: ShowPlayerDialog(playerid, 9302, DIALOG_STYLE_MSGBOX, "{0066CC}Devoirs du citoyen",
                "{FFFFFF}En tant que citoyen AFRP, vous devez :\n\n- Respecter les lois en vigueur\n- Payer vos impots et amendes\n- Cooperer avec les forces de l'ordre\n- Ne pas troubler l'ordre public\n- Aider les forces de secours\n- Respecter les autres citoyens\n- Voter aux elections (si possible)\n\n{888888}Le non-respect peut entrainer amendes ou prison.", "OK", "");
            case 2:
            {
                // Recuperer le casier civique (Wanted, Warns) depuis lvrp_users
                new sqlCas[200];
                format(sqlCas, sizeof(sqlCas), "SELECT Wanted, Warns, Member FROM lvrp_users WHERE Name='%e'", GetName(playerid));
                mysql_tquery(db_handle, sqlCas, "Gouv_LoadCasier", "d", playerid);
            }
            case 3: ShowPlayerDialog(playerid, 9303, DIALOG_STYLE_MSGBOX, "{0066CC}Lois en vigueur",
                "{FFFFFF}Lois principales AFRP :\n\n{FFAA00}- Vol / Cambriolage{FFFFFF} : amende + prison\n{FFAA00}- Agression{FFFFFF} : prison (3-10 min)\n{FFAA00}- Meurtre{FFFFFF} : prison (10-15 min)\n{FFAA00}- Conduite dangereuse{FFFFFF} : amende 500-2000$\n{FFAA00}- Exces de vitesse{FFFFFF} : amende 200$\n{FFAA00}- Fuite police{FFFFFF} : amende doublee\n{FFAA00}- Drogues{FFFFFF} : prison (5-20 min)\n{FFAA00}- Possession arme illegale{FFFFFF} : prison\n{FFAA00}- Corruption{FFFFFF} : revocation + prison", "OK", "");
        }
        return 1;
    }
    // [TRAVAIL] Sous-menu
    if(dialogid == 9310)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0:
            {
                new sqlJ[150];
                format(sqlJ, sizeof(sqlJ), "SELECT Job, JobTime FROM lvrp_users WHERE Name='%e'", GetName(playerid));
                mysql_tquery(db_handle, sqlJ, "Job_LoadStatus", "d", playerid);
            }
            case 1: ShowPlayerDialog(playerid, 9311, DIALOG_STYLE_MSGBOX, "{FF6600}Jobs disponibles",
                "{FFFFFF}Liste des jobs civils AFRP :\n\n{00FF00}1. Routier{FFFFFF} (camionneur)\n{00FF00}2. Taxi{FFFFFF} (livraison passagers)\n{00FF00}3. Bus{FFFFFF} (transport en commun)\n{00FF00}4. Pecheur{FFFFFF}\n{00FF00}5. Bucheron{FFFFFF}\n{00FF00}6. Eboueur{FFFFFF}\n{00FF00}7. Pizzaiolo{FFFFFF}\n{00FF00}8. Mecano{FFFFFF}\n{00FF00}9. Mineur{FFFFFF}\n{00FF00}10. Agriculteur{FFFFFF}\n\n{FFFF00}Tapez /jobgo pour rejoindre un job.", "OK", "");
            case 2: ShowPlayerDialog(playerid, 9312, DIALOG_STYLE_MSGBOX, "{FF6600}Salaires factions",
                "{FFFFFF}Salaires horaires des factions legales :\n\n{0066FF}Police{FFFFFF}        Rang 1 : 8k - Rang 8 : 50k\n{FF0000}Pompiers{FFFFFF}      Rang 1 : 10k - Rang 8 : 38k\n{FF1493}Hopital{FFFFFF}       Rang 1 : 10k - Rang 8 : 50k\n{888888}Armee{FFFFFF}         Rang 1 : 10k - Rang 8 : 38k\n{4B0082}Justice{FFFFFF}       Rang 1 : 10k - Rang 8 : 50k\n{FFFF00}Mairie{FFFFFF}        Rang 1 : 12k - Rang 8 : 45k\n\n{888888}Payday automatique chaque heure.", "OK", "");
            case 3:
            {
                new sqlStat[150];
                format(sqlStat, sizeof(sqlStat), "SELECT Member, `Rank`, Job FROM lvrp_users WHERE Name='%e'", GetName(playerid));
                mysql_tquery(db_handle, sqlStat, "Job_LoadProStatus", "d", playerid);
            }
        }
        return 1;
    }
    // [POLITIQUE] Sous-menu
    if(dialogid == 9340)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0:
            {
                new sqlPol[200];
                format(sqlPol, sizeof(sqlPol), "SELECT Name FROM lvrp_users WHERE Leader=8 LIMIT 1");
                mysql_tquery(db_handle, sqlPol, "Politique_LoadMaire", "d", playerid);
            }
            case 1: ShowPlayerDialog(playerid, 9342, DIALOG_STYLE_MSGBOX, "{FFD700}Lois en vigueur",
                "{FFFFFF}Lois validees par le conseil municipal :\n\n{00FF00}- Code de la route applique\n- Couvre-feu : aucun\n- Port d'arme : licence obligatoire\n- Vente d'alcool : autorisee\n- Casinos : autorises (legaux)\n- Trafic de drogue : interdit\n- Immigration : libre\n\n{888888}Lois proposees par le maire.\nProchain conseil : dimanche 21h", "OK", "");
            case 2: ShowPlayerDialog(playerid, 9343, DIALOG_STYLE_MSGBOX, "{FFD700}Prochaines elections",
                "{FFFFFF}Election municipale AFRP\n\n{FFFF00}Date{FFFFFF} : Premier dimanche du mois\n{FFFF00}Heure{FFFFFF} : 21h00\n{FFFF00}Duree mandat{FFFFFF} : 1 mois IRL\n{FFFF00}Conditions candidats{FFFFFF} :\n- Niveau 5+\n- Casier vierge depuis 7 jours\n- Inscription 3 jours avant\n\n{888888}Posez votre candidature aupres d'un admin.", "OK", "");
            case 3: SendClientMessage(playerid, -1, "{FFD700}>> Politique <<{FFFFFF} Aucune election en cours actuellement.");
        }
        return 1;
    }
    // [MARKETPLACE] Sous-menu
    if(dialogid == 9350)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0: mysql_tquery(db_handle, "SELECT id, Vendeur, Titre, Prix FROM lvrp_marketplace ORDER BY id DESC LIMIT 20", "Market_LoadAnnonces", "d", playerid);
            case 1:
            {
                if(playerCredit[playerid] < 50) { SendClientMessage(playerid, -1, "{FF0000}>> Marketplace <<{FFFFFF} Il faut 50 francs de credit pour poster."); return 1; }
                ShowPlayerDialog(playerid, 9351, DIALOG_STYLE_INPUT, "{32CD32}Poster annonce", "{FFFFFF}Format : Titre|Prix\nExemple : Voiture sportive a vendre|150000\n\n{FF0000}Cout : 50 francs", "Poster", "Annuler");
            }
            case 2:
            {
                new sqlMyAd[200];
                format(sqlMyAd, sizeof(sqlMyAd), "SELECT id, Titre, Prix FROM lvrp_marketplace WHERE Vendeur='%e' ORDER BY id DESC", GetName(playerid));
                mysql_tquery(db_handle, sqlMyAd, "Market_LoadMesAnnonces", "d", playerid);
            }
            case 3: ShowPlayerDialog(playerid, 9353, DIALOG_STYLE_INPUT, "{32CD32}Supprimer annonce", "{FFFFFF}ID de l'annonce a supprimer", "Supprimer", "Annuler");
        }
        return 1;
    }
    // [MARKETPLACE] Poster annonce
    if(dialogid == 9351 && response && strlen(inputtext) > 0)
    {
        new titre[64], prix = 0, k = 0;
        while(inputtext[k] != '|' && inputtext[k] != 0 && k < 63) { titre[k] = inputtext[k]; k++; }
        titre[k] = 0;
        if(inputtext[k] == '|') k++;
        while(inputtext[k] >= '0' && inputtext[k] <= '9') { prix = prix * 10 + (inputtext[k] - '0'); k++; }
        if(prix <= 0 || strlen(titre) < 3) { SendClientMessage(playerid, -1, "{FF0000}>> Market <<{FFFFFF} Format : Titre|Prix"); return 1; }
        playerCredit[playerid] -= 50;
        new sqlPost[400];
        format(sqlPost, sizeof(sqlPost), "INSERT INTO lvrp_marketplace (Vendeur, Titre, Prix, DatePost) VALUES ('%e', '%e', %d, UNIX_TIMESTAMP())", GetName(playerid), titre, prix);
        mysql_tquery(db_handle, sqlPost, "", "");
        new sqlUp[150];
        format(sqlUp, sizeof(sqlUp), "UPDATE lvrp_afrp_phone SET Credit=%d WHERE Username='%e'", playerCredit[playerid], GetName(playerid));
        mysql_tquery(db_handle, sqlUp, "", "");
        new okmsg[140];
        format(okmsg, sizeof(okmsg), "{00FF00}>> Marketplace <<{FFFFFF} Annonce postee : %s pour %d$.", titre, prix);
        SendClientMessage(playerid, -1, okmsg);
        return 1;
    }
    // [MARKETPLACE] Supprimer annonce
    if(dialogid == 9353 && response && strlen(inputtext) > 0)
    {
        new aid = 0, k = 0;
        while(inputtext[k] >= '0' && inputtext[k] <= '9') { aid = aid * 10 + (inputtext[k] - '0'); k++; }
        if(aid <= 0) return 1;
        new sqlDel[200];
        format(sqlDel, sizeof(sqlDel), "DELETE FROM lvrp_marketplace WHERE id=%d AND Vendeur='%e'", aid, GetName(playerid));
        mysql_tquery(db_handle, sqlDel, "", "");
        SendClientMessage(playerid, -1, "{00FF00}>> Marketplace <<{FFFFFF} Annonce supprimee.");
        return 1;
    }
    // [CARTE] Sous-menu
    if(dialogid == 9360)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0: mysql_tquery(db_handle, "SELECT Name, Owner, ZoneType FROM lvrp_turfs WHERE ZoneType=0 LIMIT 25", "Carte_LoadTurfs", "d", playerid);
            case 1: mysql_tquery(db_handle, "SELECT Name, Owner, ZoneType FROM lvrp_turfs WHERE ZoneType>=1 LIMIT 25", "Carte_LoadProtegees", "d", playerid);
            case 2: ShowPlayerDialog(playerid, 9362, DIALOG_STYLE_MSGBOX, "{00BFFF}Hopitaux AFRP",
                "{FFFFFF}Hopitaux et postes de secours :\n\n{FF1493}- All Saints General Hospital{FFFFFF}\n  Centre-ville Los Santos\n\n{FF1493}- County General Hospital{FFFFFF}\n  Las Venturas\n\n{0066FF}- Poste police LSPD{FFFFFF}\n  Pershing Square, LS\n\n{FF0000}- Caserne pompiers{FFFFFF}\n  Centre LS", "OK", "");
            case 3: ShowPlayerDialog(playerid, 9363, DIALOG_STYLE_MSGBOX, "{00BFFF}Magasins 24/7",
                "{FFFFFF}Magasins 24/7 ouverts en permanence :\n\n- Ganton 24/7\n- Idlewood 24/7\n- El Corona 24/7\n- East LS 24/7\n- Unity Station 24/7\n- Playa del Seville 24/7\n- Jefferson 24/7\n- Willowfield 24/7\n- Las Colinas 24/7\n\n{888888}Achats : armes, nourriture, telephones", "OK", "");
        }
        return 1;
    }
    // ====================================================
    // === FIN HANDLERS NOUVELLES APPS
    // ====================================================
    // [TRADING] Menu principal
    if(dialogid == 9210)
    {
        if(!response) return 1;
        switch(listitem)
        {
            case 0:
            {
                new info[512];
                new btc = 45000 + random(2000) - 1000;
                new eth = 2800 + random(200) - 100;
                new doge = 5 + random(3);
                new afrp_coin = 100 + random(50) - 25;
                new actions_lvrp = 1200 + random(100) - 50;
                format(info, sizeof(info),
                    "{FFFFFF}Cours en direct :\n\n{F7931A}Bitcoin (BTC){FFFFFF}        : {00FF00}$%d{FFFFFF}\n{627EEA}Ethereum (ETH){FFFFFF}      : {00FF00}$%d{FFFFFF}\n{C2A633}Dogecoin (DOGE){FFFFFF}     : {00FF00}$%d{FFFFFF}\n{2B6AFF}AFRP Coin{FFFFFF}            : {00FF00}$%d{FFFFFF}\n{FF6600}Actions AFRP Corp{FFFFFF}    : {00FF00}$%d{FFFFFF}",
                    btc, eth, doge, afrp_coin, actions_lvrp);
                ShowPlayerDialog(playerid, 9215, DIALOG_STYLE_MSGBOX, "{FFAA00}Cours du marche", info, "OK", "");
            }
            case 1: ShowPlayerDialog(playerid, 9211, DIALOG_STYLE_INPUT, "{FFAA00}Trading - Acheter", "{FFFFFF}Format : SYMBOLE MONTANT\nExemple : BTC 5000\nSymboles : BTC, ETH, DOGE, AFRP, LVRP\n\n{FF0000}Argent debite de votre compte", "Acheter", "Retour");
            case 2: ShowPlayerDialog(playerid, 9212, DIALOG_STYLE_INPUT, "{FFAA00}Trading - Vendre", "{FFFFFF}Format : SYMBOLE QUANTITE\nExemple : BTC 0.1\nSymboles : BTC, ETH, DOGE, AFRP, LVRP", "Vendre", "Retour");
            case 3:
            {
                new sqlPort[200];
                format(sqlPort, sizeof(sqlPort), "SELECT * FROM lvrp_trading_positions WHERE Username='%s'", GetName(playerid));
                mysql_tquery(db_handle, sqlPort, "Trading_LoadPortfolio", "d", playerid);
            }
        }
        return 1;
    }
    // [TRADING] Achat
    if(dialogid == 9211 && response && strlen(inputtext) > 0)
    {
        new symbol[10], montant = 0, k = 0;
        while(inputtext[k] > ' ' && k < 9) { symbol[k] = inputtext[k]; k++; }
        symbol[k] = 0;
        while(inputtext[k] == ' ') k++;
        while(inputtext[k] >= '0' && inputtext[k] <= '9') { montant = montant * 10 + (inputtext[k] - '0'); k++; }
        if(montant < 100) { SendClientMessage(playerid, -1, "{FF0000}>> Trading <<{FFFFFF} Montant minimum : 100$."); return 1; }
        if(GetPlayerMoney(playerid) < montant) { SendClientMessage(playerid, -1, "{FF0000}>> Trading <<{FFFFFF} Pas assez d'argent."); return 1; }
        GivePlayerMoney(playerid, -montant);
        new sqlBuy[300];
        format(sqlBuy, sizeof(sqlBuy), "INSERT INTO lvrp_trading_positions (Username, Symbol, AmountInvested) VALUES ('%s', '%s', %d) ON DUPLICATE KEY UPDATE AmountInvested = AmountInvested + %d", GetName(playerid), symbol, montant, montant);
        mysql_tquery(db_handle, sqlBuy, "", "");
        new buymsg[160];
        format(buymsg, sizeof(buymsg), "{00FF00}>> Trading <<{FFFFFF} Achat de %s pour %d$ effectue.", symbol, montant);
        SendClientMessage(playerid, -1, buymsg);
        return 1;
    }
    // [TRADING] Vente
    if(dialogid == 9212 && response && strlen(inputtext) > 0)
    {
        new symbol[10], k = 0;
        while(inputtext[k] > ' ' && k < 9) { symbol[k] = inputtext[k]; k++; }
        symbol[k] = 0;
        new sqlSell[300];
        format(sqlSell, sizeof(sqlSell), "SELECT AmountInvested FROM lvrp_trading_positions WHERE Username='%s' AND Symbol='%s'", GetName(playerid), symbol);
        mysql_tquery(db_handle, sqlSell, "Trading_SellPosition", "ds", playerid, symbol);
        return 1;
    }
// [FIX /market] Dialog d'achat de telephone
    if(dialogid == 9101)
    {
        if(response)
        {
            if(GetPlayerMoney(playerid) < 500)
            {
                SendClientMessage(playerid, -1, "{FF0000}>> AFRP Phone <<{FFFFFF} Pas assez d'argent (500$ requis).");
                return 1;
            }
            GivePlayerMoney(playerid, -500);
            hasPhone[playerid] = 1;
            playerNumber[playerid] = 1000 + random(8999); // numero 1000-9999
            playerCredit[playerid] = 100;
            new sql[300];
            format(sql, sizeof(sql),
                "INSERT INTO lvrp_afrp_phone (Username, HasPhone, Number, Credit) VALUES ('%s', 1, %d, 100) ON DUPLICATE KEY UPDATE HasPhone=1, Number=%d, Credit=100",
                GetName(playerid), playerNumber[playerid], playerNumber[playerid]);
            mysql_tquery(db_handle, sql, "", "");
            CreateTextDraws(playerid);
            new info[160];
            format(info, sizeof(info), "{00FF00}>> AFRP Phone <<{FFFFFF} Achat reussi ! Numero : {FFFF00}%d{FFFFFF}. Tapez /mobile pour utiliser.", playerNumber[playerid]);
            SendClientMessage(playerid, -1, info);
        }
        return 1;
    }

    switch(dialogid)
    {
        case MARKET_DIALOG:
        {
            if(response)
            {
                switch(listitem)
                {
                    case 0: // Acheter telephone
                    {
                        if(hasPhone[playerid] == 1)
                            return SendPlayerNotification(playerid, -1, HAS_PHONE), 1;
                        playerNumber[playerid] = 100000 + random(899000);
                        playerCredit[playerid] = 100;
                        hasPhone[playerid] = 1;
                        SendClientMessage(playerid, -1, MSG_SUCCESS);
                        SendPlayerNotification(playerid, -1, READY_TO_USE);
                        CreateTextDraws(playerid);
                        new foo[300];
                        mysql_format(db_handle, foo, sizeof(foo),
                            "INSERT INTO `lvrp_afrp_phone` (`Username`, `HasPhone`, `Number`, `Credit`, `Frame`, `Background`) VALUES ('%e', %d, %d, %d, 0, 0)",
                            GetName(playerid), 1, playerNumber[playerid], playerCredit[playerid]);
                        mysql_tquery(db_handle, foo);
                    }
                    case 1..4: // Recharger credit
                    {
                        if(hasPhone[playerid] == 0)
                            return SendPlayerNotification(playerid, -1, NO_PHONE), 1;
                        if(playerCredit[playerid] > 1000)
                            return SendClientMessage(playerid, -1, MSG_CREDIT_MAX), 1;
                        switch(listitem)
                        {
                            case 1: playerCredit[playerid] += 10;
                            case 2: playerCredit[playerid] += 20;
                            case 3: playerCredit[playerid] += 50;
                            case 4: playerCredit[playerid] += 100;
                        }
                        new foo[150];
                        mysql_format(db_handle, foo, sizeof(foo),
                            "UPDATE `lvrp_afrp_phone` SET `Credit` = %d WHERE `Username` = '%e'",
                            playerCredit[playerid], GetName(playerid));
                        mysql_tquery(db_handle, foo);
                        format(foo, sizeof(foo), MSG_CREDIT_MARKET, playerCredit[playerid]);
                        SendClientMessage(playerid, -1, foo);
                    }
                }
            }
            else OnPlayerEnterShop(playerid, EXITING);
        }
        case SETTINGS_DIALOG:
        {
            if(response)
            {
                // Nouveau layout :
                // 0..5 = 6 fonds d'ecran
                // 6 = ligne separation (------------)
                // 7..13 = 7 cadres
                switch(listitem)
                {
                    case 0..5: ChangeBackground(playerid, listitem);
                    case 6: { /* Ligne separation, ne rien faire */ }
                    case 7..13: ChangeFrame(playerid, listitem-7);
                }
                // Reaffiche le telephone seulement s'il etait deja ouvert
                if(usingPhone[playerid] == 1)
                {
                    HidePhone(playerid);
                    UseMobile(playerid, HOME, SHOW);
                    UseMobile(playerid, NOAPPS, SHOW);
                    SelectTextDraw(playerid, SELECTION_COLOR);
                }
            }
        }
    }
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if(playerOccupied[playerid] == 1)
    {
        if(strfind(text, "sms_exit", true) != -1)
            return SendClientMessage(playerid, -1, MSG_SMS_EXIT), playerOccupied[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR), 0;
        return 0;
    }
    if(writingTweet[playerid] == 1)
    {
        if(strfind(text, "tweet_exit", true) != -1)
            return SendClientMessage(playerid, -1, MSG_TWEET_EXIT), writingTweet[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR), 0;
        if(strlen(text) > 92)
            return SendClientMessage(playerid, -1, MSG_TWEET_TOO_LONG), writingTweet[playerid] = 0, SelectTextDraw(playerid, SELECTION_COLOR), 0;
        for(new i = 0; i < strlen(text); i++)
            if(text[i] == ' ') text[i] = '_';
        new string[92];
        format(string, 92, "%s", text);
        if(strlen(string) > 21) strins(string, "~n~", 21, 92);
        if(strlen(string) > 45) strins(string, "~n~", 45, 92);
        if(strlen(string) > 68) strins(string, "~n~", 68, 92);
        new foo[180];
        mysql_format(db_handle, foo, sizeof(foo),
            "UPDATE `twitter_afrp` SET `TweetString` = '%s', `User` = '%e', `TweetTime` = NOW() WHERE `TweetID` = %d",
            string, GetName(playerid), tweetID);
        mysql_tquery(db_handle, foo);
        PlayerTextDrawSetString(playerid, TEXTDRAW_TWITTER[playerid][9+tweetID], string);
        tweetID++;
        if(tweetID > 3) tweetID = 1;
        SendClientMessage(playerid, -1, " ");
        SendClientMessage(playerid, -1, MSG_TWEET_DONE);
        writingTweet[playerid] = 0;
        twitterDelay = gettime() + 30;
        SelectTextDraw(playerid, SELECTION_COLOR);
        return 0;
    }
    return 1;
}

// ============================================================
// === FONCTIONS INTERNES
// ============================================================
forward ShowCall(playerid, type, status);
public ShowCall(playerid, type, status)
{
    switch(type)
    {
        case CALLDIAL:  ShowCallDialTD(playerid, status);
        case CALLLIST:  ShowCallListTD(playerid, status);
        case CALLING:   ShowCallingTD(playerid, status);
        default: return 0;
    }
    return 1;
}

forward ShowBank(playerid, status);
public ShowBank(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 21; td++) PlayerTextDrawHide(playerid, TEXTDRAW_BANK[playerid][td]);
        case SHOW: for(new td = 0; td < 19; td++) PlayerTextDrawShow(playerid, TEXTDRAW_BANK[playerid][td]);
    }
    return 1;
}

forward ShowHome(playerid, status);
public ShowHome(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 12; td++) PlayerTextDrawHide(playerid, TEXTDRAW_HOME[playerid][td]);
        case SHOW: for(new td = 0; td < 12; td++) PlayerTextDrawShow(playerid, TEXTDRAW_HOME[playerid][td]);
    }
    return 1;
}

forward ShowNoApps(playerid, status);
public ShowNoApps(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 19; td++) PlayerTextDrawHide(playerid, TEXTDRAW_DEFAULT[playerid][td]);
        case SHOW: for(new td = 0; td < 19; td++) PlayerTextDrawShow(playerid, TEXTDRAW_DEFAULT[playerid][td]);
    }
    return 1;
}

forward ShowNotes(playerid, type, status);
public ShowNotes(playerid, type, status)
{
    switch(type)
    {
        case NOTESTD:     ShowNotesTD(playerid, status);
        case NOTESLISTTD: ShowNotesListTD(playerid, status);
        default: return 0;
    }
    return 1;
}

forward ShowSMS(playerid, status);
public ShowSMS(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 3; td++) PlayerTextDrawHide(playerid, TEXTDRAW_SMS[playerid][td]);
        case SHOW: for(new td = 0; td < 3; td++) PlayerTextDrawShow(playerid, TEXTDRAW_SMS[playerid][td]);
    }
    return 1;
}

forward ShowTime(playerid, status);
public ShowTime(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 5; td++) PlayerTextDrawHide(playerid, TEXTDRAW_TIME[playerid][td]);
        case SHOW: for(new td = 0; td < 5; td++) PlayerTextDrawShow(playerid, TEXTDRAW_TIME[playerid][td]);
    }
    return 1;
}

forward ShowTwitter(playerid, status);
public ShowTwitter(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 16; td++) PlayerTextDrawHide(playerid, TEXTDRAW_TWITTER[playerid][td]);
        case SHOW:
        {
            for(new td = 0; td < 16; td++) PlayerTextDrawShow(playerid, TEXTDRAW_TWITTER[playerid][td]);
            new foo[80];
            mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `twitter_afrp`");
            mysql_tquery(db_handle, foo, "Phone_LoadTwitter", "d", playerid);
        }
    }
    return 1;
}

forward ShowNotification(playerid, status);
public ShowNotification(playerid, status)
{
    switch(status)
    {
        case HIDE: for(new td = 0; td < 4; td++) PlayerTextDrawHide(playerid, TEXTDRAW_NOTIFICATION[playerid][td]);
        case SHOW: for(new td = 0; td < 4; td++) PlayerTextDrawShow(playerid, TEXTDRAW_NOTIFICATION[playerid][td]);
    }
    return 1;
}

forward HidePhone(playerid);
public HidePhone(playerid)
{
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
    return 1;
}

forward CreateShop();
public CreateShop()
{
    for(new ID = 0; ID < sizeof(marketCoordinates); ID++)
    {
        marketPickupID[ID] = CreatePickup(1239, 1,
            marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2], -1);
        Create3DTextLabel(MSG_SHOP" "MSG_SHOP_INFO, 0x33CCFFAA,
            marketCoordinates[ID][0], marketCoordinates[ID][1], marketCoordinates[ID][2], 25, 0, 1);
    }
    return 1;
}

forward OnPlayerEnterShop(playerid, code);
public OnPlayerEnterShop(playerid, code)
{
    switch(code)
    {
        case BUYING:
            ShowPlayerDialog(playerid, MARKET_DIALOG, DIALOG_STYLE_LIST, MSG_SHOP,
                MSG_MOBILE"\n$10 "MSG_CREDIT"\n$20 "MSG_CREDIT"\n$50 "MSG_CREDIT"\n$100 "MSG_CREDIT,
                MSG_BUY, MSG_CANCEL);
        case EXITING:
            SendClientMessage(playerid, -1, MSG_DIALOG_CLOSED);
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
        case BANK_PAYMENT:  CreateNotification(playerid, MSG_PAYMENT, MSG_CHECK_PHONE, -1);
        case CALL_RECEIVED:
        {
            new string[80];
            format(string, 80, MSG_CALL_NOTIF, GetName(receiverid));
            CreateNotification(playerid, string, MSG_CHECK_PHONE, receiverid);
        }
        case UNAVAILABLE:   CreateNotification(playerid, MSG_UNAVAILABLE, MSG_TRY_AGAIN, -1);
        case SMS_RECEIVED:
        {
            new string[80];
            format(string, 80, MSG_SMS_RECEIVED, GetName(receiverid));
            CreateNotification(playerid, string, MSG_CHECK_PHONE, receiverid);
        }
        case NEW_TWEET:     CreateNotification(playerid, MSG_NEW_TWEET, MSG_CHECK_PHONE, -1);
        case PHONE_CREDIT:  CreateNotification(playerid, MSG_CREDIT_OK, MSG_CHECK_PHONE, -1);
        case READY_TO_USE:  CreateNotification(playerid, MSG_PHONE_READY, MSG_PHONE_CMD, -1);
        case NO_PHONE:      CreateNotification(playerid, MSG_ERROR_PHONE, MSG_FORCE_BUY, -1);
        case HAS_PHONE:     CreateNotification(playerid, MSG_HAS_PHONE, MSG_PHONE_CMD, -1);
        case NOT_IN_MARKET: CreateNotification(playerid, MSG_MARKET_ERROR, MSG_TRY_AGAIN, -1);
    }
    return 1;
}

forward CreateNotification(playerid, lineone[], linetwo[], receiverid);
public CreateNotification(playerid, lineone[], linetwo[], receiverid)
{
    new string[120];
    format(string, sizeof(string), "%s~n~%s", lineone, linetwo);
    PlayerTextDrawSetString(playerid, TEXTDRAW_NOTIFICATION[playerid][3], string);
    for(new i = 0; i < 4; i++) PlayerTextDrawShow(playerid, TEXTDRAW_NOTIFICATION[playerid][i]);
    SetTimerEx("Phone_HideNotification", 2000, 0, "d", playerid);
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
        case 0: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_SHTR:bstars"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_SHTR:bstars");
        case 1: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_OTB2:backbet"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_OTB2:backbet");
        case 2: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_GRAV:sky"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_GRAV:sky");
        case 3: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_BEAT:body"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_BEAT:body");
        case 4: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_OTB:gline2"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_OTB:gline2");
        case 5: PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][0], "LD_RACE:7"),
                PlayerTextDrawSetString(playerid, TEXTDRAW_DEFAULT[playerid][1], "LD_RACE:7");
    }
    new foo[120];
    mysql_format(db_handle, foo, sizeof(foo),
        "UPDATE `lvrp_afrp_phone` SET `Background` = %d WHERE `Username` = '%e'",
        background, GetName(playerid));
    mysql_tquery(db_handle, foo);
    return 1;
}

forward ChangeFrame(playerid, frame);
public ChangeFrame(playerid, frame)
{
    new color;
    switch(frame)
    {
        case 0: color = -1;         // Blanc
        case 1: color = 0x007BFFFF; // Bleu
        case 2: color = 0xFF0000FF; // Rouge
        case 3: color = 0x0FF702FF; // Vert
        case 4: color = 0xF8CE02FF; // Jaune
        case 5: color = 0xAA02F7FF; // Violet
        case 6: color = 0xF702BEFF; // Rose
        default: color = -1;
    }
    for(new i = 4; i < 12; i++)
        PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][i], color);
    new foo[120];
    mysql_format(db_handle, foo, sizeof(foo),
        "UPDATE `lvrp_afrp_phone` SET `Frame` = %d WHERE `Username` = '%e'",
        frame, GetName(playerid));
    mysql_tquery(db_handle, foo);
    return 1;
}

// ============================================================
// === CALLBACKS SQL
// ============================================================
forward Phone_LoadUser(playerid);
public Phone_LoadUser(playerid)
{
    if(!IsPlayerConnected(playerid)) return 1;
    new rows = cache_num_rows();
    if(rows)
    {
        cache_get_value_name_int(0, "HasPhone", hasPhone[playerid]);
        cache_get_value_name_int(0, "Number", playerNumber[playerid]);
        cache_get_value_name_int(0, "Credit", playerCredit[playerid]);
    }
    // [FIX] TOUJOURS creer les textdraws meme si le joueur n'a pas (encore) de telephone
    // (sinon /mobile n'ouvre rien apres achat tant qu'il n'a pas reconnect)
    CreateTextDraws(playerid);
    return 1;
}

forward Phone_LoadSettings(playerid);
public Phone_LoadSettings(playerid)
{
    if(!IsPlayerConnected(playerid)) return 1;
    new rows = cache_num_rows();
    if(rows)
    {
        new frameID, backgroundID;
        cache_get_value_name_int(0, "Frame", frameID);
        cache_get_value_name_int(0, "Background", backgroundID);
        ChangeBackground(playerid, backgroundID);
        ChangeFrame(playerid, frameID);
    }
    return 1;
}

forward Phone_LoadTwitter(playerid);
public Phone_LoadTwitter(playerid)
{
    if(!IsPlayerConnected(playerid)) return 1;
    new rows = cache_num_rows(), string[92], tweet = 1;
    if(rows)
    {
        for(new i = 0; i < rows; i++)
        {
            cache_get_value_name_int(i, "TweetID", tweet);
            cache_get_value_name(i, "TweetString", string);
            if(tweet >= 1 && tweet <= 3)
                PlayerTextDrawSetString(playerid, TEXTDRAW_TWITTER[playerid][9+tweet], string);
        }
    }
    return 1;
}

forward Phone_LoadBalance(playerid);
public Phone_LoadBalance(playerid)
{
    if(!IsPlayerConnected(playerid)) return 1;
    new rows = cache_num_rows();
    if(rows)
    {
        new cash, bank;
        cache_get_value_name_int(0, "Cash", cash);
        cache_get_value_name_int(0, "Bank", bank);
        new solde[20], credit_str[10];
        format(solde, sizeof(solde), "$%d", cash + bank);
        format(credit_str, sizeof(credit_str), "$%d", playerCredit[playerid]);
        PlayerTextDrawSetString(playerid, TEXTDRAW_BANK[playerid][1], solde);
        PlayerTextDrawSetString(playerid, TEXTDRAW_BANK[playerid][3], credit_str);
    }
    return 1;
}

// ============================================================
// === UTILITAIRES
// ============================================================
UseMobile(playerid, type, status, additional = 0)
{
    switch(type)
    {
        case BANK:
            if(status == 0) ShowBank(playerid, HIDE); else ShowBank(playerid, SHOW);
        case CALL:
            if(status == 0) ShowCall(playerid, additional, HIDE); else ShowCall(playerid, additional, SHOW);
        case HOME:
            if(status == 0) ShowHome(playerid, HIDE); else ShowHome(playerid, SHOW);
        case NOAPPS:
            if(status == 0) ShowNoApps(playerid, HIDE); else ShowNoApps(playerid, SHOW);
        case NOTES:
            if(status == 0) ShowNotes(playerid, additional, HIDE); else ShowNotes(playerid, additional, SHOW);
        case SMS:
            if(status == 0) ShowSMS(playerid, HIDE); else ShowSMS(playerid, SHOW);
        case TIMELINE:
            if(status == 0) ShowTime(playerid, HIDE);
            else { ShowTime(playerid, SHOW); UpdateTimeDate(playerid, 1); }
        case TWITTER:
            if(status == 0) ShowTwitter(playerid, HIDE); else ShowTwitter(playerid, SHOW);
    }
}


GetName(playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    return name;
}

// ============================================================
// === TEXTDRAWS (CODE ORIGINAL ADAPTE EN FRANCAIS)
// ============================================================

// --- noapps_td (cadre telephone) ---

forward CreatePhoneTD(playerid);
public CreatePhoneTD(playerid)
{
	TEXTDRAW_DEFAULT[playerid][0] = CreatePlayerTextDraw(playerid, 483.666625, 208.925918, "ld_dual:backgnd");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][0], 90.000000, 90.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][0], 0);

	TEXTDRAW_DEFAULT[playerid][1] = CreatePlayerTextDraw(playerid, 483.666809, 298.870361, "ld_dual:backgnd");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][1], 90.000000, 90.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][1], 0);

	TEXTDRAW_DEFAULT[playerid][2] = CreatePlayerTextDraw(playerid, 484.834228, 375.011016, "ld_otb2:butnA");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][2], 87.000000, 17.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][2], 623191551);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][2], 0);

	TEXTDRAW_DEFAULT[playerid][3] = CreatePlayerTextDraw(playerid, 486.333312, 211.155563, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][3], 0.000000, 0.633333);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][3], 601.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][3], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_DEFAULT[playerid][3], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_DEFAULT[playerid][3], 120);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][3], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][3], 1);

	TEXTDRAW_DEFAULT[playerid][4] = CreatePlayerTextDraw(playerid, 481.600036, 206.537063, "ld_drv:tvcorn");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][4], 39.000000, 42.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][4], 0);

	TEXTDRAW_DEFAULT[playerid][5] = CreatePlayerTextDraw(playerid, 545.266967, 206.696212, "ld_drv:tvcorn");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][5], -38.000000, 45.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][5], 0);

	TEXTDRAW_DEFAULT[playerid][6] = CreatePlayerTextDraw(playerid, 545.334106, 391.273956, "ld_drv:tvcorn");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][6], -39.000000, -45.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][6], 0);

	TEXTDRAW_DEFAULT[playerid][7] = CreatePlayerTextDraw(playerid, 481.468383, 391.059204, "ld_drv:tvcorn");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][7], 39.000000, -47.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][7], 0);

	TEXTDRAW_DEFAULT[playerid][8] = CreatePlayerTextDraw(playerid, 520.133361, 206.696289, "ld_drv:tvbase");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][8], 18.000000, 3.250000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][8], 0);

	TEXTDRAW_DEFAULT[playerid][9] = CreatePlayerTextDraw(playerid, 520.333679, 387.396392, "ld_drv:tvbase");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][9], 18.000000, 3.809998);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][9], 0);

	TEXTDRAW_DEFAULT[playerid][10] = CreatePlayerTextDraw(playerid, 481.100585, 247.655212, "ld_drv:tvbase");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][10], 3.459999, 97.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][10], 0);

	TEXTDRAW_DEFAULT[playerid][11] = CreatePlayerTextDraw(playerid, 542.466613, 251.388565, "ld_drv:tvbase");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][11], 2.859998, 97.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][11], 0);

	TEXTDRAW_DEFAULT[playerid][12] = CreatePlayerTextDraw(playerid, 500.333190, 373.577728, "X");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][12], 0.133664, 1.421627);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][12], 535.000000, 373.577728);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][12], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][12], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][12], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_DEFAULT[playerid][12], true);

	TEXTDRAW_DEFAULT[playerid][13] = CreatePlayerTextDraw(playerid, 517.867004, 375.322235, "<");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][13], 0.243664, 1.222517);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][13], 585.000000, 375.322235);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][13], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][13], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][13], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_DEFAULT[playerid][13], true);

	TEXTDRAW_DEFAULT[playerid][14] = CreatePlayerTextDraw(playerid, 524.066284, 374.715148, "O");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][14], 0.243664, 1.222517);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][14], 562.000000, 374.715148);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][14], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][14], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][14], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_DEFAULT[playerid][14], true);

	TEXTDRAW_DEFAULT[playerid][15] = CreatePlayerTextDraw(playerid, 492.999694, 210.725982, "20:20");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][15], 0.152666, 0.670813);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][15], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][15], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][15], 1);

	TEXTDRAW_DEFAULT[playerid][16] = CreatePlayerTextDraw(playerid, 522.332824, 211.355575, "24%");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_DEFAULT[playerid][16], 0.152666, 0.670813);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][16], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][16], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][16], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][16], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][16], 1);

	TEXTDRAW_DEFAULT[playerid][17] = CreatePlayerTextDraw(playerid, 521.066589, 216.577804, "LD_DUAL:THRUSTG");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][17], -4.000000, -5.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][17], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][17], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][17], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][17], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][17], 0);

	TEXTDRAW_DEFAULT[playerid][18] = CreatePlayerTextDraw(playerid, 511.333190, 212.129653, "LD_NONE:WARP");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_DEFAULT[playerid][18], 4.000000, 5.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_DEFAULT[playerid][18], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_DEFAULT[playerid][18], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_DEFAULT[playerid][18], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_DEFAULT[playerid][18], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_DEFAULT[playerid][18], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_DEFAULT[playerid][18], 0);

	new foo[80];
	mysql_format(db_handle, foo, sizeof(foo), "SELECT * FROM `lvrp_afrp_phone` WHERE `Username` = '%s'", GetName(playerid));
	mysql_tquery(db_handle, foo, "Phone_LoadUser", "d", playerid);
	return 1;
}
// --- home_td (ecran accueil) ---

forward CreateHomescreenTD(playerid);
public CreateHomescreenTD(playerid)
{
	TEXTDRAW_HOME[playerid][0] = CreatePlayerTextDraw(playerid, 494.632812, 260.142456, "LD_BEAt:cHIT");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][0], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][0], -5963521);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][0], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][0], true);

	TEXTDRAW_HOME[playerid][1] = CreatePlayerTextDraw(playerid, 517.066040, 230.540634, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][1], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][1], -1061109505);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][1], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][1], true);

	TEXTDRAW_HOME[playerid][2] = CreatePlayerTextDraw(playerid, 509.060668, 230.540634, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][2], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][2], 65535);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][2], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][2], true);

	TEXTDRAW_HOME[playerid][3] = CreatePlayerTextDraw(playerid, 509.060668, 259.342376, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][3], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][3], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][3], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][3], true);

	TEXTDRAW_HOME[playerid][4] = CreatePlayerTextDraw(playerid, 517.066040, 259.442382, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][4], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][4], -2147450625);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][4], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][4], true);

	TEXTDRAW_HOME[playerid][5] = CreatePlayerTextDraw(playerid, 494.632812, 230.540649, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][5], 23.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][5], 623191551);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][5], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][5], true);

	TEXTDRAW_HOME[playerid][6] = CreatePlayerTextDraw(playerid, 502.499206, 267.452148, "hud:radar_qmark");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][6], 7.000000, 12.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][6], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][6], true); // [APP PARAMETRES]

	TEXTDRAW_HOME[playerid][7] = CreatePlayerTextDraw(playerid, 524.499938, 238.703826, "HUD:radar_cash");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][7], 9.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][7], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_HOME[playerid][7], true); // [APP TRADING]

	TEXTDRAW_HOME[playerid][8] = CreatePlayerTextDraw(playerid, 522.499145, 266.007720, "ld_grav:timer");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_HOME[playerid][8], 12.000000, 12.479988);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][8], 0);

	TEXTDRAW_HOME[playerid][9] = CreatePlayerTextDraw(playerid, 515.232910, 240.451766, "SMS");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][9], 0.157999, 0.720592);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][9], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][9], 1);

	TEXTDRAW_HOME[playerid][10] = CreatePlayerTextDraw(playerid, 517.999755, 261.777740, "C");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][10], 0.211999, 2.267852);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][10], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][10], 1);


	TEXTDRAW_HOME[playerid][11] = CreatePlayerTextDraw(playerid, 500.999877, 235.744445, "X");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_HOME[playerid][11], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_HOME[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_HOME[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_HOME[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_HOME[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_HOME[playerid][11], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_HOME[playerid][11], 1);
	return 1;
}
// --- bank_td (application banque) ---

forward CreateBankTD(playerid);
public CreateBankTD(playerid)
{
	TEXTDRAW_BANK[playerid][0] = CreatePlayerTextDraw(playerid, 489.000000, 230.237045, "solde_du_compte");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][0], 0.195666, 1.052443);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][0], 1);

	TEXTDRAW_BANK[playerid][1] = CreatePlayerTextDraw(playerid, 529.333190, 243.511138, "$900000");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][1], 0.344332, 1.400889);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][1], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_BANK[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][1], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][1], 1);

	TEXTDRAW_BANK[playerid][2] = CreatePlayerTextDraw(playerid, 505.333251, 277.111114, "votre_credit");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][2], 0.195666, 1.052443);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][2], 1);

	TEXTDRAW_BANK[playerid][3] = CreatePlayerTextDraw(playerid, 529.000183, 288.725982, "$0");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][3], 0.344332, 1.400889);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][3], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][3], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_BANK[playerid][3], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][3], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][3], 1);

	TEXTDRAW_BANK[playerid][4] = CreatePlayerTextDraw(playerid, 491.666625, 309.881530, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][4], 0.000000, 6.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][4], 594.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][4], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_BANK[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][4], 1);

	TEXTDRAW_BANK[playerid][5] = CreatePlayerTextDraw(playerid, 493.999938, 347.888916, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][5], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][5], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][5], 0);

	TEXTDRAW_BANK[playerid][6] = CreatePlayerTextDraw(playerid, 506.666564, 337.518493, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][6], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][6], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][6], 0);

	TEXTDRAW_BANK[playerid][7] = CreatePlayerTextDraw(playerid, 515.999877, 344.570343, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][7], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][7], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][7], 0);

	TEXTDRAW_BANK[playerid][8] = CreatePlayerTextDraw(playerid, 527.666503, 333.370300, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][8], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][8], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][8], 0);

	TEXTDRAW_BANK[playerid][9] = CreatePlayerTextDraw(playerid, 506.999877, 336.274017, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][9], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][9], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][9], 0);

	TEXTDRAW_BANK[playerid][10] = CreatePlayerTextDraw(playerid, 513.299804, 327.477661, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][10], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][10], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][10], 0);

	TEXTDRAW_BANK[playerid][11] = CreatePlayerTextDraw(playerid, 520.266601, 340.692657, "ld_beat:chit"); // Pad
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][11], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][11], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][11], 0);

	TEXTDRAW_BANK[playerid][12] = CreatePlayerTextDraw(playerid, 492.000244, 309.051879, "grafik");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_BANK[playerid][12], 0.125666, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][12], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][12], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][12], 1);

	TEXTDRAW_BANK[playerid][13] = CreatePlayerTextDraw(playerid, 505.367004, 323.069824, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][13], 16.000000, 26.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][13], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][13], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][13], 19445);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][13], 0);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][13], 180.000000, 45.000000, 0.000000, 1.000000);

	TEXTDRAW_BANK[playerid][14] = CreatePlayerTextDraw(playerid, 514.099853, 330.722290, ""); // Pad
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][14], 13.000000, 16.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][14], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][14], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][14], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][14], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][14], -90.000000, 0.000000, 30.000000, 1.000000);

	TEXTDRAW_BANK[playerid][15] = CreatePlayerTextDraw(playerid, 511.565368, 332.630126, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][15], 6.000000, 26.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][15], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][15], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][15], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][15], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][15], -90.000000, 0.000000, 76.000000, 1.000000);

	TEXTDRAW_BANK[playerid][16] = CreatePlayerTextDraw(playerid, 502.899169, 331.544738, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][16], 6.000000, 16.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][16], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][16], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][16], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][16], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][16], -90.000000, 0.000000, -100.000000, 0.800000);

	TEXTDRAW_BANK[playerid][17] = CreatePlayerTextDraw(playerid, 520.066223, 328.310791, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][17], 13.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][17], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][17], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][17], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][17], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][17], -90.000000, 0.000000, -65.000000, 1.000000);

	TEXTDRAW_BANK[playerid][18] = CreatePlayerTextDraw(playerid, 496.299682, 333.088562, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][18], 16.000000, 27.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][18], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][18], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][18], 5);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][18], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][18], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][18], -90.000000, 0.000000, -65.000000, 1.000000);

	// Rast
	TEXTDRAW_BANK[playerid][19] = CreatePlayerTextDraw(playerid, 520.533264, 317.807312, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][19], 7.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][19], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][19], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][19], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][19], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][19], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][19], 0);

	TEXTDRAW_BANK[playerid][20] = CreatePlayerTextDraw(playerid, 512.632934, 321.633636, "");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_BANK[playerid][20], 14.000000, 12.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_BANK[playerid][20], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_BANK[playerid][20], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawFont(playerid, TEXTDRAW_BANK[playerid][20], 5);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_BANK[playerid][20], 0);
	PlayerTextDrawSetPreviewModel(playerid, TEXTDRAW_BANK[playerid][20], 19445);
	PlayerTextDrawSetPreviewRot(playerid, TEXTDRAW_BANK[playerid][20], -90.000000, 0.000000, -30.000000, 0.800000);
	return 1;
}
// --- call_td (application appels) ---

forward CreateCallDialTD(playerid);
public CreateCallDialTD(playerid)
{
	TEXTDRAW_CALLDIAL[playerid][0] = CreatePlayerTextDraw(playerid, 486.766845, 254.555511, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][0], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][0], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][0], true);

	TEXTDRAW_CALLDIAL[playerid][1] = CreatePlayerTextDraw(playerid, 486.433288, 286.140625, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][1], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][1], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][1], true);

	TEXTDRAW_CALLDIAL[playerid][2] = CreatePlayerTextDraw(playerid, 486.666625, 318.281402, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][2], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][2], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][2], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][2], true);

	TEXTDRAW_CALLDIAL[playerid][3] = CreatePlayerTextDraw(playerid, 490.132385, 353.078002, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][3], 18.000000, 22.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][3], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][3], true);

	TEXTDRAW_CALLDIAL[playerid][4] = CreatePlayerTextDraw(playerid, 514.699951, 254.296325, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][4], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][4], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][4], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][4], true);

	TEXTDRAW_CALLDIAL[playerid][5] = CreatePlayerTextDraw(playerid, 514.699951, 285.798248, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][5], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][5], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][5], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][5], true);

	TEXTDRAW_CALLDIAL[playerid][6] = CreatePlayerTextDraw(playerid, 514.699951, 318.500244, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][6], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][6], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][6], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][6], true);

	TEXTDRAW_CALLDIAL[playerid][7] = CreatePlayerTextDraw(playerid, 515.192504, 318.500244, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][7], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][7], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][7], true);

	TEXTDRAW_CALLDIAL[playerid][8] = CreatePlayerTextDraw(playerid, 515.192504, 286.298278, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][8], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][8], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][8], true);

	TEXTDRAW_CALLDIAL[playerid][9] = CreatePlayerTextDraw(playerid, 515.192504, 254.096313, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][9], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][9], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][9], true);

	TEXTDRAW_CALLDIAL[playerid][10] = CreatePlayerTextDraw(playerid, 514.699951, 347.201995, "ld_beat:Chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][10], 25.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][10], 8388863);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][10], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLDIAL[playerid][10], true);

	TEXTDRAW_CALLDIAL[playerid][11] = CreatePlayerTextDraw(playerid, 495.599914, 260.288909, "1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][11], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][11], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][11], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][11], 1);

	TEXTDRAW_CALLDIAL[playerid][12] = CreatePlayerTextDraw(playerid, 523.333374, 260.159301, "2");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][12], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][12], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][12], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][12], 1);

	TEXTDRAW_CALLDIAL[playerid][13] = CreatePlayerTextDraw(playerid, 523.799926, 260.159301, "3");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][13], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][13], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][13], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][13], 1);

	TEXTDRAW_CALLDIAL[playerid][14] = CreatePlayerTextDraw(playerid, 494.799804, 292.644439, "4");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][14], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][14], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][14], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][14], 1);

	TEXTDRAW_CALLDIAL[playerid][15] = CreatePlayerTextDraw(playerid, 523.766418, 292.229614, "5");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][15], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][15], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][15], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][15], 1);

	TEXTDRAW_CALLDIAL[playerid][16] = CreatePlayerTextDraw(playerid, 523.832824, 292.659210, "6");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][16], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][16], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][16], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][16], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][16], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][16], 1);

	TEXTDRAW_CALLDIAL[playerid][17] = CreatePlayerTextDraw(playerid, 495.599426, 324.685089, "7");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][17], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][17], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][17], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][17], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][17], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][17], 1);

	TEXTDRAW_CALLDIAL[playerid][18] = CreatePlayerTextDraw(playerid, 523.432495, 325.099914, "8");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][18], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][18], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][18], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][18], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][18], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][18], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][18], 1);

	TEXTDRAW_CALLDIAL[playerid][19] = CreatePlayerTextDraw(playerid, 523.765808, 325.559143, "9");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][19], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][19], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][19], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][19], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][19], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][19], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][19], 1);

	TEXTDRAW_CALLDIAL[playerid][20] = CreatePlayerTextDraw(playerid, 496.065734, 357.985168, "0");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][20], 0.323666, 1.313776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLDIAL[playerid][20], -33.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][20], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][20], 255);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][20], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][20], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][20], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][20], 1);

	TEXTDRAW_CALLDIAL[playerid][21] = CreatePlayerTextDraw(playerid, 528.000061, 231.611068, "060060060");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLDIAL[playerid][21], 0.350998, 1.446517);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLDIAL[playerid][21], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLDIAL[playerid][21], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLDIAL[playerid][21], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_CALLDIAL[playerid][21], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLDIAL[playerid][21], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLDIAL[playerid][21], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLDIAL[playerid][21], 1);
	return 1;
}

forward CreateCallListTD(playerid);
public CreateCallListTD(playerid)
{
	TEXTDRAW_CALLLIST[playerid][0] = CreatePlayerTextDraw(playerid, 496.666687, 231.481491, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][0], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][0], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][0], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][0], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][0], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][0], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][0], 1);

	TEXTDRAW_CALLLIST[playerid][1] = CreatePlayerTextDraw(playerid, 496.666687, 245.982376, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][1], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][1], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][1], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][1], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][1], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][1], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][1], 1);

	TEXTDRAW_CALLLIST[playerid][2] = CreatePlayerTextDraw(playerid, 496.666687, 260.583251, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][2], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][2], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][2], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][2], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][2], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][2], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][2], 1);

	TEXTDRAW_CALLLIST[playerid][3] = CreatePlayerTextDraw(playerid, 496.666687, 275.384155, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][3], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][3], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][3], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][3], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][3], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][3], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][3], 1);

	TEXTDRAW_CALLLIST[playerid][4] = CreatePlayerTextDraw(playerid, 496.666687, 290.285064, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][4], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][4], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][4], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][4], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][4], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][4], 1);

	TEXTDRAW_CALLLIST[playerid][5] = CreatePlayerTextDraw(playerid, 496.666687, 305.185974, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][5], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][5], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][5], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][5], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][5], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][5], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][5], 1);

	TEXTDRAW_CALLLIST[playerid][6] = CreatePlayerTextDraw(playerid, 496.666687, 320.086883, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][6], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][6], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][6], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][6], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][6], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][6], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][6], 1);

	TEXTDRAW_CALLLIST[playerid][7] = CreatePlayerTextDraw(playerid, 496.666687, 335.087799, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][7], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][7], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][7], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][7], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][7], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][7], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][7], 1);

	TEXTDRAW_CALLLIST[playerid][8] = CreatePlayerTextDraw(playerid, 496.666687, 349.888702, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][8], 0.000000, 1.099998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][8], 590.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][8], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_CALLLIST[playerid][8], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_CALLLIST[playerid][8], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][8], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][8], 1);

	TEXTDRAW_CALLLIST[playerid][9] = CreatePlayerTextDraw(playerid, 500.333435, 363.937011, "ld_beat:right");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][9], 11.000000, 9.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][9], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][9], true);

	TEXTDRAW_CALLLIST[playerid][10] = CreatePlayerTextDraw(playerid, 499.199768, 233.040756, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][10], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][10], 529+52, 233.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][10], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][10], 1);

	TEXTDRAW_CALLLIST[playerid][11] = CreatePlayerTextDraw(playerid, 498.866455, 246.926818, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][11], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][11], 528.666625+52, 246.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][11], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][11], 1);

	TEXTDRAW_CALLLIST[playerid][12] = CreatePlayerTextDraw(playerid, 498.866455, 262.127746, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][12], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][12], 528.666625+52, 262.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][12], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][12], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][12], 1);

	TEXTDRAW_CALLLIST[playerid][13] = CreatePlayerTextDraw(playerid, 498.866455, 277.128662, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][13], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][13], 528.666625+52, 277.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][13], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][13], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][13], 1);

	TEXTDRAW_CALLLIST[playerid][14] = CreatePlayerTextDraw(playerid, 498.866455, 292.129577, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][14], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][14], 528.666625+52, 292.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][14], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][14], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][14], 1);

	TEXTDRAW_CALLLIST[playerid][15] = CreatePlayerTextDraw(playerid, 498.866455, 306.930480, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][15], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][15], 528.666625+52, 306.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][15], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][15], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][15], 1);

	TEXTDRAW_CALLLIST[playerid][16] = CreatePlayerTextDraw(playerid, 498.866455, 321.531372, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][16], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][16], 528.666625+52, 321.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][16], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][16], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][16], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][16], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][16], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][16], 1);

	TEXTDRAW_CALLLIST[playerid][17] = CreatePlayerTextDraw(playerid, 498.866455, 336.532287, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][17], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][17], 528.666625+52, 336.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][17], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][17], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][17], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][17], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][17], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][17], 1);

	TEXTDRAW_CALLLIST[playerid][18] = CreatePlayerTextDraw(playerid, 498.866455, 351.233184, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLLIST[playerid][18], 0.136665, 0.641776);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][18], 528.666625+52, 351.007385);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][18], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][18], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][18], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][18], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][18], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][18], 1);

	TEXTDRAW_CALLLIST[playerid][19] = CreatePlayerTextDraw(playerid, 521.299682, 232.885192, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][19], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][19], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][19], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][19], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][19], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][19], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][19], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][19], true);

	TEXTDRAW_CALLLIST[playerid][20] = CreatePlayerTextDraw(playerid, 521.299682, 247.286071, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][20], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][20], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][20], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][20], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][20], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][20], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][20], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][20], true);

	TEXTDRAW_CALLLIST[playerid][21] = CreatePlayerTextDraw(playerid, 521.299682, 262.086975, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][21], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][21], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][21], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][21], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][21], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][21], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][21], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][21], true);

	TEXTDRAW_CALLLIST[playerid][22] = CreatePlayerTextDraw(playerid, 521.299682, 276.987884, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][22], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][22], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][22], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][22], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][22], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][22], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][22], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][22], true);

	TEXTDRAW_CALLLIST[playerid][23] = CreatePlayerTextDraw(playerid, 521.299682, 291.788787, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][23], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][23], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][23], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][23], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][23], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][23], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][23], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][23], true);

	TEXTDRAW_CALLLIST[playerid][24] = CreatePlayerTextDraw(playerid, 521.299682, 306.389678, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][24], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][24], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][24], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][24], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][24], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][24], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][24], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][24], true);

	TEXTDRAW_CALLLIST[playerid][25] = CreatePlayerTextDraw(playerid, 521.299682, 321.690612, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][25], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][25], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][25], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][25], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][25], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][25], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][25], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][25], true);

	TEXTDRAW_CALLLIST[playerid][26] = CreatePlayerTextDraw(playerid, 521.299682, 336.091491, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][26], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][26], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][26], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][26], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][26], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][26], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][26], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][26], true);

	TEXTDRAW_CALLLIST[playerid][27] = CreatePlayerTextDraw(playerid, 521.299682, 350.992401, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][27], 6.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][27], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][27], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][27], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][27], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][27], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][27], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][27], true);

	TEXTDRAW_CALLLIST[playerid][28] = CreatePlayerTextDraw(playerid, 513.901306, 363.936981, "ld_beat:left");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLLIST[playerid][28], 11.000000, 9.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLLIST[playerid][28], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLLIST[playerid][28], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLLIST[playerid][28], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLLIST[playerid][28], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLLIST[playerid][28], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLLIST[playerid][28], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLLIST[playerid][28], true);	
	return 1;
}

forward CreateCallingTD(playerid);
public CreateCallingTD(playerid)
{
	TEXTDRAW_CALLING[playerid][0] = CreatePlayerTextDraw(playerid, 514.666748, 333.370422, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_CALLING[playerid][0], 26.000000, 29.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLING[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLING[playerid][0], -16776961);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLING[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLING[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLING[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLING[playerid][0], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_CALLING[playerid][0], true);

	TEXTDRAW_CALLING[playerid][1] = CreatePlayerTextDraw(playerid, 529.099792, 253.466613, "060606060");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLING[playerid][1], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLING[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLING[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLING[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_CALLING[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLING[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLING[playerid][1], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLING[playerid][1], 1);

	TEXTDRAW_CALLING[playerid][2] = CreatePlayerTextDraw(playerid, 529.666931, 270.473968, "ime_prezime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLING[playerid][2], 0.214331, 0.791109);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLING[playerid][2], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLING[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLING[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_CALLING[playerid][2], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLING[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLING[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLING[playerid][2], 1);

	TEXTDRAW_CALLING[playerid][3] = CreatePlayerTextDraw(playerid, 528.766479, 307.392517, "da_vrsite_razgovor~n~pisite_vase_poruke~n~u_chat");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLING[playerid][3], 0.126000, 0.757924);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLING[playerid][3], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLING[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLING[playerid][3], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_CALLING[playerid][3], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLING[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLING[playerid][3], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLING[playerid][3], 1);

	TEXTDRAW_CALLING[playerid][4] = CreatePlayerTextDraw(playerid, 525.866699, 327.448089, "c");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_CALLING[playerid][4], 0.242331, 3.653331);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_CALLING[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_CALLING[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_CALLING[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_CALLING[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_CALLING[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_CALLING[playerid][4], 1);
	return 1;
}

forward ShowCallDialTD(playerid, status);
public ShowCallDialTD(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 22; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_CALLDIAL[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 22; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_CALLDIAL[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowCallListTD(playerid, status);
public ShowCallListTD(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 29; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_CALLLIST[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 29; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_CALLLIST[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowCallingTD(playerid, status);
public ShowCallingTD(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 5; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_CALLING[playerid][td]);
			}
		}

		case SHOW: // shows textdraws
		{
			for(new td = 0; td < 5; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_CALLING[playerid][td]);
			}
		}
	}
	return 1;
}
// --- notes_td (application notes) ---

forward CreateNotesTD(playerid);
public CreateNotesTD(playerid)
{
	TEXTDRAW_NOTES[playerid][0] = CreatePlayerTextDraw(playerid, 489.000000, 226.088912, "biljesk_ime");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][0], 0.192665, 0.998517);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][0], 1);

	TEXTDRAW_NOTES[playerid][1] = CreatePlayerTextDraw(playerid, 489.333557, 239.362991, "linija_1_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][1], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][1], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][1], 1);

	TEXTDRAW_NOTES[playerid][2] = CreatePlayerTextDraw(playerid, 489.333557, 243.263229, "linija_2_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][2], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][2], 1);

	TEXTDRAW_NOTES[playerid][3] = CreatePlayerTextDraw(playerid, 489.266967, 247.448699, "linija_3_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][3], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][3], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][3], 1);

	TEXTDRAW_NOTES[playerid][4] = CreatePlayerTextDraw(playerid, 489.366943, 251.763748, "linija_4_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][4], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][4], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][4], 1);

	TEXTDRAW_NOTES[playerid][5] = CreatePlayerTextDraw(playerid, 489.366943, 255.964004, "linija_5_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][5], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][5], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][5], 1);

	TEXTDRAW_NOTES[playerid][6] = CreatePlayerTextDraw(playerid, 489.366943, 260.064239, "linija_6_proba_text_jedan_dva");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][6], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][6], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][6], 1);

	TEXTDRAW_NOTES[playerid][7] = CreatePlayerTextDraw(playerid, 528.800476, 271.678894, "poslije_stiskanja_na_'dodaj',~n~napisite_sljedecu_liniju vase_biljeske");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][7], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][7], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][7], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][7], 1);

	TEXTDRAW_NOTES[playerid][8] = CreatePlayerTextDraw(playerid, 507.000000, 344.311126, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][8], 0.000000, 1.299998);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTES[playerid][8], 577.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][8], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_NOTES[playerid][8], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_NOTES[playerid][8], 200);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][8], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][8], 1);

	TEXTDRAW_NOTES[playerid][9] = CreatePlayerTextDraw(playerid, 510.966613, 344.496246, "dodaj");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][9], 0.241666, 0.898962);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTES[playerid][9], 547.966613, 344.496246);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][9], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][9], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTES[playerid][9], true);

	TEXTDRAW_NOTES[playerid][10] = CreatePlayerTextDraw(playerid, 527.366882, 320.627258, "kada_zavrsite_sa_biljeskom~n~u_chat_upisite~n~'biljeska_end'");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTES[playerid][10], 0.112999, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTES[playerid][10], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTES[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTES[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTES[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTES[playerid][10], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTES[playerid][10], 1);
	return 1;
}

forward CreateNotesListTD(playerid);
public CreateNotesListTD(playerid)
{
	TEXTDRAW_NOTESLIST[playerid][0] = CreatePlayerTextDraw(playerid, 520.001000, 236.874114+3, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][0], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][0], 520.001000+52, 236.874114+3);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][0], 1);

	TEXTDRAW_NOTESLIST[playerid][1] = CreatePlayerTextDraw(playerid, 520.002000, 246.374450+6, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][1], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][1], 520.002000+52, 246.374450+6);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][1], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][1], 1);

	TEXTDRAW_NOTESLIST[playerid][2] = CreatePlayerTextDraw(playerid, 520.003000, 256.774780+9, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][2], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][2], 520.003000+52, 256.774780+9);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][2], 1);

	TEXTDRAW_NOTESLIST[playerid][3] = CreatePlayerTextDraw(playerid, 520.004000, 266+12, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][3], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][3], 520.004000+52, 266+12);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][3], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][3], 1);

	TEXTDRAW_NOTESLIST[playerid][4] = CreatePlayerTextDraw(playerid, 520.005000, 276+15, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][4], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][4], 520.005000+52, 276+15);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][4], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][4], 1);

	TEXTDRAW_NOTESLIST[playerid][5] = CreatePlayerTextDraw(playerid, 520.006000, 286+18, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][5], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][5], 520.006000+52, 286+18);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][5], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][5], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][5], 1);

	TEXTDRAW_NOTESLIST[playerid][6] = CreatePlayerTextDraw(playerid, 520.007000, 296+21, "biljesk_ime1");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][6], 0.204333, 0.766222);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][6], 520.007000+52, 296+21);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][6], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][6], 1);

	TEXTDRAW_NOTESLIST[playerid][7] = CreatePlayerTextDraw(playerid, 502.999938, 344.725982, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][7], 0.000000, 1.833333);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][7], 583.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][7], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_NOTESLIST[playerid][7], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_NOTESLIST[playerid][7], 200);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][7], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][7], 1);

	TEXTDRAW_NOTESLIST[playerid][8] = CreatePlayerTextDraw(playerid, 527.966491, 350.033264, "dodaj_biljesku");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTESLIST[playerid][8], 0.146332, 0.658370);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][8], 15, 350.033264);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][8], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][8], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][8], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][8], true);

	TEXTDRAW_NOTESLIST[playerid][9] = CreatePlayerTextDraw(playerid, 519.999877, 239.322235, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][9], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][9], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][9], true);

	TEXTDRAW_NOTESLIST[playerid][10] = CreatePlayerTextDraw(playerid, 519.999877, 252.123016, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][10], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][10], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][10], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][10], true);

	TEXTDRAW_NOTESLIST[playerid][11] = CreatePlayerTextDraw(playerid, 519.999877, 264.423767, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][11], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][11], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][11], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][11], true);

	TEXTDRAW_NOTESLIST[playerid][12] = CreatePlayerTextDraw(playerid, 519.999877, 278.124603, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][12], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][12], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][12], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][12], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][12], true);

	TEXTDRAW_NOTESLIST[playerid][13] = CreatePlayerTextDraw(playerid, 519.999877, 291.225402, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][13], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][13], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][13], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][13], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][13], true);

	TEXTDRAW_NOTESLIST[playerid][14] = CreatePlayerTextDraw(playerid, 519.999877, 303.926177, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][14], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][14], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][14], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][14], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][14], true);

	TEXTDRAW_NOTESLIST[playerid][15] = CreatePlayerTextDraw(playerid, 519.999877, 317.427001, "ld_chat:thumbup");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTESLIST[playerid][15], 6.000000, 8.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTESLIST[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTESLIST[playerid][15], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTESLIST[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTESLIST[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTESLIST[playerid][15], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTESLIST[playerid][15], 0);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_NOTESLIST[playerid][15], true);	
	return 1;
}

forward ShowNotesTD(playerid, status);
public ShowNotesTD(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 11; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_NOTES[playerid][td]);
			}
		}

		case SHOW: // hides textdraws
		{
			for(new td = 0; td < 11; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_NOTES[playerid][td]);
			}
		}
	}
	return 1;
}

forward ShowNotesListTD(playerid, status);
public ShowNotesListTD(playerid, status)
{
	switch(status)
	{
		case HIDE: // hides textdraws
		{
			for(new td = 0; td < 16; td++)
			{
				PlayerTextDrawHide(playerid, TEXTDRAW_NOTESLIST[playerid][td]);
			}
		}

		case SHOW: // hides textdraws
		{
			for(new td = 0; td < 16; td++)
			{
				PlayerTextDrawShow(playerid, TEXTDRAW_NOTESLIST[playerid][td]);
			}
		}
	}
	return 1;
}
// --- notification_td ---

forward CreateNotificationTD(playerid);
public CreateNotificationTD(playerid)
{
	TEXTDRAW_NOTIFICATION[playerid][0] = CreatePlayerTextDraw(playerid, 470.599975, 176.759078, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 0.000000, 2.233333);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 613.600097, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTIFICATION[playerid][0], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 140);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTIFICATION[playerid][0], 1);

	TEXTDRAW_NOTIFICATION[playerid][1] = CreatePlayerTextDraw(playerid, 472.466766, 183.207412, "ld_chat:badchat");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 7.000000, 7.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTIFICATION[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTIFICATION[playerid][1], 0);

	TEXTDRAW_NOTIFICATION[playerid][2] = CreatePlayerTextDraw(playerid, 471.999847, 172.977813, "~b~AFRP_~w~NOTIFICATION");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 0.135000, 0.562961);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTIFICATION[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTIFICATION[playerid][2], 1);

	TEXTDRAW_NOTIFICATION[playerid][3] = CreatePlayerTextDraw(playerid, 482.566467, 181.888870, "Notification_telephone");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 0.123666, 0.509037);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_NOTIFICATION[playerid][3], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_NOTIFICATION[playerid][3], 1);
	return 1;
}
// --- sms_td (application SMS) ---

forward CreateSMSTD(playerid);
public CreateSMSTD(playerid)
{
	TEXTDRAW_SMS[playerid][0] = CreatePlayerTextDraw(playerid, 529.666870, 289.140808, "ecrivez_votre_message~n~dans_le_chat_pour_l_envoyer");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][0], 0.113333, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][0], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][0], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][0], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][0], 1);

	TEXTDRAW_SMS[playerid][1] = CreatePlayerTextDraw(playerid, 529.666870, 238.118499, "06060606");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][1], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][1], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][1], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][1], 3);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][1], 1);

	TEXTDRAW_SMS[playerid][2] = CreatePlayerTextDraw(playerid, 500.666625, 324.400085, "pour_annuler_ecrivez~n~sms_exit~n~dans_le_chat");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_SMS[playerid][2], 0.113333, 0.749629);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_SMS[playerid][2], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_SMS[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_SMS[playerid][2], 0);
	PlayerTextDrawSetOutline(playerid, TEXTDRAW_SMS[playerid][2], 1);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_SMS[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_SMS[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_SMS[playerid][2], 1);
	return 1;
}

// TEXTDRAW_SMS[playerid][1] da bude clickable i da mozes upisati broj
// --- time_td (application horloge) ---

forward CreateTimeTD(playerid);
public CreateTimeTD(playerid)
{
	TEXTDRAW_TIME[playerid][0] = CreatePlayerTextDraw(playerid, 499.666687, 265.755554, "ld_grav:leaf");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TIME[playerid][0], 58.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TIME[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TIME[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TIME[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TIME[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TIME[playerid][0], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TIME[playerid][0], 0);

	TEXTDRAW_TIME[playerid][1] = CreatePlayerTextDraw(playerid, 529.333312, 250.562988, "12/12/2024");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TIME[playerid][1], 0.224000, 1.143702);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TIME[playerid][1], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_TIME[playerid][1], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TIME[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TIME[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TIME[playerid][1], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TIME[playerid][1], 1);

	TEXTDRAW_TIME[playerid][2] = CreatePlayerTextDraw(playerid, 528.999938, 307.392608, "20:20:20");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TIME[playerid][2], 0.224000, 1.143702);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TIME[playerid][2], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_TIME[playerid][2], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TIME[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TIME[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TIME[playerid][2], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TIME[playerid][2], 1);

	TEXTDRAW_TIME[playerid][3] = CreatePlayerTextDraw(playerid, 501.000061, 321.940795, "ld_grav:leaf");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TIME[playerid][3], 58.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TIME[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TIME[playerid][3], -2147450625);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TIME[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TIME[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TIME[playerid][3], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TIME[playerid][3], 0);

	TEXTDRAW_TIME[playerid][4] = CreatePlayerTextDraw(playerid, 527.867065, 357.170379, "application_actuelle:~n~horloge");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TIME[playerid][4], 0.093000, 0.525628);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TIME[playerid][4], 0.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TIME[playerid][4], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_TIME[playerid][4], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TIME[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TIME[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TIME[playerid][4], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TIME[playerid][4], 1);
	return 1;
}
// --- twitter_td (application Twitter) ---
               
forward CreateTwitterTD(playerid);
public CreateTwitterTD(playerid)
{
	TEXTDRAW_TWITTER[playerid][0] = CreatePlayerTextDraw(playerid, 523.666625, 220.281478, "x");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][0], 0.400000, 1.600000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][0], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][0], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][0], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][0], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][0], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][0], 1);

	TEXTDRAW_TWITTER[playerid][1] = CreatePlayerTextDraw(playerid, 489.666564, 243.096313, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][1], 0.000000, 2.966666);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][1], 597.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][1], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][1], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_TWITTER[playerid][1], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_TWITTER[playerid][1], 120);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][1], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][1], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][1], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][1], 1);

	TEXTDRAW_TWITTER[playerid][2] = CreatePlayerTextDraw(playerid, 489.666564, 276.098327, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][2], 0.000000, 2.966666);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][2], 597.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][2], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][2], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_TWITTER[playerid][2], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_TWITTER[playerid][2], 120);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][2], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][2], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][2], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][2], 1);

	TEXTDRAW_TWITTER[playerid][3] = CreatePlayerTextDraw(playerid, 489.666564, 309.000335, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][3], 0.000000, 2.966666);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][3], 597.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][3], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][3], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_TWITTER[playerid][3], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_TWITTER[playerid][3], 120);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][3], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][3], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][3], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][3], 1);

	TEXTDRAW_TWITTER[playerid][4] = CreatePlayerTextDraw(playerid, 508.000061, 351.362915, "box");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][4], 0.000000, 1.233332);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][4], 597.000000, 0.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][4], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][4], -1);
	PlayerTextDrawUseBox(playerid, TEXTDRAW_TWITTER[playerid][4], 1);
	PlayerTextDrawBoxColor(playerid, TEXTDRAW_TWITTER[playerid][4], 150);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][4], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][4], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][4], 1);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][4], 1);

	TEXTDRAW_TWITTER[playerid][5] = CreatePlayerTextDraw(playerid, 522.133056, 353.492767, "tweet");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][5], 0.147332, 0.650074);
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][5], 5, 353.492767);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][5], 2);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][5], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][5], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][5], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][5], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][5], 1);
	PlayerTextDrawSetSelectable(playerid, TEXTDRAW_TWITTER[playerid][5], true);

	TEXTDRAW_TWITTER[playerid][6] = CreatePlayerTextDraw(playerid, 489.466613, 344.907714, "poslije_stiskanja~n~dugmeta_tweet,~n~pisite_vasu~n~poruku_u_chat");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][6], 0.101666, 0.571259);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][6], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][6], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][6], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][6], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][6], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][6], 1);

	TEXTDRAW_TWITTER[playerid][7] = CreatePlayerTextDraw(playerid, 487.599975, 241.296234, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][7], 16.000000, 18.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][7], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][7], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][7], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][7], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][7], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][7], 0);

	TEXTDRAW_TWITTER[playerid][8] = CreatePlayerTextDraw(playerid, 487.599975, 273.983428, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][8], 16.000000, 18.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][8], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][8], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][8], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][8], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][8], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][8], 0);

	TEXTDRAW_TWITTER[playerid][9] = CreatePlayerTextDraw(playerid, 487.599975, 306.385406, "ld_beat:chit");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][9], 16.000000, 18.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][9], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][9], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][9], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][9], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][9], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][9], 0);

	TEXTDRAW_TWITTER[playerid][10] = CreatePlayerTextDraw(playerid, 504.133422, 244.725967, " ");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][10], 0.122998, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][10], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][10], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][10], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][10], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][10], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][10], 1);

	TEXTDRAW_TWITTER[playerid][11] = CreatePlayerTextDraw(playerid, 504.133422, 277.427947, " ");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][11], 0.122998, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][11], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][11], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][11], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][11], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][11], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][11], 1);

	TEXTDRAW_TWITTER[playerid][12] = CreatePlayerTextDraw(playerid, 504.133422, 310.429962, " ");
	PlayerTextDrawLetterSize(playerid, TEXTDRAW_TWITTER[playerid][12], 0.122998, 0.625185);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][12], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][12], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][12], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][12], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][12], 2);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][12], 1);

	TEXTDRAW_TWITTER[playerid][13] = CreatePlayerTextDraw(playerid, 490.867187, 245.559173, "ld_none:ship2");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][13], 9.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][13], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][13], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][13], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][13], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][13], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][13], 0);

	TEXTDRAW_TWITTER[playerid][14] = CreatePlayerTextDraw(playerid, 491.067077, 278.314819, "ld_none:ship");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][14], 9.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][14], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][14], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][14], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][14], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][14], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][14], 0);

	TEXTDRAW_TWITTER[playerid][15] = CreatePlayerTextDraw(playerid, 490.899963, 310.988708, "ld_none:Ship3");
	PlayerTextDrawTextSize(playerid, TEXTDRAW_TWITTER[playerid][15], 9.000000, 10.000000);
	PlayerTextDrawAlignment(playerid, TEXTDRAW_TWITTER[playerid][15], 1);
	PlayerTextDrawColor(playerid, TEXTDRAW_TWITTER[playerid][15], -1);
	PlayerTextDrawSetShadow(playerid, TEXTDRAW_TWITTER[playerid][15], 0);
	PlayerTextDrawBackgroundColor(playerid, TEXTDRAW_TWITTER[playerid][15], 255);
	PlayerTextDrawFont(playerid, TEXTDRAW_TWITTER[playerid][15], 4);
	PlayerTextDrawSetProportional(playerid, TEXTDRAW_TWITTER[playerid][15], 0);
	return 1;
}


// [APP TRADING] Callbacks
forward Trading_LoadPortfolio(playerid);
public Trading_LoadPortfolio(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0)
    {
        ShowPlayerDialog(playerid, 9216, DIALOG_STYLE_MSGBOX, "{FFAA00}Mon portefeuille", "{FFFFFF}Vous n'avez aucune position ouverte.\n\nUtilisez le menu Trading pour acheter.", "OK", "");
        return 1;
    }
    new portfolio[800] = "{FFFFFF}Mes positions :\n\n", line[100];
    for(new r = 0; r < rows; r++)
    {
        new sym[10], amount;
        cache_get_value_name(r, "Symbol", sym, 10);
        cache_get_value_name_int(r, "AmountInvested", amount);
        format(line, sizeof(line), "{FFAA00}%s{FFFFFF} : {00FF00}%d${FFFFFF} investis\n", sym, amount);
        strcat(portfolio, line);
    }
    ShowPlayerDialog(playerid, 9216, DIALOG_STYLE_MSGBOX, "{FFAA00}Mon portefeuille", portfolio, "OK", "");
    return 1;
}

forward Trading_SellPosition(playerid, symbol[]);
public Trading_SellPosition(playerid, symbol[])
{
    new rows = cache_num_rows();
    if(rows == 0)
    {
        new errmsg[120];
        format(errmsg, sizeof(errmsg), "{FF0000}>> Trading <<{FFFFFF} Vous n'avez pas de position en %s.", symbol);
        SendClientMessage(playerid, -1, errmsg);
        return 1;
    }
    new amount;
    cache_get_value_name_int(0, "AmountInvested", amount);
    // Variation aleatoire +/- 20% pour simuler la valeur de marche
    new profit = (amount * (80 + random(40))) / 100;
    GivePlayerMoney(playerid, profit);
    new sqlDel[200];
    format(sqlDel, sizeof(sqlDel), "DELETE FROM lvrp_trading_positions WHERE Username='%s' AND Symbol='%s'", GetName(playerid), symbol);
    mysql_tquery(db_handle, sqlDel, "", "");
    new sellmsg[160];
    new diff = profit - amount;
    if(diff >= 0)
        format(sellmsg, sizeof(sellmsg), "{00FF00}>> Trading <<{FFFFFF} Vente de %s : recu %d$ (gain +%d$)", symbol, profit, diff);
    else
        format(sellmsg, sizeof(sellmsg), "{FFAA00}>> Trading <<{FFFFFF} Vente de %s : recu %d$ (perte %d$)", symbol, profit, diff);
    SendClientMessage(playerid, -1, sellmsg);
    return 1;
}


// ====================================================
// === CALLBACKS SQL POUR LES NOUVELLES APPS
// ====================================================

forward Gouv_LoadCasier(playerid);
public Gouv_LoadCasier(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { ShowPlayerDialog(playerid, 9304, DIALOG_STYLE_MSGBOX, "{0066CC}Casier civique", "{FFFFFF}Aucune information disponible.", "OK", ""); return 1; }
    new wanted, warns, member;
    cache_get_value_name_int(0, "Wanted", wanted);
    cache_get_value_name_int(0, "Warns", warns);
    cache_get_value_name_int(0, "Member", member);
    new etat[30], info[400];
    if(wanted >= 6) etat = "{FF0000}MANDAT D'ARRET";
    else if(wanted >= 3) etat = "{FFAA00}Recherche";
    else if(wanted >= 1) etat = "{FFFF00}Soupcons";
    else etat = "{00FF00}Casier vierge";
    new facName[30] = "Aucune";
    if(member == 1) facName = "LSPD";
    else if(member == 2) facName = "SFPD";
    else if(member >= 11 && member <= 13) facName = "Armee";
    else if(member == 8) facName = "Mairie";
    format(info, sizeof(info),
        "{FFFFFF}Mon casier civique :\n\n{FFFFFF}Nom : {FFFF00}%s{FFFFFF}\nFaction : {FFFF00}%s{FFFFFF}\n\n{FFFFFF}Niveau Wanted : %s{FFFFFF} (%d/6)\nAvertissements admin : {FFAA00}%d{FFFFFF}\n\n{888888}Le casier se vide apres 7 jours sans infraction.",
        GetName(playerid), facName, etat, wanted, warns);
    ShowPlayerDialog(playerid, 9304, DIALOG_STYLE_MSGBOX, "{0066CC}Casier civique", info, "OK", "");
    return 1;
}

forward Job_LoadStatus(playerid);
public Job_LoadStatus(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { return 1; }
    new job, jobtime;
    cache_get_value_name_int(0, "Job", job);
    cache_get_value_name_int(0, "JobTime", jobtime);
    new jobName[40] = "Aucun";
    switch(job)
    {
        case 1: jobName = "Routier";
        case 2: jobName = "Taxi";
        case 3: jobName = "Bus";
        case 4: jobName = "Pecheur";
        case 5: jobName = "Bucheron";
        case 6: jobName = "Eboueur";
        case 7: jobName = "Pizzaiolo";
        case 8: jobName = "Mecano";
        case 9: jobName = "Mineur";
        case 10: jobName = "Agriculteur";
    }
    new info[300];
    format(info, sizeof(info),
        "{FFFFFF}Mon job actuel :\n\n{FF6600}Profession{FFFFFF} : {FFFF00}%s\n{FFFFFF}Temps de service : {00FF00}%d minutes\n\n{888888}Tapez /work pour rejoindre votre poste\n/quitjob pour changer de job",
        jobName, jobtime);
    ShowPlayerDialog(playerid, 9313, DIALOG_STYLE_MSGBOX, "{FF6600}Mon job", info, "OK", "");
    return 1;
}

forward Job_LoadProStatus(playerid);
public Job_LoadProStatus(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { return 1; }
    new member, rank, job;
    cache_get_value_name_int(0, "Member", member);
    cache_get_value_name_int(0, "Rank", rank);
    cache_get_value_name_int(0, "Job", job);
    new facName[40] = "Civil";
    if(member == 1) facName = "Police LSPD";
    else if(member == 2) facName = "Police SFPD";
    else if(member == 3) facName = "Police LVPD";
    else if(member == 10) facName = "Pompiers";
    else if(member == 11) facName = "Armee de Terre";
    else if(member == 12) facName = "Armee de l'Air";
    else if(member == 13) facName = "Marine";
    else if(member == 8) facName = "Mairie";
    new info[300];
    format(info, sizeof(info),
        "{FFFFFF}Mon statut professionnel :\n\n{FF6600}Faction{FFFFFF} : {FFFF00}%s\n{FF6600}Rang{FFFFFF} : {FFFF00}%d\n{FF6600}Job civil{FFFFFF} : %s\n\n{888888}Salaire au prochain payday.",
        facName, rank, (job > 0) ? ("Actif") : ("Aucun"));
    ShowPlayerDialog(playerid, 9314, DIALOG_STYLE_MSGBOX, "{FF6600}Statut pro", info, "OK", "");
    return 1;
}

forward Politique_LoadMaire(playerid);
public Politique_LoadMaire(playerid)
{
    new rows = cache_num_rows();
    new maireName[32] = "Aucun maire elu";
    if(rows > 0) cache_get_value_name(0, "Name", maireName, 32);
    new info[300];
    format(info, sizeof(info),
        "{FFFFFF}Conseil municipal AFRP :\n\n{FFD700}Maire actuel{FFFFFF} : {FFFF00}%s\n\n{FFFFFF}Le conseil se reunit le dimanche a 21h.\nDes citoyens peuvent assister aux seances en mairie.\n\n{888888}Le maire est elu pour 1 mois IRL.",
        maireName);
    ShowPlayerDialog(playerid, 9341, DIALOG_STYLE_MSGBOX, "{FFD700}Maire et conseil", info, "OK", "");
    return 1;
}

forward Market_LoadAnnonces(playerid);
public Market_LoadAnnonces(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { ShowPlayerDialog(playerid, 9354, DIALOG_STYLE_MSGBOX, "{32CD32}Marketplace", "{FFFFFF}Aucune annonce active.\nSoyez le premier a poster avec /troc puis option 2 !", "OK", ""); return 1; }
    new list[2000] = "{FFFFFF}ID  Vendeur            Titre               Prix\n", line[150];
    for(new r = 0; r < rows; r++)
    {
        new aid, vendeur[32], titre[64], prix;
        cache_get_value_name_int(r, "id", aid);
        cache_get_value_name(r, "Vendeur", vendeur, 32);
        cache_get_value_name(r, "Titre", titre, 64);
        cache_get_value_name_int(r, "Prix", prix);
        format(line, sizeof(line), "{FFFF00}#%d  {FFFFFF}%s  {00FF00}%s{FFFFFF}  {00FF00}%d${FFFFFF}\n", aid, vendeur, titre, prix);
        strcat(list, line);
    }
    strcat(list, "\n{888888}Contactez le vendeur via /wa ou /sms");
    ShowPlayerDialog(playerid, 9354, DIALOG_STYLE_MSGBOX, "{32CD32}Annonces actives", list, "OK", "");
    return 1;
}

forward Market_LoadMesAnnonces(playerid);
public Market_LoadMesAnnonces(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { ShowPlayerDialog(playerid, 9355, DIALOG_STYLE_MSGBOX, "{32CD32}Mes annonces", "{FFFFFF}Vous n'avez aucune annonce active.", "OK", ""); return 1; }
    new list[1000] = "{FFFFFF}Mes annonces actives :\n\n", line[120];
    for(new r = 0; r < rows; r++)
    {
        new aid, titre[64], prix;
        cache_get_value_name_int(r, "id", aid);
        cache_get_value_name(r, "Titre", titre, 64);
        cache_get_value_name_int(r, "Prix", prix);
        format(line, sizeof(line), "{FFFF00}#%d{FFFFFF} - %s - {00FF00}%d${FFFFFF}\n", aid, titre, prix);
        strcat(list, line);
    }
    strcat(list, "\n{888888}/troc option 4 pour supprimer une annonce");
    ShowPlayerDialog(playerid, 9355, DIALOG_STYLE_MSGBOX, "{32CD32}Mes annonces", list, "OK", "");
    return 1;
}

forward Carte_LoadTurfs(playerid);
public Carte_LoadTurfs(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { ShowPlayerDialog(playerid, 9361, DIALOG_STYLE_MSGBOX, "{00BFFF}Territoires", "{FFFFFF}Aucun territoire gang enregistre.", "OK", ""); return 1; }
    new list[2000] = "{FFFFFF}Territoires de gangs :\n\n", line[150];
    for(new r = 0; r < rows; r++)
    {
        new nm[64], owner[64];
        cache_get_value_name(r, "Name", nm, 64);
        cache_get_value_name(r, "Owner", owner, 64);
        format(line, sizeof(line), "{FFFF00}%s{FFFFFF} - Proprio : {00FF00}%s{FFFFFF}\n", nm, owner);
        strcat(list, line);
    }
    ShowPlayerDialog(playerid, 9361, DIALOG_STYLE_MSGBOX, "{00BFFF}Territoires gangs", list, "OK", "");
    return 1;
}

forward Carte_LoadProtegees(playerid);
public Carte_LoadProtegees(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { return 1; }
    new list[2000] = "{FFFFFF}Zones protegees :\n\n", line[150];
    for(new r = 0; r < rows; r++)
    {
        new nm[64], owner[64], zt;
        cache_get_value_name(r, "Name", nm, 64);
        cache_get_value_name(r, "Owner", owner, 64);
        cache_get_value_name_int(r, "ZoneType", zt);
        new type_str[30] = "Neutre";
        if(zt == 1) type_str = "{00FF00}Police";
        else if(zt == 2) type_str = "{FF1493}Hopital";
        format(line, sizeof(line), "%s{FFFFFF} %s{FFFFFF} - {00FF00}%s\n", type_str, nm, owner);
        strcat(list, line);
    }
    ShowPlayerDialog(playerid, 9364, DIALOG_STYLE_MSGBOX, "{00BFFF}Zones protegees", list, "OK", "");
    return 1;
}

forward Immo_ListHouses(playerid);
public Immo_ListHouses(playerid)
{
    new rows = cache_num_rows();
    if(rows == 0) { ShowPlayerDialog(playerid, 9380, DIALOG_STYLE_MSGBOX, "{8B4513}Immobilier", "{FFFFFF}Aucune maison disponible a la vente actuellement.\n\n{888888}Revenez plus tard.", "OK", ""); return 1; }
    new list[2000] = "{FFFFFF}Maisons a vendre :\n\n", line[160];
    for(new r = 0; r < rows; r++)
    {
        new hid, cout;
        new Float:px, Float:py;
        cache_get_value_name_int(r, "id", hid);
        cache_get_value_name_int(r, "Cout", cout);
        cache_get_value_name_float(r, "Pos_x", px);
        cache_get_value_name_float(r, "Pos_y", py);
        format(line, sizeof(line), "{FFFF00}Maison #%d{FFFFFF} - Prix : {00FF00}%d${FFFFFF} - Pos: %.0f, %.0f\n", hid, cout, px, py);
        strcat(list, line);
    }
    strcat(list, "\n{888888}Rendez-vous sur place et tapez /buyhouse pour acheter.");
    ShowPlayerDialog(playerid, 9380, DIALOG_STYLE_MSGBOX, "{8B4513}Annonces immo", list, "OK", "");
    return 1;
}
