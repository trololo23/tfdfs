#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "${SCRIPT_DIR}/test_network.sh"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "Starting integration tests..."

# Тест 1: Нормальная работа
echo -e "${GREEN}Test 1: Normal operation${NC}"
docker-compose up -d
sleep 5
docker logs integral_client
docker-compose down

# Тест 2: Потеря пакетов
echo -e "${GREEN}Test 2: Packet loss${NC}"
docker-compose up -d
sleep 2
add_network_problems integral_server1 0 30
add_network_problems integral_server2 0 50
sleep 5
docker logs integral_client
docker-compose down

# Тест 3: Задержки
echo -e "${GREEN}Test 3: Network delays${NC}"
docker-compose up -d
sleep 2
add_network_problems integral_server1 100 0
add_network_problems integral_server2 200 0
add_network_problems integral_server3 300 0
sleep 10
docker logs integral_client
docker-compose down

# Тест 4: Отключение серверов
echo -e "${GREEN}Test 4: Server failures${NC}"
docker-compose up -d
sleep 5
toggle_container integral_server1 stop
sleep 2
toggle_container integral_server2 stop
sleep 5
docker logs integral_client
# toggle_container integral_server1 start
# toggle_container integral_server2 start
sleep 5
docker logs integral_client
docker-compose down

# Тест 5: Комбинированные проблемы
echo -e "${GREEN}Test 5: Combined problems${NC}"
docker-compose up -d
sleep 2
add_network_problems integral_server1 100 20
toggle_container integral_server2 stop
add_network_problems integral_server3 200 30
sleep 10
toggle_container integral_server2 start
sleep 5
docker logs integral_client
docker-compose down

echo "Tests completed!" 