/**
 * @file   testebbchar.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the ebbchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/ebbchar.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int cifragen(char ParaCifrar[BUFFER_LENGTH]);

int main(int argc, char * argv[]){
     
   char opcao[1];
   strcpy( opcao, argv[1] );
   printf("Opcao:%s \n", opcao);

   if (argc != 3){
	   printf("Quatidade de Argumentos Errada\n");
      exit(0);
   }
      switch(opcao[0]){
         case 'c':
            printf("Cifrar\n");
            cifragen(argv[2]);
            break;
         case 'd':
            printf("Decifrar\n");
            break;
         case 'h':
            printf("Historico\n");
            break;
         default:
            printf("Opcao invalida\n");
            exit(0);
      }
}


int cifragen(char paraCifrar[BUFFER_LENGTH]){
   int ret, fd;

   printf("Iniciando programa de cifragem de dados...\n");
   fd = open("/dev/PROJ1", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Falha ao abrir o Arquivo do dispositivo...");
      return errno;
   }
   //printf("Type in a short string to send to the kernel module:\n");
   //scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)

   printf("Mensagem escrita no Arquivo do dispositivo [ %s ].\n", paraCifrar);
   ret = write(fd, paraCifrar, strlen(paraCifrar)); // Send the string to the LKM
   if (ret < 0){
      perror("Falha ao ler a mensagem do dispositivo.");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   getchar();

   printf("Lendo do Arquivo do dispositivo...\n");
   ret = read(fd, receive, BUFFER_LENGTH);        // Read the response from the LKM
   if (ret < 0){
      perror("Falha ao ler a mensagem do dispositivo.");
      return errno;
   }
   printf("A mensagem recebida foi: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
   }
