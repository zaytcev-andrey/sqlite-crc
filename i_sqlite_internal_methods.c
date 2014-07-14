#include "i_sqlite_internal_methods.h"

void InitializeInternalMethods( sqlite_internal_methods* internal
     , PFGetDbFileDescriptor xGetDbFileDescriptor )
{
     internal->xGetDbFileDescriptor = xGetDbFileDescriptor;
}
