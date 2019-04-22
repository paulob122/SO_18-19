#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "global.h"
#include "artigos.h"

//DEPOIS REMOVER ESTA FUNÇÃO DAQUI VISTO QUE ESTÁ A MAIS

//Retorna a ultima linha onde foi inserido
int inserirArtigo (char* nome, double preco) {

	//Posicao onde vai ser inserido no ficheiro de strings
	int index = linhasFicheiro(PATH_STRINGS);

	//Abrir ambos os ficheiros
	//Não é preciso append pois faço lseek
	//Faço lseek para saber o valor que retorna, ou seja, o offset
	//...e guardo no index
	int fd_artigo = open (PATH_ARTIGOS, O_WRONLY, 0666);
	int fd_string = open (PATH_STRINGS, O_APPEND|O_WRONLY, 0666);

	char buffer[MAX_LINE];

	if (fd_artigo != EOF && fd_string != EOF) {

		//Reposicionar o ficheiro de artigos no fim		
		lseek(fd_artigo, 0, SEEK_END);

		//Guardar no buffer o novo artigo e a sua posição
		//o index representa o offset desde o inicio do ficheiro ate ao inicio da string do produto
		//A segunda parte é o preço do artigo
		sprintf(buffer, "%d %.2lf\n", index, preco);
		if (write(fd_artigo, buffer, strlen(buffer)) != -1);

		char nameBuffer[MAX_LINE];
		sprintf(nameBuffer, "%s\n", nome);
		//Acrescentar o novo nome ao fim do ficheiro strings
		if (write(fd_string, nameBuffer, strlen(nameBuffer)) != -1);
	}

	//Fechar os fd's dos artigos
	close(fd_artigo);
	close(fd_string);

	//Linha onde foi inserido
	return index;
}

char* get_line_from_file (char *path, int line_to_search) {

	int fd_file = open(path, O_RDONLY, 0666);

	int n = 0, curr_line = 1, r = 0;
 	
 	//Linha lida
	char buffer[MAX_LINE];

	do {

 		n = readln(fd_file, buffer, MAX_LINE);
		
 		if (curr_line == line_to_search) {
 			r = 1;
 			break;
 		} 

 		curr_line++;

	} while (n > 0); 	

	char *res;

	if (!r) 
		res = NULL;
	else { 
		res = strdup(buffer);
	}
	
	return res;
}

int main(int argc, char const *argv[]) {
	
	int fd_artigos = open(PATH_ARTIGOS, O_RDONLY, 0666);

	GHashTable *stringsCompact = g_hash_table_new(g_str_hash, g_str_equal);

	int n = 0;
 	
 	//Linha lida
	char buffer[MAX_LINE];
	//Referencia na linha
	char *key_ref;
	//Preco lido
	char *preco;

 	//Ler a primeira linha
	n = readln(fd_artigos, buffer, MAX_LINE);
 	
 	//Ler o resto das linhas
 	do {

 		ARTIGO novo = initArtigo(novo);

 		//Sacar o preco e referencia (para o ficheiro strings)
 		key_ref = strdup(strtok(buffer, " "));
 		preco   = strdup(strtok(NULL  , " "));

 		novo = insertArtigo(novo, key_ref, 
 								  get_line_from_file(PATH_STRINGS, atoi(key_ref)),
 								  atof(preco));

 		//write_artigo_to_stdout(novo);
 		
 		//Inserir o artigo na hashtable
 		g_hash_table_insert(stringsCompact, key_ref, novo);
 		
 		n = readln(fd_artigos, buffer, MAX_LINE);
 	
	} while (n > 0);

	close(fd_artigos);

	//-------------------------------------------------------------------

	//Limpar o ficheiro de artigos
	fd_artigos = open(PATH_ARTIGOS, O_CREAT|O_TRUNC|O_WRONLY, 0666);
	close(fd_artigos);

	//Limpar o ficheiro de strings
	int fd_strings = open(PATH_STRINGS, O_CREAT|O_TRUNC|O_WRONLY, 0666);
	close(fd_strings);

	GList *values = NULL, *auxValues = NULL;

	values = g_hash_table_get_values(stringsCompact);

	for (auxValues = values; auxValues != NULL; auxValues = auxValues -> next) {

		char *nome_art = strdup(getNome(auxValues -> data));
		nome_art[strlen(nome_art)-1]='\0';

		inserirArtigo(nome_art, getPreco(auxValues -> data));
	}

	return 0;
}