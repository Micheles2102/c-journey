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
