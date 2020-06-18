#!/bin/bash

args=("$@")

SCRIPT_NAME=$(basename ${BASH_SOURCE[0]})

if [ "$#" != 2 ]; then
    echo -e "\e[31mERROR: $SCRIPT_NAME takes 2 params: msq_queue_key delay\e[0m"
    return
fi

MSG_QUEUE_KEY=${args[0]}
DELAY=${args[1]}

# definire la lista dei device
devices=(2443 2444 2445 2446 2447);
# definire quanti messaggi inviare ad ogni device (in ordine con devices)
n_messages=(4 5 6 3 6 10);
# definire messaggio di base per ogni device
# il messaggio inviato sarà poi "messaggio message_id"
messages=("system call" "ciao" "prova" "test" "ultimo");
# definire i message id di partenza per ogni gruppo di messaggi da inviare
message_ids=(1 10 20 30 40);
# definire max distance per ogni gruppo di messaggi
max_distance=(2 3 4 5 6);

for i in "${!devices[@]}"
do
	./script/generate_clients.sh $MSG_QUEUE_KEY ${n_messages[$i]} ${devices[$i]} ${message_ids[$i]} "${messages[$i]}" ${max_distance[$i]} $DELAY &
done
