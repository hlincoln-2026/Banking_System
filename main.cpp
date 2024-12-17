#include "main.h"
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <map>
#include <limits>

using std::cout; using std::cin; using std::endl; using std::string;

sqlite3 *db;  // Declare database pointer globally
bool logged_in = false;


// Callback function to display results
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    cout << endl;
    return 0;
}

void add_account(string name, int id, string type_str, double balance){
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "INSERT INTO ACCOUNTS (ACCOUNT_TYPE, USER_ID, BALANCE) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    

    sqlite3_bind_text(stmt, 1, type_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_bind_int(stmt, 3, balance);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout << "Failed to Add Account: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
    return;


}

int get_id(string name, string pass){
    sqlite3_stmt *stmt;
    const char *sqlSelect = "SELECT ID FROM USERS WHERE USERNAME = ? AND PASSWORD = ?;";
    int id = -1;
    int rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return -1.0;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pass.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        
        id = sqlite3_column_int(stmt, 0); // Retrieve the balance
    }
    sqlite3_finalize(stmt);
    return id;
}

bool account_lookup(string name, string pass) {
    sqlite3_stmt *stmt;
    bool found = false;
    int rc;

    const char *sql = "SELECT * FROM USERS WHERE USERNAME = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    // Bind the parameter (replace ? with 'name')
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    bool account_found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        account_found = true;

        // Convert the database password to a string
        const unsigned char *db_password = sqlite3_column_text(stmt, 2);
        string retrieved_password = db_password ? reinterpret_cast<const char *>(db_password) : "";

        // Compare passwords
        if (retrieved_password == pass) {
            
            found = true;
            logged_in = true;
            break;
        }
    }

    

    if (account_found == false) {
        cout << "No account found with the name: " << name << endl;
    }else if(found == false){
        cout << "Invalid password! Please try again." << endl;
    }

    sqlite3_finalize(stmt);
    return account_found;
}

void open_account(string name, string pass) {
    sqlite3_stmt *stmt;
    int rc;
    double balance;
    string type_str;
    
    int type = -1;
    while(true){
        cout << "Account Types\n";
        cout << "1. Savings Account\n";
        cout << "2. Checking Account\n";
        cout << "Enter account choice: ";
        cin >> type;

        if(cin.fail()){
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "\nInvalid Choice\n" << endl;
        }else{
            if(type == 1){
                type_str = "Savings";
                break;
            }else if(type == 2){
                type_str = "Checking";
                break;
            }else{
                cout << "\nInvalid Choice\n" << endl;
            }
        }
    }

    while(true){
        cout << "Enter Starting Balance: ";
        cin >> balance;
        if(cin.fail()){
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "\nInvalid Input! Must be a number.\n" << endl;
        }else{
            break;
        }
    }

    bool exists = account_lookup(name, pass);

    if(!exists){
        const char *sql = "INSERT INTO USERS (USERNAME, PASSWORD) VALUES (?, ?);";

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Bind parameters
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, pass.c_str(), -1, SQLITE_STATIC);

        // Execute the statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            cout << "Error inserting data: " << sqlite3_errmsg(db) << endl;
        } else {
            cout << "\nAccount successfully created!" << endl;
        }
        sqlite3_finalize(stmt);
    }

    

    int id = get_id(name, pass);

    add_account(name, id, type_str, balance);


}

void check_balance(int id){
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT ACCOUNT_TYPE,BALANCE FROM ACCOUNTS WHERE USER_ID = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    double balance = -1.0;
    int count = 1;
    cout << endl;
    while(sqlite3_step(stmt) == SQLITE_ROW){
        const unsigned char *type = sqlite3_column_text(stmt, 0);
        string retrieved_type = type ? reinterpret_cast<const char *>(type) : "";
        balance = sqlite3_column_double(stmt, 1);
        cout << count << ". Account Type: [" << type << "]   Balance: $" << balance << endl;
        count += 1;
    }
        
    
    
    sqlite3_finalize(stmt);
    
}

double get_balance(int id){
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT BALANCE FROM ACCOUNTS WHERE USER_ID = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return -1.0;
    }

    sqlite3_bind_int(stmt, 1, id);

    double balance = -1.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 0); // Retrieve the balance
    }
    
    sqlite3_finalize(stmt);
    return balance;

}




void set_balance(sqlite3_stmt *stmt, double balance, int id){
    int rc;

    const char *sql = "UPDATE ACCOUNTS SET BALANCE = ? WHERE USER_ID = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_double(stmt, 1, balance);
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout << "Failed to update balance: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return;
    }
}

double deposit(double num, int id){
    sqlite3_stmt *stmt;
    int rc;

    double balance = get_balance(id);
    if (balance < 0) {
        cout << "Failed to retrieve balance. Deposit aborted." << endl;
        return -1.0;
    }

    balance += num;

    set_balance(stmt, balance, id);
    sqlite3_finalize(stmt);
    return get_balance(id);

}

void withdraw(const double num, const int id){
    sqlite3_stmt *stmt;
    int rc;

    double balance = get_balance(id);
    if (balance < 0) {
        cout << "Failed to retrieve balance. Withdraw aborted." << endl;
        return;
    }

    balance -= num;

    if(balance >= 0){
        set_balance(stmt, balance, id);
        balance = get_balance(id);
        cout << "\nYour Updated Balance: $" << balance << endl;
    }else{
        cout << "\nYou cannont withdraw this amount!" << endl;
    }

    sqlite3_finalize(stmt);
    
}


void loggedIn_Screen(string name, string pass){
    int choice = -1;
    int id = get_id(name, pass);
    while(logged_in && (choice != 6)){
        cout << "\nWelcome back " << name << "!\n";
        cout << "1. Add Account\n";
        cout << "2. Check Balance\n";
        cout << "3. Deposit\n";
        cout << "4. Withdraw\n";
        cout << "5. Transfer\n";
        cout << "6. Log Out\n";
        cout << "7. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if(cin.fail()){
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        
        if(choice == 1){
            open_account(name, pass);

        }else if(choice == 2){
            check_balance(id);
            
        
        }else if(choice == 3){
            double num;
            cout << "\nPlease enter amount you wish to deposit($): ";
            cin >> num;
            double balance = deposit(num, id);
            if (balance >= 0) {
                cout << "\nYour Updated Balance: $" << balance << endl;
            } else {
                cout << "\nError in depositing.\n";
            }
            

        }else if(choice == 4){
            double num;
            cout << "\nPlease enter the amount you wish to withdraw($): ";
            cin >> num;
            withdraw(num, id);

        }else if(choice == 5){
            continue;

        }else if(choice == 6){
            logged_in = false;
            cout << "\nLogging Out..." << endl;
            
        }else if(choice == 7){
                cout << "\nExiting program." << endl;
                exit(0); // Terminate the program

        }else{
            cout << "\nInvalid choice. Please try again." << endl;
        }
        
    }
}

void start_screen(){
    int choice = -1;
    while ((choice != 3) && !logged_in) {
        cout << "\nMenu:\n";
        cout << "1. Create Account\n";
        cout << "2. Login\n";
        cout << "3. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if(cin.fail()){
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if(choice == 1){
            string name;
            string pass;
            cout << "Enter Name: ";
            cin >> name;
            cout << "Create Password: ";
            cin >> pass;
            open_account(name, pass);
            
        }else if(choice == 2){
            string name;
            string pass;
            cout << "Enter Name: ";
            cin >> name;
            cout << "Enter Password: ";
            cin >> pass;
            bool found = account_lookup(name, pass);
            if(logged_in){
                cout << "\nLogin Successful!" << endl;
                loggedIn_Screen(name, pass);
            }
            
        }else if(choice == 3){
            cout << "Exiting program." << endl;
            
        }else{
            cout << "Invalid choice. Please try again." << endl;
        }
    }
    
}






int main() {
    char *errMessage = 0;
    int rc;

    // Open the database
    rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 0;
    }

   
    


    
    start_screen();
    


    sqlite3_close(db);
    return 0;
}
