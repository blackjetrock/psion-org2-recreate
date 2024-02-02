//
// Dumps an object file to stdout
//
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "nopl_obj.h"

FILE *fp;

NOBJ_VAR_SPACE_SIZE    var_space_size;
NOBJ_QCODE_SPACE_SIZE  qcode_space_size;
NOBJ_NUM_PARAMETERS    num_parameters;
NOBJ_PARAMETER_TYPE    parameter_types[NOBJ_MAX_PARAMETERS];

int read_item(void *ptr, int n, size_t size)
{
  int ni = fread(ptr, n, size, fp);
  
  if( feof(fp) || (ni == 0))
    {
      // No more file
      return(0);
    }
  
  return(1);
}

void pr_uint16(uint16_t n)
{
  int h = (n & 0xFF00) >> 8;
  int l = (n & 0x00FF);

  printf("%02X%02X", l, h);
}

void pr_uint8(uint8_t n)
{
  int l = (n & 0x00FF);

  printf("%02X", l);
}

void pr_var_space_size(NOBJ_VAR_SPACE_SIZE *x)
{
  printf("\nVar Space Size:");
  pr_uint16(x->size);
}

void pr_qcode_space_size(NOBJ_QCODE_SPACE_SIZE *x)
{
  printf("\nVar Space Size:");
  pr_uint16(x->size);
}

void pr_num_parameters(NOBJ_NUM_PARAMETERS *x)
{
  printf("\nNumber of parameters:");
  pr_uint8(x->num);
}

void pr_parameter_types(void)
{
  printf("\nParameter types:");
  for(int i=0; i<num_parameters.num; i++)
    {
      pr_uint8(parameter_types[i]);
      printf(" ");
    }
}


  
int main(int argc, char *argv[])
{
  fp = fopen(argv[1], "r");

  if( fp == NULL )
    {
      printf("\nCannot open '%s'", argv[1]);
      exit(-1);
    }

  // Read the object file

  if(!read_item((void *)&var_space_size, 1, sizeof(NOBJ_VAR_SPACE_SIZE)))
    {
      printf("\nError reading var space size.");
      return(0);
    }

  if(!read_item((void *)&qcode_space_size, 1, sizeof(NOBJ_QCODE_SPACE_SIZE)))
    {
      printf("\nError reading qcode space size.");
      return(0);
    }

  if(!read_item((void *)&num_parameters.num, 1, sizeof(NOBJ_NUM_PARAMETERS)))
    {
      printf("\nError reading number of parameters.");
      return(0);
    }

  if(!read_item((void *)&parameter_types, num_parameters.num, sizeof(NOBJ_PARAMETER_TYPE)))
    {
      printf("\nError reading parameter types.");
      printf("\nnum=%d sizeof=%ld", num_parameters.num, sizeof(NOBJ_PARAMETER_TYPE));
      return(0);
    }
    
    fclose(fp);

    printf("\nDump of @%s'\n", argv[1]);

    pr_var_space_size(&var_space_size);
    pr_qcode_space_size(&qcode_space_size);
    pr_num_parameters(&num_parameters);
    pr_parameter_types();
    printf("\n");
    
}
