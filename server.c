#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
/*
Cum funcționează:

    Adăugarea în coadă:
        Clientul este adăugat în coadă doar când trimite un mesaj nou.
       Verificam daca clientul nu e deja in coada

    Procesarea mesajelor:
        Thread-ul preia clientul, procesează toate mesajele din buffer, și apoi:
            Îl elimină din coadă (deque).

    Mesaj nou:
        Dacă clientul trimite un mesaj nou, serverul verifică dacă este deja în coadă:
            Dacă nu este, îl adaugă din nou.
            Daca este adauaga mesajul la coada de mesaje
    Daca coada de mesaje e plina, modificam fd din epoll si ii scoatem epollin

    is_processing:

    Indică dacă un thread prelucrează în prezent mesajele clientului.
    Previne atribuirea aceluiași client altui thread în timp ce primul îl procesează.
    Si ajuta al blocarea threadurilor variabila de cond



*/

#define NUMBER_OF_CLIENTS 20
#define NUMBER_OF_THREADS 10
#define MAX_NUMBER_MESSAGES 20
#define MESSAGE_LENGTH 1024


int server_socket,rc,epoll_fd,client_fd; // rc e pt erori
struct sockaddr_in server_addr,client_addr;
struct epoll_event epoll_ev,ret_events[NUMBER_OF_CLIENTS];
pthread_t thread_id[NUMBER_OF_THREADS];

typedef struct client
{
    int client_fd;
    bool is_in_processing;//1 este 0 nu este
    char message_queue[MAX_NUMBER_MESSAGES][MESSAGE_LENGTH];
    int messaje_count;
    pthread_mutex_t client_data_mutex;

}client;

typedef struct queue
{
    int actual_size_of_queue,front,rear,capacity,number_of_clients_processing;
    client clients[NUMBER_OF_CLIENTS];
    pthread_cond_t queue_is_empty;
    pthread_cond_t queue_is_full;
    pthread_mutex_t mutex_queue_empty;
    pthread_mutex_t mutex_queue_full;
}queue;

int get_client_from_queue(queue* coada,int cl_fd)
{
    for(int i=coada->front;i<coada->actual_size_of_queue;i++)
    {
        if(coada->clients[i].client_fd==cl_fd)
            return 1;
    }
    return -1;
}

queue* create_queue(int capacity)
{
    //to do capacity e number_of_clients
}

void enqueue(queue* coada, int client_fd)
{
    //to do ai grija sa l adaugi in coada doar daca nu e in coada(vei face o functie care cauta prin coada si verifica daca client_fd exista deja sau nu)
    // daca exista adaugi mesajul in coada de mesaje
}
void dequee(queue* coada)
{
    // daca vrei o faci daca nu o fac eu
}

int set_nonblocking(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1)
        return -1;
    // to do setare fd flag sa nu se blocheze
}
void* process_client(void* params)
{
    //vedem cine o face pe asta
}

int main(){

    server_socket=socket(AF_INET,SOCK_STREAM,0);
    if(server_socket<0)
    {
        perror("eroare la crearea server socket");
        exit(-1);
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(12345);
    //to do: setare adresa ip server


    rc=bind(server_socket,(struct sockaddr*)(&server_addr),sizeof(server_addr));
    if(rc<0)
    {
        perror("server socket bind");
        exit(-1);
    }

    //to do:urmatoarea etapa server tcp

    rc=set_nonblocking(server_socket);
    if(rc<0)
    {
        perror("setare flag non-blocking server socker");
        exit(-1);
    }

    //to do: creare threaduri cu functia process_client (ne gandim daca le facem detasabile sau nu)

    epoll_fd=epoll_create(NUMBER_OF_CLIENTS);
    if(epoll_fd<0)
    {
        perror("epoll");
        exit(-1);
    }

    //to do: adaugarea server socket in epoll

    while(1)
    {
       int n=epoll_wait(epoll_fd,ret_events,NUMBER_OF_CLIENTS,-1);

        for(int i=0;i<n;i++)
        {
            if(ret_events[i].data.fd==server_socket)
            {
                //to do: acceptare TOTI clientii si adaugarea lor in epoll




            }else
            {
                enque();
            }
        }
    }




}