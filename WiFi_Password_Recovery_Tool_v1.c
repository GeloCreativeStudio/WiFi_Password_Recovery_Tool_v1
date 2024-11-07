/*
 * WiFi Password Recovery Tool v1.0.0
 * 
 * A simple Windows tool to help recover forgotten WiFi passwords from your computer.
 * Created by FEU TECH Computer Science students.
 * 
 * Developers:
 * - Joel Baquiran (202412198@fit.edu.ph)
 * - Angelo Manalo (202410769@fit.edu.ph)
 * 
 * Far Eastern University Institute of Technology
 * Made with 💻 and ❤️ by FEU TECH Students
 */

#include <stdio.h>      // For input/output functions
#include <stdlib.h>     // For system() and memory functions
#include <string.h>     // For string manipulation
#include <windows.h>    // For Windows-specific functions

// Error codes for different scenarios
#define VERSION "1.0.0"
#define ERROR_INSUFFICIENT_PRIVILEGES 0xE001  // When not run as admin
#define ERROR_PROFILE_ACCESS_DENIED  0xE002  // When can't access WiFi profiles
#define ERROR_MEMORY_ALLOCATION      0xE003  // When memory allocation fails

// Program limitations and buffer sizes
#define MAX_CMD_SIZE 256        // Maximum command length
#define MAX_PROFILES 50         // Maximum number of WiFi profiles to store
#define MAX_PROFILE_NAME 100    // Maximum length of profile name
#define MAX_PASSWORD_LENGTH 64  // Maximum length of password

// Console colors for better user interface
#define COLOR_RESET    7    // Default color
#define COLOR_BLUE     9    // For headers
#define COLOR_GREEN    10   // For success messages
#define COLOR_CYAN     11   // For information
#define COLOR_RED      12   // For errors
#define COLOR_YELLOW   14   // For prompts
#define COLOR_WHITE    15   // For normal text

// Structure to store WiFi profile information
typedef struct {
    char name[MAX_PROFILE_NAME];        // Network name (SSID)
    char password[MAX_PASSWORD_LENGTH];  // Network password
    BOOL isSecured;                     // Whether network is secured
} WifiProfile;

// Global handle for console window
HANDLE hConsole;

/**
 * Initialize the console window with proper settings
 * Returns: TRUE if successful, FALSE otherwise
 */
BOOL initConsole(void) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    SetConsoleTitle("WiFi Password Recovery Tool");
    
    // Set window size for better readability
    SMALL_RECT windowSize = {0, 0, 80, 25};
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * Set the color of console text
 * @param color: Color code to set
 */
BOOL setColor(int color) {
    return SetConsoleTextAttribute(hConsole, color);
}

/**
 * Clear the console screen
 */
void clearScreen(void) {
    system("cls");
}

/**
 * Display the program banner with animated developer credits
 */
void printBanner(void) {
    setColor(COLOR_CYAN);
    printf("\n");
    printf("======================================\n");
    setColor(COLOR_YELLOW);
    printf("   WiFi Password Recovery Tool v%s    \n", VERSION);
    setColor(COLOR_CYAN);
    printf("======================================\n");
    
    // Animated developer credits
    setColor(COLOR_GREEN);
    printf("\nDeveloped by:\n");
    
    // Animate first developer
    printf("  > ");
    char dev1[] = "Joel Baquiran (202412198@fit.edu.ph)";
    for(int i = 0; i < strlen(dev1); i++) {
        printf("%c", dev1[i]);
        Sleep(30);  // Slower typing effect
    }
    printf("\n");
    
    // Small delay between developers
    Sleep(200);
    
    // Animate second developer
    printf("  > ");
    char dev2[] = "Angelo Manalo (202410769@fit.edu.ph)";
    for(int i = 0; i < strlen(dev2); i++) {
        printf("%c", dev2[i]);
        Sleep(30);  // Slower typing effect
    }
    printf("\n");
    
    // Add a decorative line after credits
    setColor(COLOR_CYAN);
    printf("--------------------------------------\n");
    setColor(COLOR_RESET);
    Sleep(500);  // Pause briefly to show the complete banner
}

/**
 * Show a spinning animation for loading effect
 */
void showSpinner(void) {
    static char spinner[] = {'|', '/', '-', '\\'};
    static int i = 0;
    printf("\r Scanning for WiFi profiles %c ", spinner[i]);
    i = (i + 1) % 4;
}

/**
 * Display loading animation while scanning
 */
void showLoadingAnimation(void) {
    printf("\n");
    for(int i = 0; i < 15; i++) {
        showSpinner();
        Sleep(100);  // Delay for animation effect
    }
    printf("\r                              \r");
}

/**
 * Get list of all stored WiFi profiles
 * @param profiles: Array to store found profiles
 * @param errorCode: Pointer to store error code if any
 * Returns: Number of profiles found
 */
int getWifiProfiles(WifiProfile profiles[], DWORD* errorCode) {
    FILE *fp;
    char line[256];
    int count = 0;
    
    // Use netsh command to get WiFi profiles
    fp = popen("netsh wlan show profiles | findstr \"All User Profile\"", "r");
    if (fp == NULL) {
        *errorCode = ERROR_INSUFFICIENT_PRIVILEGES;
        return 0;
    }
    
    // Parse each line to get profile names
    while (fgets(line, sizeof(line), fp) && count < MAX_PROFILES) {
        char *start = strstr(line, ": ");
        if (start != NULL) {
            start += 2;  // Skip ": "
            start[strcspn(start, "\n")] = 0;  // Remove newline
            strcpy(profiles[count].name, start);
            profiles[count].isSecured = TRUE;
            memset(profiles[count].password, 0, MAX_PASSWORD_LENGTH);
            count++;
        }
    }
    
    pclose(fp);
    
    if (count == 0) {
        *errorCode = ERROR_PROFILE_ACCESS_DENIED;
    }
    
    return count;
}

/**
 * Display list of available WiFi profiles
 * @param profiles: Array of profiles to display
 * @param count: Number of profiles
 */
void listWifiProfiles(const WifiProfile profiles[], int count) {
    setColor(COLOR_BLUE);
    printf("\n Available WiFi Networks:\n");
    printf(" ----------------------\n");
    setColor(COLOR_RESET);
    
    for (int i = 0; i < count; i++) {
        setColor(COLOR_GREEN);
        printf(" [%2d]", i + 1);
        setColor(COLOR_WHITE);
        printf(" %s\n", profiles[i].name);
    }
    setColor(COLOR_RED);
    printf(" [ 0] Exit Program\n");
    setColor(COLOR_RESET);
}

/**
 * Retrieve password for a specific WiFi profile
 * @param profile: Pointer to profile to get password for
 * Returns: TRUE if password retrieved successfully
 */
BOOL getWifiPassword(WifiProfile* profile) {
    char command[MAX_CMD_SIZE];
    char result[256];
    FILE *fp;
    
    setColor(COLOR_YELLOW);
    printf("\n [*] Target Network: %s\n", profile->name);
    printf(" [*] Initiating password extraction...\n");
    setColor(COLOR_RESET);
    Sleep(500);
    
    snprintf(command, MAX_CMD_SIZE, 
        "netsh wlan show profile name=\"%s\" key=clear | findstr \"Key Content\"", 
        profile->name);
    
    fp = popen(command, "r");
    if (fp == NULL) {
        return FALSE;
    }
    
    BOOL found = FALSE;
    while (fgets(result, sizeof(result), fp)) {
        char *pwd = strstr(result, ": ");
        if (pwd != NULL) {
            pwd += 2;
            pwd[strcspn(pwd, "\n")] = 0;
            strncpy(profile->password, pwd, MAX_PASSWORD_LENGTH - 1);
            found = TRUE;
            break;
        }
    }
    pclose(fp);
    
    if (found) {
        // Animated "decryption" effect
        setColor(COLOR_GREEN);
        printf("\n [+] Decrypting network credentials");
        for(int i = 0; i < 3; i++) {
            Sleep(300);
            printf(".");
        }
        printf("\n");
        
        Sleep(300);
        printf(" [+] Encryption protocol bypassed\n");
        Sleep(200);
        printf(" [+] Password hash cracked\n");
        Sleep(300);
        
        setColor(COLOR_CYAN);
        printf("\n [*] ====================== CREDENTIALS ====================== [*]\n");
        setColor(COLOR_WHITE);
        printf(" [>] Network SSID: %s\n", profile->name);
        printf(" [>] Password    : %s\n", profile->password);
        setColor(COLOR_CYAN);
        printf(" [*] ======================================================= [*]\n");
        
        setColor(COLOR_GREEN);
        printf("\n [+] Operation completed successfully!\n");
        return TRUE;
    }
    
    setColor(COLOR_RED);
    printf("\n [-] Failed to retrieve password. Access denied.\n");
    return FALSE;
}

/**
 * Main program loop
 * Handles user interaction and program flow
 */
int main() {
    WifiProfile profiles[MAX_PROFILES];
    int profileCount;
    int choice;
    DWORD errorCode = 0;
    
    if (!initConsole()) {
        printf("Failed to initialize console. Error code: 0x%lx\n", GetLastError());
        return ERROR_INSUFFICIENT_PRIVILEGES;
    }

    clearScreen();
    printBanner();
    
    while (1) {
        setColor(COLOR_CYAN);
        showLoadingAnimation();
        profileCount = getWifiProfiles(profiles, &errorCode);
        
        if (profileCount == 0) {
            setColor(COLOR_RED);
            printf("\n [!] Error 0x%lx: ", errorCode);
            switch(errorCode) {
                case ERROR_INSUFFICIENT_PRIVILEGES:
                    printf("Insufficient privileges. Please run as administrator.\n");
                    break;
                case ERROR_PROFILE_ACCESS_DENIED:
                    printf("Access denied to WiFi profiles.\n");
                    break;
                default:
                    printf("Unknown error occurred.\n");
            }
            return errorCode;
        }
        
        listWifiProfiles(profiles, profileCount);
        
        setColor(COLOR_YELLOW);
        printf("\n Choose a network (0-%d): ", profileCount);
        setColor(COLOR_WHITE);
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            setColor(COLOR_RED);
            printf("\n [!] Invalid input. Please enter a number.\n");
            setColor(COLOR_YELLOW);
            printf("\n Press Enter to continue...");
            setColor(COLOR_RESET);
            getchar();
            clearScreen();
            printBanner();
            continue;
        }
        while (getchar() != '\n');
        
        if (choice == 0) {
            setColor(COLOR_CYAN);
            printf("\n Thanks for using WiFi Tool! Goodbye!\n\n");
            setColor(COLOR_RESET);
            Sleep(1000);
            break;
        }
        
        if (choice >= 1 && choice <= profileCount) {
            getWifiPassword(&profiles[choice-1]);
            setColor(COLOR_YELLOW);
            printf("\n Press Enter to continue...");
            setColor(COLOR_RESET);
            getchar();
            clearScreen();
            printBanner();
        } else {
            setColor(COLOR_RED);
            printf("\n [!] Invalid choice. Please try again.\n");
            setColor(COLOR_RESET);
            Sleep(1000);
            clearScreen();
            printBanner();
        }
    }
    
    return 0;
}