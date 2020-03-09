/*
 * Includes Necessarios
 */
#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>		/* for printf() */
#include <unistd.h>
#include <stdlib.h>		/* for fork() */
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>		/* for wait() */
#include <sys/ipc.h>			/* for msgget(), msgctl() */
#include <sys/msg.h>			/* for msgget(), msgctl() */

/*
 * NO_OF_ITERATIONS corresponde ao numero de mensagens que serao enviadas.
 * Se este numero cresce, o tempo de execucao tambem deve crescer.
 */
#define NO_OF_ITERATIONS	500

/*
 * MICRO_PER_SECOND define o numero de microsegundos em um segundo
 */
#define MICRO_PER_SECOND	1000000

/*
 * MESSAGE_QUEUE_ID eh uma chave arbitraria, foi escolhido um numero qualquer,
 * que deve ser unico. Se outra pessoa estiver executando este mesmo programa
 * ao mesmo tempo, o numero pode ter que ser mudado!
 */
#define MESSAGE_QUEUE_ID	3102

/*
 * Constantes
 */
#define SENDER_DELAY_TIME	10
#define MESSAGE_MTYPE		1
#define NO_OF_CHILDREN 		2

/*
 * Filhos
 */
void Receiver(int queue_id);

void Sender(int queue_id);

int main( int argc, char *argv[] )
{

        int rtn = 1;
        int count = 10;

        /* 
         * Variaveis relativas a fila, id e key
         */
        int queue_id;
        key_t key = MESSAGE_QUEUE_ID;

        /*
         * Cria a fila de mensagens
         */
        if( (queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		/*
		 * Inicializa dois filhos
		 */

		for( count = 0; count < NO_OF_CHILDREN; count++) {
			if( rtn != 0 ) {
				rtn = fork();
			} else {
				break;
			}
		}

		/*
		 * Verifica o valor retornado para determinar se o processo eh pai ou filho
		 *
		 * OBS:  o valor de count no loop anterior indicara cada um dos filhos
		 *       count = 1 para o primeiro filho, 2 para o segundo, etc.
		 */
		if( rtn == 0 && count == 1 ) {

			/*
			 * Sou o primeiro filho me preparando para receber uma mensagem
			 */
                printf("Receptor iniciado ...\n");
                Receiver(queue_id);
                exit(0);

		} else if( rtn == 0 && count == 2 ) {
			/*
                   	 * Sou o segundo filho me preparando para enviar uma mensagem
			 */
                printf("Emissor iniciado ...\n");
                Sender(queue_id);
                exit(0);

		} else {
			/*
			 * Sou o pai aguardando meus filhos terminarem
			 */
                  printf("Pai aguardando ...\n");
			  wait(NULL);
			  wait(NULL);

            /*
             * Removendo a fila de mensagens
             */
            if( msgctl(queue_id,IPC_RMID,NULL) == -1 ) {
				fprintf(stderr,"Impossivel remover a fila!\n");
				exit(1);
			}
	    /*
	     * Pergunta 7: O que ocorre com a fila de mensagens, se ela n�o � removida e os
	     * processos terminam?
 	     */
            exit(0);
		}
}

/*
 * O tipo de dados seguinte pode ser usado para declarar uma estrutura que
 * contera os dados que serao transferidos pela fila.  A estrutura vai conter 
 * um numero de mensagem (msg_no) e o tempo de envio (send_time).  Para filas 
 * de mensagens, o tipo da estrutura pode definir qualquer dado que seja necessario.
 */
typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
} data_t; 

/* 
 * O conteudo de uma estrutura com o seguinte tipo de dados sera enviado 
 * atraves da fila de mensagens. O tipo define dois dados.  O primeiro eh
 * o tipo da mensagem (mtype) que sera como uma identificacao de mensagem. 
 * Neste exemplo, o tipo eh sempre o mesmo. O segundo eh um vetor com tamanho
 * igual ao definido pelo tipo declarado anteriormente. Os dados a serem 
 * transferidos sao colocados nesse vetor. Na maioria das circunstancias,
 * esta estrutura nao necessita mudar.
 */
typedef struct {
	long mtype;
	char mtext[sizeof(data_t)];
} msgbuf_t;

/*
 * Esta funcao executa o recebimento das mensagens
 */
void Receiver(int queue_id)
{
	/*
	 * Variaveis locais
	 */
	int count;
	struct timeval receive_time;
	float delta;
	float max;
	float total;

	/*
	 * Este eh o buffer para receber a mensagem
	 */
	msgbuf_t message_buffer;

	/*
	 * Este e o ponteiro para os dados no buffer.  Note
	 * como e setado para apontar para o mtext no buffer
	 */
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	//Pergunta 8: Qual ser� o conte�do de data_ptr?	

	/*
	 * Inicia o loop
	 */
	for( count = 0; count < NO_OF_ITERATIONS;count++ ) {
		/*
		 * Recebe qualquer mensagem do tipo MESSAGE_MTYPE
		 */
		if( msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MESSAGE_MTYPE,0) == -1 ) {
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

		/*
		 * Chama gettimeofday()
		 */
		gettimeofday(&receive_time,NULL);

		/*
		 * Calcula a diferenca
		 */
            	delta = receive_time.tv_sec  - data_ptr->send_time.tv_sec;
            	delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec)/(float)MICRO_PER_SECOND;
			total += delta;

		/*
		 * Salva o tempo maximo
		 */
		if( delta > max ) {
			max = delta;
		}
	}

	/*
	 * Exibe os resultados
	 */
	printf("O tempo medio de transferencia: %.10f\n",(total / NO_OF_ITERATIONS) );
	printf("O tempo maximo de transferencia: %.6f\n", max );

    return;
}

/*
 * Esta funcao envia mensagens
 */
void Sender(int queue_id)
{
	/*
	 * Variaveis locais
	 */
	int count;
	struct timeval send_time;

	/*
	 * Este e o buffer para as mensagens enviadas
	 */
	msgbuf_t message_buffer;

	/*
	 * Este e o ponteiro para od dados no buffer.  Note
	 * como e setado para apontar para mtext no buffer
	 */
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	/*
	 * Inicia o loop
	 */
	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		/*
		 * Chama gettimeofday()
		 */
		gettimeofday(&send_time,NULL);

		/*
		 * Apronta os dados
		 */
		message_buffer.mtype = MESSAGE_MTYPE;
		data_ptr->msg_no = count;
		data_ptr->send_time = send_time;

		/*
		 * Envia a mensagem... usa a identificacao da fila, um ponteiro
		 * para o buffer, e o tamanho dos dados enviados
		 */
		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}

		/*
		 * Dorme por um curto espaco de tempo 
          	 */
		usleep(SENDER_DELAY_TIME);
	}
        return;
}
