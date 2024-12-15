#ifndef HEADER
#define HEADER

#include <string>

static int callback(void *NotUsed, int argc, char **argv, char **azColName);
void start_screen();
void open_account(std::string name);
bool account_lookup();


#endif