+++
title = "Communication"
summary = "Comment le serveur et le client TCP échangent des payloads, comment le protocole EETP les chiffre, et comment les processus communiquent par pipes et par la mémoire partagée à slots."
keywords = "réseau ipc tcp payload séparateur epoll listen join eetp syn so aes gcm rsa handshake processus pipe mémoire partagée slot ownership futex target"
+++

Deux façons pour des programmes de communiquer : le **réseau** (un `Server` et un `Client` TCP, chiffrés en option par EETP) et les **processus** d'une même machine (pipes, un processus enfant, une mémoire partagée). Les classes sont sur [Network](page:network), [SManip](page:smanip#eetp) et [Encapsulation](page:encapsulation).

## Payloads sur TCP {#payloads}

Un `Server` et un `Client` échangent des **payloads** (des chaînes terminées par un séparateur, `'\n'` par défaut) à travers un objet socket. `listen()` lit ce qui est disponible sans bloquer, `join()` attend le prochain événement epoll ; `stop()` / `kill()` peuvent être appelés depuis un autre thread. Sous Windows, les sockets utilisent Winsock 2 et l'epoll est émulé sur `WSAPoll`.

:::html
<figure>
<svg viewBox="0 0 760 210" role="img" aria-label="Server and Client use an ISocket, the EETPParser frames the payloads">
<defs>
<marker id="arr2-fig4-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
<marker id="arr2a-fig4-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow-accent" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="box" x="30" y="30" width="180" height="50" rx="8"/>
<text class="lbl" x="120" y="60" text-anchor="middle">Client</text>
<rect class="box" x="550" y="30" width="180" height="50" rx="8"/>
<text class="lbl" x="640" y="60" text-anchor="middle">Server</text>
<rect class="box-alt" x="30" y="130" width="180" height="50" rx="8"/>
<text class="lbl" x="120" y="160" text-anchor="middle">TCPSocket</text>
<rect class="box-alt" x="550" y="130" width="180" height="50" rx="8"/>
<text class="lbl" x="640" y="160" text-anchor="middle">TCPSocket + epoll</text>
<path class="edge" d="M120 80V126" marker-end="url(#arr2-fig4-fr)"/>
<path class="edge" d="M640 80V126" marker-end="url(#arr2-fig4-fr)"/>
<path class="edge-accent" d="M210 155H546" marker-start="url(#arr2a-fig4-fr)" marker-end="url(#arr2a-fig4-fr)"/>
<text class="small" x="378" y="146" text-anchor="middle">payload '\n' payload '\n' ...</text>
<text class="small" x="378" y="176" text-anchor="middle">TCP</text>
<path class="edge dash" d="M210 55H546" marker-end="url(#arr2-fig4-fr)"/>
<text class="small" x="378" y="47" text-anchor="middle">EETPParser::format / parse</text>
</svg>
<figcaption>Les couches réseau ; le parser EETP est optionnel, au-dessus des payloads.</figcaption>
</figure>
:::

Le socket découpe le flux en payloads et garde ce qui n'est pas encore complet : un payload plus long que la limite de débordement (`4048` caractères sans séparateur) est une erreur, un pair qui ferme la connexion donne l'exception `None` `SocketClosed` et le statut `Down`.

## EETP {#eetp}

`EETPParser` garde les clés de chaque pair : le client envoie `SYN` (sa clé publique RSA, chiffrée avec la clé RSA commune de `~/.ssh/common`), le serveur répond `SO` (une nouvelle clé AES-256, chiffrée avec la clé du client), puis chaque message est chiffré en AES-256-GCM avec un nouvel iv aléatoire.

| Type | Sens | Contenu |
|---|---|---|
| `SYN` | client → serveur | clé publique RSA du client (RSA commune) |
| `SO` | serveur → client | clé AES-256 (RSA du client) |
| autre | ↔ | AES-256-GCM, trame `<iv><tag> ETB <content>`, encodée en Base64 |
| `EM` | ↔ | déconnexion, jamais chiffrée |

> [!WARNING]
> Le tramage a changé en `v3.0.0` : les deux côtés doivent utiliser `v3.0.0` ou plus récent.

## Processus {#processes}

`Process` enveloppe `fork` / `execvp` / `waitpid` avec des pipes et des duplications de fd appliquées dans l'enfant ; `Poll` enveloppe epoll ; `SharedObject` charge un `.so`. Sous Windows : `CreateProcess`, `WSAPoll`, `LoadLibrary` (`.dll`) ; il n'y a pas de `fork`, donc `spawn()` seul est une `FatalException` (`UndefinedCall`) et seuls les fds standard peuvent être redirigés.

## Mémoire partagée {#shared-memory}

`SharedMemory` transmet des messages entre processus à travers une mémoire partagée POSIX découpée en **slots**. Sous Windows, c'est un file mapping avec un événement nommé.

:::html
<figure>
<svg viewBox="0 0 760 200" role="img" aria-label="Shared memory with a metadata header and slots read by reader threads">
<defs>
<marker id="arr3-fig5-fr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
<path class="arrow" d="M0 0L10 5L0 10z"/>
</marker>
</defs>
<rect class="group" x="170" y="20" width="420" height="160" rx="10"/>
<text class="title" x="186" y="44">/name (shm)</text>
<rect class="box-alt" x="186" y="56" width="388" height="30" rx="6"/>
<text class="small" x="380" y="76" text-anchor="middle">ShmMetadata: size, queue, connected, readable (futex), sequence</text>
<rect class="box" x="186" y="100" width="90" height="60" rx="6"/>
<text class="small" x="231" y="126" text-anchor="middle">slot 0</text>
<text class="small" x="231" y="146" text-anchor="middle">flag · target</text>
<rect class="box" x="285" y="100" width="90" height="60" rx="6"/>
<text class="small" x="330" y="134" text-anchor="middle">slot 1</text>
<rect class="box" x="384" y="100" width="90" height="60" rx="6"/>
<text class="small" x="429" y="134" text-anchor="middle">slot ...</text>
<rect class="box" x="483" y="100" width="90" height="60" rx="6"/>
<text class="small" x="528" y="134" text-anchor="middle">slot n-1</text>
<rect class="box" x="20" y="100" width="120" height="60" rx="8"/>
<text class="lbl" x="80" y="126" text-anchor="middle">process A</text>
<text class="small" x="80" y="146" text-anchor="middle">send(bytes, target)</text>
<rect class="box" x="620" y="100" width="120" height="60" rx="8"/>
<text class="lbl" x="680" y="126" text-anchor="middle">process B</text>
<text class="small" x="680" y="146" text-anchor="middle">reader thread</text>
<path class="edge" d="M140 130H182" marker-end="url(#arr3-fig5-fr)"/>
<path class="edge" d="M574 130H616" marker-end="url(#arr3-fig5-fr)"/>
</svg>
<figcaption>Un écrivain réserve un slot libre (flag 0 → 1), écrit, le publie (flag 2) et réveille les lecteurs ; le thread lecteur de chaque instance copie les messages qui lui sont adressés dans un stockage local lu avec <code>read()</code>.</figcaption>
</figure>
:::

Un message est adressé à un lecteur (`Target(limit, ownership)`) ou à tous les lecteurs (`Target(limit)`, `0` = tous ceux qui sont connectés). Un `Id{id, ownership}` relie les réponses d'une conversation, `join()` attend un message et rend la main quand la mémoire est fermée. La **disposition** des slots dépend des écrivains : `Compact` pour un seul écrivain, `Interleaved*` pour plusieurs écrivains en même temps (une ligne de cache par slot pour `InterleavedAligned`).
