#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>

int oggi_pranzo = 20, oggi_cena = 20, domani_pranzo = 20, domani_cena = 20;

void save_booking(const char* name, int num_seats, char day, char meal) {
    FILE* file = fopen("prenotazioni.txt", "a");
    if (file == NULL) {
        printf("Errore nell'apertura del file delle prenotazioni.\n");
        return;
    }
    fprintf(file, "Nome e cognome: %s, Numero di posti prenotati: %d, Giorno: %c, Pasto: %c\n", name, num_seats, day, meal);
    fclose(file);
}

void print_seats_for_client(char* informazioni) {
    sprintf(informazioni, "Posti disponibili oggi a pranzo: %d\nPosti disponibili oggi a cena: %d\nPosti disponibili domani a pranzo: %d\nPosti disponibili domani a cena: %d", oggi_pranzo, oggi_cena, domani_pranzo, domani_cena);
}

void print_seats() {
    printf("\nPosti disponibili oggi a pranzo: %d", oggi_pranzo);
    printf("\nPosti disponibili oggi a cena: %d", oggi_cena);
    printf("\nPosti disponibili domani a pranzo: %d", domani_pranzo);
    printf("\nPosti disponibili domani a cena: %d", domani_cena);
}

char* check_and_book_seats(char giorno, char pasto, int N) {
    int* posti_disponibili = NULL;
    char* messaggio = (char*)malloc(256);

    if (giorno == 'o') {
        if (pasto == 'p') {
            posti_disponibili = &oggi_pranzo;
        } else if (pasto == 'c') {
            posti_disponibili = &oggi_cena;
        }
    } else if (giorno == 'd') {
        if (pasto == 'p') {
            posti_disponibili = &domani_pranzo;
        } else if (pasto == 'c') {
            posti_disponibili = &domani_cena;
        }
    }

    if (posti_disponibili != NULL && *posti_disponibili >= N) {
        *posti_disponibili -= ((N + 3) / 4)*4;
        sprintf(messaggio, "Ci sono posti disponibili e la prenotazione e' avvenuta con successo");
        return messaggio;
    } else {
       sprintf(messaggio, "Non ci sono posti disponibili, provi per un altro giorno");
       return messaggio;
    }
}

char* check_booking(const char* booking) {
    char* messaggio = (char*)malloc(256);
    if (strlen(booking) < 13) {
        sprintf(messaggio, "Errore: La prenotazione è troppo corta.\n");
        return messaggio;
    }

    if (booking[1] != 'o' && booking[1] != 'd') {
        sprintf(messaggio, "Errore: Formato errato dopo 'G'. Deve essere 'o' o 'd'.\n");
        return messaggio;
    }

    if (booking[4] != 'p' && booking[4] != 'c') {
        sprintf(messaggio, "Errore: Formato errato dopo 'T'. Deve essere 'p' o 'c'.\n");
        return messaggio;
    }

    int hh, mm;
    if (sscanf(booking + 7, "%d:%d", &hh, &mm) != 2 || hh < 0 || hh > 23 || mm < 0 || mm > 59) {
        sprintf(messaggio, "Errore: Formato dell'orario non valido.\n");
        return messaggio;
    }

    char meal_type = booking[4];
    if ((meal_type == 'p' && (hh < 12 || hh >= 14)) || (meal_type == 'c' && (hh < 19 || hh >= 22))) {
        sprintf(messaggio, "Errore: L'orario non è consentito per il tipo di pasto selezionato.\n");
        return messaggio;
    }
    sprintf(messaggio, "OK\n");
    return messaggio;
}

void handle_client(void* arg) {
    SOCKET clientSocket = (SOCKET)arg;
    char buffer[32], NomeCognome[32], G, T;
    int hh, mm, N;

    char seats_message[145];
    print_seats_for_client(seats_message);
    send(clientSocket, seats_message, strlen(seats_message), 0);
    recv(clientSocket, NomeCognome, sizeof(NomeCognome), 0);

    recv(clientSocket, buffer, sizeof(buffer), 0);

    sscanf(buffer, "G%c T%c O%d:%d N%d", &G, &T, &hh, &mm, &N);

    // Esempio di conferma
    printf("In arrivo una prenotazione per %c, %c, alle %d:%d per %d persone", G, T, hh, mm, N);
    printf("\nElaborazione..\n");
    char* sintax_message = check_booking(buffer);
    send(clientSocket, sintax_message, strlen(sintax_message), 0);
    free(sintax_message);

    char *check_seats = check_and_book_seats(G, T, N);
    send(clientSocket, check_seats, strlen(check_seats), 0);
    if (strstr(check_seats, "avvenuta con successo") != NULL) {
        save_booking(NomeCognome, N, G, T);
    }

    free(check_seats);

}

int main() {
    // Inizializzazione della libreria Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore durante l'inizializzazione di Winsock.\n");
        return 1;
    }

    // Creazione del socket del server
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Errore durante la creazione del socket del server.\n");
        WSACleanup();
        return 1;
    }

    // Configurazione dell'indirizzo del server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(4444); // Porta di ascolto

    // Associazione del socket a un indirizzo
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore durante l'associazione del socket.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Inizio dell'ascolto per le connessioni in entrata
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        printf("Errore durante l'inizio dell'ascolto.\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server in ascolto...\n");

    while (1) {
        // Accettazione delle connessioni dei client
        SOCKET clientSocket;
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Errore durante l'accettazione della connessione.\n");
        } else {
            printf("Connessione accettata.\n");
            // Avvia un thread per gestire il client
            _beginthread(handle_client, 0, (void*)clientSocket);
        }
        printf("Nuova connessione, da: %s, nella porta: %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
