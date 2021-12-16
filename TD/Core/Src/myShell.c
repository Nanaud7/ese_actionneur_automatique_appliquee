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
#define SHELL_CMD_MAX_SIZE 16
#define ARGC_MAX 8
#define BUFFER_SIZE 40
/* End of macros -------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
/* End of constants ----------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
UART_HandleTypeDef* shell_huart = NULL;
char prompt[] = "@Nucleo-G431 >> ";

char c = 0;
uint8_t pos = 0;
static char buf[BUFFER_SIZE];
static char backspace[] = "\b \b";

static int shell_func_list_size = 0;
static shell_func_t shell_func_list[SHELL_FUNC_LIST_MAX_SIZE];

/* End of variables ----------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**
 * Fonction indispensable pour utiliser printf() sur la liaison uart
 * @param ch Caractère à écrire sur la liaison uart
 * @return Caractère écrit sur la liaison uart
 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&SHELL_UART_DEVICE, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

/**
 * Écriture sur la liaison uart
 * @param s Chaîne de caractères à écrire sur la liaison uart
 * @param size Longueur de la chaîne de caractère
 * @return size
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
		printf("%s : %s\r\n", shell_func_list[i].cmd, shell_func_list[i].description);
	}
	return 0;
}

/**
 * @brief	Initialisation du shell
 */
void shell_init() {
	printf("\r\n\r\n===== Shell =====\r\n");
	uart_write(prompt,sizeof(prompt));

	HAL_UART_Receive_IT(&SHELL_UART_DEVICE, (uint8_t*)&c, 1);

	shell_add("help", sh_help, (char *)"help");

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
		//printf(":%s\r\n", buf);
		pos = 0;
		shell_exec(buf);
		uart_write(prompt,sizeof(prompt));
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
	int argc;
	char * argv[ARGC_MAX];
	char *p;

	// Séparation du header et des paramètres
	char header[SHELL_CMD_MAX_SIZE] = "";
	int h = 0;

	while(cmd[h] != ' ' && h < SHELL_CMD_MAX_SIZE){
		header[h] = cmd[h];
		h++;
	}
	header[h] = '\0';

	// Recherche de la commande et paramètres
	for(int i = 0 ; i < shell_func_list_size ; i++) {
		if (!strcmp(shell_func_list[i].cmd, header)) {
			argc = 1;
			argv[0] = cmd;

			for(p = cmd ; *p != '\0' && argc < ARGC_MAX ; p++){
				if(*p == ' ') {
					*p = '\0';
					argv[argc++] = p+1;
				}
			}

			return shell_func_list[i].func(argc, argv);
		}
	}
	printf("%s: command not found\r\n", cmd);
	return -1;
}

/* End of functions ----------------------------------------------------------*/
