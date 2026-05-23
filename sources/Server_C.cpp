/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_C.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dnayel <dnayel@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 21:54:23 by afontele          #+#    #+#             */
/*   Updated: 2026/05/23 09:25:15 by dnayel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server_C.hpp"

/* ===========================================================================
 *  VARIABLES STATIQUES
 * ===========================================================================
 * _running est défini ici (hors de la classe) parce que les membres statiques
 * doivent être définis dans exactement un fichier .cpp. Le header déclare
 * l'existence (déclaration), ici on lui alloue réellement de la mémoire
 * (définition). Valeur initiale : true → le serveur tourne jusqu'au signal.
 * =========================================================================== */
bool Server::_running = true;

/* ===========================================================================
 *  CONSTRUCTEUR
 * ===========================================================================
 *
 * On convertit le port depuis une chaîne (argv[1]) vers un entier via
 * stringstream, ce qui permet de détecter les erreurs de format et les
 * caractères non-numériques qui resteraient dans le flux (eof non atteint).
 *
 * La RFC 1459 ne précise pas de plage de port obligatoire, mais l'usage
 * conventionnel IRC est 6667 (non-TLS). Les ports 1-1023 sont réservés
 * (well-known ports, IANA) et nécessitent les droits root sur Linux, d'où
 * la vérification portNb >= 1024. La borne supérieure 65535 correspond à
 * la valeur maximale d'un unsigned short (2^16 - 1).
 * =========================================================================== */
Server::Server(const std::string &port, const std::string &password)
    : _password(password), _serverSocket(-1)
{
    std::stringstream   extractInt(port);
    int                 portNb = 0;

    extractInt >> portNb;

    if (extractInt.fail() || !extractInt.eof())
        throw std::invalid_argument("Invalid port format.");
    if (portNb < 1024 || portNb > 65535)
        throw std::invalid_argument("Invalid port number.");

    _port = static_cast<unsigned short>(portNb);
}

/* ===========================================================================
 *  DESTRUCTEUR
 * ===========================================================================
 *
 * Ordre de nettoyage : socket serveur, puis chaque socket client (close + delete),
 * puis chaque canal (delete). Il faut fermer les fds avant de libérer la
 * mémoire pour éviter les fuites de descripteurs. Les clear() finaux sont
 * optionnels (le destructeur des conteneurs les vide) mais explicitent l'intention.
 * =========================================================================== */
Server::~Server()
{
    close(_serverSocket);

    for (std::map<int, Client *>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    for (std::map<std::string, Channel *>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
        delete it->second;

    _clients.clear();
    _channels.clear();

    std::cout << "[INFO] Server shutdown cleanly." << std::endl;
}

/* ===========================================================================
 *  signalHandler()
 * ===========================================================================
 *
 * Gestionnaire de signaux SIGINT (Ctrl+C), SIGQUIT (Ctrl+\) et SIGTERM.
 * Positionner _running = false suffit pour arrêter la boucle ServerRun()
 * proprement à la prochaine itération, sans interrompre un traitement en cours.
 *
 * write() est utilisé à la place de std::cout car write() est "async-signal-safe"
 * selon POSIX (c'est-à-dire qu'il peut être appelé depuis un gestionnaire de
 * signal sans risque d'état indéfini). printf(), std::cout et même fprintf()
 * NE sont PAS async-signal-safe car ils utilisent des buffers internes.
 *
 * Le param sig est marqué void pour éviter l'avertissement "unused parameter"
 * du compilateur.
 * =========================================================================== */
void Server::signalHandler(int sig)
{
    (void)sig;
    _running = false;
    write(STDOUT_FILENO, "\b\b[INFO] Signal received. Shutting down...\n", 43);
}

/* ===========================================================================
 *  ServerInit()
 * ===========================================================================
 *
 * ÉTAPE 1 — socket()
 *   AF_INET  : adresses IPv4 (Internet Protocol version 4).
 *   SOCK_STREAM : connexion TCP séquentielle et fiable (vs SOCK_DGRAM pour UDP).
 *   IPPROTO_TCP : protocole TCP explicitement spécifié (cohérence).
 *
 *   POURQUOI NE PAS UTILISER SOCK_NONBLOCK ici ?
 *   SOCK_NONBLOCK est une extension Linux non POSIX, absente de C++98 standard.
 *   Le sujet autorise UNIQUEMENT fcntl(fd, F_SETFL, O_NONBLOCK).
 *   On crée donc le socket en mode bloquant par défaut, puis on le passe en
 *   non-bloquant avec fcntl() à l'étape 3.
 *
 * ÉTAPE 2 — setsockopt(SO_REUSEADDR)
 *   Sans SO_REUSEADDR, si le serveur est arrêté et relancé immédiatement, le
 *   bind() échoue avec "Address already in use". Cela vient de l'état TIME_WAIT
 *   du protocole TCP : le noyau garde le port réservé quelques minutes (2*MSL,
 *   en général 60-120 secondes) pour absorber les paquets TCP tardifs.
 *   SO_REUSEADDR dit au noyau de réutiliser le port même en TIME_WAIT.
 *
 * ÉTAPE 3 — fcntl(F_SETFL, O_NONBLOCK)
 *   Met le socket serveur en mode non-bloquant. Ainsi, si poll() détecte un
 *   POLLIN sur le socket serveur mais qu'aucune connexion n'est en attente
 *   (race condition rare), accept() retourne -1 avec errno=EAGAIN au lieu de
 *   bloquer indéfiniment le processus.
 *
 * ÉTAPE 4 — bind()
 *   Associe le socket à l'adresse IP et au port.
 *   INADDR_ANY (0.0.0.0) signifie "écouter sur toutes les interfaces réseau"
 *   (loopback 127.0.0.1, réseau local 192.168.x.x, etc.).
 *   htonl() et htons() convertissent en Network Byte Order (big-endian),
 *   car le réseau utilise big-endian (MSB en premier) et les PC x86 sont
 *   little-endian. Sans cette conversion, le port serait inversé.
 *
 * ÉTAPE 5 — listen()
 *   Marque le socket comme passif (en attente de connexions).
 *   SOMAXCONN est la taille maximale de la file d'attente de connexions en
 *   attente d'accept() (typiquement 128 sur Linux).
 *
 * ÉTAPE 6 — Premier pollfd
 *   On ajoute le socket serveur au vecteur _pollFds avec uniquement POLLIN.
 *   Quand poll() signale POLLIN sur ce fd, un nouveau client cherche à se
 *   connecter → on appelle acceptNewClient().
 * =========================================================================== */
bool Server::ServerInit()
{
    /* Étape 1 : créer le socket TCP */
    _serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_serverSocket < 0)
    {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return false;
    }

    /* Étape 2 : autoriser la réutilisation du port après un arrêt rapide */
    int enable = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR,
                   &enable, sizeof(enable)) < 0)
    {
        close(_serverSocket);
        std::cerr << "Error: setsockopt(SO_REUSEADDR) failed." << std::endl;
        return false;
    }

    /*
     * Étape 3 : rendre le socket serveur non-bloquant.
     * Le sujet précise : "You can use fcntl(). Your server must not use fcntl
     * in any other way than fcntl(fd, F_SETFL, O_NONBLOCK)."
     */
    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        close(_serverSocket);
        std::cerr << "Error: fcntl(O_NONBLOCK) failed on server socket." << std::endl;
        return false;
    }

    /* Étape 4 : lier le socket à l'adresse et au port */
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port        = htons(_port);

    if (bind(_serverSocket, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        close(_serverSocket);
        std::cerr << "Error: bind() failed." << std::endl;
        return false;
    }

    /* Étape 5 : démarrer l'écoute */
    if (listen(_serverSocket, SOMAXCONN) < 0)
    {
        close(_serverSocket);
        std::cerr << "Error: listen() failed." << std::endl;
        return false;
    }

    /* Étape 6 : enregistrer le socket serveur dans la liste poll() */
    struct pollfd serverpfd;
    serverpfd.fd      = _serverSocket;
    serverpfd.events  = POLLIN;
    serverpfd.revents = 0;
    _pollFds.push_back(serverpfd);

    std::cout << "[INFO] Server listening on port " << _port << std::endl;
    return true;
}

/* ===========================================================================
 *  ServerRun()  —  BOUCLE PRINCIPALE I/O
 * ===========================================================================
 *
 * Ce pattern "boucle infinie + poll()" s'appelle un "event loop" ou
 * "reactor pattern". C'est l'architecture imposée par le sujet.
 *
 * POURQUOI poll() ET PAS select() OU epoll() ?
 *   select() souffre de la limite FD_SETSIZE (1024 fds sur Linux) et demande
 *   de reconstruire le fd_set à chaque appel. epoll() est plus performant mais
 *   Linux uniquement. poll() est le bon compromis : portable, pas de limite
 *   fixe de fds, API simple.
 *
 * POURQUOI TIMEOUT -1 ?
 *   Timeout -1 = bloquer indéfiniment jusqu'à un événement. Le processus ne
 *   consomme aucun CPU pendant l'attente. Un timeout de 0 ferait du busy-wait.
 *
 * ERRNO == EINTR :
 *   Un signal (Ctrl+C) peut interrompre poll() et lui faire retourner -1 avec
 *   errno=EINTR. Ce n'est pas une erreur fatale : on continue la boucle. Le
 *   gestionnaire de signal a positionné _running = false, donc la condition
 *   while se réévalue et on sort proprement.
 *
 * ITÉRATION INVERSE (i de size-1 à 0) :
 *   Quand cleanClosure() efface un élément de _pollFds, les indices des
 *   éléments suivants changent (décalage vers la gauche). Itérer en sens
 *   inverse garantit que les indices déjà traités ne sont pas affectés par
 *   les suppressions d'éléments à des indices supérieurs.
 *
 * RÉVENTS (bitmask) :
 *   revents & POLLIN  : des données sont disponibles en lecture (recv()).
 *   revents & POLLOUT : le buffer d'envoi du noyau a de la place (send()).
 *   On utilise & (AND bit à bit) et pas == car plusieurs drapeaux peuvent
 *   être positionnés simultanément dans le même revents.
 * =========================================================================== */
void Server::ServerRun()
{
    while (_running)
    {
        int eventCount = poll(&_pollFds[0], _pollFds.size(), -1);

        if (eventCount < 0)
        {
            if (errno == EINTR) /* signal reçu pendant poll() → pas une erreur */
                continue;
            std::cerr << "Error: poll() failed." << std::endl;
            break;
        }

        for (int i = static_cast<int>(_pollFds.size() - 1); i >= 0; i--)
        {
            if (_pollFds[i].revents & POLLIN)
            {
                if (_pollFds[i].fd == _serverSocket)
                    acceptNewClient();
                else
                    receiveClientData(_pollFds[i].fd);
            }
            if (_pollFds[i].revents & POLLOUT)
                sendMessage(_pollFds[i].fd);
        }
    }
}

/* ===========================================================================
 *  acceptNewClient()
 * ===========================================================================
 *
 * accept() accepte la première connexion en attente dans la file. Elle remplit
 * clientAddr avec l'adresse IP et le port du client. Le fd retourné est un
 * NOUVEAU socket uniquement pour cette connexion (le socket serveur reste en
 * attente d'autres connexions).
 *
 * INET_ADDRSTRLEN = 16 : taille suffisante pour une adresse IPv4 au format
 * pointé ("255.255.255.255\0", soit 16 octets).
 * inet_ntop(AF_INET, ...) convertit l'adresse binaire stockée dans
 * clientAddr.sin_addr (struct in_addr, 4 octets) en chaîne lisible.
 * On stocke cette chaîne dans Client._hostname via set_hostname() pour
 * construire ensuite les préfixes IRC ":nick!user@hostname".
 *
 * POURQUOI fcntl(O_NONBLOCK) SUR LE SOCKET CLIENT ?
 * Le socket retourné par accept() hérite des propriétés du socket d'écoute,
 * SAUF le flag O_NONBLOCK en C POSIX. On doit donc le positionner
 * explicitement. Sans ça, recv() sur ce fd bloquerait le processus entier
 * si le client n'envoie pas de données.
 * =========================================================================== */
void Server::acceptNewClient()
{
    struct sockaddr_in  clientAddr;
    socklen_t           clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(_serverSocket,
                              (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket < 0)
    {
        /* EAGAIN peut arriver si un autre accept() a déjà traité la connexion */
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "Error: accept() failed." << std::endl;
        return;
    }

    /* Rendre le socket client non-bloquant */
    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
    {
        close(clientSocket);
        std::cerr << "Error: fcntl(O_NONBLOCK) failed on client socket." << std::endl;
        return;
    }

    Client *newClient = new Client(clientSocket);
    if (!newClient)
    {
        close(clientSocket);
        return;
    }

    /*
     * Extraire l'adresse IP et la stocker dans le client.
     * inet_ntop retourne NULL en cas d'erreur (très rare) ; dans ce cas,
     * _hostname conserve sa valeur par défaut "localhost" (initialisée
     * dans le constructeur Client).
     */
    char hostBuf[INET_ADDRSTRLEN];
    std::memset(hostBuf, 0, sizeof(hostBuf));
    if (inet_ntop(AF_INET, &clientAddr.sin_addr, hostBuf, sizeof(hostBuf)))
        newClient->set_hostname(std::string(hostBuf));

    _clients[clientSocket] = newClient;

    struct pollfd clientpfd;
    clientpfd.fd      = clientSocket;
    clientpfd.events  = POLLIN;
    clientpfd.revents = 0;
    _pollFds.push_back(clientpfd);

    std::cout << "[INFO] New client fd=" << clientSocket
              << " from " << newClient->get_hostname() << std::endl;
}

/* ===========================================================================
 *  receiveClientData()  —  PIPELINE IRC COMPLET
 * ===========================================================================
 *
 * C'est ici que les données brutes TCP deviennent des commandes IRC traitées.
 *
 * PIPELINE :
 *   recv()            → octets bruts dans buff
 *   set_bufferIn()    → accumulation dans _bufferIn du Client (+=)
 *   extractLines()    → extraction des lignes complètes (\n trouvé),
 *                       stripping du \r final, limite à 510 octets de contenu.
 *                       Modifie _bufferIn en retirant les lignes extraites.
 *   parseLine()       → analyse grammaticale : prefix, command, params, trailing
 *   handleCommand()   → routage et exécution de la commande
 *
 * POURQUOI ACCUMULER DANS bufferIn ?
 * TCP est un protocole de flux (stream). Un seul recv() peut recevoir :
 *   - une seule commande complète  : "NICK alice\r\n"
 *   - plusieurs commandes          : "NICK alice\r\nUSER ali 0 * :Ali\r\n"
 *   - un début de commande         : "NI" (suite dans le prochain recv())
 * Sans accumulation, les commandes fragmentées seraient perdues.
 *
 * RFC 1459 §2.3 : "IRC messages are always lines of characters terminated
 * with a CR-LF pair. These messages shall not exceed 512 characters in
 * length, counting all characters including the trailing CR-LF."
 *
 * GESTION DE EAGAIN/EWOULDBLOCK :
 * Puisque le socket est non-bloquant (fcntl O_NONBLOCK), recv() retourne
 * immédiatement -1 avec errno=EAGAIN s'il n'y a pas de données disponibles.
 * Ce n'est PAS une erreur : cela signifie simplement "rien à lire pour l'instant".
 * On return silencieusement. AVANT cette correction, le code tombait dans
 * cleanClosure() ce qui déconnectait le client sans raison.
 *
 * bytesRead == 0 :
 * Le client a fermé proprement sa connexion (envoyé FIN TCP). On ferme
 * notre côté avec cleanClosure().
 *
 * shouldClose après le dispatch :
 * handleQUIT() et handlePASS() (mauvais mot de passe) positionnent
 * client->_shouldClose = true après avoir mis un message dans bufferOut.
 * On arrête de traiter les commandes suivantes (le client part) mais on
 * ne ferme pas le fd ici : sendMessage() fermera après avoir vidé bufferOut,
 * garantissant que le message ERROR atteint le client avant la coupure.
 * =========================================================================== */
void Server::receiveClientData(int clientFd)
{
    char    buff[1024];
    std::memset(buff, 0, sizeof(buff));

    ssize_t bytesRead = recv(clientFd, buff, sizeof(buff) - 1, 0);

    if (bytesRead < 0)
    {
        /*
         * EAGAIN/EWOULDBLOCK = pas de données disponibles en ce moment.
         * Ce n'est pas une erreur avec un socket non-bloquant.
         */
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        std::cerr << "Error: recv() failed for fd=" << clientFd << std::endl;
        cleanClosure(clientFd);
        return;
    }

    if (bytesRead == 0)
    {
        /* Fermeture propre TCP : le client a envoyé FIN */
        cleanClosure(clientFd);
        return;
    }

    /* Vérification de cohérence : le client existe-t-il encore ? */
    if (_clients.find(clientFd) == _clients.end())
        return;

    Client *client = _clients[clientFd];

    /*
     * Accumuler les octets reçus dans le buffer d'entrée du client.
     * set_bufferIn() fait _bufferIn += str, ce qui permet de reconstituer
     * les commandes fragmentées sur plusieurs recv().
     * On passe std::string(buff, bytesRead) plutôt que juste buff pour
     * gérer correctement les éventuels octets nuls en milieu de buffer
     * (peu probable en IRC mais défensif).
     */
    client->set_bufferIn(std::string(buff, bytesRead));

    /*
     * Extraire toutes les lignes complètes du buffer d'entrée.
     * extractLines() retire de _bufferIn les lignes qu'elle retourne,
     * gardant les données partielles pour le prochain recv().
     * Elle gère aussi le strip de \r et la troncature à 510 octets.
     */
    std::vector<std::string>    lines   = Parser::extractLines(client->get_bufferIn());
    CommandHandler              handler;

    for (std::size_t i = 0; i < lines.size(); i++)
    {
        if (lines[i].empty())
            continue;

        Message msg = Parser::parseLine(lines[i]);

        if (!msg.command.empty())
            handler.handleCommand(*this, client, msg);

        /*
         * Vérifier shouldClose APRÈS chaque commande.
         * Si handleQUIT() ou handlePASS() (mauvais mdp) l'ont positionné,
         * on arrête immédiatement le dispatch. Les commandes suivantes
         * dans la même rafale sont ignorées : le client est en train de partir.
         */
        if (client->get_shouldClose())
            break;
    }
}

/* ===========================================================================
 *  cleanClosure()
 * ===========================================================================
 *
 * Fermeture immédiate d'un client (déconnexion réseau brutale, erreur, etc.).
 * Appelée aussi par sendMessage() quand shouldClose = true et bufferOut vide.
 *
 * ORDRE IMPORTANT :
 *   1. removeClientFromAllChannels() → retire le client de tous les canaux
 *      et diffuse QUIT_MSG aux autres membres (sans raison car déco brutale).
 *   2. removeAllChannel() → vide la liste _channels du côté Client.
 *   3. close(clientFd) → libère le fd au niveau du noyau.
 *   4. delete → libère la mémoire.
 *   5. Érasure de la map → le pointeur n'est plus référencé.
 *   6. Érasure du _pollFds → poll() ne surveille plus ce fd.
 *
 * Appeler removeAllChannel() après removeClientFromAllChannels() est correct
 * car removeClientFromAllChannels() supprime le client de Channel._members
 * AVANT que removeAllChannel() ne tente de le faire depuis Client._channels.
 * =========================================================================== */
void Server::cleanClosure(int clientFd)
{
    if (_clients.find(clientFd) == _clients.end())
        return;

    /* Diffusion QUIT_MSG sans raison (déco brutale = raison vide) */
    removeClientFromAllChannels(clientFd, "");

    _clients[clientFd]->removeAllChannel();

    close(clientFd);
    delete _clients[clientFd];
    _clients.erase(clientFd);

    for (std::size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == clientFd)
        {
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }

    std::cout << "[INFO] Client fd=" << clientFd << " disconnected." << std::endl;
}

/* ===========================================================================
 *  switchPollOut()
 * ===========================================================================
 *
 * Active l'événement POLLOUT pour un fd donné.
 *
 * POURQUOI CE MÉCANISME ?
 * On ne peut pas appeler send() directement dans les handlers IRC car :
 *   - send() peut être partiel (buffers TCP pleins) → nécessite une boucle
 *   - send() avec MSG_DONTWAIT peut échouer silencieusement
 *   - L'architecture event-loop demande que tout l'envoi passe par sendMessage()
 *
 * FLUX CORRECT :
 *   1. Un handler enqueue un message via client->set_bufferOut("...")
 *   2. server.switchPollOut(fd) active POLLOUT sur ce fd
 *   3. poll() signale POLLOUT à la prochaine itération
 *   4. sendMessage(fd) envoie le contenu du bufferOut
 * =========================================================================== */
void Server::switchPollOut(int clientFd)
{
    for (std::size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == clientFd)
        {
            _pollFds[i].events = POLLIN | POLLOUT;
            break;
        }
    }
}

/* ===========================================================================
 *  sendMessage()
 * ===========================================================================
 *
 * Envoie le contenu du bufferOut via send().
 *
 * ENVOI PARTIEL :
 * TCP garantit que send() envoie soit tous les octets demandés, soit moins
 * si le buffer interne du noyau est plein. En cas d'envoi partiel, on retire
 * les octets envoyés du buffer et on laisse POLLOUT actif pour que la boucle
 * principale rappelle sendMessage() et continue l'envoi.
 *
 * DÉSACTIVATION DE POLLOUT :
 * Une fois le buffer vidé, on repasse events à POLLIN seul. Sans ça, poll()
 * signalerait POLLOUT en continu (le buffer d'envoi du noyau est toujours
 * disponible) ce qui entraînerait une boucle à 100% CPU.
 *
 * VÉRIFICATION shouldClose :
 * Après avoir vidé completement le bufferOut, si shouldClose est true, le
 * client attend sa déconnexion (handleQUIT ou mauvais PASS). On appelle
 * cleanClosure() maintenant que le message ERROR a été envoyé.
 *
 * C'est cette vérification qui manquait précédemment : les clients qui
 * envoyaient QUIT ou un mauvais mot de passe recevaient ERROR mais restaient
 * connectés indéfiniment.
 * =========================================================================== */
void Server::sendMessage(int clientFd)
{
    if (_clients.find(clientFd) == _clients.end())
        return;

    std::string &msg = _clients[clientFd]->get_bufferOut();
    if (msg.empty())
        return;

    ssize_t bytesSent = send(clientFd, msg.c_str(), msg.length(), 0);

    if (bytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; /* buffer du noyau plein : réessayer à la prochaine itération */
        std::cerr << "Error: send() failed for fd=" << clientFd << std::endl;
        cleanClosure(clientFd);
        return;
    }

    if (bytesSent < static_cast<ssize_t>(msg.length()))
    {
        /* Envoi partiel : retirer les octets envoyés, garder POLLOUT actif */
        msg.erase(0, bytesSent);
        return;
    }

    /* Envoi complet */
    msg.clear();

    for (std::size_t i = 0; i < _pollFds.size(); i++)
    {
        if (_pollFds[i].fd == clientFd)
        {
            _pollFds[i].events = POLLIN;
            break;
        }
    }

    /*
     * FERMETURE DIFFÉRÉE :
     * Si shouldClose est true (positionné par handleQUIT ou handlePASS),
     * le bufferOut est maintenant vide → le message ERROR a été envoyé
     * → on peut fermer proprement la connexion.
     *
     * Ne pas appeler cleanClosure() avant cela : le client n'aurait jamais
     * reçu le message ERROR (close() avant send() = données perdues).
     */
    if (_clients[clientFd]->get_shouldClose())
        cleanClosure(clientFd);
}

/* ===========================================================================
 *  removeClientFromAllChannels()
 * ===========================================================================
 *
 * Retire un client de tous les canaux IRC, en diffusant un QUIT_MSG si une
 * raison est fournie (c'est-à-dire si c'est un QUIT volontaire).
 *
 * SÉQUENCE PAR CANAL :
 *   1. Si le client est membre du canal ET qu'une raison est fournie :
 *      envoyer QUIT_MSG à tous les AUTRES membres.
 *   2. Retirer le client des trois sets : membres, opérateurs, invités.
 *   3. Si le canal est vide après retrait : le supprimer (libération mémoire).
 *
 * POURQUOI SUPPRIMER LES CANAUX VIDES ?
 * La RFC 1459 ne l'impose pas explicitement mais c'est la convention standard :
 * un canal IRC n'existe que tant qu'il a au moins un membre. Garder des canaux
 * vides en mémoire serait une fuite de ressources.
 *
 * CONSTRUCTION DU QUIT_MSG :
 * modern.ircdocs.horse/quit : "Servers SHOULD prepend <reason> with the ASCII
 * string 'Quit: ' when sending QUIT messages to other clients."
 * La raison est déjà préfixée "Quit: " par handleQUIT() avant cet appel.
 *
 * QUAND reason = "" (déconnexion brutale via cleanClosure) :
 * On ne construit pas de quitMsg et on ne diffuse rien. Les autres membres
 * du canal verront juste le client disparaître sans message de départ.
 * C'est le comportement correct pour une perte de connexion TCP.
 * =========================================================================== */
void Server::removeClientFromAllChannels(int clientFd, const std::string &reason)
{
    if (_clients.find(clientFd) == _clients.end())
        return;

    Client     *client  = _clients[clientFd];
    std::string quitMsg;

    /*
     * Construire le QUIT_MSG UNE SEULE FOIS pour tous les canaux.
     * On ne le construit que si :
     *   - Le client est enregistré (a un nick!user@host valide)
     *   - Une raison est fournie (QUIT volontaire, pas déco brutale)
     */
    if (client->get_registered() && !reason.empty())
    {
        quitMsg = Replies::QUIT_MSG(
            client->get_nickname(),
            client->get_username(),
            client->get_hostname(),
            reason
        );
    }

    std::map<std::string, Channel *>::iterator it = _channels.begin();
    while (it != _channels.end())
    {
        Channel *chan = it->second;

        if (chan->isMember(client) && !quitMsg.empty())
        {
            /* Diffuser à tous les membres SAUF le client qui part */
            std::set<Client *>              members = chan->get_members();
            std::set<Client *>::iterator    mit     = members.begin();

            while (mit != members.end())
            {
                if (*mit != client)
                {
                    (*mit)->set_bufferOut(quitMsg);
                    switchPollOut((*mit)->get_socketFd());
                }
                ++mit;
            }
        }

        /* Retirer le client des trois listes du canal */
        chan->removeMember(client);
        chan->removeOperator(client);
        chan->removeInvite(client);

        if (chan->nbMembers() == 0)
        {
            delete chan;
            _channels.erase(it++);
        }
        else
            ++it;
    }
}
