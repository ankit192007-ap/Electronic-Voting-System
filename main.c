#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 50
#define MAX_PIN 20

typedef struct {
    int id;
    char name[MAX_NAME];
    char pin[MAX_PIN];
    int hasVoted;
} Voter;

typedef struct {
    int id;
    char name[MAX_NAME];
} Candidate;

typedef struct {
    unsigned int ballotID;
    int candidateID;
    time_t timestamp;
    unsigned long long checksum;
} Transaction;

Voter *voters = NULL;
Candidate *candidates = NULL;
Transaction *logs = NULL;

int voterCount = 0;
int candidateCount = 0;
int logCount = 0;

int voterCapacity = 0;
int candidateCapacity = 0;
int logCapacity = 0;

unsigned int ballotCounter = 0;

/* Input function */
void readLine(char *text, int size) {
    if (fgets(text, size, stdin) != NULL) {
        text[strcspn(text, "\n")] = '\0';
    }
}

/* Integer input validation */
int readInt(char message[]) {
    char line[100];
    int value;
    char extra;

    while (1) {
        printf("%s", message);
        readLine(line, sizeof(line));

        if (sscanf(line, " %d %c", &value, &extra) == 1)
            return value;

        printf("Invalid input! Enter a number.\n");
    }
}

/* Find voter */
int findVoter(int id) {
    for (int i = 0; i < voterCount; i++) {
        if (voters[i].id == id)
            return i;
    }
    return -1;
}

/* Find candidate */
int findCandidate(int id) {
    for (int i = 0; i < candidateCount; i++) {
        if (candidates[i].id == id)
            return i;
    }
    return -1;
}

/* Dynamic memory for voters */
int growVoters() {
    if (voterCount < voterCapacity)
        return 1;

    int capacity = voterCapacity == 0 ? 2 : voterCapacity * 2;

    Voter *temp = realloc(voters,
                          (size_t)capacity * sizeof(Voter));

    if (temp == NULL)
        return 0;

    voters = temp;
    voterCapacity = capacity;
    return 1;
}

/* Dynamic memory for candidates */
int growCandidates() {
    if (candidateCount < candidateCapacity)
        return 1;

    int capacity = candidateCapacity == 0 ? 2 : candidateCapacity * 2;

    Candidate *temp = realloc(candidates,
                              (size_t)capacity * sizeof(Candidate));

    if (temp == NULL)
        return 0;

    candidates = temp;
    candidateCapacity = capacity;
    return 1;
}

/* Dynamic memory for transaction logs */
int growLogs() {
    if (logCount < logCapacity)
        return 1;

    int capacity = logCapacity == 0 ? 2 : logCapacity * 2;

    Transaction *temp = realloc(logs,
                                (size_t)capacity * sizeof(Transaction));

    if (temp == NULL)
        return 0;

    logs = temp;
    logCapacity = capacity;
    return 1;
}

/* Register voter */
void registerVoter() {
    if (!growVoters()) {
        printf("Memory allocation failed!\n");
        return;
    }

    Voter v;

    v.id = voterCount + 1;
    v.hasVoted = 0;

    printf("Enter voter name: ");
    readLine(v.name, MAX_NAME);

    printf("Create your PIN: ");
    readLine(v.pin, MAX_PIN);

    if (strlen(v.name) == 0 || strlen(v.pin) == 0) {
        printf("Name and PIN cannot be empty!\n");
        return;
    }

    voters[voterCount++] = v;

    printf("\nVoter registered successfully!\n");
    printf("Your Voter ID is: %d\n", v.id);
}

/* Register candidate */
void registerCandidate() {
    if (!growCandidates()) {
        printf("Memory allocation failed!\n");
        return;
    }

    Candidate c;

    c.id = candidateCount + 1;

    printf("Enter candidate name: ");
    readLine(c.name, MAX_NAME);

    if (strlen(c.name) == 0) {
        printf("Candidate name cannot be empty!\n");
        return;
    }

    candidates[candidateCount++] = c;

    printf("\nCandidate registered successfully!\n");
    printf("Candidate ID: %d\n", c.id);
}

/* Calculate checksum */
unsigned long long calculateChecksum(
    unsigned long long previous,
    Transaction t
) {
    return previous
        + (unsigned long long)t.candidateID * 31ULL
        + (unsigned long long)t.ballotID * 17ULL
        + (unsigned long long)t.timestamp;
}

/* Display candidates */
void displayCandidates() {
    printf("\n------ CANDIDATE LIST ------\n");

    for (int i = 0; i < candidateCount; i++) {
        printf("%d. %s\n",
               candidates[i].id,
               candidates[i].name);
    }
}

/* Cast vote */
void castVote() {
    int id = readInt("\nEnter your Voter ID: ");

    int index = findVoter(id);

    if (index == -1) {
        printf("Voter not found!\n");
        return;
    }

    if (voters[index].hasVoted) {
        printf("You have already voted!\n");
        return;
    }

    char enteredPIN[MAX_PIN];

    printf("Enter your PIN: ");
    readLine(enteredPIN, MAX_PIN);

    if (strcmp(voters[index].pin, enteredPIN) != 0) {
        printf("Incorrect PIN!\n");
        return;
    }

    if (candidateCount == 0) {
        printf("No candidates registered!\n");
        return;
    }

    displayCandidates();

    int choice = readInt("\nEnter candidate ID: ");

    if (findCandidate(choice) == -1) {
        printf("Invalid candidate ID!\n");
        return;
    }

    if (!growLogs()) {
        printf("Memory allocation failed!\n");
        return;
    }

    Transaction t;

    t.candidateID = choice;
    t.timestamp = time(NULL);

    ballotCounter++;

    t.ballotID =
        ((unsigned int)id * 2654435761U)
        ^ ((unsigned int)t.timestamp + ballotCounter);

    unsigned long long previous = 0;

    if (logCount > 0)
        previous = logs[logCount - 1].checksum;

    t.checksum = calculateChecksum(previous, t);

    logs[logCount++] = t;

    voters[index].hasVoted = 1;

    printf("\nVote cast successfully!\n");
    printf("Your vote has been recorded anonymously.\n");
}

/* Calculate votes */
void calculateResults(int tally[]) {
    for (int i = 0; i < logCount; i++) {
        int index = findCandidate(logs[i].candidateID);

        if (index != -1)
            tally[index]++;
    }
}

/* Display results */
void viewResults() {
    if (candidateCount == 0) {
        printf("No candidates registered!\n");
        return;
    }

    int *tally = calloc((size_t)candidateCount, sizeof(int));

    if (tally == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    calculateResults(tally);

    printf("\n====== ELECTION RESULTS ======\n");

    for (int i = 0; i < candidateCount; i++) {
        printf("%s : %d votes\n",
               candidates[i].name,
               tally[i]);
    }

    printf("\nTotal registered voters: %d\n", voterCount);
    printf("Total votes cast: %d\n", logCount);

    if (voterCount > 0) {
        float turnout =
            ((float)logCount / voterCount) * 100;

        printf("Voter turnout: %.2f%%\n", turnout);
    }

    free(tally);
}

/* Display voter details */
void displayVoters() {
    printf("\n====== VOTER DETAILS ======\n");

    if (voterCount == 0) {
        printf("No voters registered.\n");
        return;
    }

    for (int i = 0; i < voterCount; i++) {
        printf("ID: %d | Name: %s | Status: %s\n",
               voters[i].id,
               voters[i].name,
               voters[i].hasVoted ? "Voted" : "Not Voted");
    }
}

/* Audit transaction logs */
void auditLogs() {
    printf("\n====== AUDIT REPORT ======\n");

    int *tally = calloc(
        (size_t)(candidateCount > 0 ? candidateCount : 1),
        sizeof(int)
    );

    if (tally == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    unsigned long long previous = 0;
    int valid = 1;

    for (int i = 0; i < logCount; i++) {
        Transaction t = logs[i];

        unsigned long long expected =
            calculateChecksum(previous, t);

        if (expected != t.checksum ||
            findCandidate(t.candidateID) == -1) {

            printf("Tampering detected at transaction %d!\n",
                   i + 1);

            valid = 0;
            break;
        }

        int index = findCandidate(t.candidateID);
        tally[index]++;

        previous = t.checksum;
    }

    if (valid) {
        printf("Audit successful!\n");
        printf("All transaction checksums are valid.\n");
        printf("Transactions verified: %d\n", logCount);

        printf("\n--- Replayed Results ---\n");

        for (int i = 0; i < candidateCount; i++) {
            printf("%s: %d votes\n",
                   candidates[i].name,
                   tally[i]);
        }
    } else {
        printf("Audit failed! Log is not trustworthy.\n");
    }

    free(tally);
}

/* Display transaction logs */
void displayLogs() {
    printf("\n====== TRANSACTION LOGS ======\n");

    if (logCount == 0) {
        printf("No transactions recorded.\n");
        return;
    }

    for (int i = 0; i < logCount; i++) {
        printf("\nTransaction %d\n", i + 1);

        printf("Ballot ID: %u\n", logs[i].ballotID);
        printf("Candidate ID: %d\n", logs[i].candidateID);

        char *date = ctime(&logs[i].timestamp);

        if (date != NULL)
            printf("Timestamp: %s", date);

        printf("Checksum: %llu\n", logs[i].checksum);
    }
}

/* Main function */
int main() {
    int choice;

    printf("====================================\n");
    printf("    ELECTRONIC VOTING SYSTEM\n");
    printf("         WITH AUDIT TRAIL\n");
    printf("====================================\n");

    do {
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Register Voter\n");
        printf("2. Register Candidate\n");
        printf("3. Cast Vote\n");
        printf("4. View Election Results\n");
        printf("5. Display Voter Details\n");
        printf("6. Audit Vote Records\n");
        printf("7. Display Transaction Logs\n");
        printf("8. Exit\n");

        choice = readInt("\nEnter your choice: ");

        switch (choice) {

            case 1:
                registerVoter();
                break;

            case 2:
                registerCandidate();
                break;

            case 3:
                castVote();
                break;

            case 4:
                viewResults();
                break;

            case 5:
                displayVoters();
                break;

            case 6:
                auditLogs();
                break;

            case 7:
                displayLogs();
                break;

            case 8:
                printf("\nThank you for using the system!\n");
                break;

            default:
                printf("Invalid choice! Try again.\n");
        }

    } while (choice != 8);

    free(voters);
    free(candidates);
    free(logs);

    return 0;
}