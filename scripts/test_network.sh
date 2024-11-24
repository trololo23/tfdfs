#!/bin/bash

# Функция для добавления задержки и потерь пакетов
add_network_problems() {
    local container=$1
    local delay=$2
    local loss=$3
    
    echo "Adding network problems to $container: delay=${delay}ms, loss=${loss}%"
    if ! docker exec $container tc qdisc add dev eth0 root netem delay ${delay}ms loss ${loss}%; then
        echo "Failed to add network problems to $container. Make sure the container has NET_ADMIN capability."
        return 1
    fi
}

# Функция для удаления правил tc
remove_network_problems() {
    local container=$1
    
    echo "Removing network problems from $container"
    docker exec $container tc qdisc del dev eth0 root 2>/dev/null || true
}

# Функция для остановки/запуска контейнера
toggle_container() {
    local container=$1
    local action=$2
    
    echo "${action^}ing container $container"
    docker $action $container
}

# Очистка всех правил tc
cleanup() {
    echo "Cleaning up network rules..."
    for container in integral_server1 integral_server2 integral_server3; do
        remove_network_problems $container
    done
}

trap cleanup EXIT 