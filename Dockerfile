# Базовый образ с компилятором
FROM ubuntu:22.04

# Устанавливаем компилятор и утилиты
RUN apt-get update && apt-get install -y g++ make

# Копируем исходники внутрь контейнера
WORKDIR /app
COPY . .

# Компилируем программу
RUN g++ -std=c++17 -o parser main.cpp

# Команда по умолчанию при запуске контейнера
CMD ["./parser"]