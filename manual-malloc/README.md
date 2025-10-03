# Confronto tra `sbrk()` e `mmap()` per l'allocazione dinamica in C

## Analisi tecnica di `sbrk()` e `mmap()` per l’allocazione dinamica della memoria in C

### 📘 Introduzione

L’allocazione dinamica della memoria rappresenta uno degli aspetti fondamentali nella programmazione di sistema in C, specialmente in ambiente Linux o Unix-like. L’evoluzione delle strategie di gestione della memoria nei sistemi operativi riflette sia il progresso dell’hardware, sia la necessità di soddisfare requisiti di flessibilità, efficienza e sicurezza sempre crescenti.

In questo contesto, le funzioni `sbrk()` (oggi considerata deprecata) e `mmap()` hanno rappresentato — e rappresentano tuttora — due paradigmi radicalmente diversi per la gestione dello spazio di memoria dei processi.

In questo report analizzeremo in profondità i due approcci, a partire dalla struttura della memoria virtuale di un processo in ambiente Linux, illustrando il funzionamento e l’architettura di `sbrk()` e del concetto di breaking point, evidenziandone i limiti strutturali. Si mostrerà come `mmap()` abbia superato molte di queste limitazioni, offrendo flessibilità, sicurezza e granularità nell’allocazione e deallocazione della memoria — sia anonima che file-backed.

Il documento è pensato come materiale didattico per una repository GitHub e segue le migliori pratiche di chiarezza, precisione e rigore tecnico nel linguaggio e nella struttura.

---

## 🧠 Layout della memoria virtuale di un processo in C/Linux

Il modello di memoria virtuale in Linux prevede la suddivisione dello spazio di indirizzamento di ogni processo in segmenti distinti, ognuno con specifiche finalità semantiche e di accesso:

- **Segmento di testo (text segment)**: contiene il codice eseguibile del programma. È di sola lettura e può essere condiviso fra più processi che eseguono lo stesso binario, massimizzando l’efficienza della cache e la sicurezza (protezione contro sovrascritture accidentali o malevole).

- **Segmento dati (data segment)**: contiene variabili globali e statiche inizializzate. Questo segmento è r/w (read-write).

- **Segmento BSS (Block Started by Symbol)**: destinato alle variabili globali e statiche non inizializzate esplicitamente. Allocato e inizializzato a zero dal kernel, non occupa spazio nel file eseguibile.

- **Heap**: spazio dinamico che cresce verso gli indirizzi più alti, utilizzato per l’allocazione dinamica a runtime (tipicamente tramite `malloc()`, `calloc()`, `realloc()` e le primitive di basso livello come `sbrk()` e, nei sistemi moderni, `mmap()`).

- **No man’s land**: area vuota o di riserva fra heap e stack per ridurre il rischio di collisione fra i due segmenti.

- **Stack**: cresce verso indirizzi più bassi, memorizza dati temporanei (variabili locali, parametri di funzione, indirizzi di ritorno).

- **Argomenti da riga di comando**: allocati all’estremo superiore dello spazio virtuale.

La seguente illustrazione (semplificata) rappresenta tale struttura:

![Struttura della memoria virtuale](percorso/immagine.png)




Un esempio chiarificatore:

Supponiamo che si allochino in sequenza tre blocchi: A, B, C. Si liberano nell’ordine A, poi B, poi C:

Solo liberando C (ultimo) si può realmente “accorciare” l’heap con sbrk(negativo);

Finché B e C sopravvivono, lo spazio di A rimane inutilizzabile dall’OS, anche se può essere eventualmente riutilizzato da successive malloc().

Altre limitazioni
Oltre al limite LIFO:

Non è thread-safe: chiamate concorrenti possono produrre risultati imprevedibili.

Gestione conflittuale: se la memoria viene gestita sia da malloc che direttamente via sbrk da altre parti del codice, si possono corrompere le strutture dati interne del gestore della memoria (dangerous).

Limiti di sicurezza: nessun meccanismo di protezione degli accessi (es. scrittura accidentale, esecuzione di codice dove non consentito).

Portabilità: sbrk() e brk() non sono più parte della specifica POSIX da anni e sono considerate obsolete5.

Deprecazione e sicurezza
Le man page e la documentazione ufficiale scoraggiano l’uso di sbrk() e brk() negli sviluppi moderni:

“Avoid using brk() and sbrk(): the malloc(3) memory allocation package is the portable and comfortable way of allocating memory…”

L’uso diretto di queste primitive dovrebbe essere riservato solo a specifici contesti legacy, programmi embedded o test didattici. Nel resto dei casi è caldamente consigliato fare affidamento sulle funzioni della libreria standard (malloc, calloc, realloc, free) o — per allocazioni avanzate — sulle mappature fornite da mmap().

Introduzione a mmap() nell’allocazione della memoria
Cos’è mmap()
La system call mmap() consente di mappare direttamente porzioni di memoria virtuale del processo sia su file fisici che su pagine anonime non associate a file, con controllo granulare sulle protezioni di accesso, la visibilità (privata o condivisa), e la posizione nello spazio virtuale12.

La dichiarazione è la seguente:

c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
addr: indirizzo suggerito (o NULL perché scelga il kernel).

length: dimensione in byte.

prot: protezioni di accesso (PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE).

flags: comportamento della mappatura (MAP_PRIVATE, MAP_SHARED, MAP_ANONYMOUS, MAP_FIXED, ecc.).

fd: file descriptor (ignorato se MAP_ANONYMOUS).

offset: offset all’interno del file (deve essere multiplo della dimensione di pagina).

Campi fondamentali: protezioni e flag
Protezioni (PROT)
PROT_READ: accesso in lettura consentito.

PROT_WRITE: accesso in scrittura consentito.

PROT_EXEC: esecuzione di codice consentita (es. per pagine di codice).

PROT_NONE: nessun accesso consentito (utile per pagine di guardia).

Questa granularità permette, ad esempio, di implementare stack o heap non eseguibili (NX), proteggendo da errori o attacchi di sicurezza.

Flag (MAP)
MAP_ANONYMOUS (o MAP_ANON): la memoria non è associata a file; i contenuti sono zero-initialized.

MAP_SHARED: le scritture sulla mappatura sono riflesse nel file sottostante e visibili a tutti i processi che condividono la mappatura.

MAP_PRIVATE: le scritture sono private al processo mapppante ed avvengono in modalità copy-on-write (COW).

MAP_FIXED: la mappatura deve avvenire esattamente all’indirizzo specificato (per usi avanzati e pericolosi).

MAP_POPULATE, MAP_LOCKED, ecc: ottimizzazioni aggiuntive.

Allocazioni anonime vs file-backed
Anonime: (MAP_ANONYMOUS) si ottengono pagine di memoria inizializzate a zero non associate a nessun file. Perfette come alternativa a malloc per grandi allocazioni, oppure per gestire aree di memoria condivisa tra processi attraverso meccanismi di fork e IPC.

File-backed: la memoria è “collegata” a un file sul disco; può essere usata per mappare interi file in memoria, consentendo accesso random efficiente a dati molto grandi (database, immagini, ecc.)14.

Esempi didattici con mmap()
Allocazione anonima (stile malloc)
c
#include <sys/mman.h>
#include <stdio.h>
#define SIZE 4096
int main() {
    void *mem = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    // Uso di mem come un array o buffer
    munmap(mem, SIZE);
    return 0;
}
Questa allocazione riserva 4096 byte di memoria anonima: l’area è perfettamente isolata ed efficientemente restituita col munmap, senza vincoli di LIFO15.

Mappatura di file in memoria
c
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    struct stat sb;
    fstat(fd, &sb);
    void *data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    // Usare *data come buffer in sola lettura
    munmap(data, sb.st_size);
    close(fd);
    return 0;
}
L’uso di mmap per file di grandi dimensioni evita la duplicazione inutile (no memcpy): il kernel effettua il caricamento in RAM delle pagine su richiesta (demand paging)12.

Memoria condivisa (shared memory)
Le mappature anonime condivise tra processi tramite fork sono uno strumento efficiente di IPC. Per accedervi da più processi nativamente separati si possono usare le funzioni shm_open() e shm_unlink().

Come mmap() risolve i limiti di sbrk()
Deallocazione arbitraria e riduzione della frammentazione
Principale differenza con sbrk(): ogni area creata con mmap può essere rilasciata (munmap) in qualsiasi ordine e anche se altri blocchi mappati sono ancora attivi12. Non esiste più il vincolo LIFO: la memoria virtuale viene suddivisa in regioni (VMA, Virtual Memory Area) indipendenti, ciascuna delle quali può essere gestita separatamente.

Questo comporta:

Flessibilità: allocazioni di lunga durata e con pattern casuali o misti LIFO/FIFO sono possibili senza perdita di efficienza;

Frammentazione minima: le regioni liberate vengono immediatamente restituite al kernel, che può riutilizzarle per nuove mappature;

Gestione avanzata: possibilità di mapping temporaneo di file, condivisione di memoria fra processi, creazione di regioni “guard” tramite PROT_NONE/PROT_EXEC e molto altro.

Un tipico allocatore moderno (es. glibc malloc) tende a sfruttare sbrk per le richieste piccole e mmap per quelle di grandi dimensioni, proprio per evitare la frammentazione eccessiva e per poter deallocare subito i blocchi molto grandi10.

Sicurezza: protezioni granulari
Grazie ai campi PROT e MAP, si possono implementare facilmente policy di sicurezza avanzate:

Heap e stack non eseguibili (NX);

Pagine di guardia (PROT_NONE) per identificare overflow;

Area condivisa in sola lettura, scrittura o esecuzione selettiva;

Separazione robusta delle aree di memoria sensibili.

Questa granularità migliora drasticamente la sicurezza rispetto a sbrk, dove ogni modifica del break si riflette su tutta la regione dati.

Compatibilità, standardizzazione e portabilità
A differenza di brk/sbrk (non più POSIX), mmap() è uno standard de facto su tutti i sistemi Unix-like evoluti (e completamente supportato anche in Windows tramite API analoghe — CreateFileMapping, VirtualAlloc, ecc.), rendendo i programmi molto più portabili3.

Copy-on-write: ottimizzazione e casi d’uso
Mappando un file in modalità MAP_PRIVATE, le scritture vengono gestite mediante copy-on-write (COW): le modifiche non sono riflesse nel file originale, e il kernel alloca una pagina privata lazily, solo nel caso in cui il processo scriva su una parte di memoria mappata. Fino al primo write, tutte le copie virtuali condividono la stessa pagina fisica17.

Questa tecnica è alla base anche di fork() ottimizzato nei moderni kernel: quando un processo duplica sé stesso, non si effettua alcuna copia fisica delle pagine, ma si marcano solo come “copy-on-write”. Solo in caso di scrittura una copia effettiva viene allocata. Il risultato è un’enorme efficienza nella creazione di processi, nelle snapshot, e nella gestione della memoria condivisa o semi-privata (esempio lampante: database e processi server)17.

Esempio di utilizzo Copy-On-Write
c
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#define SIZE 4096

int main() {
    int fd = open("testfile", O_RDWR);
    char *area = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    area[0] = 'A'; // Su questa pagina viene effettuata la copia fisica reale, leaving le altre pagine ancora condivise fra processi.
    munmap(area, SIZE);
    close(fd);
    return 0;
}
In questo esempio, la modifica area[0] non è visibile ad altri processi che hanno mappato lo stesso file: solo il processo corrente ne possiede la nuova copia della pagina, a costo zero fino al primo write.

Confronto sintetico sbrk() vs mmap()
Caratteristica	sbrk()/brk()	mmap()
Tipo di gestione	LIFO (solo estensione/accorciamento heap)	Arbitrario (ogni regione può essere liberata indipendentemente)
Frammentazione	Elevata, impossibile liberare “buchi” centrali	Assente, ogni blocco può essere rimappato/rilasciato singolarmente
Sicurezza	Nessuna protezione, tutto r/w	Protezioni granulari: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE
Allocazione file	Impossibile	Si possono mappare file, dispositivi, shared memory
Copy-on-write	Non supportato	Sì (MAP_PRIVATE, fork, snapshot efficienti)
Portabilità (POSIX)	Deprecato, non standard	Standard POSIX (e anche in altri OS moderni)
Uso consigliato	Sconsigliato, solo per legacy	Consigliato per allocazioni avanzate/grandi o IPC, standard per gestori di heap moderni
Deallocazione	Solo alla “punta”, con rischio bug	Liberazione arbitraria con munmap
Tabella di sintesi: differenze fra sbrk()/brk() e mmap9.

Esempi pratici di codice e pattern di uso
Esempio didattico con sbrk()
c
#include <unistd.h>
#include <stdio.h>
int main() {
    void *start = sbrk(0);
    printf("Break iniziale: %p\n", start);
    void *buffer = sbrk(1024); // Alloca 1 KB
    printf("Buffer: %p\n", buffer);
    void *end = sbrk(0);
    printf("Break dopo allocazione: %p\n", end);
    // Liberazione solo possibile LIFO
    brk(start); // Riporta il break al valore iniziale
    return 0;
}
Nota: se usate sbrk insieme a malloc, si rischia la corruzione del gestore heap della glibc, con crash e undefined behaviour.


Dettaglio avanzato: copy-on-write, sicurezza, sharing
Copy-on-write in mmap
L’allocazione copy-on-write rappresenta uno degli aspetti più sofisticati di mmap. Nel caso di MAP_PRIVATE, il kernel condivide fisicamente le stesse pagine tra più processi; solo al primo evento di scrittura, la MMU genera un page fault che:

Alloca una nuova pagina fisica.

Copia il contenuto dalla pagina condivisa alla nuova pagina privata del processo.

Aggiorna la tabella delle pagine del processo.

Fino a quel momento, la RAM non viene duplicata, colossale risparmio di memoria e capacità di scalare la creazione di processi e snapshot19.

Sicurezza: protezioni come garanzia
L’uso di flag come PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE permette policy di sicurezza efficaci:

Heap e stack non eseguibili — riduzione dei risk buffer overflows;

Stack “a pezzi” con pagine di guardia (PROT_NONE);

File di configurazione mappati SOLO in sola lettura;

Dati critici con accesso selettivo, gestiti via mprotect() (ad esempio cambio dinamico delle protezioni a runtime).

Sharing e IPC
Le regioni mmap possono essere dichiarate MAP_SHARED; con l’aggiunta di meccanismi come shm_open(), è possibile simulare “shared memory” anche tra processi non imparentati (diversi dal semplice fork). Strumento chiave nella progettazione di server ad alte prestazioni, IPC, high-frequency trading, database shared memory.

Best-practice, errori concettuali frequenti e raccomandazioni
Mai mischiare sbrk()/brk() e malloc(): si rischia inconsistenza nel gestore heap.

Verifica sempre il valore di ritorno delle system call: MAP_FAILED per mmap, -1 per sbrk.

Liberare sempre la memoria con munmap()/brk(): evitare memory leak.

Usare mmap() per grandi blocchi e sbrk/malloc/free solo per esigenze legacy o allocazioni piccole.

Quando mmap è usato per file-backed, assicurarsi che fd sia aperto con i permessi coerenti con protezioni e flag richieste (MAP_SHARED, PROT_WRITE ecc.).

Per la sicurezza, minimizzare le aree PROT_WRITE ed evitare MAP_FIXED se non strettamente necessario.

Per sharing tra processi non legati da fork, usare shm_open e named shared memory.

In architetture a 64 bit, considerare la randomizzazione indirizzi (ASLR): non fare affidamento su posizionamenti fissi per la sicurezza.

Per IPC o memorizzazione persistente, preferire sempre mappature file-backed rispetto ad aree anonime.

Conclusioni
Il passaggio dall’allocazione heap mediante sbrk/brk alla gestione avanzata tramite mmap riflette la maturazione dell’ecosistema Unix/Linux: la memoria virtuale moderna richiede flessibilità, efficienza nella deallocazione e frammentazione ridotta, protezione avanzata dei dati e compatibilità sistemica.

Sbrk e brk, sebbene storicamente importanti, sono oggi soluzioni obsolete, limitate ad usi didattici o legacy. Le applicazioni robuste e moderne devono preferire mmap per ogni caso in cui sia necessario controllo sull’allocazione o la condivisione della memoria, specialmente dove la granularità, la sicurezza o la gestione efficiente della memoria a lunga durata sono requisiti chiave.

L’utilizzo consapevole di mmap — sfruttando protezioni, flag e modalità copy-on-write — permette sia la realizzazione di sistemi efficienti e sicuri sia una portabilità di massimo livello nei contesti POSIX e beyond.

