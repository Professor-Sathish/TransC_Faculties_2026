// Bank-account program using random-access file
#include <stdio.h>
#include <stdlib.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum;   // account number
    char lastName[15];      // last name
    char firstName[10];     // first name
    double balance;         // account balance
};

// function prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void viewRecord(FILE *fPtr);   // NEW FEATURE

int main(int argc, char *argv[])
{
    FILE *cfPtr;            // credit.dat file pointer
    unsigned int choice;    // user's choice

    // open file for read/write in binary mode
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(1);
    }

    // menu loop
    while ((choice = enterChoice()) != 5)
    {
        switch (choice)
        {
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
        case 6:
            viewRecord(cfPtr);   // NEW OPTION
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(cfPtr);   // close file
    return 0;
}

/* -------------------------------------------------- */
// create formatted text file
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    int result;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (!feof(readPtr))
    {
        result = fread(&client, sizeof(struct clientData), 1, readPtr);

        if (result && client.acctNum != 0)
        {
            fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
        }
    }

    fclose(writePtr);
    puts("accounts.txt created successfully.");
}

/* -------------------------------------------------- */
// update an existing record
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    printf("Enter account to update (1 - 100): ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", account);
    }
    else
    {
        printf("Current balance: %.2f\n", client.balance);
        printf("Enter charge (+) or payment (-): ");
        scanf("%lf", &transaction);

        client.balance += transaction;

        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);

        printf("Updated balance: %.2f\n", client.balance);
    }
}

/* -------------------------------------------------- */
// add a new record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter new account number (1 - 100): ");
    scanf("%u", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0)
    {
        printf("Account #%u already exists.\n", client.acctNum);
    }
    else
    {
        printf("Enter lastname firstname balance: ");
        scanf("%14s%9s%lf",
              client.lastName,
              client.firstName,
              &client.balance);

        client.acctNum = accountNum;

        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);

        puts("Account added successfully.");
    }
}

/* -------------------------------------------------- */
// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blankClient = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter account number to delete (1 - 100): ");
    scanf("%u", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
    }
    else
    {
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        puts("Account deleted successfully.");
    }
}

/* -------------------------------------------------- */
// NEW FEATURE: view a single account
void viewRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter account number to view (1 - 100): ");
    scanf("%u", &accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
    }
    else
    {
        printf("\nAccount Details\n");
        printf("-----------------------\n");
        printf("Account Number : %u\n", client.acctNum);
        printf("Last Name      : %s\n", client.lastName);
        printf("First Name     : %s\n", client.firstName);
        printf("Balance        : %.2f\n", client.balance);
    }
}

/* -------------------------------------------------- */
// menu function
unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n"
           "1 - store a formatted text file of accounts\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - end program\n"
           "6 - view a single account\n? ");

    scanf("%u", &menuChoice);
    return menuChoice;
}
