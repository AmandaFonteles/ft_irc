/*
 * ===========================================================================
 *  CommandHandler.cpp   —   PERSONNE B
 * ===========================================================================
 *
 *  NOTE D'INTÉGRATION IMPORTANTE :
 *  Ce fichier utilise Server et Client. Ces deux classes sont définies par
 *  Personne A. Ce .cpp ne compilera pas sans leurs définitions.
 *
 *  CONTRAT MINIMAL QUE SERVER.HPP DOIT EXPOSER :
 *    class Server {
 *    public:
 *        const std::string& getName() const;
 *        const std::string& getPassword() const;
 *        Client*            findClientByNick(const std::string& nick);
 *        std::map<int, Client>& getClients();               // pour itération
 *        std::map<std::string, Channel>& getChannels();     // pour canaux
 *    };
 *
 *  CONTRAT MINIMAL QUE CLIENT.HPP DOIT EXPOSER :
 *    class Client {
 *    public:
 *        int         fd;
 *        std::string nickname;
 *        std::string username;
 *        std::string realname;
 *        std::string hostname;
 *        bool        passOk;
 *        bool        registered;
 *        bool        shouldClose;
 *        std::vector<std::string> channels;  // noms de canaux rejoints
 *        void enqueue(const std::string& msg);  // ajoute à outBuffer
 *    };
 *
 *  PLACEHOLDER POUR PERSONNE C :
 *  Les commandes JOIN, PRIVMSG, KICK, INVITE, TOPIC, MODE sont déclarées
 *  dans dispatch() mais leurs handlers sont à implémenter par Personne C.
 *  On utilise des stubs (fonctions vides) que Personne C remplacera.
 *
 * ===========================================================================
 */

#include "CommandHandler.hpp"
#include "Server.hpp"   /* Personne A — doit fournir ce fichier */
#include "Client.hpp"   /* Personne A — doit fournir ce fichier */
#include "Channel.hpp"  /* Personne C — doit fournir ce fichier */

#include <cctype>       /* std::isalpha, std::isdigit */
#include <iostream>     /* std::cerr pour le debug (à retirer avant rendu) */

/* ===========================================================================
 *  dispatch()   —   ROUTEUR CENTRAL
 * ===========================================================================
 *
 *  STRUCTURE DU ROUTEUR :
 *
 *  1. FILTRE PRÉ-REGISTRATION
 *     Certaines commandes sont autorisées avant registration (PASS, NICK,
 *     USER, PING, PONG, QUIT). Toutes les autres requièrent registration.
 *
 *  2. TABLE DE DISPATCH
 *     Une série de if/else sur msg.command. En C++98, pas de std::map
 *     vers des fonctions membres ou de switch sur std::string.
 *     → Série de if/else if avec comparaisons de chaînes.
 *     → En pratique, avec ~10 commandes, c'est O(n) mais n ≤ 10,
 *       donc parfaitement acceptable.
 *
 *  COMMANDES IGNORÉES SILENCIEUSEMENT :
 *  Les clients IRC modernes envoient des commandes que le sujet ne demande
 *  pas d'implémenter : CAP, WHO, WHOIS, AWAY, USERHOST, ISON, etc.
 *  On les ignore sans envoyer d'erreur pour ne pas perturber le client.
 *
 *  COMMANDES RECONNUES MAIS NON IMPLÉMENTÉES :
 *  Si on veut être propre, on peut envoyer ERR_UNKNOWNCOMMAND (421)
 *  pour les commandes vraiment inconnues. Pour ft_irc mandatory, ce
 *  n'est pas obligatoire.
 * =========================================================================== */
void CommandHandler::dispatch(Server &server, Client &client, const Message &msg)
{
    /*
     * GARDE-FOU : commande vide (ligne vide parsée).
     * parseLine("") retourne un Message avec command="".
     * On ne fait rien, pas d'erreur.
     */
    if (msg.command.empty())
        return;

    /* =======================================================================
     *  BLOC 1 : COMMANDES TOUJOURS AUTORISÉES (avant et après registration)
     *  Ces commandes doivent fonctionner dans tous les états du client.
     * ======================================================================= */

    if (msg.command == "PASS")
    {
        handlePass(server, client, msg);
        return;
    }

    if (msg.command == "NICK")
    {
        handleNick(server, client, msg);
        return;
    }

    if (msg.command == "USER")
    {
        handleUser(server, client, msg);
        return;
    }

    if (msg.command == "PING")
    {
        handlePing(server, client, msg);
        return;
    }

    if (msg.command == "PONG")
    {
        /*
         * Le client répond à un PING du serveur (keepalive).
         * Pour ft_irc, on ignore simplement le PONG reçu.
         * On pourrait mettre à jour un timestamp "last_pong" pour détecter
         * les connexions mortes, mais ce n'est pas requis par le sujet.
         */
        return;
    }

    if (msg.command == "QUIT")
    {
        handleQuit(server, client, msg);
        return;
    }

    /* =======================================================================
     *  BLOC 2 : GARDE DE REGISTRATION
     *  Toute commande suivante requiert que le client soit enregistré.
     * ======================================================================= */

    if (!client.registered)
    {
        /*
         * ERR_NOTREGISTERED (451) : "You have not registered"
         * Le client doit d'abord compléter PASS + NICK + USER.
         *
         * On utilise client.nickname comme targetNick, qui peut valoir ""
         * avant que NICK soit reçu. Dans ce cas, on affiche "*".
         * La RFC 1459 recommande "*" comme nick de substitution avant
         * que le nick soit défini.
         */
        const std::string &nick = client.nickname.empty() ? "*" : client.nickname;
        client.enqueue(Replies::ERR_NOTREGISTERED(server.getName(), nick));
        return;
    }

    /* =======================================================================
     *  BLOC 3 : COMMANDES POST-REGISTRATION (client.registered == true)
     * ======================================================================= */

    /*
     * COMMANDES DE PERSONNE C
     * ┌─────────────────────────────────────────────────────────────────┐
     * │  Ces appels seront complétés par Personne C.                    │
     * │  Pendant la phase de développement (24-27 avril), on laisse     │
     * │  des stubs vides pour que le code compile.                      │
     * │  Personne C REMPLACE ces stubs par ses implémentations réelles. │
     * └─────────────────────────────────────────────────────────────────┘
     */
    if (msg.command == "JOIN")
    {
        /* Personne C implémente handleJoin() */
        /* ChannelHandler::handleJoin(server, client, msg); */
        return;
    }

    if (msg.command == "PRIVMSG")
    {
        /* Personne C implémente handlePrivmsg() */
        /* ChannelHandler::handlePrivmsg(server, client, msg); */
        return;
    }

    if (msg.command == "KICK")
    {
        /* Personne C implémente handleKick() */
        /* ChannelHandler::handleKick(server, client, msg); */
        return;
    }

    if (msg.command == "INVITE")
    {
        /* Personne C implémente handleInvite() */
        /* ChannelHandler::handleInvite(server, client, msg); */
        return;
    }

    if (msg.command == "TOPIC")
    {
        /* Personne C implémente handleTopic() */
        /* ChannelHandler::handleTopic(server, client, msg); */
        return;
    }

    if (msg.command == "MODE")
    {
        /* Personne C implémente handleMode() */
        /* ChannelHandler::handleMode(server, client, msg); */
        return;
    }

    /*
     * COMMANDES IGNORÉES SILENCIEUSEMENT
     * ────────────────────────────────────
     * Ces commandes sont envoyées par les clients IRC modernes mais ne sont
     * pas demandées par le sujet ft_irc. On les ignore sans erreur pour
     * ne pas perturber la session du client.
     *
     * CAP    : négociation de capacités IRC (IRCv3)
     * WHO    : liste des utilisateurs correspondant à un masque
     * WHOIS  : informations détaillées sur un utilisateur
     * AWAY   : marquer comme absent
     * NAMES  : liste des membres d'un canal
     * LIST   : liste des canaux disponibles
     * MOTD   : Message Of The Day
     * LUSERS : statistiques du serveur
     */
    if (msg.command == "CAP"  || msg.command == "WHO"   ||
        msg.command == "WHOIS"|| msg.command == "AWAY"  ||
        msg.command == "NAMES"|| msg.command == "LIST"  ||
        msg.command == "MOTD" || msg.command == "LUSERS")
    {
        return;  /* Ignorer silencieusement */
    }

    /*
     * COMMANDE INCONNUE
     * ──────────────────
     * Pour être propre, on pourrait envoyer 421 ERR_UNKNOWNCOMMAND.
     * Pour ft_irc, on ignore silencieusement (évite les messages parasites
     * lors des tests avec des clients qui envoient des commandes inattendues).
     *
     * Version avec ERR_UNKNOWNCOMMAND (optionnel) :
     *   client.enqueue(Replies::ERR_UNKNOWNCOMMAND(server.getName(),
     *                                               client.nickname,
     *                                               msg.command));
     */
    (void)server; /* Évite le warning "unused parameter" si les blocs C sont vides */
}

/* ===========================================================================
 *  handlePass()
 * ===========================================================================
 *
 *  DÉTAIL DE LA LOGIQUE DE VALIDATION :
 *
 *  Cas 1 : client déjà registered → ERR_ALREADYREGISTERED (462)
 *  Cas 2 : pas de paramètre → ERR_NEEDMOREPARAMS (461)
 *  Cas 3 : mauvais mot de passe → ERR_PASSWDMISMATCH (464) + ERROR + shouldClose
 *  Cas 4 : bon mot de passe → client.passOk = true (pas de réponse)
 *
 * =========================================================================== */
void CommandHandler::handlePass(Server &server, Client &client, const Message &msg)
{
    const std::string &serverName = server.getName();
    const std::string &nick       = client.nickname.empty() ? "*" : client.nickname;

    /* Cas 1 : déjà enregistré */
    if (client.registered)
    {
        client.enqueue(Replies::ERR_ALREADYREGISTERED(serverName, nick));
        return;
    }

    /* Cas 2 : pas de paramètre */
    if (msg.paramCount() < 1)
    {
        client.enqueue(Replies::ERR_NEEDMOREPARAMS(serverName, nick, "PASS"));
        return;
    }

    /*
     * Récupérer le mot de passe fourni par le client.
     * msg.param(0) retourne "" si pas de paramètre, mais on a déjà vérifié
     * paramCount() >= 1 donc c'est sûr.
     *
     * Note : certains clients envoient le mot de passe en trailing
     * ("PASS :motdepasse"). msg.param(0) unifie les deux cas grâce à la
     * méthode param() de Message qui retourne le trailing si c'est l'index
     * correspondant.
     */
    const std::string provided = msg.param(0);

    /* Cas 3 : mauvais mot de passe */
    if (provided != server.getPassword())
    {
        client.enqueue(Replies::ERR_PASSWDMISMATCH(serverName, nick));
        /*
         * On envoie ERROR avant de fermer. Le client verra la raison
         * du refus dans son log avant la fermeture de la connexion.
         */
        client.enqueue(Replies::ERROR_MSG("Password incorrect"));
        /*
         * shouldClose = true : signal pour Personne A de fermer le fd
         * après avoir vidé outBuffer. NE PAS appeler close(fd) ici car
         * on est en train d'itérer sur les events poll() de Personne A.
         * Fermer le fd pendant poll() causerait un comportement indéfini.
         */
        client.shouldClose = true;
        return;
    }

    /* Cas 4 : mot de passe correct */
    client.passOk = true;
    /*
     * PAS DE RÉPONSE au client : le silence signifie le succès pour PASS.
     * C'est la convention IRC. Envoyer une réponse ici perturberait
     * certains clients qui ne s'y attendent pas.
     */
}

/* ===========================================================================
 *  handleNick()
 * ===========================================================================
 *
 *  CAS COMPLEXE : NICK pendant la registration VS NICK post-registration
 *
 *  PENDANT LA REGISTRATION :
 *    Le client n'a pas encore de nick. On le définit.
 *    Si NICK + USER + PASS sont maintenant tous OK → registerClient().
 *
 *  POST-REGISTRATION :
 *    Le client change son pseudo. On doit notifier tous les membres
 *    de ses canaux partagés.
 *
 * =========================================================================== */
void CommandHandler::handleNick(Server &server, Client &client, const Message &msg)
{
    const std::string &serverName = server.getName();
    const std::string &oldNick    = client.nickname.empty() ? "*" : client.nickname;

    /* VÉRIFICATION 1 : pas assez de paramètres */
    if (msg.paramCount() < 1)
    {
        client.enqueue(Replies::ERR_NONICKNAMEGIVEN(serverName, oldNick));
        return;
    }

    const std::string newNick = msg.param(0);

    /* VÉRIFICATION 2 : nick invalide */
    if (!isValidNick(newNick))
    {
        client.enqueue(Replies::ERR_ERRONEUSNICKNAME(serverName, oldNick, newNick));
        return;
    }

    /* VÉRIFICATION 3 : même nick que l'actuel (changement inutile) */
    if (newNick == client.nickname)
        return;  /* Ignorer silencieusement */

    /* VÉRIFICATION 4 : nick déjà utilisé par un autre client */
    /*
     * On itère sur tous les clients connectés pour vérifier l'unicité.
     *
     * ┌─────────────────────────────────────────────────────────────────┐
     * │  INTERFACE AVEC PERSONNE A :                                    │
     * │  server.getClients() retourne une map<int, Client> ou           │
     * │  équivalent contenant tous les clients connectés.               │
     * │  On itère avec un iterator en style C++98.                      │
     * │  Alternative : server.findClientByNick(newNick) retourne        │
     * │  un Client* (NULL si non trouvé).                               │
     * └─────────────────────────────────────────────────────────────────┘
     *
     * Implémentation avec findClientByNick() (plus propre) :
     */
    if (server.findClientByNick(newNick) != NULL)
    {
        /*
         * Un autre client utilise ce nick.
         * On envoie ERR_NICKNAMEINUSE avec le nick DEMANDÉ (newNick),
         * pas l'ancien nick du client, pour indiquer quel nick est pris.
         */
        client.enqueue(Replies::ERR_NICKNAMEINUSE(serverName, oldNick, newNick));
        return;
    }

    /* =======================================================================
     *  TOUTES LES VÉRIFICATIONS PASSÉES : le changement de nick est valide
     * ======================================================================= */

    if (client.registered)
    {
        /* ===================================================================
         *  CAS POST-REGISTRATION : diffuser NICK_CHANGE
         * ===================================================================
         *
         *  On notifie :
         *  1. Le client lui-même (son logiciel met à jour son nick affiché)
         *  2. Tous les membres de tous ses canaux partagés
         *
         *  ┌─────────────────────────────────────────────────────────────┐
         *  │  INTERFACE AVEC PERSONNE C :                                │
         *  │  client.channels contient les noms des canaux où est le     │
         *  │  client (std::vector<std::string> ou std::set<std::string>).│
         *  │  server.getChannels() retourne la map<string, Channel>.     │
         *  │  On itère pour envoyer NICK_CHANGE à tous les membres.      │
         *  │                                                             │
         *  │  ATTENTION : ne pas envoyer deux fois NICK_CHANGE au client │
         *  │  lui-même si il est dans un canal. On garde une trace des   │
         *  │  fds déjà notifiés avec un std::set<int>.                   │
         *  └─────────────────────────────────────────────────────────────┘
         */
        const std::string nickMsg = Replies::NICK_CHANGE(
            client.nickname, client.username, client.hostname, newNick
        );

        /* Ensemble des fds déjà notifiés pour éviter les doublons */
        /*
         * std::set<int> notified;
         *
         * Envoyer à soi-même :
         * notified.insert(client.fd);
         * client.enqueue(nickMsg);
         *
         * Envoyer aux membres de canaux partagés (Personne C intègre ici) :
         * for chaque canal dans client.channels :
         *     Channel &chan = server.getChannels()[canal];
         *     for chaque membre dans chan.members :
         *         if notified.find(membre.fd) == notified.end() :
         *             membre.enqueue(nickMsg);
         *             notified.insert(membre.fd);
         *
         * Version simplifiée pour l'instant (soi-même seulement) :
         */
        client.enqueue(nickMsg);

        /*
         * TODO (phase intégration avec Personne C) :
         * Ajouter la diffusion aux membres des canaux partagés.
         */
    }

    /* Mettre à jour le nick dans tous les cas */
    client.nickname = newNick;

    /* Vérifier si la registration est maintenant complète */
    if (!client.registered)
        registerClient(server, client);
}

/* ===========================================================================
 *  handleUser()
 * ===========================================================================
 *
 *  USER n'est envoyé qu'UNE SEULE FOIS pendant la registration.
 *  Après registration, toute tentative de re-USER est rejetée.
 *
 * =========================================================================== */
void CommandHandler::handleUser(Server &server, Client &client, const Message &msg)
{
    const std::string &serverName = server.getName();
    const std::string &nick       = client.nickname.empty() ? "*" : client.nickname;

    /* Vérification 1 : déjà enregistré */
    if (client.registered)
    {
        client.enqueue(Replies::ERR_ALREADYREGISTERED(serverName, nick));
        return;
    }

    /* Vérification 2 : pas assez de paramètres */
    if (msg.paramCount() < 4)
    {
        /*
         * USER requiert : username mode unused realname
         * param(0) = username    (ex: "ali")
         * param(1) = mode        (ex: "0", ignoré)
         * param(2) = unused      (ex: "*", ignoré, historiquement servername)
         * param(3) = realname    (trailing, ex: "Alice Martin")
         *
         * Si le realname est absent ou si un paramètre manque → 461.
         */
        client.enqueue(Replies::ERR_NEEDMOREPARAMS(serverName, nick, "USER"));
        return;
    }

    /*
     * Extraire username et realname.
     *
     * POURQUOI msg.param(0) ET msg.param(3) ?
     * La méthode param() unifie params[] et trailing.
     * "USER ali 0 * :Alice Martin" → param(0)="ali", param(3)="Alice Martin"
     * "USER ali 0 * Alice"        → param(0)="ali", param(3)="Alice"
     * Les deux formes sont gérées.
     */
    client.username = msg.param(0);
    client.realname = msg.param(3);

    /*
     * Vérifier si la registration est maintenant complète.
     * (passOk peut être déjà true si PASS a été reçu avant USER)
     */
    registerClient(server, client);
}

/* ===========================================================================
 *  handlePing()
 * ===========================================================================
 *
 *  TOUJOURS répondre à PING, même avant registration.
 *  Le token peut être dans param(0) OU dans trailing.
 *
 * =========================================================================== */
void CommandHandler::handlePing(Server &server, Client &client, const Message &msg)
{
    /*
     * RÉCUPÉRATION DU TOKEN :
     *
     * Cas 1 : "PING token" (paramètre normal, pas de ':')
     *   msg.param(0) = "token"   msg.trailing = ""   msg.hasTrailing = false
     *
     * Cas 2 : "PING :token" (trailing)
     *   msg.params = []   msg.trailing = "token"   msg.hasTrailing = true
     *   msg.param(0) retourne msg.trailing car params est vide et hasTrailing=true
     *
     * msg.param(0) unifie les deux cas grâce à la logique de Message::param().
     * Si les deux sont vides (PING sans argument), on répond PONG avec "".
     */
    const std::string token = msg.param(0);

    client.enqueue(Replies::PONG(server.getName(), token));
}

/* ===========================================================================
 *  handleQuit()
 * ===========================================================================
 *
 *  Déconnexion volontaire du client.
 *  La raison est optionnelle (trailing IRC).
 *
 * =========================================================================== */
void CommandHandler::handleQuit(Server &server, Client &client, const Message &msg)
{
    /*
     * Récupérer la raison. Par convention IRC, si pas de raison fournie,
     * on utilise "Quit" comme raison par défaut.
     *
     * msg.hasTrailing : true si le client a envoyé "QUIT :raison"
     *                   false si le client a envoyé juste "QUIT"
     */
    const std::string reason = msg.hasTrailing ? msg.trailing : "Quit";

    /*
     * ┌─────────────────────────────────────────────────────────────────┐
     * │  DIFFUSION AUX CANAUX (responsabilité Personne C) :             │
     * │                                                                 │
     * │  Personne C doit implémenter une fonction comme :               │
     * │    void removeClientFromAllChannels(Server &s, Client &c,       │
     * │                                     const std::string &reason)  │
     * │  qui :                                                          │
     * │    1. Pour chaque canal dans client.channels :                  │
     * │         - Envoyer QUIT_MSG à tous les autres membres            │
     * │         - Retirer le client du canal                            │
     * │         - Si le canal est vide → supprimer le canal             │
     * │    2. Vider client.channels                                     │
     * │                                                                 │
     * │  Intégration (à décommenter quand Personne C l'a implémenté) :  │
     * │    ChannelHandler::removeClientFromAllChannels(server, client, reason); │
     * └─────────────────────────────────────────────────────────────────┘
     */

    /* Envoyer ERROR_MSG au client avant fermeture */
    client.enqueue(Replies::ERROR_MSG("Quit: " + reason));

    /*
     * Marquer pour fermeture différée.
     * Personne A vérifie client.shouldClose après avoir vidé outBuffer
     * et ferme le fd proprement.
     */
    client.shouldClose = true;

    (void)server; /* TODO : retirer quand l'intégration avec Personne C est faite */
}

/* ===========================================================================
 *  registerClient()  —  Helper privé
 * ===========================================================================
 *
 *  Vérifie si les TROIS conditions de registration sont remplies :
 *    1. client.passOk      : PASS correct (ou pas de password requis)
 *    2. !nickname.empty()  : NICK défini
 *    3. !username.empty()  : USER reçu
 *
 *  Si oui ET si pas déjà registered : finalise la registration.
 *
 * =========================================================================== */
void CommandHandler::registerClient(Server &server, Client &client)
{
    /*
     * GARDE : ne pas ré-enregistrer un client déjà registered.
     * Cette méthode est appelée depuis handleNick() ET handleUser().
     * Le premier des deux qui arrive complète la registration.
     * Le second appel doit être ignoré.
     */
    if (client.registered)
        return;

    /*
     * VÉRIFICATION DES CONDITIONS DE REGISTRATION :
     *
     * passOk peut valoir false si :
     *   1. Le serveur a un mot de passe ET PASS n'a pas encore été envoyé
     *   2. Le serveur a un mot de passe ET PASS a été envoyé mais était incorrect
     *      (dans ce cas le client devrait déjà avoir shouldClose=true)
     *
     * On vérifie les 3 conditions. Si l'une manque, on attend le prochain
     * message du client (PASS, NICK ou USER selon ce qui manque).
     */
    if (!client.passOk || client.nickname.empty() || client.username.empty())
        return;  /* Registration incomplète, on attend */

    /* ═══════════════════════════════════════════════════════════════════
     *  REGISTRATION COMPLÈTE
     * ═══════════════════════════════════════════════════════════════════ */

    client.registered = true;

    const std::string &name = server.getName();
    const std::string &nick = client.nickname;
    const std::string &user = client.username;
    const std::string &host = client.hostname;

    /*
     * SÉQUENCE DE BIENVENUE 001-004
     * ──────────────────────────────
     * Ces 4 messages doivent être envoyés dans cet ordre.
     *
     * Pourquoi 4 messages et pas juste 001 ?
     *   001 = OBLIGATOIRE : déclenche l'affichage de la fenêtre principale
     *                        du client IRC
     *   002 = Attendu par irssi et weechat
     *   003 = Attendu par la plupart des clients
     *   004 = Permet aux clients d'adapter leur interface aux modes supportés
     *
     * Tous sont mis en file dans outBuffer par enqueue().
     * Personne A les enverra en un ou plusieurs paquets TCP lors du prochain
     * poll(POLLOUT) sur le fd du client.
     */
    client.enqueue(Replies::RPL_WELCOME(name, nick, user, host));
    client.enqueue(Replies::RPL_YOURHOST(name, nick));
    client.enqueue(Replies::RPL_CREATED(name, nick));
    client.enqueue(Replies::RPL_MYINFO(name, nick));

    /*
     * DEBUG (à retirer avant le rendu final) :
     * Afficher dans la console serveur qu'un nouveau client est enregistré.
     */
    std::cerr << "[DEBUG] Client registered: " << nick
              << "!" << user << "@" << host << std::endl;
}

/* ===========================================================================
 *  isValidNick()  —  Helper privé
 * ===========================================================================
 *
 *  LOGIQUE DE VALIDATION PAS À PAS :
 *
 *  RFC 1459 §2.3.1 :
 *    nickname   ::= ( letter / special ) *8( letter / digit / special / '-' )
 *    special    ::= '[' | ']' | '\' | '^' | '{' | '}' | '|' | '-' | '_' | '`'
 *
 *  En pratique, on simplifie légèrement :
 *    - Premier char : isalpha OU dans SPECIAL_CHARS
 *    - Chars suivants : isalpha OU isdigit OU dans SPECIAL_CHARS
 *    - Longueur : 1 à 9
 *
 * =========================================================================== */
bool CommandHandler::isValidNick(const std::string &nick)
{
    /* Vérification de longueur */
    if (nick.empty() || nick.size() > 9)
        return false;

    /*
     * Caractères spéciaux autorisés dans un nick (en plus des lettres/chiffres).
     * Cette chaîne sert de "liste blanche" de caractères spéciaux.
     * std::string::find() retourne npos si le caractère n'est pas trouvé.
     */
    const std::string SPECIAL = "[]\\^{}|`-_";

    /* Vérification du premier caractère */
    {
        char c = nick[0];
        /*
         * isalpha() : vérifie si c est une lettre (A-Z, a-z).
         * SPECIAL.find(c) != npos : vérifie si c est dans la liste spéciale.
         * Le premier caractère DOIT être une lettre ou un char spécial.
         * Les CHIFFRES sont INTERDITS en première position.
         */
        bool isLetter  = (std::isalpha(static_cast<unsigned char>(c)) != 0);
        bool isSpecial = (SPECIAL.find(c) != std::string::npos);

        if (!isLetter && !isSpecial)
            return false;
    }

    /* Vérification des caractères suivants */
    for (std::string::size_type i = 1; i < nick.size(); ++i)
    {
        char c = nick[i];

        /*
         * Les caractères suivants peuvent être :
         *   - Lettres (isalpha)
         *   - Chiffres (isdigit)  ← autorisés à partir du 2e caractère
         *   - Caractères spéciaux (SPECIAL)
         *   - '-' est déjà dans SPECIAL
         */
        bool isLetter  = (std::isalpha(static_cast<unsigned char>(c)) != 0);
        bool isDigit   = (std::isdigit(static_cast<unsigned char>(c)) != 0);
        bool isSpecial = (SPECIAL.find(c) != std::string::npos);

        if (!isLetter && !isDigit && !isSpecial)
            return false;
    }

    return true;
}
