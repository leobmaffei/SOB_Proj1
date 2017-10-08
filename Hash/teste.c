/**
 * @file   Proj1.c
 * @author Grupo SOb
 * @date   3 Outubro 2017
 * @version 0.1
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static unsigned char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM


int main(int argc, char * argv[]){
   
   int ret, fd, a = 0;
   char paraCifrar[BUFFER_LENGTH];
   char opcao[BUFFER_LENGTH];
   strcpy( opcao, argv[1] );
   printf("Opcao Escolhida:%s \n", opcao);

   if (argc != 3){
	   printf("Quatidade de Argumentos Errada\n");
      exit(0);
   }
   switch(opcao[0]){
      case 'c':
         printf("### CIFRAR ###\n");
         strcat(opcao, argv[2]);
         strcpy(paraCifrar, opcao);
         break;
      case 'd':
         printf("### DECIFRAR ###\n");
         strcat(opcao, argv[2]);
         strcpy(paraCifrar, opcao);
         break;
      case 'h':
         printf("### CALCULO DO HASH ###\n");
         strcat(opcao, argv[2]);
         strcpy(paraCifrar, opcao);
         break;
      default:
         printf("Opcao invalida\n");
         exit(0);
   }

   //system("insmod Proj1.ko key=\"chave\"");

   printf("Iniciando programa de cifragem de dados...\n");
   fd = open("/dev/PROJ1", O_RDWR);             // Open the device with read/write access
   if (fd < 0){
      perror("Falha ao abrir o Arquivo do dispositivo...");
      return errno;
   }
   
   if (opcao[0] == 'c'){

   }else if (opcao[0] == 'd'){

   }else if(opcao[0] == 'h'){

   }
   printf("Mensagem escrita no Arquivo do dispositivo [ %s ].\n", argv[2]);
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
   printf("tamanho da resposta %zu \n", strlen(receive));
   printf("A mensagem recebida foi: [ ");
   for(a = 0; a < strlen(receive); a++){
            printf("%X", receive[a]);
   }
   printf(" ]\n");
   printf("Fim do programa.\n");
   return 0;
}

