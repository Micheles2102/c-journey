# Confronto tra sbrk() e mmap() per l'allocazione dinamica in C

Analisi tecnica di sbrk() e mmap() per l’allocazione dinamica della memoria in C

## Introduzione
L’allocazione dinamica della memoria rappresenta uno degli aspetti fondamentali nella programmazione di sistema in C, specialmente in ambiente Linux o Unix-like. L’evoluzione delle strategie di gestione della memoria nei sistemi operativi riflette sia il progresso dell’hardware, sia la necessità di soddisfare requisiti di flessibilità, efficienza e sicurezza sempre crescenti. In questo contesto, le funzioni sbrk() (oggi considerata deprecata) e mmap() hanno rappresentato — e rappresentano tuttora — due paradigmi radicalmente diversi per la gestione dello spazio di memoria dei processi.

In questo report analizzeremo in profondità i due approcci, a partire dalla struttura della memoria virtuale di un processo in ambiente Linux, illustrando il funzionamento e l’architettura di sbrk() e del concetto di breaking point, evidenziandone i limiti strutturali. Si mostrerà come mmap() abbia superato molte di queste limitazioni, offrendo flessibilità, sicurezza e granularità nell’allocazione e deallocazione della memoria — sia anonima che file-backed.

Il documento è pensato come materiale didattico per una repository GitHub e segue le migliori pratiche di chiarezza, precisione e rigore tecnico nel linguaggio e nella struttura.

## Layout della memoria virtuale di un processo in C/Linux
Il modello di memoria virtuale in Linux prevede la suddivisione dello spazio di indirizzamento di ogni processo in segmenti distinti, ognuno con specifiche finalità semantiche e di accesso:

- **Segmento di testo (text segment):** contiene il codice eseguibile del programma. È di sola lettura e può essere condiviso fra più processi.
- **Segmento dati (data segment):** contiene variabili globali e statiche inizializzate. Questo segmento è r/w (read-write).
- **Segmento BSS:** destinato alle variabili globali e statiche non inizializzate esplicitamente. Allocato e inizializzato a zero dal kernel.
- **Heap:** spazio dinamico che cresce verso gli indirizzi più alti, utilizzato per l’allocazione dinamica a runtime.
- **No man’s land:** area vuota o di riserva fra heap e stack.
- **Stack:** cresce verso indirizzi più bassi, memorizza dati temporanei.
- **Argomenti da riga di comando:** allocati all’estremo superiore dello spazio virtuale.

![Struttura della memoria virtuale](https://github.com/Micheles2102/c-journey/blob/main/manual-malloc/Memory_Layout.jpg)

## Il breaking point e la gestione dell’heap con sbrk() e brk()
**Definizione del breaking point:** Il breaking point (brk, o "programma break") rappresenta il confine superiore della regione dati allocata dinamicamente dal processo: tipicamente, la fine del segmento heap5. L’heap viene creato subito dopo i segmenti dati e BSS (vedi layout sopra); la posizione del breaking point viene gestita dal kernel e può essere modificata via le system call brk() e sbrk().

- `brk(void *end_data_segment)`: imposta direttamente il nuovo breaking point all’indirizzo fornito.
- `sbrk(intptr_t increment)`: sposta il break di `increment` byte(positivo o negativo).
Queste funzioni permettono di aumentare (o, in linea teorica, anche diminuire) la quantità di memoria allocata per l’heap del processo. Lo spazio allocato viene inizializzato a zero

## Funzionamento di sbrk() in C
L’uso diretto di sbrk() è oggi raro e sconsigliato, tuttavia ai fini didattici è importante comprenderne la logica. Un esempio minimale:
```c
#include <unistd.h>
#include <stdio.h>
int main() {
    void *current_break = sbrk(0);
    printf("Break iniziale: %p\n", current_break);
    sbrk(1024);
    void *new_break = sbrk(0);
    printf("Nuovo break: %p\n", new_break);
    return 0;
}
```
Questo programma mostra come incrementare il breaking point, di fatto riservando più memoria all’heap attraverso la chiamata a basso livello a sbrk()7. Si noti che lo spazio aggiunto non viene automaticamente “gestito”: il programmatore deve tenere traccia degli intervalli allocati con sbrk() e non esiste alcun controllo formale su sovrascritture o perdite

## Heap, break e segmenti dati: chiarimenti
Nel lessico tradizionale e in molte fonti, con “segmento dati” si intende spesso data + bss + heap. In ambiente Linux, però, è importante distinguere che:
- Lo spazio heap è la parte dinamica allocata fra la fine di BSS ed il breaking point; la posizione del breaking point può essere ispezionata con sbrk(0).
- Il `brk` rappresenta quindi la “fine dell’heap” e la “fine del data segment” in senso C storico.
Attenzione: per limiti architetturali e del kernel, la crescita dell’heap è limitata. Se heap e stack si incontrano, si ottiene una condizione di memoria esaurita: la malloc fallisce e ritorna NULL

## Come interagiscono malloc(), free() e sbrk()
Nell’implementazione classica della libreria C (glibc), le funzioni malloc() e free() utilizzano internamente sbrk() per espandere o contrarre l’heap (anche se, nei moderni sistemi, per grandi allocazioni si utilizzano direttamente le mappature con mmap(), come vedremo).

Durante le allocazioni e deallocazioni di piccole dimensioni:
- il gestore heap (malloc) tiene un “free-list” di blocchi liberi;
- se necessario richiede nuovo spazio “allungando” il break tramite sbrk();
- la memoria liberata tramite free viene reinserita in questa lista per riuso;
- **non è possibile restituire spazio intermedio all’OS se non coincidente col break**.

| Funzione | Scopo | Libertà |
|----------|-------|--------|
| malloc(size) | Alloca memoria dinamica | Può usare sbrk o mmap |
| free(ptr) | Libera blocco precedentemente allocato | Rimane nel pool |
| sbrk(increment) | Estende (o riduce) l’heap | Solo LIFO |

## Limiti strutturali e deprecazione di sbrk()
**Gestione LIFO e frammentazione:** 
Il paradigma di sbrk()/brk() impone una struttura logica di tipo LIFO (Last-In, First-Out): la memoria può essere effettivamente restituita al sistema solo se si libera esattamente l’ultima porzione allocata (cioè l’heap “risale” al breaking point precedente). Se si libera un blocco intermedio fra altri blocchi ancora in uso, non è possibile restituire tale spazio all’OS fino a che tutti i blocchi successivi non vengono anch’essi liberati.

Questo causa:

**frammentazione interna:** lo spazio liberato ma non ricongiungibile al breaking point rimane inutilizzato ma occupato nel pool dell’heap;

**impossibilità di deallocare selettivamente::** solo strategie LIFO possono essere efficienti — pattern simili a stack, non adatti a sistemi reali e generici che richiedono allocazioni/deallocazioni arbitrarie;

**perdita di memoria:** in applicazioni di lunga durata, la frammentazione può degradare progressivamente le prestazioni complessive del sistema.

Un esempio chiarificatore:

Supponiamo che si allochino in sequenza tre blocchi: A, B, C. Si liberano nell’ordine A, poi B, poi C:

- Solo liberando C (ultimo) si può realmente “accorciare” l’heap con sbrk(negativo);

- Finché B e C sopravvivono, lo spazio di A rimane inutilizzabile dall’OS, anche se può essere eventualmente riutilizzato da successive malloc().

## Altre limitazioni
Oltre al limite LIFO:

-**Non è thread-safe:** chiamate concorrenti possono produrre risultati imprevedibili.

- **Gestione conflittuale:** se la memoria viene gestita sia da malloc che direttamente via sbrk da altre parti del codice, si possono corrompere le strutture dati interne del gestore della memoria (dangerous).

- **Limiti di sicurezza:** nessun meccanismo di protezione degli accessi (es. scrittura accidentale, esecuzione di codice dove non consentito).

- **Portabilità:** sbrk() e brk() non sono più parte della specifica POSIX da anni e sono considerate obsolete

## Deprecazione e sicurezza
Le man page e la documentazione ufficiale scoraggiano l’uso di sbrk() e brk() negli sviluppi moderni:

“Avoid using brk() and sbrk(): the malloc(3) memory allocation package is the portable and comfortable way of allocating memory…”

L’uso diretto di queste primitive dovrebbe essere riservato solo a specifici contesti legacy, programmi embedded o test didattici. Nel resto dei casi è caldamente consigliato fare affidamento sulle funzioni della libreria standard (malloc, calloc, realloc, free) o — per allocazioni avanzate — sulle mappature fornite da mmap().

## Introduzione a mmap() nell’allocazione della memoria
**Cos’è mmap():**
La system call mmap() consente di mappare direttamente porzioni di memoria virtuale del processo sia su file fisici che su pagine anonime non associate a file, con controllo granulare sulle protezioni di accesso, la visibilità (privata o condivisa), e la posizione nello spazio virtuale12.

La dichiarazione è la seguente:
```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```
- `addr`: indirizzo suggerito (NULL per kernel).
- `length`: dimensione in byte.
- `prot`: protezioni di accesso (PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE).
- `flags`: comportamento della mappatura (MAP_PRIVATE, MAP_SHARED, MAP_ANONYMOUS, MAP_FIXED, ecc.).
- `fd`:file descriptor (ignorato se MAP_ANONYMOUS).
- `offset`: offset all’interno del file (deve essere multiplo della dimensione di pagina).

## Campi fondamentali: protezioni e flag

### **Protezioni (PROT)**

- **PROT_READ**: accesso in lettura consentito.
- **PROT_WRITE**: accesso in scrittura consentito.
- **PROT_EXEC**: esecuzione di codice consentita (es. per pagine di codice).
- **PROT_NONE**: nessun accesso consentito (utile per pagine di guardia).

**Questa granularità permette, ad esempio, di implementare stack o heap non eseguibili (NX), proteggendo da errori o attacchi di sicurezza.**

### **Flag (MAP)**

- **MAP_ANONYMOUS (o MAP_ANON)**: la memoria non è associata a file; i contenuti sono zero-initialized.
- **MAP_SHARED**: le scritture sulla mappatura sono riflesse nel file sottostante e visibili a tutti i processi che condividono la mappatura.
- **MAP_PRIVATE**: le scritture sono private al processo mapppante ed avvengono in modalità copy-on-write (COW).
- **MAP_FIXED**: la mappatura deve avvenire esattamente all’indirizzo specificato (per usi avanzati e pericolosi).
- **MAP_POPULATE, MAP_LOCKED, ecc**: ottimizzazioni aggiuntive.

## Allocazioni anonime vs file-backed

### **Anonime**  
- (**MAP_ANONYMOUS**) si ottengono pagine di memoria inizializzate a zero non associate a nessun file.  
- Perfette come alternativa a malloc per grandi allocazioni, oppure per gestire aree di memoria condivisa tra processi attraverso meccanismi di fork e IPC.

### **File-backed**  
- La memoria è “collegata” a un file sul disco.  
- Può essere usata per mappare interi file in memoria, consentendo accesso random efficiente a dati molto grandi (database, immagini, ecc.).

## Come `mmap()` risolve i limiti di `sbrk()`

### **Deallocazione arbitraria e riduzione della frammentazione**

Principale differenza con `sbrk()`: ogni area creata con `mmap` può essere rilasciata (`munmap`) in qualsiasi ordine e anche se altri blocchi mappati sono ancora attivi12. Non esiste più il vincolo LIFO: la memoria virtuale viene suddivisa in regioni (VMA, Virtual Memory Area) indipendenti, ciascuna delle quali può essere gestita separatamente.

Questo comporta:

- **Flessibilità**: allocazioni di lunga durata e con pattern casuali o misti LIFO/FIFO sono possibili senza perdita di efficienza;
- **Frammentazione minima**: le regioni liberate vengono immediatamente restituite al kernel, che può riutilizzarle per nuove mappature;
- **Gestione avanzata**: possibilità di mapping temporaneo di file, condivisione di memoria fra processi, creazione di regioni “guard” tramite `PROT_NONE`/`PROT_EXEC` e molto altro.

Un tipico allocatore moderno (es. `glibc malloc`) tende a sfruttare `sbrk` per le richieste piccole e `mmap` per quelle di grandi dimensioni, proprio per evitare la frammentazione eccessiva e per poter deallocare subito i blocchi molto grandi10.

---

### **Sicurezza: protezioni granulari**

Grazie ai campi `PROT` e `MAP`, si possono implementare facilmente policy di sicurezza avanzate:

- Heap e stack non eseguibili (NX);
- Pagine di guardia (`PROT_NONE`) per identificare overflow;
- Area condivisa in sola lettura, scrittura o esecuzione selettiva;
- Separazione robusta delle aree di memoria sensibili.

**Questa granularità migliora drasticamente la sicurezza rispetto a `sbrk`, dove ogni modifica del break si riflette su tutta la regione dati.**

---

### **Compatibilità, standardizzazione e portabilità**

A differenza di `brk`/`sbrk` (non più POSIX), `mmap()` è uno standard de facto su tutti i sistemi Unix-like evoluti (e completamente supportato anche in Windows tramite API analoghe — `CreateFileMapping`, `VirtualAlloc`, ecc.), rendendo i programmi molto più portabili3.

---

### **Copy-on-write: ottimizzazione e casi d’uso**

Mappando un file in modalità `MAP_PRIVATE`, le scritture vengono gestite mediante copy-on-write (COW): le modifiche non sono riflesse nel file originale, e il kernel alloca una pagina privata lazily, solo nel caso in cui il processo scriva su una parte di memoria mappata. Fino al primo write, tutte le copie virtuali condividono la stessa pagina fisica17.

Questa tecnica è alla base anche di `fork()` ottimizzato nei moderni kernel: quando un processo duplica sé stesso, non si effettua alcuna copia fisica delle pagine, ma si marcano solo come “copy-on-write”. Solo in caso di scrittura una copia effettiva viene allocata. Il risultato è un’enorme efficienza nella creazione di processi, nelle snapshot, e nella gestione della memoria condivisa o semi-privata (esempio lampante: database e processi server)

---

## Confronto sintetico `sbrk()` vs `mmap()`

| **Caratteristica**       | **sbrk()/brk()**                                      | **mmap()**                                                             |
|--------------------------|-------------------------------------------------------|------------------------------------------------------------------------|
| Tipo di gestione         | LIFO (solo estensione/accorciamento heap)            | Arbitrario (ogni regione può essere liberata indipendentemente)       |
| Frammentazione           | Elevata, impossibile liberare “buchi” centrali       | Assente, ogni blocco può essere rimappato/rilasciato singolarmente    |
| Sicurezza                | Nessuna protezione, tutto r/w                        | Protezioni granulari: `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`, `PROT_NONE` |
| Allocazione file         | Impossibile                                           | Si possono mappare file, dispositivi, shared memory                   |
| Copy-on-write            | Non supportato                                        | Sì (`MAP_PRIVATE`, `fork`, snapshot efficienti)                       |
| Portabilità (POSIX)      | Deprecato, non standard                               | Standard POSIX (e anche in altri OS moderni)                          |
| Uso consigliato          | Sconsigliato, solo per legacy                         | Consigliato per allocazioni avanzate/grandi o IPC, standard per gestori di heap moderni |
| Deallocazione            | Solo alla “punta”, con rischio bug                   | Liberazione arbitraria con `munmap`                                   |

**Tabella di sintesi: differenze fra `sbrk()`/`brk()` e `mmap`**

## 🧾 Conclusioni

Il passaggio dall’allocazione heap mediante `sbrk()`/`brk()` alla gestione avanzata tramite `mmap()` riflette la maturazione dell’ecosistema Unix/Linux: la memoria virtuale moderna richiede flessibilità, efficienza nella deallocazione e frammentazione ridotta, protezione avanzata dei dati e compatibilità sistemica.

`Sbrk` e `brk`, sebbene storicamente importanti, sono oggi soluzioni obsolete, limitate ad usi didattici o legacy. Le applicazioni robuste e moderne devono preferire `mmap()` per ogni caso in cui sia necessario controllo sull’allocazione o la condivisione della memoria, specialmente dove la granularità, la sicurezza o la gestione efficiente della memoria a lunga durata sono requisiti chiave.

L’utilizzo consapevole di `mmap` — sfruttando protezioni, flag e modalità copy-on-write — permette sia la realizzazione di sistemi efficienti e sicuri sia una portabilità di massimo livello nei contesti POSIX e beyond.

---
## 📚 Key References

1. [Memory Allocators 101 - Write a simple memory allocator](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory)

2. [c - Advantages of mmap () over sbrk ()? - Stack Overflow](https://stackoverflow.com)  
## 📚 References

1. [MemoryLayoutofLinuxProcess/3_Memory_layout.md at main - GitHub](https://github.com)  
2. [Presentazione standard di PowerPoint - units.it](https://moodle2.units.it)  
3. [linuxmemory.html - Florida State University](https://www.cs.fsu.edu)  
4. [sbrk (2): change data segment size - Linux man page](https://linux.die.net)  
5. [c - What does the brk () system call do? - Stack Overflow](https://stackoverflow.com)  
6. [brk o sbrk Subroutine - IBM](https://www.ibm.com)  
7. [Mastering the Brk System Call in C – TheLinuxCode](https://thelinuxcode.com)  
8. [Understanding Heap Memory Allocation in C - sbrk and brk](https://dev.to)  
9. [Process memory layout - Unix & Linux Stack Exchange](https://unix.stackexchange.com)  
10. [mmap (2) - Linux manual page - man7.org](https://www.man7.org)  
11. [An In-Depth Guide to Using mmap () for Memory Mapping in C on Linux](https://thelinuxcode.com)  
12. [mmap File-backed mapping vs Anonymous mapping in Linux](https://stackoverflow.com)  
13. [c - Using mmap over a file - Stack Overflow](https://stackoverflow.com)  
14. [what scenarios we set file descriptor as -1 in mmap?](https://stackoverflow.com)  
15. [The mmap() copy-on-write trick: reducing memory usage of array copies](https://pythonspeed.com)  
16. [Copy-on-write - Wikipedia](https://it.wikipedia.org)  
17. [Abort in glibc while trying to use sbrk to reduce the size of the data ...](https://stackoverflow.com)  
18. [How does copy-on-write work with read-only virtual pages in ... - linux](https://unix.stackexchange.com)  
19. [Sottoroutine di protezione - IBM](https://www.ibm.com)  
20. [What is the difference between MAP_SHARED and MAP_PRIVATE in the mmap ...](https://stackoverflow.com)


