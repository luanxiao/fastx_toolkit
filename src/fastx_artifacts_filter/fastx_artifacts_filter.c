#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <err.h>

#include <config.h>

#include "fastx.h"
#include "fastx_args.h"

#define MAX_ADAPTER_LEN 100

const char* usage=
"usage: fastq_artifacts_filter [-h] [-v] [-z] [-i INFILE] [-o OUTFILE]\n" \
"\n" \
"version " VERSION "\n" \
"   [-h]         = This helpful help screen.\n" \
"   [-i INFILE]  = FASTA/Q input file. default is STDIN.\n" \
"   [-o OUTFILE] = FASTA/Q output file. default is STDOUT.\n" \
"   [-z]         = Compress output with GZIP.\n" \
"   [-v]         = Verbose - report number of processed reads.\n" \
"                  If [-o] is specified,  report will be printed to STDOUT.\n" \
"                  If [-o] is not specified (and output goes to STDOUT),\n" \
"                  report will be printed to STDERR.\n" \
"\n";

#define DO_NOT_TRIM_LAST_BASE (0)

FASTX fastx;

int parse_commandline(int argc, char* argv[])
{
	return fastx_parse_cmdline(argc, argv, "", NULL);
}

int artifact_sequence(const FASTX *fastx)
{
	int n_count=0;
	int a_count=0;
	int c_count=0;
	int t_count=0;
	int g_count=0;
	int total_count=0;

	int max_allowed_different_bases = 3 ;

	int i=0;

	while (1) {
		if (fastx->nucleotides[i]==0)
			break;

		total_count++;
		switch(fastx->nucleotides[i])
		{
		case 'A':
			a_count++;
			break;
		case 'C':
			c_count++;
			break;
		case 'G':
			g_count++;
			break;
		case 'T':
			t_count++;
			break;
		case 'N':
			n_count++;
			break;
		}
		i++;
	}

	//Rules for artifacts
	
	if ( a_count>=(total_count-max_allowed_different_bases) 
	     ||
	     c_count>=(total_count-max_allowed_different_bases)
	     ||
	     g_count>=(total_count-max_allowed_different_bases)
	     ||
	     t_count>=(total_count-max_allowed_different_bases)
	     )
	     return 1;
	 

	 return 0;
}

int main(int argc, char* argv[])
{
	parse_commandline(argc, argv);

	fastx_init_reader(&fastx, get_input_filename(), 
		FASTA_OR_FASTQ, ALLOW_N, REQUIRE_UPPERCASE);

	fastx_init_writer(&fastx, get_output_filename(), 
		OUTPUT_SAME_AS_INPUT, compress_output_flag());

	while ( fastx_read_next_record(&fastx) ) {
		
		if ( artifact_sequence(&fastx)  ) {
		} else {
			fastx_write_record(&fastx);
		}
	}
	
	//Print verbose report
	if ( verbose_flag() ) {
		fprintf(get_report_file(), "Input: %zu reads.\n", num_input_reads(&fastx) ) ;
		fprintf(get_report_file(), "Output: %zu reads.\n", num_output_reads(&fastx) ) ;

		size_t discarded = num_input_reads(&fastx) - num_output_reads(&fastx) ;
		fprintf(get_report_file(), "discarded %u (%u%%) artifact reads.\n", 
			discarded, (discarded*100)/( num_input_reads(&fastx) ) ) ;
	}	

	return 0;
}