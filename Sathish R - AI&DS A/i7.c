#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // For isprint()

// Function Prototypes
void sortOption(FILE *fPtr);
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void sortAccounts(FILE *fPtr, int criterion, int ascending);  // Sort function prototype

// clientData structure definition
struct clientData {
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
}; // end structure clientData

// Global variable to hold sorting order (ascending or descending)
int ascending_order = 1; // 1 for ascending, 0 for descending

// Function to clean up and sanitize names (strip out non-printable characters)
void sanitizeString(char *str, int maxLength) {
    int i, j = 0;
    for (i = 0; i < maxLength && str[i] != '\0'; i++) {
        if (isprint(str[i])) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0'; // Null terminate the cleaned string
}

// Function to compare two accounts by balance (for low-to-high or high-to-low)
int compareByBalance(const void *a, const void *b) {
    struct clientData *accountA = (struct clientData *)a;
    struct clientData *accountB = (struct clientData *)b;

    if (ascending_order) {
        return (accountA->balance > accountB->balance) - (accountA->balance < accountB->balance);
    } else {
        return (accountB->balance > accountA->balance) - (accountB->balance < accountA->balance);
    }
}

// Function to compare two accounts by name (alphabetical order)
int compareByName(const void *a, const void *b) {
    struct clientData *accountA = (struct clientData *)a;
    struct clientData *accountB = (struct clientData *)b;
    
    int cmpLast = strcmp(accountA->lastName, accountB->lastName);
    
    if (cmpLast == 0) {
        return (ascending_order ? strcmp(accountA->firstName, accountB->firstName) : strcmp(accountB->firstName, accountA->firstName));
    }
    
    return (ascending_order ? cmpLast : -cmpLast);
}

// Function to handle the sort option (after user selects '5' for sorting)
void sortOption(FILE *fPtr) {
    int criterion, ascending;

    // Ask the user for the sorting criterion
    printf("\nChoose sorting criterion:\n");
    printf("1 - Sort by Balance\n");
    printf("2 - Sort by Last Name and First Name\n");
    printf("3 - Show Account with Maximum Balance\n");
    printf("4 - Show Account with Minimum Balance\n");
    printf("Enter your choice: ");
    scanf("%d", &criterion);

    if (criterion == 3 || criterion == 4) {
        // For min/max options, we don't need sorting order
        sortAccounts(fPtr, criterion, 1);
        return;
    }

    // Ask the user for ascending or descending order (only for sorting options 1 & 2)
    printf("\nChoose sorting order:\n");
    printf("1 - Ascending\n");
    printf("2 - Descending\n");
    printf("Enter your choice: ");
    scanf("%d", &ascending);

    // Convert input to 0 or 1 for ascending or descending
    ascending = (ascending == 1) ? 1 : 0;

    // Call sortAccounts with the selected criterion and order
    sortAccounts(fPtr, criterion, ascending);
}

// Main function
int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        printf("%s: File could not be opened.\n", argv[0]);
        exit(-1);
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 6)  // CHANGED: from 5 to 6
    {
        switch (choice)
        {
        case 1:  // create text file from record file
            textFile(cfPtr);
            break;
        case 2:  // update record
            updateRecord(cfPtr);
            break;
        case 3:  // create record
            newRecord(cfPtr);
            break;
        case 4:  // delete existing record
            deleteRecord(cfPtr);
            break;
        case 5:  // sort accounts by balance or other criteria
            sortOption(cfPtr);  // Handle sort option
            break;
        default:
            puts("Incorrect choice");
            break;
        }
    }

    fclose(cfPtr); // fclose closes the file
} // end main

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - sort accounts by balance or other criteria\n"  // NEW: Sort option
                 "6 - end program\n? ");           // CHANGED: from 5 to 6

    scanf("%u", &menuChoice); // receive choice from user
    return menuChoice;
} // end function enterChoice

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (!feof(readPtr))
        {
            result = fread(&client, sizeof(struct clientData), 1, readPtr);

            // write single record to text file
            if (result != 0 && client.acctNum != 0)
            {
                // Sanitize the data before writing to text file
                sanitizeString(client.lastName, 15);
                sanitizeString(client.firstName, 10);
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    scanf("%d", &account);

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        scanf("%lf", &transaction);
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        // move back by 1 record length
        fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    scanf("%d", &accountNum);

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance);

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
    } // end else
} // end function newRecord

// Enhanced sortAccounts function with all features
void sortAccounts(FILE *fPtr, int criterion, int ascending) {
    struct clientData allAccounts[100];  // Array to hold all accounts from file
    struct clientData validAccounts[100]; // Array to hold only valid accounts
    int count = 0;
    int i;

    // Store the ascending order
    ascending_order = ascending;

    // Read all accounts from file
    rewind(fPtr);
    for (i = 0; i < 100; i++) {
        fread(&allAccounts[i], sizeof(struct clientData), 1, fPtr);
    }

    // Extract only valid accounts (acctNum != 0 and within valid range)
    for (i = 0; i < 100; i++) {
        if (allAccounts[i].acctNum != 0 && 
            allAccounts[i].acctNum >= 1 && 
            allAccounts[i].acctNum <= 100 &&
            allAccounts[i].balance >= -1000000.0 && 
            allAccounts[i].balance <= 10000000.0) {
            
            // Sanitize the names to ensure they are clean
            sanitizeString(allAccounts[i].lastName, 15);
            sanitizeString(allAccounts[i].firstName, 10);
            validAccounts[count] = allAccounts[i];
            count++;
        }
    }

    if (count == 0) {
        printf("No accounts found.\n");
        return;
    }

    // Perform the sorting based on the criterion
    switch (criterion) {
        case 1:  // Sort by Balance
            qsort(validAccounts, count, sizeof(struct clientData), compareByBalance);
            break;
        case 2:  // Sort by Last Name and First Name
            qsort(validAccounts, count, sizeof(struct clientData), compareByName);
            break;
        case 3:  // Show Account with Maximum Balance
            {
                struct clientData *maxBalance = &validAccounts[0];
                for (i = 1; i < count; i++) {
                    if (validAccounts[i].balance > maxBalance->balance) {
                        maxBalance = &validAccounts[i];
                    }
                }
                printf("\n%-6s%-16s%-11s%-15s\n", "Acct", "Last Name", "First Name", "Balance");
                printf("====================================================\n");
                printf("Account with MAXIMUM balance:\n");
                printf("%-6d%-16s%-11s%-15.2f\n",
                    maxBalance->acctNum, maxBalance->lastName, maxBalance->firstName, maxBalance->balance);
                return;
            }
        case 4:  // Show Account with Minimum Balance
            {
                struct clientData *minBalance = &validAccounts[0];
                for (i = 1; i < count; i++) {
                    if (validAccounts[i].balance < minBalance->balance) {
                        minBalance = &validAccounts[i];
                    }
                }
                printf("\n%-6s%-16s%-11s%-15s\n", "Acct", "Last Name", "First Name", "Balance");
                printf("====================================================\n");
                printf("Account with MINIMUM balance:\n");
                printf("%-6d%-16s%-11s%-15.2f\n",
                    minBalance->acctNum, minBalance->lastName, minBalance->firstName, minBalance->balance);
                return;
            }
        default:
            printf("Invalid sorting criterion!\n");
            return;
    }

    // Print sorted accounts with proper alignment (only for cases 1 and 2)
    printf("\n%-6s%-16s%-11s%-15s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("====================================================\n");
    for (i = 0; i < count; i++) {
        printf("%-6d%-16s%-11s%-15.2f\n", 
               validAccounts[i].acctNum, 
               validAccounts[i].lastName, 
               validAccounts[i].firstName,
               validAccounts[i].balance);
    }
}