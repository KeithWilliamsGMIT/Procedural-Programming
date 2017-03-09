// Keith Williams
// 03/12/2015
// Snakes and Ladders text based game written in C

// Notes: Had trouble with fflush() so I replaced it with fseek()
//			Used fgets() to get strings instead of scanf()

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

// Structures
typedef struct {
	char name[21];
	int position;
} player_t;

// This struct will be used by both the snakes and the ladders
typedef struct {
	int upperPosition;
	int lowerPosition;
} relocate_t;

// Function prototypes
player_t* newGame(struct relocate_t snakes[], struct relocate_t ladders[], int* numPlayers, int* saveID);
player_t* loadGame(struct relocate_t snakes[], struct relocate_t ladders[], int* numPlayers, int* saveID, int* turn);
void saveGame(struct relocate_t snakes[], struct relocate_t ladders[], struct player_t players[], int numPlayers, int* saveID, int turn);
void printInstructions();
void printBoard(struct relocate_t snakes[], struct relocate_t ladders[], struct player_t players[], int numPlayers);
void winGame(int saveID);
void playGame(struct relocate_t snakes[], struct relocate_t ladders[], struct player_t players[], int numPlayers, int* saveID, int turn);

// Constants
#define MAX_SNAKES_AND_LADDERS 7
#define MAX_FILENAME_LENGTH 20
#define CONFIG_FILENAME "Config.txt"

typedef enum { false, true } bool;

void main() {
	int ret, i, j;
	bool showMenu = true;
	int option;
	int numPlayers;
	int saveID = 0;
	int turn;
	
	// Snake and ladder arrays
	relocate_t snakes[MAX_SNAKES_AND_LADDERS];
	relocate_t ladders[MAX_SNAKES_AND_LADDERS];

	// Array of players specified by the user
	player_t* players;

	// Seed random (used to roll dice and generate snake and ladder positions)
	srand((unsigned)time(NULL));

	printf("==================== Snakes and Ladders ====================\n");
	
	do {
		// Show the options menu
		if (showMenu == true) {
			printf("=========================== Menu ===========================\n");
			printf("Press 1 to load a previous game\n");
			printf("Press 2 to start a new game\n");
			printf("Press 3 to read instructions\n");
			printf("Press 4 to quit game\n");
		}

		showMenu = true;

		// Scan the option entered by the user
		fseek(stdin, 0, SEEK_END);
		ret = scanf("%d", &option);

		// if option is valid
		if (ret == 1 && (option == 1 || option == 2 || option == 3 || option == 4)) {
			if (option == 3) {
				printInstructions();
			}
			else if (option == 4) {
				// break out of do while loop to quit game
				break;
			}
			else {
				if (option == 1) {
					players = loadGame(snakes, ladders, &numPlayers, &saveID, &turn);
				}
				else if (option == 2) {
					players = newGame(snakes, ladders, &numPlayers, &saveID);
					// If it's a new game reset turn to zero
					turn = 0;
				}

				if (!(saveID == 0 && option == 1)) {
					// Print the board before the game starts
					printBoard(snakes, ladders, players, numPlayers);
					playGame(players, snakes, ladders, numPlayers, &saveID, turn);
				}
			}
		} else {
			// Print warning if an invalid value is entered
			printf("The value you entered is not valid, please enter either 1, 2, 3 or 4\n", option);
			showMenu = false;
		}
	} while (true);
}

// Save the game to a file
void saveGame(relocate_t* snakes, relocate_t* ladders, player_t* players, int numPlayers, int* saveID, int turn) {
	FILE* config;
	FILE* fp;
	int i, ret, option, chosenFile;
	char filename[MAX_FILENAME_LENGTH + 1]; // +1 for string terminator

	// Initialise chosen file to zero
	chosenFile = 0;

	// Open config file for reading
	config = fopen(CONFIG_FILENAME, "r");

	if (config != NULL) {
		// Give player option to overwrite the old save file if one exits
		if (*saveID != 0) {
			printf("Choose one of the following options:\n");
			printf("Press 1 to overwrite the old save file\n");
			printf("Press 2 to save as a new file\n");

			// Get option from player
			do {
				fseek(stdin, 0, SEEK_END);
				ret = scanf("%d", &option);
			} while (ret != 1 || (option != 1 && option != 2));

			if (option == 1) {
				chosenFile = *saveID;

				// Set pointer to start of file
				fseek(config, 0, SEEK_SET);

				for (i = 0; i < chosenFile; ++i) {
					fscanf(config, "%s", filename);
				}
			}
		}

		// Close config file only if it was opened
		fclose(config);
	}

	// If the player chooses not to overwrite the old file or no old file exits
	if (chosenFile == 0 || *saveID == 0) {
		// Get file name
		printf("Enter file name (Max %d characters): ", MAX_FILENAME_LENGTH);

		fseek(stdin, 0, SEEK_END);
		fgets(filename, sizeof(filename), stdin);

		if (filename[strlen(filename) - 1] == '\n') {
			filename[strlen(filename) - 1] = '\0';
		}
	}

	fp = fopen(filename, "w");

	if (fp != NULL) {
		// Only append new filename to config file if the file is created
		if (chosenFile == 0 || *saveID == 0) {
			// Append new file to config file
			config = fopen(CONFIG_FILENAME, "a");
			fprintf(config, "%s\n", filename);
			fclose(config);

			// Set saveID to the total number of lines in file as the new filename will be appended to the end
			config = fopen(CONFIG_FILENAME, "r");
			*saveID = 0;

			while (fgets(filename, sizeof(filename), config)) {
				if (filename[strlen(filename) - 1] == '\n') {
					filename[strlen(filename) - 1] = '\0';
				}
				else {
					fseek(config, 2, SEEK_CUR);
				}

				(*saveID)++;
			}

			// Close file
			fclose(config);
		}

		// Save whose turn it is
		fprintf(fp, "%d\n", turn);

		// Save number of players
		fprintf(fp, "%d\n", numPlayers);

		for (i = 0; i < numPlayers; ++i) {
			fprintf(fp, "%s\n%d\n", players[i].name, players[i].position);
		}

		// Save snakes
		for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
			fprintf(fp, "%d %d\n", snakes[i].upperPosition, snakes[i].lowerPosition);
		}

		// Save Ladders
		for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
			fprintf(fp, "%d %d\n", ladders[i].upperPosition, ladders[i].lowerPosition);
		}

		fclose(fp);
	}
	else {
		// If an error occurs when trying to write to or create the file print an error and set saveID to 0
		printf("Failed to save file!\n");
		*saveID = 0;
	}
}

// Print the instructions for the game to the screen
void printInstructions() {
	printf("\n======================= Instructions =======================");
	printf("\nPlayers take turns rolling a dice.");
	printf("\nThe player moves up the board by the number they rolled.");
	printf("\nIf the player lands at the foot of a ladder they move to the top of the ladder.");
	printf("\nIf the player lands at the head of a snake they move to the tail of the snake.");
	printf("\nThe first player to reach square 100 wins.");
	printf("\n============================================================\n");
}

// Print snake and ladder coordinates and player positions on board
void printBoard(relocate_t* snakes, relocate_t* ladders, player_t* players, int numPlayers) {
	int i;

	printf("========================== Board ===========================\n");

	// Print coordinates for snakes and ladders
	for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
		printf("Snake #%d: Mouth Location: %d Tail Location: %d\n", (i + 1), snakes[i].upperPosition, snakes[i].lowerPosition);
		printf("Ladder #%d: Bottom of Ladder Location: %d Top of Ladder Location: %d\n", (i + 1), ladders[i].lowerPosition, ladders[i].upperPosition);
		printf("\n");
	}

	// Print player positions
	for (i = 0; i < numPlayers; ++i) {
		printf("%s is on %d", players[i].name, players[i].position);
		printf("\n");
	}

	printf("\n");
}

// Start a new game - Get the number of players and their names and generate a new board
player_t* newGame(relocate_t* snakes, relocate_t* ladders, int* numPlayers, int* saveID) {
	int ret, i, j, r, minimumUpperPosition;
	player_t* players;
	bool isOccupiedSquare;

	printf("========================= New Game =========================\n");

	// Ask user to enter number of players
	do {
		fseek(stdin, 0, SEEK_END);
		printf("Please enter the number of players (Minimum is 2 and Maximum is 6): ");
		ret = scanf("%d", numPlayers);
	} while (ret != 1 || *numPlayers < 2 || *numPlayers > 6);

	// Dynamically allocate players array
	players = (player_t*)malloc(sizeof(player_t) * (*numPlayers));

	// Loop through players and enter their details
	for (i = 0; i < *numPlayers; ++i) {
		// Enter player name
		printf("Please enter the name of player %d: ", (i + 1));
		fseek(stdin, 0, SEEK_END);
		fgets((players + i)->name, sizeof((players + i)->name), stdin);

		if ((players + i)->name[strlen((players + i)->name) - 1] == '\n') {
			(players + i)->name[strlen((players + i)->name) - 1] = '\0';
		}

		// Set position to zero
		players[i].position = 0;
	}

	// Set upper and lower positions for all snakes and ladders to zero
	for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
		snakes[i].lowerPosition = 0;
		snakes[i].upperPosition = 0;
		ladders[i].lowerPosition = 0;
		ladders[i].upperPosition = 0;
	}

	// Loop through snake and ladder arrays and set positions
	for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
		// r = (rand() % (max_number + 1 - minimum_number)) + minimum_number;
		// Rule: Upper and lower positions of snake or ladder cannot be on the same row
		// Rule: Snakes and ladders do not occupy the same square

		// SET SNAKE LOWER POSITION
		do {
			isOccupiedSquare = false;
			r = (rand() % (90 + 1 - 1)) + 1;

			for (j = 0; j < MAX_SNAKES_AND_LADDERS; ++j) {
				if (snakes[j].lowerPosition == r || snakes[j].upperPosition == r || ladders[j].lowerPosition == r || ladders[j].upperPosition == r) {
					isOccupiedSquare = true;
					break;
				}
			}
		} while(isOccupiedSquare == true);

		snakes[i].lowerPosition = r;

		// SET SNAKE UPPER POSITION
		if ((r % 10) == 0) {
			minimumUpperPosition = (r - (r % 10)) + 1;
		}
		else {
			minimumUpperPosition = (r - (r % 10)) + 11;
		}

		do {
			isOccupiedSquare = false;
			r = (rand() % (99 + 1 - minimumUpperPosition)) + minimumUpperPosition;

			for (j = 0; j < MAX_SNAKES_AND_LADDERS; ++j) {
				if (snakes[j].lowerPosition == r || snakes[j].upperPosition == r || ladders[j].lowerPosition == r || ladders[j].upperPosition == r) {
					isOccupiedSquare = true;
					break;
				}
			}
		} while (isOccupiedSquare == true);

		snakes[i].upperPosition = r;

		// SET LADDER LOWER POSITION
		do {
			isOccupiedSquare = false;
			r = (rand() % (90 + 1 - 1)) + 1;

			for (j = 0; j < MAX_SNAKES_AND_LADDERS; ++j) {
				if (snakes[j].lowerPosition == r || snakes[j].upperPosition == r || ladders[j].lowerPosition == r || ladders[j].upperPosition == r) {
					isOccupiedSquare = true;
					break;
				}
			}
		} while (isOccupiedSquare == true);

		ladders[i].lowerPosition = r;

		// SET LADDER UPPER POSITION
		if ((r % 10) == 0) {
			minimumUpperPosition = (r - (r % 10)) + 1;
		}
		else {
			minimumUpperPosition = (r - (r % 10)) + 11;
		}

		do {
			isOccupiedSquare = false;
			r = (rand() % (100 + 1 - minimumUpperPosition)) + minimumUpperPosition;
			
			for (j = 0; j < MAX_SNAKES_AND_LADDERS; ++j) {
				if (snakes[j].lowerPosition == r || snakes[j].upperPosition == r || ladders[j].lowerPosition == r || ladders[j].upperPosition == r) {
					isOccupiedSquare = true;
					break;
				}
			}
		} while (isOccupiedSquare == true);

		ladders[i].upperPosition = r;
	}

	// Since it's a new game there will be no old file so reset the save ID to 0
	*saveID = 0;

	printf("Creating new game...\n");

	return players;
}

// Load a saved game from a file
player_t* loadGame(relocate_t* snakes, relocate_t* ladders, int* numPlayers, int* saveID, int* turn) {
	int i, ret, chosenFile;
	int numFiles = 0;
	char savedFilename[MAX_FILENAME_LENGTH + 1]; // +1 for string terminator
	FILE* config;
	FILE* fp;
	player_t* players;

	printf("======================== Load Game =========================\n");

	// Open the config file
	config = fopen(CONFIG_FILENAME, "r");

	if (config != NULL) {
		// Chose to loop through the config file using this method rather than EOF for two reasons:
		// It ignores the empty line at the end of the file
		// It reads the file name in loop making it shorter
		while (fgets(savedFilename, sizeof(savedFilename), config) != NULL) {
			if (savedFilename[strlen(savedFilename) - 1] == '\n') {
				savedFilename[strlen(savedFilename) - 1] = '\0';
			}
			else {
				fseek(config, 2, SEEK_CUR);
			}

			++numFiles;
			printf("%d) %s\n", numFiles, savedFilename);
		}
	}
	else {
		printf("No saved files available!\n");
		return;
	}

	if (numFiles > 0) {
		do {
			printf("Choose the file you want to load (Enter a number between 1 and %d): ", numFiles);
			fseek(stdin, 0, SEEK_END);
			ret = scanf("%d", &chosenFile);
		} while (ret != 1 || (chosenFile < 1 || chosenFile > numFiles));
	}
	else {
		printf("No saved files available!\n");
		return;
	}

	// Set saveID to the line in the config file the name of the file was saved
	*saveID = chosenFile;

	fseek(config, 0, SEEK_SET);

	for (i = 0; i < chosenFile; ++i) {
		fgets(savedFilename, sizeof(savedFilename), config);

		if (savedFilename[strlen(savedFilename) - 1] == '\n') {
			savedFilename[strlen(savedFilename) - 1] = '\0';
		}
		else {
			fseek(config, 2, SEEK_CUR);
		}
	}

	// Close config file
	fclose(config);

	fp = fopen(savedFilename, "r");

	if (fp != NULL) {
		printf("Loading %s...\n", savedFilename);

		// First line is the indicates whose turn it is
		fscanf(fp, "%d", turn);

		// Second line is the number of players
		fscanf(fp, "%d", numPlayers);

		// Allocate memory for the player array
		players = (player_t*)malloc(sizeof(player_t) * (*numPlayers));

		for (i = 0; i < *numPlayers; ++i) {
			// Read player i's name and position
			while ((char)fgetc(fp) != '\n');

			if (fgets((players + i)->name, sizeof((players + i)->name), fp) != NULL) {
				if ((players + i)->name[strlen((players + i)->name) - 1] == '\n') {
					(players + i)->name[strlen((players + i)->name) - 1] = '\0';
				}
			}

			fscanf(fp, "%d", &(players + i)->position);
		}

		// Read snakes
		for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
			fscanf(fp, "%d %d", &(snakes + i)->upperPosition, &(snakes + i)->lowerPosition);
		}

		// Read ladders
		for (i = 0; i < MAX_SNAKES_AND_LADDERS; ++i) {
			fscanf(fp, "%d %d", &(ladders + i)->upperPosition, &(ladders + i)->lowerPosition);
		}

		// Close file
		fclose(fp);
	}
	else {
		printf("Error opening %s\n", savedFilename);
		*saveID = 0;
	}

	return players;
}

// Delete game file and entry from config file and set save ID to zero
void winGame(int* saveID) {
	FILE* config;
	FILE* temp;
	char filename[MAX_FILENAME_LENGTH];
	int numFiles = 0;
	int count = 0;

	// Open config file
	config = fopen(CONFIG_FILENAME, "r");
	temp = fopen("Temp.txt", "w");

	if (config != NULL || temp != NULL) {
		while (fscanf(config, "%s", filename) == 1) {
			++count;

			if (count != *saveID) {
				fprintf(temp, "%s\n", filename);
			}
			else {
				// Delete the save file as it's no longer needed
				remove(filename);
			}
		}

		// Close config file
		fclose(config);
		fclose(temp);

		// Remove old config file and rename temp file to Config.txt
		remove(CONFIG_FILENAME);
		rename("Temp.txt", CONFIG_FILENAME);
	} else {
		printf("ERROR: Could not open %s or do not have permissions to create file", CONFIG_FILENAME);
	}

	// When the game is finished reset id to 0
	*saveID = 0;
}

void playGame(player_t* players, relocate_t* snakes, relocate_t* ladders, int numPlayers, int* saveID, int turn) {
	int i, j, r;
	int newPosition;
	bool isValid, showMenu;
	char action, save;

	isValid = false;

	i = turn;

	do {
		while (i < numPlayers) {
			printf("It's %s's turn \n", (*(players + i)).name);
			
			showMenu = true;

			do {
				if (showMenu) {
					printf("Press 'r' to roll\n");
					printf("Press 's' to save\n");
					printf("Press 'p' to print board\n");
					printf("Press 'q' to quit\n");
				}

				showMenu = false;

				fseek(stdin, 0, SEEK_END);
				scanf("%c", &action);

				if (action == 's') {
					saveGame(snakes, ladders, players, numPlayers, saveID, i);
					showMenu = true;
				}
				else if (action == 'p') {
					printBoard(snakes, ladders, players, numPlayers);
					showMenu = true;
				}
				else if (action == 'q') {
					do {
						printf("Do you want to save before you quit (Y/N): ");
						fseek(stdin, 0, SEEK_END);
						scanf("%c", &save);
					} while (save != 'y' && save != 'n');

					if (save == 'y') {
						saveGame(snakes, ladders, players, numPlayers, saveID, i);
					}
				}
				else if (action != 'r') {
					printf("Please enter a valid value!\n");
				}
			} while (action != 'r' && action != 'q');

			if (action == 'r') {
				// Roll the dice
				r = (rand() % 6) + 1;

				// Calculate new position
				newPosition = players[i].position + r;

				if (newPosition > 100) {
					newPosition = 100 - (r - (100 - players[i].position));
				}

				// Update position
				players[i].position = newPosition;

				// Print out message
				printf("%s rolled a %d and landed on square %d", (*(players + i)).name, r, (*(players + i)).position);

				for (j = 0; j < MAX_SNAKES_AND_LADDERS; ++j) {
					if (players[i].position == snakes[i].upperPosition) {
						players[i].position = snakes[i].lowerPosition;
						printf("\n%s landed on a snake and moves down to square %d", (*(players + i)).name, (*(players + i)).position);
					}
					else if (players[i].position == ladders[i].lowerPosition) {
						players[i].position = ladders[i].upperPosition;
						printf("\n%s landed on a ladder and moves up to square %d", (*(players + i)).name, (*(players + i)).position);
					}
				}

				printf("\n\n");

				if ((*(players + i)).position == 100) {
					isValid = true;
					printf("==================== %s Wins ====================\n", (*(players + i)).name);
					winGame(saveID);
					break; // break out of for loop
				}

				if (r == 6) {
					printf("%s rolled a 6. Roll again.\n", (*(players + i)).name);
				}
				else {
					++i;
				}
			}
			else if (action == 'q') {
				isValid = true;
				break; // break out of for loop
			}
		}

		i = 0;
	} while (isValid != true);

	// Free dynamically allocated array when the game is won or quit
	free(players);
}