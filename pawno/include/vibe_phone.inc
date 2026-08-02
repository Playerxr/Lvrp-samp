// ============================================================================
// === SMARTPHONE - HABILLAGE "HANDPHONE" (pack textdraw fourni par la ======
// === proprietaire du serveur, remplace l'ancien design maison) ============
// ============================================================================
// Le pack d'origine (Textdraw_Handphone.txt, 84 elements, source Indonesienne)
// ne dessine qu'un seul ecran (chassis + barre d'etat + grille d'icones) : il
// n'a pas de lockscreen. L'ecran de verrouillage (grosse horloge, date, indice
// "cliquez pour deverrouiller") reste donc celui qui existait deja, seulement
// repositionne pour tenir dans l'ecran du nouveau chassis.
//
// 15 elements du pack ont ete retires (budget + hors sujet pour ce serveur) :
//   - "Airdrop" (35, 42, 56, 71, 72, 73) : aucune fonctionnalite equivalente
//   - "Hanako Bento" (70, 74-81) : commande de restaurant d'un autre serveur
// Les 69 elements restants sont recrees a l'identique (position, couleur,
// police) sous des indices renumerotes en continu (0..68), plus 3 elements
// pour le lockscreen (69..71) : PHONE_TD_COUNT = 72.
//
// Correspondance icone du pack -> fonction du serveur (decidee par la
// proprietaire) :
//   Close HP   -> ferme le telephone (comme l'ancien bouton X)
//   Bank       -> Banque (inchangee)
//   Kontak     -> Contacts (inchangee)
//   GPS        -> Plan (inchangee)
//   Info       -> Infos (inchangee)
//   Whatsapp   -> Telephone/appels (l'equivalent le plus proche d'un
//                 Whatsapp sur un serveur SA-MP)
//   Twitter/X  -> Messages/SMS (icone reaffectee)
//   Layanan (Services) -> Urgences (un service d'urgence est un "service")
//   Lainnya (Autres)   -> Reglages (une icone "divers" convient aux reglages)
//   Vehicle    -> raccourci direct vers /vehicule (pas une app du telephone,
//                 rappelle juste la commande comme afrp_raccourcis.inc)
//
// Etats (phone_State[playerid]) :
//   0 = ferme
//   1 = lockscreen
//   2 = home
//   3..10 = apps (contenu dans afrp_telapps.inc)
// ============================================================================

#if defined _vibe_phone_included
    #endinput
#endif
#define _vibe_phone_included

#define PHONE_TD_COUNT       72

#define PHONE_STATE_CLOSED   0
#define PHONE_STATE_LOCK     1
#define PHONE_STATE_HOME     2
#define PHONE_STATE_APP_TEL  3
#define PHONE_STATE_APP_SMS  4
#define PHONE_STATE_APP_CTC  5
#define PHONE_STATE_APP_BNK  6
#define PHONE_STATE_APP_GAL  7
#define PHONE_STATE_APP_SET  8
#define PHONE_STATE_APP_MAP  9
#define PHONE_STATE_APP_CAL  10

// ----- Index nommes dans phone_TD[] (evite les nombres magiques) -----
#define PHONE_TD_WALLPAPER          17   // fond d'ecran (afrp_telapps.inc : /tel Reglages)
#define PHONE_TD_STATUSBAR_CLOCK    19   // horloge barre d'etat ("08:00")
#define PHONE_TD_STATUSBAR_BATTERY  24   // pourcentage batterie ("60")
#define PHONE_TD_CLOSE              29   // bouton "Close HP"
#define PHONE_TD_ICON_INFO          30
#define PHONE_TD_ICON_LAYANAN       31
#define PHONE_TD_ICON_BANK          32
#define PHONE_TD_ICON_TWITTER       33
#define PHONE_TD_ICON_VEHICLE       34
#define PHONE_TD_ICON_LAINNYA       35
#define PHONE_TD_ICON_WHATSAPP      41
#define PHONE_TD_ICON_GPS           42
#define PHONE_TD_ICON_KONTAK        43
#define PHONE_TD_LOCK_CLOCK         69
#define PHONE_TD_LOCK_DATE          70
#define PHONE_TD_LOCK_HINT          71

new PlayerText:phone_TD[MAX_PLAYERS][PHONE_TD_COUNT];
new phone_State[MAX_PLAYERS];
new phone_Wallpaper[MAX_PLAYERS];

// Compat ancien code
#define phone_Visible phone_State

// ----- Helpers de creation TD -----
// Les elements du pack sont soit des sprites (texture LD_SPAC:white /
// LD_BEAT:chit affichee en boite via Font 4 + TextSize), soit du texte
// (Font 0/1 + LetterSize). On reprend ces deux familles telles quelles.
stock PlayerText:phoneTD_Sprite(playerid, Float:x, Float:y, Float:w, Float:h, color, sprite[], bool:selectable = false)
{
    new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, sprite);
    // [FIDELITE PACK] Font 4 + nom de sprite ("LD_...") = texture dessinee en
    // taille (w,h) pixels, PAS des coordonnees de fin comme un UseBox. Le pack
    // source ne pose jamais PlayerTextDrawUseBox : on ne l'ajoute pas non plus.
    PlayerTextDrawTextSize(playerid, t, w, h);
    PlayerTextDrawAlignment(playerid, t, 1);
    PlayerTextDrawColor(playerid, t, color);
    PlayerTextDrawSetShadow(playerid, t, 0);
    PlayerTextDrawSetOutline(playerid, t, 0);
    PlayerTextDrawBackgroundColor(playerid, t, 255);
    PlayerTextDrawFont(playerid, t, 4);
    PlayerTextDrawSetProportional(playerid, t, 1);
    if(selectable) PlayerTextDrawSetSelectable(playerid, t, 1);
    return t;
}

stock PlayerText:phoneTD_Label(playerid, Float:x, Float:y, Float:lx, Float:ly, color, font, txt[])
{
    new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, txt);
    PlayerTextDrawLetterSize(playerid, t, lx, ly);
    PlayerTextDrawAlignment(playerid, t, 1);
    PlayerTextDrawColor(playerid, t, color);
    PlayerTextDrawSetShadow(playerid, t, 0);
    PlayerTextDrawSetOutline(playerid, t, 0);
    PlayerTextDrawBackgroundColor(playerid, t, 150);
    PlayerTextDrawFont(playerid, t, font);
    PlayerTextDrawSetProportional(playerid, t, 1);
    return t;
}

stock phone_Create(playerid)
{
    if(phone_TD[playerid][0] != PlayerText:INVALID_TEXT_DRAW) return 1;

    // ===================================================================
    // CHASSIS (coins arrondis, ecran, encoche, barre d'etat) - toujours
    // affiche, lockscreen comme home. Indices 0..28, copies du pack.
    // ===================================================================
    phone_TD[playerid][0]  = phoneTD_Sprite(playerid, 500.000, 142.000, 24.000, 27.000, 943410687, "LD_BEAT:chit");
    phone_TD[playerid][1]  = phoneTD_Sprite(playerid, 599.000, 142.000, 24.000, 27.000, 943410687, "LD_BEAT:chit");
    phone_TD[playerid][2]  = phoneTD_Sprite(playerid, 500.000, 397.000, 24.000, 27.000, 943410687, "LD_BEAT:chit");
    phone_TD[playerid][3]  = phoneTD_Sprite(playerid, 599.000, 397.000, 24.000, 27.000, 943410687, "LD_BEAT:chit");
    phone_TD[playerid][4]  = phoneTD_Sprite(playerid, 512.000, 146.000, 100.000, 273.000, 943410687, "LD_SPAC:white");
    phone_TD[playerid][5]  = phoneTD_Sprite(playerid, 504.000, 155.000, 115.000, 258.000, 943410687, "LD_SPAC:white");
    phone_TD[playerid][6]  = phoneTD_Sprite(playerid, 502.000, 144.000, 24.000, 27.000, 255, "LD_BEAT:chit");
    phone_TD[playerid][7]  = phoneTD_Sprite(playerid, 597.000, 144.000, 24.000, 27.000, 255, "LD_BEAT:chit");
    phone_TD[playerid][8]  = phoneTD_Sprite(playerid, 502.000, 395.000, 24.000, 27.000, 255, "LD_BEAT:chit");
    phone_TD[playerid][9]  = phoneTD_Sprite(playerid, 597.000, 395.000, 24.000, 27.000, 255, "LD_BEAT:chit");
    phone_TD[playerid][10] = phoneTD_Sprite(playerid, 514.000, 148.500, 96.000, 269.000, 255, "LD_SPAC:white");
    phone_TD[playerid][11] = phoneTD_Sprite(playerid, 506.000, 157.000, 111.000, 250.000, 1684170495, "LD_SPAC:white");
    phone_TD[playerid][12] = phoneTD_Sprite(playerid, 504.000, 146.000, 24.000, 27.000, 455562751, "LD_BEAT:chit");
    phone_TD[playerid][13] = phoneTD_Sprite(playerid, 595.000, 146.000, 24.000, 27.000, 455562751, "LD_BEAT:chit");
    phone_TD[playerid][14] = phoneTD_Sprite(playerid, 504.000, 393.000, 24.000, 27.000, 455562751, "LD_BEAT:chit");
    phone_TD[playerid][15] = phoneTD_Sprite(playerid, 595.000, 393.000, 24.000, 27.000, 455562751, "LD_BEAT:chit");
    phone_TD[playerid][16] = phoneTD_Sprite(playerid, 516.000, 150.500, 92.000, 265.000, 455562751, "LD_SPAC:white");
    // 17 : fond d'ecran visible (afrp_telapps.inc change sa couleur via /tel Reglages)
    phone_TD[playerid][PHONE_TD_WALLPAPER] = phoneTD_Sprite(playerid, 508.000, 157.000, 107.000, 249.000, 455562751, "LD_SPAC:white");
    phone_TD[playerid][18] = phoneTD_Sprite(playerid, 554.000, 153.000, 12.000, 17.000, 255, "LD_BEAT:chit"); // encoche/haut-parleur

    // Barre d'etat : horloge + batterie + reseau + barres de signal.
    phone_TD[playerid][PHONE_TD_STATUSBAR_CLOCK] = phoneTD_Label(playerid, 517.000, 156.000, 0.179, 0.898, -1, 1, "08:00");
    phone_TD[playerid][20] = phoneTD_Sprite(playerid, 598.000, 157.000, 7.000, 6.000, 1768516095, "LD_SPAC:white");
    phone_TD[playerid][21] = phoneTD_Sprite(playerid, 598.000, 157.000, 4.000, 6.000, -1, "LD_SPAC:white");
    phone_TD[playerid][22] = phoneTD_Sprite(playerid, 597.000, 159.000, 4.000, 3.000, -1, "LD_SPAC:white");
    phone_TD[playerid][23] = phoneTD_Sprite(playerid, 606.000, 159.000, 1.000, 3.000, 1768516095, "LD_SPAC:white");
    phone_TD[playerid][PHONE_TD_STATUSBAR_BATTERY] = phoneTD_Label(playerid, 599.000, 157.000, 0.119, 0.698, 255, 1, "60");
    phone_TD[playerid][25] = phoneTD_Label(playerid, 590.000, 157.000, 0.119, 0.698, -1, 1, "4G");
    phone_TD[playerid][26] = phoneTD_Sprite(playerid, 588.000, 158.000, 1.000, 5.000, -1, "LD_SPAC:white");
    phone_TD[playerid][27] = phoneTD_Sprite(playerid, 586.500, 160.000, 1.000, 3.000, -1, "LD_SPAC:white");
    phone_TD[playerid][28] = phoneTD_Sprite(playerid, 584.500, 162.000, 1.000, 1.000, -1, "LD_SPAC:white");

    // Bouton "Close HP" (equivalent du bouton home/X, ferme le telephone).
    phone_TD[playerid][PHONE_TD_CLOSE] = phoneTD_Sprite(playerid, 539.500, 403.000, 46.000, 6.000, -1, "LD_SPAC:white", true);

    // ===================================================================
    // LOCKSCREEN (pas dans le pack : reprend le contenu de l'ancien design,
    // recentre pour tenir dans l'ecran du nouveau chassis, x ~ 508..615).
    // ===================================================================
    phone_TD[playerid][PHONE_TD_LOCK_CLOCK] = CreatePlayerTextDraw(playerid, 561.500, 220.000, "00:00");
    PlayerTextDrawFont(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 1);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 0.45, 2.2);
    PlayerTextDrawColor(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 0xFFFFFFFF);
    PlayerTextDrawBackgroundColor(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 0x00000000);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 2);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], 2);

    phone_TD[playerid][PHONE_TD_LOCK_DATE] = CreatePlayerTextDraw(playerid, 561.500, 265.000, "Samedi 1 Janvier");
    PlayerTextDrawFont(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], 2);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], 0.20, 0.9);
    PlayerTextDrawColor(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], 0xE0E0E0FF);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], 1);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], 2);

    phone_TD[playerid][PHONE_TD_LOCK_HINT] = CreatePlayerTextDraw(playerid, 561.500, 340.000, "Cliquez pour deverrouiller");
    PlayerTextDrawFont(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 2);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 0.15, 0.75);
    PlayerTextDrawColor(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 1);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 2);
    PlayerTextDrawSetSelectable(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 1);
    PlayerTextDrawTextSize(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT], 100.0, 10.0);

    // ===================================================================
    // HOME : grille 2x4 (Info, Layanan, Bank, Twitter/X, Vehicle, Lainnya
    // -- Airdrop supprime) + dock 3 icones (Whatsapp, GPS, Kontak) + leurs
    // libelles et pictogrammes. Indices 30..68, copies du pack.
    // ===================================================================
    phone_TD[playerid][PHONE_TD_ICON_INFO]    = phoneTD_Sprite(playerid, 513.500, 180.000, 21.000, 26.000, 425831935, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_LAYANAN] = phoneTD_Sprite(playerid, 538.500, 180.000, 21.000, 26.000, -181516545, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_BANK]    = phoneTD_Sprite(playerid, 563.500, 180.000, 21.000, 26.000, -1724723969, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_TWITTER] = phoneTD_Sprite(playerid, 587.500, 180.000, 21.000, 26.000, 255, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_VEHICLE] = phoneTD_Sprite(playerid, 513.500, 230.000, 21.000, 26.000, 421097727, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_LAINNYA] = phoneTD_Sprite(playerid, 563.500, 230.000, 21.000, 26.000, -626712321, "LD_SPAC:white", true);

    // [FIX LANGUE] Le pack source est indonesien : ces libelles etaient
    // restes "Informasi/Layanan/M-Banking/X/Vehicles" sous des icones dont la
    // fonction reelle est deja en francais (voir la correspondance icone ->
    // fonction en tete de fichier). Traduits pour dire ce qui s'ouvre vraiment.
    phone_TD[playerid][36] = phoneTD_Label(playerid, 515.500, 208.000, 0.108, 0.698, -1, 1, "Infos");
    phone_TD[playerid][37] = phoneTD_Label(playerid, 542.000, 208.000, 0.108, 0.698, -1, 1, "Urgences");
    phone_TD[playerid][38] = phoneTD_Label(playerid, 565.000, 208.000, 0.108, 0.698, -1, 1, "Banque");
    phone_TD[playerid][39] = phoneTD_Label(playerid, 597.000, 208.000, 0.108, 0.698, -1, 1, "SMS");
    phone_TD[playerid][40] = phoneTD_Label(playerid, 517.000, 258.000, 0.108, 0.698, -1, 1, "Vehicule");

    phone_TD[playerid][PHONE_TD_ICON_WHATSAPP] = phoneTD_Sprite(playerid, 522.500, 361.000, 21.000, 26.000, -2048950785, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_GPS]      = phoneTD_Sprite(playerid, 550.500, 361.000, 21.000, 26.000, -1, "LD_SPAC:white", true);
    phone_TD[playerid][PHONE_TD_ICON_KONTAK]   = phoneTD_Sprite(playerid, 578.500, 361.000, 21.000, 26.000, -12254977, "LD_SPAC:white", true);

    // Pictogrammes (dessines par-dessus les icones ci-dessus, non cliquables).
    phone_TD[playerid][44] = phoneTD_Label(playerid, 521.000, 185.000, 0.407, 1.598, -1, 0, "i");                              // Info
    phone_TD[playerid][45] = phoneTD_Sprite(playerid, 547.500, 188.000, 3.000, 12.000, -1, "LD_SPAC:white");                   // Layanan
    phone_TD[playerid][46] = phoneTD_Sprite(playerid, 566.500, 188.000, 14.000, 11.000, -1, "LD_SPAC:white");                  // Bank
    phone_TD[playerid][47] = phoneTD_Sprite(playerid, 567.500, 190.000, 11.000, 2.000, -1724723969, "LD_SPAC:white");          // Bank
    phone_TD[playerid][48] = phoneTD_Sprite(playerid, 575.000, 193.000, 3.000, 4.000, -1724723969, "LD_BEAT:chit");            // Bank
    phone_TD[playerid][49] = phoneTD_Label(playerid, 596.000, 187.000, 0.247, 1.297, -1, 1, "X");                              // Twitter/X
    phone_TD[playerid][50] = phoneTD_Sprite(playerid, 517.000, 243.000, 14.000, 3.000, -1, "LD_SPAC:white");                   // Vehicle
    phone_TD[playerid][51] = phoneTD_Sprite(playerid, 520.000, 239.000, 8.000, 3.000, -1, "LD_SPAC:white");                    // Vehicle
    phone_TD[playerid][52] = phoneTD_Sprite(playerid, 518.000, 247.000, 3.000, 4.000, -1, "LD_BEAT:chit");                     // Vehicle (roue)
    phone_TD[playerid][53] = phoneTD_Sprite(playerid, 527.000, 247.000, 3.000, 4.000, -1, "LD_BEAT:chit");                     // Vehicle (roue)
    phone_TD[playerid][54] = phoneTD_Sprite(playerid, 524.000, 365.000, 19.000, 16.000, -1, "LD_BEAT:chit");                   // Whatsapp
    phone_TD[playerid][55] = phoneTD_Label(playerid, 529.000, 370.000, 0.310, 1.190, -1, 1, ")");                              // Whatsapp
    phone_TD[playerid][56] = phoneTD_Sprite(playerid, 549.500, 361.000, 23.000, 27.000, 512819199, "LD_BEAT:chit");            // GPS
    phone_TD[playerid][57] = phoneTD_Sprite(playerid, 550.500, 362.000, 21.000, 25.000, -1, "LD_BEAT:chit");                   // GPS
    phone_TD[playerid][58] = phoneTD_Sprite(playerid, 551.500, 363.000, 19.000, 23.000, 512819199, "LD_BEAT:chit");            // GPS
    phone_TD[playerid][59] = phoneTD_Label(playerid, 558.000, 370.000, 0.300, 0.990, -1, 1, "/");                              // GPS
    phone_TD[playerid][60] = phoneTD_Label(playerid, 559.000, 367.000, 0.300, 0.990, -16776961, 1, "/");                       // GPS
    phone_TD[playerid][61] = phoneTD_Sprite(playerid, 583.500, 362.000, 10.000, 14.000, -1, "LD_BEAT:chit");                   // Kontak
    phone_TD[playerid][62] = phoneTD_Sprite(playerid, 584.500, 363.000, 8.000, 12.000, -12254977, "LD_BEAT:chit");             // Kontak
    phone_TD[playerid][63] = phoneTD_Sprite(playerid, 580.500, 372.000, 16.000, 14.000, -1, "LD_BEAT:chit");                   // Kontak
    phone_TD[playerid][64] = phoneTD_Sprite(playerid, 581.500, 373.000, 14.000, 12.000, -12254977, "LD_BEAT:chit");            // Kontak
    phone_TD[playerid][65] = phoneTD_Label(playerid, 568.500, 232.000, 0.337, 1.598, -1, 1, "...");                            // Lainnya
    phone_TD[playerid][66] = phoneTD_Label(playerid, 567.000, 258.000, 0.108, 0.698, -1, 1, "Reglages");                       // [FIX LANGUE] etait "Lainnya"
    phone_TD[playerid][67] = phoneTD_Sprite(playerid, 546.500, 191.000, 4.500, 4.000, -1, "LD_SPAC:white");                    // Layanan
    phone_TD[playerid][68] = phoneTD_Sprite(playerid, 545.500, 192.000, 6.500, 5.000, -1, "LD_SPAC:white");                    // Layanan
    return 1;
}

static stock phone_HideAll(playerid)
{
    for(new k = 0; k < PHONE_TD_COUNT; k++)
    {
        if(phone_TD[playerid][k] != PlayerText:INVALID_TEXT_DRAW)
            PlayerTextDrawHide(playerid, phone_TD[playerid][k]);
    }
}

// Chassis + barre d'etat + bouton Close : commun au lockscreen et au home.
static stock phone_ShowFrame(playerid)
{
    for(new k = 0; k <= PHONE_TD_CLOSE; k++) PlayerTextDrawShow(playerid, phone_TD[playerid][k]);
}

stock phone_RefreshStatusBar(playerid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
    new h, m, s; gettime(h, m, s);
    new buf[16];
    format(buf, sizeof(buf), "%02d:%02d", h, m);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][PHONE_TD_STATUSBAR_CLOCK], buf);

    new batt = (playerCredit[playerid] >= 100) ? 100 : (playerCredit[playerid] > 0 ? playerCredit[playerid] : 1);
    format(buf, sizeof(buf), "%d", batt);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][PHONE_TD_STATUSBAR_BATTERY], buf);
    return 1;
}

stock phone_RefreshLockscreen(playerid)
{
    new h, m, s; gettime(h, m, s);
    new d, mo, y; getdate(y, mo, d);
    new buf[24];
    format(buf, sizeof(buf), "%02d:%02d", h, m);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK], buf);

    static const jours[7][12] = { !"Dimanche", !"Lundi", !"Mardi", !"Mercredi", !"Jeudi", !"Vendredi", !"Samedi" };
    static const mois[13][12] = { !"", !"Janvier", !"Fevrier", !"Mars", !"Avril", !"Mai", !"Juin", !"Juillet", !"Aout", !"Septembre", !"Octobre", !"Novembre", !"Decembre" };
    new q = d;
    new mm = mo;
    new yy = y;
    if(mm < 3) { mm += 12; yy -= 1; }
    new K = yy % 100;
    new J = yy / 100;
    new dow = (q + (13*(mm+1))/5 + K + K/4 + J/4 + 5*J) % 7;
    new idx = (dow + 6) % 7;
    format(buf, sizeof(buf), "%s %d %s", jours[idx], d, mois[mo]);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE], buf);
    return 1;
}

stock phone_ShowLockscreen(playerid)
{
    phone_Create(playerid);
    phone_HideAll(playerid);
    phone_ShowFrame(playerid);
    phone_RefreshStatusBar(playerid);
    phone_RefreshLockscreen(playerid);
    PlayerTextDrawShow(playerid, phone_TD[playerid][PHONE_TD_LOCK_CLOCK]);
    PlayerTextDrawShow(playerid, phone_TD[playerid][PHONE_TD_LOCK_DATE]);
    PlayerTextDrawShow(playerid, phone_TD[playerid][PHONE_TD_LOCK_HINT]);
    phone_State[playerid] = PHONE_STATE_LOCK;
    // Active la souris pour pouvoir cliquer meme sur lockscreen
    SelectTextDraw(playerid, 0xCCCCCCFF);
    return 1;
}

stock phone_ShowHome(playerid)
{
    phone_Create(playerid);
    phone_HideAll(playerid);
    phone_ShowFrame(playerid);
    phone_RefreshStatusBar(playerid);
    for(new k = PHONE_TD_ICON_INFO; k < PHONE_TD_COUNT; k++)
    {
        if(k == PHONE_TD_LOCK_CLOCK || k == PHONE_TD_LOCK_DATE || k == PHONE_TD_LOCK_HINT) continue;
        PlayerTextDrawShow(playerid, phone_TD[playerid][k]);
    }
    SelectTextDraw(playerid, 0xCCCCCCFF);
    phone_State[playerid] = PHONE_STATE_HOME;
    return 1;
}

stock phone_Open(playerid)
{
    if(!IsPlayerConnected(playerid)) return 0;
    // [BUDGET TEXTDRAWS] La grille du sac et le smartphone ne tiennent pas
    // ensemble dans les 256 textdraws d'un client : on ferme la grille.
    SacV_Fermer(playerid);
    // [TABLECRAFT] meme raison de budget avec le panneau de craft.
    TC_Fermer(playerid);
    if(hasPhone[playerid] != 1)
    {
        SendClientMessage(playerid, 0xFF4444FF, "{FF4444}[Telephone]{FFFFFF} Vous n'avez pas de telephone. Achetez-en un au 24/7 ou via /market.");
        return 0;
    }
    phone_ShowLockscreen(playerid);
    return 1;
}

stock phone_Close(playerid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
    phone_HideAll(playerid);
    CancelSelectTextDraw(playerid);

    // [BUDGET TEXTDRAWS] Les elements sont detruits, pas seulement masques.
    // Un client SA-MP n'en accepte que 256 par joueur et le gamemode en tient
    // deja pres de 200 : les garder en reserve telephone ferme empechait le sac
    // ou le tableau de bord de s'afficher, sans aucun message d'erreur.
    // phone_ShowLockscreen les recreera a la prochaine ouverture.
    phone_Destroy(playerid);
    phone_State[playerid] = PHONE_STATE_CLOSED;
    return 1;
}

stock phone_Unlock(playerid)
{
    if(phone_State[playerid] != PHONE_STATE_LOCK) return 0;
    phone_ShowHome(playerid);
    return 1;
}

stock phone_GoHome(playerid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED || phone_State[playerid] == PHONE_STATE_LOCK) return 0;
    phone_ShowHome(playerid);
    return 1;
}

stock phone_OpenApp(playerid, app)
{
    // [TELAPPS] Le contenu des applications est dans afrp_telapps.inc, qui
    // rebranche chaque icone sur ce que le gamemode sait deja faire.
    phone_State[playerid] = app;
    return TelApp_Ouvrir(playerid, app);
}

stock phone_OnClick(playerid, PlayerText:clickedid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
    if(clickedid == phone_TD[playerid][PHONE_TD_CLOSE]) { phone_Close(playerid); return 1; }

    if(phone_State[playerid] == PHONE_STATE_LOCK)
    {
        if(clickedid == phone_TD[playerid][PHONE_TD_LOCK_HINT]) { phone_Unlock(playerid); return 1; }
        return 0;
    }

    if(phone_State[playerid] == PHONE_STATE_HOME)
    {
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_INFO])     { phone_OpenApp(playerid, PHONE_STATE_APP_CAL); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_LAYANAN])  { phone_OpenApp(playerid, PHONE_STATE_APP_GAL); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_BANK])     { phone_OpenApp(playerid, PHONE_STATE_APP_BNK); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_TWITTER])  { phone_OpenApp(playerid, PHONE_STATE_APP_SMS); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_LAINNYA])  { phone_OpenApp(playerid, PHONE_STATE_APP_SET); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_WHATSAPP]) { phone_OpenApp(playerid, PHONE_STATE_APP_TEL); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_GPS])      { phone_OpenApp(playerid, PHONE_STATE_APP_MAP); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_KONTAK])   { phone_OpenApp(playerid, PHONE_STATE_APP_CTC); return 1; }
        if(clickedid == phone_TD[playerid][PHONE_TD_ICON_VEHICLE])
        {
            // Pas une app du telephone : rappelle /vehicule, comme
            // afrp_raccourcis.inc le fait pour son propre raccourci.
            phone_Close(playerid);
            CallLocalFunction("OnPlayerCommandText", "is", playerid, "/vehicule");
            return 1;
        }
    }
    return 0;
}

stock phone_OnKey(playerid, newkeys)
{
    if(phone_State[playerid] == PHONE_STATE_LOCK && (newkeys & KEY_SECONDARY_ATTACK))
    {
        phone_Unlock(playerid);
        return 1;
    }
    return 0;
}

stock phone_Init(playerid)
{
    for(new k = 0; k < PHONE_TD_COUNT; k++)
        phone_TD[playerid][k] = PlayerText:INVALID_TEXT_DRAW;
    phone_State[playerid] = PHONE_STATE_CLOSED;
    phone_Wallpaper[playerid] = 0;
    return 1;
}

stock phone_Destroy(playerid)
{
    for(new k = 0; k < PHONE_TD_COUNT; k++)
    {
        if(phone_TD[playerid][k] != PlayerText:INVALID_TEXT_DRAW)
        {
            PlayerTextDrawDestroy(playerid, phone_TD[playerid][k]);
            phone_TD[playerid][k] = PlayerText:INVALID_TEXT_DRAW;
        }
    }
    phone_State[playerid] = PHONE_STATE_CLOSED;
    return 1;
}

// Wrappers compat
stock phoneUI_Init(playerid)    { return phone_Init(playerid); }
stock phoneUI_Destroy(playerid) { return phone_Destroy(playerid); }
stock phoneUI_Show(playerid)    { return phone_Open(playerid); }
stock phoneUI_Hide(playerid)    { return phone_Close(playerid); }
