#include "qemu/osdep.h"
#include "monitor/monitor.h"
#include "monitor/hmp.h"
#include "qapi/qapi-commands-misc-i386.h"
#include "qapi/error.h"

void sgx_epc_build_srat(GArray *table_data)
{
}

SgxInfo *qmp_query_sgx(Error **errp)
{
    error_setg(errp, "SGX support is not compiled in");
    return NULL;
}

SgxInfo *qmp_query_sgx_capabilities(Error **errp)
{
    error_setg(errp, "SGX support is not compiled in");
    return NULL;
}

bool check_sgx_support(void)
{
    return false;
}

bool sgx_epc_get_section(int section_nr, uint64_t *addr, uint64_t *size)
{
    return true;
}
