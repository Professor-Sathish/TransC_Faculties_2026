#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void initializeFile(void);
int login(void);

int main() {
    FILE *cfPtr;
    unsigned int choice;

    // PASSWORD AUTHENTICATION
    if (!login()) {
        printf("Access Denied. Program Terminated.\n");
        return 0;
    }

    initializeFile();

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        puts("File could not be opened.");
        exit(1);
    }

    while ((choice = enterChoice()) != 5) {
        switch (choice) {
        case 1: textFile(cfPtr); break;
        case 2: updateRecord(cfPtr); break;
        case 3: newRecord(cfPtr); break;
        case 4: deleteRecord(cfPtr); break;
        default: puts("Invalid choice");
        }
    }

    fclose(cfPtr);
    return 0;
}

// ---------------- PASSWORD FUNCTION ----------------
int login(void) {
    char username[20];
    char password[20];
    int attempts = 3;

    while (attempts--) {
        printf("\nUsername: ");
        scanf("%19s", username);
        printf("Password: ");
        scanf("%19s", password);

        if (strcmp(username, "admin") == 0 &&
            strcmp(password, "bank123") == 0) {
            printf("Login successful!\n");
            return 1;
        } else {
            printf("Invalid credentials. Attempts left: %d\n", attempts);
        }
    }
    return 0;
}

// ---------------- INITIALIZE FILE ----------------
void initializeFile(void) {
    FILE *fp;
    struct clientData blank = {0, "", "", 0.0};

    if ((fp = fopen("credit.dat", "rb")) == NULL) {
        fp = fopen("credit.dat", "wb");
        for (int i = 0; i < 100; i++) {
            fwrite(&blank, sizeof(struct clientData), 1, fp);
        }
        fclose(fp);
    } else {
        fclose(fp);
    }
}

// ---------------- CREATE TEXT FILE ----------------
void textFile(FILE *readPtr) {
    FILE *writePtr;
    struct clientData client;

    writePtr = fopen("accounts.txt", "w");
    rewind(readPtr);

    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr)) {
        if (client.acctNum != 0) {
            fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
                    client.acctNum, client.lastName,
                    client.firstName, client.balance);
        }
    }
    fclose(writePtr);
    puts("accounts.txt created");
}

// ---------------- UPDATE RECORD ----------------
void updateRecord(FILE *fPtr) {
    struct clientData client;
    unsigned int account;
    double transaction;

    printf("Enter account number: ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        puts("Account does not exist.");
    } else {
        printf("Balance: %.2f\n", client.balance);
        printf("Enter transaction amount: ");
        scanf("%lf", &transaction);

        client.balance += transaction;

        fseek(fPtr, -long sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// ---------------- NEW RECORD ----------------
void newRecord(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter new account number: ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) {
        puts("Account already exists.");
    } else {
        printf("Enter lastname firstname balance: ");
        scanf("%14s %9s %lf",
              client.lastName, client.firstName, &client.balance);

        client.acctNum = account;
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    }
}

// ---------------- DELETE RECORD ----------------
void deleteRecord(FILE *fPtr) {
    struct clientData client;
    struct clientData blank = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter account to delete: ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0) {
        puts("Account does not exist.");
    } else {
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blank, sizeof(struct clientData), 1, fPtr);
        puts("Account deleted.");
    }
}

// ---------------- MENU ----------------
unsigned int enterChoice(void) {
    unsigned int choice;
    printf("\n1 - Create accounts.txt\n"
           "2 - Update account\n"
           "3 - Add account\n"
           "4 - Delete account\n"
           "5 - Exit\n"
           "Enter choice: ");
    scanf("%u", &choice);
    return choice;
}
