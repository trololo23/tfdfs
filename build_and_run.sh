#!/bin/bash

# Собираем образы
echo "Building Docker images..."
docker-compose build

# Даем права на выполнение скриптам
chmod +x scripts/test_network.sh
chmod +x scripts/run_tests.sh

# Запускаем тесты
echo "Running tests..."
./scripts/run_tests.sh 