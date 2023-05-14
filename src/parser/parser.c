#include "parser.h"
#include "../common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char doc[] = "A seccomp based sandbox.Works with root privilege.";

struct argp_option options[] = {
    {"exe_path", 'p', "PATH", OPTION_ARG_OPTIONAL,
     "Path to the executable to run"},
    {"exe_args", 'a', "ARGS", OPTION_ARG_OPTIONAL,
     "Arguments to pass to the executable"},
    {"exe_envs", 'e', "ENVS", OPTION_ARG_OPTIONAL,
     "Environment variables to pass to the executable"},
    {"input_path", 'i', "PATH", OPTION_ARG_OPTIONAL, "Path to the input file"},
    {"output_path", 'o', "PATH", OPTION_ARG_OPTIONAL,
     "Path to the output file"},
    {"log_path", 'l', "PATH", OPTION_ARG_OPTIONAL,
     "Path to the log file (default: $cwd/sandbox.log)"},
    {"seccomp_rule", 'r', "RULE", OPTION_ARG_OPTIONAL,
     "Path to the seccomp rule file"},
    {"max_cpu_time", 'c', "MILLISECONDS", OPTION_ARG_OPTIONAL,
     "Maximum CPU time"},
    {"max_real_time", 't', "MILLISECONDS", OPTION_ARG_OPTIONAL,
     "Maximum real time"},
    {"max_memory", 'm', "BYTES", OPTION_ARG_OPTIONAL, "Maximum memory"},
    {"max_stack", 's', "BYTES", OPTION_ARG_OPTIONAL, "Maximum stack memory"},
    {"max_process_number", 'n', "NUM", OPTION_ARG_OPTIONAL,
     "Maximum process(threads) number"},
    {"max_output_size", 'z', "BYTES", OPTION_ARG_OPTIONAL,
     "Maximum output file size"},
    {0}};

// Splits a string into an array of strings.
// if any of substrings is longer than MAX_ARG_LEN, it will be dropped.
// returns the number of substrings.
int split_str(char **dst, char *str, const char *spl, int max_substring_len,
              char *exceed_message)
{
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while (result != NULL)
    {
        if (strlen(result) <= max_substring_len)
        {
            strcpy(dst[n++], result);
        }
        else
        {
            printf("%s: %s\n", exceed_message, result);
        }

        result = strtok(NULL, spl);
    }
    return n;
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    RunConfig *config = (RunConfig *)state->input;

    switch (key)
    {
    case 'p':
        config->exe_path = arg;
        break;
    case 'a':
        // Declartion after a label is not allowed, so we have to put a ; here.
        ;
        config->exe_args[0] = "guest";
        // -1: we have set exe_args[0] to "guest" above.
        // +1: we need to set the last element to NULL.
        char **args = malloc((MAX_ARG_NUM - 1 + 1)*sizeof(char *));
        for (int i = 0; i < MAX_ARG_NUM - 1; i++)
        {
            args[i] = malloc(MAX_ARG_LEN);
        }

        int count_args = split_str(args, arg, DELIMITER_ARG, MAX_ARG_LEN,
                                   MESSAGE_ARG_LEN_EXCEED);
        if (count_args > 0)
        {
            for (int i = 0; i < count_args; i++)
            {
                config->exe_args[i + 1] = args[i];
            }
        }
        config->exe_args[count_args + 1] = NULL;
        break;
    case 'e':
        // Almost the same as the exe_args case.
        ;
        config->exe_envs[0] = "PATH=/usr/local/bin:/usr/bin:/bin";
        char **envs = malloc((MAX_ARG_NUM - 1 + 1)*sizeof(char *));
        for (int i = 0; i < MAX_ENV_NUM - 1; i++)
        {
            envs[i] = malloc(MAX_ENV_LEN);
        }

        int count_envs = split_str(envs, arg, DELIMITER_ENV, MAX_ENV_LEN,
                                   MESSAGE_ENV_LEN_EXCEED);
        if (count_envs > 0)
        {
            for (int i = 0; i < count_envs; i++)
            {
                config->exe_envs[i + 1] = envs[i];
            }
        }
        config->exe_envs[count_envs + 1] = NULL;
        break;
    case 'i':
        config->input_path = arg;
        break;
    case 'o':
        config->output_path = arg;
        break;
    case 't':
        config->test_output_path = arg;
        break;
    case 'l':
        config->log_path = arg;
        break;
    case 'r':
        config->seccomp_rule = arg;
        break;
    case 'c':
        if (arg != NULL)
        {
            config->max_cpu_time = atoi(arg);
        }
        else
        {
            printf("You sure the number after -m is valid?\n");
            exit(0);
        }
        break;
    case 't':
        if (arg != NULL)
        {
            config->max_real_time = atoi(arg);
        }
        else
        {
            printf("You sure the number after -t is valid?\n");
            exit(0);
        }
        break;
    case 'm':
        if (arg != NULL)
        {
            config->max_memory = atoi(arg);
        }
        else
        {
            printf("You sure the number after -m is valid?\n");
            exit(0);
        }
        break;
    case 's':
        if (arg != NULL)
        {
            config->max_stack = atoi(arg);
        }
        else
        {
            printf("You sure the number after -s is valid?\n");
            exit(0);
        }
        break;
    case 'n':
        if (arg != NULL)
        {
            config->max_process_number = atoi(arg);
        }
        else
        {
            printf("You sure the number after -n is valid?\n");
            exit(0);
        }
        break;
    case 'z':
        if (arg != NULL)
        {
            config->max_output_size = atoi(arg);
        }
        else
        {
            printf("You sure the number after -u is valid?\n");
            exit(0);
        }
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 0)
            printf("This program only takes options, use --help option to get "
                   "information\n");
        exit(0);
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = {options, parse_opt, NULL, doc};

void InitConfig(int argc, char **argv, RunConfig *config)
{
    config->max_cpu_time = RLIM_INFINITY;
    config->max_real_time = RLIM_INFINITY;
    config->max_memory = RLIM_INFINITY;
    // The stack size is set to 8MB by default.
    config->max_stack = MAX_STACK_SIZE_DEFAULT;
    config->max_process_number = RLIM_INFINITY;
    config->max_output_size = RLIM_INFINITY;
    config->exe_path = malloc(MAX_PATH_LEN);
    // if no argument are passed , the default value is NULL.
    // if any argument is passed, the first argument is always string "guest".
    // and we need to allocate one more space for the null terminator.
    config->exe_args = malloc((MAX_ARG_NUM + 1)*sizeof(char *));
    config->exe_args[0] = NULL;
    // same as exe_args.
    config->exe_envs = malloc((MAX_ENV_NUM + 1)*sizeof(char *));
    config->exe_envs[0] = NULL;
    config->input_path = NULL;
    config->output_path = NULL;
    config->test_output_path = NULL;
    config->log_path = NULL;
    config->seccomp_rule = "general";

    config->gid = 65534;
    config->uid = 65534;
}
void ParseCommandline(int argc, char **argv, RunConfig *config)
{
    InitConfig(argc, argv, config);
    argp_parse(&argp, argc, argv, 0, 0, config);
    LOG_INFO("Program to be executed: %s", config->exe_path);
}
