/*
 * discord_relay_worker.js - Relais SA-MP -> Discord via Cloudflare Worker
 * -----------------------------------------------------------------------
 * ALTERNATIVE GRATUITE au discord_relay.php : pas besoin d'hebergement PHP,
 * un simple Worker Cloudflare (gratuit, 100.000 requetes/jour) suffit.
 * Tu as deja un compte Cloudflare (utilise pour le cache R2 du launcher).
 *
 * INSTALLATION (5 minutes) :
 * 1. Va sur https://dash.cloudflare.com -> Workers & Pages -> Create ->
 *    Create Worker -> donne un nom (ex: "lvrp-discord") -> Deploy.
 * 2. Clique "Edit code", EFFACE tout et COLLE ce fichier entier.
 * 3. Remplace WEBHOOK_URL ci-dessous par l'URL de ton webhook Discord
 *    (salon Discord -> Parametres -> Integrations -> Webhooks -> Nouveau
 *    webhook -> Copier l'URL).
 * 4. Remplace SECRET_KEY par un mot de passe de ton choix.
 * 5. Clique "Deploy". Note l'adresse du worker, du type :
 *       lvrp-discord.TONCOMPTE.workers.dev
 * 6. Sur ton serveur SA-MP, cree le fichier scriptfiles/discord.cfg :
 *       ligne 1 : http://lvrp-discord.TONCOMPTE.workers.dev/
 *       ligne 2 : le meme mot de passe que SECRET_KEY
 * 7. Redemarre le serveur -> l'annonce "@everyone serveur en ligne" part
 *    automatiquement a chaque demarrage.
 *
 * TEST RAPIDE (sans le serveur) : ouvre dans ton navigateur
 *   https://lvrp-discord.TONCOMPTE.workers.dev/?key=TONSECRET&everyone=0&msg=test
 * -> le message "test" doit apparaitre dans ton salon Discord.
 */

// ================== A CONFIGURER ==================
const WEBHOOK_URL = "COLLE_ICI_TON_URL_DE_WEBHOOK_DISCORD";
const SECRET_KEY  = "change-moi-cle-secrete";
// ==================================================

export default {
  async fetch(request) {
    const url = new URL(request.url);

    const key = url.searchParams.get("key") || "";
    if (key !== SECRET_KEY) {
      return new Response("forbidden", { status: 403 });
    }

    let msg = (url.searchParams.get("msg") || "").trim();
    if (msg === "") {
      return new Response("empty", { status: 400 });
    }
    if (msg.length > 1500) msg = msg.slice(0, 1500);

    const everyone = url.searchParams.get("everyone") === "1";
    const content = (everyone ? "@everyone\n" : "") + msg;

    const payload = {
      content: content,
      allowed_mentions: { parse: everyone ? ["everyone"] : [] }
    };

    const resp = await fetch(WEBHOOK_URL, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(payload)
    });

    if (resp.ok) {
      return new Response("ok", { status: 200 });
    }
    return new Response("discord_error_" + resp.status, { status: 502 });
  }
};
