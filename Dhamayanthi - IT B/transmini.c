#include <stdio.h>
#include <stdlib.h>

/* Structure */
struct climini
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

/* Function prototypes */
unsigned int menu(void);
void textFile(FILE *);
void updateRecord(FILE *);
void newRecord(FILE *);
void deleteRecord(FILE *);
void applyInterest(FILE *);

int main()
{
    FILE *creminiPtr;

    /* Open file in read/write binary mode */
    if ((creminiPtr = fopen("cremini.dat", "rb+")) == NULL)
    {
        /* Create file if it doesn't exist */
        creminiPtr = fopen("cremini.dat", "wb+");
        if (creminiPtr == NULL)
        {
            printf("File could not be opened.\n");
            exit(1);
        }
    }

    unsigned int choice;

    while ((choice = menu()) != 6)
    {
        switch (choice)
        {
        case 1: textFile(creminiPtr); break;
        case 2: updateRecord(creminiPtr); break;
        case 3: newRecord(creminiPtr); break;
        case 4: deleteRecord(creminiPtr); break;
        case 5: applyInterest(creminiPtr); break;
        default: printf("Invalid choice\n");
        }
    }

    fclose(creminiPtr);
    return 0;
}

/* Menu */
unsigned int menu(void)
{
    unsigned int choice;
    printf("\n--- BANK MENU ---\n");
    printf("1. Create text file\n");
    printf("2. Update account\n");
    printf("3. Add new account\n");
    printf("4. Delete account\n");
    printf("5. Apply interest\n");
    printf("6. Exit\n");
    printf("Enter choice: ");
    scanf("%u", &choice);
    return choice;
}

/* Create text file from binary */
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct climini rec = {0};

    writePtr = fopen("accmini.txt", "w");
    rewind(readPtr);

    fprintf(writePtr, "%-6s %-15s %-10s %10s\n",
            "Acct", "Last Name", "First Name", "Balance");

    while (fread(&rec, sizeof(rec), 1, readPtr))
    {
        if (rec.acctNum != 0)
        {
            fprintf(writePtr, "%-6d %-15s %-10s %10.2f\n",
                    rec.acctNum, rec.lastName, rec.firstName, rec.balance);
        }
    }

    fclose(writePtr);
    printf("accmini.txt created successfully\n");
}

/* Update account */
void updateRecord(FILE *fPtr)
{
    struct climini rec = {0};
    unsigned int acc;
    double amount;

    printf("Enter account number: ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fread(&rec, sizeof(rec), 1, fPtr);

    if (rec.acctNum == 0)
    {
        printf("Account not found\n");
        return;
    }

    printf("Current balance: %.2f\n", rec.balance);
    printf("Enter amount (+deposit / -withdraw): ");
    scanf("%lf", &amount);

    rec.balance += amount;

    fseek(fPtr, -sizeof(rec), SEEK_CUR);
    fwrite(&rec, sizeof(rec), 1, fPtr);

    printf("Account updated successfully\n");
}

/* Add new account */
void newRecord(FILE *fPtr)
{
    struct climini rec = {0};
    unsigned int acc;

    printf("Enter new account number: ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fread(&rec, sizeof(rec), 1, fPtr);

    if (rec.acctNum != 0)
    {
        printf("Account already exists\n");
        return;
    }

    printf("Enter Last Name, First Name and Balance: ");
    scanf("%14s %9s %lf", rec.lastName, rec.firstName, &rec.balance);

    rec.acctNum = acc;

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fwrite(&rec, sizeof(rec), 1, fPtr);

    printf("New account added successfully\n");
}

/* Delete account */
void deleteRecord(FILE *fPtr)
{
    struct climini rec = {0};
    struct climini blank = {0};
    unsigned int acc;

    printf("Enter account number to delete: ");
    scanf("%u", &acc);

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fread(&rec, sizeof(rec), 1, fPtr);

    if (rec.acctNum == 0)
    {
        printf("Account does not exist\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fwrite(&blank, sizeof(blank), 1, fPtr);

    printf("Account deleted successfully\n");
}

/* Apply interest */
void applyInterest(FILE *fPtr)
{
    struct climini rec = {0};
    unsigned int acc;
    double rate;

    printf("Enter account number: ");
    scanf("%u", &acc);

    printf("Enter interest rate (%%): ");
    scanf("%lf", &rate);

    fseek(fPtr, (acc - 1) * sizeof(rec), SEEK_SET);
    fread(&rec, sizeof(rec), 1, fPtr);

    if (rec.acctNum == 0)
    {
        printf("Account not found\n");
        return;
    }

    rec.balance += rec.balance * rate / 100;

    fseek(fPtr, -sizeof(rec), SEEK_CUR);
    fwrite(&rec, sizeof(rec), 1, fPtr);

    printf("Interest applied successfully\n");
}