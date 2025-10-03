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

Illustrazione semplificata:
```
+----------------------------+  Indirizzi alti
|  Argomenti/ambiente        |
+----------------------------+
|  Stack                     |  <--- Cresce verso il basso
+----------------------------+
|  No man’s land             |
+----------------------------+
|  Heap                      |  <--- Cresce verso l’alto
+----------------------------+
|  BSS (dati non inizial.)   |
+----------------------------+
|  Data (dati inizial.)      |
+----------------------------+
|  Codice (text)             |  Indirizzi bassi
+----------------------------+
```

## Il breaking point e la gestione dell’heap con sbrk() e brk()
**Definizione del breaking point:** il breaking point (brk) rappresenta il confine superiore della regione dati allocata dinamicamente dal processo.

- `brk(void *end_data_segment)`: imposta il nuovo breaking point.
- `sbrk(intptr_t increment)`: sposta il break di `increment` byte.

## Funzionamento di sbrk() in C
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

## Heap, break e segmenti dati: chiarimenti
- Lo spazio heap è fra la fine di BSS e il breaking point.
- Il `brk` rappresenta la fine dell’heap e del data segment.
- Crescita limitata: se heap e stack si incontrano, malloc fallisce.

## Come interagiscono malloc(), free() e sbrk()
- `malloc()` e `free()` usano internamente sbrk() o mmap() per grandi allocazioni.
- Free-list per blocchi liberi.
- Spazio liberato non restituito all’OS se non all’estremità.

| Funzione | Scopo | Libertà |
|----------|-------|--------|
| malloc(size) | Alloca memoria dinamica | Può usare sbrk o mmap |
| free(ptr) | Libera blocco precedentemente allocato | Rimane nel pool |
| sbrk(increment) | Estende (o riduce) l’heap | Solo LIFO |

## Limiti strutturali e deprecazione di sbrk()
**Gestione LIFO e frammentazione:** solo l’ultimo blocco può essere liberato, causando frammentazione e perdita di memoria.

**Altre limitazioni:** non thread-safe, gestione conflittuale e limiti di sicurezza.

**Deprecazione:** documentazione ufficiale scoraggia l’uso di sbrk() e brk().

## Introduzione a mmap() nell’allocazione della memoria
**Cos’è mmap():** consente di mappare porzioni di memoria virtuale sia su file fisici che anonime.
```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```
- `addr`: indirizzo suggerito (NULL per kernel).
- `length`: dimensione in byte.
- `prot`: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_NONE.
- `flags`: MAP_PRIVATE, MAP_SHARED, MAP_ANONYMOUS, MAP_FIXED.
- `fd` e `offset`: per file mapping.

**Protezioni e flag:** granularità e sicurezza.

**Allocazioni anonime vs file-backed:**
- Anonime: inizializzate a zero, non associate a file.
- File-backed: mapping di file per accesso efficiente.

## Come mmap() risolve i limiti di sbrk()
- **Deallocazione arbitraria:** ogni area può essere rilasciata indipendentemente.
- **Flessibilità:** pattern di allocazioni casuali o miste.
- **Frammentazione minima:** regioni liberate subito.
- **Sicurezza:** protezioni granulari, heap/stack non eseguibili, pagine di guardia.
- **Compatibilità e portabilità:** standard POSIX, supportato anche su Windows.

## Copy-on-write: ottimizzazione e casi d’uso
- MAP_PRIVATE implementa COW.
- Fino al primo write, RAM non duplicata.
- Base per fork() ottimizzato e snapshot.

## Dettaglio avanzato: copy-on-write, sicurezza, sharing
- Heap e stack non eseguibili.
- Stack con pagine di guardia.
- File in sola lettura.
- Dati critici gestiti con mprotect().
- MAP_SHARED per IPC tra processi non legati.

## Best-practice e raccomandazioni
- Mai mischiare sbrk() e malloc().
- Verifica valori di ritorno delle system call.
- Liberare memoria sempre con munmap()/brk().
- Usare mmap() per grandi blocchi.
- Minimizzare PROT_WRITE e evitare MAP_FIXED.
- Per IPC, usare shm_open e named shared memory.
- Considerare ASLR su 64 bit.

## Conclusioni
Il passaggio da sbrk/brk a mmap riflette la maturazione dell’ecosistema Unix/Linux: flessibilità, efficienza, sicurezza e portabilità. mmap() permette sistemi robusti e sicuri, con granularità e gestione avanzata della memoria.

Nota per il riutilizzo didattico: gli esempi e i concetti presentati sono pensati per repository GitHub e ambienti di esercitazione, con attenzione alle best-practice e conformità POSIX.

