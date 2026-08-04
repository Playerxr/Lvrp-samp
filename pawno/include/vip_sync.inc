// ════════════════════════════════════════════════════════════════════
// vip_sync.inc — Attribution automatique VIP/Cash approuvee par le staff
// ════════════════════════════════════════════════════════════════════
// Toutes les 30s, interroge Firebase (HTTP GET) pour recuperer les VIP/
// argent en attente, ecrits par le staff depuis l'app AFRP apres avoir
// verifie un paiement. Applique le tout dans lvrp_users EN LOCAL (aucune
// connexion MySQL externe : c'est TOUJOURS ce serveur qui ecrit dans sa
// propre base). A inclure APRES la declaration de PlayerInfo (le module
// met a jour la memoire des joueurs en ligne).
//
// Format d'une entree "pending_vip_grants/<pushId>" (chaine) :
//   "Username|DonateRank|Cash|VipMinutes"
//     - DonateRank = 0 si ce n'est pas un achat VIP (sinon 1..4)
//     - Cash       = 0 si ce n'est pas un pack d'argent (sinon montant a AJOUTER)
//     - VipMinutes = duree VIP a AJOUTER (mensuel ~43200, a vie ~2000000000)
//
// EN LIGNE vs HORS LIGNE (evite que l'autosave ecrase le don) :
//   - joueur EN LIGNE  : on met a jour SA MEMOIRE (l'autosave la persiste).
//   - joueur HORS LIGNE: on met a jour la BASE (chargee a sa reconnexion).
//   -> jamais les deux, donc pas de double-credit ni d'ecrasement.
//
// Anti double-credit : chaque pushId traite est enregistre dans une table
// locale (vip_sync_processed) ; s'il est revu, il est ignore. Necessaire
// car Cash/VipTime sont additifs (pas idempotents).
//
// Anti-downgrade : le rang VIP n'est jamais abaisse (GREATEST / max).
// ════════════════════════════════════════════════════════════════════

#define VIP_SYNC_HOST "afrp-f0ef7-default-rtdb.europe-west1.firebasedatabase.app"
#define VIP_SYNC_POLL_MS 30000
#define VIP_TIME_MAX 2000000000   // plafond (evite le depassement d'entier)

// Le a_samp.inc de ce projet ne declare pas HTTP() (natif present depuis
// SA-MP 0.3.7 officiel, confirme sur ce serveur, mais absent de ce header
// tronque). On le declare : ca n'ajoute rien, ca expose ce que le moteur
// sait deja faire.
#if !defined HTTP_GET
enum HTTP_METHOD
{
    HTTP_METHOD_ERROR = -1,
    HTTP_GET,
    HTTP_POST,
    HTTP_HEAD
};
native HTTP(index, HTTP_METHOD:type, const url[], const data[], const callback[]);
#endif

forward VipSync_Poll();
forward VipSync_OnPollResponse(index, response_code, data[]);

VipSync_Init()
{
    mysql_tquery(MYSQL, "CREATE TABLE IF NOT EXISTS vip_sync_processed (push_id VARCHAR(40) PRIMARY KEY, processed_at INT UNSIGNED DEFAULT 0) ENGINE=InnoDB", "", "");
    SetTimer("VipSync_Poll", VIP_SYNC_POLL_MS, true);
    print("[VIPSYNC] Module demarre - verification VIP/Cash toutes les 30s");
}

public VipSync_Poll()
{
    HTTP(1, HTTP_GET, "https://"VIP_SYNC_HOST"/pending_vip_grants.json", "", "VipSync_OnPollResponse");
}

// Reponse Firebase :
//   {"-Nabc123":"Jackson_Smith|1|0|43200","-Nqrs456":"Fatou_M|0|3250000|0"}
// ou "null" s'il n'y a rien en attente.
// [DIAGNOSTIC] Ce module echouait en SILENCE : tout code != 200 sortait sans
// un mot, donc rien dans les logs ne disait que la synchro VIP ne marche pas.
// Or l'appel vise "https://" alors que le natif HTTP() de SA-MP ne sait pas
// faire de TLS - meme cause exacte que l'annonce Discord qui ne partait jamais
// (HTTP_ERROR_BAD_HOST, code 1). On rend l'echec visible sans inonder le log :
// premier echec, puis un rappel tous les 20 essais (~10 min a 30s d'interval).
// Volontairement PAS "repare" ici : contrairement au relais Discord (qu'on
// heberge et qu'on peut servir en HTTP simple), l'hote vise ici est un service
// externe. Le corriger demande de savoir s'il peut repondre en HTTP simple ou
// s'il faut lui aussi passer par un relais PHP - une decision a prendre avec
// la proprietaire, pas a deviner a sa place.
new vipSyncEchecs = 0;

public VipSync_OnPollResponse(index, response_code, data[])
{
    if (response_code != 200)
    {
        if(vipSyncEchecs == 0 || (vipSyncEchecs % 20) == 0)
        {
            if(response_code >= 1 && response_code <= 6)
                printf("[VIPSYNC] Echec synchro (erreur client SA-MP %d) : la requete ne sort meme pas du serveur. L'URL visee est en https://, or SA-MP ne sait pas faire de HTTPS.", response_code);
            else
                printf("[VIPSYNC] Echec synchro (code HTTP %d).", response_code);
        }
        vipSyncEchecs++;
        return 1;
    }
    if(vipSyncEchecs > 0)
    {
        printf("[VIPSYNC] Synchro retablie apres %d echec(s).", vipSyncEchecs);
        vipSyncEchecs = 0;
    }
    if (strcmp(data, "null", true) == 0) return 1;

    new vsPos = 0;
    new vsLen = strlen(data);

    while (vsPos < vsLen)
    {
        new keyStart = strfind(data, "\"", true, vsPos);
        if (keyStart == -1) break;
        new keyEnd = strfind(data, "\"", true, keyStart + 1);
        if (keyEnd == -1) break;

        new pushId[40];
        strmid(pushId, data, keyStart + 1, keyEnd);

        new valStart = strfind(data, "\"", true, keyEnd + 1);
        if (valStart == -1) break;
        new valEnd = strfind(data, "\"", true, valStart + 1);
        if (valEnd == -1) break;

        new valeur[128];
        strmid(valeur, data, valStart + 1, valEnd);

        VipSync_Traiter(pushId, valeur);

        vsPos = valEnd + 1;
    }
    return 1;
}

VipSync_Traiter(const pushId[], const valeur[])
{
    new safePushId[80];
    mysql_escape_string(pushId, safePushId);

    // Deja traite ? (table locale, requete minuscule et indexee)
    new checkSql[160];
    format(checkSql, sizeof(checkSql), "SELECT 1 FROM vip_sync_processed WHERE push_id='%s' LIMIT 1", safePushId);
    mysql_query(MYSQL, checkSql);
    if (cache_num_rows() > 0) return; // deja fait -> on ignore

    // Parse "Username|DonateRank|Cash|VipMinutes" (le 4e champ est optionnel)
    new p1 = strfind(valeur, "|", true, 0);
    new p2 = (p1 != -1) ? strfind(valeur, "|", true, p1 + 1) : -1;
    if (p1 == -1 || p2 == -1) return; // format invalide -> ignore proprement
    new p3 = strfind(valeur, "|", true, p2 + 1);

    new username[32], donateRankStr[8], cashStr[16];
    new vipMin = 0;
    strmid(username, valeur, 0, p1);
    strmid(donateRankStr, valeur, p1 + 1, p2);
    if (p3 == -1) {
        strmid(cashStr, valeur, p2 + 1, strlen(valeur));
    } else {
        strmid(cashStr, valeur, p2 + 1, p3);
        new vipMinStr[16];
        strmid(vipMinStr, valeur, p3 + 1, strlen(valeur));
        vipMin = strval(vipMinStr);
    }

    new donateRank = strval(donateRankStr);
    new cash = strval(cashStr);
    if (donateRank <= 0 && cash <= 0) return; // rien a faire

    new playerid = VipSync_FindPlayerByName(username);

    if (playerid != INVALID_PLAYER_ID)
    {
        // ── EN LIGNE : on modifie la memoire, l'autosave la persistera ──
        if (donateRank > 0)
        {
            if (donateRank > PlayerInfo[playerid][pDonateRank])   // jamais de downgrade
                PlayerInfo[playerid][pDonateRank] = donateRank;
            // Ajoute le temps VIP (plafonne pour eviter le depassement)
            if (PlayerInfo[playerid][pVipTime] > VIP_TIME_MAX - vipMin)
                PlayerInfo[playerid][pVipTime] = VIP_TIME_MAX;
            else
                PlayerInfo[playerid][pVipTime] += vipMin;

            new rangNom[16];
            VipSync_RankName(donateRank, rangNom);
            new msg[144];
            format(msg, sizeof(msg), "{00FF88}[VIP]{FFFFFF} Ton rang %s est actif ! Merci pour ton soutien a AFRP.", rangNom);
            SendClientMessage(playerid, -1, msg);
        }
        if (cash > 0)
        {
            PlayerInfo[playerid][pCash] += cash;
            new msg2[128];
            format(msg2, sizeof(msg2), "{00FF88}[BOUTIQUE]{FFFFFF} Tu as recu %d$ ! (visible apres reconnexion si le HUD ne bouge pas)", cash);
            SendClientMessage(playerid, -1, msg2);
        }
    }
    else
    {
        // ── HORS LIGNE : on ecrit dans la base (chargee a la reconnexion) ──
        new usernameSafe[64];
        mysql_escape_string(username, usernameSafe);
        new vsSql[256];
        if (donateRank > 0)
        {
            // GREATEST = jamais de downgrade ; VipTime additif plafonne
            format(vsSql, sizeof(vsSql), "UPDATE lvrp_users SET DonateRank=GREATEST(DonateRank,%d), VipTime=LEAST(VipTime+%d,%d) WHERE Name='%s'",
                donateRank, vipMin, VIP_TIME_MAX, usernameSafe);
            mysql_tquery(MYSQL, vsSql, "", "");
        }
        if (cash > 0)
        {
            format(vsSql, sizeof(vsSql), "UPDATE lvrp_users SET Cash=Cash+%d WHERE Name='%s'", cash, usernameSafe);
            mysql_tquery(MYSQL, vsSql, "", "");
        }
    }

    // Marque comme traite (empeche toute reapplication, meme apres reboot)
    new markSql[160];
    format(markSql, sizeof(markSql), "INSERT INTO vip_sync_processed (push_id, processed_at) VALUES ('%s', %d)", safePushId, gettime());
    mysql_tquery(MYSQL, markSql, "", "");
}

stock VipSync_RankName(rank, dest[], size = sizeof dest)
{
    switch (rank)
    {
        case 1: strmid(dest, "Fer", 0, 3, size);
        case 2: strmid(dest, "Argent", 0, 6, size);
        case 3: strmid(dest, "Or", 0, 2, size);
        case 4: strmid(dest, "Diamant", 0, 7, size);
        default: strmid(dest, "VIP", 0, 3, size);
    }
}

stock VipSync_FindPlayerByName(const name[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        new pname[MAX_PLAYER_NAME];
        GetPlayerName(i, pname, sizeof(pname));
        if (strcmp(pname, name, true) == 0) return i;
    }
    return INVALID_PLAYER_ID;
}
