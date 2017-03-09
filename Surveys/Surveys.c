/*
* Keith Williams
* 12/03/2016
* This program will create a database (singe linked-list) which will store the details of all the surveys carried out by Custom-Surveys Ltd.
* The database is able to save, restore, retrieve, add, delete and update survey details.
* In addition to storing all the survey data the application should allow survey statistics to be generated.
* This system is password protected.
*/

#include<stdio.h>
#include<conio.h>
#include<stdlib.h>

// Define constants
#define MAX_STRING_LENGTH 155
#define MAX_CREDINTIAL_LENGTH 7
#define MAX_LOGIN_ATTEMPTS 3
#define FILE_NAME "surveys.txt"

typedef enum { false, true } bool;

// A struct to represent survey details
// Added a gender property to the survey struct which is needed for generating statistics
typedef struct {
	int pps;
	int gender;
	char firstName[MAX_STRING_LENGTH];
	char secondName[MAX_STRING_LENGTH];
	char address[MAX_STRING_LENGTH];
	char email[MAX_STRING_LENGTH];
	int ageBracket;
	int incomeBracket;
	int exerciseBracket;
	int alcoholBracket;
	int cigarettesBracket;
} survey_t;

// A struct to represent a survey element in t linked list
typedef struct {
	survey_t survey;
	survey_t* next;
} surveyelement_t;

// A struct to store each users credentials
typedef struct {
	char username[MAX_CREDINTIAL_LENGTH];
	char password[MAX_CREDINTIAL_LENGTH];
} user_t;

// A struct to represent a user element in t linked list
typedef struct {
	user_t user;
	user_t* next;
} userelement_t;

// Function prototypes
bool login();
void getPassword(char* password);
void addSurvey(surveyelement_t** head);
void displayAllSurveys(surveyelement_t* head);
void displaySpecificSurvey(surveyelement_t* head);
void displaySurveyDetails(surveyelement_t* element, FILE* fp);
void updateSurvey(surveyelement_t** head);
void deleteSurvey(surveyelement_t** head);
void printStatistics(surveyelement_t* head, FILE* fp);
void generateReportFile(surveyelement_t* head);
void generateStatistics(surveyelement_t* head, float statistics[][12]);
void add(surveyelement_t** head, surveyelement_t* element);
surveyelement_t* erase(surveyelement_t** head, int index);
int searchByPPSOrName(surveyelement_t* head);
int searchByPPS(surveyelement_t* head, int pps);
int getNewIndex(surveyelement_t* head, int pps);
void initialiseLinkedList(surveyelement_t** head);
void updateFile(surveyelement_t* head);
bool verifyEmail(char* email);
void getString(char* str, char* message);
void getInt(int* num, char* message);

// The main function displays a menu to the user if they login correctly
// If they they fail to login correctly the program will terminate
// The menu loop will end when the user enters a sentinel value
// The program will then exit after calling a function to update the survey.txt file
void main() {
	int option;
	int ret;
	surveyelement_t* surveys = NULL;

	if (login() == true) {
		initialiseLinkedList(&surveys);

		do {
			// Print menu
			printf("1) Add a survey\n");
			printf("2) Display all surveys\n");
			printf("3) Display survey details\n");
			printf("4) Update survey\n");
			printf("5) Delete survey\n");
			printf("6) Generate Statistics\n");
			printf("7) Print all survey details to a report file\n");
			printf("8) Exit\n");

			// Get option from the user
			// It must be between 1 and 8 to be valid
			do {
				getInt(&option, "Choose option: ");
			} while (option < 1 || option > 8);

			printf("\n");

			// Call appropriate function
			switch (option) {
			case 1:
				addSurvey(&surveys);
				break;
			case 2:
				displayAllSurveys(surveys);
				break;
			case 3:
				displaySpecificSurvey(surveys);
				break;
			case 4:
				updateSurvey(&surveys);
				break;
			case 5:
				deleteSurvey(&surveys);
				break;
			case 6:
				printStatistics(surveys, stdout);
				break;
			case 7:
				generateReportFile(surveys);
				break;
			case 8:
				updateFile(surveys);
				break;
			}

			printf("\n");
		} while (option != 8);
	}

	getch();
}

// The login function will prompt the user for there credintials (i.e. there username and password)
// If the user enters incorrect credintials three times the function will print an error message and return false, else it will return true
// All user credintials are read from a text file called users.txt and stored in an array of user structs
bool login() {
	int attempts = 0;
	char username[MAX_STRING_LENGTH];
	char password[MAX_STRING_LENGTH];
	FILE* fp;
	userelement_t* head = NULL;
	userelement_t* element;

	// Open the user file
	fp = fopen("users.txt", "r");

	if (fp == NULL) {
		printf("There was a problem opening the user file!\n");
		return false;
	}
	else {
		while (!feof(fp)) {
			// Create a new user element
			element = (userelement_t*)malloc(sizeof(userelement_t));

			fgets(element->user.username, MAX_CREDINTIAL_LENGTH, fp);
			getc(fp);
			fgets(element->user.password, MAX_CREDINTIAL_LENGTH, fp);
			getc(fp);

			// Add element to start of linked list (order doesn't matter)
			element->next = head;
			head = element;
		}

		// Close the file
		fclose(fp);

		// Prompt the user for their credentials
		// Compare the users credentials to the credentials in the file
		do {
			getString(username, "Username: ");
			fseek(stdin, 0, SEEK_END);

			getPassword(password);

			element = head;

			while (element != NULL) {
				if (strcmp(username, element->user.username) == 0 && strcmp(password, element->user.password) == 0) {
					attempts = MAX_LOGIN_ATTEMPTS;
				}

				element = element->next;
			}

			// Print an error message if the credentials did not match
			if (attempts < MAX_LOGIN_ATTEMPTS) {
				printf("Incorrect username or password! %d attempts remaining!\n", (MAX_LOGIN_ATTEMPTS - attempts - 1));
			}

			attempts++;
		} while (attempts < MAX_LOGIN_ATTEMPTS);

		// Free memory for each element
		while ((element = head) != NULL) {
			head = head->next;
			free(element);
		}
	}

	// Determine weather the login was successful
	if (attempts == MAX_LOGIN_ATTEMPTS) {
		printf("Login failed!\n");
		return false;
	}
	else {
		printf("Login successful!\n\n");
		return true;
	}
}

// This function recieves a pointer to a string which will store the password entered by the user
// The password entered by the user is masked (i.e. asterisk will be echoed)
void getPassword(char* password) {
	int i = 0;
	char c;

	printf("Password: ");

	do {
		c = getch();
		fseek(stdin, 0, SEEK_END);

		if (c == '\b') {
			if (i > 0) {
				printf("\b \b");
				i -= 1;
			}
		}
		else {
			if (c == ' ' || c == '\r') {
				printf(" ");
			}
			else {
				printf("*");
			}

			if (i < MAX_STRING_LENGTH) {
				password[i] = c;
			}

			++i;
		}
	} while (c != '\r');

	printf("\n");

	// Add string terminator
	if (i >= MAX_STRING_LENGTH) {
		password[MAX_STRING_LENGTH - 1] = '\0';
	}
	else {
		password[i - 1] = '\0';
	}
}

// This function allow the user to create a new survey
// This survey will be added to the linked list
// The PPS number of the new survey must be unique
void addSurvey(surveyelement_t** head) {
	int i, pps;
	int index;
	surveyelement_t* element;
	surveyelement_t* temp;

	getInt(&pps, "PPS number: ");

	// Check if the pps number already exists in the linked list
	index = searchByPPS(*head, pps);

	if (index == -1) {
		// Create new element
		element = (surveyelement_t*)malloc(sizeof(surveyelement_t));

		element->survey.pps = pps;

		printf("Gender\n");
		printf("1) Male\n");
		printf("2) Female\n");

		do {
			getInt(&element->survey.gender, "Enter option: ");
		} while (element->survey.gender < 1 || element->survey.gender > 2);

		getString(element->survey.firstName, "First name: ");
		getString(element->survey.secondName, "Second name: ");
		getString(element->survey.address, "Address: ");

		do {
			getString(element->survey.email, "Email address: ");
		} while (verifyEmail(element->survey.email) == false);

		printf("Age Bracket\n");
		printf("1) 18 - 20 yrs\n");
		printf("2) 20 - 30 yrs\n");
		printf("3) 30 - 50 yrs\n");
		printf("4) 50 - 65 yrs\n");
		printf("5) 65+ yrs\n");

		do {
			getInt(&element->survey.ageBracket, "Enter option: ");
		} while (element->survey.ageBracket < 1 || element->survey.ageBracket > 5);

		printf("Income Bracket\n");
		printf("1) No Income\n");
		printf("2) Less than €20,000\n");
		printf("3) Less than €40,000\n");
		printf("4) Less than €60,000\n");
		printf("5) Less than €80,000\n");
		printf("6) Less than €100,000\n");
		printf("7) Greater than €100,000\n");

		do {
			getInt(&element->survey.incomeBracket, "Enter option: ");
		} while (element->survey.incomeBracket < 1 || element->survey.incomeBracket > 7);

		printf("How often do you exercise?\n");
		printf("1) Never\n");
		printf("2) Less than three times per week\n");
		printf("3) Less than five times per week\n");
		printf("4) More than five times per week\n");

		do {
			getInt(&element->survey.exerciseBracket, "Enter option: ");
		} while (element->survey.exerciseBracket < 1 || element->survey.exerciseBracket > 4);

		printf("How much alcohol do you consume per week?\n");
		printf("1) None\n");
		printf("2) Less than 2 units\n");
		printf("3) Less than 4 units\n");
		printf("4) More than 4 units\n");

		do {
			getInt(&element->survey.alcoholBracket, "Enter option: ");
		} while (element->survey.alcoholBracket < 1 || element->survey.alcoholBracket > 4);

		printf("How many cigarettes do you smoke per week?\n");
		printf("1) None\n");
		printf("2) Less than 20 cigarettes\n");
		printf("3) Less than 40 cigarettes\n");
		printf("4) More than 40 cigarettes\n");

		do {
			getInt(&element->survey.cigarettesBracket, "Enter option: ");
		} while (element->survey.cigarettesBracket < 1 || element->survey.cigarettesBracket > 4);

		add(head, element);

		printf("Added new survey to the list.\n");
	}
	else {
		printf("Failed to add the new survey to the list. The PPS number %d is not unique!\n", pps);
	}
}

// Loop through all suveys in the linked list
// Print the details of each survey in the list
void displayAllSurveys(surveyelement_t* head) {
	surveyelement_t* element = head;

	while (element != NULL) {
		displaySurveyDetails(element, stdout);

		// Print a space between each survey
		if (element->next != NULL) {
			printf("\n");
		}

		element = element->next;
	}
}

// Get the PPS number of the survey to display from the user
// Check if the linked list contains a survey with the specified PPS number
// If it does print the survey details
// Else print an appropriate error message
void displaySpecificSurvey(surveyelement_t* head) {
	int i, index, pps;
	surveyelement_t* element = head;

	index = searchByPPSOrName(head);

	if (index != -1) {
		for (i = 0; i < index; ++i) {
			element = element->next;
		}

		displaySurveyDetails(element, stdout);
	}
}

// This function will print the details of the surveys passed as a parameter
void displaySurveyDetails(surveyelement_t* element, FILE* fp) {
	char *gender[] = { "Male", "Female" };
	const char *age[] = { "18 - 20 yrs", "20 - 30 yrs", "30 - 50 yrs", "50 - 65 yrs", "65+ yrs" };
	const char *income[] = { "No Income", "Less than €20,000", "Less than €40,000", "Less than €60,000", "Less than €80,000", "Less than €100,000", "Greater than €100,000" };
	const char *exercise[] = { "Never", "Less than three times per week", "Less than five times per week", "More than five times per week" };
	const char *alcohol[] = { "None", "Less than 2 units", "Less than 4 units", "More than 4 units" };
	const char *cigarettes[] = { "None", "Less than 20 cigarettes", "Less than 40 cigarettes", "More than 40 cigarettes" };

	fprintf(fp, "PPS number: %d\n", element->survey.pps);
	fprintf(fp, "Gender: %s\n", gender[element->survey.gender - 1]);
	fprintf(fp, "First name: %s\n", element->survey.firstName);
	fprintf(fp, "Second name: %s\n", element->survey.secondName);
	fprintf(fp, "Address: %s\n", element->survey.address);
	fprintf(fp, "Email address: %s\n", element->survey.email);
	fprintf(fp, "Age: %s\n", age[element->survey.ageBracket - 1]);
	fprintf(fp, "Income: %s\n", income[element->survey.incomeBracket - 1]);
	fprintf(fp, "Exercise: %s\n", exercise[element->survey.exerciseBracket - 1]);
	fprintf(fp, "Alchol: %s\n", alcohol[element->survey.alcoholBracket - 1]);
	fprintf(fp, "Cigarettes: %s\n", cigarettes[element->survey.cigarettesBracket - 1]);
}

// This function will allow the user to update any property of a specified survey
// The user can pick multiple properties to edit from a menu
void updateSurvey(surveyelement_t** head) {
	int pps, i;
	int index;
	int searchIndex;
	int option;
	surveyelement_t* element = *head;

	char *gender[] = { "Male", "Female" };
	const char *age[] = { "18 - 20 yrs", "20 - 30 yrs", "30 - 50 yrs", "50 - 65 yrs", "65+ yrs" };
	const char *income[] = { "No Income", "Less than €20,000", "Less than €40,000", "Less than €60,000", "Less than €80,000", "Less than €100,000", "Greater than €100,000" };
	const char *exercise[] = { "Never", "Less than three times per week", "Less than five times per week", "More than five times per week" };
	const char *alcohol[] = { "None", "Less than 2 units", "Less than 4 units", "More than 4 units" };
	const char *cigarettes[] = { "None", "Less than 20 cigarettes", "Less than 40 cigarettes", "More than 40 cigarettes" };

	index = searchByPPSOrName(*head);

	if (index != -1) {
		// Get the element in the linked list to edit
		for (i = 0; i < index; ++i) {
			element = element->next;
		}

		do {
			printf("1) Update PPS number           (currently - %d)\n", element->survey.pps);
			printf("2) Update gender               (currently - %s)\n", gender[element->survey.gender - 1]);
			printf("3) Update first name           (currently - %s)\n", element->survey.firstName);
			printf("4) Update second name          (currently - %s)\n", element->survey.secondName);
			printf("5) Update address              (currently - %s)\n", element->survey.address);
			printf("6) Update email address        (currently - %s)\n", element->survey.email);
			printf("7) Update age bracket          (currently - %s)\n", age[element->survey.ageBracket - 1]);
			printf("8) Update income bracket       (currently - %s)\n", income[element->survey.incomeBracket - 1]);
			printf("9) Update exercise bracket     (currently - %s)\n", exercise[element->survey.exerciseBracket - 1]);
			printf("10) Update alcohol bracket     (currently - %s)\n", alcohol[element->survey.alcoholBracket - 1]);
			printf("11) Update cigarettes bracket  (currently - %s)\n", cigarettes[element->survey.cigarettesBracket - 1]);
			printf("12) Exit\n");

			// Get option from the user
			// It must be between 1 and 11 to be valid
			do {
				getInt(&option, "Choose option: ");
			} while (option < 1 || option > 12);

			switch (option) {
			case 1:
				getInt(&pps, "PPS number: ");

				// Check if the pps number already exists in the linked list
				searchIndex = searchByPPS(*head, pps);

				if (searchIndex == -1) {
					erase(head, index);
					element->survey.pps = pps;
					add(head, element);

					index = searchByPPS(*head, pps);
				}
				else {
					printf("The PPS number %d is not unique!\n", pps);
				}
				break;

			case 2:
				printf("Gender\n");
				printf("1) Male\n");
				printf("2) Female\n");

				do {
					getInt(&element->survey.gender, "Enter option: ");
				} while (element->survey.gender < 1 || element->survey.gender > 2);
				break;

			case 3:
				getString(element->survey.firstName, "First name: ");
				break;

			case 4:
				getString(element->survey.secondName, "Second name: ");
				break;

			case 5:
				getString(element->survey.address, "Address: ");
				break;

			case 6:
				do {
					getString(element->survey.email, "Email address: ");
				} while (verifyEmail(element->survey.email) == false);
				break;

			case 7:
				printf("Age Bracket\n");

				printf("1) 18 - 20 yrs\n");
				printf("2) 20 - 30 yrs\n");
				printf("3) 30 - 50 yrs\n");
				printf("4) 50 - 65 yrs\n");
				printf("5) 65+ yrs\n");

				do {
					getInt(&element->survey.ageBracket, "Enter option: ");
				} while (element->survey.ageBracket < 1 || element->survey.ageBracket > 5);
				break;

			case 8:
				printf("Income Bracket\n");
				printf("1) No Income\n");
				printf("2) Less than €20,000\n");
				printf("3) Less than €40,000\n");
				printf("4) Less than €60,000\n");
				printf("5) Less than €80,000\n");
				printf("6) Less than €100,000\n");
				printf("7) Greater than €100,000\n");

				do {
					getInt(&element->survey.incomeBracket, "Enter option: ");
				} while (element->survey.incomeBracket < 1 || element->survey.incomeBracket > 7);
				break;

			case 9:
				printf("How often do you exercise?\n");
				printf("1) Never\n");
				printf("2) Less than three times per week\n");
				printf("3) Less than five times per week\n");
				printf("4) More than five times per week\n");

				do {
					getInt(&element->survey.exerciseBracket, "Enter option: ");
				} while (element->survey.exerciseBracket < 1 || element->survey.exerciseBracket > 4);
				break;

			case 10:
				printf("How much alcohol do you consume per week?\n");
				printf("1) None\n");
				printf("2) Less than 2 units\n");
				printf("3) Less than 4 units\n");
				printf("4) More than 4 units\n");

				do {
					getInt(&element->survey.alcoholBracket, "Enter option: ");
				} while (element->survey.alcoholBracket < 1 || element->survey.alcoholBracket > 4);
				break;

			case 11:
				printf("How many cigarettes do you smoke per week?\n");
				printf("1) None\n");
				printf("2) Less than 20 cigarettes\n");
				printf("3) Less than 40 cigarettes\n");
				printf("4) More than 40 cigarettes\n");

				do {
					getInt(&element->survey.cigarettesBracket, "Enter option: ");
				} while (element->survey.cigarettesBracket < 1 || element->survey.cigarettesBracket > 4);
				break;
			}
		} while (option != 12);
	}
}

// This function will delete a survey with the specified PPS number from the linked list
// If there is no survey with the specified PPS number print an appropriate error message
// Else let the user know the survey was deleted and free the memory allocated to the deleted survey
void deleteSurvey(surveyelement_t** head) {
	int i, pps;
	int index;
	surveyelement_t* element;

	if (*head != NULL) {
		getInt(&pps, "Please enter the PPS number you want to delete: ");

		index = searchByPPS(*head, pps);

		if (index == -1) {
			printf("There was no survey in the list with the PPS number %d!\n", pps);
		}
		else {
			element = erase(head, index);
			free(element);

			printf("Successfully deleted the survey with the PPS number %d.\n", pps);
		}
	}
	else {
		printf("There are no surveys in the database!\n");
	}
}

// This function will call the generateStatistics function and will print the results to the specified file
// The results are clearly formatted
void printStatistics(surveyelement_t* head, FILE* fp) {
	int i, j;
	float statistics[14][12];
	char *criteria[] = { "18 - 20 yrs",
		"20 - 30 yrs",
		"30 - 50 yrs",
		"50 - 65 yrs",
		"65+ yrs",
		"No Income",
		"Less than €20,000",
		"Less than €40,000",
		"Less than €60,000",
		"Less than €80,000",
		"Less than €100,000",
		"Greater than €100,000",
		"Male",
		"Female" };

	char *statements[] = { " of people smoke",
		" of people smoke less than 20 cigarettes per week",
		" of people smoke less than 40 cigarettes per week",
		" of people smoke greater than 40 cigarettes per week",
		" of people never exercise",
		" of people exercise less than three times per week",
		" of people exercise less than five times per week",
		" of people exercise more than five times per week",
		" of people do not consume alcohol",
		" of people consume less than 2 units of alcohol per week",
		" of people consume less than 4 units of alcohol per week",
		" of people consume more than 4 units of alcohol per week" };

	generateStatistics(head, statistics);

	for (i = 0; i < 14; ++i) {
		fprintf(fp, "%s\n", criteria[i]);

		for (j = 0; j < 12; ++j) {
			fprintf(fp, "%.0f%%%s\n", statistics[i][j], statements[j]);
		}

		if (i < 13) {
			fprintf(fp, "\n");
		}
	}
}

// Print all survey details and statistics to a report file called report.txt
// This file is designed to be easy to read
// If the file fails to open print an appropriate error message
void generateReportFile(surveyelement_t* head) {
	int i, j;
	float statistics[14][12];
	surveyelement_t* element = head;
	FILE* fp;

	fp = fopen("report.txt", "w");

	if (fp == NULL) {
		printf("There was a problem opening the the file!\n");
	}
	else {
		while (element != NULL) {
			displaySurveyDetails(element, fp);
			fprintf(fp, "\n");
			element = element->next;
		}

		generateStatistics(head, statistics);

		printStatistics(head, fp);

		fclose(fp);

		printf("The report file was created.\n");
	}
}

// Store all data for each survey in a 2d array of integers
// This dat will be used to generate the statistics
void generateStatistics(surveyelement_t* head, float statistics[][12]) {
	int i, j;
	float data[14][13] = { 0 }; // Initialise all values to zero
	surveyelement_t* element = head;

	while (element != NULL) {
		// Age bracket
		data[element->survey.ageBracket - 1][element->survey.cigarettesBracket - 1]++;
		data[element->survey.ageBracket - 1][element->survey.exerciseBracket + 3]++;
		data[element->survey.ageBracket - 1][element->survey.alcoholBracket + 7]++;
		data[element->survey.ageBracket - 1][12]++;

		// Income bracket
		data[element->survey.incomeBracket - 1 + 5][element->survey.cigarettesBracket - 1]++;
		data[element->survey.incomeBracket - 1 + 5][element->survey.exerciseBracket + 3]++;
		data[element->survey.incomeBracket - 1 + 5][element->survey.alcoholBracket + 7]++;
		data[element->survey.incomeBracket - 1 + 5][12]++;

		// Gender
		data[element->survey.gender - 1 + 12][element->survey.cigarettesBracket - 1]++;
		data[element->survey.gender - 1 + 12][element->survey.exerciseBracket + 3]++;
		data[element->survey.gender - 1 + 12][element->survey.alcoholBracket + 7]++;
		data[element->survey.gender - 1 + 12][12]++;

		element = element->next;
	}

	// The values in the first column contain the percentage of people who don't smoke
	// Subtract this from total number of people in that criteria group to get the percentage that do smoke
	for (i = 0; i < 14; ++i) {
		data[i][0] = data[i][12] - data[i][0];
	}

	for (i = 0; i < 14; ++i) {
		for (j = 0; j < 12; ++j) {
			if (data[i][12] == 0 || data[i][j] == 0) {
				statistics[i][j] = 0;
			}
			else {
				statistics[i][j] = (data[i][j] / data[i][12]) * 100;
			}
		}
	}
}

// Add a new survey to the linked list
// The surveys position in the list will depend on its pps number
// The list must be ordered in ascending order
void add(surveyelement_t** head, surveyelement_t* element) {
	int index, i;
	surveyelement_t* temp;

	// Get the index for this element in the linked list
	index = getNewIndex(*head, element->survey.pps);

	// If the index is 0 add the element to the start
	if (index == 0) {
		element->next = *head;
		*head = element;
	}
	else {
		temp = *head;

		for (i = 0; i < index - 1; ++i) {
			temp = temp->next;
		}

		element->next = temp->next;
		temp->next = element;
	}
}

// Remove the survey at the specified index from the linked list
// Note: this function does not free the memory allocated to the removed survey
surveyelement_t* erase(surveyelement_t** head, int index) {
	int i;
	surveyelement_t* oldTemp;
	surveyelement_t* temp;

	temp = *head;

	if (index == 0) {
		*head = temp->next;
	}
	else {
		for (i = 0; i < index; ++i) {
			oldTemp = temp;
			temp = temp->next;
		}

		oldTemp->next = temp->next;
	}

	return temp;
}

// Search the linked list for a survey with either a PPS number or a name
// Allow the user to choose which to search with
// If the user chooses to search with a PPS number call the searchByPPS function
// If the user chhoses to search with a name return a list of surveys to choose from
// Return the index of the survey if one was found
// Else return -1 and print an error message
int searchByPPSOrName(surveyelement_t* head) {
	surveyelement_t* element = head;
	int *validIndices;
	char searchName[MAX_STRING_LENGTH * 2 + 1]; // Allow space for concatenation of first and last names and the space in between
	char surveyName[MAX_STRING_LENGTH * 2 + 1];
	int option;
	int i, j;
	int pps;
	int count = 0;
	int index = -1;

	printf("Search by\n");
	printf("1) PPS\n");
	printf("2) Full name\n");

	do {
		getInt(&option, "Enter option: ");
	} while (option < 1 || option > 2);

	if (option == 1) {
		getInt(&pps, "Enter PPS: ");
		index = searchByPPS(head, pps);
	}
	else {
		getString(searchName, "Enter full name: ");

		while (element != NULL) {
			strcpy(surveyName, element->survey.firstName);
			strcat(surveyName, " ");
			strcat(surveyName, element->survey.secondName);

			if (strcmp(surveyName, searchName) == 0) {
				count++;
			}

			element = element->next;
		}

		if (count != 0) {
			validIndices = (int)malloc(count * sizeof(int));

			element = head;
			count = 0;

			while (element != NULL) {
				index++;

				strcpy(surveyName, element->survey.firstName);
				strcat(surveyName, " ");
				strcat(surveyName, element->survey.secondName);

				if (strcmp(surveyName, searchName) == 0) {
					validIndices[count] = index;
					count++;
				}

				element = element->next;
			}

			if (count == 1) {
				index = validIndices[0];
			}
			else {
				element = head;

				printf("Choose survey via PPS number.\n");

				for (i = 0; i < count; ++i) {
					for (j = 0; j < validIndices[count]; ++j) {
						element = element->next;
					}

					printf("%d) %d\n", (i + 1), element->survey.pps);

					element = head;
				}

				do {
					getInt(&option, "Choose survey: ");
				} while (option < 1 || option > count);

				index = validIndices[option - 1];
			}

			free(validIndices);
		}
	}

	if (index == -1) {
		printf("There was no survey found which matches your search!\n");
	}

	return index;
}

// Search the linked list for a survey with the specified PPS number
// If the PPS number is found in the linked list return its index in the linked list starting at 0
// Else if the pps number is not found return -1
int searchByPPS(surveyelement_t* head, int pps) {
	surveyelement_t* element = head;
	int count = 0;
	int index = -1;

	while (element != NULL) {
		if (element->survey.pps == pps) {
			index = count;
			break;
		}

		count++;
		element = element->next;
	}

	return index;
}

// Get the index for a new element in the linked list based on the specified PPS number
// The linked list should be ordered in ascending order
int getNewIndex(surveyelement_t* head, int pps) {
	surveyelement_t* element = head;
	int count = 1;
	int index = 0;

	if (element != NULL) {
		if (element->survey.pps < pps) {
			while (element != NULL) {
				if ((element->next != NULL && element->next->pps > pps) || element->next == NULL) {
					index = count;
					break;
				}

				element = element->next;
				count++;
			}
		}
	}

	return index;
}

// This function is responible for reading previously saved surveys from a file called survey.txt and storing them in the linked list
// If the file cannot be opened print an appropriate error message
// The double pointer to the head of the linked ist is required as the list will be edited in this function
void initialiseLinkedList(surveyelement_t** head) {
	FILE* fp;
	surveyelement_t* element;
	surveyelement_t* temp;

	fp = fopen(FILE_NAME, "r");

	if (fp == NULL) {
		printf("There was a problem opening the file!\n");
	}
	else {
		while (!feof(fp)) {
			// Create a new element
			element = (surveyelement_t*)malloc(sizeof(surveyelement_t));

			fscanf(fp, "%d\n", &element->survey.pps);
			fgets(element->survey.firstName, MAX_STRING_LENGTH, fp);
			strtok(element->survey.firstName, "\n");
			fgets(element->survey.secondName, MAX_STRING_LENGTH, fp);
			strtok(element->survey.secondName, "\n");
			fgets(element->survey.address, MAX_STRING_LENGTH, fp);
			strtok(element->survey.address, "\n");
			fgets(element->survey.email, MAX_STRING_LENGTH, fp);
			strtok(element->survey.email, "\n");
			fscanf(fp, "%d%d%d%d%d%d\n", &element->survey.gender, &element->survey.ageBracket, &element->survey.incomeBracket, &element->survey.exerciseBracket, &element->survey.alcoholBracket, &element->survey.cigarettesBracket);

			if (*head == NULL) {
				element->next = *head;
				*head = element;
			}
			else {
				element->next = temp->next;
				temp->next = element;
			}

			temp = element;
		}

		fclose(fp);
	}
}

// This function is responible writing all surveys to a file called survey.txt
// The file is opened in write mode
// If the file cannot be opened or created print an appropriate error message
void updateFile(surveyelement_t* head) {
	FILE* fp;
	surveyelement_t* element = head;

	fp = fopen(FILE_NAME, "w");

	if (fp == NULL) {
		printf("There was a problem opening the file!\n");
	}
	else {
		while (element != NULL) {
			// Write surveys to file
			fprintf(fp, "%d\n", element->survey.pps);
			fprintf(fp, "%s\n", element->survey.firstName);
			fprintf(fp, "%s\n", element->survey.secondName);
			fprintf(fp, "%s\n", element->survey.address);
			fprintf(fp, "%s\n", element->survey.email);
			fprintf(fp, "%d %d %d %d %d %d\n", element->survey.gender, element->survey.ageBracket, element->survey.incomeBracket, element->survey.exerciseBracket, element->survey.alcoholBracket, element->survey.cigarettesBracket);

			element = element->next;
		}

		fclose(fp);

		printf("Successfully wrote surveys to file.\n");
	}
}

// This function will recieve an email address and verify it based on a number of rules
// Rule 1) It must contain a full stop before @
// Rule 2) It must contain a single @ symbol
// Rule 3) There must be a domain name (i.e. text between @ and .com)
// Rule 4) It must end in .com
// If any of these rules are broke print an error message
// Therefore, a valid email will be similar to some.name@somedomain.com
bool verifyEmail(char* email) {
	bool isValid = false;
	char *token;
	int i, count = 0;

	// Loop through the email string and count the number of @ symbols
	for (i = 0; i < strlen(email); i++) {
		if (email[i] == '@') {
			count++;
		}
	}

	if (count != 1) { // Check if email address contains a single @ symbol
		printf("Invalid email: must contain a single @ symbol\n");
	}
	else {
		token = strtok(strdup(email), "@"); // Pass in a duplicated string so the original email address isn't modified

		if (!strstr(token, ".")) { // Check if theres a full stop before @
			printf("Invalid email: must contain a full stop before @ symbol\n");
		}
		else {
			token = strtok(NULL, "@");

			if (token == NULL) {
				printf("Invalid email: missing domain\n");
			}
			else if (!strstr(token, ".com")) { // Check if email has .com
				printf("Invalid email: must contain .com\n");
			}
			else if (strcmp(".com", token + strlen(token) - 4) != 0) {  // Check if email ends .com
				printf("Invalid email: must end in .com\n");
			}
			else if (strlen(token) < 5) {  // Check if email has a domain
				printf("Invalid email: must enter a domain (e.g. gmail, yahoo, gmit)\n");
			}
			else { // If all rules are adhered to the email must be valid
				isValid = true;
			}
		}
	}

	return isValid;
}

// This function prompts the user for a string
// This function accepts two parameters -
// The first is a character pointer which will hold the string the user enters (Pass by reference)
// The second is a message to instuct the user what to enter
void getString(char* str, char* message) {
	int length;
	char ch;

	do {
		printf("%s", message);
		fseek(stdin, 0, SEEK_END);
		fgets(str, MAX_STRING_LENGTH, stdin);
		length = strlen(str);

		if (str[length - 1] != '\0') {
			str[length - 1] = '\0';
		}
	} while (str[0] == '\0');

	strtok(str, "\n");
}

// This function prompts the user for an integer
// This function accepts two parameters -
// The first is a pointer to the int the user will change (Pass by reference)
// The second is a message to instuct the user what to enter
void getInt(int* num, char* message) {
	int ret;

	do {
		fseek(stdin, 0, SEEK_END);
		printf(message);
		ret = scanf("%d", num);
	} while (ret != 1);
}