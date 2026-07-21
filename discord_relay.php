<?php
/*
 * discord_relay.php - Relais SA-MP (HTTP) -> Discord (HTTPS)
 * ----------------------------------------------------------
 * A UPLOADER sur ton hebergement web (le meme que ton site, R2, etc.).
 * Le serveur SA-MP l'appelle en HTTP simple (via le natif HTTP()), et ce
 * script transmet le message a ton webhook Discord en HTTPS avec @everyone.
 *
 * >>> PAS BESOIN D'UN PLUGIN DISCORD DANS SA-MP. <<<
 *
 * INSTALLATION :
 * 1. Cree un webhook Discord : Parametres du salon -> Integrations ->
 *    Webhooks -> Nouveau webhook -> Copier l'URL du webhook.
 * 2. Colle cette URL ci-dessous dans $WEBHOOK_URL.
 * 3. Choisis une cle secrete (n'importe quel mot de passe) dans $SECRET_KEY.
 * 4. Uploade ce fichier sur ton hebergement, note son URL publique
 *    (ex: http://ton-site.com/discord_relay.php).
 * 5. Cote serveur SA-MP, cree scriptfiles/discord.cfg avec :
 *      ligne 1 = http://ton-site.com/discord_relay.php
 *      ligne 2 = la meme cle secrete que $SECRET_KEY
 *
 * IMPORTANT : le natif HTTP() de SA-MP ne fait QUE du HTTP (pas HTTPS).
 * L'URL du relais dans discord.cfg doit donc commencer par http:// et ton
 * hebergement doit repondre en HTTP (sans redirection forcee vers https://).
 */

// ================== A CONFIGURER ==================
$WEBHOOK_URL = "COLLE_ICI_TON_URL_DE_WEBHOOK_DISCORD";
$SECRET_KEY  = "change-moi-cle-secrete";
// =================================================

// Verification de la cle secrete (evite que n'importe qui spamme ton Discord)
$key = isset($_GET['key']) ? $_GET['key'] : '';
if (!hash_equals($SECRET_KEY, $key)) {
    http_response_code(403);
    echo "forbidden";
    exit;
}

$msg = isset($_GET['msg']) ? trim($_GET['msg']) : '';
if ($msg === '') {
    http_response_code(400);
    echo "empty";
    exit;
}

// Securite : limite la taille du message
if (strlen($msg) > 1500) {
    $msg = substr($msg, 0, 1500);
}

$everyone = isset($_GET['everyone']) && $_GET['everyone'] == '1';
$content  = ($everyone ? "@everyone\n" : "") . $msg;

$payload = array(
    "content"          => $content,
    "allowed_mentions" => array(
        "parse" => $everyone ? array("everyone") : array()
    )
);

$ch = curl_init($WEBHOOK_URL);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, array("Content-Type: application/json"));
curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($payload));
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 10);

$result = curl_exec($ch);
$httpcode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

// Discord repond 204 No Content en cas de succes
if ($httpcode >= 200 && $httpcode < 300) {
    http_response_code(200);
    echo "ok";
} else {
    http_response_code(502);
    echo "discord_error_" . $httpcode;
}
