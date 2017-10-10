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
	printf("\n\n");
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

	char hexstring[32] =  "teste", *pos = hexstring;
    strcpy(hexstring, argv[2]);
	int tam = strlen(argv[2]);
	unsigned char val[32];
	memset(val,0,sizeof(val));
    size_t count = 0;
	int a =0;

	printf("String inicial %s\n", hexstring);
     /* WARNING: no sanitization or error-checking whatsoever */
    for(count = 0; count < sizeof(val)/sizeof(val[0]); count++) {
	
	a++;
        sscanf(pos, "%2hhx", &val[count]);
	pos += 2;
    }
	val[a] = '\0';
	strcat(opcao, val);
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
   printf("Tamanho da resposta %zu \n", strlen(receive));
	printf("A mensagem recebida foi: #HEXA# [ ");

	for(a = 0; a < strlen(receive); a++){
            printf("%X", receive[a]);
   }
   printf(" ]\n");

	printf("A mensagem recebida foi: #STRING# [ %s ]\n", receive);
   printf("Fim do programa.\n");
	printf("\n\n");
   return 0;
}

