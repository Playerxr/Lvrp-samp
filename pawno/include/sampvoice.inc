// =====================================================================================
// SAMPVOICE INC - AFRP edition
// =====================================================================================

#if defined SAMPVOICE_INC
    #endinput
#endif
#define SAMPVOICE_INC

// =====================================================================================
// Types
// =====================================================================================
#define SV_VOID:
#define SV_UINT:
#define SV_LSTREAM:
#define SV_GSTREAM:
#define SV_BOOL:

// =====================================================================================
// Constantes generales
// =====================================================================================
#define SV_NULL        (0)
#define SV_INFINITY    (0xFFFF)
#define SV_FALSE       (0)
#define SV_TRUE        (1)
#define SV_NONE_MICRO  (-1)
#define SV_KEY_NONE    (0x00)

// Touches d'activation (key codes SA-MP)
#define SV_KEY_B       (0x42)
#define SV_KEY_N       (0x4E)

// Couleurs
#define SV_COLOR_RED    0xFF0000FF
#define SV_COLOR_GREEN  0x00FF00FF
#define SV_COLOR_BLUE   0x0000FFFF
#define SV_COLOR_WHITE  0xFFFFFFFF
#define SV_COLOR_GOLD   0xFFD700FF

// =====================================================================================
// AFRP - Constantes pour les EMOTES parler (extensions LVRP)
// =====================================================================================
// 2 emotes upper-body walkable, appliquees automatiquement sur push-to-talk B
// Le joueur peut continuer a marcher/courir pendant que l'anim s'execute.

// EMOTE 1 - Local (parler de proximite, voix dans 20m)
// prtial_gngtlkA = gesticulation mains, animation partielle upper-body
#define SV_EMOTE_LOCAL_LIB        "GANGS"
#define SV_EMOTE_LOCAL_NAME       "prtial_gngtlkA"
#define SV_EMOTE_LOCAL_FDELTA     4.0

// EMOTE 2 - Radio/Talkie (talkStats == 3, frequence)
// prtial_gngtlkB = geste different (variante), pour distinguer visuellement
#define SV_EMOTE_RADIO_LIB        "GANGS"
#define SV_EMOTE_RADIO_NAME       "prtial_gngtlkB"
#define SV_EMOTE_RADIO_FDELTA     4.0

// Mode talk (valeur du PVar "talkStats")
#define SV_TALK_MODE_LOCAL        0
#define SV_TALK_MODE_RADIO        3

// Touche de talk (push-to-talk)
#define SV_TALK_KEY               0x42

// Anti-spam
#define SV_VOICE_MAX_DURATION     30   // sec max consecutives
#define SV_VOICE_COOLDOWN         5    // sec pause apres spam

// Rayon local en metres
#define SV_VOICE_LOCAL_RADIUS     20.0

// Max canaux radio (frequences)
#define SV_VOICE_MAX_RADIO        999

// =====================================================================================
// Natives - Info plugin client
// =====================================================================================
native SvGetVersion(playerid);          // Retourne version client, SV_NULL si pas le plugin
native SvHasMicro(playerid);            // Retourne SV_TRUE/SV_FALSE
native SvIsPlayerSpeaking(playerid);    // Retourne SV_TRUE/SV_FALSE

// =====================================================================================
// Natives - Gestion touches
// =====================================================================================
native SvAddKey(playerid, keyid);
native SvRemoveKey(playerid, keyid);
native SvRemoveAllKeys(playerid);
native SvHasKey(playerid, keyid);

// =====================================================================================
// Natives - Streams Local (proximite)
// =====================================================================================
native SV_LSTREAM:SvCreateDLStreamAtPlayer(Float:radius, maxplayers, playerid, color, const name[]);
native SvDeleteStream(SV_LSTREAM:streamid);
native SvAttachSpeakerToStream(SV_LSTREAM:streamid, playerid);
native SvDetachSpeakerFromStream(SV_LSTREAM:streamid, playerid);
native SvHasSpeakerInStream(SV_LSTREAM:streamid, playerid);

// =====================================================================================
// Natives - Streams Global (telephone/radio)
// =====================================================================================
native SV_GSTREAM:SvCreateGStream(color, const name[]);
native SvAttachListenerToStream(SV_GSTREAM:streamid, playerid);
native SvDetachListenerFromStream(SV_GSTREAM:streamid, playerid);
native SvHasListenerInStream(SV_GSTREAM:streamid, playerid);

// =====================================================================================
// Callbacks emis par le plugin
// =====================================================================================
forward OnPlayerVoiceUpdate(playerid);
// Touches d'activation (le plugin appelle automatiquement quand SvAddKey est utilisee)
forward SV_VOID:OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid);
forward SV_VOID:OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid);

// =====================================================================================
// Callbacks emis par le FS sampvoice.pwn (pour le gamemode)
// =====================================================================================
forward OnPlayerVoiceStart(playerid);
forward OnPlayerVoiceStop(playerid);

// Remote functions (le gamemode appelle ces fonctions sur le FS)
forward JoinPrivateVoiceChannel(playerid, targetid);
forward LeavePrivateVoiceChannel(playerid);
forward JoinGroupVoiceChannel(playerid, frequency_id);
forward LeaveGroupVoiceChannel(playerid);
