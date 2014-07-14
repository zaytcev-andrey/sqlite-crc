#ifndef db_state_h__
#define db_state_h__

#include "i_sqlite_internal_methods.h"

/// @brief database file state
typedef enum { DB_OPENED_CREATING, DB_OPENED_EXISTING, DB_CLOSED } db_open_state;

/// @brief get database file state
int GetDbOpenState( Btree* btree, db_open_state* state, sqlite_internal_methods* int_methods );

#endif // db_state_h__