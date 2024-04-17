#include "parser.h"
#include "interpreter.h"

void print_help() {
	printf("command line options:\n");
	printf("\t'--quiet'               : disables printing inbetween operations\n");
	printf("\t\n");
	printf("\t'--verbose-parser'     : prints opcodes and parameters of the awatalk\n");
	printf("\t'--verbose-interpreter' : prints current instruction and stack while executing\n");
	printf("\t'--verbose'             : enables both of the above\n");
	printf("\t\n");
	printf("\t'--help'                : prints this message\n");
	printf("\t\n");
	printf("\t'-string'               : the next provided argument will be awatalk to be parsed\n");
	printf("\t'-file'                 : the next provided argument will be awatalk file to be parsed\n");
	printf("example usage:\n");
	printf("\t'awaparser -verbose -string 'awa awa awawa awawa awa awa awa awa awa awawa awa awa awawa awawa awa awa awa awa awa awawawa awa awawa awawa awa awa awa awa awa awawa awa awawa awa awawa awa awa awawawa awa awa awa awawa'\n");
	printf("\t'awaparser --quiet -file hello_world.awa\n");
	printf("\t'awaparser --verbose-parser -file hello_world.awa\n");
}

int main(int argc, char** argv)
{
	bool verbose_parser = false;
	bool verbose_interpreter = false;
	bool quiet = false;
	char* file = NULL;
	char* string = NULL;
	bool file_next = false;
	bool string_next = false;
	for (int i = 1; i < argc; i++) {
		if (file_next) {
			file_next = false;
			file = argv[i];
			if (*argv[i] == '-')
				printf("WARNING: -file argument might not be a propper filepath '%s'\n", argv[i]);
			continue;
		}
		if (string_next) {
			string_next = false;
			string = argv[i];
			if (*argv[i] == '-')
				printf("WARNING: -string argument might not be a propper awatalk '%s'\n", argv[i]);
			continue;
		}
		if (strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		}
		if (strcmp(argv[i], "--quiet") == 0)
			quiet = true;
		else if (strcmp(argv[i], "--verbose") == 0)
			verbose_parser = verbose_interpreter = true;
		else if (strcmp(argv[i], "--verbose-parser") == 0)
			verbose_parser = true;
		else if (strcmp(argv[i], "--verbose-interpreter") == 0)
			verbose_interpreter = true;
		else if (strcmp(argv[i], "-file") == 0)
			file_next = true;
		else if (strcmp(argv[i], "-string") == 0)
			string_next = true;
		else
			printf("WARNING: unknown command line argument '%s'\n", argv[i]);
	}
	if (string_next) {
		puts("ERROR: '-string' was provided but the next argument didn't exist");
		return 2;
	}
	if (file_next) {
		puts("ERROR: '-file' was provided but the next argument didn't exist");
		return 2;
	}
	if (file && string) {
		puts("ERROR: '-file' and '-string' were provided at the same time");
		return 2;
	}
	if (!file && !string) {
		puts("ERROR: neither '-file' nor '-string' were provided");
		print_help();
		return 0;
	}
	awa_program_t prog = {0};
	if (file)   {
		prog = awatalk_file_to_program(file, verbose_parser);
	}
	if (string) {
		int l = strlen(string);
		char* prog_source = malloc(l+1);
		memcpy(prog_source, string, l+1);
		prog = awatalk_to_program(prog_source, verbose_parser);
	}
	if (prog.failed_parse) {
		puts("error during parsing, exiting.");
		awa_program_free(&prog);
		return 1;
	}
	if (!quiet) puts("parsed file succesfully");
	if (!quiet) puts("running program:\n-");
	awa_program_run(prog, verbose_interpreter);
	if (!quiet) puts("\n-\nprogram done!");
	awa_program_free(&prog);
}
