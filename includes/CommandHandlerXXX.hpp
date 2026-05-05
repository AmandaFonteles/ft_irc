#ifndef COMMANDHANDLER_HPP
# define COMMANDHANDLER_HPP

/*
 * ===========================================================================
 *  CommandHandler.hpp   —   PERSONNE B
 * ===========================================================================
 *
 *  CommandHandler est le ROUTEUR DE COMMANDES du serveur.
 *  Il reçoit un Message parsé et appelle le bon handler selon command.
 *
 * ---------------------------------------------------------------------------
 *  POSITION DANS LE PIPELINE GLOBAL
 * ---------------------------------------------------------------------------
 *
 *  [PERSONNE A]  poll() → recv() → client.inBuffer +=
 *  [PERSONNE B]  extractLines() → parseLine() → dispatch()  ← ICI
 *  [PERSONNE B]  handlePass/Nick/User/Ping/Quit             ← ICI
 *  [PERSONNE C]  handleJoin/Privmsg/Kick/Invite/Topic/Mode  ← LEURS HANDLERS
 *
 *  dispatch() est le POINT D'ENTRÉE UNIQUE : toutes les commandes passent
 *  par lui. Il décide qui traite quoi.
 *
 * ---------------------------------------------------------------------------
 *  LES ÉTATS D'UN CLIENT IRC
 * ---------------------------------------------------------------------------
 *
 *  Un client IRC passe par des états distincts. C'est la "machine à états"
 *  la plus importante du serveur. Personne B la gère entièrement.
 *
 *  ÉTAT 1 — CONNECTÉ (non enregistré, pas de PASS encore)
 *  ──────────────────────────────────────────────────────
 *  Le client vient de se connecter TCP. Il n'a rien envoyé.
 *    client.passOk     = false
 *    client.nickname   = ""
 *    client.username   = ""
 *    client.registered = false
 *  Commandes autorisées : PASS, NICK, USER, PING, PONG, QUIT
 *  Toute autre commande → ERR_NOTREGISTERED (451)
 *
 *  ÉTAT 2 — AUTHENTIFIÉ (PASS reçu et valide)
 *  ───────────────────────────────────────────
 *  Le client a envoyé le bon mot de passe.
 *    client.passOk     = true
 *    client.nickname   = "" ou défini si NICK avant/après PASS
 *    client.registered = false
 *  Commandes autorisées : NICK, USER, PING, PONG, QUIT
 *
 *  ÉTAT 3 — ENREGISTRÉ (PASS + NICK + USER tous valides)
 *  ──────────────────────────────────────────────────────
 *  Le client a complété la séquence d'enregistrement.
 *    client.passOk     = true
 *    client.nickname   != ""
 *    client.username   != ""
 *    client.registered = true
 *  → On envoie la séquence 001-004 (RPL_WELCOME, YOURHOST, CREATED, MYINFO)
 *  → Toutes les commandes IRC sont désormais autorisées
 *
 *  DIAGRAMME DE TRANSITIONS :
 *
 *    TCP Connect
 *        ↓
 *    [NON ENREGISTRÉ]
 *        ↓ PASS correct
 *    [PASS OK]
 *        ↓ NICK valide + USER reçu (dans n'importe quel ordre)
 *    [REGISTERED] → Séquence 001-004 envoyée une fois
 *
 *  NOTE : NICK peut arriver AVANT ou APRÈS USER. Les deux ordres doivent
 *  fonctionner. C'est pourquoi on vérifie dans handleNick() ET handleUser()
 *  si la registration est désormais complète.
 *
 * ---------------------------------------------------------------------------
 *  ORDRE DES COMMANDES D'ENREGISTREMENT (PASS → NICK → USER)
 * ---------------------------------------------------------------------------
 *
 *  La RFC 1459 recommande l'ordre PASS → NICK → USER.
 *  En pratique, les clients envient souvent NICK et USER presque simultanément.
 *  Certains clients envoient : PASS, NICK, USER
 *  D'autres envoient         : CAP LS, PASS, NICK, USER (avec CAP negociation)
 *  Pour ft_irc, on ignore CAP et on accepte PASS/NICK/USER dans n'importe
 *  quel ordre tant que les trois sont présents.
 *
 *  RÈGLE SUR PASS :
 *  Si un mot de passe serveur est configuré, PASS doit être envoyé AVANT
 *  que la registration soit complète. Si PASS n'est pas envoyé (ou mauvais
 *  mot de passe), on rejette le client avec ERR_PASSWDMISMATCH + ERROR_MSG.
 *
 * ---------------------------------------------------------------------------
 *  INTERFACES AVEC PERSONNE A ET PERSONNE C
 * ---------------------------------------------------------------------------
 *
 *  INTERFACES AVEC PERSONNE A :
 *  ┌─────────────────────────────────────────────────────────────────────┐
 *  │  dispatch() reçoit des RÉFÉRENCES à Server et Client.              │
 *  │  Ces types sont définis dans Server.hpp et Client.hpp (Personne A). │
 *  │                                                                     │
 *  │  CommandHandler UTILISE de Server :                                 │
 *  │    server.getName()           → nom du serveur pour les Replies     │
 *  │    server.getPassword()       → mot de passe pour handlePass        │
 *  │    server.findClientByNick()  → chercher un client par son nick     │
 *  │    server.getClients()        → map<int,Client> pour NICK unicité   │
 *  │    server.getChannels()       → map<string,Channel> pour canaux     │
 *  │    server.removeClient(fd)    → supprimer un client (handleQuit)    │
 *  │                                                                     │
 *  │  CommandHandler UTILISE de Client :                                 │
 *  │    client.fd                  → identificateur de la connexion      │
 *  │    client.nickname            → pseudo IRC                          │
 *  │    client.username            → username (défini par USER)          │
 *  │    client.realname            → nom complet (trailing de USER)      │
 *  │    client.hostname            → hostname de la connexion            │
 *  │    client.passOk              → bool : PASS validé ?                │
 *  │    client.registered          → bool : registration complète ?      │
 *  │    client.enqueue(msg)        → ajouter msg à outBuffer             │
 *  └─────────────────────────────────────────────────────────────────────┘
 *
 *  INTERFACES AVEC PERSONNE C :
 *  ┌─────────────────────────────────────────────────────────────────────┐
 *  │  dispatch() délègue à des fonctions de Personne C pour :            │
 *  │    JOIN, PRIVMSG, KICK, INVITE, TOPIC, MODE                         │
 *  │                                                                     │
 *  │  Deux approches possibles pour cette délégation :                   │
 *  │                                                                     │
 *  │  APPROCHE A (recommandée pour l'intégration simple) :               │
 *  │    CommandHandler::dispatch() appelle des méthodes statiques         │
 *  │    déclarées dans CommandHandler.hpp mais DÉFINIES dans un fichier   │
 *  │    séparé Channel_commands.cpp géré par Personne C.                 │
 *  │    → Un seul fichier .hpp, deux fichiers .cpp                       │
 *  │                                                                     │
 *  │  APPROCHE B (séparation plus nette) :                               │
 *  │    Personne C crée ChannelHandler.hpp/.cpp avec ses propres         │
 *  │    méthodes statiques. dispatch() #include "ChannelHandler.hpp"     │
 *  │    et appelle ChannelHandler::handleJoin(...).                      │
 *  │    → Deux fichiers .hpp indépendants                                │
 *  │                                                                     │
 *  │  Pour ft_irc en équipe de 3, l'APPROCHE A est recommandée car elle  │
 *  │  évite les conflits de merge sur dispatch() : Personne B écrit le   │
 *  │  squelette, Personne C remplit ses handlers séparément.             │
 *  └─────────────────────────────────────────────────────────────────────┘
 *
 * ===========================================================================
 */

# include "Message.hpp"
# include "Replies.hpp"

/*
 * FORWARD DECLARATIONS
 * ─────────────────────
 * On déclare Server et Client sans les inclure pour éviter les inclusions
 * circulaires. CommandHandler.hpp est inclus dans Server.cpp (Personne A)
 * qui définit déjà Server et Client.
 *
 * Forward declaration = "Cette classe existe, tu la verras plus tard."
 * Permet d'utiliser Server& et Client& dans les signatures de méthodes
 * sans avoir besoin de la définition complète de ces classes dans ce .hpp.
 *
 * En contrepartie, CommandHandler.cpp devra inclure Server.hpp et Client.hpp
 * pour avoir accès aux membres (server.getName(), client.nickname...).
 *
 * ┌─────────────────────────────────────────────────────────────────────┐
 * │  CONTRAT AVEC PERSONNE A :                                          │
 * │  Server et Client DOIVENT exposer les méthodes et membres listés    │
 * │  dans la section "INTERFACES AVEC PERSONNE A" ci-dessus.           │
 * │  C'est LE CONTRAT D'INTERFACE entre les rôles A et B.               │
 * │  À définir ensemble pendant la phase "squelette commun" (24-27/04). │
 * └─────────────────────────────────────────────────────────────────────┘
 */
class Server;
class Client;

class CommandHandler
{
public:

    /* -----------------------------------------------------------------------
     *  dispatch(server, client, msg)  —  POINT D'ENTRÉE UNIQUE
     * -----------------------------------------------------------------------
     *  RÔLE :
     *    Router le Message vers le bon handler selon msg.command.
     *    Vérifier les PRÉCONDITIONS COMMUNES avant de router (état client).
     *
     *  PARAMÈTRES :
     *    server : référence au serveur (accès aux clients, canaux, config)
     *    client : référence au client qui a envoyé la commande
     *    msg    : le Message parsé par Parser::parseLine()
     *
     *  Pourquoi des RÉFÉRENCES (Server &, Client &) et pas des pointeurs ?
     *  → En C++, les références expriment "cet objet existe et est valide".
     *    Pas besoin de vérifier != NULL avant chaque accès.
     *  → dispatch() est appelé par Personne A qui possède Server et Client.
     *    Les deux objets sont garantis valides au moment de l'appel.
     *  → C'est une convention C++ idiomatique : pointeur = "peut être NULL",
     *    référence = "toujours valide".
     *
     *  LOGIQUE DE DISPATCH :
     *
     *    1. Si msg.command est vide → ignorer (ligne vide)
     *    2. TOUJOURS autorisé (même non registered) :
     *         PASS, NICK, USER, PING, PONG, QUIT
     *         (un client doit pouvoir s'enregistrer et quitter)
     *    3. Si !client.registered → ERR_NOTREGISTERED et retour
     *    4. Commandes registered seulement :
     *         JOIN, PRIVMSG, KICK, INVITE, TOPIC, MODE
     *    5. Commande inconnue → ERR_UNKNOWNCOMMAND (421) ou ignorer
     *
     *  GESTION DES COMMANDES INCONNUES :
     *  Pour ft_irc, on peut ignorer les commandes inconnues (pas de crash).
     *  En bonus, on peut envoyer ERR_UNKNOWNCOMMAND (421).
     *  Les clients envoient parfois des commandes que notre serveur ne
     *  supporte pas (CAP, WHO, WHOIS, etc.) : les ignorer silencieusement
     *  est la stratégie la plus robuste.
     *
     *  APPEL DEPUIS PERSONNE A (Server.cpp) :
     *    std::vector<std::string> lines = Parser::extractLines(client.inBuffer);
     *    for (std::size_t i = 0; i < lines.size(); ++i) {
     *        Message msg = Parser::parseLine(lines[i]);
     *        if (!msg.command.empty())
     *            CommandHandler::dispatch(server, client, msg);
     *    }
     * ----------------------------------------------------------------------- */
    static void dispatch(Server &server, Client &client, const Message &msg);

private:

    /* =======================================================================
     *  HANDLERS DE PERSONNE B (Registration / Protocole de base)
     * ======================================================================= */

    /* -----------------------------------------------------------------------
     *  handlePass(server, client, msg)
     * -----------------------------------------------------------------------
     *  COMMANDE : PASS <password>
     *  RÔLE     : Vérifier le mot de passe du serveur.
     *
     *  RÈGLES RFC :
     *    - PASS DOIT être envoyé AVANT NICK et USER (recommandation RFC).
     *    - Une fois registered, PASS est refusé (ERR_ALREADYREGISTERED).
     *    - PASS peut être envoyé plusieurs fois avant registration (on
     *      prend le dernier si le serveur est clément, ou on rejette tout
     *      de suite si le mot de passe est mauvais).
     *
     *  COMPORTEMENT :
     *    - Si client.registered → ERR_ALREADYREGISTERED (462), retour.
     *    - Si msg.paramCount() < 1 → ERR_NEEDMOREPARAMS (461), retour.
     *    - Si param(0) != server.getPassword() → ERR_PASSWDMISMATCH (464),
     *      ERROR_MSG, client.shouldClose = true, retour.
     *    - Sinon → client.passOk = true, pas de réponse (comportement normal).
     *
     *  NOTE : On ne répond RIEN en cas de PASS correct. C'est la convention
     *  IRC : le silence signifie le succès pour PASS.
     *
     *  CAS PARTICULIER — PAS DE MOT DE PASSE :
     *  Si server.getPassword() == "" (aucun mot de passe configuré), que faire ?
     *  Option A : accepter PASS quelle que soit la valeur → client.passOk=true
     *  Option B : ignorer PASS complètement, passOk=true dès la connexion
     *  Le sujet impose TOUJOURS un mot de passe (./ircserv port password),
     *  donc ce cas ne devrait pas arriver. Mais pour la robustesse, l'option A
     *  est recommandée.
     * ----------------------------------------------------------------------- */
    static void handlePass(Server &server, Client &client, const Message &msg);

    /* -----------------------------------------------------------------------
     *  handleNick(server, client, msg)
     * -----------------------------------------------------------------------
     *  COMMANDE : NICK <nickname>
     *  RÔLE     : Définir ou changer le pseudo du client.
     *
     *  DEUX CAS D'USAGE :
     *  1. PENDANT LA REGISTRATION : nick pas encore défini
     *     → Valider le nick, l'affecter, vérifier si registration complète.
     *  2. APRÈS REGISTRATION : changement de pseudo
     *     → Valider, diffuser NICK_CHANGE aux canaux partagés, changer.
     *
     *  VALIDATIONS À EFFECTUER (dans l'ordre) :
     *
     *    a) Pas assez de paramètres → ERR_NONICKNAMEGIVEN (431)
     *
     *    b) Nick invalide → ERR_ERRONEUSNICKNAME (432)
     *       Règles de validité (voir isValidNick()) :
     *         - Longueur 1-9 caractères
     *         - Premier char : lettre ou [ ] \ { } | ^ ` _ -
     *         - Chars suivants : idem + chiffres
     *
     *    c) Nick déjà utilisé → ERR_NICKNAMEINUSE (433)
     *       ┌─────────────────────────────────────────────────────────────┐
     *       │  INTERFACE AVEC PERSONNE A :                                │
     *       │  On doit parcourir TOUS les clients connectés pour vérifier │
     *       │  l'unicité. server.findClientByNick(nick) est la méthode    │
     *       │  que Personne A doit fournir (ou on itère sur server.getClients()) │
     *       └─────────────────────────────────────────────────────────────┘
     *
     *    d) Si le nouveau nick == ancien nick → ignorer silencieusement.
     *
     *  APRÈS VALIDATION RÉUSSIE :
     *    Si client.registered (changement de pseudo en cours de session) :
     *      → Diffuser NICK_CHANGE à soi-même ET aux membres des canaux partagés
     *        ┌─────────────────────────────────────────────────────────────┐
     *        │  INTERFACE AVEC PERSONNE C :                                │
     *        │  Pour diffuser aux canaux : itérer sur client.channels      │
     *        │  (std::vector<std::string> ou std::set<std::string> défini  │
     *        │  dans Client.hpp par Personne A), récupérer chaque Channel  │
     *        │  via server.getChannels(), et envoyer NICK_CHANGE à tous    │
     *        │  les membres de chaque canal.                               │
     *        └─────────────────────────────────────────────────────────────┘
     *    Mettre à jour client.nickname.
     *    Si !client.registered → vérifier si registration complète
     *      (passOk && !nickname.empty() && !username.empty())
     *      → appeler registerClient() si oui.
     * ----------------------------------------------------------------------- */
    static void handleNick(Server &server, Client &client, const Message &msg);

    /* -----------------------------------------------------------------------
     *  handleUser(server, client, msg)
     * -----------------------------------------------------------------------
     *  COMMANDE : USER <username> <hostname> <servername> :<realname>
     *  RÔLE     : Définir l'identité utilisateur du client.
     *
     *  FORMAT RFC 1459 §4.1.3 :
     *    USER ali 0 * :Alice Martin
     *    param(0) = "ali"         → username
     *    param(1) = "0"           → mode (ignoré dans ft_irc)
     *    param(2) = "*"           → servername (ignoré dans ft_irc)
     *    param(3) = "Alice Martin" → realname (trailing)
     *
     *  VALIDATIONS :
     *    - Si client.registered → ERR_ALREADYREGISTERED (462)
     *      USER ne peut être envoyé qu'UNE SEULE FOIS avant registration.
     *    - Si paramCount() < 4 → ERR_NEEDMOREPARAMS (461)
     *      USER requiert EXACTEMENT 4 paramètres (username + 2 ignorés + realname).
     *
     *  APRÈS VALIDATION RÉUSSIE :
     *    client.username = msg.param(0)  → le username IRC
     *    client.realname = msg.param(3)  → le realname (souvent avec espaces)
     *    Vérifier si registration complète → appeler registerClient() si oui.
     *
     *  NOTE SUR LE USERNAME :
     *  Le username est ce qu'on stocke dans client.username. Il n'est pas
     *  modifiable après registration (contrairement au nick).
     *  Il apparaît dans le format "nick!user@host" de tous les messages.
     *
     *  NOTE SUR LE HOSTNAME :
     *  client.hostname est l'adresse IP ou le nom DNS du client, obtenu
     *  lors de accept() par Personne A (via getnameinfo ou inet_ntop).
     *  On ne l'extrait PAS de la commande USER (le client pourrait mentir).
     * ----------------------------------------------------------------------- */
    static void handleUser(Server &server, Client &client, const Message &msg);

    /* -----------------------------------------------------------------------
     *  handlePing(server, client, msg)
     * -----------------------------------------------------------------------
     *  COMMANDE : PING <token>
     *  RÔLE     : Répondre au PING du client pour maintenir la connexion.
     *
     *  COMPORTEMENT :
     *    Répondre avec PONG en reprenant exactement le token envoyé.
     *    TOUJOURS traiter PING, même avant registration.
     *
     *  IMPORTANT : Certains clients envoient PING comme TOUT PREMIER message
     *  (avant PASS, NICK, USER) pour tester si le serveur est vivant.
     *  Si on refuse PING avant registration, ces clients ne se connecteront
     *  jamais. → PING est toujours autorisé.
     *
     *  EXTRACTION DU TOKEN :
     *    Le token peut être dans msg.param(0) (paramètre normal) ou dans
     *    msg.trailing. Certains clients envoient "PING :token" (avec ':'),
     *    d'autres "PING token" (sans ':').
     *    → On essaie d'abord msg.param(0), puis msg.trailing.
     * ----------------------------------------------------------------------- */
    static void handlePing(Server &server, Client &client, const Message &msg);

    /* -----------------------------------------------------------------------
     *  handleQuit(server, client, msg)
     * -----------------------------------------------------------------------
     *  COMMANDE : QUIT [:<reason>]
     *  RÔLE     : Déconnecter proprement le client.
     *
     *  SÉQUENCE DE DÉCONNEXION PROPRE :
     *
     *    1. Récupérer la raison (msg.trailing si hasTrailing, sinon "Quit")
     *
     *    2. Diffuser QUIT_MSG à tous les membres des canaux partagés
     *       ┌─────────────────────────────────────────────────────────────┐
     *       │  INTERFACE AVEC PERSONNE C :                                │
     *       │  On doit retirer le client de tous ses canaux ET diffuser   │
     *       │  QUIT_MSG aux membres. C'est la responsabilité de          │
     *       │  Personne C qui connaît la structure Channel.               │
     *       │  handleQuit() appelle une méthode de Personne C :          │
     *       │    ChannelHandler::removeClientFromAllChannels(server, client, reason)  │
     *       │  ou équivalent.                                             │
     *       └─────────────────────────────────────────────────────────────┘
     *
     *    3. Envoyer ERROR_MSG au client lui-même
     *
     *    4. Marquer le client pour fermeture (client.shouldClose = true)
     *       ┌─────────────────────────────────────────────────────────────┐
     *       │  INTERFACE AVEC PERSONNE A :                                │
     *       │  On ne ferme PAS le fd ici. On donne à Personne A le signal │
     *       │  de fermeture différée via client.shouldClose.              │
     *       │  Personne A fermera le fd après avoir vidé outBuffer.       │
     *       └─────────────────────────────────────────────────────────────┘
     *
     *  QUIT vs déconnexion brutale :
     *  - QUIT : client se déconnecte volontairement avec la commande QUIT
     *  - Déconnexion brutale : recv()==0 ou POLLHUP (socket coupée)
     *  Les deux cas doivent déclencher la même séquence de nettoyage.
     *  Personne A appelle la même logique de nettoyage dans les deux cas.
     *  Pour la déconnexion brutale, la raison est "Connection reset" ou similaire.
     * ----------------------------------------------------------------------- */
    static void handleQuit(Server &server, Client &client, const Message &msg);

    /* -----------------------------------------------------------------------
     *  registerClient(server, client)  —  HELPER PRIVÉ
     * -----------------------------------------------------------------------
     *  RÔLE : Finaliser l'enregistrement quand PASS + NICK + USER sont OK.
     *  Appelé depuis handleNick() ET handleUser() quand la condition est remplie.
     *
     *  ACTIONS :
     *    1. client.registered = true
     *    2. Envoyer la séquence de bienvenue :
     *         RPL_WELCOME (001)
     *         RPL_YOURHOST (002)
     *         RPL_CREATED (003)
     *         RPL_MYINFO (004)
     *
     *  CONDITION DE DÉCLENCHEMENT :
     *    client.passOk == true
     *    && !client.nickname.empty()
     *    && !client.username.empty()
     *    && !client.registered  (ne pas ré-enregistrer)
     *
     *  PRIVÉ car appelé uniquement depuis handleNick() et handleUser().
     * ----------------------------------------------------------------------- */
    static void registerClient(Server &server, Client &client);

    /* -----------------------------------------------------------------------
     *  isValidNick(nick)  →  bool  —  HELPER PRIVÉ
     * -----------------------------------------------------------------------
     *  RÔLE : Valider la conformité d'un pseudo IRC selon la RFC 1459.
     *
     *  RÈGLES IMPLÉMENTÉES :
     *    1. Longueur : 1 à 9 caractères (IRC historic limit).
     *       Note : les implémentations modernes permettent jusqu'à 30 chars
     *       mais le sujet ne spécifie pas → on reste à 9 pour la conformité.
     *
     *    2. Premier caractère : isalpha(c) OU l'un de :
     *         '[' ']' '\' '{' '}' '|' '^' '`' '-' '_'
     *       (Ces caractères spéciaux sont historiquement utilisés pour les
     *       nicks en Europe : [] correspond à {} en ISO-8859-1, etc.)
     *
     *    3. Caractères suivants : idem + isdigit(c)
     *
     *  CARACTÈRES EXPLICITEMENT INTERDITS (même si pas dans la liste ci-dessus) :
     *    '#' '&' ':' '@' ' ' '!' '$' '.' ','
     *
     *  IMPLÉMENTATION :
     *    Boucle sur chaque caractère avec les tests adéquats.
     *    Retourne false dès qu'un caractère invalide est trouvé.
     *
     *  POURQUOI PRIVÉ ?
     *  C'est un détail d'implémentation interne à handleNick().
     *  Personne C n'a pas besoin de valider des nicks directement.
     * ----------------------------------------------------------------------- */
    static bool isValidNick(const std::string &nick);
};

#endif /* COMMANDHANDLER_HPP */
