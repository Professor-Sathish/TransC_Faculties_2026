#include <stdio.h>
#include <stdlib.h>

// ---------------- STRUCTURES ----------------
struct clientData
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

struct transactionData
{
    unsigned int acctNum;
    double amount;
    double balanceAfter;
};

// ---------------- FUNCTION PROTOTYPES ----------------
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void viewTransactions(void);

// ---------------- MAIN ----------------
int main(int argc, char *argv[])
{
    FILE *cfPtr;
    unsigned int choice;

    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        // create file if not exists
        cfPtr = fopen("credit.dat", "wb+");
        if (cfPtr == NULL)
        {
            puts("File could not be opened.");
            exit(1);
        }

        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < 100; i++)
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
    }

    while ((choice = enterChoice()) != 6)
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
        case 5:
            viewTransactions();
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(cfPtr);
    return 0;
}

// ---------------- FUNCTIONS ----------------

// create formatted text file
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr))
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n",
                    client.acctNum, client.lastName,
                    client.firstName, client.balance);
        }
    }

    fclose(writePtr);
    puts("accounts.txt generated successfully.");
}

// update record + transaction history
void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    printf("Enter account to update (1 - 100): ");
    scanf("%u", &account);

    if (account < 1 || account > 100)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("%-6d%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName,
           client.firstName, client.balance);

    printf("Enter charge (+) or payment (-): ");
    scanf("%lf", &transaction);

    if (client.balance + transaction < 0)
    {
        puts("Insufficient balance. Transaction denied.");
        return;
    }

    client.balance += transaction;

    fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    // ----- TRANSACTION LOG -----
    struct transactionData t;
    t.acctNum = client.acctNum;
    t.amount = transaction;
    t.balanceAfter = client.balance;

    FILE *tPtr = fopen("transactions.dat", "ab");
    fwrite(&t, sizeof(struct transactionData), 1, tPtr);
    fclose(tPtr);

    puts("Transaction successful.");
}

// delete record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;
    struct clientData blank = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter account number to delete (1 - 100): ");
    scanf("%u", &account);

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum == 0)
    {
        puts("Account does not exist.");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&blank, sizeof(struct clientData), 1, fPtr);

    puts("Account deleted successfully.");
}

// add new record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int account;

    printf("Enter new account number (1 - 100): ");
    scanf("%u", &account);

    if (account < 1 || account > 100)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0)
    {
        puts("Account already exists.");
        return;
    }

    printf("Enter lastname firstname balance: ");
    scanf("%14s %9s %lf",
          client.lastName, client.firstName, &client.balance);

    client.acctNum = account;

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);

    puts("Account created successfully.");
}

// view transaction history
void viewTransactions(void)
{
    FILE *tPtr;
    struct transactionData t;

    if ((tPtr = fopen("transactions.dat", "rb")) == NULL)
    {
        puts("No transactions found.");
        return;
    }

    printf("\nAcct   Amount      Balance After\n");
    printf("--------------------------------\n");

    while (fread(&t, sizeof(struct transactionData), 1, tPtr))
    {
        printf("%-6d%-12.2f%.2f\n",
               t.acctNum, t.amount, t.balanceAfter);
    }

    fclose(tPtr);
}

// menu
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\nEnter your choice\n"
           "1 - store formatted accounts.txt\n"
           "2 - update an account\n"
           "3 - add a new account\n"
           "4 - delete an account\n"
           "5 - view transaction history\n"
           "6 - exit\n? ");

    scanf("%u", &choice);
    return choice;
}
