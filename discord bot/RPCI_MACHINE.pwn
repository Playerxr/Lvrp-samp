#include <a_samp>
#include <discord-connector>

// ================== CONFIGURATION ==================
#define LOG_CHANNEL_ID "1485722864400334918"   // ← Remplace par l'ID du salon ╰🤖・〢𝐜𝐦𝐝

new DCC_Channel:g_LogChannel;

// ==================================================

public OnFilterScriptInit()
{
    g_LogChannel = DCC_FindChannelById(LOG_CHANNEL_ID);
    
    if (g_LogChannel == DCC_INVALID_CHANNEL)
        print("[Discord Logs] ERREUR : Salon ╰🤖・〢𝐜𝐦𝐝 non trouvé ! Vérifie l'ID.");
    else
        print("[Discord Logs] === FULL LOGS POUR LVRP CHARGÉ ===\n"
              "→ Salon : ╰🤖・〢𝐜𝐦𝐝\n"
              "→ Commandes admin (/a ...) ignorées automatiquement\n"
              "→ Seules les commandes civiles sont envoyées");
    
    return 1;
}

// ====================== CONNEXIONS ======================
public OnPlayerConnect(playerid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    
    new msg[250];
    format(msg, sizeof(msg), "🔌 **%s** (ID: %d) s'est connecté au serveur", name, playerid);
    DCC_SendChannelMessage(g_LogChannel, msg);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    
    new msg[250];
    format(msg, sizeof(msg), "❌ **%s** (ID: %d) s'est déconnecté", name, playerid);
    DCC_SendChannelMessage(g_LogChannel, msg);
    return 1;
}

// ====================== CHAT ======================
public OnPlayerText(playerid, text[])
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    
    new msg[450];
    format(msg, sizeof(msg), "💬 **%s** (ID: %d) : %s", name, playerid, text);
    DCC_SendChannelMessage(g_LogChannel, msg);
    return 1;
}

// ====================== COMMANDES CIVILES ======================
public OnPlayerCommandText(playerid, cmdtext[])
{
    // Ignore TOUTES les commandes admin qui commencent par /a ou /A
    if (cmdtext[0] == '/' && (cmdtext[1] == 'a' || cmdtext[1] == 'A'))
        return 0;

    // Si c'est une commande civile (/pay, /stats, /me, /do, /job, etc.)
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    
    new msg[500];
    format(msg, sizeof(msg), 
        "⌨️ **Commande civile utilisée** dans ╰🤖・〢𝐜𝐦𝐝\n" \
        "👤 Joueur : **%s** (ID: %d)\n" \
        "📝 Commande : `%s`",
        name, playerid, cmdtext);
    
    DCC_SendChannelMessage(g_LogChannel, msg);
    
    return 0; // La commande continue de fonctionner normalement
}