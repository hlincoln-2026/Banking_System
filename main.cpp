#include "main.h"
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <map>

using std::cout; using std::cin; using std::endl; using std::string;

sqlite3 *db;  // Declare database pointer globally
bool logged_in = false;
string username;
int user_id;

// Callback function to display results
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++) {
        cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
    }
    cout << endl;
    return 0;
}

void open_account(string name) {
    sqlite3_stmt *stmt;
    int rc;

    double balance;
    cout << "Enter Starting Balance: ";
    cin >> balance;

    const char *sql = "INSERT INTO USERS (NAME, BALANCE) VALUES (?, ?);";
    const char *count = "SELECT COUNT(1) FROM USERS;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, balance);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout << "Error inserting data: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Account successfully created!" << endl;
    }

    // Prepare to count number of rows
    rc = sqlite3_prepare_v2(db, count, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }
    // Execute counting number of rows
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_count = sqlite3_column_int(stmt, 0);
        cout << "Your ID number is: " << row_count << endl;
    }


    sqlite3_finalize(stmt);
}

double check_balance(string name, int id){
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT * FROM USERS WHERE NAME = ? AND ID = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    double balance = -1.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        balance = sqlite3_column_double(stmt, 2); // Retrieve the balance
    }
    
    sqlite3_finalize(stmt);
    return balance;

}

bool account_lookup() {
    sqlite3_stmt *stmt;
    bool found = false;
    int rc;

    const char *sql = "SELECT ID, NAME, BALANCE FROM USERS WHERE NAME = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    string name;
    cout << "Please enter your name: ";
    cin >> name;
    username = name;

    int id;
    cout << "Please enter your ID number: ";
    cin >> id;
    user_id = id;

    // Bind the parameter (replace ? with 'name')
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    bool account_found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        account_found = true;
        if(sqlite3_column_int(stmt, 0) == id){
            const unsigned char *retrieved_name = sqlite3_column_text(stmt, 1);
            double balance = sqlite3_column_double(stmt, 2);

            // Print the retrieved values
            cout << "\nLogin Sucessful!" << endl;
            found = true;
        }
        
    }

    if (!account_found) {
        cout << "No account found with the name: " << name << endl;
        found = false;
    }
    sqlite3_finalize(stmt);
    return found;
}

double deposit(double num){
    sqlite3_stmt *stmt;
    int rc;

    double balance = check_balance(username, user_id);
    if (balance < 0) {
        cout << "Failed to retrieve balance. Deposit aborted." << endl;
        return -1.0;
    }
    balance += num;

    const char *sql = "UPDATE USERS SET BALANCE = ? WHERE ID = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return -1.0;
    }

    sqlite3_bind_double(stmt, 1, balance);
    sqlite3_bind_int(stmt, 2, user_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout << "Failed to update balance: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return -1.0;
    }

    sqlite3_finalize(stmt);

    return check_balance(username, user_id);

}

void loggedIn_Screen(){
    int choice = -1;
    while(logged_in && (choice != 6)){
        cout << "\nWelcome back " << username << "!\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. Transfer\n";
        cout << "5. Log Out\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if(choice == 1){
            double balance = check_balance(username, user_id);
            if (balance >= 0) {
                cout << "\nYour Balance: $" << balance << endl;
            } else {
                cout << "\nError retrieving balance.\n";
            }
        

        }else if(choice == 2){
            double num;
            cout << "\nPlease enter amount you wish to deposit: ";
            cin >> num;
            double balance = deposit(num);
            if (balance >= 0) {
                cout << "\nYour Updated Balance: $" << balance << endl;
            } else {
                cout << "\nError in depositing.\n";
            }

        }else if(choice == 3){
            continue;

        }else if(choice == 4){
            continue;

        }else if(choice == 5){
            logged_in = false;
            cout << "\nLogging Out..." << endl;
            
        }else if(choice == 6){
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

        if(choice == 1){
            string name;
            cout << "Enter Name: ";
            cin >> name;
            open_account(name);
            
        }else if(choice == 2){
            bool found = account_lookup();
            if(found){
                logged_in = true;
                loggedIn_Screen();
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
