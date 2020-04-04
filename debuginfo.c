#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ipftrace.h"

void debuginfo_destroy(struct ipft_debuginfo *dinfo) {
  dinfo->destroy(dinfo);
  free(dinfo);
}

int debuginfo_fill_sym2info(struct ipft_debuginfo *dinfo,
                            struct ipft_symsdb *sdb) {
  return dinfo->fill_sym2info(dinfo, sdb);
}
