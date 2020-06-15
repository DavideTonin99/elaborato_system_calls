# PROGETTO SYSTEM CALLS

## SCRIPT
Nella cartella ``script`` sono presenti tre script bash:

#### generate_clients.sh

``Parametri``: 

    msq_queue_key n_clients pid_receiver message_id message max_distance delay
``Descrizione``

- msq_queue_key (int): la msq_queue_key su cui comunicano il client e ackManager
- n_clients (int): quanti client devo avviare
- pid_receiver (int): pid del device a cui mandare il messaggio creato dal client
- message_id (int): id del messaggio. Se vengono avviati più client, message_id rappresenta il message_id di partenza, per ogni client aggiuntivo il message_id verrà incrementato di 1
- message (string): il messaggio che il client manda al device se vengono avviati più client, ad ogni messaggio viene aggiunto in append il message_id
- max_distance (double): massima distanza a cui il device può inviare messaggi

``NB``: se il messaggio inserito ha spazi in mezzo, mettere " all inizio e alla fine

---

#### remove_ipcs.sh

    Rimuove tutte le ipc generate dall'utente

---

#### generate_pos_file.sh

``Parametri``:

    n_devices n_posizioni

``Descrizione``:
    
    genera il file posizioni con posizioni random

---

    Progetto: system calls
    Corso: sistemi operativi
    Davide Tonin VR437255
    Anno accademico 2019/2020