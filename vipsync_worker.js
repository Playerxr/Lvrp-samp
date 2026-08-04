/*
 * vipsync_worker.js - Relais HTTP -> HTTPS pour la synchro VIP (Firebase)
 *
 * POURQUOI CE FICHIER EXISTE
 * --------------------------
 * Le serveur SA-MP ne sait faire que du HTTP simple (pas de HTTPS/TLS).
 * Firebase, lui, n'accepte QUE du HTTPS. Le serveur de jeu ne peut donc pas
 * appeler Firebase directement : la synchro VIP echouait a chaque essai
 * depuis son installation, en silence.
 *
 * Ce petit relais se met au milieu : le serveur de jeu l'appelle en HTTP
 * simple, lui va chercher Firebase en HTTPS, et renvoie la reponse telle
 * quelle. C'est exactement le meme principe que le relais deja utilise pour
 * les annonces Discord.
 *
 *      Serveur SA-MP  --HTTP-->  ce Worker  --HTTPS-->  Firebase
 *
 *
 * COMMENT LE METTRE EN PLACE (5 minutes, gratuit)
 * ----------------------------------------------
 * 1. Va sur https://dash.cloudflare.com  ->  "Workers & Pages"  ->  "Create"
 * 2. Cree un Worker, donne-lui un nom (ex: lvrp-vip)
 * 3. Clique "Edit code", efface tout, colle CE fichier en entier, puis "Deploy"
 * 4. Cloudflare te donne une adresse du style :
 *        https://lvrp-vip.TON-COMPTE.workers.dev
 * 5. Cree le fichier  scriptfiles/vipsync.cfg  sur ton serveur de jeu,
 *    avec UNE SEULE LIGNE : cette adresse.
 *        lvrp-vip.TON-COMPTE.workers.dev/
 *    (avec ou sans le https:// devant, le serveur le retire tout seul)
 * 6. Redemarre le serveur. Le log doit afficher :
 *        [VIPSYNC] Module demarre - verification VIP/Cash toutes les 30s via le relais.
 *    S'il affiche "DESACTIVE", c'est que vipsync.cfg est absent ou vide.
 *
 *
 * NOTE DE SECURITE
 * ----------------
 * Ce Worker ne peut lire QUE la branche definie dans CHEMINS_AUTORISES
 * ci-dessous, sur TA base Firebase. Il ne peut rien ecrire, rien supprimer,
 * et ne peut pas servir a lire une autre base. C'est volontaire : meme si
 * quelqu'un trouve l'adresse du Worker, il ne peut rien en faire d'autre que
 * lire la liste des VIP en attente.
 *
 * Si ta base Firebase demande un jeton d'authentification, ajoute-le dans
 * FIREBASE_SECRET ci-dessous : il restera cote Cloudflare et ne circulera
 * jamais jusqu'au serveur de jeu.
 */

// ─── CONFIGURATION ──────────────────────────────────────────────────────────

// Ta base Firebase (sans https:// ni slash final).
const FIREBASE_HOST = "afrp-f0ef7-default-rtdb.europe-west1.firebasedatabase.app";

// Jeton Firebase, si ta base n'est pas en lecture publique. Laisse "" sinon.
const FIREBASE_SECRET = "";

// Seules ces branches peuvent etre lues. N'ajoute une entree que si tu sais
// pourquoi : c'est ce qui empeche le Worker de servir a autre chose.
const CHEMINS_AUTORISES = ["pending_vip_grants"];

// ─── CODE ───────────────────────────────────────────────────────────────────

export default {
  async fetch(request) {
    const url = new URL(request.url);
    const chemin = url.searchParams.get("path") || "pending_vip_grants";

    // Refuse tout chemin non explicitement autorise.
    if (!CHEMINS_AUTORISES.includes(chemin)) {
      return new Response("null", {
        status: 403,
        headers: { "Content-Type": "text/plain" },
      });
    }

    let cible = `https://${FIREBASE_HOST}/${chemin}.json`;
    if (FIREBASE_SECRET) {
      cible += `?auth=${encodeURIComponent(FIREBASE_SECRET)}`;
    }

    try {
      const reponse = await fetch(cible, {
        headers: { Accept: "application/json" },
      });

      // On renvoie le JSON tel quel : le serveur de jeu attend exactement le
      // format Firebase, y compris la chaine "null" quand il n'y a rien.
      const corps = await reponse.text();

      return new Response(corps, {
        status: reponse.ok ? 200 : reponse.status,
        headers: { "Content-Type": "text/plain; charset=utf-8" },
      });
    } catch (err) {
      // Le serveur de jeu ignore tout ce qui n'est pas un code 200, et
      // "null" est sa valeur "rien a traiter" : en cas de panne du relais,
      // il ne se passe rien plutot qu'un comportement imprevu.
      return new Response("null", {
        status: 502,
        headers: { "Content-Type": "text/plain" },
      });
    }
  },
};
