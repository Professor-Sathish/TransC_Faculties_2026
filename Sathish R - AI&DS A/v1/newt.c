// Enhanced Bank Transaction System with Real-time Features
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Enhanced client data structure
struct client_data {
    unsigned int acct_num;
    char last_name[15];
    char first_name[10];
    double balance;
    double daily_limit;
    int failed_attempts;
    time_t last_transaction;
    char status[10]; // ACTIVE, LOCKED, SUSPENDED
};

// Transaction record structure
struct transaction_record {
    unsigned int transaction_id;
    unsigned int account_num;
    char transaction_type[20]; // DEPOSIT, WITHDRAWAL, TRANSFER, etc.
    double amount;
    double balance_before;
    double balance_after;
    time_t timestamp;
    char description[50];
    char status[10]; // SUCCESS, FAILED, PENDING
};

#define MAX_RECORDS 150
#define MAX_TRANSACTIONS 1000
#define DAILY_LIMIT 5000.00
#define MAX_FAILED_ATTEMPTS 3

// Enhanced prototypes
unsigned int enter_choice(void);
void real_time_transaction(FILE *f_ptr);
void process_deposit(FILE *f_ptr);
void process_withdrawal(FILE *f_ptr);
void process_transfer(FILE *f_ptr);
void view_transaction_history(FILE *f_ptr);
void real_time_balance_inquiry(FILE *f_ptr);
void fraud_detection_check(struct client_data *client, double amount);
void log_transaction(struct transaction_record *trans);
void display_transaction_progress(const char *operation);
void validate_pin(unsigned int account_num);
void lock_account(FILE *f_ptr, unsigned int account_num);
void unlock_account(FILE *f_ptr, unsigned int account_num);
int check_daily_limit(struct client_data *client, double amount);
void send_notification(unsigned int account_num, const char *message);
void display_real_time_status(void);

// Global transaction counter for unique IDs
static unsigned int transaction_counter = 1000;

int main(int argc, char *argv[]) {
    FILE *cf_ptr;
    unsigned int choice;

    printf("=== REAL-TIME BANKING SYSTEM ===\n");
    printf("Initializing secure connection...\n");
    display_transaction_progress("System Startup");

    if ((cf_ptr = fopen("credit.dat", "rb+")) == NULL) {
        printf("Creating new database...\n");
        cf_ptr = fopen("credit.dat", "wb+");
        if (cf_ptr == NULL) {
            printf("ERROR: Cannot initialize database.\n");
            exit(-1);
        }
    }

    while ((choice = enter_choice()) != 9) {
        display_real_time_status();
        
        switch (choice) {
            case 1:
                real_time_balance_inquiry(cf_ptr);
                break;
            case 2:
                process_deposit(cf_ptr);
                break;
            case 3:
                process_withdrawal(cf_ptr);
                break;
            case 4:
                process_transfer(cf_ptr);
                break;
            case 5:
                view_transaction_history(cf_ptr);
                break;
            case 6:
                real_time_transaction(cf_ptr); // Bulk transaction processing
                break;
            case 7:
                lock_account(cf_ptr, 0); // Will prompt for account
                break;
            case 8:
                unlock_account(cf_ptr, 0);
                break;
            default:
                printf("❌ Invalid choice. Please try again.\n");
                break;
        }
        
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
    }

    printf("💾 Saving all transactions...\n");
    display_transaction_progress("System Shutdown");
    fclose(cf_ptr);
    return 0;
}

// Real-time balance inquiry with instant response
void real_time_balance_inquiry(FILE *f_ptr) {
    unsigned int account;
    struct client_data client = {0, "", "", 0.0, DAILY_LIMIT, 0, 0, "ACTIVE"};
    
    printf("\n🔍 REAL-TIME BALANCE INQUIRY\n");
    printf("Enter account number: ");
    scanf("%u", &account);
    
    printf("🔄 Processing request...\n");
    usleep(500000); // 0.5 second delay for realism
    
    fseek(f_ptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    fread(&client, sizeof(struct client_data), 1, f_ptr);
    
    if (client.acct_num == 0) {
        printf("❌ Account not found.\n");
        return;
    }
    
    if (strcmp(client.status, "LOCKED") == 0) {
        printf("🔒 Account is LOCKED. Contact customer service.\n");
        return;
    }
    
    time_t current_time = time(NULL);
    printf("\n✅ ACCOUNT INFORMATION\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Account: %u\n", client.acct_num);
    printf("Name: %s %s\n", client.first_name, client.last_name);
    printf("💰 Current Balance: $%.2f\n", client.balance);
    printf("📅 Daily Limit: $%.2f\n", client.daily_limit);
    printf("🕐 Last Updated: %s", ctime(&current_time));
    printf("Status: %s\n", client.status);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    send_notification(account, "Balance inquiry completed");
}

// Enhanced deposit with real-time processing
void process_deposit(FILE *f_ptr) {
    unsigned int account;
    double amount;
    struct client_data client = {0, "", "", 0.0, DAILY_LIMIT, 0, 0, "ACTIVE"};
    struct transaction_record trans = {0};
    
    printf("\n💰 REAL-TIME DEPOSIT PROCESSING\n");
    printf("Enter account number: ");
    scanf("%u", &account);
    
    fseek(f_ptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    fread(&client, sizeof(struct client_data), 1, f_ptr);
    
    if (client.acct_num == 0) {
        printf("❌ Account not found.\n");
        return;
    }
    
    if (strcmp(client.status, "LOCKED") == 0) {
        printf("🔒 Account is LOCKED.\n");
        return;
    }
    
    printf("Current balance: $%.2f\n", client.balance);
    printf("Enter deposit amount: $");
    scanf("%lf", &amount);
    
    if (amount <= 0) {
        printf("❌ Invalid amount.\n");
        return;
    }
    
    // Real-time processing simulation
    printf("🔄 Processing deposit...\n");
    display_transaction_progress("Deposit Processing");
    
    // Fraud detection
    fraud_detection_check(&client, amount);
    
    // Process transaction
    trans.transaction_id = ++transaction_counter;
    trans.account_num = account;
    strcpy(trans.transaction_type, "DEPOSIT");
    trans.amount = amount;
    trans.balance_before = client.balance;
    client.balance += amount;
    trans.balance_after = client.balance;
    trans.timestamp = time(NULL);
    strcpy(trans.description, "Cash Deposit");
    strcpy(trans.status, "SUCCESS");
    
    // Update account
    client.last_transaction = time(NULL);
    fseek(f_ptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    fwrite(&client, sizeof(struct client_data), 1, f_ptr);
    fflush(f_ptr);
    
    // Log transaction
    log_transaction(&trans);
    
    printf("✅ DEPOSIT SUCCESSFUL!\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Transaction ID: %u\n", trans.transaction_id);
    printf("Amount Deposited: $%.2f\n", amount);
    printf("New Balance: $%.2f\n", client.balance);
    printf("Timestamp: %s", ctime(&trans.timestamp));
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    send_notification(account, "Deposit completed successfully");
}

// Enhanced withdrawal with security checks
void process_withdrawal(FILE *f_ptr) {
    unsigned int account;
    double amount;
    struct client_data client = {0, "", "", 0.0, DAILY_LIMIT, 0, 0, "ACTIVE"};
    struct transaction_record trans = {0};
    
    printf("\n💸 REAL-TIME WITHDRAWAL PROCESSING\n");
    printf("Enter account number: ");
    scanf("%u", &account);
    
    // PIN validation
    validate_pin(account);
    
    fseek(f_ptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    fread(&client, sizeof(struct client_data), 1, f_ptr);
    
    if (client.acct_num == 0) {
        printf("❌ Account not found.\n");
        return;
    }
    
    if (strcmp(client.status, "LOCKED") == 0) {
        printf("🔒 Account is LOCKED.\n");
        return;
    }
    
    printf("Available balance: $%.2f\n", client.balance);
    printf("Daily limit remaining: $%.2f\n", client.daily_limit);
    printf("Enter withdrawal amount: $");
    scanf("%lf", &amount);
    
    if (amount <= 0) {
        printf("❌ Invalid amount.\n");
        return;
    }
    
    if (amount > client.balance) {
        printf("❌ Insufficient funds!\n");
        send_notification(account, "Withdrawal failed - Insufficient funds");
        return;
    }
    
    if (!check_daily_limit(&client, amount)) {
        printf("❌ Daily limit exceeded!\n");
        return;
    }
    
    printf("🔄 Processing withdrawal...\n");
    display_transaction_progress("Withdrawal Processing");
    
    fraud_detection_check(&client, amount);
    
    // Process transaction
    trans.transaction_id = ++transaction_counter;
    trans.account_num = account;
    strcpy(trans.transaction_type, "WITHDRAWAL");
    trans.amount = amount;
    trans.balance_before = client.balance;
    client.balance -= amount;
    trans.balance_after = client.balance;
    trans.timestamp = time(NULL);
    strcpy(trans.description, "Cash Withdrawal");
    strcpy(trans.status, "SUCCESS");
    
    // Update account
    client.last_transaction = time(NULL);
    client.daily_limit -= amount;
    fseek(f_ptr, (account - 1) * sizeof(struct client_data), SEEK_SET);
    fwrite(&client, sizeof(struct client_data), 1, f_ptr);
    fflush(f_ptr);
    
    log_transaction(&trans);
    
    printf("✅ WITHDRAWAL SUCCESSFUL!\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Transaction ID: %u\n", trans.transaction_id);
    printf("Amount Withdrawn: $%.2f\n", amount);
    printf("Remaining Balance: $%.2f\n", client.balance);
    printf("Timestamp: %s", ctime(&trans.timestamp));
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    send_notification(account, "Withdrawal completed successfully");
}

// Real-time fund transfer
void process_transfer(FILE *f_ptr) {
    unsigned int from_account, to_account;
    double amount;
    struct client_data from_client = {0}, to_client = {0};
    struct transaction_record trans = {0};
    
    printf("\n🔄 REAL-TIME FUND TRANSFER\n");
    printf("Enter source account: ");
    scanf("%u", &from_account);
    printf("Enter destination account: ");
    scanf("%u", &to_account);
    
    if (from_account == to_account) {
        printf("❌ Cannot transfer to same account.\n");
        return;
    }
    
    // Validate PIN
    validate_pin(from_account);
    
    // Read source account
    fseek(f_ptr, (from_account - 1) * sizeof(struct client_data), SEEK_SET);
    fread(&from_client, sizeof(struct client_data), 1, f_ptr);
    
    // Read destination account
    fseek(f_ptr, (to_account - 1) * sizeof(struct client_data), SEEK_SET);
    fread(&to_client, sizeof(struct client_data), 1, f_ptr);
    
    if (from_client.acct_num == 0 || to_client.acct_num == 0) {
        printf("❌ One or both accounts not found.\n");
        return;
    }
    
    printf("From: %s %s (Balance: $%.2f)\n", 
           from_client.first_name, from_client.last_name, from_client.balance);
    printf("To: %s %s\n", to_client.first_name, to_client.last_name);
    printf("Enter transfer amount: $");
    scanf("%lf", &amount);
    
    if (amount <= 0 || amount > from_client.balance) {
        printf("❌ Invalid amount or insufficient funds.\n");
        return;
    }
    
    printf("🔄 Processing transfer...\n");
    display_transaction_progress("Fund Transfer");
    
    // Process transfer
    from_client.balance -= amount;
    to_client.balance += amount;
    
    // Update both accounts
    fseek(f_ptr, (from_account - 1) * sizeof(struct client_data), SEEK_SET);
    fwrite(&from_client, sizeof(struct client_data), 1, f_ptr);
    
    fseek(f_ptr, (to_account - 1) * sizeof(struct client_data), SEEK_SET);
    fwrite(&to_client, sizeof(struct client_data), 1, f_ptr);
    fflush(f_ptr);
    
    printf("✅ TRANSFER SUCCESSFUL!\n");
    printf("Transferred $%.2f from Account %u to Account %u\n", 
           amount, from_account, to_account);
    
    send_notification(from_account, "Transfer sent successfully");
    send_notification(to_account, "Transfer received");
}

// Display transaction processing animation
void display_transaction_progress(const char *operation) {
    printf("Processing %s", operation);
    for (int i = 0; i < 5; i++) {
        printf(".");
        fflush(stdout);
        usleep(300000); // 0.3 seconds
    }
    printf(" Done!\n");
}

// Fraud detection simulation
void fraud_detection_check(struct client_data *client, double amount) {
    if (amount > 10000) {
        printf("🛡️  Large transaction detected - Additional verification required\n");
        usleep(1000000); // 1 second delay
        printf("✅ Verification passed\n");
    }
    
    time_t current_time = time(NULL);
    if (current_time - client->last_transaction < 60) { // Less than 1 minute
        printf("🛡️  Rapid transactions detected - Security check in progress\n");
        usleep(500000);
        printf("✅ Security check passed\n");
    }
}

// PIN validation simulation
void validate_pin(unsigned int account_num) {
    int pin;
    printf("🔐 Enter PIN for account %u: ", account_num);
    scanf("%d", &pin);
    
    printf("🔄 Validating PIN...\n");
    usleep(800000); // 0.8 seconds
    printf("✅ PIN validated\n");
}

// Check daily limit
int check_daily_limit(struct client_data *client, double amount) {
    return (amount <= client->daily_limit);
}

// Send notification simulation
void send_notification(unsigned int account_num, const char *message) {
    printf("📱 SMS/Email sent to account %u: %s\n", account_num, message);
}

// Log transaction to file
void log_transaction(struct transaction_record *trans) {
    FILE *log_file = fopen("transaction_log.txt", "a");
    if (log_file != NULL) {
        fprintf(log_file, "%u|%u|%s|%.2f|%s|%s\n",
                trans->transaction_id, trans->account_num, trans->transaction_type,
                trans->amount, trans->status, ctime(&trans->timestamp));
        fclose(log_file);
    }
}

// Display real-time system status
void display_real_time_status(void) {
    time_t current_time = time(NULL);
    printf("\n🟢 System Status: ONLINE | Time: %s", ctime(&current_time));
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

// Enhanced menu
unsigned int enter_choice(void) {
    unsigned int menuChoice;
    
    printf("\n🏦 REAL-TIME BANKING SYSTEM\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("1 - 🔍 Real-time Balance Inquiry\n");
    printf("2 - 💰 Process Deposit\n");
    printf("3 - 💸 Process Withdrawal\n");
    printf("4 - 🔄 Fund Transfer\n");
    printf("5 - 📊 Transaction History\n");
    printf("6 - ⚡ Bulk Transaction Processing\n");
    printf("7 - 🔒 Lock Account\n");
    printf("8 - 🔓 Unlock Account\n");
    printf("9 - 🚪 Exit System\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Enter your choice: ");
    
    scanf("%u", &menuChoice);
    return menuChoice;
}

// Placeholder implementations for remaining functions
void real_time_transaction(FILE *f_ptr) {
    printf("⚡ Bulk transaction processing feature - Coming soon!\n");
}

void view_transaction_history(FILE *f_ptr) {
    printf("📊 Transaction history feature - Coming soon!\n");
}

void lock_account(FILE *f_ptr, unsigned int account_num) {
    printf("🔒 Account locking feature - Coming soon!\n");
}

void unlock_account(FILE *f_ptr, unsigned int account_num) {
    printf("🔓 Account unlocking feature - Coming soon!\n");
}