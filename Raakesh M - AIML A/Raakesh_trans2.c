// Bank-account program with random-access files + timestamp logging
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ACCOUNTS 100

// clientData structure definition
struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void logTransaction(const char *action, unsigned int acctNum);

int main(int argc, char *argv[])
{
    FILE *cfPtr = fopen("credit.dat", "rb+");
    unsigned int choice;

    if (!cfPtr) {
        printf("%s: File could not be opened.\n", argv[0]);
        return EXIT_FAILURE;
    }

    while ((choice = enterChoice()) != 5) {
        switch (choice) {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        default:
            puts("Incorrect choice");
        }
    }

    fclose(cfPtr);
    return EXIT_SUCCESS;
}

// ---------- Logging with timestamp ----------
void logTransaction(const char *action, unsigned int acctNum)
{
    FILE *logPtr = fopen("transactions.log", "a");
    if (!logPtr) return;

    time_t now = time(NULL);
    char timeStr[64];

    strftime(timeStr, sizeof(timeStr),
             "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(logPtr, "[%s] %s - Account #%u\n",
            timeStr, action, acctNum);

    fclose(logPtr);
}

// ---------- Create formatted text file ----------
void textFile(FILE *readPtr)
{
    FILE *writePtr = fopen("accounts.txt", "w");
    struct clientData client;

    if (!writePtr) {
        puts("accounts.txt could not be opened.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);
    puts("accounts.txt created successfully.");
}

// ---------- Update record ----------
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client;

    printf("Enter account to update (1 - 100): ");
    if (scanf("%u", &account) != 1 || account < 1 || account > MAX_ACCOUNTS) {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);

    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 ||
        client.acctNum == 0) {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("%-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName,
           client.firstName, client.balance);

    printf("Enter charge (+) or payment (-): ");
    if (scanf("%lf", &transaction) != 1) {
        puts("Invalid transaction.");
        return;
    }

    client.balance += transaction;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    logTransaction("UPDATED", account);
    puts("Record updated successfully.");
}

// ---------- Delete record ----------
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter account number to delete (1 - 100): ");
    if (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);

    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 ||
        client.acctNum == 0) {
        printf("Account #%u does not exist.\n", accountNum);
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);

    logTransaction("DELETED", accountNum);
    puts("Record deleted successfully.");
}

// ---------- Create new record ----------
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter new account number (1 - 100): ");
    if (scanf("%u", &accountNum) != 1 || accountNum < 1 || accountNum > MAX_ACCOUNTS) {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) {
        printf("Account #%u already exists.\n", accountNum);
        return;
    }

    printf("Enter lastname firstname balance:\n? ");
    if (scanf("%14s %9s %lf",
              client.lastName,
              client.firstName,
              &client.balance) != 3) {
        puts("Invalid input.");
        return;
    }

    client.acctNum = accountNum;

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    logTransaction("CREATED", accountNum);
    puts("New record added successfully.");
}

// ---------- Menu ----------
unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n"
           "1 - store a formatted text file of accounts\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1)
        return 0;

    return menuChoice;
}

