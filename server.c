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
#include <sys/errno.h>
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
    bool is_in_epoll;
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

queue *coada;

int get_client_from_queue(queue* coada,int cl_fd)
{
    for(int i=coada->front;i<coada->actual_size_of_queue;i++)
    {
        if(coada->clients[i].client_fd==cl_fd)
            return i;
    }
    return -1;
}

void add_message_in_client_queue(queue*q,int poz,char buff[MESSAGE_LENGTH])
{
    pthread_mutex_lock(&(q->mutex_queue_full));
    client client=q->clients[poz];
    pthread_mutex_unlock(&(q->mutex_queue_full));
    pthread_mutex_lock(&(client.client_data_mutex));
    if(client.messaje_count>MAX_NUMBER_MESSAGES)
    {
        //Daca coada de mesaje e plina, modificam fd din epoll si ii scoatem epollin
        struct epoll_event ev;
        ev.events = 0; 
        ev.data.fd = client.client_fd;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client.client_fd, &ev) == -1) {
            perror("epoll_ctl: EPOLL_CTL_MOD");
        }
    }
    strcpy(client.message_queue[client.messaje_count],buff);
    client.message_queue[client.messaje_count][MAX_NUMBER_MESSAGES-1]='\0';
    client.messaje_count++;
    pthread_mutex_unlock(&(client.client_data_mutex));

}

queue* create_queue(int capacity)
{
    //to do capacity e number_of_clients
    queue *q=(queue*)malloc(sizeof(queue));
    q->capacity=capacity;
    q->actual_size_of_queue=q->front=q->rear=q->number_of_clients_processing=0;
    pthread_cond_init(&(q->queue_is_empty),NULL);
    pthread_cond_init(&(q->queue_is_full),NULL);
    pthread_mutex_init(&(q->mutex_queue_empty),NULL);
    pthread_mutex_init(&(q->mutex_queue_full),NULL);
    for (int i = 0; i < NUMBER_OF_CLIENTS; i++) {
        pthread_mutex_init(&(q->clients[i].client_data_mutex), NULL);
    }
    return q;
}

void enqueue(queue* coada, int client_fd)
{
    //to do ai grija sa l adaugi in coada doar daca nu e in coada(vei face o functie care cauta prin coada si verifica daca client_fd exista deja sau nu)
    // daca exista adaugi mesajul in coada de mesaje
    pthread_mutex_lock(&(coada->mutex_queue_full));
    while(coada->actual_size_of_queue==coada->capacity)
    {
        pthread_cond_wait(&(coada->queue_is_full),&(coada->mutex_queue_full));
    }
    coada->clients[coada->actual_size_of_queue].client_fd=client_fd;
    coada->actual_size_of_queue++;
    coada->rear=(coada->rear+1)%coada->capacity;
    pthread_mutex_unlock(&(coada->mutex_queue_full));
    pthread_cond_signal(&(coada->queue_is_empty));
}
void dequeue(queue* coada)
{
    pthread_mutex_lock(&(coada->mutex_queue_full));
    while(coada->actual_size_of_queue==0)
    {
        pthread_cond_wait(&(coada->queue_is_empty),&(coada->mutex_queue_full));
    }
    coada->actual_size_of_queue--;
    coada->front=(coada->front+1)%coada->capacity;
    pthread_mutex_unlock(&(coada->mutex_queue_full));
    pthread_cond_signal(&(coada->queue_is_full));
}

int set_nonblocking(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1)
        return -1;
    // to do setare fd flag sa nu se blocheze
    return fcntl(fd,F_SETFL,O_NONBLOCK|flags);
}
void* process_client(void* params)
{
   
    queue *q=(queue *)params;
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
    rc=inet_pton(AF_INET,"127.0.0.1",&(server_addr.sin_addr));
    if(rc<0)
    {
        perror("eroare inet_pton\n");
        exit(-1);
    }


    rc=bind(server_socket,(struct sockaddr*)(&server_addr),sizeof(server_addr));
    if(rc<0)
    {
        perror("server socket bind");
        exit(-1);
    }

    //to do:urmatoarea etapa server tcp
    rc=listen(server_socket,NUMBER_OF_CLIENTS);
    if(rc<0)
    {
        perror("eroare la listen\n");
        exit(-1);
    }

    rc=set_nonblocking(server_socket);
    if(rc<0)
    {
        perror("setare flag non-blocking server socker");
        exit(-1);
    }

    coada=create_queue(NUMBER_OF_CLIENTS);

    //to do: creare threaduri cu functia process_client (ne gandim daca le facem detasabile sau nu)
    for(int i=0;i<NUMBER_OF_THREADS;i++)
    {
        pthread_create(&thread_id[i],NULL,process_client,coada);
    }

    epoll_fd=epoll_create(NUMBER_OF_CLIENTS);
    if(epoll_fd<0)
    {
        perror("epoll");
        exit(-1);
    }

    //to do: adaugarea server socket in epoll
    epoll_ev.data.fd=server_socket;
    epoll_ev.events=EPOLLIN;
    rc=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_socket,&epoll_ev);
    if(rc<0)
    {
        perror("eroare adaugare socket server in epoll\n");
        exit(-1);
    }
    

    while(1)
    {
       int n=epoll_wait(epoll_fd,ret_events,NUMBER_OF_CLIENTS,-1);

        for(int i=0;i<n;i++)
        {
            if(ret_events[i].data.fd==server_socket)
            {
                //to do: acceptare TOTI clientii si adaugarea lor in epoll
                while(1)
                {
                    client_fd=accept(server_socket,(struct sockaddr*)(&client_addr),sizeof(client_addr));
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("Eroare la acceptare client\n");
                        }
                    }
                    epoll_ev.data.fd=client_fd;
                    epoll_ev.events=EPOLLIN;
                    rc=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&epoll_ev);
                    if(rc<0)
                    {
                        perror("eroare la epoll_ctl la client\n");
                        exit(-1);
                    }
                    
                }

            }else
            {
                int poz_client_in_queue=get_client_from_queue(coada,ret_events[i].data.fd);
                if(poz_client_in_queue==-1)
                {
                    enqueue(coada,ret_events[i].data.fd);
                    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ret_events[i].data.fd,NULL);
                }else
                {
                    client_fd = ret_events[i].data.fd;
                    char buffer[MESSAGE_LENGTH];
                    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_read <= 0) {
                        if (bytes_read == 0 || errno == ECONNRESET) {
                            printf("Client %d s-a deconectat.\n", client_fd);
                            close(client_fd);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                        } else {
                            perror("Eroare la recv\n");
                        }
                    }else
                    {
                        add_message_in_client_queue(coada,poz_client_in_queue,buffer);
                    }    
                }
            }
        }
    }




}