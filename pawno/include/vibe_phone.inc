// ============================================================================
// === SMARTPHONE VIBE-STYLE (P1 : foundation - lockscreen + home + statusbar) =
// ============================================================================
// Style realiste smartphone moderne : cadre arrondi, encoche en haut, fond
// d'ecran, status bar (heure/reseau/batterie), lockscreen avec grosse horloge
// + date + hint, home screen avec 8 apps en grille + dock 4 apps.
// Ouverture via /phone /tel /telephone /mobile. Touche ESC ou icone X pour
// fermer. ENTER pour deverrouiller depuis le lockscreen.
//
// Dependances cote LVRP.pwn (declarees AVANT ce include) :
//   - new hasPhone[MAX_PLAYERS];
//   - new playerCredit[MAX_PLAYERS];
//   - PlayerInfo[playerid][pNumber]
//
// Etats (phone_State[playerid]) :
//   0 = ferme
//   1 = lockscreen
//   2 = home
//   3..10 = apps (impl en P2-P4)
// ============================================================================

#if defined _vibe_phone_included
    #endinput
#endif
#define _vibe_phone_included

#define PHONE_TD_COUNT       50
#define PHONE_APP_COUNT      8
#define PHONE_DOCK_COUNT     4

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

new PlayerText:phone_TD[MAX_PLAYERS][PHONE_TD_COUNT];
new phone_State[MAX_PLAYERS];
new phone_Wallpaper[MAX_PLAYERS];

// Compat ancien code
#define phone_Visible phone_State

static stock const phone_AppIcons[PHONE_APP_COUNT][8] = {
    !"Tel",   !"SMS",   !"Ctc",   !"Bnk",
    !"SOS",   !"Set",   !"Map",   !"Inf"
};
static stock const phone_AppLabels[PHONE_APP_COUNT][16] = {
    !"Telephone", !"Messages",  !"Contacts", !"Banque",
    !"Urgences",  !"Reglages",  !"Plan",     !"Infos"
};
static stock const phone_AppColors[PHONE_APP_COUNT] = {
    0x34C759FF, 0x32D74BFF, 0xFFAA00FF, 0x007AFFFF,
    0xFF375FFF, 0x8E8E93FF, 0x30D158FF, 0xFF453AFF
};
static stock const phone_AppStates[PHONE_APP_COUNT] = {
    PHONE_STATE_APP_TEL, PHONE_STATE_APP_SMS, PHONE_STATE_APP_CTC, PHONE_STATE_APP_BNK,
    PHONE_STATE_APP_GAL, PHONE_STATE_APP_SET, PHONE_STATE_APP_MAP, PHONE_STATE_APP_CAL
};
static stock const phone_DockIcons[PHONE_DOCK_COUNT][8] = {
    !"Tel", !"SMS", !"Ctc", !"Set"
};
static stock const phone_DockColors[PHONE_DOCK_COUNT] = {
    0x34C759FF, 0x32D74BFF, 0xFFAA00FF, 0x8E8E93FF
};
static stock const phone_DockStates[PHONE_DOCK_COUNT] = {
    PHONE_STATE_APP_TEL, PHONE_STATE_APP_SMS, PHONE_STATE_APP_CTC, PHONE_STATE_APP_SET
};

static stock const phone_WallpaperColors[6] = {
    0x1C1C1EFF, 0x0A2540FF, 0x2D0A4FFF,
    0x3A2317FF, 0x0F3D2EFF, 0x4A0E1AFF
};

// ----- Helpers de creation TD -----
// IMPORTANT : font=1 + LetterSize tiny + Color alpha=00 = box visible, texte invisible.
// Font 4 (preview model) ne rend PAS les boxes correctement.
stock PlayerText:phoneTD_Box(playerid, Float:x, Float:y, Float:w, Float:h, color)
{
    new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, "_");
    PlayerTextDrawFont(playerid, t, 1);
    PlayerTextDrawLetterSize(playerid, t, 0.0, 0.0);
    PlayerTextDrawColor(playerid, t, 0x00000000);
    PlayerTextDrawBackgroundColor(playerid, t, 0x00000000);
    PlayerTextDrawTextSize(playerid, t, x + w, y + h);
    PlayerTextDrawUseBox(playerid, t, 1);
    PlayerTextDrawBoxColor(playerid, t, color);
    return t;
}

stock PlayerText:phoneTD_Text(playerid, Float:x, Float:y, Float:lx, Float:ly, color, font, txt[])
{
    new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, txt);
    PlayerTextDrawFont(playerid, t, font);
    PlayerTextDrawLetterSize(playerid, t, lx, ly);
    PlayerTextDrawColor(playerid, t, color);
    PlayerTextDrawBackgroundColor(playerid, t, 0x00000000);
    PlayerTextDrawSetOutline(playerid, t, 0);
    PlayerTextDrawSetShadow(playerid, t, 1);
    return t;
}

// Alignment 1 (left) pour TextSize prenne (x_end, y_end) absolus -> clic propre
stock PlayerText:phoneTD_Tile(playerid, Float:x, Float:y, Float:size, color, ic[])
{
    new PlayerText:t = CreatePlayerTextDraw(playerid, x, y, ic);
    PlayerTextDrawFont(playerid, t, 2);
    PlayerTextDrawLetterSize(playerid, t, 0.22, 1.0);
    PlayerTextDrawColor(playerid, t, 0xFFFFFFFF);
    PlayerTextDrawBackgroundColor(playerid, t, 0x00000000);
    PlayerTextDrawSetOutline(playerid, t, 0);
    PlayerTextDrawSetShadow(playerid, t, 1);
    PlayerTextDrawUseBox(playerid, t, 1);
    PlayerTextDrawBoxColor(playerid, t, color);
    PlayerTextDrawAlignment(playerid, t, 1);
    PlayerTextDrawTextSize(playerid, t, x + size, y + size);
    PlayerTextDrawSetSelectable(playerid, t, 1);
    return t;
}

stock phone_Create(playerid)
{
    if(phone_TD[playerid][0] != PlayerText:INVALID_TEXT_DRAW) return 1;

    new const Float:PX = 488.0;
    new const Float:PY = 100.0;
    new const Float:PW = 140.0;
    new const Float:PH = 240.0;

    phone_TD[playerid][0] = phoneTD_Box(playerid, PX - 3.0, PY - 3.0, PW + 6.0, PH + 6.0, 0x000000AA);
    phone_TD[playerid][1] = phoneTD_Box(playerid, PX, PY, PW, PH, 0x1A1A1CFF);
    phone_TD[playerid][2] = phoneTD_Box(playerid, PX + 4.0, PY + 16.0, PW - 8.0, PH - 30.0, 0x000000FF);
    phone_TD[playerid][3] = phoneTD_Box(playerid, PX + 48.0, PY + 16.0, 44.0, 6.0, 0x000000FF);
    phone_TD[playerid][4] = phoneTD_Box(playerid, PX + 45.0, PY + PH - 10.0, 50.0, 2.5, 0xFFFFFF99);
    phone_TD[playerid][5] = phoneTD_Box(playerid, PX + 4.0, PY + 22.0, PW - 8.0, 9.0, 0x00000088);
    phone_TD[playerid][6] = phoneTD_Text(playerid, PX + 8.0, PY + 22.5, 0.16, 0.8, 0xFFFFFFFF, 2, "00:00");
    phone_TD[playerid][7] = phoneTD_Text(playerid, PX + PW - 42.0, PY + 22.5, 0.16, 0.8, 0xFFFFFFFF, 2, "4G 100%");
    phone_TD[playerid][8] = phoneTD_Box(playerid, PX + 4.0, PY + 31.0, PW - 8.0, PH - 45.0, phone_WallpaperColors[0]);

    // LOCKSCREEN
    phone_TD[playerid][9] = CreatePlayerTextDraw(playerid, PX + PW/2.0, PY + 70.0, "00:00");
    PlayerTextDrawFont(playerid, phone_TD[playerid][9], 1);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][9], 0.55, 2.8);
    PlayerTextDrawColor(playerid, phone_TD[playerid][9], 0xFFFFFFFF);
    PlayerTextDrawBackgroundColor(playerid, phone_TD[playerid][9], 0x00000000);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][9], 2);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][9], 2);

    phone_TD[playerid][10] = CreatePlayerTextDraw(playerid, PX + PW/2.0, PY + 110.0, "Samedi 1 Janvier");
    PlayerTextDrawFont(playerid, phone_TD[playerid][10], 2);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][10], 0.22, 1.0);
    PlayerTextDrawColor(playerid, phone_TD[playerid][10], 0xE0E0E0FF);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][10], 1);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][10], 2);

    phone_TD[playerid][11] = CreatePlayerTextDraw(playerid, PX + PW/2.0, PY + PH - 35.0, "Cliquez pour deverrouiller");
    PlayerTextDrawFont(playerid, phone_TD[playerid][11], 2);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][11], 0.16, 0.8);
    PlayerTextDrawColor(playerid, phone_TD[playerid][11], 0xFFFFFFFF);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][11], 1);
    PlayerTextDrawAlignment(playerid, phone_TD[playerid][11], 2);
    PlayerTextDrawSetSelectable(playerid, phone_TD[playerid][11], 1);
    // Box cliquable centree sur le texte, largeur 100px, hauteur 10px
    PlayerTextDrawTextSize(playerid, phone_TD[playerid][11], 100.0, 10.0);

    phone_TD[playerid][12] = phoneTD_Text(playerid, PX + 12.0, PY + 150.0, 0.16, 0.8, 0xFFFFFFCC, 2, " ");

    // HOME : 8 apps en grille 4x2
    new const Float:TILE_SIZE = 22.0;
    new const Float:TILE_GAP_X = 8.0;
    new const Float:TILE_GAP_Y = 18.0;
    new const Float:GRID_X = PX + 14.0;
    new const Float:GRID_Y = PY + 45.0;
    for(new k = 0; k < PHONE_APP_COUNT; k++)
    {
        new Float:col = float(k % 4);
        new Float:row = float(k / 4);
        new Float:tx = GRID_X + col * (TILE_SIZE + TILE_GAP_X);
        new Float:ty = GRID_Y + row * (TILE_SIZE + TILE_GAP_Y);
        phone_TD[playerid][13 + k] = phoneTD_Tile(playerid, tx, ty, TILE_SIZE, phone_AppColors[k], phone_AppIcons[k]);
        phone_TD[playerid][21 + k] = CreatePlayerTextDraw(playerid, tx + TILE_SIZE/2.0, ty + TILE_SIZE + 1.0, phone_AppLabels[k]);
        PlayerTextDrawFont(playerid, phone_TD[playerid][21 + k], 2);
        PlayerTextDrawLetterSize(playerid, phone_TD[playerid][21 + k], 0.13, 0.7);
        PlayerTextDrawColor(playerid, phone_TD[playerid][21 + k], 0xFFFFFFFF);
        PlayerTextDrawSetShadow(playerid, phone_TD[playerid][21 + k], 1);
        PlayerTextDrawAlignment(playerid, phone_TD[playerid][21 + k], 2);
    }

    // DOCK
    new const Float:DOCK_Y = PY + PH - 38.0;
    new const Float:DOCK_X_START = PX + 18.0;
    new const Float:DOCK_SIZE = 22.0;
    new const Float:DOCK_GAP = 12.0;
    phone_TD[playerid][37] = phoneTD_Box(playerid, PX + 10.0, DOCK_Y - 3.0, PW - 20.0, DOCK_SIZE + 6.0, 0xFFFFFF22);
    for(new k = 0; k < PHONE_DOCK_COUNT; k++)
    {
        new Float:tx = DOCK_X_START + float(k) * (DOCK_SIZE + DOCK_GAP);
        phone_TD[playerid][29 + k] = phoneTD_Tile(playerid, tx, DOCK_Y, DOCK_SIZE, phone_DockColors[k], phone_DockIcons[k]);
    }

    // Bouton X (TextSize en coords ABSOLUES : end_x, end_y)
    new const Float:XBX = PX + PW - 18.0;
    new const Float:XBY = PY + 2.0;
    phone_TD[playerid][38] = CreatePlayerTextDraw(playerid, XBX, XBY, "X");
    PlayerTextDrawFont(playerid, phone_TD[playerid][38], 2);
    PlayerTextDrawLetterSize(playerid, phone_TD[playerid][38], 0.30, 1.4);
    PlayerTextDrawColor(playerid, phone_TD[playerid][38], 0xFF3333FF);
    PlayerTextDrawSetShadow(playerid, phone_TD[playerid][38], 1);
    PlayerTextDrawSetOutline(playerid, phone_TD[playerid][38], 1);
    PlayerTextDrawSetSelectable(playerid, phone_TD[playerid][38], 1);
    PlayerTextDrawTextSize(playerid, phone_TD[playerid][38], XBX + 14.0, XBY + 14.0);
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

static stock phone_ShowFrame(playerid)
{
    for(new k = 0; k <= 8; k++) PlayerTextDrawShow(playerid, phone_TD[playerid][k]);
    PlayerTextDrawShow(playerid, phone_TD[playerid][38]);
}

stock phone_RefreshStatusBar(playerid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
    new h, m, s; gettime(h, m, s);
    new buf[16];
    format(buf, sizeof(buf), "%02d:%02d", h, m);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][6], buf);

    new batt = (playerCredit[playerid] >= 100) ? 100 : (playerCredit[playerid] > 0 ? playerCredit[playerid] : 1);
    format(buf, sizeof(buf), "4G %d%%", batt);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][7], buf);
    return 1;
}

stock phone_RefreshLockscreen(playerid)
{
    new h, m, s; gettime(h, m, s);
    new d, mo, y; getdate(y, mo, d);
    new buf[24];
    format(buf, sizeof(buf), "%02d:%02d", h, m);
    PlayerTextDrawSetString(playerid, phone_TD[playerid][9], buf);

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
    PlayerTextDrawSetString(playerid, phone_TD[playerid][10], buf);
    return 1;
}

stock phone_ShowLockscreen(playerid)
{
    phone_Create(playerid);
    phone_HideAll(playerid);
    phone_ShowFrame(playerid);
    phone_RefreshStatusBar(playerid);
    phone_RefreshLockscreen(playerid);
    PlayerTextDrawShow(playerid, phone_TD[playerid][9]);
    PlayerTextDrawShow(playerid, phone_TD[playerid][10]);
    PlayerTextDrawShow(playerid, phone_TD[playerid][11]);
    phone_State[playerid] = PHONE_STATE_LOCK;
    // Active la souris pour pouvoir cliquer X meme sur lockscreen
    SelectTextDraw(playerid, 0xCCCCCCFF);
    return 1;
}

stock phone_ShowHome(playerid)
{
    phone_Create(playerid);
    phone_HideAll(playerid);
    phone_ShowFrame(playerid);
    phone_RefreshStatusBar(playerid);
    for(new k = 0; k < PHONE_APP_COUNT; k++)
    {
        PlayerTextDrawShow(playerid, phone_TD[playerid][13 + k]);
        PlayerTextDrawShow(playerid, phone_TD[playerid][21 + k]);
    }
    PlayerTextDrawShow(playerid, phone_TD[playerid][37]);
    for(new k = 0; k < PHONE_DOCK_COUNT; k++)
        PlayerTextDrawShow(playerid, phone_TD[playerid][29 + k]);
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

    // [BUDGET TEXTDRAWS] Les 50 elements sont detruits, pas seulement masques.
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
    // [TELAPPS] Le corps d'origine annoncait "App en developpement - P2-P4" et
    // revenait a l'accueil : les huit icones ne faisaient rien. Le contenu est
    // dans afrp_telapps.inc, qui rebranche chaque application sur ce que le
    // gamemode sait deja faire.
    phone_State[playerid] = app;
    return TelApp_Ouvrir(playerid, app);
}

stock phone_OnClick(playerid, PlayerText:clickedid)
{
    if(phone_State[playerid] == PHONE_STATE_CLOSED) return 0;
    if(clickedid == phone_TD[playerid][38]) { phone_Close(playerid); return 1; }
    if(phone_State[playerid] == PHONE_STATE_LOCK)
    {
        if(clickedid == phone_TD[playerid][11]) { phone_Unlock(playerid); return 1; }
    }
    if(phone_State[playerid] == PHONE_STATE_HOME)
    {
        for(new k = 0; k < PHONE_APP_COUNT; k++)
        {
            if(clickedid == phone_TD[playerid][13 + k])
            { phone_OpenApp(playerid, phone_AppStates[k]); return 1; }
        }
        for(new k = 0; k < PHONE_DOCK_COUNT; k++)
        {
            if(clickedid == phone_TD[playerid][29 + k])
            { phone_OpenApp(playerid, phone_DockStates[k]); return 1; }
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
