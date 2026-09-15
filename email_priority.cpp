/*
EECS 348 Assignment 1
Author(s): Jun Brewer (No collaborators)
Sources: Claude, W3Schools
Creation date: 2026/09/14
No revisions has been made to this program thus far
*/
// email_priority.cpp
//
// Prioritizes a CEO's inbox using a MaxHeap (array/vector-based, built from
// scratch -- no std::priority_queue or other pre-existing heap module).
//
// Priority rules:
//   1. Sender category, highest first:
//        Boss > Subordinate > Peer > ImportantPerson > OtherPerson
//   2. Within the same category, the newest email (by date) is read first.
//
// Commands (read from stdin, one per line):
//   EMAIL <sender category>,<subject line>,<date MM-DD-YYYY>
//   NEXT   - show the next email to be read (does not remove it)
//   READ   - CEO has read/handled the current top email (removes it)
//   COUNT  - display current count of unread emails
//
// Build:  g++ -std=c++17 -O2 -o email_priority email_priority.cpp
// Run:    ./email_priority < testfile.txt

#include <iostream> // imports iostream module
#include <string> // imports string module
#include <vector> // imports vector module

using namespace std; // shortcut so we don't have to type std:: every time

// ---------------------------------------------------------------------
// Email record
// ---------------------------------------------------------------------
struct Email {
    string sender;      // sender category (Boss, Subordinate, Peer, ...)
    string subject;     // subject/content of the Email
    string date;         // original date string, MM-DD-YYYY
    int    categoryRank; // higher = more important
    int    dateValue;    // comparable form of date, higher = newer
};

// Map a sender category string to its priority rank.
// Higher number = higher priority (read first).
int categoryRank(const string& category) {
    if (category == "Boss")           return 5; // If the Email is from boss
    if (category == "Subordinate")    return 4; // If the Email is from Subordinate
    if (category == "Peer")           return 3; // If the Email is from Peer
    if (category == "ImportantPerson")return 2; // If the Email is from an Important Person
    if (category == "OtherPerson")    return 1; // If the Email is from an Other Person
    return 0; // unknown category - lowest priority (shouldn't occur per spec)
}

// Convert MM-DD-YYYY into an integer YYYYMMDD so later dates compare larger.
int dateToValue(const string& date) {
    // Expect exactly "MM-DD-YYYY"
    int mm = 0, dd = 0, yyyy = 0; // initializes the values that we want to return
    // Checks if the input is in the correct format and populating the dates value
    // sscanf() is a c function and doesn't accept the c++ format for strings so we convert it using .c_str()
    if (sscanf(date.c_str(), "%d-%d-%d", &mm, &dd, &yyyy) == 3) {
        return yyyy * 10000 + mm * 100 + dd; // returns it as a single int
    }
    return 0; // malformed date - treat as oldest
}

// Returns true if 'a' has strictly higher reading priority than 'b'.
bool hasHigherPriority(const Email& a, const Email& b) {
    // Checks if 'a' and 'b' do not have equal category rank
    if (a.categoryRank != b.categoryRank) {
        return a.categoryRank > b.categoryRank; // Returns true if 'a' is higher. Otherwise false
    }
    return a.dateValue > b.dateValue; // newer email wins ties
}

// ---------------------------------------------------------------------
// MaxHeap: list(array)-based implementation, built from scratch.
// Root (index 0) always holds the highest-priority email.
// ---------------------------------------------------------------------
class MaxHeap {
public:
    // Insert method for the Maxheap
    void insert(const Email& e) {
        data.push_back(e);  // Pushes a new Email to the end of the vector(array)
        heapifyUp(static_cast<int>(data.size()) - 1); // Upheaps the last element (the one we just added)
    }

    // Remove and return the highest-priority email.
    void pop() {
        data[0] = data.back(); // Replace the first element with the last
        data.pop_back(); // pop the last element
        // Checks if the vector is not empty
        if (!data.empty()) {
            heapifyDown(0); // If so downHeap the element we replace the top with
        }
    }
    
    // Peek method for the MaxHeap
    const Email& peekMax() const {
        return data[0]; // Just returns the first element in vector
    }

    
    bool empty() const { return data.empty(); } // Method for checking if the vector is empty
    size_t size() const { return data.size(); }  // Method that returns the number of elements in vector

private:
    vector<Email> data; // Declaring the actual dataBase vector for Emails

    static int parent(int i) { return (i - 1) / 2; } // Helper Method that finds the parent of the current element
    static int leftChild(int i) { return 2 * i + 1; } // Helper Method that finds the leftChild of the current element
    static int rightChild(int i) { return 2 * i + 2; } // Helper Method that finds the rightChild of the current element

    // Logic for the UpHeaping. While the current element is bigger than its parent and is not the root, it will keep on swapping with parent.
    void heapifyUp(int i) {
        // Checks if it is bigger than its parent and is not the root
        while (i > 0 && hasHigherPriority(data[i], data[parent(i)])) {
            swap(data[i], data[parent(i)]); // swap places with parent
            i = parent(i); // update the pointer to point at the element we were working with
        }
    }

    // Logic for DownHeaping.
    void heapifyDown(int i) {
        int n = static_cast<int>(data.size()); // Creates a value that stores the size of the vector
        // Loops until a break gets called
        while (true) {
            int largest = i; // Keeps track of the index of the largest element as it downheaps
            int l = leftChild(i); // updates the left child
            int r = rightChild(i); // updates the right child

            // Checks if the left child index is valid, and see if parent is greater than child
            if (l < n && hasHigherPriority(data[l], data[largest])) {
                largest = l; // Moves the largest variable to left child
            }
            // Checks if the right child index is valid, and see if parent is greater than child
            if (r < n && hasHigherPriority(data[r], data[largest])) {
                largest = r; // Moves the largest variable to right child
            }

            if (largest == i) { break; } // If parent is greater than its children break out of loop

            swap(data[i], data[largest]); // Swap values
            i = largest; // Updates i to be the largest of the values compared
        }
    }
};

// ---------------------------------------------------------------------
// Helpers for parsing input lines
// ---------------------------------------------------------------------
static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n"); // trims off any leading white spaces, tabs, returns, and newlines
    if (start == string::npos) { return ""; } // returns nothing if it start didn't find anything.
    size_t end = s.find_last_not_of(" \t\r\n"); // trims off any trailing white spaces, tabs, returns, and newlines
    return s.substr(start, end - start + 1); // Returns a substring(copy) of original
}

// Split "EMAIL <category>,<subject>,<date>" into its three fields.
// Subject may contain spaces but never commas, so a simple split on
// commas (into exactly 3 parts, first two split, remainder is date) works.
// Returns true if parse is succesful. False if something went wrong (e.g. wrong input file format)
static bool parseEmailLine(const string& rest, Email& outEmail) {
    // rest is everything after "EMAIL " (already trimmed of leading spaces)
    size_t firstComma = rest.find(','); // Finds the index of the first comma located in string.
    if (firstComma == string::npos) { return false; } // Fails parsing if it cannot find commas

    size_t secondComma = rest.rfind(','); // Finds the index of the second comma located in string
    if (secondComma == string::npos || secondComma == firstComma) { return false; } // Fails parsing if there is only one comma

    string category = trim(rest.substr(0, firstComma)); // Cleans the first section and assigns it to category
    string subject  = trim(rest.substr(firstComma + 1, secondComma - firstComma - 1)); // Cleans the second section and assigns it to subject
    string date     = trim(rest.substr(secondComma + 1)); // Cleans the last section and assigns it to date

    outEmail.sender       = category; // Populate the Emails sender section
    outEmail.subject      = subject; // Populate the Emails subject section
    outEmail.date         = date; // Populate the Emails date section
    outEmail.categoryRank = categoryRank(category); // Populate the Emails categoryrank section
    outEmail.dateValue    = dateToValue(date); // Populate the Emails dateValue section
    return true;
}

// ---------------------------------------------------------------------
// Main: read commands from stdin, drive the MaxHeap
// ---------------------------------------------------------------------
int main() {
    MaxHeap inbox; // Initiates the MaxHeap "Inbox"
    string line; // Variable for storing the line we are processing
    // Loops until there are no more data to read
    // getline doesn't directly output a bool but the while can convert the logic into a bool
    while (getline(cin, line)) {
        string trimmed = trim(line); // creates a trimmed string so it is easier to read
        if (trimmed.empty()) { continue; } // If there is nothing in the string skip the rest of the conditions and continue reading the next command line

        // Looks at the very first element and executes only if it is Email
        // Uses rfind() so it won't keep on searching to the left if it doesn't find "EMAIL"
        if (trimmed.rfind("EMAIL", 0) == 0) {
            // "EMAIL" followed by a space, then the delimited fields
            string rest = trimmed.substr(5); // strip "EMAIL"
            rest = trim(rest); // Takes out any extra spaces, tabs, and newlines
            Email e; // Creates a new Email object
            // populates the new Email with its properties
            if (parseEmailLine(rest, e)) {
                inbox.insert(e); // Inserts the Email into the MaxHeap
            } else {
                cerr << "Warning: could not parse EMAIL line: " << line << endl; // If parsing fails (malformed input) it returns an error instead
            }
        }
        // Since we only expect only one string, we do not need to do rfind()
        // Checks and runs if the program reads "Next"
        else if (trimmed == "NEXT") {
            // If inbox is empty, ouputs a message that notifies that there are no emails in inbox rather than failing silently
            if (inbox.empty()) {
                cout << "Next email:" << endl; // ouput
                cout << "    No emails in inbox." << endl; // ouput
            } else {
                // If inbox is not empty, it returns each value of the email
                const Email& e = inbox.peekMax(); // looks at the root of the Maxheap(0th index) and returns that Email obejct
                cout << "Next email:" << endl; // output
                cout << "    Sender: "  << e.sender  << endl; // output (sender)
                cout << "    Subject: " << e.subject << endl; // output (subject)
                cout << "    Date: "    << e.date    << endl; // output (date)
            }
            cout << endl; // newline
        }
        // Checks and run if the program reads "READ"
        else if (trimmed == "READ") {
            // Checks if there are any Emails
            if (inbox.empty()) {
                cout << "No emails to read." << endl; // output if there is none
                cout << endl; // newline
            } else {
                inbox.pop(); // pops the root
            }
        }
        // Checks and run if the program reads "Count"
        else if (trimmed == "COUNT") {
            cout << "There are " << inbox.size() << " emails to read." << endl;
            cout << endl; // newline
        }
        // Edge case where none of the commands match
        else {
            cerr << "Warning: unrecognized command: " << line << endl; // if none of these cases match the input, it throws an error
        }
    }

    return 0;
}