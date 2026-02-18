#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clientData structure definition
struct clientData {
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void transferFunds(FILE *fPtr); // Kept this feature

int main(int argc, char *argv[]) {
    FILE *cfPtr;             // credit.dat file pointer
    unsigned int choice;     // user's choice

    // Open file. 
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        // Try creating it if it doesn't exist
        if ((cfPtr = fopen("credit.dat", "wb+")) == NULL) {
             printf("File could not be opened.\n");
             exit(-1);
        }
    }

    // CHANGED: Loop ends at 6 now, since we removed one option
    while ((choice = enterChoice()) != 6) {
        switch (choice) {
            case 1: textFile(cfPtr); break;
            case 2: updateRecord(cfPtr); break;
            case 3: newRecord(cfPtr); break;
            case 4: deleteRecord(cfPtr); break;
            case 5: transferFunds(cfPtr); break;
            default: puts("Incorrect choice"); break;
        }
    }

    fclose(cfPtr);
    return 0;
}

// 1. Store formatted text file
void textFile(FILE *readPtr) {
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL) {
        puts("File could not be opened.");
    } else {
        rewind(readPtr);
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        while (!feof(readPtr)) {
            size_t result = fread(&client, sizeof(struct clientData), 1, readPtr);
            if (result != 0 && client.acctNum != 0) {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
                        client.acctNum, client.lastName, client.firstName, client.balance);
            }
        }
        fclose(writePtr);
        puts("Data exported to accounts.txt successfully.");
    }
}

// 2. Update account (With Overdraft Protection)
void updateRecord(FILE *fPtr) {
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account #%d has no information.\n", account);
    } else {
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);

        if (client.balance + transaction < 0) {
            printf("Error: Insufficient funds. Transaction denied.\n");
        } else {
            client.balance += transaction;
            printf("New Balance: %10.2f\n", client.balance);
            
            fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
        }
    }
}

// 3. Create new record
void newRecord(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) {
        printf("Account #%d already contains information.\n", client.acctNum);
    } else {
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);
        client.acctNum = accountNum;
        
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// 4. Delete record
void deleteRecord(FILE *fPtr) {
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0};
    unsigned int accountNum;

    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        printf("Account %d does not exist.\n", accountNum);
    } else {
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        printf("Account %d deleted.\n", accountNum);
    }
}

// 5. Transfer Funds
void transferFunds(FILE *fPtr) {
    unsigned int senderID, receiverID;
    double amount;
    struct clientData sender = {0, "", "", 0.0};
    struct clientData receiver = {0, "", "", 0.0};

    printf("Enter Sender Account #: ");
    scanf("%d", &senderID);
    printf("Enter Receiver Account #: ");
    scanf("%d", &receiverID);
    printf("Enter Amount to Transfer: ");
    scanf("%lf", &amount);

    // 1. Retrieve Sender
    fseek(fPtr, (senderID - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&sender, sizeof(struct clientData), 1, fPtr);

    // 2. Retrieve Receiver
    fseek(fPtr, (receiverID - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&receiver, sizeof(struct clientData), 1, fPtr);

    // 3. Validate Transaction
    if (sender.acctNum == 0) {
        printf("Error: Sender account %d does not exist.\n", senderID);
    } else if (receiver.acctNum == 0) {
        printf("Error: Receiver account %d does not exist.\n", receiverID);
    } else if (sender.balance < amount) {
        printf("Error: Insufficient funds in Sender account.\n");
    } else {
        // 4. Perform Calculation
        sender.balance -= amount;
        receiver.balance += amount;

        // 5. Write Sender Back
        fseek(fPtr, (senderID - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&sender, sizeof(struct clientData), 1, fPtr);

        // 6. Write Receiver Back
        fseek(fPtr, (receiverID - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&receiver, sizeof(struct clientData), 1, fPtr);

        printf("Success: Transferred %.2f from #%d to #%d\n", amount, senderID, receiverID);
    }
}

unsigned int enterChoice(void) {
    unsigned int menuChoice;
    printf("%s", "\nEnter your choice\n"
                 "1 - Store formatted text file (accounts.txt)\n"
                 "2 - Update an account\n"
                 "3 - Add a new account\n"
                 "4 - Delete an account\n"
                 "5 - Transfer Funds\n"
                 "6 - End program\n? "); // Renumbered to 6
    scanf("%u", &menuChoice);
    return menuChoice;
}