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
#include <signal.h>
#include <sys/signal.h>
#include <sys/signalfd.h>
#define _GNU_SOURCE
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


 !!!!!! cleanup, rezolvare deconectare client
*/

#define NUMBER_OF_CLIENTS 20
#define NUMBER_OF_THREADS 10
#define MAX_NUMBER_MESSAGES 20
#define MESSAGE_LENGTH 1056


int server_socket,rc,epoll_fd,client_fd, signal_fd; // rc e pt erori
struct sockaddr_in server_addr,client_addr;
struct epoll_event epoll_ev,ret_events[NUMBER_OF_CLIENTS];
pthread_t thread_id[NUMBER_OF_THREADS];
volatile int shutdown_flag = 0;

typedef struct client
{
    int client_fd;
    int is_in_epoll;
    char message_queue[MAX_NUMBER_MESSAGES][MESSAGE_LENGTH];
    int messaje_count;
    int client_id;
    pthread_mutex_t client_data_mutex;
}client;

typedef struct queue
{
    int actual_size_of_queue,front,rear,capacity;
    client* clients[NUMBER_OF_CLIENTS];
    pthread_cond_t queue_is_empty;
    pthread_cond_t queue_is_full;
    pthread_mutex_t mutex_queue_empty;
    pthread_mutex_t mutex_queue_full;
    
}queue;

queue *coada;

client* get_client_from_queue(queue* coada, int cl_fd)
{
   
    if (!coada) return NULL;
    
    pthread_mutex_lock(&(coada->mutex_queue_full));
    client* result = NULL;
    
    // Search from front to actual size, not just using front
    int curent = coada->front;
    int count = 0;
    
    while (count < coada->actual_size_of_queue) {
        int idx = (curent + count) % coada->capacity;
        if (coada->clients[idx] && coada->clients[idx]->client_fd == cl_fd) {
            result = coada->clients[idx];
            break;
        }
        count++;
    }
    
    pthread_mutex_unlock(&(coada->mutex_queue_full));

    return result;
}

void add_message_in_client_queue(queue* q, client* c, char buff[MESSAGE_LENGTH])
{
    if (!c || !q) return;

    pthread_mutex_lock(&c->client_data_mutex);
    
    if (c->messaje_count >= MAX_NUMBER_MESSAGES) {
        struct epoll_event ev;
        ev.events = 0;
        ev.data.fd = c->client_fd;
        
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, c->client_fd, &ev) == -1) {
            perror("epoll_ctl: EPOLL_CTL_MOD");
        }
        c->is_in_epoll = 0;
        pthread_mutex_unlock(&c->client_data_mutex);
        return;
    }


    strncpy(c->message_queue[c->messaje_count], buff, MESSAGE_LENGTH - 1);
    c->message_queue[c->messaje_count][MESSAGE_LENGTH - 1] = '\0';
    c->messaje_count++;
    
    pthread_mutex_unlock(&c->client_data_mutex);
}


queue* create_queue(int capacity)
{
    //to do capacity e number_of_clients
    queue *q=(queue*)malloc(sizeof(queue));
    q->capacity=capacity;
    q->actual_size_of_queue=q->front=q->rear=0;
    memset(q->clients, 0, sizeof(client*) * NUMBER_OF_CLIENTS);
    pthread_cond_init(&(q->queue_is_empty),NULL);
    pthread_cond_init(&(q->queue_is_full),NULL);
    pthread_mutex_init(&(q->mutex_queue_empty),NULL);
    pthread_mutex_init(&(q->mutex_queue_full),NULL);

    return q;
}

void enqueue(queue* coada, client* client)
{
   // printf("Attempting to enqueue client %d\n", client->client_fd);
   // printf("Queue size before enqueue: %d\n", coada->actual_size_of_queue);
    pthread_mutex_lock(&(coada->mutex_queue_full));
    while(coada->actual_size_of_queue==coada->capacity)
    {
        pthread_cond_wait(&(coada->queue_is_full),&(coada->mutex_queue_full));
    }
    coada->clients[coada->rear]=client;
    coada->actual_size_of_queue++;
    coada->rear=(coada->rear+1)%coada->capacity;
    pthread_cond_signal(&(coada->queue_is_empty));
    pthread_mutex_unlock(&(coada->mutex_queue_full));
    //printf("Queue size after enqueue: %d\n", coada->actual_size_of_queue);

}
client* dequeue(queue* coada)
{
    pthread_mutex_lock(&(coada->mutex_queue_full));
    while(coada->actual_size_of_queue == 0 && !shutdown_flag)
    {
        pthread_cond_wait(&(coada->queue_is_empty), &(coada->mutex_queue_full));
        if (shutdown_flag) {
            pthread_mutex_unlock(&(coada->mutex_queue_full));
            return NULL;
        }
    }
    
    client* c = NULL;
    if (coada->actual_size_of_queue > 0 && coada->clients[coada->front] != NULL) {
        c = coada->clients[coada->front];
        coada->clients[coada->front] = NULL;
        coada->actual_size_of_queue--;
        coada->front = (coada->front + 1) % coada->capacity;
        pthread_cond_signal(&(coada->queue_is_full));
    }
    
    pthread_mutex_unlock(&(coada->mutex_queue_full));
    return c;
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
    queue *q = (queue *)params;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while(!shutdown_flag)
    {
        client* c = dequeue(q);
        if (c == NULL) continue;

        pthread_mutex_lock(&c->client_data_mutex);
        char filename[256];
        snprintf(filename, sizeof(filename), "/tmp/client_%d.log", c->client_id);
        int fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, 0664);
        if(fd < 0)
        {
            perror("eroare la creare fisier tmp pt client\n");
            pthread_mutex_unlock(&c->client_data_mutex);
            continue;
        }

        for(int i = 0; i < c->messaje_count; i++)
        {
            char temp_message[MESSAGE_LENGTH + 1];
            snprintf(temp_message, sizeof(temp_message), "%s\n", c->message_queue[i]);
            write(fd, temp_message, strlen(temp_message));
            printf("%s\n",c->message_queue[i]);
            if(c->is_in_epoll == 0)
            {
                struct epoll_event epll;
                epll.data.fd = c->client_fd;
                epll.events = EPOLLIN;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c->client_fd, &epll);
            }
        }
        close(fd);
        pthread_mutex_unlock(&c->client_data_mutex);
    }
    return NULL;
}

int set_signal_fd()
{
    //printf("setaza signal_fd\n");
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);
    sigaddset(&mask,SIGTERM);

    if(sigprocmask(SIG_BLOCK,&mask,NULL)<0)
    {
        perror("eroare la sigprocmask pt sigint\n");
        return -1;
    }

    int signal_fd=signalfd(-1,&mask,0);
    if(signal_fd<0)
    {
        perror("eroare la signal_fd\n");
        return -1;
    }
    return signal_fd;
}

void cleanup()
{
    printf("Cleanup inceput.\n");
    
    shutdown_flag = 1;

    pthread_mutex_lock(&coada->mutex_queue_full);
    pthread_cond_broadcast(&coada->queue_is_empty);
    pthread_mutex_unlock(&coada->mutex_queue_full);

    usleep(100000);

 
    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_cancel(thread_id[i]);
    }

    for(int i = 0; i < NUMBER_OF_THREADS; i++) {
        pthread_join(thread_id[i], NULL);
    }

    for(int i = 0; i < coada->capacity; i++) {
        if(coada->clients[i] != NULL) {
            if(coada->clients[i]->client_fd > 0) {
                close(coada->clients[i]->client_fd);
            }
            pthread_mutex_destroy(&coada->clients[i]->client_data_mutex);
            free(coada->clients[i]);
        }
    }

    pthread_mutex_destroy(&coada->mutex_queue_empty);
    pthread_mutex_destroy(&coada->mutex_queue_full);
    pthread_cond_destroy(&coada->queue_is_empty);
    pthread_cond_destroy(&coada->queue_is_full);

    if(epoll_fd > 0) close(epoll_fd);
    if(server_socket > 0) close(server_socket);
    if(signal_fd > 0) close(signal_fd);

    free(coada);
    
    printf("Cleanup finalizat.\n");
    _exit(0);
}

int main(){

    setvbuf(stdout,NULL,_IONBF,0);
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
    signal_fd=set_signal_fd();

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

    //setare signalfd
    
    if(signal_fd==-1)
    {
        perror("eroare la signalfd din main\n");
        exit(-1);
    }
    epoll_ev.data.fd=signal_fd;
    epoll_ev.events=EPOLLIN;
    rc=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,signal_fd,&epoll_ev);
    if(rc<0)
    {
        perror("eroare adaugare signalfd in epoll\n");
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
                    unsigned int size=sizeof(client_addr);
                    client_fd=accept(server_socket,(struct sockaddr*)(&client_addr),&size);
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

            }else if(ret_events[i].data.fd==signal_fd)
            {
                //aici se apeleaza functiile de cleanup
                printf("s-a primit signalfd\n");
                 struct signalfd_siginfo fdsi;
                 ssize_t bytes_read=read(signal_fd,&fdsi,sizeof(fdsi));
                 if(bytes_read<0)
                 {
                    perror("eroare la citire signalfd\n");
                 }

                cleanup();

            }else {
                client* c = get_client_from_queue(coada, ret_events[i].data.fd);
                if (!c) {
                    c = (client*)calloc(1, sizeof(client));
                    if (!c) {
                        perror("Failed to allocate client");
                        continue;
                    }
                    c->client_id=-1;
                    c->client_fd = ret_events[i].data.fd;
                    c->is_in_epoll = 1;
                    pthread_mutex_init(&c->client_data_mutex, NULL);
                    enqueue(coada, c);
                }

                char buffer[MESSAGE_LENGTH];
                int bytes_read = recv(c->client_fd, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes_read <= 0) {
                    if (bytes_read == 0 || errno == ECONNRESET) {
                        printf("Client %d disconnected\n", c->client_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, c->client_fd, NULL);
                    }
                    continue;
                }
                sscanf(buffer, "ID:%d", &c->client_id);
               // printf("clientul are id-ul : %d\n",c->client_id);
                //char*rest_message=strchr(buffer,' ');
                //if (rest_message) {
                // strcpy(buffer, rest_message + 1); 
                //} 
                buffer[bytes_read] = '\0';
                add_message_in_client_queue(coada, c, buffer);
            }
        }
    }




}