// New OPL object code definitions.
//
// This is backwards compatible with QCode where possible.
//

#define NOBJ_MAX_PARAMETERS  255
typedef uint8_t NOBJ_PARAMETER_TYPE;

typedef struct _NOBJ_VAR_SPACE_SIZE
{
  uint16_t size;
} NOBJ_VAR_SPACE_SIZE;

typedef struct _NOBJ_QCODE_SPACE_SIZE
{
  uint16_t size;
} NOBJ_QCODE_SPACE_SIZE;

typedef struct _NOBJ_NUM_PARAMETERS
{
  uint8_t num;
} NOBJ_NUM_PARAMETERS;
