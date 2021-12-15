/**
 ******************************************************************************
 * @file	myShell.c
 * @author 	Arnaud CHOBERT
 * @brief	Shell
 ******************************************************************************
 */

#include "myShell.h"

/* Types ---------------------------------------------------------------------*/
typedef struct{
	char * cmd;
	int (* func)(int argc, char ** argv);
	char * description;
} shell_func_t;
/* End of types --------------------------------------------------------------*/

/* Macros --------------------------------------------------------------------*/
#define SHELL_UART_DEVICE hlpuart1
#define SHELL_FUNC_LIST_MAX_SIZE 16
#define ARGC_MAX 8
#define BUFFER_SIZE 40
/* End of macros -------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
char help[] = "help";

char c = 0;
uint8_t pos = 0;
static char buf[BUFFER_SIZE];
static char backspace[] = "\b \b";

static int shell_func_list_size = 0;
static shell_func_t shell_func_list[SHELL_FUNC_LIST_MAX_SIZE];

/* End of variables ----------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 *	@brief	Fonction pour pouvoir utiliser le printf() sur la liaison uart
 *	@param	Caractère à écrire sur la liaison uart
 *	@retval	Caractère écrit sur la liaison uart
 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&SHELL_UART_DEVICE, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/**
 * @brief	Écriture sur la liaison uart
 * @param	s Chaîne de caractères à écrire sur la liaison uart
 * @param	size Longueur de la chaîne de caractère
 * @retval	size
 */
int uart_write(char *s, uint16_t size) {
	HAL_UART_Transmit(&SHELL_UART_DEVICE, (uint8_t*)s, size, 0xFFFF);
	return size;
}

/**
 * @brief	Affichage du menu d'aide sur le shell
 * @param	argc
 * @param	argv
 * @retval	0
 */
int sh_help(int argc, char ** argv) {
	int i;
	for(i = 0 ; i < shell_func_list_size ; i++) {
		printf("%s %s\r\n", shell_func_list[i].cmd, shell_func_list[i].description);
	}
	return 0;
}

/**
 * @brief	Initialisation du shell
 */
void shell_init() {
	printf("\r\n\r\n===== Shell =====\r\n");
	HAL_UART_Receive_IT(&SHELL_UART_DEVICE, (uint8_t*)&c, 1);
	//uart_write(prompt,sizeof(prompt));

	shell_add("help", sh_help, help);

	for (int i = 0 ; i < 3 ; i++) {

		HAL_Delay(200);
	}
}

/**
 *	@brief	Ajouter une commande dans la liste des commandes
 *	@param	c
 *	@param
 *	@param
 *	@retval	0
 */
int shell_add(char * cmd, int (* pfunc)(int argc, char ** argv), char * description) {
	if (shell_func_list_size < SHELL_FUNC_LIST_MAX_SIZE) {
		shell_func_list[shell_func_list_size].cmd = cmd;
		shell_func_list[shell_func_list_size].func = pfunc;
		shell_func_list[shell_func_list_size].description = description;
		shell_func_list_size++;
		return 0;
	}

	return -1;
}

/**
 *	@brief	Traitement d'un caractère reçu
 */
void shell_char_received() {

	switch (c) {

	case '\r':
		// Enter
		printf("\r\n");
		buf[pos++] = 0;
		printf(":%s\r\n", buf);
		pos = 0;
		shell_exec(buf);
		break;

	case '\b':
		// Delete
		if (pos > 0) {
			pos--;
			uart_write(backspace, 3);
		}
		break;

	default:
		if (pos < BUFFER_SIZE) {
			uart_write(&c, 1);
			buf[pos++] = c;
		}
	}
}

/**
 *	@brief	Execution d'une commande du shell
 *	@param	cmd
 *	@retval
 */
int shell_exec(char * cmd) {

	// Initialisation du tableau des paramètres
	char argv[ARGC_MAX][16];
	char * p_argv[ARGC_MAX];


	for(int i = 0; i < ARGC_MAX; i++){
		for(int j = 0; j < 16; j++){
			argv[i][j] = 0;
		}

		if(i == 0){
			p_argv[i] = argv[0];
		}
		else{
			p_argv[i] = p_argv[i-1] + 16;
		}
	}


	// Séparation de la commande et des paramètres
	int tmp_i = 0;
	int cmd_i = 0;
	int argc = 0;
	char tmpbuf[16] = "";

	for(char *p = cmd ; *p != '\0' && argc < ARGC_MAX ; p++){
		if(*p == ' ') {
			tmpbuf[tmp_i] = '\0';
			strcpy(argv[argc],tmpbuf);
			memset(tmpbuf, 0, strlen(tmpbuf));
			tmp_i = 0;
			cmd_i++;
			argc++;
		}
		else{
			tmpbuf[tmp_i] = cmd[cmd_i];
			tmp_i++;
			cmd_i++;
			printf("tmpbuf = %s\r\n", tmpbuf);
		}
	}

	tmpbuf[tmp_i] = '\0';
	strcpy(argv[argc],tmpbuf);

	// Recherche de la fonction à exécuter
	printf("Recherche de la fonction a executer\r\n");

	int i = 0;
	int ret_cmp = 1;

	while(i < shell_func_list_size && ret_cmp != 0){
		ret_cmp = strcmp(argv[0],shell_func_list[i].cmd);
		i++;
	}

	// Execution de la commande
	if(ret_cmp == 0){
		return shell_func_list[i].func(argc, p_argv);
	}

	printf("Commande inconnu\r\n");
	return -1;

}

/* End of functions ----------------------------------------------------------*/
