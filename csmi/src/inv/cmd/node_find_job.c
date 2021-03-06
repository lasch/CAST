/*================================================================================

    csmi/src/inv/cmd/node_find_job.c

  © Copyright IBM Corporation 2015-2018. All Rights Reserved

    This program is licensed under the terms of the Eclipse Public License
    v1.0 as published by the Eclipse Foundation and available at
    http://www.eclipse.org/legal/epl-v10.html

    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
    restricted by GSA ADP Schedule Contract with IBM Corp.

================================================================================*/
/*
* Author: Nick Buonarota
* Email: nbuonar@us.ibm.com
*/
/*C Include*/
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
/*CORAL includes*/
#include "utilities/include/string_tools.h"
/*CSM Include*/
#include "csmi/include/csm_api_inventory.h"
/*Needed for infrastructure logging*/
#include "csmutil/include/csmutil_logging.h"

/* Define API types to make life easier. */
#define API_PARAMETER_INPUT_TYPE csm_node_find_job_input_t
#define API_PARAMETER_OUTPUT_TYPE csm_node_find_job_output_t

#define USAGE csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input); help

void help() {
	puts("_____CSM_NODE_FIND_JOB_CMD_HELP_____");
	puts("USAGE:");
	puts("  csm_node_find_job ARGUMENTS [OPTIONS]");
	puts("  csm_node_find_job [-b begin_time_search_begin] [-B begin_time_search_end] [-e end_time_search_begin] [-E end_time_search_end] [-n node_names] [-l limit] [-o offset] [-h] [-v verbose_level] [-Y]");
	puts("");
	puts("SUMMARY: Used to help find what job was running on a node during a specific time.");
	puts("");
	puts("EXIT STATUS:");
	puts("  0  if OK,");
	puts("  1  if ERROR.");
	puts("");
	puts("ARGUMENTS:");
	puts("  REQUIRED:");
	puts("    csm_node_find_job has 1 required argument");
	puts("    Argument         | Example value       | Description  ");                                                 
	puts("    -----------------|---------------------|--------------");
	puts("    -n, --node_names | \"node_01,node_02\"   | (STRING) Filter results to only include records that have a matching node name. This is a csv string of valid node names in the CSM database.");
	puts("  OPTIONAL:");
	puts("    csm_node_find_job can have 9 optional arguments and requires at least 1");
	puts("    Argument                      | Example value                | Description  ");                                                 
	puts("    ------------------------------|------------------------------|--------------");
	puts("    -b, --begin_time_search_begin | \"1999-01-15 12:00:00.123456\" | (STRING) A time used to filter results of the SQL query and only include records with a begin_time at or after (ie: '>=' ) this time.");
	puts("    -B, --begin_time_search_end   | \"1999-01-15 13:00:00.123456\" | (STRING) A time used to filter results of the SQL query and only include records with a begin_time at or before (ie: '<=' ) this time.");
	puts("    -e, --end_time_search_begin   | \"2000-01-15 12:34:56.123456\" | (STRING) A time used to filter results of the SQL query and only include records with an end_time at or after (ie: '>=' ) this time.");
	puts("    -E, --end_time_search_end     | \"2000-01-15 13:37:33.134317\" | (STRING) A time used to filter results of the SQL query and only include records with an end_time at or before (ie: '<=' ) this time.");
	puts("    -m, --midpoint                | \"2000-01-15 12:00:00.000000\" | (STRING) A time used to filter results of the SQL query and only include records active during this time.");
	puts("    -M, --midpoint_interval       | \"1 day 2 hour 1 minute\"      | (STRING) A time that will be added and subtracted from the midpoint field to expand the range of the search window. Read more about interval syntax in postgres for more info.");
	puts("    -s, --search_range_begin      | \"2000-01-15 12:00:00.000000\" | (STRING) A time used to filter results of the SQL query and only include records that were active during or after this time.");
	puts("    -S, --search_range_end        | \"2000-01-15 13:00:00.000000\" | (STRING) A time used to filter results of the SQL query and only include records that were active during or before this time.");
    puts("    -u, --user_name               | \"csm_admin\"                  | (STRING) Filter results to only include this user_name.");
	puts("  FILTERS:");
	puts("    csm_diag_run_query can have 2 optional filters.");
	puts("    Argument       | Example value | Description  ");                                                 
	puts("    ---------------|---------------|--------------");
	puts("    -l, --limit    | 10            | (INTEGER) SQL 'LIMIT' numeric value.");
    puts("    -o, --offset   | 1             | (INTEGER) SQL 'OFFSET' numeric value.");
	puts("    -O, --order_by | a             | (CHAR) SQL 'ORDER BY' numeric value. Default Value: 'a'");
	puts("                                   | Valid Values: [a] = 'ORDER BY node_name, allocation_id ASC NULLS LAST'"); 
	puts("                                   |               [b] = 'ORDER BY node_name, allocation_id DESC NULLS LAST'");
	puts("                                   |               [c] = 'ORDER BY allocation_id ASC NULLS LAST'"); 
	puts("                                   |               [d] = 'ORDER BY allocation_id DESC NULLS LAST'");
	puts("                                   |               [e] = 'ORDER BY primary_job_id ASC NULLS LAST'");
	puts("                                   |               [f] = 'ORDER BY primary_job_id DESC NULLS LAST'");
	puts("                                   |               [g] = 'ORDER BY user_name ASC NULLS LAST'");
	puts("                                   |               [h] = 'ORDER BY user_name DESC NULLS LAST'");
	puts("                                   |               [i] = 'ORDER BY num_nodes ASC NULLS LAST'");
	puts("                                   |               [j] = 'ORDER BY num_nodes DESC NULLS LAST'");
	puts("                                   |               [k] = 'ORDER BY begin_time ASC NULLS LAST'");
	puts("                                   |               [l] = 'ORDER BY begin_time DESC NULLS LAST'");
	puts("                                   |               [m] = 'ORDER BY end_time ASC NULLS LAST'");
	puts("                                   |               [n] = 'ORDER BY end_time DESC NULLS LAST'");
	puts("");
	puts("GENERAL OPTIONS:");
	puts("[-h, --help]                  | Help.");
	puts("[-v, --verbose verbose_level] | Set verbose level. Valid verbose levels: {off, trace, debug, info, warning, error, critical, always, disable}");
	puts("[-Y, --YAML]                  | Set output to YAML. By default for this API, we have a custom output for ease of reading the many jobs on a node.");
	puts("");
	puts("EXAMPLE OF USING THIS COMMAND:");
	puts("  csm_node_find_job -n node_01 -m \"2000-01-15 12:00:00.000000\" -M \"1 hour\"");
	puts("");
	puts("OUTPUT OF THIS COMMAND CAN BE DISPLAYED IN THE YAML FORMAT.");
	puts("____________________");
}

struct option longopts[] = {
	//general options
	{"help",                    no_argument,       0, 'h'},
	{"verbose",                 required_argument, 0, 'v'},
	{"YAML",                    no_argument,       0, 'Y'},
	//arguments
	{"begin_time_search_begin", required_argument, 0, 'b'},
	{"begin_time_search_end",   required_argument, 0, 'B'},
	{"midpoint",                required_argument, 0, 'm'},
	{"midpoint_interval",       required_argument, 0, 'M'},
	{"node_names",              required_argument, 0, 'n'},
	{"search_range_begin",      required_argument, 0, 's'},
	{"search_range_end",        required_argument, 0, 'S'},
	{"user_name",               required_argument, 0, 'u'},
	//filters
	{"limit",                   required_argument, 0, 'l'},
	{"offset",                  required_argument, 0, 'o'},
	{"order_by",                required_argument, 0, 'O'},
	{0,0,0,0}
};

/*
* Summary: Simple command line interface for the CSM API 'node find job'. 
*			Works as interface between system admin and the CSM DB.
* 			Takes in the options via command line parameters, and queries the data in the CSM database.
*/
int main(int argc, char *argv[])
{
	
	/*CSM Variables*/
	csm_api_object *csm_obj = NULL;
	/*Helper Variables*/
	int return_value = 0;
	int requiredParameterCounter = 0;
	int optionalParameterCounter = 0;
	const int NUMBER_OF_REQUIRED_ARGUMENTS = 1;
	const int MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS = 1;
	/*Variables for checking cmd line args*/
	int opt;
	/* getopt_long stores the option index here. */
	int indexptr = 0;
	/* For for loops.*/
	uint32_t i = 0;
	char *arg_check = NULL; ///< Used in verifying the long arg values.
	/*For format printing later. */
	char YAML = 0;
	
	/*Set up data to call API*/
	API_PARAMETER_INPUT_TYPE* input = NULL;
	/* CSM API initialize and malloc function*/
	csm_init_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
	API_PARAMETER_OUTPUT_TYPE* output = NULL;

	/*check optional args*/
	while ((opt = getopt_long(argc, argv, "hv:b:B:e:E:m:M:n:l:o:O:s:S:u:Y", longopts, &indexptr)) != -1) {
		switch(opt){
			case 'h':      
                USAGE();
				return CSMI_HELP;
			case 'v':
				/*Error check to make sure 'verbose' field is valid.*/
                csm_set_verbosity( optarg, USAGE )
				break;
			case 'b':
                csm_optarg_test( "-b, --begin_time_search_begin", optarg, USAGE );
				input->begin_time_search_begin = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'B':
                csm_optarg_test( "-B, --begin_time_search_end", optarg, USAGE );
				input->begin_time_search_end = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'e':
                csm_optarg_test( "-e, --end_time_search_begin", optarg, USAGE );
				input->end_time_search_begin = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'E':
                csm_optarg_test( "-E, --end_time_search_end", optarg, USAGE );
				input->end_time_search_end = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'm':
				csm_optarg_test( "-m, --midpoint", optarg, USAGE );
				input->midpoint = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'M':
				csm_optarg_test( "-M, --midpoint_interval", optarg, USAGE );
				input->midpoint_interval = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'n':
				csm_optarg_test( "-n, --node_names", optarg, USAGE );
				csm_parse_csv( optarg, input->node_names, input->node_names_count, char*, csm_str_to_char, NULL, "-n, --node_names", USAGE );
				requiredParameterCounter++;
				break;
            case 'l':
                csm_optarg_test( "-l, --limit", optarg, USAGE );
                csm_str_to_int32( input->limit, optarg, arg_check, "-l, --limit", USAGE );
                break;
            case 'o':
                csm_optarg_test( "-o, --offset", optarg, USAGE );
                csm_str_to_int32( input->offset, optarg, arg_check, "-o, --offset", USAGE );
				break;
			case 'O':
				if(strlen(optarg) == 1 && 
                    (  optarg[0] == 'a' 
					|| optarg[0] == 'b' 
					|| optarg[0] == 'c' 
					|| optarg[0] == 'd' 
					|| optarg[0] == 'e' 
					|| optarg[0] == 'f' 
					|| optarg[0] == 'g' 
					|| optarg[0] == 'h' 
					|| optarg[0] == 'i' 
					|| optarg[0] == 'j' 
					|| optarg[0] == 'k' 
					|| optarg[0] == 'l' 
					|| optarg[0] == 'm' 
					|| optarg[0] == 'n' 
					)
				)
                {
					input->order_by = optarg[0];
				}else{
					csmutil_logging(error, "Invalid parameter for -O: optarg , encountered: %s", optarg);
                    USAGE();
					return CSMERR_INVALID_PARAM;
				}
				break;
			case 's':
				csm_optarg_test( "-s, --search_range_begin", optarg, USAGE );
				input->search_range_begin = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'S':
				csm_optarg_test( "-S, --search_range_end", optarg, USAGE );
				input->search_range_end = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'u':
				csm_optarg_test( "-u, --user_name", optarg, USAGE );
				input->user_name = strdup(optarg);
				optionalParameterCounter++;
				break;
			case 'Y':
				YAML = 1;
				break;
			default:
				csmutil_logging(error, "unknown arg: '%c'\n", opt);
				csmutil_logging(error, "unknown option: '%s'\n", optarg);
                USAGE();
				return CSMERR_INVALID_PARAM; 
		}
	}
	
	/*Handle command line args*/
	argc -= optind;
	argv += optind;
	
	/*Collect mandatory args*/
	/*Check to see if expected number of arguments is correct.*/
	if(requiredParameterCounter < NUMBER_OF_REQUIRED_ARGUMENTS || optionalParameterCounter < MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS)
    {
		/*We don't have the correct number of needed arguments passed in.*/
		csmutil_logging(error, "%s-%d:", __FILE__, __LINE__);
		csmutil_logging(error, "  Missing operand(s).");
		csmutil_logging(error, "    Encountered %i required parameter(s). Expected %i required parameter(s).", requiredParameterCounter, NUMBER_OF_REQUIRED_ARGUMENTS);
		csmutil_logging(error, "    Encountered %i optional parameter(s). Expected at least %i optional parameter(s).", optionalParameterCounter, MINIMUM_NUMBER_OF_OPTIONAL_ARGUMENTS);
        USAGE();
		return CSMERR_MISSING_PARAM;
	}
	
	/* Success required to be able to communicate between library and daemon - csmi calls must be made inside the frame created by csm_init_lib() and csm_term_lib()*/
	return_value = csm_init_lib();
	if( return_value != 0){
		csmutil_logging(error, "%s-%d:", __FILE__, __LINE__);
		csmutil_logging(error, "  csm_init_lib rc= %d, Initialization failed. Success is required to be able to communicate between library and daemon. Are the daemons running?", return_value);
        csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
		return return_value;           
	}

	//This will print out the contents of the struct that we will pass to the api
	csmutil_logging(debug, "%s-%d:", __FILE__, __LINE__);
	csmutil_logging(debug, "  Preparing to call the CSM API...");
	csmutil_logging(debug, "  value of input:         %p", input);
	csmutil_logging(debug, "  address of input:       %p", &input);
	csmutil_logging(debug, "  input contains the following:");
	csmutil_logging(debug, "    begin_time_search_begin: %s", input->begin_time_search_begin);
	csmutil_logging(debug, "    begin_time_search_end:   %s", input->begin_time_search_end);
	csmutil_logging(debug, "    end_time_search_begin:   %s", input->end_time_search_begin);
	csmutil_logging(debug, "    end_time_search_end:     %s", input->end_time_search_end);
	csmutil_logging(debug, "    limit:                   %i", input->limit);
	csmutil_logging(debug, "    midpoint:                %s", input->midpoint);
	csmutil_logging(debug, "    midpoint_interval:       %s", input->midpoint_interval);
	csmutil_logging(debug, "    node_names_count:        %i", input->node_names_count);
	csmutil_logging(debug, "    node_names:              %p", input->node_names);
	for(i = 0; i < input->node_names_count; i++){
		csmutil_logging(debug, "      node_names[%i]: %lld", i, input->node_names[i]);
	}
	csmutil_logging(debug, "    offset:                  %i", input->offset);
	csmutil_logging(debug, "    search_range_begin:      %s", input->search_range_begin);
	csmutil_logging(debug, "    search_range_end:        %s", input->search_range_end);
	csmutil_logging(debug, "    user_name:               %s", input->user_name);
	csmutil_logging(debug, "  value of output:        %p", output);
	csmutil_logging(debug, "  address of output:      %p", &output);
	
	/* Call the actual CSM API */
	return_value = csm_node_find_job(&csm_obj, input, &output);
    
	//Use CSM API free to release arguments. We no longer need them.
	csm_free_struct_ptr(API_PARAMETER_INPUT_TYPE, input);
    
    switch(return_value)
    {
        case CSMI_SUCCESS:
			if(YAML == 1)
			{
				puts("---");
				printf("Total_Records: %u\n", output->results_count);
				for (i = 0; i < output->results_count; i++) {
					printf("RECORD_%i:\n", i+1);
					printf("  node_name:      %s\n", output->results[i]->node_name);
					printf("  allocation_id:  %" PRId64 "\n", output->results[i]->allocation_id);
					printf("  primary_job_id: %" PRId64 "\n", output->results[i]->primary_job_id);
					printf("  user_name:      %s\n", output->results[i]->user_name);
					printf("  num_nodes:      %" PRId32 "\n", output->results[i]->num_nodes);
					printf("  begin_time:     %s\n", output->results[i]->begin_time);
					printf("  end_time:       %s\n", output->results[i]->end_time);
				}
				puts("...");
			}
			else
			{
				int largest_nodename_length = 9;
				int largest_allocationid_length = 13;
				int largest_primaryjobid_length = 14;
				int largest_username_length = 9;
				int largest_numnodes_length = 9;
				
				// 10 = largest 
				for(i = 0; i < output->results_count; i++)
				{
					int temp_nodeNameSize = strlen(output->results[i]->node_name);
					int temp_userNameSize = strlen(output->results[i]->user_name);
					
					if(temp_nodeNameSize > largest_nodename_length){
						largest_nodename_length = temp_nodeNameSize;
					}
					if(temp_userNameSize > largest_username_length){
						largest_username_length = temp_userNameSize;
					}
				}
				
				puts("---");
				//printf("node_name: %s\n", output->results[i]->node_name);
				printf("# %-*s | %-*s | %-*s | %-*s | %-*s | %-26s | %-26s\n", largest_nodename_length, "node_name", largest_allocationid_length, "allocation_id", largest_primaryjobid_length, "primary_job_id", largest_username_length, "user_name", largest_numnodes_length, "num_nodes", "begin_time", "end_time");
				printf("# %-*s + %-*s + %-*s + %-*s + %-*s + %-26s + %-26s\n", largest_nodename_length, " "        , largest_allocationid_length, " "            , largest_primaryjobid_length, " "             , largest_username_length, " "        , largest_numnodes_length, " "        , " "         , " ");
				
				//printf("#    node_name    | allocation_id | primary_job_id | user_name | num_nodes |         begin_time         |          end_time          \n");
				//printf("#-----------------+---------------+----------------+-----------+-----------+----------------------------+----------------------------\n");
				for(i = 0; i < output->results_count; i++){
					//format the string for nice padding
					printf("# %-*s | %-*"PRId64" | %-*"PRId64" | %-*s | %-*"PRId32" | %-26s | %-26s\n", largest_nodename_length, output->results[i]->node_name, largest_allocationid_length, output->results[i]->allocation_id, largest_primaryjobid_length, output->results[i]->primary_job_id, largest_username_length, output->results[i]->user_name, largest_numnodes_length, output->results[i]->num_nodes, output->results[i]->begin_time, output->results[i]->end_time);
				}
				puts("...");
			}
			
		    
            break;

        case CSMI_NO_RESULTS:
            puts("---");
            printf("Total_Records: 0\n");
            puts("# No matching records found.");
            puts("...");
            break;
        
        default:
            printf("%s FAILED: errcode: %d errmsg: %s\n",
                argv[0], return_value,  csm_api_object_errmsg_get(csm_obj));
    }

	//Call internal CSM API clean up.
    csm_api_object_destroy(csm_obj);
	
    // Cleanup the library and print the error.
    int lib_return_value = csm_term_lib();
    if( lib_return_value != 0 )
    {
        csmutil_logging(error, "csm_term_lib rc= %d, Initialization failed. Success "
            "is required to be able to communicate between library and daemon. Are the "
            "daemons running?", lib_return_value);
        return lib_return_value;
    }

	return return_value;
}
