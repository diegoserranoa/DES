#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_FILE 200
#define KW_LENGTH 8

int result[64];
int currentBlock[64];
int destination[64];
unsigned char block[8];
char key[8];
int newKey[56];
int n;
int encrypt;

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "Cannot determine size of %s: %s\n",
            filename, strerror(errno));

    return -1;
}


int sbox[8][4][16] = {
{
	{14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
	{ 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
	{ 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
	{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
},
{
    {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
    { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
    { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
    {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
},
{
    {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
    {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
    {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
    {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
},
{
    {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
    {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
    {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
    {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
},
{
    {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
    {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
    {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
    {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
},
{
    {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
    {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
    {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
    {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
},
{
    {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
    {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
    {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
    {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
},
{
    {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
    {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
    {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
    {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
}
};

int binary_decimal(unsigned char n) /* Function to convert binary to decimal.*/

{
    int decimal=0, i=0, rem;
    while (n!=0)
    {
        rem = n%10;
        n/=10;
        decimal += rem*pow(2,i);
        i++;
    }
    return decimal;
}

unsigned char bitsToChar(int bits[8]){
	int resultado = 0;
	for (int i = 7, j = 0; i >= 0; i--, j++)
	{
		resultado |= bits[j] << i;
	}
	return resultado;
}

int bitAtPosition(unsigned char byte, int position){
	if (byte & (1 << position)){
		return 1;
	} else {
		return 0;
	}
}

void byteToArray(int byte, int array[8]){
	int aux = byte;
	for (int i = 0; i < 8; i++){
		if (aux & 128)
			array[i] = 1;
		else
			array[i] = 0;

		aux <<= 1;
	}
}

void printBits(unsigned char byte[8], int size){
	unsigned char aux = *byte;
	for (int i = (size*8) - 1; i >= 0; i--){
		 
		if (aux & 128)
			printf("1");
		else
			printf("0");

		aux <<= 1;
		if (i % 8 == 0)
		{
			byte++;
			aux = *byte;
		}
	}
	printf("\n");
	byte -= size;
}

void printArray(int array[], int size){
	for (int i = 0; i < size; i++)
	{
		printf("%i", array[i]);
	}
	printf("\n");
}

void getInput(char *question, char *inputBuffer, int bufferLength)
{
    printf("%s", question);
    fgets(inputBuffer, bufferLength, stdin);

    if (inputBuffer[strlen(inputBuffer) -1] != '\n') {
        int dropped = 0;
        while (fgetc(stdin) != '\n')
            dropped++;

        if (dropped > 0) {
            printf("Error: input too long.\n");
        }
    } else {
        inputBuffer[strlen(inputBuffer) -1] = '\0';      
    }
}

// funcion debuggeada, todo bien...
void initialPermutation(){
	// permutation table
	// e.g. bit 1 va a posicion 40 (39)
	// bit 2 va a posicion 8
	int initialPermutationTable[64] = {
		40, 8, 48, 16, 56, 24, 64, 32,
		39, 7, 47, 15, 55, 23, 63, 31,
		38, 6, 46, 14, 54, 22, 62, 30,
		37, 5, 45, 13, 53, 21, 61, 29,
		36, 4, 44, 12, 52, 20, 60, 28,
		35, 3, 43, 11, 51, 19, 59, 27,
		34, 2, 42, 10, 50, 18, 58, 26,
		33, 1, 41,  9, 49, 17, 57, 25
	};
	for (int i = 0; i < 64; i++){
		// colocar en la posicion initialPermutationTable[i] el bit currentBlock[i]
		result[initialPermutationTable[i] - 1] = currentBlock[i];

		//destination[initialPermutationTable[i] - 1] = result[i];
	}
}

// funcion debuggeada, todo bien...
void inversePermutation(){
	// inverse ermutation table
	// e.g. bit 1 va a posicion 58
	// bit 2 va a posicion 50
	int inversePermutationTable[64] = {
        58, 50, 42, 34, 26, 18, 10, 2,
        60, 52, 44, 36, 28, 20, 12, 4,
        62, 54, 46, 38, 30, 22, 14, 6,
        64, 56, 48, 40, 32, 24, 16, 8,
        57, 49, 41, 33, 25, 17,  9, 1,
        59, 51, 43, 35, 27, 19, 11, 3,
        61, 53, 45, 37, 29, 21, 13, 5,
        63, 55, 47, 39, 31, 23, 15, 7
    };
	for (int i = 0; i < 64; i++){
		// colocar en la posicion inversePermutationTable[i] el bit result[i]
		destination[inversePermutationTable[i] - 1] = result[i];

		//result[inversePermutationTable[i] - 1] = currentBlock[i];
	}
}

void transformBlock(){
	// pasar valores del byte a un arreglo
	int z = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 7; j >= 0; j--)
		{
			currentBlock[z] = bitAtPosition(block[i], j);
			z++;
		}
	}
}

void transformKey(){
	// ignore most significative bit
	//printBits((unsigned char *)key, 8);
	int j = 0;
	for (int z = 0; z < 8; z++)
	{
		//printf(" ");
		for (int i = 6; i >= 0; i--)
		{
			if (key[z] & (1 << i)){
				//printf("1");
				newKey[j] = 1;
			}else{
				//printf("0");
				newKey[j] = 0;
			}
			j++;
		}
	}
	//printf("\n");
}

void setArrays(int left[32], int right[32]){
	for (int i = 0; i < 32; i++)
	{
		left[i] = result[i];
		right[i] = result[i + 32];
	}
}

void setKeys(int left[28], int right[28]){
	for (int i = 0; i < 28; i++)
	{
		left[i] = newKey[i];
		right[i] = newKey[i + 28];
	}
}

void setSubkey(int subkey[56], int left[28], int right[28]){
	for (int i = 0; i < 28; i++)
	{
		subkey[i] = left[i];
		subkey[i + 28] = right[i];
	}
}

void expansion(int original[32], int expansion[48]){
	// El bloque de 32 bits se expande a uno de 48 bits. Esta funcion permuta el orden los
	// 32 bits originales y repite algunos para completar los 48 bits.
	// El objetivo de esta funcion es que el bloque de la derecha sea del mismo tamano que la llave (48)

	// Tablas de permutacion
	// bit 1 va a la posicion 2 y 48
	// bit 2 va a la posicion 8 
	int expansionPermutationTable[32] = {
		2, 3, 4, 5, 6, 9, 10, 11,
		12, 15, 16, 17, 18, 21, 22, 23,
		24, 27, 28, 29, 30, 33, 34, 35,
		36, 39, 40, 41, 42, 45, 46, 47
	};
	int expansionPermutationTableTwo[32] = {
		48, 0, 0, 7, 8, 0, 0, 13,
		14, 0, 0, 19, 20, 0, 0, 25,
		26, 0, 0, 31, 32, 0, 0, 37,
		38, 0, 0, 43, 44, 0, 0, 1
	};

	for (int i = 0; i < 32; i++)
	{
		expansion[expansionPermutationTable[i] - 1] = original[i];
		if (expansionPermutationTableTwo[i] != 0)
		{
			expansion[expansionPermutationTableTwo[i] - 1] = original[i];
		}
	}
}

void shiftKeyLeft(int key[28]){
	int shiftTable[16] = {
		1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
	};
	for (int j = 0; j < shiftTable[n]; j++)
	{
		int first = key[0];
		for (int i = 0; i < 27; i++)
		{
			key[i] = key[i + 1];
		}
		key[27] = first;
	}
	
}

void permuteSubkey(int roundSubkey[48], int subkey[56]){
	int subkeyPermutationTable[56] = {
		 5, 24,  7, 16,  6, 10, 20, 18,  0, 12,  3, 15, 23,  1,
		 9, 19,  2,  0, 14, 22, 11,  0, 13,  4,  0, 17, 21,  8,
		47, 31, 27, 48, 35, 41,  0, 46, 28,  0, 39, 32, 25, 44,
		 0, 37, 34, 43, 29, 36, 38, 45, 33, 26, 42,  0, 30, 40
	};
	for (int i = 0; i < 56; i++)
	{
		if (subkeyPermutationTable[i] != 0)
		{
			roundSubkey[subkeyPermutationTable[i] - 1] = subkey[i];
		}
	}
}

void keyMixing(int roundSubkey[48], int left[28], int right[28]){
	// En cada round, la llave de 56 bits se divide en 2 bloques de 28 bits
	int auxKey[56];
	for (int i = 0; i < 56; i++)
	{
		auxKey[i] = newKey[i];
	}

	// A cada bloque se le hace un shift hacia la izquierda dependiendo de la tabla. 
	shiftKeyLeft(left);
	shiftKeyLeft(right);

	// Se unen ambas partes para nuevamente formar un bloque de 56 bits
	setSubkey(auxKey, left, right);

	// A este nuevo bloque se le permutan 48 bits para usar como subllave en el round
	permuteSubkey(roundSubkey, auxKey);

	
}

void feistelFunction(int right[32], int extendedBlock[48]){
	// Primer paso de la funcion Feistel
	// Expansion
	expansion(right, extendedBlock);

	// Segundo paso
	// Revolver las llaves
	// dividir llave newKey[56] en dos bloques de 28
	int leftKey[28];
	int rightKey[28];
	int roundSubkey[48];
	setKeys(leftKey, rightKey);
	keyMixing(roundSubkey ,leftKey, rightKey);

	// Finalmente, se utiliza la subllave de 48 bits para aplicarle una
	// funcion con XOR a los 48 bits del bloque de la derecha
	
	for (int i = 0; i < 48; i++)
	{
		if (extendedBlock[i] == roundSubkey[i])
		{
			extendedBlock[i] = 0;
		} else {
			extendedBlock[i] = 1;
		}
	}
	
}

void substitution(int extendedBlock[48], int resultBlock[32]){
	// Las substituciones se hacen con 8 s-boxes. Un x-box360 convierte 6 bits en 4 bits.
	// Se recibe un bloque de 48 bits que se divide en 8 bloques de 6 bits.
	int z = 0;
	for (int j = 0; j < 8; j++)
	{
		int i = j * 6;
		unsigned char auxRow = 0x00;
		unsigned char auxCol = 0x00;

		// el primero y ultimo bit del bloque (1 y 6) se utilizan para determinar la fila
		auxRow |= (int)extendedBlock[i] << 1;
		auxRow |= (int)extendedBlock[i + 5] << 0;

		// los bits 2, 3, 4 y 5 se utilizan para determinar la columna
		auxCol |= (int)extendedBlock[i+1] << 3;
		auxCol |= (int)extendedBlock[i+2] << 2;
		auxCol |= (int)extendedBlock[i+3] << 1;
		auxCol |= (int)extendedBlock[i+4] << 0;
		int column = (int)auxCol & 0xff;
		int row = (int)auxRow & 0xff;
		// utilizando la x-box correspondiente se determinan los 4 bits nuevos
		int substitutedChar = sbox[j][row][column];
		for (int k = 3; k >= 0; k--)
		{
			// Cada bloque de 4 bits se utiliza para formar un nuevo bloque de 32 bits
			resultBlock[z] = (int)bitAtPosition(substitutedChar, k);
			z++;
		}
	}
	
}
void permutation(int permutationBlock[32], int finalBlock[32]){
	// los 32 bits resultantes de la substitucion se permutan con la siguiente table
	int permutationBoxP[32] = {
		 9, 17, 23, 31, 13, 28,  2, 18,
		24, 16, 30,  6, 26, 20, 10,  1,
		 8, 14, 25,  3,  4, 29, 11, 19,
		32, 12, 22,  7,  5, 27, 15, 21
	};
	for (int i = 0; i < 32; i++)
	{
		finalBlock[permutationBoxP[i] - 1] = permutationBlock[i];
	}
}

void rounds(){
	// permutacion inicial del bloque de 64 bits
	initialPermutation();

	// empezar con los 16 rounds sobre el bloque
	int left[32];
	int right[32];
	int final[32];
	// primero hay que dividir el bloque en dos de 32 bits
	setArrays(left, right);

	for (int i = 0; i < 16; i++)
	{
		// n = el round
		if (encrypt)
		{
			n = i;
		} else {
			n = 16 - i - 1; 
		}
		int extendedBlock[48];
		int lastBlock[32];
		
		// Aplicar la funcion Feistel al bloque de la derecha
		// guardar resultado en extendedBlock el cual es de 48 bits
		feistelFunction(right, extendedBlock);

		// Tercer Paso
		// Substitucion
		substitution(extendedBlock, lastBlock);

		// Cuarto Paso
		// Permutacion
		permutation(lastBlock, final);

		// Se aplica la operacion XOR del bloque izquierdo con el bloque derecho resultante
		for (int j = 0; j < 32; j++)
		{
			if (final[j] == left[j])
			{
				final[j] = 0;
			} else {
				final[j] = 1;
			}
		}

		// El bloque derecho original pasa a ser el siguiente bloque izquierdo
		for (int j = 0; j < 32; j++)
		{
			left[j] = right[j];
		}

		// El nuevo bloque derecho es el resultado obtenido del ultimo XOR
		for (int j = 0; j < 32; j++)
		{
			right[j] = final[j];
		}
		
	}
	

	// despues de los 16 rounds se juntan ambos bloques para crear el resultante de 64 bits
	for (int i = 0; i < 32; i++)
	{
		result[i] = right[i];
		result[i + 32] = left[i];
	}

	// final permutation
	// (inverse initial permutation)
	inversePermutation();
}

void cipherFile(char *sourceFileName, char* destinationFileName){
	FILE *sourceFile;
	FILE *destinationFile;

	unsigned char header[54];

	// checar si tamano de archivo es multiplo de 8
	// en caso de no serlo, agregar los bytes que falten
	
	int overhead = (fsize(sourceFileName)- 54) % 8;
	if (overhead != 0)
	{
		printf("Overhead: %i\n", overhead);
		if ((sourceFile = fopen(sourceFileName, "a")) != NULL){
			for (int i = 0; i < overhead; i++)
			{
				printf("Inserted overhead.\n");
				char stuffing = 0xFF;
				fwrite(&stuffing, 1, 1, sourceFile);
			}
			fclose(sourceFile);
		} else {
			printf("Error: could not open file %s. Check if the file exists.\n", sourceFileName);
		}
	}
	if ((sourceFile = fopen(sourceFileName, "r+b")) != NULL)	
	{
		if ((destinationFile = fopen(destinationFileName, "w+b")) != NULL)
		{
			// No modificar los primeros 54 bytes del archivo
			fread(header, 1, 54, sourceFile);
			fwrite(&header, 1, 54, destinationFile);

			// leer en bloques de 8 bytes (64 bits)
			
			while (fread(block, 1, 8, sourceFile)){

				transformBlock();
				// empezar los "rounds"
				rounds();

				// transformar a tipo char para poder escribir en el archivo
				int bit = 7;
				int byte[8];
				for (int i = 63; i >= 0; i--)
				{
					if (i % 8 != 0)
					{
						byte[i % 8] = destination[i];
					} else {
						byte[i % 8] = destination[i];
						block[bit] = bitsToChar(byte);
						bit--;
					}
				}

				// write on the destination file
				fwrite(&block, 1, 8, destinationFile);	
			}
			fclose(destinationFile);
		} else {
			printf("Error: Unable to create new file.\n");
		}
		fclose(sourceFile);
	} else {
		printf("Error: could not open file %s. Check if the file exists.\n", sourceFileName);
	}
}


int main(int argc, char *argv[])
{
	char sourceFileName[MAX_FILE];
	char destinationFileName[MAX_FILE];
	(void) argv;

	if (argc == 2)
	{
		//request source file from user
    	//getInput("Source file name: ", sourceFileName, MAX_FILE);
		if (*argv[1] == 'e')
		{
			encrypt = 1;
			printf("Encrypt\n");
			// recibir archivo fuente
			printf("source: ");
			fgets(sourceFileName, MAX_FILE, stdin);
			strtok(sourceFileName, "\n");

			// recibir archivo destino
			printf("destination: ");
			fgets(destinationFileName, MAX_FILE, stdin);
			strtok(destinationFileName, "\n");

			// recibir llave
			printf("keyword: ");
			fgets(key, KW_LENGTH + 1, stdin);
			strtok(key, "\n");

			// quitar bit mas significativo de cada byte
			transformKey();

			cipherFile(sourceFileName, destinationFileName);
		} else if(*argv[1] == 'd') {
			encrypt = 0;
			printf("Decrypt\n");

			// recibir archivo fuente
			printf("source: ");
			fgets(sourceFileName, MAX_FILE, stdin);
			strtok(sourceFileName, "\n");

			// recibir archivo destino
			printf("destination: ");
			fgets(destinationFileName, MAX_FILE, stdin);
			strtok(destinationFileName, "\n");

			// recibir llave
			printf("keyword: ");
			fgets(key, KW_LENGTH + 1, stdin);
			strtok(key, "\n");

			// quitar bit mas significativo de cada byte
			transformKey();
			cipherFile(sourceFileName, destinationFileName);
		}
    	
	}
	return 0;
}