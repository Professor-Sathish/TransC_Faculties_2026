#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "credit.dat"
#define MAX_RECORDS 100

/* -------- Structure Definition -------- */
struct clientData {
    char acctNum[21];
    char lastName[15];
    char firstName[10];
    double balance;
};

/* -------- Function Prototypes -------- */
void initializeFile(void);
void menu(void);
void addRecord(FILE *fPtr);
void updateRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayRecords(FILE *fPtr);
void searchByLastName(FILE *fPtr);
void exportToText(FILE *fPtr);

int findAccount(FILE *fPtr, const char *acctNum, struct clientData *client);

/* -------- Main Function -------- */
int main(void) {
    FILE *fPtr;
    int choice;

    initializeFile();

    fPtr = fopen(FILE_NAME, "rb+");
    if (fPtr == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    do {
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: exportToText(fPtr); break;
            case 2: updateRecord(fPtr); break;
            case 3: addRecord(fPtr); break;
            case 4: deleteRecord(fPtr); break;
            case 5: displayRecords(fPtr); break;
            case 6: searchByLastName(fPtr); break;
            case 7: printf("Exiting program...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 7);

    fclose(fPtr);
    return 0;
}

/* -------- Menu -------- */
void menu(void) {
    printf("\n===== BANK APPLICATION MENU =====\n");
    printf("1. Export accounts to text file\n");
    printf("2. Update an account\n");
    printf("3. Add a new account\n");
    printf("4. Delete an account\n");
    printf("5. Display all accounts\n");
    printf("6. Search by last name\n");
    printf("7. Exit\n");
}

/* -------- Initialize File -------- */
void initializeFile(void) {
    FILE *fPtr = fopen(FILE_NAME, "rb");

    if (fPtr == NULL) {
        struct clientData blank = {"", "", "", 0.0};
        fPtr = fopen(FILE_NAME, "wb");

        for (int i = 0; i < MAX_RECORDS; i++) {
            fwrite(&blank, sizeof(struct clientData), 1, fPtr);
        }
    }
    fclose(fPtr);
}

/* -------- Find Account (Reusable) -------- */
int findAccount(FILE *fPtr, const char *acctNum, struct clientData *client) {
    rewind(fPtr);
    while (fread(client, sizeof(*client), 1, fPtr)) {
        if (strcmp(client->acctNum, acctNum) == 0) {
            return 1;
        }
    }
    return 0;
}

/* -------- Add Record -------- */
void addRecord(FILE *fPtr) {
    struct clientData client;
    char acctNum[21];

    printf("Enter account number: ");
    scanf("%s", acctNum);

    if (findAccount(fPtr, acctNum, &client)) {
        printf("Account already exists.\n");
        return;
    }

    rewind(fPtr);
    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strlen(client.acctNum) == 0) {
            strcpy(client.acctNum, acctNum);
            printf("Enter first name: ");
            scanf("%s", client.firstName);
            printf("Enter last name: ");
            scanf("%s", client.lastName);
            printf("Enter balance: ");
            scanf("%lf", &client.balance);

            fseek(fPtr, -sizeof(client), SEEK_CUR);
            fwrite(&client, sizeof(client), 1, fPtr);

            printf("Account added successfully.\n");
            return;
        }
    }
    printf("No space available for new records.\n");
}

/* -------- Update Record -------- */
void updateRecord(FILE *fPtr) {
    struct clientData client;
    char acctNum[21];
    double amount;

    printf("Enter account number to update: ");
    scanf("%s", acctNum);

    rewind(fPtr);
    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strcmp(client.acctNum, acctNum) == 0) {
            printf("Current balance: %.2lf\n", client.balance);
            printf("Enter amount (+deposit / -withdraw): ");
            scanf("%lf", &amount);

            client.balance += amount;

            fseek(fPtr, -sizeof(client), SEEK_CUR);
            fwrite(&client, sizeof(client), 1, fPtr);

            printf("Account updated successfully.\n");
            return;
        }
    }
    printf("Account not found.\n");
}

/* -------- Delete Record -------- */
void deleteRecord(FILE *fPtr) {
    struct clientData client;
    struct clientData blank = {"", "", "", 0.0};
    char acctNum[21];

    printf("Enter account number to delete: ");
    scanf("%s", acctNum);

    rewind(fPtr);
    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strcmp(client.acctNum, acctNum) == 0) {
            fseek(fPtr, -sizeof(client), SEEK_CUR);
            fwrite(&blank, sizeof(blank), 1, fPtr);

            printf("Account deleted successfully.\n");
            return;
        }
    }
    printf("Account not found.\n");
}

/* -------- Display Records -------- */
void displayRecords(FILE *fPtr) {
    struct clientData client;

    rewind(fPtr);
    printf("\n%-20s %-10s %-15s %-10s\n",
           "Account No", "First", "Last", "Balance");

    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strlen(client.acctNum) != 0) {
            printf("%-20s %-10s %-15s %.2lf\n",
                   client.acctNum, client.firstName,
                   client.lastName, client.balance);
        }
    }
}

/* -------- Search by Last Name -------- */
void searchByLastName(FILE *fPtr) {
    struct clientData client;
    char lname[15];
    int found = 0;

    printf("Enter last name to search: ");
    scanf("%s", lname);

    rewind(fPtr);
    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strcmp(client.lastName, lname) == 0) {
            printf("%s %s %s %.2lf\n",
                   client.acctNum, client.firstName,
                   client.lastName, client.balance);
            found = 1;
        }
    }
    if (!found)
        printf("No records found.\n");
}

/* -------- Export to Text File -------- */
void exportToText(FILE *fPtr) {
    FILE *txtPtr = fopen("accounts.txt", "w");
    struct clientData client;

    rewind(fPtr);
    fprintf(txtPtr, "%-20s %-10s %-15s %-10s\n",
            "Account No", "First", "Last", "Balance");

    while (fread(&client, sizeof(client), 1, fPtr)) {
        if (strlen(client.acctNum) != 0) {
            fprintf(txtPtr, "%-20s %-10s %-15s %.2lf\n",
                    client.acctNum, client.firstName,
                    client.lastName, client.balance);
        }
    }
    fclose(txtPtr);
    printf("Accounts exported to accounts.txt\n");
}
