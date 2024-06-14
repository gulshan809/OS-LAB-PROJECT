#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

// Define user structure
struct User {
    string username;
    string password;
    string role;
};

// Define resource structure
struct Resource {
    string name;
    string owner;
    string permissions[2]; // Permissions for admin and regular users
};

// Sample data
User users[] = {
    {"admin", "admin123", "admin"},
    {"user1", "password1", "regular"},
    {"user2", "password2", "regular"}
};

Resource resources[] = {
    {"file1.txt", "user1", {"true", "true"}},
    {"file2.txt", "user2", {"true", "false"}}
};

// Function to authenticate users with basic validation
User* authenticateUser(const string& username, const string& password) {
    if (username.empty() || password.empty()) {
        MessageBox(NULL, "Username or password cannot be empty.", "Input Error", MB_ICONERROR | MB_OK);
        return nullptr;
    }

    for (auto& user : users) {
        if (user.username == username && user.password == password) {
            return &user;
        }
    }
    return nullptr;
}

// Function to check if a user has permission to access a resource
bool hasAccess(const User* user, const Resource& resource) {
    if (user->role == "admin" || user->username == resource.owner) {
        return true; // Admins and resource owners have full access
    } else {
        int permissionIndex = (user->role == "regular") ? 1 : 0; // Regular users have index 1 in permissions array
        return (resource.permissions[permissionIndex] == "true");
    }
}

// Function to log access attempts
void logAccessAttempt(const string& username, const string& result) {
    ofstream logFile("access_log.txt", ios_base::app);
    logFile << "Username: " << username << " - " << result << endl;
    logFile.close();
}

// Window procedure callback
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hUsernameEdit, hPasswordEdit, hButton;
    static HWND hResultLabel;
    static string resultText;

    switch (msg) {
    case WM_CREATE:
        // Create username label
        CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD, 20, 50, 75, 25, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // Create username edit control
        hUsernameEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 100, 50, 200, 25, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // Create password label
        CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD, 20, 100, 75, 25, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // Create password edit control
        hPasswordEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL, 100, 100, 200, 25, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // Create login button
        hButton = CreateWindow("BUTTON", "Login", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 150, 150, 100, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // Create result label (EDIT control for multiline support)
        hResultLabel = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 100, 200, 200, 100, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            // Get username and password
            char username[256], password[256];
            GetWindowText(hUsernameEdit, username, 256);
            GetWindowText(hPasswordEdit, password, 256);

            // Authenticate user
            User* currentUser = authenticateUser(username, password);
            if (currentUser) {
                resultText = "Access Granted\n";
                resultText += "User Role: " + currentUser->role + "\n";

                // Access control
                for (const auto& resource : resources) {
                    resultText += "Resource: " + resource.name + " (Owner: " + resource.owner + "), Access: " + (hasAccess(currentUser, resource) ? "Granted" : "Denied") + "\n";
                }
                logAccessAttempt(username, "Access Granted");
            } else {
                resultText = "Invalid username or password.";
                logAccessAttempt(username, "Access Denied");
            }

            // Update result label
            SetWindowText(hResultLabel, resultText.c_str());

            // Clear the input fields
            SetWindowText(hUsernameEdit, "");
            SetWindowText(hPasswordEdit, "");
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create window
    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", "Access Control System", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Show window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

   // return msg.wParam;
}
