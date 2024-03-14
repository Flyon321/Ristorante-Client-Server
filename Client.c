#include <stdio.h>
#include <winsock2.h>


int main() {
    // Inizializzazione della libreria Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Errore durante l'inizializzazione di Winsock.\n");
        return 1;
    }

    // Creazione del socket del client
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Errore durante la creazione del socket del client.\n");
        WSACleanup();
        return 1;
    }

    // Configurazione dell'indirizzo del server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Indirizzo IP del server (locale)
    serverAddr.sin_port = htons(4444); // Porta del server

    // Connessione al server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Errore durante la connessione al server.\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connesso al server del ristorante.\n");

    //Il server manda i posti disponibili
    char seats_info[145];
    int posti = recv(clientSocket, seats_info, sizeof(seats_info), 0);

    if (posti > 0) {
        seats_info[posti] = '\0';
        printf("Dettagli sui posti disponibili:\n%s\n", seats_info);
    } 
    else {
        printf("Errore durante la ricezione dei dettagli sui posti disponibili dal server.\n");
    }

    char NomeCognome[32], buffer[16], G, T, conferma;
    int hh, mm, N = 0;

    printf("Inserire il proprio nome e cognome per la prenotazione: ");
    gets(NomeCognome);
    send(clientSocket, NomeCognome, sizeof(NomeCognome), 0);

    printf("Buongiorno Signor/a %s, per che giorno vuole prenotare? (oggi/domani): ", NomeCognome);
    scanf(" %c", &G);
    printf("Bene, desidererebbe pranzo o cena? (pranzo/cena): ");
    scanf(" %c", &T);
    printf("Selezionare l'ora di arrivo (hh:mm): ");
    scanf("%d:%d", &hh, &mm);
    printf("E come ultimo passo inserire il numero di clienti: ");
    scanf("%d", &N);

    sprintf(buffer, "G%c T%c O%d:%d N%d", G, T, hh, mm, N);
    send(clientSocket, buffer, sizeof(buffer), 0);

    char sintax_message[75];
    int bytesRead1 = recv(clientSocket, sintax_message, sizeof(sintax_message), 0);
    if (bytesRead1 > 0) {
        //sintax_message[bytesRead1] = '\0';
        printf("Messaggio dal server: %s\n", sintax_message);
        if(strstr(sintax_message, "Errore") != NULL){
            closesocket(clientSocket);
            WSACleanup();
        }
    } else {
        printf("Errore durante la ricezione del messaggio dal server.\n");
    }

    //Attesa della risposta dal server
    char check_seats[70];
    int bytesRead2 = recv(clientSocket, check_seats, sizeof(check_seats), 0);

    if (bytesRead2 > 0) {
        //check_seats[bytesRead2] = '\0';
        printf("Messaggio dal server: %s\n", check_seats);
    } else {
        printf("Errore durante la ricezione del messaggio dal server.\n");
    }
    

    // Chiudi la socket e libera le risorse
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}