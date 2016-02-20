#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE   "Game Genie Encoder/Decoder"
#define VERSION "1.0.1"

#define ACTION_ENCODE  0x01
#define ACTION_DECODE  0x02
#define ACTION_VERBOSE 0x04

#define TABLE_LENGTH 16

static char table[TABLE_LENGTH] = "APZLGITYEOXUKSVN";

static int encode(char **code, char *address, char *data, char *compare) {
	int rc = 0, i = 0, l = 0;
	int address_int = 0, data_int = 0, compare_int = 0;
	int n[8];
	size_t length = 0;
	char code_str[9];

	sscanf(address, "0x%04X", &address_int);
	sscanf(data, "0x%02X", &data_int);

	if (address_int < 0x8000 || address_int > 0xffff) {
		printf("Address %s is out of bounds\n", address);

		rc = 1;
		goto cleanup;
	}

	if (data_int < 0x00 || data_int > 0xff) {
		printf("Data %s is out of bounds\n", data);

		rc = 1;
		goto cleanup;
	}

	length = 6;

	if (compare != NULL) {
		sscanf(compare, "0x%02X", &compare_int);

		if (compare_int < 0x00 || compare_int > 0xff) {
			printf("Compare %s is out of bounds\n", compare);

			rc = 1;
			goto cleanup;
		}

		length = 8;
	}

	n[0] = ((data_int >> 4) & 8) | (data_int & 7);
	n[1] = ((address_int >> 4) & 8) | ((data_int >> 4) & 7);
	n[2] = 8 | ((address_int >> 4) & 7);
	n[3] = (address_int & 8) | ((address_int >> 12) & 7);
	n[4] = ((address_int >> 8) & 8) | (address_int & 7);

	if (length == 6) {
		n[5] = (data_int & 8) | ((address_int >> 8) & 7);
	} else {
		n[5] = (compare_int & 8) | ((address_int >> 8) & 7);
		n[6] = ((compare_int >> 4) & 8) | (compare_int & 7);
		n[7] = (data_int & 8) | ((compare_int >> 4) & 7);
	}

	for (i = 0, l = (int)length; i < l; i++) {
		code_str[i] = table[n[i]];
	}

	code_str[(int)length] = '\0';

	*code = malloc(sizeof(char) * (length + 1));

	strcpy(*code, code_str);

cleanup:
	return rc;
}

static int decode(char *code, char **address, char **data, char **compare) {
	int rc = 0, found = 0, i = 0, j = 0, k = 0, l = 0;
	int address_int = 0, data_int = 0, compare_int = 0;
	int n[8];
	size_t length = 0;

	length = strlen(code);

	for (i = 0, l = (int)length; i < l; i++) {
		if (code[i] > 'Z') {
			code[i] -= 32;
		}
	}

	if (length != 6 && length != 8) {
		printf("Incorrect code length %zu\n", length);

		rc = 1;
		goto cleanup;
	}

	for (i = 0, j = (int)length; i < j; i++) {
		found = 0;

		for (k = 0, l = TABLE_LENGTH; k < l; k++) {
			if (code[i] == table[k]) {
				n[i] = k;
				found = 1;
			}
		}

		if (found == 0) {
			printf("Invalid character %c\n", code[i]);
			rc = 1;
			goto cleanup;
		}
	}

	*address = malloc(sizeof(char) * 7);
	*data = malloc(sizeof(char) * 5);
	*compare = malloc(sizeof(char) * 5);

	address_int = 0x8000 +
		(((n[3] & 7) << 12) |
		 ((n[5] & 7) << 8) |
		 ((n[4] & 8) << 8) |
		 ((n[2] & 7) << 4) |
		 ((n[1] & 8) << 4) |
		 (n[4] & 7) |
		 (n[3] & 8));

	sprintf(*address, "0x%04X", address_int);

	if (length == 6) {
		data_int = ((n[1] & 7) << 4) |
			((n[0] & 8) << 4) |
			(n[0] & 7) |
			(n[5] & 8);
	} else {
		data_int = ((n[1] & 7) << 4) |
			((n[0] & 8) << 4) |
			(n[0] & 7) |
			(n[7] & 8);
	}

	sprintf(*data, "0x%02X", data_int);

	if (length == 8) {
		compare_int = ((n[7] & 7) << 4) |
			((n[6] & 8) << 4) |
			(n[6] & 7) |
			(n[5] & 8);

		sprintf(*compare, "0x%02X", compare_int);
	} else {
		*compare = NULL;
	}

cleanup:
	return rc;
}

static void usage(char *exec) {
	int length = 0;

	length = (int)strlen(exec) + 7;

	printf("%s (v%s)\n\n", TITLE, VERSION);

	printf("Usage: %s [-e,--encode <address> <data> (<compare>)]\n", exec);
	printf("%*s [-d,--decode <code>]\n", length, "");
	printf("%*s [-v,--verbose]\n", length, "");
}

int main(int argc, char *argv[]) {
	int rc = 0, flags = 0, index = 0;
	char *exec = NULL, *action = NULL;
	char *address_input = NULL, *data_input = NULL, *compare_input = NULL;
	char *address_output = NULL, *data_output = NULL, *compare_output = NULL;
	char *code_input = NULL, *code_output = NULL;

	exec = argv[0];

	if (argc < 2) {
		usage(exec);

		rc = 1;
		goto cleanup;
	}

	index = 1;

	while (index < argc) {
		action = argv[index];

		if (strcmp(action, "-e") == 0 || strcmp(action, "--encode") == 0) {
			flags |= ACTION_ENCODE;

			if (index + 2 >= argc) {
				usage(exec);

				rc = 1;
				goto cleanup;
			}

			address_input = argv[index + 1];

			if (strncmp(address_input, "-", 1) == 0) {
				printf("Invalid address %s\n", address_input);

				rc = 1;
				goto cleanup;
			}

			data_input = argv[index + 2];

			if (strncmp(data_input, "-", 1) == 0) {
				printf("Invalid data %s\n", data_input);

				rc = 1;
				goto cleanup;
			}

			if (index + 3 < argc && strncmp(argv[index + 3], "-", 1) != 0) {
				compare_input = argv[index + 3];

				index++;
			}

			index += 3;

			continue;
		}

		if (strcmp(action, "-d") == 0 || strcmp(action, "--decode") == 0) {
			flags |= ACTION_DECODE;

			if (index + 1 >= argc) {
				usage(exec);

				rc = 1;
				goto cleanup;
			}

			code_input = argv[index + 1];

			if (strncmp(code_input, "-", 1) == 0) {
				printf("Invalid code %s\n", code_input);

				rc = 1;
				goto cleanup;
			}

			index += 2;

			continue;
		}

		if (strcmp(action, "-v") == 0 || strcmp(action, "--verbose") == 0) {
			flags |= ACTION_VERBOSE;
			index++;

			continue;
		}

		printf("Unknown action %s\n", action);

		rc = 1;
		goto cleanup;
	}

	if ((flags & ACTION_DECODE) != 0) {
		if (decode(code_input, &address_output, &data_output, &compare_output) != 0) {
			rc = 1;
			goto cleanup;
		}

		if ((flags & ACTION_VERBOSE) != 0) {
			printf("%s returns %s", address_output, data_output);

			if (compare_output != NULL) {
				printf(" if read as %s", compare_output);
			}

			printf("\n");
		} else {
			printf("%s\n%s\n", address_output, data_output);

			if (compare_output != NULL) {
				printf("%s\n", compare_output);
			}
		}
	} else if ((flags & ACTION_ENCODE) != 0) {
		if (encode(&code_output, address_input, data_input, compare_input) != 0) {
			rc = 1;
			goto cleanup;
		}

		printf("%s\n", code_output);
	}

cleanup:
	if (address_output) {
		free(address_output);
	}

	if (data_output) {
		free(data_output);
	}

	if (compare_output) {
		free(compare_output);
	}

	if (code_output) {
		free(code_output);
	}

	return rc;
}