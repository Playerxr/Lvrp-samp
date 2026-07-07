// =====================================================================================
// SAMPVOICE - Version AFRP avec 2 EMOTES walkable (parler + marcher)
// =====================================================================================
// Pile complete :
// - Local 20m (couleur rouge)        -> emote upper-body "prtial_gngtlkA" (mains)
// - Radio (couleur verte)            -> emote upper-body "prtial_gngtlkB" (variante)
// - Telephone (couleur or)           -> emote phone_talk laisse au gamemode
// - Anti-spam : 30s max, cooldown 5s
// - TOUTES les constantes sont dans sampvoice.inc (centralise)
// =====================================================================================

#include <a_samp>
#include <sampvoice>

// =====================================================================================
// Variables globales
// =====================================================================================
new SV_LSTREAM:local_stream[MAX_PLAYERS] = { SV_NULL, ... };
new voice_phonecall_targetid[MAX_PLAYERS];
new SV_GSTREAM:phone_stream[MAX_PLAYERS] = { SV_NULL, ... };
new voice_radiocall_radioid[MAX_PLAYERS];
new SV_GSTREAM:radio_stream[SV_VOICE_MAX_RADIO] = { SV_NULL, ... };

new bool:voice_IsTalking[MAX_PLAYERS] = false;
new voice_TalkDuration[MAX_PLAYERS]   = 0;
// voice_TalkCooldown supprime - plus de pause obligatoire apres timeout

#define VOICE_COLOR_LOCAL  0xFF4444FF
#define VOICE_COLOR_PHONE  0xFFD700FF
#define VOICE_COLOR_RADIO  0x00FF80FF

// =====================================================================================
// HELPERS - Application des 2 emotes walkable
// =====================================================================================
// Important : ces animations sont PARTIELLES (prefixe "prtial_") = upper-body only.
// Le joueur peut continuer a marcher/courir/sprinter pendant que l'anim s'execute.
// loop=1, lockx=0, locky=0, freeze=0, time=0 -> pas de blocage des controles.
//
// Pour preload : ApplyAnimation avec time=1 a la connexion (cf. OnPlayerConnect)
// =====================================================================================

// EMOTE 1 - Discussion locale (voix proche)
stock voice_ApplyEmoteLocal(playerid)
{
    if(!IsPlayerConnected(playerid)) return;
    if(IsPlayerInAnyVehicle(playerid)) return;   // skip si en vehicule
    ApplyAnimation(playerid,
        SV_EMOTE_LOCAL_LIB,
        SV_EMOTE_LOCAL_NAME,
        SV_EMOTE_LOCAL_FDELTA,
        1,  // loop
        0,  // lockx
        0,  // locky
        0,  // freeze
        0); // time (0 = boucle infinie tant que loop=1)
}

// EMOTE 2 - Discussion radio (talkie)
stock voice_ApplyEmoteRadio(playerid)
{
    if(!IsPlayerConnected(playerid)) return;
    if(IsPlayerInAnyVehicle(playerid)) return;
    ApplyAnimation(playerid,
        SV_EMOTE_RADIO_LIB,
        SV_EMOTE_RADIO_NAME,
        SV_EMOTE_RADIO_FDELTA,
        1, 0, 0, 0, 0);
}

// Dispatcher : choisit l'emote selon le mode de talk
stock voice_ApplyEmote(playerid, talkMode)
{
    switch(talkMode)
    {
        case SV_TALK_MODE_RADIO: voice_ApplyEmoteRadio(playerid);
        default:                 voice_ApplyEmoteLocal(playerid);
    }
}

// Clear emote (relachement B ou timeout)
stock voice_ClearEmote(playerid)
{
    if(!IsPlayerConnected(playerid)) return;
    if(IsPlayerInAnyVehicle(playerid)) return;
    ClearAnimations(playerid);
}

// Preload des 2 emotes pour eviter le lag au premier usage
stock voice_PreloadEmotes(playerid)
{
    if(!IsPlayerConnected(playerid)) return;
    if(IsPlayerInAnyVehicle(playerid)) return;
    // time=1 : charge l'anim sans la jouer visiblement
    ApplyAnimation(playerid, SV_EMOTE_LOCAL_LIB, SV_EMOTE_LOCAL_NAME, 4.0, 0, 0, 0, 0, 1);
    ApplyAnimation(playerid, SV_EMOTE_RADIO_LIB, SV_EMOTE_RADIO_NAME, 4.0, 0, 0, 0, 0, 1);
}

// =====================================================================================
// Touche d'activation B (SV_TALK_KEY = 0x42)
// =====================================================================================
public SV_VOID:OnPlayerActivationKeyPress(SV_UINT:playerid, SV_UINT:keyid)
{
    if (keyid != SV_TALK_KEY) return;

    new pvarTalkStats = GetPVarInt(playerid, "talkStats");

    voice_IsTalking[playerid] = true;

    // FIX BUG VISUEL : ne plus appliquer d'animation a chaque pression de B
    // Les prtial_gngtlk causaient des conflits avec les anims du gamemode
    // (T-pose, weapon stuck, animation desync apres entree/sortie vehicule)
    // voice_ApplyEmote(playerid, pvarTalkStats);

    CallRemoteFunction("OnPlayerVoiceStart", "i", playerid);

    if (pvarTalkStats != SV_TALK_MODE_RADIO) {
        new callid = voice_phonecall_targetid[playerid];
        if (callid != 65535) {
            if (phone_stream[playerid]) {
                if (!SvHasSpeakerInStream(phone_stream[playerid], playerid)) {
                    SvAttachSpeakerToStream(phone_stream[playerid], playerid);
                }
            }
        }
        else if (local_stream[playerid]) {
            if (!SvHasSpeakerInStream(local_stream[playerid], playerid)) {
                SvAttachSpeakerToStream(local_stream[playerid], playerid);
            }
        }
    }
    else {
        new radioid = voice_radiocall_radioid[playerid];
        if (radioid != 0) {
            if (radio_stream[radioid]) {
                if (!SvHasSpeakerInStream(radio_stream[radioid], playerid)) {
                    SvAttachSpeakerToStream(radio_stream[radioid], playerid);
                }
            }
        }
    }
}

public SV_VOID:OnPlayerActivationKeyRelease(SV_UINT:playerid, SV_UINT:keyid)
{
    if (keyid != SV_TALK_KEY) return;

    voice_IsTalking[playerid] = false;
    voice_TalkDuration[playerid] = 0;

    // FIX : plus besoin de clear puisque on n'applique plus rien au start
    // voice_ClearEmote(playerid);

    CallRemoteFunction("OnPlayerVoiceStop", "i", playerid);

    new callid = voice_phonecall_targetid[playerid];
    if (callid != 65535 && phone_stream[playerid]) {
        if (SvHasSpeakerInStream(phone_stream[playerid], playerid)) {
            SvDetachSpeakerFromStream(phone_stream[playerid], playerid);
        }
    }
    if (local_stream[playerid]) {
        if (SvHasSpeakerInStream(local_stream[playerid], playerid)) {
            SvDetachSpeakerFromStream(local_stream[playerid], playerid);
        }
    }
    new radioid = voice_radiocall_radioid[playerid];
    if (radioid != 0 && radio_stream[radioid]) {
        if (SvHasSpeakerInStream(radio_stream[radioid], playerid)) {
            SvDetachSpeakerFromStream(radio_stream[radioid], playerid);
        }
    }
}

// =====================================================================================
// Timer anti-spam (1 seconde)
// =====================================================================================
forward voice_AntispamTick();
public voice_AntispamTick()
{
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (!IsPlayerConnected(i)) continue;
        if (!voice_IsTalking[i]) continue;

        voice_TalkDuration[i]++;
        // Couper le micro apres SV_VOICE_MAX_DURATION secondes (sans cooldown ni message)
        if (voice_TalkDuration[i] >= SV_VOICE_MAX_DURATION) {
            voice_IsTalking[i] = false;
            voice_TalkDuration[i] = 0;

            if (phone_stream[i] && SvHasSpeakerInStream(phone_stream[i], i))
                SvDetachSpeakerFromStream(phone_stream[i], i);
            if (local_stream[i] && SvHasSpeakerInStream(local_stream[i], i))
                SvDetachSpeakerFromStream(local_stream[i], i);
            new radioid = voice_radiocall_radioid[i];
            if (radioid != 0 && radio_stream[radioid] && SvHasSpeakerInStream(radio_stream[radioid], i))
                SvDetachSpeakerFromStream(radio_stream[radioid], i);

            CallRemoteFunction("OnPlayerVoiceStop", "i", i);
            // Pas de message, pas de cooldown : le joueur peut reparler immediatement
        }
    }
}

// =====================================================================================
// Connect / Disconnect
// =====================================================================================
public OnPlayerConnect(playerid) {
    if (SvGetVersion(playerid) == SV_NULL) {
        SetPVarInt(playerid, "hasVoiceOnClient", 0);
    }
    else if (SvHasMicro(playerid) == SV_FALSE) {
        SetPVarInt(playerid, "hasVoiceOnClient", 2);
    }
    else if ((local_stream[playerid] = SvCreateDLStreamAtPlayer(SV_VOICE_LOCAL_RADIUS, SV_INFINITY, playerid, VOICE_COLOR_LOCAL, "Local 20m")))
    {
        SvAddKey(playerid, SV_TALK_KEY);
        SetPVarInt(playerid, "hasVoiceOnClient", 1);
    }
    voice_phonecall_targetid[playerid] = 65535;
    voice_radiocall_radioid[playerid] = 0;
    voice_IsTalking[playerid] = false;
    voice_TalkDuration[playerid] = 0;

    // FIX : plus de preload puisqu'on n'utilise plus les emotes
    // SetTimerEx("_voice_PreloadDelayed", 2000, 0, "i", playerid);
    return 1;
}

forward _voice_PreloadDelayed(playerid);
public _voice_PreloadDelayed(playerid)
{
    voice_PreloadEmotes(playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason) {
    if (local_stream[playerid]) {
        SvDeleteStream(local_stream[playerid]);
        local_stream[playerid] = SV_NULL;
    }
    LeavePrivateVoiceChannel(playerid);
    LeaveGroupVoiceChannel(playerid);
    voice_IsTalking[playerid] = false;
    voice_TalkDuration[playerid] = 0;
    return 1;
}

// =====================================================================================
// Init / Exit
// =====================================================================================
public OnFilterScriptInit() {
    new string[128];
    for (new i = 0; i < MAX_PLAYERS; i++) {
        format(string, sizeof(string), "Phone-%i", i);
        phone_stream[i] = SvCreateGStream(VOICE_COLOR_PHONE, string);
    }
    for (new i = 0; i < SV_VOICE_MAX_RADIO; i++) {
        format(string, sizeof(string), "Radio-%i", i);
        radio_stream[i] = SvCreateGStream(VOICE_COLOR_RADIO, string);
    }
    SetTimer("voice_AntispamTick", 1000, 1);
    print("[SampVoice AFRP] FS charge - local 20m, limite duree, sans cooldown");
    return 1;
}

public OnFilterScriptExit() {
    for (new i = 0; i < MAX_PLAYERS; i++) {
        if (phone_stream[i]) SvDeleteStream(phone_stream[i]);
    }
    for (new i = 0; i < SV_VOICE_MAX_RADIO; i++) {
        if (radio_stream[i]) SvDeleteStream(radio_stream[i]);
    }
    return 1;
}

// =====================================================================================
// Remote Functions (appelables depuis le gamemode LVRP)
// =====================================================================================
forward JoinPrivateVoiceChannel(playerid, targetid);
forward LeavePrivateVoiceChannel(playerid);

public JoinPrivateVoiceChannel(playerid, targetid)
{
    LeavePrivateVoiceChannel(playerid);
    if (phone_stream[targetid]) {
        if (!SvHasListenerInStream(phone_stream[targetid], playerid)) {
            SvAttachListenerToStream(phone_stream[targetid], playerid);
        }
    }
    voice_phonecall_targetid[playerid] = targetid;
    return 1;
}

public LeavePrivateVoiceChannel(playerid)
{
    new oldchannelid = voice_phonecall_targetid[playerid];
    voice_phonecall_targetid[playerid] = 65535;
    if (oldchannelid != 65535 && phone_stream[oldchannelid]) {
        if (SvHasSpeakerInStream(phone_stream[oldchannelid], playerid))
            SvDetachSpeakerFromStream(phone_stream[oldchannelid], playerid);
        if (SvHasListenerInStream(phone_stream[oldchannelid], playerid))
            SvDetachListenerFromStream(phone_stream[oldchannelid], playerid);
    }
    return 1;
}

forward JoinGroupVoiceChannel(playerid, frequency_id);
forward LeaveGroupVoiceChannel(playerid);

public JoinGroupVoiceChannel(playerid, frequency_id)
{
    LeaveGroupVoiceChannel(playerid);
    if (radio_stream[frequency_id]) {
        if (!SvHasListenerInStream(radio_stream[frequency_id], playerid)) {
            SvAttachListenerToStream(radio_stream[frequency_id], playerid);
        }
    }
    voice_radiocall_radioid[playerid] = frequency_id;
    return 1;
}

public LeaveGroupVoiceChannel(playerid)
{
    new oldchannelid = voice_radiocall_radioid[playerid];
    voice_radiocall_radioid[playerid] = 0;
    if (oldchannelid != 0 && radio_stream[oldchannelid]) {
        if (SvHasSpeakerInStream(radio_stream[oldchannelid], playerid))
            SvDetachSpeakerFromStream(radio_stream[oldchannelid], playerid);
        if (SvHasListenerInStream(radio_stream[oldchannelid], playerid))
            SvDetachListenerFromStream(radio_stream[oldchannelid], playerid);
    }
    return 1;
}
