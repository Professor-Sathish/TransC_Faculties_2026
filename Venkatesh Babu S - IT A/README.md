# Code By Venkatesh 
# Added a New Fuction to display the First Name, Last Name and Account Number
# function prototype
     void displayDetails(FILE *readPtr);   
# Fucntion Body
void displayDetails(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};

    rewind(fPtr); // go to beginning of file

    printf("\n%-6s %-16s %-11s\n", "Acct", "Last Name", "First Name");
    printf("----------------------------------\n");

    while (fread(&client, sizeof(struct clientData), 1, fPtr))
    {
        if (client.acctNum != 0)
        {
            printf("%-6u %-16s %-11s\n",
                   client.acctNum,
                   client.lastName,
                   client.firstName);
        }
    }
}
# Additional Modification 
    @ Choice Selection 5 - display account details\n"
    @ Main()
        case 5:
            displayDetails(cfPtr);
            break;
